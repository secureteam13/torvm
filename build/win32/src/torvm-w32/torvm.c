/* Copyright (C) 2008-2009  The Tor Project, Inc.
 * See LICENSE file for rights and terms.
 */
#include "torvm.h"
#include <getopt.h>

#define WIN_DRV_DIR    "system32\\drivers"
#define TOR_TAP_NAME   "Tor VM Tap32"
#define TOR_TAP_SVC    "tortap91"
/* TODO: network config defaults via vmconfig.h and runtime configuration */
#define TOR_TAP_NET    "255.255.255.252"  /* mask 255.255.255.252 or CIDR /30 */
#define TOR_TAP_VMIP   "10.10.10.1" 
#define TOR_TAP_HOSTIP "10.10.10.2" 
#define TOR_TAP_DNS1   "4.2.2.4"
#define TOR_TAP_DNS2   "4.2.2.2"
#define TOR_CAP_SYS    "tornpf.sys"
#define TOR_HDD_FILE   "hdd.img"
#define TOR_RESTRICTED_USER "Tor"
#define QEMU_DEF_MEM   32
#define CAP_MTU        1480

struct s_rconnelem {
  BOOL    isactive;
  BOOL    isdefgw;
  BOOL    isdhcp;
  BOOL    istortap;
  LPTSTR  name;
  LPTSTR  guid;
  LPTSTR  macaddr;
  LPTSTR  ipaddr;
  LPTSTR  netmask;
  LPTSTR  gateway;
  LPTSTR  gwmacaddr;
  LPTSTR  dhcpsvr;
  LPTSTR  svrmacaddr;
  LPTSTR  dhcpname;
  LPTSTR  driver;
  struct s_rconnelem * next;
};

/* logging:
 *   lerror to stderr and log file(s) if set
 *   linfo  to log and debug file
 *   ldebug to debug file
 *   fatal logs error and then exits process
 */
static LPCRITICAL_SECTION  s_logcs = NULL;
static HANDLE  s_logh = INVALID_HANDLE_VALUE;
static HANDLE  s_dbgh = INVALID_HANDLE_VALUE;

void loginit (void) {
  if (!s_logcs) {
    s_logcs = malloc(sizeof(CRITICAL_SECTION));
    createcs(s_logcs);
  }
}

void logto (LPTSTR  path)
{
  loginit();
  entercs(s_logcs);
  if (s_logh != INVALID_HANDLE_VALUE) {
    CloseHandle (s_logh);
  }
  s_logh = CreateFile (path,
                       GENERIC_WRITE,
                       FILE_SHARE_READ,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
}


static void _flog (HANDLE        fda,
                   HANDLE        fdb,
                   HANDLE        fdc,
                   const char *  msgtype,
                   const char *  format,
                   va_list       argptr)
{
  static const int  msgmax = CMDMAX;
  static char *     msgbuf = NULL;
  static char *     coff = NULL;
  const char *      newline = "\r\n";
  int               len;
  int               thrno;
  DWORD             written;
  SYSTEMTIME        now;
  va_list           ap;

  GetSystemTime (&now);
  thrno = thrnum();

  /* XXX: This will block all other threads trying to log while waiting for
   * file I/O. To prevent badness when writes block a log writer thread
   * dedicated to disk I/O should be used and all other logging appends to
   * queue and unblocks.
   *   (For example, write to stalled SMB/USB/etc file system.)
   * XXX: Fix potential race if we're moving log files around.
   */
  loginit();
  entercs(s_logcs);

  if ( (fda == INVALID_HANDLE_VALUE) &&
       (fdb == INVALID_HANDLE_VALUE) &&
       (fdc == INVALID_HANDLE_VALUE)   )
    goto finished;

  if (msgbuf == NULL) {
    msgbuf = malloc (msgmax);
    if (!msgbuf) 
      goto finished;
  }
  coff = msgbuf;
  coff[msgmax -1] = 0;
  len = snprintf (coff,
                  msgmax -1,
                  "[%4.4d/%-2.2d/%-2.2d %-2.2d:%-2.2d:%-2.2d.%-3.3d UTC][Thr %d] %s: ",
                  now.wYear,
                  now.wMonth,
                  now.wDay,
                  now.wHour,
                  now.wMinute,
                  now.wSecond,
                  now.wMilliseconds,
                  thrno,
                  msgtype);
  if (len > 0) {
    coff += len;
    len = vsnprintf (coff,
                     (msgmax -1) - len,
                     format,
                     argptr);
    if (len > 0) {
      len += abs(coff - msgbuf);
    }
    else {
      /* full msg buffer */
      len = msgmax -1;
    }
  }
  else {
    /* full msg buffer */
    len = msgmax -1;
  }
  if (fda != INVALID_HANDLE_VALUE) {
    WriteFile (fda, msgbuf, len, &written, NULL);
    WriteFile (fda, newline, strlen(newline), &written, NULL);
    FlushFileBuffers (fda);
  }
  if (fdb != INVALID_HANDLE_VALUE) {
    WriteFile (fdb, msgbuf, len, &written, NULL);
    WriteFile (fdb, newline, strlen(newline), &written, NULL);
    FlushFileBuffers (fdb);
  }
  if (fdc != INVALID_HANDLE_VALUE) {
    WriteFile (fdc, msgbuf, len, &written, NULL);
    WriteFile (fdc, newline, strlen(newline), &written, NULL);
    FlushFileBuffers (fdc);
  }

 finished:
  leavecs(s_logcs);
  return;
}


void fatal (const char* format, ...)
{
  HANDLE   fd = INVALID_HANDLE_VALUE;
  va_list  argptr;
  
  fd = GetStdHandle (STD_ERROR_HANDLE);
  if (fd == INVALID_HANDLE_VALUE)
    fd = GetStdHandle (STD_OUTPUT_HANDLE);
    
  va_start (argptr, format);
  _flog (fd, s_logh, s_dbgh, "FATAL", format, argptr);
  va_end (argptr);
  _exit (9);
  return;
}


void lerror (const char * format, ...)
{
  HANDLE   fd = INVALID_HANDLE_VALUE;
  va_list  argptr;

  fd = s_logh;
  if (fd == INVALID_HANDLE_VALUE) {
    fd = GetStdHandle (STD_ERROR_HANDLE);
    if (fd == INVALID_HANDLE_VALUE)
      fd = GetStdHandle (STD_OUTPUT_HANDLE);
  }

  va_start (argptr, format);
  _flog (fd, s_logh, s_dbgh, "ERROR", format, argptr);
  va_end (argptr);
  return;
}

void linfo (const char* format, ...)
{
  va_list  argptr;
  va_start (argptr, format);
  _flog (s_logh, s_dbgh, INVALID_HANDLE_VALUE, "info", format, argptr);
  va_end (argptr);
  return;
}

void debugto (LPTSTR  path)
{ 
  loginit();
  entercs(s_logcs);

  if (s_dbgh != INVALID_HANDLE_VALUE) {
    CloseHandle (s_dbgh);
  }
  s_dbgh = CreateFile (path,
                       GENERIC_WRITE,
                       FILE_SHARE_READ,
                       NULL,
                       CREATE_ALWAYS,   
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
  leavecs(s_logcs);
}


void ldebug (const char* format, ...)
{
  va_list  argptr;
  va_start (argptr, format);
  _flog (s_dbgh, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, "debug", format, argptr);
  va_end (argptr);
  return;
}

/* XXX: for now we just print some things to console to indicate what is going on.
 * At some point a better interface with localized string will be needed.
 * We always grab a current stdout handle in case it gets redirected at some point.
 * (For example, as a service these messages would go into events)
 */
static void dispmsg(LPTSTR msg)
{
  HANDLE  hnd;
  hnd = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hnd != INVALID_HANDLE_VALUE) {
    WriteFile(hnd, msg, strlen(msg), NULL, NULL);
    msg = "\r\n";
    WriteFile(hnd, msg, strlen(msg), NULL, NULL);
  }
}

static BOOL escquote(LPTSTR  path,
                     LPTSTR *epath)
{
  DWORD  buflen;
  LPTSTR ci = path;
  LPTSTR cv;
  *epath = NULL;
  if (!*path)
    return FALSE;
  buflen = strlen(path)*2 + 1;
  *epath = malloc(buflen);
  if (!*epath)
    return FALSE;
  cv=*epath;
  while (*ci) {
    if (*ci == '\\') {
      *cv++ = '\\';
      *cv++ = '\\';
    }
    else {
      *cv++ = *ci;
    }
    *ci++;
  }
  *cv = 0;
  return TRUE;
}

BOOL copyvidaliacfg (LPTSTR srcpath,
                     LPTSTR destpath,
                     LPTSTR datadir)
{
  HANDLE src, dest;
  DWORD buffsz = CMDMAX;
  DWORD len, written;
  LPTSTR buff;
  LPTSTR epath;
  src = CreateFile (srcpath,
                    GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
 /* XXX: nobody cares
  if (src == INVALID_HANDLE_VALUE) {
    return FALSE;
  }
 */
  dest = CreateFile (destpath,
                     GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_NEW,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
  if (dest == INVALID_HANDLE_VALUE) {
    return FALSE;
  }
  buff = malloc(buffsz);
  if (!buff) 
    return FALSE;
  snprintf(buff, buffsz -1, "RunTorAtStart=true\r\n\r\n");
  WriteFile(dest, buff, strlen(buff), &written, NULL);

  /* XXX: Let Tor VM launch Polipo directly now. */
#if 0
  if (escquote(polipocfg, &epath)) {
    snprintf(buff, buffsz -1,
             "[General]\r\nProxyExecutableArguments=-c, %s\r\n",
             epath);
    WriteFile(dest, buff, strlen(buff), &written, NULL);
    free(epath);
  }
  while (ReadFile(src, buff, buffsz, &len, NULL) && (len > 0)) 
    WriteFile(dest, buff, len, &written, NULL);
#endif

  snprintf(buff, buffsz -1, "[Tor]\r\nChanged=true\r\nTorExecutable=\r\n");
  WriteFile(dest, buff, strlen(buff), &written, NULL);
  if (escquote(datadir, &epath)) {
    snprintf(buff, buffsz -1,
             "DataDirectory=%s\r\n",
             epath);
    WriteFile(dest, buff, strlen(buff), &written, NULL);
    free(epath);
  }
  snprintf(buff, buffsz -1, "ControlAddr=%s\r\n",
           TOR_TAP_VMIP);
  WriteFile(dest, buff, strlen(buff), &written, NULL);
  snprintf(buff, buffsz -1, "UseRandomPassword=false\r\n");
  WriteFile(dest, buff, strlen(buff), &written, NULL);
  snprintf(buff, buffsz -1, "ControlPassword=%s\r\n",
           "password"); /* XXX: TEMP static default passwd */
  WriteFile(dest, buff, strlen(buff), &written, NULL);
  free (buff);
  CloseHandle (src);
  CloseHandle (dest);

  return TRUE;
}

BOOL installtap(void)
{
  BOOL retval = TRUE;
  LPTSTR cmd = NULL;
  LPTSTR dir = NULL;
  LPTSTR devcon = NULL;
  DWORD cmdlen;

  if (!buildfpath(PATH_FQ, VMDIR_LIB, NULL, NULL, &dir)) {
    lerror ("Unable to build path for lib dir.");
    return FALSE;
  }
  if (!buildfpath(PATH_FQ, VMDIR_BIN, dir, "devcon.exe", &devcon)) {
    lerror ("Unable to build path for devcon.exe utility.");
    return FALSE;
  }

  cmdlen = strlen(devcon) + 64;
  cmd = malloc(cmdlen);
  snprintf (cmd, cmdlen, "\"%s\" install tortap91.inf TORTAP91", devcon);
  ldebug ("Tap install pwd: %s, cmd: %s", dir, cmd);
  if (runcommand(cmd,dir))
    retval = TRUE;
  if (cmd)
    free(cmd);
  return retval;
}

BOOL uninstalltap(void)
{
  BOOL retval = FALSE;
  LPTSTR cmd = NULL;
  LPTSTR dir = NULL;
  LPTSTR devcon = NULL;
  DWORD cmdlen;
  LONG status;
  HKEY key;
  DWORD len;
  int i = 0;
  int stop = 0;
  int numconn = 0;
  const char name_string[] = "Name";
  char svc_string[REG_NAME_MAX];
  
  if (!buildfpath(PATH_FQ, VMDIR_LIB, NULL, NULL, &dir)) {
    lerror ("Unable to build path for lib dir.");
    return FALSE;
  }
  if (!buildfpath(PATH_FQ, VMDIR_BIN, dir, "devcon.exe", &devcon)) {
    lerror ("Unable to build path for devcon.exe utility.");
    return FALSE;
  }

  cmdlen = strlen(devcon) + 64;
  cmd = malloc(cmdlen);
  snprintf (cmd, cmdlen, "\"%s\" install tortap91.inf TORTAP91", devcon);
  ldebug ("Tap un-install pwd: %s, cmd: %s", dir, cmd);
  if (runcommand(cmd,dir))
    retval = TRUE; 
  if (cmd)
    free (cmd);

  ldebug ("Removal complete.  Checking registry for Tor Tap connection entries.");
  /* clean up registry keys left after tap adapter is removed
   */
  status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        NETWORK_CONNECTIONS_KEY,
                        0,
                        KEY_READ,
                        &key);
  if (status != ERROR_SUCCESS) {
    lerror ("Failed to open key for read: %d", status); 
    return -1;
  }

  while (!stop) {
    char enum_name[REG_NAME_MAX];
    char connection_string[REG_NAME_MAX];
    HKEY ckey;
    HKEY dkey;
    char name_data[REG_NAME_MAX];
    DWORD name_type;
    int j;

    len = sizeof (enum_name);
    status = RegEnumKeyEx(key,
                          i++,
                          enum_name,
                          &len,
                          NULL,
                          NULL,
                          NULL,
                          NULL);
    if (status == ERROR_NO_MORE_ITEMS)
        break;
    else if (status != ERROR_SUCCESS) {
      lerror ("Failed to query members of network connection tree.");
      RegCloseKey (key);
      return FALSE;
    }

    ldebug ("Checking connection entry %s name.", enum_name);
    snprintf(connection_string,
             sizeof(connection_string),
             "%s\\%s\\Connection",
             NETWORK_CONNECTIONS_KEY, enum_name);
    status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            connection_string,
            0,
            KEY_READ,
            &ckey);

    if (status == ERROR_SUCCESS) {
        len = sizeof (name_data);
        status = RegQueryValueEx(
                ckey,
                name_string,
                NULL,
                &name_type,
                name_data,
                &len);

      if (status != ERROR_SUCCESS || name_type != REG_SZ) {
        continue;
      }
      if (strcmp(name_data, TOR_TAP_NAME) == 0) {
        /* remove this connection entry to non-existant Tor Tap32 device */
        ldebug ("Removing registry data for %s adapter key %s.", TOR_TAP_NAME, enum_name);
        ldebug ("Deleting Connection subkey.");
        snprintf(connection_string,
                 sizeof(connection_string),
                 "%s\\%s",
                 NETWORK_CONNECTIONS_KEY, enum_name);
        status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              connection_string,
                              0,
                              KEY_SET_VALUE,
                              &dkey);
        if (status != ERROR_SUCCESS) {
          lerror ("Failed to open network connection key for write: %d", GetLastError());
          continue; 
        }
        /* now we can delete the connection key itself */
        status = RegDeleteKey(dkey, "Connection");
        if (status != ERROR_SUCCESS) {
          lerror ("Failed to remove tap connection subkey from registry: %d", GetLastError());
        }
        RegCloseKey (dkey);
        /* finally, remove the top level connection key from the list of connections ids */
        ldebug ("Deleting connection entry %s from top level connections key.", enum_name);
        status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              NETWORK_CONNECTIONS_KEY,
                              0,
                              KEY_SET_VALUE, 
                              &dkey);
        if (status != ERROR_SUCCESS) {
          lerror ("Failed to open top level network connection key for write: %d", GetLastError());
        }
        status = RegDeleteKey(dkey, enum_name);
        if (status != ERROR_SUCCESS) {
          lerror ("Failed to remove top level tap key from registry: %d", GetLastError());
        }
        RegCloseKey (dkey);
      }
      RegCloseKey (ckey);
    }
  }

  RegCloseKey (key);

  /* now for any service entries for the Tor tap service */
  snprintf(svc_string,
           sizeof(svc_string),
           "%s\\%s",
           SERVICES_KEY, TOR_TAP_SVC);
  status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        svc_string,
                        0,
                        KEY_READ,
                        &key);
  if (status == ERROR_SUCCESS) {
    RegCloseKey (key);
    ldebug ("Found a Tor tap service entry.  Attempting removal...");
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          svc_string,
                          0,
                          KEY_SET_VALUE,
                          &key);
    if (status != ERROR_SUCCESS) {
      lerror ("Cannot open Tor tap services key for write access.  Error code: %d", GetLastError());
    }
    else {
      RegDeleteKey(key, "Enum");
      RegDeleteKey(key, "Security");
      RegCloseKey (key);
    }
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          SERVICES_KEY,
                          0, 
                          KEY_SET_VALUE,
                          &key);
    if (status != ERROR_SUCCESS) {
      lerror ("Cannot open service parent key for write access to remove tap subkey.  Error code: %d", GetLastError());
    }
    else {
      status = RegDeleteKey(key, TOR_TAP_SVC);
      if (status != ERROR_SUCCESS) {
        lerror ("Failed to remove tap service key from registry: %d", GetLastError());
      }
      else {
        ldebug ("Removal complete.");
      }
      RegCloseKey (key);
    }
  }
  
  return TRUE;
}

