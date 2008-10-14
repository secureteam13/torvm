/* Copyright (C) 2008  The Tor Project, Inc.
 * See LICENSE file for rights and terms.
 */
#include "torvm.h"

#define TOR_VM_BASE    "C:\\Tor_VM"
#define W_TOR_VM_BASE  L"C:\\Tor_VM"
#define TOR_VM_BIN     TOR_VM_BASE "\\bin"
#define TOR_VM_LIB     TOR_VM_BASE "\\lib"
#define TOR_VM_STATE   TOR_VM_BASE "\\state"
#define WIN_DRV_DIR    "C:\\WINDOWS\\system32\\drivers"
#define TOR_TAP_NAME   "Tor VM Tap32"
#define TOR_TAP_SVC    "tortap91"

BOOL buildpath (const TCHAR *dirname,
                TCHAR **fullpath);
BOOL buildwsyspath (TCHAR **fullpath);


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
  LPTSTR  dhcpsvr;
  LPTSTR  driver;
  struct s_rconnelem * next;
};

/* logging:
 *   lerror to stderr and log file(s) if set
 *   linfo  to log and debug file
 *   ldebug to debug file
 *   fatal logs error and then exits process
 */
static HANDLE  s_logh = INVALID_HANDLE_VALUE;
static HANDLE  s_dbgh = INVALID_HANDLE_VALUE;


void logto (LPTSTR  path)
{
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
  static const int  msgmax = 4096;
  static char *     msgbuf = NULL;
  static char *     coff = NULL;
  const char *      newline = "\r\n";
  int               len;
  DWORD             written;
  SYSTEMTIME        now;
  va_list           ap;

  if ( (fda == INVALID_HANDLE_VALUE) &&
       (fdb == INVALID_HANDLE_VALUE) &&
       (fdc == INVALID_HANDLE_VALUE)   )
    return;

  if (msgbuf == NULL) {
    msgbuf = (char *) malloc (msgmax);
    if (!msgbuf) return;
  }
  GetSystemTime (&now);
  coff = msgbuf;
  coff[msgmax -1] = 0;
  len = snprintf (coff,
                  msgmax -1,
                  "[%4.4d/%-2.2d/%-2.2d %-2.2d:%-2.2d:%-2.2d.%-3.3d UTC] %s: ",
                  now.wYear,
                  now.wMonth,
                  now.wDay,
                  now.wHour,
                  now.wMinute,
                  now.wSecond,
                  now.wMilliseconds,
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
}


void ldebug (const char* format, ...)
{
  va_list  argptr;
  va_start (argptr, format);
  _flog (s_dbgh, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, "debug", format, argptr);
  va_end (argptr);
  return;
}


BOOL installtap(void)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  LPTSTR cmd = NULL;
  LPTSTR dir = NULL;
  DWORD exitcode;
  DWORD opts = 0;

  opts = CREATE_NEW_PROCESS_GROUP;

  ZeroMemory( &pi, sizeof(pi) );
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  dir = TOR_VM_LIB;
  cmd = "\"" TOR_VM_BIN "\\devcon.exe\" install tortap91.inf TORTAP91";
  ldebug ("Tap install cmd: %s", cmd);

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

  linfo ("waiting for TAP-Win32 driver install to complete ...");
  while ( GetExitCodeProcess(pi.hProcess, &exitcode) && (exitcode == STILL_ACTIVE) ) {
    Sleep (500);
  }
  linfo ("TAP-Win32 install exited with value %d", exitcode);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  return TRUE;  
}

