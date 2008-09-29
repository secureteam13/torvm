/* Copyright (C) 2008  The Tor Project, Inc.
 * See LICENSE file for rights and terms.
 */

/* enable certain parts of the win32 API for process / system functions */
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <tchar.h>
#include <winreg.h>
#include <winioctl.h>
#include <winerror.h>
#include <wincrypt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/* things that should go into configure / headers / runtime dynamic */
#define REG_NAME_MAX   256
#define TOR_VM_ROOT    "C:\\Tor_VM"
#define W_TOR_VM_ROOT  L"C:\\Tor_VM"
#define TOR_VM_BIN     TOR_VM_ROOT "\\bin"
#define TOR_VM_LIB     TOR_VM_ROOT "\\lib"
#define TOR_VM_STATE   TOR_VM_ROOT "\\state"
#define WIN_DRV_DIR    "C:\\WINDOWS\\system32\\drivers"
#define TOR_TAP_NAME   "Tor VM Tap32"
#define TOR_TAP_SVC    "tortap91"

struct s_rtcpipvals {
  BOOL    isdhcp;
  LPTSTR  ipaddr;
  LPTSTR  netmask;
  LPTSTR  gateway;
  LPTSTR  hostname;
};

struct s_rconnelem {
  int     idx;
  BOOL    isactive;
  BOOL    isdefgw;
  LPTSTR  name;
  LPTSTR  guid;
  LPTSTR  driver;
  LPTSTR  macaddr;
  struct s_rconnelem * next;
};


/* jump hoops to read ethernet adapter MAC address.
 */
#define _NDIS_CONTROL_CODE(request,method) \
	CTL_CODE(FILE_DEVICE_PHYSICAL_NETCARD, request, method, FILE_ANY_ACCESS)
#define IOCTL_NDIS_QUERY_GLOBAL_STATS   _NDIS_CONTROL_CODE( 0, METHOD_OUT_DIRECT )

/* these are not yet used, but we may need them to interact with network devices
 * directly to enable/disable, change MAC, etc.
 */
#define IOCTL_NDIS_QUERY_ALL_STATS      _NDIS_CONTROL_CODE( 1, METHOD_OUT_DIRECT )
#define IOCTL_NDIS_ADD_DEVICE           _NDIS_CONTROL_CODE( 2, METHOD_BUFFERED )
#define IOCTL_NDIS_DELETE_DEVICE        _NDIS_CONTROL_CODE( 3, METHOD_BUFFERED )
#define IOCTL_NDIS_TRANSLATE_NAME       _NDIS_CONTROL_CODE( 4, METHOD_BUFFERED )
#define IOCTL_NDIS_ADD_TDI_DEVICE       _NDIS_CONTROL_CODE( 5, METHOD_BUFFERED )
#define IOCTL_NDIS_NOTIFY_PROTOCOL      _NDIS_CONTROL_CODE( 6, METHOD_BUFFERED )
#define IOCTL_NDIS_GET_LOG_DATA         _NDIS_CONTROL_CODE( 7, METHOD_OUT_DIRECT )

/* OID's we need to query */
#define OID_802_3_PERMANENT_ADDRESS             0x01010101
#define OID_802_3_CURRENT_ADDRESS               0x01010102
/* probably will never need these, but just in case ... */
#define OID_GEN_MEDIA_IN_USE                    0x00010104
#define OID_WAN_PERMANENT_ADDRESS               0x04010101
#define OID_WAN_CURRENT_ADDRESS                 0x04010102
#define OID_WW_GEN_PERMANENT_ADDRESS            0x0901010B
#define OID_WW_GEN_CURRENT_ADDRESS              0x0901010C

/* win32 registry fun */
#define ADAPTER_KEY "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}"
#define NETWORK_CONNECTIONS_KEY "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}"
#define NETWORK_CLIENTS_KEY "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E973-E325-11CE-BFC1-08002BE10318}"
#define NETWORK_SERVICES_KEY "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E974-E325-11CE-BFC1-08002BE10318}"
#define NETWORK_PROTOCOLS_KEY "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E975-E325-11CE-BFC1-08002BE10318}"
#define TCPIP_KEY "SYSTEM\\CurrentControlSet\\Services\\Tcpip"