BOOL installtornpf (void)
{
  HANDLE src = NULL;
  HANDLE dest = NULL;
  LPTSTR srcname = NULL;
  LPTSTR destname = NULL;
  CHAR * buff = NULL;
  DWORD  buffsz = CMDMAX;
  DWORD  len;
  DWORD  written;
  if (!buildsyspath(SYSDIR_WINROOT, WIN_DRV_DIR "\\" TOR_CAP_SYS, &destname)) {
    lerror ("Unable to build path for WINROOT.");
    return FALSE;
  } 
  if (!buildfpath(PATH_FQ, VMDIR_LIB, NULL, TOR_CAP_SYS, &srcname)) {
    free (destname);
    srcname = "C:\\Tor_VM\\lib\\" TOR_CAP_SYS;
    lerror ("Unable to build path for %s, using default: %s", TOR_CAP_SYS, srcname);
  }
  
  src = CreateFile (srcname,
                    GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
  if (src == INVALID_HANDLE_VALUE) {
    free (destname);
    return FALSE;
  } 
  dest = CreateFile (destname,
                     GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_SYSTEM,
                     NULL);
  free (destname);
  if (dest == INVALID_HANDLE_VALUE) {
    return FALSE;
  } 
  
  buff = malloc(buffsz);
  while (ReadFile(src, buff, buffsz, &len, NULL) && (len > 0)) {
    WriteFile(dest, buff, len, &written, NULL);
  }
  free (buff);
  CloseHandle (src);
  CloseHandle (dest);

  return TRUE;
}

BOOL uninstalltornpf (void)
{
  LPTSTR fname = NULL;
  LPTSTR cmd = "\"net.exe\" stop tornpf";
  if (! runcommand(cmd,NULL)) {
    lerror ("Unable to run net stop for tornpf service.");
  }
  if (0) { /* XXX: for now we don't ever delete the npf device file. */
    if (!buildsyspath(SYSDIR_WINROOT, WIN_DRV_DIR "\\" TOR_CAP_SYS, &fname)) {
      lerror ("Unable to build path for WINROOT to uninstall tap.");
      return FALSE;
    } 
    DeleteFile (fname);
    free (fname);
  } 
  return TRUE;
}

BOOL savenetconfig(void)
{
#define READSIZE 4096
  HANDLE fh = NULL;
  HANDLE stdin_rd = NULL;
  HANDLE stdin_wr = NULL;
  HANDLE stdout_rd = NULL;
  HANDLE stdout_wr = NULL;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  SECURITY_ATTRIBUTES sattr;
  LPTSTR cmd = NULL;
  LPTSTR dir = NULL;
  LPTSTR savepath = NULL;
  DWORD exitcode;
  DWORD opts = CREATE_NEW_PROCESS_GROUP;
  DWORD numread;
  DWORD numwritten;
  CHAR * buff = NULL;

  /* for vista or above also save the firewall context
   * do this before the IP context so that restore doesn't leave us vulnerable (even briefly)
   */
  if (getosversion() >= OS_VISTA) {
    cmd = "\"netsh.exe\" advfirewall export \"" TOR_VM_STATE "\\firewall.wfw\"";
    runcommand(cmd,NULL);
    linfo ("Saved current firewall configuration state.");
  }

  if (!buildfpath(PATH_FQ, VMDIR_STATE, NULL, NULL, &dir)) {
    lerror ("Unable to build path for state dir.");
    return FALSE;
  }
  if (!buildfpath(PATH_FQ, VMDIR_STATE, NULL, "netcfg.save", &savepath)) {
    lerror ("Unable to build path for save file in state dir.");
    return FALSE;
  }
  fh = CreateFile (savepath,
                   GENERIC_WRITE,
                   0,
                   NULL,
                   CREATE_ALWAYS,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL);
  if (fh == INVALID_HANDLE_VALUE) {
    ldebug ("Unable to open network save file for writing. Error code: %d", GetLastError());
    return FALSE;
  }
  ldebug ("Opened %s for write at offset 0", savepath);

  ZeroMemory( &pi, sizeof(pi) );
  ZeroMemory( &si, sizeof(si) );
  ZeroMemory( &sattr, sizeof(sattr) );
  si.cb = sizeof(si);
  sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
  sattr.bInheritHandle = TRUE;
  sattr.lpSecurityDescriptor = NULL;
  cmd = "\"netsh.exe\" interface ip dump";

  CreatePipe(&stdout_rd, &stdout_wr, &sattr, 0);
  SetHandleInformation(stdout_rd, HANDLE_FLAG_INHERIT, 0);

  CreatePipe(&stdin_rd, &stdin_wr, &sattr, 0);
  SetHandleInformation(stdin_wr, HANDLE_FLAG_INHERIT, 0);

  si.hStdError = stdout_wr;
  si.hStdOutput = stdout_wr;
  si.hStdInput = stdin_rd;
  si.dwFlags |= STARTF_USESTDHANDLES;

  if( !CreateProcess(NULL,
                     cmd,
                     NULL,   // process handle no inherit
                     NULL,   // thread handle no inherit
                     TRUE,   // must inherit handles for redirection to work
                     opts,
                     NULL,   // environment block
                     dir,
                     &si,
                     &pi) ) {
    lerror ("Failed to launch netsh process.  Error code: %d", GetLastError());
    if (buff)
      free(buff);
    CloseHandle(fh);
    return FALSE;
  }

  CloseHandle(stdout_wr);
  CloseHandle(stdin_rd);
  CloseHandle(stdin_wr);

  buff = malloc(READSIZE);
  while (ReadFile(stdout_rd, buff, READSIZE, &numread, NULL) && (numread > 0)) {
    WriteFile(fh, buff, numread, &numwritten, NULL);
    ldebug ("Read %d bytes from net dump and wrote %d to save file.", numread, numwritten);
  }

  linfo ("Saved current IP network configuration state.");
  free(buff);
  CloseHandle(stdout_rd);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  CloseHandle (fh);

  return TRUE;  
}

BOOL restorenetconfig(void)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  LPTSTR cmd = NULL;
  LPTSTR dir = NULL;
  LPTSTR savepath = NULL;
  DWORD exitcode;
  DWORD opts = 0;

  if (getosversion() >= OS_VISTA) {
    cmd = "\"netsh.exe\" advfirewall import \"" TOR_VM_STATE "\\firewall.wfw\"";
    runcommand(cmd,NULL);
    linfo ("Imported saved firewall configuration.");
  }

  if (!buildfpath(PATH_FQ, VMDIR_STATE, NULL, NULL, &dir)) {
    lerror ("Unable to build path for state dir.");
    return FALSE;
  }
  if (!buildfpath(PATH_FQ, VMDIR_STATE, NULL, "netcfg.save", &savepath)) {
    lerror ("Unable to build path for save file in state dir.");
    return FALSE;
  }

  ZeroMemory( &pi, sizeof(pi) );
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  cmd = "\"netsh.exe\" exec netcfg.save";

  if( !CreateProcess(NULL,
                     cmd,
                     NULL,   // process handle no inherit
                     NULL,   // thread handle no inherit
                     FALSE,  // default handle inheritance false
                     opts,
                     NULL,   // environment block
                     dir,
                     &si,
                     &pi) ) {
    lerror ("Failed to launch process.  Error code: %d", GetLastError());
  }

  while ( GetExitCodeProcess(pi.hProcess, &exitcode) && (exitcode == STILL_ACTIVE) ) {
    Sleep (500);
  }

  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  ldebug ("Removing original network save file at %s", savepath);
  DeleteFile (savepath);

  linfo ("Restored current network configuration state.");
  return TRUE;  
}

