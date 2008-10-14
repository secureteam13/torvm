/* Copyright (C) 2008  The Tor Project, Inc.
 * See LICENSE file for rights and terms.
 */
#ifndef __apicommon_h__
#define __apicommon_h__

/* enable certain parts of the win32 API for process / system functions
 * default to win2k.
 * other versions:
 *   0x0501 = XP / Server2003
 *   0x0502 = XP SP2 / Server2003 SP1
 *   0x0600 = Vista / Server2008
 */
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <security.h>
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

/* misc win32 api utility functions
 */
BOOL getmypath (TCHAR **path);
BOOL getprocwd (TCHAR **cwd);
BOOL setprocwd (const TCHAR *cwd);

/* localize a handle resource to the current process (no inherit)
 * types of handles supported:
 *   Access token
 *   Change notification
 *   Communications device
 *   Console input
 *   Console screen buffer
 *   Desktop
 *   Event
 *   File
 *   File mapping
 *   Job
 *   Mailslot
 *   Mutex
 *   Pipe
 *   Process
 *   Registry key
 *   Semaphore
 *   Thread
 *   Time
 *   Window station
 */
BOOL localhnd (HANDLE  *hnd);

/* duplicating files in other processes does not close original */
BOOL proclocalhnd (HANDLE srcproc,
                   HANDLE dstproc,
                   HANDLE *hnd);

/* get the current Windows OS version.  this is needed for things like the network
 * configuration export and some API calls.
 */
#define OS_UNKNOWN     0
#define OS_SERVER2008  5
#define OS_VISTA       4
#define OS_SERVER2003  3
#define OS_XP          2
#define OS_2000        1

int getosversion (void);
int getosbits (void);

BOOL getcompguid (TCHAR **guid);
void bgstartupinfo (STARTUPINFO *si);

BOOL getmacaddr(const char *  devguid,
                char **       mac);
BOOL isconnected(const char *  devguid);

#endif /* apicommon_h */