BOOL uninstalltap(void)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  LPTSTR cmd = NULL;
  LPTSTR dir = NULL;
  DWORD exitcode;
  DWORD opts = 0;
  LONG status;
  HKEY key;
  DWORD len;
  int i = 0;
  int stop = 0;
  int numconn = 0;
  const char name_string[] = "Name";
  char svc_string[REG_NAME_MAX];
  
  opts = CREATE_NEW_PROCESS_GROUP;
  
  ZeroMemory( &pi, sizeof(pi) );
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  dir = TOR_VM_LIB;
  cmd = "\"" TOR_VM_BIN "\\devcon.exe\" remove TORTAP91";
  ldebug ("Tap un-install cmd: %s", cmd);
 
  ldebug ("Removing TORTAP91 device via devcon."); 
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
    return FALSE;
  }
  
  while ( GetExitCodeProcess(pi.hProcess, &exitcode) && (exitcode == STILL_ACTIVE) ) {
    Sleep (200);
  }
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

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
  LPTSTR srcname = TOR_VM_LIB "\\tornpf.sys";
  LPTSTR destname = WIN_DRV_DIR "\\tornpf.sys";
  CHAR * buff = NULL;
  DWORD  buffsz = 4096;
  DWORD  len;
  DWORD  written;
  
  src = CreateFile (srcname,
                    GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
  if (src == INVALID_HANDLE_VALUE) {
    return FALSE;
  } 
  dest = CreateFile (destname,
                     GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_SYSTEM,
                     NULL);
  if (dest == INVALID_HANDLE_VALUE) {
    return FALSE;
  } 
  
  buff = (CHAR *)malloc(buffsz);
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
  LPTSTR fname = WIN_DRV_DIR "\\tornpf.sys";
  LPTSTR cmd = "\"net.exe\" stop tornpf";
  DeleteFile (fname);
  if (! runcommand(cmd)) {
    return FALSE;
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
    runcommand(cmd);
    linfo ("Saved current firewall configuration state.");
  }

  fh = CreateFile (TOR_VM_STATE "\\netcfg.save",
                   GENERIC_WRITE,
                   0,
                   NULL,
                   CREATE_ALWAYS,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL);
  if (fh == INVALID_HANDLE_VALUE) {
    lerror ("Unable to open network save file for writing. Error code: %d", GetLastError());
    return FALSE;
  }

  ZeroMemory( &pi, sizeof(pi) );
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
  sattr.bInheritHandle = TRUE;
  sattr.lpSecurityDescriptor = NULL;
  dir = TOR_VM_STATE;
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
                     FALSE,  // default handle inheritance false
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

  buff = (CHAR *)malloc(READSIZE);
  while (ReadFile(stdout_rd, buff, READSIZE, &numread, NULL) && (numread > 0)) {
    WriteFile(fh, buff, numread, &numwritten, NULL);
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
  DWORD exitcode;
  DWORD opts = CREATE_NEW_PROCESS_GROUP;

  if (getosversion() >= OS_VISTA) {
    cmd = "\"netsh.exe\" advfirewall import \"" TOR_VM_STATE "\\firewall.wfw\"";
    runcommand(cmd);
    linfo ("Imported saved firewall configuration.");
  }

  ZeroMemory( &pi, sizeof(pi) );
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  dir = TOR_VM_STATE;
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
    Sleep (200);
  }

  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  LPTSTR fname = TOR_VM_STATE "\\netcfg.save";
  DeleteFile (fname);
  linfo ("Restored current network configuration state.");

  return TRUE;  
}

BOOL runcommand(LPSTR cmd)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  LPTSTR dir = NULL;
  DWORD exitcode;
  DWORD opts = 0;

  opts = CREATE_NEW_PROCESS_GROUP;

  ZeroMemory( &pi, sizeof(pi) );
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  dir = TOR_VM_BIN;

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
    return FALSE;
  }

  while ( GetExitCodeProcess(pi.hProcess, &exitcode) && (exitcode == STILL_ACTIVE) ) {
    Sleep (200);
  }
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

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
  if (! runcommand(cmd)) {
    return FALSE;
  }
  return TRUE;
}

BOOL enablefirewall(void)
{
  /* TODO: we need to check if exceptions are disabled, and set opmode enable disable accordingly. */
  LPTSTR cmd = "\"netsh.exe\" firewall set opmode enable";
  ldebug ("Re-enable firewall cmd: %s", cmd);
  if (! runcommand(cmd)) {
    return FALSE;
  }
  return TRUE;
}

BOOL cleararpcache(void)
{
  LPSTR cmd;
  cmd = "\"netsh.exe\" interface ip delete arpcache";
  ldebug ("Clear ARP cache cmd: %s", cmd);
  if (! runcommand(cmd)) {
    return FALSE;
  }
  return TRUE;
}