BOOL disableservices(void)
{
  /* TODO: check which of the following are running and stop them.
   * RemoteRegistry lanmanserver TermService WebClient LmHosts Messenger mnmsrvc RDSessMgr
   * also need to remember what was running so we can resume it at shutdown.
   */
  return TRUE;
}

BOOL disablefirewall(void)
{
  LPTSTR cmd = "\"netsh.exe\" firewall set opmode disable";
  ldebug ("Disable firewall cmd: %s", cmd);
  if (! runcommand(cmd,NULL)) {
    return FALSE;
  }
  return TRUE;
}

BOOL enablefirewall(void)
{
  /* TODO: we need to check if exceptions are disabled, and set opmode enable disable accordingly. */
  LPTSTR cmd = "\"netsh.exe\" firewall set opmode enable";
  LPTSTR dir = NULL;
  if (!buildfpath(PATH_FQ, VMDIR_BIN, NULL, NULL, &dir)) {
    lerror ("Unable to build path for bin dir.");
    return FALSE;
  }
  ldebug ("Re-enable firewall cmd: %s", cmd);
  if (! runcommand(cmd,NULL)) {
    return FALSE;
  }
  return TRUE;
}

BOOL cleararpcache(void)
{
  LPSTR cmd;
  cmd = "\"netsh.exe\" interface ip delete arpcache";
  ldebug ("Clear ARP cache cmd: %s", cmd);
  if (! runcommand(cmd,NULL)) {
    return FALSE;
  }
  return TRUE;
}

BOOL flushdns(void)
{ 
  LPSTR cmd = "\"ipconfig.exe\" /flushdns";
  ldebug ("Flush DNS cmd: %s", cmd);
  if (! runcommand(cmd,NULL)) {
    return FALSE;
  }
  return TRUE;
}

BOOL configtap(void)
{
  const DWORD cmdlen = 1024;
  LPTSTR cmd;
  LPTSTR netsh = "netsh.exe";
  cmd = malloc(cmdlen);

  snprintf (cmd, cmdlen,
            "\"%s\" interface ip set address \"%s\" static %s %s %s 1",
            netsh,
            TOR_TAP_NAME,
            TOR_TAP_HOSTIP,
            TOR_TAP_NET,
            TOR_TAP_VMIP);
  ldebug ("Tap config cmd: %s", cmd);
  if (! runcommand(cmd,NULL)) {
    free (cmd);
    return FALSE;
  }
  snprintf (cmd, cmdlen,
            "\"%s\" interface ip set dns  \"%s\" static %s",
            netsh,
            TOR_TAP_NAME,
            TOR_TAP_DNS1);
  ldebug ("Tap dns config cmd: %s", cmd);
  if (! runcommand(cmd,NULL)) {
    free (cmd);
    return FALSE;
  }
  snprintf (cmd, cmdlen,
            "\"%s\" interface ip add dns  \"%s\" %s",
            netsh,
            TOR_TAP_NAME,
            TOR_TAP_DNS2);
  ldebug ("Tap dns2 config cmd: %s", cmd);
  if (! runcommand(cmd,NULL)) {
    free (cmd);
    return FALSE;
  }
  ldebug ("Tap config complete.");
  free (cmd);
  return TRUE;
}

BOOL configbridge(void)
{
  LPSTR cmd;
  cmd = "\"netsh.exe\" interface ip set address \"Local Area Connection\" static 10.231.254.1 255.255.255.254";
  ldebug ("Bridge interface null route cmd: %s", cmd);
  if (! runcommand(cmd,NULL)) {
    return FALSE;
  }
  return TRUE;
}

