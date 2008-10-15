/* Copyright (C) 2008  The Tor Project, Inc.
 * See LICENSE file for rights and terms.
 */

#include "apicommon.h"

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
#define OID_GEN_MEDIA_CONNECT_STATUS            0x00010114
/* probably will never need these, but just in case ... */
#define OID_GEN_MEDIA_IN_USE                    0x00010104
#define OID_WAN_PERMANENT_ADDRESS               0x04010101
#define OID_WAN_CURRENT_ADDRESS                 0x04010102
#define OID_WW_GEN_PERMANENT_ADDRESS            0x0901010B
#define OID_WW_GEN_CURRENT_ADDRESS              0x0901010C

BOOL getmypath (TCHAR **path)
{
  TCHAR  mypath[MAX_PATH];
  memset (mypath, 0, sizeof(mypath));
  if (! GetModuleFileName(NULL,
                          &mypath,
                          sizeof(mypath)-1)) {
    lerror ("Unable to obtain current program path.");
    return FALSE;
  }
  *path = strdup(mypath);
  return TRUE;
}

void bgstartupinfo (STARTUPINFO *si)
{
  si->dwXCountChars = 48;
  si->dwYCountChars = 5;
  si->lpTitle = "Tor VM Win32";
  si->dwFillAttribute = BACKGROUND_BLUE | BACKGROUND_INTENSITY; 
  si->wShowWindow = SW_HIDE;
  si->dwFlags |= STARTF_USECOUNTCHARS | STARTF_USEFILLATTRIBUTE | STARTF_USESHOWWINDOW;
  return;
}

BOOL localhnd (HANDLE  *hnd)
{
  HANDLE  orighnd = *hnd;
  /* dupe handle for no inherit */
  if (! DuplicateHandle(GetCurrentProcess(),
                        orighnd,
                        GetCurrentProcess(),
                        hnd,
                        0,
                        FALSE, /* no inherit */
                        DUPLICATE_SAME_ACCESS)) {
    lerror ("Unable to duplicate handle.  Error code: %d", GetLastError());
    *hnd = orighnd;
    return FALSE;
  }
  /* now that we know the dupe was successful, close the original handle. */
  CloseHandle (orighnd);
  return TRUE;
}

BOOL proclocalhnd (HANDLE srcproc,
                   HANDLE dstproc,
                   HANDLE *hnd)
{
  HANDLE  orighnd = *hnd;
  if (! DuplicateHandle(srcproc,
                        orighnd,
                        dstproc,
                        hnd,
                        0,
                        FALSE, /* no inherit */
                        DUPLICATE_SAME_ACCESS)) {
    lerror ("Unable to duplicate handle.  Error code: %d", GetLastError());
    *hnd = orighnd;
    return FALSE;
  }
  return TRUE;
}

BOOL getcompguid (TCHAR **guid)
{
/* MRP_TEMP this needs dynamic linkage */
  return FALSE;
  static const int  alen = 64 * sizeof(TCHAR);
  *guid = (TCHAR *)malloc(alen);
  if (! *guid)
    fatal ("Allocation failure in: %s line no: %s with sz: %d", __FILE__ , __LINE__ , alen);
#if 0
  if (! GetComputerObjectName(NameUniqueId,
                              *guid,
                              alen)) {
    lerror ("Unable to obtain computer unique id name.  Error code: %d", GetLastError());
    free (*guid);
    *guid = NULL;
    return FALSE;
  }
#endif
  return TRUE;
}

int getosversion (void) {
  static int osver = -1;

  /* used cached version info if version has been checked already */
  if (osver >= 0)
    return osver;

  osver = OS_UNKNOWN;
  OSVERSIONINFO info;
  ZeroMemory(&info, sizeof(OSVERSIONINFO));
  info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&info);
  if (info.dwMajorVersion == 5) {
    if (info.dwMinorVersion == 0) {
      ldebug ("Operating system version is Windows 2000");
      osver = OS_2000;
    }
    else if (info.dwMinorVersion == 1) {
      ldebug ("Operating system version is Windows XP");
      osver = OS_XP;
    }
    else if (info.dwMinorVersion == 2) {
      ldebug ("Operating system version is Windows Server 2003");
      osver = OS_SERVER2003;
    }
  }
  else if (info.dwMajorVersion == 6) {
    OSVERSIONINFOEX exinfo;
    ZeroMemory(&exinfo, sizeof(OSVERSIONINFOEX));
    exinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx(&exinfo);
    if (exinfo.wProductType != VER_NT_WORKSTATION) {
      ldebug ("Operating system version is Windows Vista");
      osver = OS_VISTA;
    }
    else {
      ldebug ("Operating system version is Windows Server 2008");
      osver = OS_SERVER2008;
    }
  }
  return osver;
}

int getosbits (void) {
  static int  osbits = -1;

  if (osbits >= 0)
    return osbits;

  osbits = 0;
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  if (info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
    ldebug ("Operating system is running on 64bit architecture.");
    osbits = 64;
  }
  else if (info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
    ldebug ("Operating system is running on 32bit architecture.");
    osbits = 32;
  }
  else {
    ldebug ("Operating system is running on UNKNOWN architecture.");
  }
  return osbits;
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

BOOL isconnected(const char *  devguid)
{
  char *  devfstr = NULL;
  BOOL   status;
  HANDLE devfd;
  DWORD retsz, oidcode, intfStatus;
  BOOL  retval = FALSE;

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

  oidcode = OID_GEN_MEDIA_CONNECT_STATUS;
  status = DeviceIoControl(devfd,
                           IOCTL_NDIS_QUERY_GLOBAL_STATS,
                           &oidcode, sizeof(oidcode),
                           &intfStatus, sizeof(intfStatus),
                           &retsz,
                           (LPOVERLAPPED) NULL);
  if (status) {
    ldebug ("Received media connect status %d for device %s.", intfStatus, devguid);
    retval = (intfStatus == 0) ? TRUE : FALSE;
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



