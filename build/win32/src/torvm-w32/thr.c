#include "torvm.h"
/*XXX ignore threading until bundle merge completed so buildbot is happy now.*/
#if 0
/* Some statics to keep track of things...
 * XXX: note that this is inherently unaware of a thread handle
 * allocated by an external process with privs in the Tor VM process.
 * Inter-process threading and locking is explicitly not provided.
 */
LPCRITICAL_SECTION  s_thridx_cs = NULL;
DWORD s_thrcount = 0;
struct s_thrinfo *  s_thrlist = NULL;

BOOL  createcs (LPCRITICAL_SECTION cs)
{
   /* The high bit is set to pre-allocate any necessary resources so that
    * a low memory condition does introduce an exception leading to ugly
    * failure recovery...
    */
   if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x80000400) ) return FALSE;
   return TRUE;
}

BOOL  destroycs (LPCRITICAL_SECTION cs)
{
  return TRUE;
}

BOOL  entercs (LPCRITICAL_SECTION cs)
{
  return TRUE;
}

BOOL  leavecs (LPCRITICAL_SECTION cs)
{
  return TRUE;
}

BOOL  createlock (LPHANDLE lockptr)
{
  *lockptr = CreateMutex(0, FALSE, 0);
  return TRUE;
}

BOOL  destroylock (HANDLE lockptr)
{
  return TRUE;
}

BOOL  trylock (HANDLE lock)
{
  return TRUE;
}

BOOL  waitlock (HANDLE lock,
                DWORD  mstimout)
{
  return TRUE;
}

BOOL  unlock (HANDLE lock)
{
  return TRUE;
}


/* Semaphore signalling primitives. */
BOOL  createsem (LPHANDLE semptr,
                 LONG     limit,
                 BOOL     startsignaled)
{
  DWORD icount = 0;
  if (limit > MAX_SEM_COUNT) limit = MAX_SEM_COUNT;
  if (startsignaled == TRUE) icount = limit;
  *semptr = CreateSemaphore( 
              0,              // default security attributes
              icount,         // initial count
              limit,          // maximum count
              0               // unnamed semaphore
            );
  return TRUE;
}

BOOL  destroysem (HANDLE semptr)
{
  return TRUE;
}
 
BOOL  trysem (HANDLE semptr)
{
  return TRUE;
}
 
BOOL  waitsem (HANDLE semptr,
               DWORD  mstimout)
{
  return TRUE;
}

BOOL  signalsem (HANDLE semptr)
{
  return TRUE;
}

BOOL  createthr (PFnThreadMain  thrmain,
                 LPDWORD        thrid,
                 BOOL           suspended)
{
  LPTHREAD_START_ROUTINE f = (LPTHREAD_START_ROUTINE) thrmain;
  DWORD tid;
  DWORD cflags = 0;
  HANDLE newthr;
  if (suspended) cflags |= CREATE_SUSPENDED;
  newthr = CreateThread(
             NULL,    // default security attributes
             0,       // use default stack size
             f,
             arg,
             cflags,
             &tid);
  return TRUE;
}

BOOL  destroythr (HANDLE thr)
{
  return TRUE;
}

BOOL  pausethr (HANDLE thr)
{
  return TRUE;
}

BOOL  resumethr (HANDLE thr)
{
  return TRUE;
}

VOID  exitthr (DWORD exitcode)
{
  return TRUE;
}
 
BOOL  checkthr (HANDLE thr,
                LPDWORD retval)
{
  return TRUE;
}

BOOL  waitforthr (HANDLE thr,
                  DWORD  mstimout,
                  LPDWORD retval)
{
  return TRUE;
}

BOOL  waitforallthr (const HANDLE *thrlist,
                     DWORD         count,
                     DWORD         mstimout)
{
  return TRUE;
}

BOOL  waitforanythr (const HANDLE *thrlist,
                     DWORD         count,
                     DWORD         mstimout,
                     LPHANDLE      signaledhnd)
{
  return TRUE;
}

BOOL  setupthrctx (VOID)
{
  s_thridx_cs = 0;
  return TRUE;
}

VOID  cleanupthrctx (VOID)
{
  return;
}

BOOL  enumthrhnds (LPHANDLE *hndlist)
{
  return TRUE;
}

VOID  destroythrhnds (LPHANDLE hndlist)
{
  return;
}
#endif /* XXX end if 0 */