BOOL checkvirtdisk(void) {
  HANDLE src = NULL;
  HANDLE dest = NULL;
  LPTSTR srcname = NULL;
  LPTSTR destname = NULL;
  CHAR * buff = NULL;
  DWORD  buffsz = 4096;
  DWORD  len;
  DWORD  written;

  if (!buildfpath(PATH_FQ, VMDIR_LIB, NULL, TOR_HDD_FILE, &srcname)) {
    lerror ("Unable to build path for src %s", TOR_HDD_FILE);
    return FALSE;
  }
  if (!buildfpath(PATH_FQ, VMDIR_STATE, NULL, TOR_HDD_FILE, &destname)) {
    lerror ("Unable to build path for dest %s", TOR_HDD_FILE);
    return FALSE;
  }
  
  dest = CreateFile (destname,
                     GENERIC_READ,
                     0,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
  if (dest == INVALID_HANDLE_VALUE) {
    if (GetLastError() != ERROR_FILE_NOT_FOUND) {
      return FALSE;
    }
  } 
  else {
    CloseHandle (dest);
    return TRUE;
  }

  dest = CreateFile (destname,
                     GENERIC_WRITE,
                     0,  
                     NULL,
                     CREATE_NEW,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
  if (dest == INVALID_HANDLE_VALUE) {
    lerror ("Unable to open virtual disk file %s for writing", destname);
    return FALSE;
  }
 
  src = CreateFile (srcname,
                    GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
  if (src == INVALID_HANDLE_VALUE) {
    lerror ("Unable to open virtual disk file %s for reading", srcname);
    CloseHandle (dest);
    return FALSE;
  }
  
  buff = malloc(buffsz);
  while (ReadFile(src, buff, buffsz, &len, NULL) && (len > 0)) {
    WriteFile(dest, buff, len, &written, NULL);
  }
  ldebug ("Created new virtual disk image file at %s", destname);
  free (buff);
  free (srcname);
  free (destname);
  CloseHandle (src);
  CloseHandle (dest);

  return TRUE;
}

int loadnetinfo(struct s_rconnelem **connlist)
{
  LONG status;
  HKEY key;
  HKEY wkey;
  DWORD len;
  DWORD retval;
  int i, j;
  int numconn = 0;
  struct s_rconnelem *  ce = NULL;
  struct s_rconnelem *  ne = NULL;
  const char name_string[] = "Name";
  ULONG arpentsz = 128 * sizeof(MIB_IPNETROW);
  PMIB_IPNETTABLE pmib = NULL;
  IN_ADDR addr;
  char *ipstr;

  /* Load the ARP table before iterating through interfaces
   */
  pmib = malloc(sizeof(MIB_IPNETTABLE)+arpentsz);
  
  retval = GetIpNetTable(pmib,&arpentsz,FALSE);
  if (retval == ERROR_INSUFFICIENT_BUFFER) {
    /* XXX: re-alloc instead with returned hint */
    lerror ("ARP table is huge, skipping static ARP assignments. Would need %d.", arpentsz);
    free(pmib);
    pmib = NULL;
  }
  else {
    if (retval != NO_ERROR) {
      lerror ("GetIpNetTable failed with error code %d in call to loadnetinfo.", retval);
      free(pmib);
      pmib = NULL;
    }
    else {
      for (i=0; i<pmib->dwNumEntries; i++) {
        addr.S_un.S_addr = pmib->table[i].dwAddr;
        ipstr = inet_ntoa(addr);
        if (!ipstr)
          ipstr = "";
        if (pmib->table[i].dwPhysAddrLen == 6) {
          ldebug ("MIB enumerate found ARP entry HWADDR: %02X:%02X:%02X:%02X:%02X:%02X -> IP: %s [%s]",
                  pmib->table[i].bPhysAddr[0],pmib->table[i].bPhysAddr[1],
                  pmib->table[i].bPhysAddr[2],pmib->table[i].bPhysAddr[3],
                  pmib->table[i].bPhysAddr[4],pmib->table[i].bPhysAddr[5],
                  ipstr,
                  /* 4-Static, 3-Dynamic, 2-Invalid, 1-Other */
                  (pmib->table[i].dwType == 3) ? "Dynamic" : "Static"
                 );
        }
        else {
          ldebug ("MIB enumerate found ARP entry with non Ethernet sized physical address for IP: %s. Ignoring.",
                  ipstr);
        }
      }
    }
  }


  /* Now enumerate all interfaces and list details for caller.
   */
  status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        NETWORK_CONNECTIONS_KEY,
                        0,
                        KEY_READ,
                        &key);
  if (status != ERROR_SUCCESS) {
    lerror ("Failed to open key for read: %d", status); 
    return -1;
  }

  i = 0;
  while (1) {
    char enum_name[REG_NAME_MAX];
    char connection_string[REG_NAME_MAX];
    char tcpip_string[REG_NAME_MAX];
    HKEY ckey;
    HKEY tkey;
    char name_data[REG_NAME_MAX];
    DWORD name_type;

    len = sizeof (enum_name);
    status = RegEnumKeyEx(key,
                          i++,
                          enum_name,
                          &len,
                          NULL,
                          NULL,
                          NULL,
                          NULL);
    if (status == ERROR_NO_MORE_ITEMS)
        break;
    else if (status != ERROR_SUCCESS) {
        break;
    }

    snprintf(connection_string,
             sizeof(connection_string),
             "%s\\%s\\Connection",
             NETWORK_CONNECTIONS_KEY, enum_name);

    status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            connection_string,
            0,
            KEY_READ,
            &ckey);

    if (status == ERROR_SUCCESS) {
        len = sizeof (name_data);
        status = RegQueryValueEx(
                ckey,
                name_string,
                NULL,
                &name_type,
                name_data,
                &len);

      if (status != ERROR_SUCCESS || name_type != REG_SZ) {
        break;
      }
      else {
        /* add this connection info to the list */
        numconn++;
        if (ce == NULL) {
          *connlist = ce = malloc(sizeof(struct s_rconnelem));
          memset(ce, 0, sizeof(struct s_rconnelem));
        }
        else {
          ne = malloc(sizeof(struct s_rconnelem));
          memset(ne, 0, sizeof(struct s_rconnelem));
          ce->next = ne;
          ce = ne;
        }
        ce->name = strdup(name_data);
        ce->guid = strdup(enum_name);
        if (getmacaddr (ce->guid, &(ce->macaddr))) {
          linfo ("Interface %s => %s  mac(%s)", name_data, enum_name, ce->macaddr);
        }
        if (isconnected (ce->guid)) {
          linfo ("Interface %s (%s) is currently connected.", ce->name, ce->macaddr);
          ce->isactive = TRUE;
          snprintf(tcpip_string,
                   sizeof(tcpip_string),
                   "%s\\%s",
                   TCPIP_INTF_KEY, enum_name);
          status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                tcpip_string,
                                0,
                                KEY_READ,   
                                &tkey);
          if (status == ERROR_SUCCESS) {
            len = sizeof (BOOL);
            status = RegQueryValueEx(tkey,
                                     "EnableDHCP",
                                     NULL,
                                     NULL,
                                     (LPBYTE)&(ce->isdhcp),
                                     &len);
            if (status == ERROR_SUCCESS) {
              ldebug ("Connection %s %s using DHCP.", ce->name, ce->isdhcp ? "is" : "is NOT");
            }
            len = sizeof (name_data);
            status = RegQueryValueEx(tkey,
                                     ce->isdhcp ? "DhcpDefaultGateway" : "DefaultGateway",
                                     NULL,
                                     &name_type,
                                     name_data,
                                     &len);
            if (status == ERROR_SUCCESS) {
              ce->gateway = strdup(name_data); 
              ldebug ("Connection %s default gateway: %s.", ce->name, ce->gateway); 
              if ( (strlen(ce->gateway) > 6) && (strcmp(ce->gateway, "0.0.0.0") != 0) ) {
                ce->isdefgw = TRUE;
                ldebug ("Connection %s has the default route.", ce->name);
              }
            }
            len = sizeof (name_data);
            status = RegQueryValueEx(tkey,
                                     ce->isdhcp ? "DhcpIPAddress" : "IPAddress",
                                     NULL,
                                     &name_type,
                                     name_data,
                                     &len);
            if (status == ERROR_SUCCESS) {
              ce->ipaddr = strdup(name_data); 
              ldebug ("Connection %s current IP address: %s.", ce->name, ce->ipaddr); 
            }
            len = sizeof (name_data);
            status = RegQueryValueEx(tkey,
                                     ce->isdhcp ? "DhcpSubnetMask" : "SubnetMask",
                                     NULL,
                                     &name_type,
                                     name_data,
                                     &len);
            if (status == ERROR_SUCCESS) {
              ce->netmask = strdup(name_data);
              ldebug ("Connection %s netmask: %s.", ce->name, ce->netmask);
            }
            /* Set ARP entries for this interface if needed. */
            if (pmib && ce->isdefgw) {
              for (j=0; j<pmib->dwNumEntries; j++) {
                addr.S_un.S_addr = pmib->table[j].dwAddr;
                ipstr = inet_ntoa(addr);
                if ((pmib->table[j].dwPhysAddrLen == 6) && 
                    (strcmp(ipstr, ce->gateway) == 0)   ) {
                  ce->gwmacaddr = malloc(32);
                  snprintf(ce->gwmacaddr, 32-1, "%02X:%02X:%02X:%02X:%02X:%02X",
                           pmib->table[j].bPhysAddr[0],pmib->table[j].bPhysAddr[1],
                           pmib->table[j].bPhysAddr[2],pmib->table[j].bPhysAddr[3],
                           pmib->table[j].bPhysAddr[4],pmib->table[j].bPhysAddr[5]);
                  ldebug ("Found ARP entry for gateway %s with hwaddr %s",
                          ce->gateway, ce->gwmacaddr);
                }
              }
            }
            if (ce->isdhcp) {
              len = sizeof (name_data);
              status = RegQueryValueEx(tkey,
                                       "DhcpServer",
                                       NULL,
                                       &name_type,
                                       name_data,
                                       &len);
              if (status == ERROR_SUCCESS) {
                ce->dhcpsvr = strdup(name_data);
                ldebug ("Connection %s dhcp server: %s.", ce->name, ce->dhcpsvr);
              }
              RegCloseKey (tkey);
              status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    TCPIP_PARM_KEY,
                                    0,
                                    KEY_READ,   
                                    &tkey);
              if (status == ERROR_SUCCESS) {
                len = sizeof (name_data);
                status = RegQueryValueEx(tkey,
                                         "Hostname",
                                         NULL,
                                         &name_type,
                                         name_data,
                                         &len);
                if (status == ERROR_SUCCESS) {
                  ce->dhcpname = strdup(name_data);
                  ldebug ("Connection %s using dhcp hostname: %s", ce->name, ce->dhcpname);
                }
                RegCloseKey (tkey);
              }
              /* Set ARP info for DHCP server if needed. */
              if (pmib && ce->isdefgw) {
                for (j=0; j<pmib->dwNumEntries; j++) {
                  addr.S_un.S_addr = pmib->table[j].dwAddr;
                  ipstr = inet_ntoa(addr);
                  if ((pmib->table[j].dwPhysAddrLen == 6) &&
                      (strcmp(ipstr, ce->dhcpsvr) == 0)   ) {
                    ce->svrmacaddr = malloc(32);
                    snprintf(ce->svrmacaddr, 32-1, "%02X:%02X:%02X:%02X:%02X:%02X",
                             pmib->table[j].bPhysAddr[0],pmib->table[j].bPhysAddr[1],
                             pmib->table[j].bPhysAddr[2],pmib->table[j].bPhysAddr[3],
                             pmib->table[j].bPhysAddr[4],pmib->table[j].bPhysAddr[5]);
                    ldebug ("Found ARP entry for DHCP server %s with hwaddr %s",
                            ce->dhcpsvr, ce->svrmacaddr);
                  } 
                }
              }
            }
            else {
              RegCloseKey (tkey);
            }
          }
        }
      }
      RegCloseKey (ckey);
    }
  }

  RegCloseKey (key);

  if (pmib)
    free(pmib);

  if (numconn <= 0)
    return numconn;

  i = 0;
  status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        ADAPTER_KEY,
                        0,
                        KEY_READ,
                        &key);
  if (status != ERROR_SUCCESS) {
    lerror ("Failed to open key for read: %d", status);
  }
  else {
    while (1) {
      char enum_name[REG_NAME_MAX];
      char connection_string[REG_NAME_MAX];
      HKEY ckey;
      char name_data[REG_NAME_MAX];
      char cguid[REG_NAME_MAX];
      DWORD name_type;

      len = sizeof (enum_name);
      status = RegEnumKeyEx(
            key,
            i++,
            enum_name,
            &len,
            NULL,
            NULL,
            NULL,
            NULL);

      if (status == ERROR_NO_MORE_ITEMS)
        break;
      else if (status != ERROR_SUCCESS) 
        break;

      snprintf(connection_string,
             sizeof(connection_string),
             "%s\\%s",
             ADAPTER_KEY, enum_name);

      status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            connection_string,
            0,
            KEY_READ,
            &ckey);

      if (status == ERROR_SUCCESS) {
            len = sizeof (name_data);
            status = RegQueryValueEx(
                ckey,
                "DriverDesc",
                NULL,
                &name_type,
                name_data,
                &len);

            if (status != ERROR_SUCCESS || name_type != REG_SZ) {
            }
            else {
                ldebug ("-%s- %s", enum_name, name_data); 
            }

            RegCloseKey (ckey);
      }

      snprintf(connection_string,
             sizeof(connection_string),
             "%s\\%s",
             ADAPTER_KEY, enum_name);

      status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            connection_string,
            0,
            KEY_READ,
            &ckey);

      if (status == ERROR_SUCCESS) {
            len = sizeof (name_data);
            status = RegQueryValueEx(
                ckey,
                "NetCfgInstanceId",
                NULL,
                &name_type,
                name_data,
                &len);

            if (status != ERROR_SUCCESS || name_type != REG_SZ) 
                ldebug ("Failed parse of key %s\\NetCfgInstanceId , errorno: %d", connection_string, status);
            else {
                ldebug ("GUID: %s", name_data);
                strcpy (cguid, name_data);
            }

            RegCloseKey (ckey);
      }
      else {
          ldebug ("Failed read key %s , errorno: %d", connection_string, status);
      }

      snprintf(connection_string,
             sizeof(connection_string),
             "%s\\%s\\Ndi",
             ADAPTER_KEY, enum_name);

      status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            connection_string,
            0,
            KEY_READ,
            &ckey);

      if (status == ERROR_SUCCESS) {
            len = sizeof (name_data);
            status = RegQueryValueEx(
                ckey,
                "Service",
                NULL,
                &name_type,
                name_data,
                &len);

          if (status != ERROR_SUCCESS || name_type != REG_SZ)
                ldebug ("Failed parse of key %s\\Service , errorno: %d", connection_string, status);
          else {
                ldebug ("Service: %s", name_data);
                ce = *connlist;
                while (ce && strcmp(ce->guid , cguid) != 0) {
                  ce = ce->next;
                }
                if (ce) {
                  ce->driver = strdup(name_data);
                }
                if (strcmp(name_data, TOR_TAP_SVC) == 0) {
                  if (ce) 
                    ce->istortap = TRUE;
                  snprintf(connection_string,
                           sizeof(connection_string),
                           "%s\\%s\\Connection",
                           NETWORK_CONNECTIONS_KEY, cguid);
                  status = RegOpenKeyEx(
                          HKEY_LOCAL_MACHINE,
                          connection_string,
                          0,
                          KEY_WRITE,   
                          &wkey);
                  if (status == ERROR_SUCCESS) {
                    strcpy(name_data, TOR_TAP_NAME);
                    if (RegSetValueEx(wkey,
                                      name_string,
                                      0,
                                      REG_SZ,
                                      name_data,
                                      strlen(name_data)) != ERROR_SUCCESS) {
                      lerror ("Unable to update name of Tor Tap32 device interface.");
                    }
                    RegCloseKey (wkey);
                  }
                }
          }

          RegCloseKey (ckey);
        }
        else 
            ldebug ("Failed read key %s , errorno: %d", connection_string, status); 
    }

    RegCloseKey (key);
  }

  /* Before we return make sure to resolve any necessary ARP entries. */
  ce = *connlist;
  while (ce) {
    IPAddr arpsrcip = 0;
    IPAddr arpdestip = 0;
    ULONG ulmacaddr[2];
    ULONG paddrlen = 6;
    BYTE *hwaddr;
    if (ce->isdefgw) {
      if (ce->gwmacaddr == NULL) {
        arpdestip = inet_addr(ce->gateway);
        memset(ulmacaddr, 255, sizeof(ulmacaddr));
        retval = SendARP(arpdestip, arpsrcip, ulmacaddr, &paddrlen);
        if ((retval != NO_ERROR) || (paddrlen != 6)) {
          ldebug("Failed to resolve ARP for gateway address %s", ce->gateway);
        }
        else {
          hwaddr = (BYTE *)ulmacaddr;
          ce->gwmacaddr = malloc(32);
          snprintf(ce->gwmacaddr, 32-1, "%02X:%02X:%02X:%02X:%02X:%02X",
                   hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
          ldebug ("Received ARP reply for gateway %s with hwaddr %s",
                  ce->gateway, ce->gwmacaddr);
        }
      }
      if ( (ce->isdhcp) && (ce->svrmacaddr == NULL) ) {
        arpdestip = inet_addr(ce->dhcpsvr);
        memset(&ulmacaddr, 255, sizeof(ulmacaddr));
        retval = SendARP(arpdestip, arpsrcip, ulmacaddr, &paddrlen);
        if ((retval != NO_ERROR) || (paddrlen != 6)) {
          ldebug("Failed to resolve ARP for DHCP server address %s", ce->dhcpsvr);
        }
        else {
          hwaddr = (BYTE *)&ulmacaddr;
          ce->gwmacaddr = malloc(32);
          snprintf(ce->svrmacaddr, 32-1, "%02X:%02X:%02X:%02X:%02X:%02X",
                   hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
          ldebug ("Received ARP reply for DHCP server %s with hwaddr %s",
                  ce->dhcpsvr, ce->svrmacaddr);
        }
      }
    }
    ce = ce->next;
  }

  return numconn;
}

BOOL buildcmdline (struct s_rconnelem *  brif,
                   BOOL                  bundle,
                   BOOL                  usedebug,
                   BOOL                  noinit,
                   char **               cmdline)
{
  const DWORD  cmdlen = CMDMAX;
  *cmdline = malloc(cmdlen);
  const char * basecmds = "quiet loglevel=0 clocksource=hpet";
  const char * dbgcmds  = "loglevel=9 clocksource=hpet DEBUGINIT";

  /* Give the VM our hostname, since it is assuming the host's place in the network. */
  char * myhostname = getenv("COMPUTERNAME");
  if (!myhostname)
    myhostname = getenv("HOSTNAME");

  /* control port password is "password"
   * TODO: use Crypto API to collect entropy for ephemeral password generation
   */
  char * ctlpass = "16:6407E39581A121B26051A360CA8BB1535C73877C894E7B6EC554422789";

  if (noinit) {
    snprintf (*cmdline, cmdlen -1,
              "%s NOINIT",
              basecmds);
  }
  else {
    if (brif->isdhcp == FALSE) {
      snprintf (*cmdline, cmdlen -1,
                "%s %s%s %s IP=%s MASK=%s GW=%s MAC=%s MTU=%d PRIVIP=%s CTLSOCK=%s:9051 CTLREADY=9052 HASHPW=%s %s%s%s%s",
                usedebug ? dbgcmds : basecmds,
                myhostname ? "USEHOSTNAME=" : "",
                myhostname ? myhostname : "",
                bundle ? "FOLLOWTOR=TRUE" : "",
                brif->ipaddr,
                brif->netmask,
                brif->gateway,
                brif->macaddr,
                CAP_MTU,
                TOR_TAP_VMIP,
                TOR_TAP_VMIP,
                ctlpass,
                brif->gwmacaddr ? "ARPENT1=" : "",
                brif->gwmacaddr ? brif->gwmacaddr : "",
                brif->gwmacaddr ? "-" : "",
                brif->gwmacaddr ? brif->gateway : "");
    }
    else {
      /* fallback if we can't get HOSTNAME, use DHCP client name. */
      if (!myhostname)
        myhostname = brif->dhcpname;

      snprintf (*cmdline, cmdlen -1,
                "%s %s%s %s IP=%s MASK=%s GW=%s MAC=%s MTU=%d PRIVIP=%s ISDHCP DHCPSVR=%s DHCPNAME=%s CTLSOCK=%s:9051 CTLREADY=9052 HASHPW=%s %s%s%s%s %s%s%s%s",
                usedebug ? dbgcmds : basecmds,
                myhostname ? "USEHOSTNAME=" : "",
                myhostname ? myhostname : "",
                bundle ? "FOLLOWTOR=TRUE" : "",
                brif->ipaddr,
                brif->netmask,
                brif->gateway,
                brif->macaddr,
                CAP_MTU,
                TOR_TAP_VMIP,
                brif->dhcpsvr,
                brif->dhcpname,
                TOR_TAP_VMIP,
                ctlpass,
                brif->gwmacaddr ? "ARPENT1=" : "",
                brif->gwmacaddr ? brif->gwmacaddr : "",
                brif->gwmacaddr ? "-" : "",
                brif->gwmacaddr ? brif->gateway : "",
                brif->svrmacaddr ? "ARPENT2=" : "",
                brif->svrmacaddr ? brif->svrmacaddr : "",
                brif->svrmacaddr ? "-" : "",
                brif->svrmacaddr ? brif->dhcpsvr : "");
    }
  }
  return TRUE;
}

BOOL spawnprocess (PROCESS_INFORMATION * pi,
                   const TCHAR *cmdline)
{
  STARTUPINFO si;
  SECURITY_ATTRIBUTES sattr;
  DWORD opts = 0;
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( pi, sizeof(PROCESS_INFORMATION) );
  LPTSTR dir = NULL;
  LPTSTR qemubin = NULL;

  if (!buildfpath(PATH_FQ, VMDIR_BIN, NULL, NULL, &dir)) {
    lerror ("Unable to build path for bin dir.");
    return FALSE;
  }
  if (!buildfpath(PATH_FQ, VMDIR_BIN, NULL, "qemu.exe", &qemubin)) {
    lerror ("Unable to build path for qemu program.");
    return FALSE;
  }

  TCHAR *cmd = malloc(CMDMAX);
  /* TODO: clean this up once the msys path munging works.  kernel and hdd need to be unixy paths */
  snprintf (cmd, CMDMAX -1,
            "\"%s\" -L . -no-reboot -kernel ../lib/vmlinuz -append \"loglevel=9 NOINIT\" -hda ../state/hdd.img -m %d -std-vga", qemubin, QEMU_DEF_MEM);
  ldebug ("Launching Qemu with cmd: %s", cmd);
  if( !CreateProcess(NULL,
                     cmd,
                     NULL,   // process handle no inherit
                     NULL,   // thread handle no inherit
                     TRUE,
                     opts,
                     NULL,   // environment block
                     TOR_VM_BIN,
                     &si,
                     pi) ) {
    lerror ("Failed to launch process.  Error code: %d", GetLastError());
    return FALSE;
  }
  return TRUE;
}

BOOL runvidalia (BOOL  indebug)
{
  BOOL  retval = FALSE;
  PROCESS_INFORMATION pi;
  STARTUPINFO si;
  SECURITY_ATTRIBUTES sattr;
  TCHAR * cmd = NULL;
  LPTSTR exe = NULL;
  LPTSTR dir = NULL;
  LPTSTR vcfgtmp = NULL;
  LPTSTR pcfgtmp = NULL;
  LPTSTR vcfgdest = NULL;
  LPTSTR pcfgdest = NULL;
  LPTSTR pcfgdestsave = NULL;
  DWORD opts = CREATE_NEW_PROCESS_GROUP;
  HANDLE tmphnd;
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof(pi) );
  
  if (!buildfpath(PATH_FQ, VMDIR_LIB, NULL, "defvidalia.conf", &vcfgtmp)) {
    lerror ("Unable to build path for default vidalia config file."); 
    goto cleanup;
  } 
  if (!buildfpath(PATH_FQ, VMDIR_LIB, NULL, "defpolipo.conf", &pcfgtmp)) {
    lerror ("Unable to build path for default polipo config file.");
    goto cleanup;
  } 
  if (!buildsyspath(SYSDIR_LCLDATA, "Vidalia", &dir)) {
    lerror ("Unable to build path for Vidalia programs dir."); 
    goto cleanup;
  } 
  if (!buildsyspath(SYSDIR_LCLDATA, "Vidalia\\vidalia.conf", &vcfgdest)) {
    lerror ("Unable to build path for vidalia dest config file."); 
    goto cleanup;
  } 
  if (!buildsyspath(SYSDIR_LCLDATA, "Polipo\\config.txt", &pcfgdest)) {
    lerror ("Unable to build path for polipo dest config."); 
    goto cleanup;
  } 
  if (!buildsyspath(SYSDIR_LCLDATA, "Polipo\\save-cfg.txt", &pcfgdestsave)) {
    lerror ("Unable to build path for polipo saved dest config.");
    goto cleanup;
  }
  if (!buildsyspath(SYSDIR_LCLPROGRAMS, "Vidalia\\vidalia-marble.exe", &exe)) {
    lerror ("Unable to build path for vidalia marble exe."); 
    goto cleanup;
  } 
  if (!exists(exe)) {
    /* assume not a marble vidalia install */
    free (exe);
    if (!buildsyspath(SYSDIR_LCLPROGRAMS, "Vidalia\\vidalia.exe", &exe)) {
      lerror ("Unable to build path for vidalia exe."); 
      goto cleanup;
    } 
  }

  /* for now we always force a correct vidalia config to temporarily resolve
   * flyspray 945
   */
  ldebug ("Copying default vidalia config from %s to %s", vcfgtmp, vcfgdest);
  copyvidaliacfg(vcfgtmp, vcfgdest, dir);

  /* same for polipo and its backup file; see flyspray 946.
   */
  ldebug ("Copying default polipo config from %s to %s", pcfgtmp, pcfgdest);
  copyfile(pcfgtmp, pcfgdest);
  cmd = malloc(CMDMAX);
  snprintf (cmd, CMDMAX -1,
            "\"%s\" -tor-address %s %s",
            exe,
            TOR_TAP_VMIP,
            indebug ? " -loglevel debug -logfile debuglog.txt" :
                      " -loglevel info -logfile infolog.txt");
  ldebug ("Launching Vidalia in dir: %s , with cmd: %s", dir, cmd);
  if( !CreateProcess(NULL,
                     cmd,
                     NULL,   // process handle no inherit
                     NULL,   // thread handle no inherit
                     TRUE,   
                     opts,
                     NULL,   // environment block
                     dir,
                     &si,
                     &pi) ) {
    lerror ("Failed to launch process.  Error code: %d", GetLastError());
    goto cleanup;
  }
  else {
    retval = TRUE;
  }

 cleanup:
  if(cmd)
    free(cmd);
  if(exe)
    free(exe);
  if(dir)
    free(dir);
  if(vcfgtmp)
    free(vcfgtmp);
  if(pcfgtmp)
    free(pcfgtmp);
  if(vcfgdest)
    free(vcfgdest);
  if(pcfgdest)
    free(pcfgdest);
  if(pcfgdestsave)
    free(pcfgdestsave);

  return retval;
}