BOOL flushdns(void)
{ 
  LPSTR cmd;
  cmd = "\"ipconfig.exe\" /flushdns";
  ldebug ("Flush DNS cmd: %s", cmd);
  if (! runcommand(cmd)) {
    return FALSE;
  }
  return TRUE;
}

BOOL configtap(void)
{
  LPSTR cmd;
  cmd = "\"netsh.exe\" interface ip set address \"" TOR_TAP_NAME "\" static 10.10.10.2 255.255.255.252 10.10.10.1 1";
  ldebug ("Tap config cmd: %s", cmd);
  if (! runcommand(cmd)) {
    return FALSE;
  }
  cmd = "\"netsh.exe\" interface ip set dns \"" TOR_TAP_NAME "\" static 4.2.2.2";
  ldebug ("Tap config cmd: %s", cmd);
  if (! runcommand(cmd)) {
    return FALSE;
  }
  cmd = "\"netsh.exe\" interface ip add dns \"" TOR_TAP_NAME "\" 4.2.2.4";
  ldebug ("Tap config cmd: %s", cmd);
  if (! runcommand(cmd)) {
    return FALSE;
  }
  ldebug ("Tap config complete.");
  return TRUE;
}

BOOL configbridge(void)
{
  LPSTR cmd;
  cmd = "\"netsh.exe\" interface ip set address \"Local Area Connection\" static 10.231.254.1 255.255.255.254";
  ldebug ("Bridge interface null route cmd: %s", cmd);
  if (! runcommand(cmd)) {
    return FALSE;
  }
  return TRUE;
}

BOOL checkvirtdisk(void) {
  HANDLE src = NULL;
  HANDLE dest = NULL;
  LPTSTR srcname = TOR_VM_LIB "\\hdd.img";
  LPTSTR destname = TOR_VM_STATE "\\hdd.img";
  CHAR * buff = NULL;
  DWORD  buffsz = 4096;
  DWORD  len;
  DWORD  written;
  
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
    CloseHandle (dest);
    return FALSE;
  }
  
  buff = (CHAR *)malloc(buffsz);
  while (ReadFile(src, buff, buffsz, &len, NULL) && (len > 0)) {
    WriteFile(dest, buff, len, &written, NULL);
  }
  free (buff); 
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
  int i = 0;
  int numconn = 0;
  struct s_rconnelem *  ce = NULL;
  struct s_rconnelem *  ne = NULL;
  const char name_string[] = "Name";

  status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        NETWORK_CONNECTIONS_KEY,
                        0,
                        KEY_READ,
                        &key);
  if (status != ERROR_SUCCESS) {
    lerror ("Failed to open key for read: %d", status); 
    return -1;
  }

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
        if (isconnected (enum_name)) {
          linfo ("Interface %s (%s) is currently connected.", ce->name, ce->macaddr);
          ce->isactive = TRUE;
          snprintf(tcpip_string,
                   sizeof(tcpip_string),
                   "%s\\%s",
                   TCPIP_KEY, enum_name);
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
            RegCloseKey (tkey);
          }
        }
      }
      RegCloseKey (ckey);
    }
  }

  RegCloseKey (key);

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
      else {
            /* printf ("Failed read key %s , errorno: %d", connection_string, status); */
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

  return numconn;
}