/* debug, info and error logging
 *  lerror to stderr and log file (if set)
 *  linfo to log file
 *  ldebug to debug log file
 *  fatal logs error and then exits process
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
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
}


static void _flog (HANDLE        fd,
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

  if (fd == INVALID_HANDLE_VALUE)
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
  WriteFile (fd, msgbuf, len, &written, NULL);
  WriteFile (fd, newline, strlen(newline), &written, NULL);
  FlushFileBuffers (fd);
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
  _flog (fd, "ERROR", format, argptr);
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
  _flog (fd, "ERROR", format, argptr);
  va_end (argptr);
  return;
}

void linfo (const char* format, ...)
{
  va_list  argptr;
  if (s_logh == INVALID_HANDLE_VALUE)
    return;

  va_start (argptr, format);
  _flog (s_logh, "info", format, argptr);
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
                       0,
                       NULL,
                       CREATE_ALWAYS,   
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
}


void ldebug (const char* format, ...)
{
  va_list  argptr;
  if (s_dbgh == INVALID_HANDLE_VALUE)
    return;

  va_start (argptr, format);
  _flog (s_dbgh, "debug", format, argptr);
  va_end (argptr);
  return;
}


BOOL setdriversigning (BOOL sigcheck)
{
  /* thanks to Stefan 'Sec' Zehl and Blaine Fleming for this snippet.
   * see http://support.microsoft.com/?kbid=298503 for details on this subversion.
   * the ideal alternative is to pay the thousands of dollars for a driver signature.
   */
#define HP_HASHVALUE HP_HASHVAL
  HCRYPTPROV cryptoprovider;
  HCRYPTHASH digest;
  BYTE data[16];
  DWORD len;
  DWORD seed;
  HKEY rkey;
  BYTE onoff;
  char regval[4];
  int x;

  onoff = sigcheck ? 1 : 0;
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   "System\\WPA\\PnP",
                   0,
                   KEY_READ,
                   &rkey) != ERROR_SUCCESS){
    return FALSE;
  }
  len = sizeof(seed);
  if(RegQueryValueEx(rkey,
                     "seed",
                     NULL,
                     NULL,
                     (BYTE*)&seed,
                     &len) != ERROR_SUCCESS){
    return FALSE;
  }
  RegCloseKey(rkey);
  if (!CryptAcquireContext(&cryptoprovider,
                          NULL,
                          NULL,
                          PROV_RSA_FULL,
                          0)) {
    if (!CryptAcquireContext(&cryptoprovider,
                             NULL,
                             NULL,
                             PROV_RSA_FULL,
                             CRYPT_NEWKEYSET)) {
      return FALSE;
    }
  }
  if (!CryptCreateHash(cryptoprovider,
                       CALG_MD5,
                       0,
                       0,
                       &digest)) {
    return FALSE;
  }
  ZeroMemory( regval, sizeof(regval) );
  regval[1] = onoff;
  if (!CryptHashData(digest,
                     regval,
                     sizeof(regval),
                     0)) {
    return FALSE;
  }
  if (!CryptHashData(digest,
                     (BYTE*)&seed,
                     sizeof(seed),
                     0)) {
    return FALSE;
  }
  len = sizeof(data);
  if (!CryptGetHashParam(digest,
                         HP_HASHVALUE,
                         data,
                         &len,
                         0)) {
    return FALSE;
  }
  CryptDestroyHash(digest);
  CryptReleaseContext(cryptoprovider, 0);
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   "Software\\Microsoft\\Windows\\CurrentVersion\\Setup",
                   0,
                   KEY_WRITE,
                   &rkey) != ERROR_SUCCESS) {
    return FALSE;
  }
  if (RegSetValueEx(rkey,
                    "PrivateHash",
                    0,
                    REG_BINARY,
                    data,
                    sizeof(data)) != ERROR_SUCCESS) {
    return FALSE;
  }
  RegCloseKey(rkey);

  /* the user preference may or may not be set.  if not, go to machine pref. */
  if (RegOpenKeyEx(HKEY_CURRENT_USER,
                   "Software\\Microsoft\\Driver Signing",
                   0,
                   KEY_WRITE,
                   &rkey) == ERROR_SUCCESS) {
    if(RegSetValueEx(rkey,
                     "Policy",
                     0,
                     REG_BINARY,
                     &onoff,
                     1) != ERROR_SUCCESS) {
      /* return FALSE; */
    }
    RegCloseKey(rkey);
  }
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   "Software\\Microsoft\\Driver Signing",
                   0,
                   KEY_WRITE,
                   &rkey) != ERROR_SUCCESS) {
    return FALSE;
  }
  if(RegSetValueEx(rkey,
                   "Policy",
                   0,
                   REG_BINARY,
                   &onoff,
                   1) != ERROR_SUCCESS) {
    return FALSE;
  }
  RegCloseKey(rkey);

  return TRUE;
}