BOOL launchtorvm (PROCESS_INFORMATION * pi,
                  char *  bridgeintf,
                  char *  macaddr,
                  char *  tapname,
                  char *  cmdline)
{
  STARTUPINFO si;
  HANDLE stdin_rd = NULL;
  HANDLE stdin_wr = NULL;
  HANDLE stdout_h = NULL;
  SECURITY_ATTRIBUTES sattr;
  LPTSTR cmd = NULL;
  LPTSTR dir = NULL;
  /* If Tor VM Qemu instance is not below normal prio, performance of host suffers. */
  DWORD opts = CREATE_NEW_PROCESS_GROUP | BELOW_NORMAL_PRIORITY_CLASS;
  DWORD numwritten;
  DWORD pipesz;
  LPTSTR qemubin = NULL;

  if (!buildfpath(PATH_FQ, VMDIR_BIN, NULL, NULL, &dir)) {
    lerror ("Unable to build path for bin dir.");
    return FALSE;
  }
  if (!buildfpath(PATH_FQ, VMDIR_BIN, NULL, "qemu.exe", &qemubin)) {
    lerror ("Unable to build path for qemu program.");
    return FALSE;
  }

  ZeroMemory( &si, sizeof(si) );
  ZeroMemory( &sattr, sizeof(sattr) );
  ZeroMemory( pi, sizeof(PROCESS_INFORMATION) );
  si.cb = sizeof(si);
/*  sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
  sattr.bInheritHandle = TRUE;
  sattr.lpSecurityDescriptor = NULL; */
  cmd = malloc(CMDMAX);
  if (tapname) {
    snprintf (cmd, CMDMAX -1,
              "\"%s\" -name \"Tor VM \" -L . -no-reboot -kernel ../lib/vmlinuz -append \"%s\" -hda ../state/hdd.img -m %d -std-vga -net nic,model=pcnet,macaddr=%s -net pcap,devicename=\"%s\" -net nic,vlan=0,model=pcnet -net tap,vlan=0,ifname=\"%s\"",
	      qemubin,
              cmdline,
              QEMU_DEF_MEM,
              macaddr,
              bridgeintf,
              tapname);
  }
  else {
    snprintf (cmd, CMDMAX -1,
              "\"%s\" -name \"Tor VM \" -L . -no-reboot -kernel ../lib/vmlinuz -append \"%s\" -hda ../state/hdd.img -m %d -std-vga -net nic,model=pcnet,macaddr=%s -net pcap,devicename=\"%s\"",
	      qemubin,
              cmdline,
              QEMU_DEF_MEM,
              macaddr,
              bridgeintf);
  }
  ldebug ("Launching Qemu with cmd: %s", cmd);

/*
  pipesz = strlen(cmdline);
  CreatePipe(&stdin_rd, &stdin_wr, &sattr, pipesz);
  SetHandleInformation(stdin_wr, HANDLE_FLAG_INHERIT, 0);

  if (! WriteFile(stdin_wr, cmdline, strlen(cmdline), &numwritten, NULL)) {
    lerror ("Failed to write kernel command line to stdin handle.  Error code: %d", GetLastError());
  }
  else {
    ldebug ("Wrote %d bytes of cmdline len %d to qemu stdin pipe.", numwritten, strlen(cmdline));
  }

  stdout_h = GetStdHandle(STD_OUTPUT_HANDLE);

  si.hStdError = stdout_h;
  si.hStdOutput = stdout_h;
  si.hStdInput = stdin_rd;
  si.dwFlags |= STARTF_USESTDHANDLES;
*/
  if( !CreateProcess(NULL,
                     cmd,
                     NULL,
                     NULL,
                     TRUE,
                     opts,
                     NULL,
                     dir,
                     &si,
                     pi) ) {
    lerror ("Failed to launch Qemu Tor VM process.  Error code: %d", GetLastError());
    return FALSE;
  }
/*
  FlushFileBuffers (stdin_wr);
  CloseHandle(stdin_rd);
  CloseHandle(stdin_wr);
*/
  return TRUE;
}