BOOL buildcmdline (struct s_rconnelem *  brif,
                   BOOL                  usedebug,
                   BOOL                  noinit,
                   char **               cmdline)
{
/* DHCPSVR DHCPNAME LEASE ISDHCP CTLSOCK HASHPW */
  *cmdline = (char *)malloc(4096);
  const char * basecmds = "quiet loglevel=0 clocksource=hpet";
  const char * dbgcmds  = "loglevel=9 clocksource=hpet DEBUGINIT";
  if (noinit) {
    snprintf (*cmdline, 4095,
              "%s NOINIT",
              basecmds);
  }
  else {
    if (brif->isdhcp == FALSE) {
      snprintf (*cmdline, 4095,
                "%s IP=%s MASK=%s GW=%s MAC=%s MTU=1480 PRIVIP=10.10.10.1",
                usedebug ? dbgcmds : basecmds,
                brif->ipaddr,
                brif->netmask,
                brif->gateway,
                brif->macaddr);
    }
    else {
      snprintf (*cmdline, 4095,
                "%s IP=%s MASK=%s GW=%s MAC=%s MTU=1480 PRIVIP=10.10.10.1 ISDHCP DHCPSVR=%s LEASE=%d CTLSOCK=10.10.10.1:9051 HASHPW=%s",
                usedebug ? dbgcmds : basecmds,
                brif->ipaddr,
                brif->netmask,
                brif->gateway,
                brif->macaddr,
                brif->dhcpsvr,
                0,
                /* control port password is "password"
                 * TODO: use Crypto API to collect entropy for ephemeral password generation
                 */
                "16:6407E39581A121B26051A360CA8BB1535C73877C894E7B6EC554422789");
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

  TCHAR *cmd = (TCHAR*)malloc(4096);
  snprintf (cmd, 4095,
            "\"C:\\Tor_VM\\bin\\qemu.exe\" -L . -kernel ../lib/vmlinuz -hda ../state/hdd.img -m %d -std-vga");
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
  /* DWORD opts = BELOW_NORMAL_PRIORITY_CLASS; */
  DWORD opts = CREATE_NEW_PROCESS_GROUP;
  DWORD numwritten;
  DWORD pipesz;

  ZeroMemory( &si, sizeof(si) );
  ZeroMemory( &sattr, sizeof(sattr) );
  ZeroMemory( pi, sizeof(PROCESS_INFORMATION) );
  si.cb = sizeof(si);
/*  sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
  sattr.bInheritHandle = TRUE;
  sattr.lpSecurityDescriptor = NULL; */
  dir = TOR_VM_BIN;
  cmd = (LPTSTR)malloc(4096);
  if (tapname) {
    snprintf (cmd, 4095,
              "\"" TOR_VM_BIN "\\qemu.exe\" -name \"Tor VM \" -L . -kernel ../lib/vmlinuz -append \"%s\" -hda ../state/hdd.img -m %d -std-vga -net nic,model=pcnet,macaddr=%s -net pcap,devicename=\"%s\" -net nic,vlan=1,model=pcnet -net tap,vlan=1,ifname=\"%s\"",
            cmdline,
            32,
            macaddr,
            bridgeintf,
            tapname);
  }
  else {
    snprintf (cmd, 4095,
              "\"" TOR_VM_BIN "\\qemu.exe\" -name \"Tor VM \" -L . -kernel ../lib/vmlinuz -append \"%s\" -hda ../state/hdd.img -m %d -std-vga -net nic,model=pcnet,macaddr=%s -net pcap,devicename=\"%s\"",
            cmdline,
            32,
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
  while ( GetExitCodeProcess(pi->hProcess, &exitcode) && (exitcode == STILL_ACTIVE) ) {
    ldebug ("waiting for process to exit ...");
    Sleep (2000);
  }
  ldebug ("Done.");
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
  wcmd = W_TOR_VM_BASE L"\\torvm.exe";
  cmd = TOR_VM_BASE "\\torvm.exe";

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
    linfo ("Failed to re-launch process automatically with Administrator rights. Prompting user with Runas.");
    if (ShellExecute(NULL,
                     "runas",
                     cmd,
                     NULL,
                     NULL,
                     SW_HIDE) != ERROR_SUCCESS) {
      lerror ("Failed to re-launch via runas with Administrator rights. Unable to continue.");
      return FALSE;
    }
  }
  return TRUE;
}

BOOL runningdetached (void)
{
  return TRUE;
}

BOOL detachself (void)
{
  STARTUPINFOW si = {0};
  PROCESS_INFORMATION pi = {0};
  LPTSTR cmd = NULL;
  LPTSTR mypath = NULL;
  LPTSTR args = "";
  bgstartupinfo (&si);
  getmypath(&mypath);
  cmd = (TCHAR *)malloc (4096);
  snprintf (cmd, 4095,
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
 
  envvar = (LPTSTR) malloc(EBUFSZ * sizeof(TCHAR));
  if(envvar == NULL) {
    lerror ("setupenv: out of memory.");
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
  newvar = (LPTSTR) malloc(retval * sizeof(TCHAR));
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
  snprintf (newvar, retval -1, "%s;%s;%s", TOR_VM_LIB, TOR_VM_BIN, envvar);

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


int main(int argc, char **argv)
{
  const char *cmd;
  int  numintf;
  struct s_rconnelem *connlist = NULL;
  struct s_rconnelem *ce = NULL;
  BOOL  indebug = FALSE;
  BOOL  vmnop = FALSE;
  BOOL  noinit = FALSE;
  BOOL  foundit = FALSE;
  char *  cmdline = NULL;

  if (getosbits() > 32) {
    lerror ("Error: only 32bit operating systems are currently supported.");
    MessageBox(NULL,
               "Sorry, only 32bit operating systems are currently supported.",
               "Unsupported Operating System Architecture",
               MB_OK);
    exit (1);
  }

  if (!setupenv()) {
    fatal ("Unable to prepare process environment.");
  }

  /* ensure we are running detached or as service */
  if (! runningdetached()) {
    if(detachself()) {
      _exit(0);
    }
  }

  /* invocation options:
   * clean - restore network setup (if saved config exists), clean up tap, etc.
   * debug - launch vm in debug mode, shell at console in vm, etc.
   * vmnop - launch vm without any network devices.
   * repair - replace virtual disk with original empty disk image.
   * TODO: implement "real" command line options
   */
  if (argc > 1) {
    if (strcmp(argv[1], "clean") == 0) {
      uninstalltap();
      restorenetconfig();
      exit (0);
    }
    else if (strcmp(argv[1], "debug") == 0) {
      indebug = TRUE;
    }
    else if (strcmp(argv[1], "vmnop") == 0) {
      indebug = TRUE;
      vmnop = TRUE;
    }
    else if (strcmp(argv[1], "noinit") == 0) {
      noinit = TRUE;
    }
    else if (strcmp(argv[1], "detach") == 0) {
      if(detachself()) { _exit(0); }
    }
    else if (strcmp(argv[1], "repair") == 0) {
      LPTSTR fname = TOR_VM_STATE "\\hdd.img";
      DeleteFile (fname);
      linfo ("Restored virtual disk image back to original state.");
    }
  }

  if (!haveadminrights()) {
    if (promptrunasadmin()) {
      if (respawnasadmin() == TRUE) {
        return 0;
      }
    }
    return 1;
  }

  logto (TOR_VM_STATE "\\vmlog.txt");
  debugto (TOR_VM_STATE "\\debug.txt");

  if (!vmnop) {
    if (!savenetconfig()) {
      fatal ("Unable to save current network configuration.");
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
    if (! buildcmdline(ce, indebug, noinit, &cmdline)) {
      lerror ("Unable to generate command line for kernel.");
      goto shutdown;
    }
  }

  ldebug ("Generated kernel command line: %s", cmdline);

  PROCESS_INFORMATION pi;
  if (!vmnop) {
    if (! launchtorvm(&pi,
                      ce->name,
                      ce->macaddr,
                      TOR_TAP_NAME,
                      cmdline)) {
      lerror ("Unable to launch Qemu TorVM instance.");
      goto shutdown;
    }
  }
  else {
    if (! spawnprocess(&pi, "qemu.exe")) {
      lerror ("Unable to launch default Qemu instance.");
      goto shutdown;
    }
  }

  /* need to delay long enough to allow qemu to start and open tap device */
  Sleep (2000);

  if (! isrunning(&pi)) {
    buildcmdline(ce, FALSE, TRUE, &cmdline);
    launchtorvm(&pi,
                ce->name,
                ce->macaddr,
                NULL,
                cmdline);
    /* need to delay long enough to allow qemu to start and open tap device */
    Sleep (2000);
    if (! isrunning(&pi)) {
      lerror ("Tor VM no tap fallback failed to start properly.");
      goto shutdown;
    }
  }
  else {
    if (! configtap()) {
      lerror ("Unable to configure tap device.  Exiting.");
      goto shutdown;
    }
  }

  waitforit(&pi);

  linfo ("Tor VM closed, restoring host network and services.");

 shutdown:
  if (getosversion() > OS_2000) {
    if (! enablefirewall()) {
      lerror ("Unable to re-enable windows firewall.");
    }
  }
 /* TEMP: leave for now ....
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