BOOL istapinstalled(struct s_rconnelem *  connlist)
{
  return FALSE;
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
  
  opts = CREATE_NEW_PROCESS_GROUP;
  
  ZeroMemory( &pi, sizeof(pi) );
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  dir = TOR_VM_LIB;
  cmd = "\"" TOR_VM_BIN "\\devcon.exe\" remove TORTAP91";
  
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
  DeleteFile (fname);
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
  DWORD opts = 0;
  DWORD numread;
  DWORD numwritten;
  CHAR * buff;

  opts = CREATE_NEW_PROCESS_GROUP;

  ZeroMemory( &pi, sizeof(pi) );
  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
  sattr.bInheritHandle = TRUE;
  sattr.lpSecurityDescriptor = NULL;
  dir = TOR_VM_STATE;
  /* cmd = "\"netsh.exe\" interface ip dump" */
  cmd = "\"netsh.exe\" dump";

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
                     TRUE,   // handles are inherited
                     opts,
                     NULL,   // environment block
                     dir,
                     &si,
                     &pi) ) {
    lerror ("Failed to launch process.  Error code: %d", GetLastError());
    return FALSE;
  }

  CloseHandle(stdout_wr);
  CloseHandle(stdin_rd);
  CloseHandle(stdin_wr);

  fh = CreateFile (TOR_VM_STATE "\\netcfg.save",
                   GENERIC_WRITE,
                   0,
                   NULL,
                   CREATE_ALWAYS,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL);

  buff = (CHAR *)malloc(READSIZE);
  while (ReadFile(stdout_rd, buff, READSIZE, &numread, NULL) && (numread > 0)) {
    WriteFile(fh, buff, numread, &numwritten, NULL);
  }
  CloseHandle (fh);

  linfo ("Saved current network configuration state.");
  CloseHandle(stdout_rd);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  return TRUE;  
}

BOOL restorenetconfig(void)
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
  LPSTR cmd;
  cmd = "\"netsh.exe\" firewall set opmode disable";
  if (! runcommand(cmd)) {
    return FALSE;
  }
  return TRUE;
}

BOOL cleararpcache(void)
{
  LPSTR cmd;
  cmd = "\"netsh.exe\" interface ip delete arpcache";
  if (! runcommand(cmd)) {
    return FALSE;
  }
  return TRUE;
}

BOOL flushdns(void)
{ 
  LPSTR cmd;
  cmd = "\"ipconfig.exe\" /flushdns";
  if (! runcommand(cmd)) {
    return FALSE;
  }
  return TRUE;
}

BOOL configtap(void)
{
  LPSTR cmd;
  cmd = "\"netsh.exe\" interface ip set address \"" TOR_TAP_NAME "\" static 10.10.10.2 255.255.255.252 10.10.10.1 1";
  if (! runcommand(cmd)) {
    return FALSE;
  }
  cmd = "\"netsh.exe\" interface ip set dns \"" TOR_TAP_NAME "\" static 4.2.2.2";
  if (! runcommand(cmd)) {
    return FALSE;
  }
  cmd = "\"netsh.exe\" interface ip add dns \"" TOR_TAP_NAME "\" 4.2.2.4";
  if (! runcommand(cmd)) {
    return FALSE;
  }
  return TRUE;
}