BOOL isrunning (PROCESS_INFORMATION * pi) {
  DWORD exitcode;
  if (GetExitCodeProcess(pi->hProcess, &exitcode) && (exitcode == STILL_ACTIVE) ) {
    return TRUE;
  }
  return FALSE;
}

BOOL waitforit (PROCESS_INFORMATION * pi) {
  DWORD exitcode;
  ldebug ("Waiting for process to exit ...");
  while ( GetExitCodeProcess(pi->hProcess, &exitcode) && (exitcode == STILL_ACTIVE) ) {
    Sleep (1000);
  }
  ldebug ("Done waiting.");
  CloseHandle(pi->hThread);
  CloseHandle(pi->hProcess);

  return TRUE;
}

BOOL promptrunasadmin (void)
{
  int  retval;
  if ( MessageBox(NULL,
                  "Tor VM requires Administrator rights.  Attempt run as Administrator?",
                  "Elevated Privileges Needed",
                  MB_OKCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL | MB_SETFOREGROUND) == IDOK) {
    return TRUE;
  }
  return FALSE;
}

BOOL respawnasadmin (void)
{
  STARTUPINFOW si = {0};
  PROCESS_INFORMATION pi = {0};
  LPTSTR cmd = NULL;
  LPWSTR wcmd = NULL;
  LPCWSTR username = L"Administrator";
  LPCWSTR password = L"";
  DWORD exitcode;
  DWORD authopts = 0;
  DWORD propts = 0;

  propts = CREATE_NEW_PROCESS_GROUP | HIGH_PRIORITY_CLASS;

  si.cb = sizeof(si);
  /* TODO: also need to fix unicode / wide char issues. */
  wcmd = W_TOR_VM_BASE L"\\torvm.exe";
  getmypath(&cmd);

  /* first, let's see if Administrator has no password set. */
  if( !CreateProcessWithLogonW(username,
                     NULL,   // default domain
                     password,
                     authopts,
                     NULL,   // no process name
                     wcmd,
                     propts,
                     NULL,   // environment block
                     NULL,   // keep same directory
                     &si,
                     &pi) ) {
    lerror ("Failed to re-launch with Administrator rights. Unable to continue.");
  }
  return TRUE;
}

BOOL runningdetached (void)
{
  return TRUE;
}

BOOL detachself (void)
{
  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi = {0};
  LPTSTR cmd = NULL;
  LPTSTR mypath = NULL;
  LPTSTR args = "";
  bgstartupinfo (&si);
  getmypath(&mypath);
  cmd = malloc (CMDMAX);
  snprintf (cmd, CMDMAX -1,
            "\"%s\" %s",
            mypath, args);
  if( !CreateProcess(NULL,
                     cmd,
                     NULL,
                     NULL,
                     FALSE,
                     CREATE_NEW_PROCESS_GROUP | HIGH_PRIORITY_CLASS,
                     NULL,
                     NULL,
                     &si,
                     &pi) ) {
    lerror ("Failed to launch detached torvm.exe process.  Error code: %d", GetLastError());
    free (cmd);
    return FALSE;
  }
  free (cmd);
  return TRUE;
}

BOOL setupuser (LPTSTR username,
                LPTSTR ctlip,
                LPTSTR ctlport)
{
  BOOL retval = FALSE;
  userinfo * ui;
  char * myhostname = getenv("COMPUTERNAME");
  if (!myhostname)
    myhostname = getenv("HOSTNAME");
  if (createruser (myhostname,
                   username,
                   &ui)) {
    if (!initruserprofile(ui)) {
      ldebug ("Failed to initialize user profile data in setupuser.");
    }
    else {
      if (!setupruserfollow(ui, ctlip, ctlport)) {
        ldebug ("Failed to setup Tor follow startup script for user %s.", username);
      }
      else {
        ldebug ("All setup completed for restricted user %s.", username);
        retval = TRUE;
      }
    }
  }
  return retval;
}

/* XXX: This is a temporary method to clean out the usual culprits.
 * Note that there are many other places to store data, particularly the registry.
 */
BOOL cleanruserfiles (LPTSTR username)
{
  LPTSTR dirpath;
  LPTSTR auppath;
  LPTSTR coff;
  if (!buildsyspath(SYSDIR_ALLPROFILE, NULL, &auppath)) {
    lerror ("Unable to build path for all users profile destination.");
    return FALSE;
  }
  /* Trim off the "All Users" part as we just want Documents and Settings
   * XXX: all of the path handling needs to be cleaned up, localized, collected.
   */
  coff = auppath + strlen(auppath) - 1;
  while ( (coff > auppath) && (*coff != '\\') ) coff--;
  if (coff > auppath)
    *coff = 0;
  dirpath = malloc(CMDMAX);
  snprintf(dirpath, CMDMAX -1, "%s\\%s\\Local Settings\\Temporary Internet Files", auppath, username);
  rmdirtree(dirpath);
  snprintf(dirpath, CMDMAX -1, "%s\\%s\\Local Settings\\Temp", auppath, username);
  rmdirtree(dirpath);
  snprintf(dirpath, CMDMAX -1, "%s\\%s\\Local Settings\\SendTo", auppath, username);
  rmdirtree(dirpath);
  snprintf(dirpath, CMDMAX -1, "%s\\%s\\Local Settings\\Cookies", auppath, username);
  rmdirtree(dirpath);
  snprintf(dirpath, CMDMAX -1, "%s\\%s\\Local Settings\\History", auppath, username);
  rmdirtree(dirpath);

  free(auppath);
  free(dirpath);  
  return TRUE;
}

BOOL setupenv (void)
{
#define EBUFSZ 4096
#define PATHVAR  TEXT("PATH")
  DWORD   retval;
  DWORD   errnum;
  DWORD   buflen;
  LPTSTR  envvar;
  LPTSTR  newvar;
  DWORD   envopts=0;
  BOOL    exists = FALSE;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  LPTSTR  libpath;
  LPTSTR  binpath;
 
  envvar = malloc(EBUFSZ * sizeof(TCHAR));
  if(envvar == NULL) {
    lerror ("setupenv: out of memory.");
    return FALSE;
  }

  if (!buildfpath(PATH_FQ, VMDIR_LIB, NULL, NULL, &libpath)) {
    lerror ("Unable to build vm lib path");
    return FALSE;
  }
  if (!buildfpath(PATH_FQ, VMDIR_BIN, NULL, NULL, &binpath)) {
    lerror ("Unable to build vm bin path");
    return FALSE;
  }

  retval = GetEnvironmentVariable(PATHVAR, envvar, (EBUFSZ -1));
  if(retval == 0) {
    errnum = GetLastError();
    if( errnum == ERROR_ENVVAR_NOT_FOUND ) {
      exists = FALSE;
    }
  }
  else if (retval >= EBUFSZ) {
    envvar = (LPTSTR) realloc(envvar, retval*sizeof(TCHAR));   
    if(envvar == NULL) {
      lerror ("setupenv: out of memory.");
      return FALSE;
    }
    retval = GetEnvironmentVariable(PATHVAR, envvar, retval);
    if(!retval) {
      lerror ("setupenv: GetEnvironmentVariable failed with errornum: %d",
              GetLastError());
      return FALSE;
    }
    else exists = TRUE;
  }
  else exists = TRUE;

  retval = (exists) ? strlen(envvar) : 0;
  retval +=  EBUFSZ;
  newvar = malloc(retval * sizeof(TCHAR));
  if (newvar == NULL) {
    lerror ("setupenv: out of memory.");
    return FALSE;
  }
  if (exists) {
    strcat (envvar, ";");
  }
  else {
    *envvar = (TCHAR)0;
  }
  snprintf (newvar, retval -1, "%s;%s;%s", libpath, binpath, envvar);

  if (! SetEnvironmentVariable(PATHVAR, newvar)) {
    lerror ("setupenv: SetEnvironmentVariable failed with errornum: %d for new val: %s",
            GetLastError(),
            newvar);
    return FALSE;
  }
  ldebug ("setupenv: %s => %s",
          PATHVAR,
          newvar);

  return TRUE; 
}

static struct option torvm_options[] =
{
  /* opt name,
   * no_argument | required_argument | optional_argument,
   * int* flag | NULL,
   * 'x' (char)  OR  flag && lval
   */
  { "accel" , no_argument , NULL, 'a' },
  { "verbose" , no_argument , NULL, 'v' },
  { "update" , no_argument , NULL, 'u' },
  { "bundle" , no_argument , NULL, 'b' },
  { "service" , no_argument , NULL, 's' },
  { "replace" , no_argument , NULL, 'r' },
  { "clean" , no_argument , NULL, 'c' },
  { "vmnop" , no_argument , NULL, 'X' },
  { "noinit" , no_argument , NULL, 'Z' },
  { "help" , no_argument , NULL, 'h' },
  { "follow" , no_argument , NULL, 'F' },
  { "ctlip" , required_argument, NULL, 'I' },
  { "ctlport" , required_argument, NULL, 'P' },
  {0}
};

void usage(void)
{
  fprintf(stderr, "Usage:\t"
    "torvm.exe [options]\n\n"
    "Valid options are:\n"
    "  --accel\n"
    "  --verbose\n"
    "  --update\n"
    "  --bundle\n"
    "  --service\n"
    "  --replace\n"
    "  --clean\n"
    "  --vmnop\n"
    "  --noinit\n"
    "  --help\n");
  exit (1);
}