BOOL getmacaddr(const char *  devguid,
                char **       mac)
{
  char *  devfstr = NULL;
  unsigned char  macbuf[6];
  BOOL   status;
  HANDLE devfd;
  DWORD retsz, oidcode;
  BOOL  retval = FALSE;

  *mac = NULL;
  devfstr = malloc(1024);
  snprintf (devfstr, 1023, "\\\\.\\%s", devguid);
  devfd = CreateFile(devfstr,
                     0,
                     FILE_SHARE_READ | FILE_SHARE_WRITE, 
                     NULL,
                     OPEN_EXISTING,
                     0,
                     NULL);
  if (devfd == INVALID_HANDLE_VALUE)
  {
    lerror ("Unable to open net device handle for path: %s", devfstr);
    goto cleanup;
  }

#define MAXMAC 24
  oidcode = OID_802_3_CURRENT_ADDRESS;
  status = DeviceIoControl(devfd,
                           IOCTL_NDIS_QUERY_GLOBAL_STATS,
                           &oidcode, sizeof(oidcode),
                           macbuf, sizeof(macbuf),
                           &retsz,
                           (LPOVERLAPPED) NULL);
  if (retsz == sizeof(macbuf)) {
    *mac = malloc(MAXMAC);
    memset(*mac, 0, MAXMAC);
    snprintf(*mac, MAXMAC-1,
             "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
             macbuf[0], macbuf[1], macbuf[2], macbuf[3], macbuf[4], macbuf[5]);
    retval = TRUE;
  }
  else {
    retval = FALSE;
  }

 cleanup:
  if (devfd != INVALID_HANDLE_VALUE)
    CloseHandle(devfd);
  free(devfstr);

  return retval;
}