int main(int argc, char **argv)
{
  const char *cmd;
  int numintf;
  struct s_rconnelem *connlist = NULL;
  struct s_rconnelem *ce = NULL;
  struct s_rconnelem *tapconn = NULL;
  BOOL vmaccel = FALSE;
  BOOL bundle = FALSE;
  BOOL follow = FALSE;
  BOOL clean = FALSE;
  BOOL indebug = FALSE;
  BOOL vmnop = FALSE;
  BOOL noinit = FALSE;
  BOOL foundit = FALSE;
  char *cmdline = NULL;
  LPTSTR logfile = NULL;
  LPTSTR ctliparg = NULL;
  LPTSTR ctlportarg = NULL;
  LPTSTR polipodir;
  DWORD taptimeout = 60; /* the tap device can't be configured until the VM connects it */
  int c, optidx = 0;

  setupthrctx();

  while (1) {
    c = getopt_long(argc, argv, "avubshrcXZ", torvm_options, &optidx);
    if (c == -1)
      break;

    switch (c) {
        case 'a':
          ldebug ("Set option %s.", torvm_options[optidx].name);
          vmaccel = TRUE;
          break;

        case 'v':
          ldebug ("Set option %s.", torvm_options[optidx].name);
          indebug = TRUE;
          break;

        case 'b':
          ldebug ("Set option %s.", torvm_options[optidx].name);
          bundle = TRUE;
          break;

        case 's':
          ldebug ("Set option %s.", torvm_options[optidx].name);
          break;

        case 'r':
          ldebug ("Set option %s.", torvm_options[optidx].name);
          LPTSTR fname = NULL;
          if (!buildfpath(PATH_RELATIVE, VMDIR_STATE, NULL, TOR_HDD_FILE, &fname)) {
            lerror ("Unable to build path for dest %s", TOR_HDD_FILE);
          }
          else {
            DeleteFile (fname);
	    free(fname);
            linfo ("Removed existing virtual disk image for replacement to original state.");
          }
          break;

        case 'c':
          ldebug ("Set option %s.", torvm_options[optidx].name);
          clean = TRUE;
          break;

        case 'X':
          ldebug ("Set option %s.", torvm_options[optidx].name);
          indebug = TRUE;
          vmnop = TRUE;
          break;

        case 'Z':
          ldebug ("Set option %s.", torvm_options[optidx].name);
          indebug = TRUE;
          noinit = TRUE;
          break;

        case 'F':
          follow = TRUE;
          break;

        case 'I':
          if (optarg)
            ctliparg = optarg;
          else
            ctliparg = TOR_TAP_VMIP;
          break;

        case 'P':
          if (optarg)
            ctlportarg = optarg;
          else
            ctlportarg = "9051";
          break;

        case 'h':
          linfo ("Help for command usage invoked.");
          usage();
          break;

        case 0:  /* not used for flags currently. */
          break;
      default:
        usage();
        break;
    }
  }
 
  /* The Tor follow mode is a special case. All we do is loop until the control
   * port is no longer accepting connections and then we issue a logoff request.
   */ 
  if (follow) {
    while(follow) {
      Sleep(1000);
      if (!tryconnect(ctliparg, atol(ctlportarg))) {
        Sleep(1000);
        /* XXX: Increase prio if intermittent connect timeouts? */
        if (!tryconnect(ctliparg, atol(ctlportarg))) {
          follow = FALSE;
        }
      }
    }
    /* At this point Tor in the Admin user desktop inside the VM has failed or exited.
     * This is our cue to force the restricted user to log off.
     */
    runcommand("shutdown -l -f", NULL);
    return 0;
  }

  if (buildfpath(PATH_FQ, VMDIR_STATE, NULL, "vmlog.txt", &logfile)) {
    logto (logfile);
    free (logfile);
    logfile = NULL;
  }
  if (buildfpath(PATH_FQ, VMDIR_STATE, NULL, "debug.txt", &logfile)) {
    debugto (logfile);
    free (logfile);
    logfile = NULL;
  }

  if (getosbits() > 32) {
    lerror ("Error: only 32bit operating systems are currently supported.");
    MessageBox(NULL,
               "Sorry, only 32bit operating systems are currently supported.",
               "Unsupported Operating System Architecture",
               MB_OK);
    exit (1);
  }

  if (!haveadminrights()) {
    if (promptrunasadmin()) {
      if (respawnasadmin() == TRUE) {
        return 0;
      }
    }
    return 1;
  }

  if (!setupenv()) {
    fatal ("Unable to prepare process environment.");
  }

  if (clean)
    goto shutdown;

  dispmsg("Tor VM is starting. Please be patient.");
  if (!vmnop) {
    if (!savenetconfig()) {
      fatal ("Unable to save current network configuration.");
    }

    if (bundle) {
      /* XXX: note we're using the "all ready" alias for the control port. */
      if (!setupuser(TOR_RESTRICTED_USER, TOR_TAP_VMIP, "9052")) {
        lerror ("Unable to setup restricted user.");
      }
    }

    ce = NULL;
    numintf = loadnetinfo(&connlist);
    if (numintf > 0) {
      ce = connlist;
      while (ce && ce->istortap != TRUE) {
        ce = ce->next;
      }
    }

    /* disable removing the tap automatically until reload issues resolved.
     * uninstalltap(); */
    if (ce == NULL) {
      if (!setdriversigning (FALSE)) {
        lerror ("Unable to disable driver signing checks. Installing tap anyway...");
      }
      if (!installtap()) {
        lerror ("Unable to load TAP-Win32 network driver.");
        goto shutdown;
      }
      if (!setdriversigning (TRUE)) {
        lerror ("Unable to restore driver signing checks.");
      }
      /* TODO: util method to free structure of list elem */
      numintf = loadnetinfo(&connlist);
    }
    tapconn = connlist;
    while (tapconn && tapconn->istortap != TRUE) {
      tapconn = tapconn->next;
    }

    dispmsg(" - Configuring network settings");
    if (!installtornpf()) {
      lerror ("Unable to install Tor NPF service driver.");
      goto shutdown;
    }

    if (! configbridge()) {
      lerror ("Unable to configure blackhole route for bridged interface.");
    }
    if (! disableservices()) {
      lerror ("Unable to disable dangerous windows network services.");
    }
    if (getosversion() > OS_2000) {
      if (! disablefirewall()) {
        lerror ("Unable to disable windows firewall.");
      }
    }
    if (! cleararpcache()) {
      lerror ("Unable to clear arp cache.");
    }
    if (! flushdns()) {
      lerror ("Unable to flush cached DNS entries.");
    }
  }

  /* all invocations past this point need a virtual disk at minimum */
  if (! checkvirtdisk()) {
    lerror ("Unable to confirm usable virtual disk is present.");
  }

  if (!vmnop) {
    if (numintf <= 0) {
      lerror ("Unable to find any usable network interfaces.");
      goto shutdown;
    }

    ce = connlist;
    while (!foundit && ce) {
      if (ce->isdefgw) {
        foundit = TRUE;
      }
      else {
        ce = ce->next;
      }
    }
    if (ce == NULL) {
      lerror ("Unable to find network interface with a default route.");
      goto shutdown;
    }
  }

  if (!vmnop) {
    if (! buildcmdline(ce, bundle, indebug, noinit, &cmdline)) {
      lerror ("Unable to generate command line for kernel.");
      goto shutdown;
    }
    ldebug ("Generated kernel command line: %s", cmdline);
  }

  dispmsg(" - Launching QEMU virtual machine");
  PROCESS_INFORMATION pi;
  if (vmnop) {
    if (! spawnprocess(&pi, "qemu.exe")) {
      lerror ("Unable to launch default Qemu instance.");
    }
    /* This mode does nothing but run Qemu with the kernel and virtual disk.
     * no need for cleanup or interface configuration.
     */
    exit (0);
  }
  if (! launchtorvm(&pi,
                    ce->guid,
                    ce->macaddr,
                    TOR_TAP_NAME,
                    cmdline)) {
    lerror ("Unable to launch Qemu TorVM instance.");
    goto shutdown;
  }

  /* need to delay long enough to allow qemu to start and open tap device */
  if (tapconn) {
    while ( taptimeout-- && isrunning(&pi) && (! isconnected(tapconn->guid)) ) {
      ldebug ("Waiting for tap adapter to be connected...");
      Sleep (1000);
    }
  }
  ldebug ("Done waiting.");

  if (! isrunning(&pi)) {
    lerror ("Virtual machine failed to start properly.");
    goto shutdown;
  }
  if (! isconnected(tapconn->guid)) {
    lerror ("Network tap device is not connected to VM.");
    goto shutdown;
  }
  if (! configtap()) {
    lerror ("Unable to configure tap device.");
    goto shutdown;
  }

  /* XXX: temp hack - in bundle mode launch Vidalia with a custom config
   * for the 10.x tap control port and externally managed Tor instance.
   * The control port is used to signal both Tor starting correctly, and
   * once Tor is stopped the no longer listening control port signals
   * restricted user log off and clean shutdown.
   */
  if (bundle) {
    dispmsg(" - Waiting for Tor control port to open");
    /* try to confirm control port is up before launching vidalia... */
    int i = 10;
    while ( (!tryconnect(TOR_TAP_VMIP, 9051)) && (i > 0) ) {
      ldebug("Control port connect attempt failed, trying again... [%d left]", i);
      Sleep(1000);
      if (!isrunning(&pi)) 
        i = 0;
      else
        i--;
    }
    if (i > 0) {
      ldebug("Control port connected. Starting controller ...");

      /* XXX: Why does vidalia have trouble immediately after start?
       * May need a few seconds for Tor in the VM to get up to speed
       * even though control socket is accepting in event loop.
       */
      Sleep(2000);
      dispmsg(" - Launching Vidalia");
      runvidalia(indebug);
      /* XXX: Next step to launch polipo and vidalia separately, then handle restart/kill as needed.
       * buildsyspath(SYSDIR_LCLPROGRAMS, "Polipo", &polipdir);
       * runcommand("polipo.exe -c polipo.conf", polipodir);
       */

      /* XXX: Now we wait for the ALL READY socket to be listening before switching.
       * If we don't get bootstrapped within this period of time something is broken/blocked.
       */
      ldebug("Waiting for Tor to bootstrap ...");
      dispmsg(" - Waiting for Tor to establish a circuit");
      i = 60 * 5; 
      while ( (!tryconnect(TOR_TAP_VMIP, 9052)) && (i > 0) ) {
        Sleep(1000);
        ldebug("Tor has not bootstrapped yet, checking again... [%d left]", i);
        if (!isrunning(&pi)) 
          i = 0;
        else
          i--;
      }
      if (i > 0) {
        /* Once/if bootstrapped allow the user to run applications with restricted privs. */
        userswitcher();
      }
    }
  }

  if (isrunning(&pi)) {
    dispmsg("");
    dispmsg("GOOD! Tor VM is running.");
    dispmsg(" - Waiting for VM to exit ...");
    if (bundle)
      dispmsg(" NOTE: Select the \"Exit\" option in Vidalia to shutdown.");
    else
      dispmsg(" NOTE: Close the \"QEMU (Tor VM)\" window to shutdown.");
    dispmsg("");
    /* TODO: once the pcap bridge is up we can re-enable the firewall IF we
     * add an exception for the control port on the Tap adapter.
     */
    waitforit(&pi);
    linfo ("Tor VM closed, restoring host network and services.");
  }
  else {
    lerror ("Virtual machine failed to start properly.");
    linfo ("Tor VM Qemu failed to start properly.");
  }
  dispmsg("Shutting down.");
  dispmsg("CAUTION: Restoring network settings. Do NOT close this window!");

 shutdown:
  if (bundle) {
    disableuser(TOR_RESTRICTED_USER);
    /* cleanruserfiles(TOR_RESTRICTED_USER); */
  }
  if (getosversion() > OS_2000) {
    if (! enablefirewall()) {
      lerror ("Unable to re-enable windows firewall.");
    }
  }
 /* TODO: leave for now, perhaps as default unless running from removable media?
  if (! uninstalltap()) {
    lerror ("Unable to remove TAP-Win32 device.");
  }
 */
  if (! uninstalltornpf()) {
    lerror ("Unable to remove Tor NPF service driver.");
  }
  if (! cleararpcache()) {
    lerror ("Unable to clear arp cache.");
  }
  if (! flushdns()) {
    lerror ("Unable to flush cached DNS entries.");
  }
  if (! restorenetconfig()) {
    lerror ("Unable to restore network configuration.");
  }
  linfo ("Tor VM shutdown completed.");
  return 0;
}