int loadnetinfo(struct s_rconnelem **connlist)
{
  LONG status;
  HKEY key;
  HKEY wkey;
  DWORD len;
  int i = 0;
  int stop = 0;
  int numconn = 0;
  struct s_rconnelem *  ce = NULL;
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

  while (!stop) {
    ce = malloc(sizeof(struct s_rconnelem));
    char enum_name[REG_NAME_MAX];
    char connection_string[REG_NAME_MAX];
    HKEY ckey;
    char name_data[REG_NAME_MAX];
    DWORD name_type;

    len = sizeof (enum_name);
    status = RegEnumKeyEx(key,
                          i,
                          enum_name,
                          &len,
                          NULL,
                          NULL,
                          NULL,
                          NULL);
    if (status == ERROR_NO_MORE_ITEMS)
        break;
    else if (status != ERROR_SUCCESS) {
        return -1;
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
        return -1;
      }
      else {
        if (getmacaddr (enum_name, &(ce->macaddr)) == TRUE) {
          linfo ("Interface %s => %s  mac(%s)", name_data, enum_name, ce->macaddr);
        }
      }
      RegCloseKey (ckey);
    }
    ++i;
  }

  RegCloseKey (key);

    i = 0;
    stop = 0;
    status = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        ADAPTER_KEY,
        0,
        KEY_READ,
        &key);

    if (status != ERROR_SUCCESS) {
        lerror ("Failed to open key for read: %d", status);
        return -1;
    }

    while (!stop)
    {
        char enum_name[REG_NAME_MAX];
        char connection_string[REG_NAME_MAX];
        HKEY ckey;
        char name_data[REG_NAME_MAX];
        char cguid[REG_NAME_MAX];
        DWORD name_type;

        len = sizeof (enum_name);
        status = RegEnumKeyEx(
            key,
            i,
            enum_name,
            &len,
            NULL,
            NULL,
            NULL,
            NULL);

        if (status == ERROR_NO_MORE_ITEMS)
            break;
        else if (status != ERROR_SUCCESS) {
            return -1;
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
                "DriverDesc",
                NULL,
                &name_type,
                name_data,
                &len);

            if (status != ERROR_SUCCESS || name_type != REG_SZ) {
            }
            else {
                /* printf ("-%s- %s", enum_name, name_data); */
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

            if (status != ERROR_SUCCESS || name_type != REG_SZ) {
                /* printf ("Failed parse of key %s\\NetCfgInstanceId , errorno: %d", connection_string, status); */
            }
            else {
                /* printf ("GUID: %s", name_data); */
                strcpy (cguid, name_data);
            }

            RegCloseKey (ckey);
        }
        else {
            /* printf ("Failed read key %s , errorno: %d", connection_string, status); */
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

            if (status != ERROR_SUCCESS || name_type != REG_SZ) {
                /* printf ("Failed parse of key %s\\Service , errorno: %d", connection_string, status); */
            }
            else {
                /* printf ("Service: %s", name_data); */
                if (strcmp(name_data, TOR_TAP_SVC) == 0) {
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
        else {
            /* printf ("Failed read key %s , errorno: %d", connection_string, status); */
        }

        i++;
    }

    RegCloseKey (key);

    return 0;
}

/* keep linkage to these dynamic, in case the requisite Dll's don't exist. */
typedef BOOL (__stdcall *PFnIsUserAnAdmin)(void);
typedef BOOL (__stdcall *PFnAllocateAndInitializeSid)(PSID_IDENTIFIER_AUTHORITY pIdAuth, BYTE nSubAuthCount, DWORD dwSubAuth0, DWORD dwSubAuth1, DWORD dwSubAuth2, DWORD dwSubAuth3, DWORD dwSubAuth4, DWORD dwSubAuth5, DWORD dwSubAuth6, DWORD dwSubAuth7, PSID pSid);
typedef BOOL (WINAPI *PFnCheckTokenMembership)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);
typedef PVOID (__stdcall *PFnFreeSid)(PSID pSid);

BOOL haveadminrights (void)
{
  SID_IDENTIFIER_AUTHORITY  ntauth = SECURITY_NT_AUTHORITY;
  PSID  admgroup;
  BOOL  isadmin = FALSE;
  HMODULE   module;
  PFnIsUserAnAdmin  pfnIsUserAnAdmin = NULL;
  PFnAllocateAndInitializeSid  pfnAllocateAndInitializeSid = NULL;
  PFnCheckTokenMembership  pfnCheckTokenMembership = NULL;
  PFnFreeSid  pfnFreeSid = NULL;
  
  /* use IsUserAnAdmin when possible (Vista or greater).  otherwise we fall back to checking
   * token membership manually.  For Vista and greater we want to know if we are currently running
   * with Administrator rights, not only that user is a member of Administrator group.
   */
  module = LoadLibrary("shell32.dll");
  if (module) {
    pfnIsUserAnAdmin = (PFnIsUserAnAdmin) GetProcAddress(module, "IsUserAnAdmin");
    if (pfnIsUserAnAdmin) {
      isadmin = pfnIsUserAnAdmin();
      FreeLibrary(module);
      return isadmin;
    }
    FreeLibrary(module);
  }
  module = LoadLibrary("advapi32.dll");
  if (module) {
    pfnAllocateAndInitializeSid = (PFnAllocateAndInitializeSid) GetProcAddress(module, "AllocateAndInitializeSid");
    pfnCheckTokenMembership = (PFnCheckTokenMembership) GetProcAddress(module, "CheckTokenMembership");
    pfnFreeSid = (PFnFreeSid) GetProcAddress(module, "FreeSid");
    if (pfnAllocateAndInitializeSid && pfnCheckTokenMembership && pfnFreeSid) {
      if(pfnAllocateAndInitializeSid(&ntauth,
                                     2,
                                     SECURITY_BUILTIN_DOMAIN_RID,
                                     DOMAIN_ALIAS_RID_ADMINS,
                                     0, 0, 0, 0, 0, 0,
                                     &admgroup))
      {
        if( !pfnCheckTokenMembership(NULL,
                                     admgroup,
                                     &isadmin) )
        {
          /* error occurred? default to false */
          isadmin = FALSE;
        }
        pfnFreeSid(admgroup);
      }
    }
    FreeLibrary(module);
  }

  return isadmin;
}

BOOL launchtorvm (PROCESS_INFORMATION * pi)
{
  STARTUPINFO si;
  LPTSTR cmd = NULL;
  LPTSTR dir = NULL;
  DWORD opts = CREATE_NEW_PROCESS_GROUP | BELOW_NORMAL_PRIORITY_CLASS;

  ZeroMemory( &si, sizeof(si) );
  ZeroMemory( pi, sizeof(PROCESS_INFORMATION) );
  si.cb = sizeof(si);
  dir = TOR_VM_BIN;
  cmd = "\"" TOR_VM_BIN "\\qemu.exe\" -name \"Tor VM \" -L . -kernel ../lib/vmlinuz -append \"clocksource=hpet IP=172.16.2.42 MASK=255.255.255.0 GW=172.16.2.1 MAC=00:11:22:33:44:55 MTU=1480 PRIVIP=10.10.10.1\" -hda ../state/hdd.img -m 32 -std-vga -net nic,model=pcnet,macaddr=00:11:22:33:44:55 -net pcap,devicename=\"Local Area Connection\" -net nic,vlan=1,model=pcnet -net tap,vlan=1,ifname=\"" TOR_TAP_NAME "\" -net user,vlan=2 -net nic,vlan=2,model=pcnet";

  if( !CreateProcess(NULL, 
                     cmd,
                     NULL,   // process handle no inherit
                     NULL,   // thread handle no inherit
                     FALSE,  // default handle inheritance false
                     opts,
                     NULL,   // environment block
                     dir,
                     &si,
                     pi) ) {
    lerror ("Failed to launch Qemu Tor VM process.  Error code: %d", GetLastError());
    return FALSE;
  }

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

  //authopts = LOGON_WITH_PROFILE;
  propts = CREATE_NEW_PROCESS_GROUP | HIGH_PRIORITY_CLASS;

  si.cb = sizeof(si);
  wcmd = W_TOR_VM_ROOT L"\\torvm.exe";
  cmd = TOR_VM_ROOT "\\torvm.exe";

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
  struct s_rconnelem *connlist = NULL;

  if (!haveadminrights()) {
    if (promptrunasadmin()) {
      if (respawnasadmin() == TRUE) {
        return 0;
      }
    }
    return 1;
  }

  logto (TOR_VM_STATE "\\torvm.log");
  debugto (TOR_VM_STATE "\\debug.log");

  if (!setupenv()) {
    fatal ("Unable to prepare process environment.");
  }

  if (!savenetconfig()) {
    fatal ("Unable to save current network configuration.");
  }

  if (!installtornpf()) {
    lerror ("Unable to install Tor NPF service driver.");
    goto shutdown;
  }

  uninstalltap();

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

  loadnetinfo(&connlist);

  if (! disableservices()) {
    lerror ("Unable to disable dangerous windows network services.");
  }
  if (! disablefirewall()) {
    lerror ("Unable to disable windows firewall.");
  }
  if (! cleararpcache()) {
    lerror ("Unable to clear arp cache.");
  }
  if (! flushdns()) {
    lerror ("Unable to flush cached DNS entries.");
  }

  PROCESS_INFORMATION pi;
  if (! launchtorvm(&pi)) {
    lerror ("Unable to launch Qemu TorVM instance.");
    goto shutdown;
  }
  Sleep (4000);
  if (! isrunning(&pi)) {
    lerror ("Tor VM failed to start properly.");
    goto shutdown;
  }

  if (! configtap()) {
    lerror ("Unable to configure tap device.  Exiting.");
    goto shutdown;
  }
  Sleep (4000);

  waitforit(&pi);

  linfo ("Tor VM closed, restoring host network and services.");

 shutdown:
  if (! uninstalltap()) {
    lerror ("Unable to remove TAP-Win32 device.");
  }
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

