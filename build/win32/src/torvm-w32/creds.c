/* Copyright (C) 2008  The Tor Project, Inc.
 * See LICENSE file for rights and terms.
 */
#include "creds.h"

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

/* keep linkage to these dynamic, in case the requisite Dll's don't exist. */
#define NTSTATUS ULONG

typedef struct _LSA_TRANSLATED_SID2 {
  SID_NAME_USE Use;
  PSID Sid;
  LONG DomainIndex;
  ULONG Flags;
} LSA_TRANSLATED_SID2, *PLSA_TRANSLATED_SID2;

typedef BOOL (__stdcall *PFnIsUserAnAdmin)(void);
typedef BOOL (__stdcall *PFnAllocateAndInitializeSid)(PSID_IDENTIFIER_AUTHORITY pIdAuth,
                                                      BYTE nSubAuthCount,
                                                      DWORD dwSubAuth0,
                                                      DWORD dwSubAuth1,
                                                      DWORD dwSubAuth2,
                                                      DWORD dwSubAuth3,
                                                      DWORD dwSubAuth4,
                                                      DWORD dwSubAuth5,
                                                      DWORD dwSubAuth6,
                                                      DWORD dwSubAuth7,
                                                      PSID pSid);
typedef BOOL (WINAPI *PFnCheckTokenMembership)(HANDLE TokenHandle,
                                               PSID SidToCheck,
                                               PBOOL IsMember);
typedef PVOID (__stdcall *PFnFreeSid)(PSID pSid);
typedef NTSTATUS (__stdcall *PFnLsaOpenPolicy)(PLSA_UNICODE_STRING SystemName,
                                               PLSA_OBJECT_ATTRIBUTES ObjectAttributes,
                                               ACCESS_MASK DesiredAccess,
                                               PLSA_HANDLE PolicyHandle);
typedef NTSTATUS (__stdcall *PFnLsaLookupNames2)(LSA_HANDLE PolicyHandle,
                                                 ULONG Flags,
                                                 ULONG Count,
                                                 PLSA_UNICODE_STRING Names,
                                                 PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
                                                 PLSA_TRANSLATED_SID2 *Sids);
typedef NTSTATUS (__stdcall *PFnLsaAddAccountRights)(LSA_HANDLE PolicyHandle,
                                                     PSID AccountSid,
                                                     PLSA_UNICODE_STRING UserRights,
                                                     ULONG CountOfRights);
typedef NTSTATUS (__stdcall *PFnLsaRemoveAccountRights)(LSA_HANDLE PolicyHandle,
                                                        PSID AccountSid,
                                                        BOOLEAN AllRights,
                                                        PLSA_UNICODE_STRING UserRights,
                                                        ULONG CountOfRights);
typedef NTSTATUS (__stdcall *PFnLsaEnumerateAccountRights)(LSA_HANDLE PolicyHandle,
                                                           PSID AccountSid,
                                                           PLSA_UNICODE_STRING *UserRights,
                                                           PULONG CountOfRights);
typedef BOOL (__stdcall *PFnAdjustTokenPrivileges)(HANDLE TokenHandle,
                                                   BOOL DisableAllPrivileges,
                                                   PTOKEN_PRIVILEGES NewState,
                                                   DWORD BufferLength,
                                                   PTOKEN_PRIVILEGES PreviousState,
                                                   PDWORD ReturnLength);
typedef ULONG (__stdcall *PFnLsaNtStatusToWinError)(NTSTATUS Status);

struct ft_advapi {
  PFnAllocateAndInitializeSid   AllocateAndInitializeSid;
  PFnFreeSid                    FreeSid;
  PFnCheckTokenMembership       CheckTokenMembership;
  PFnLsaOpenPolicy              LsaOpenPolicy;
  PFnLsaLookupNames2            LsaLookupNames2;
  PFnLsaAddAccountRights        LsaAddAccountRights;
  PFnLsaRemoveAccountRights     LsaRemoveAccountRights;
  PFnLsaEnumerateAccountRights  LsaEnumerateAccountRights;
  PFnAdjustTokenPrivileges      AdjustTokenPrivileges;
  PFnLsaNtStatusToWinError      LsaNtStatusToWinError;
};

static struct ft_advapi *s_advapi = NULL;
static HMODULE           s_advapi_hnd = INVALID_HANDLE_VALUE;

static void  loadadvapifuncs (void)
{
  if (s_advapi != NULL)
    return;

  s_advapi = (struct ft_advapi *)malloc(sizeof(struct ft_advapi));
  memset(s_advapi, 0, sizeof(struct ft_advapi));
  s_advapi_hnd = LoadLibrary("advapi32.dll");
  if (s_advapi_hnd) {
    ldebug ("Loading advapi functions from library.");
    s_advapi->AllocateAndInitializeSid = (PFnAllocateAndInitializeSid) GetProcAddress(s_advapi_hnd, "AllocateAndInitializeSid");
    s_advapi->FreeSid = (PFnFreeSid) GetProcAddress(s_advapi_hnd, "FreeSid");
    s_advapi->CheckTokenMembership = (PFnCheckTokenMembership) GetProcAddress(s_advapi_hnd, "CheckTokenMembership");
    s_advapi->LsaOpenPolicy = (PFnLsaOpenPolicy) GetProcAddress(s_advapi_hnd, "LsaOpenPolicy");
    s_advapi->LsaLookupNames2 = (PFnLsaLookupNames2) GetProcAddress(s_advapi_hnd, "LsaLookupNames2");
    s_advapi->LsaAddAccountRights = (PFnLsaAddAccountRights) GetProcAddress(s_advapi_hnd, "LsaAddAccountRights");
    s_advapi->LsaRemoveAccountRights = (PFnLsaRemoveAccountRights) GetProcAddress(s_advapi_hnd, "LsaRemoveAccountRights");
    s_advapi->LsaEnumerateAccountRights = (PFnLsaEnumerateAccountRights) GetProcAddress(s_advapi_hnd, "LsaEnumerateAccountRights");
    s_advapi->AdjustTokenPrivileges = (PFnAdjustTokenPrivileges) GetProcAddress(s_advapi_hnd, "AdjustTokenPrivileges");
    s_advapi->LsaNtStatusToWinError = (PFnLsaNtStatusToWinError) GetProcAddress(s_advapi_hnd, "LsaNtStatusToWinError");
  }
  else {
    ldebug ("No advapi library located; unable to map API functions.");
  }
  return;
}

BOOL haveadminrights (void)
{
  SID_IDENTIFIER_AUTHORITY  ntauth = SECURITY_NT_AUTHORITY;
  PSID  admgroup;
  BOOL  isadmin = FALSE;
  HMODULE   module;
  PFnIsUserAnAdmin  pfnIsUserAnAdmin;
  
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

  if (s_advapi == NULL)
    loadadvapifuncs();

  if (s_advapi->AllocateAndInitializeSid && 
      s_advapi->CheckTokenMembership &&
      s_advapi->FreeSid) {
    if(s_advapi->AllocateAndInitializeSid(&ntauth,
                                            2,
                                            SECURITY_BUILTIN_DOMAIN_RID,
                                            DOMAIN_ALIAS_RID_ADMINS,
                                            0, 0, 0, 0, 0, 0,
                                            &admgroup)) {
      if( !s_advapi->CheckTokenMembership(NULL,
                                          admgroup,
                                          &isadmin) ) {
        /* error occurred? default to false */
        isadmin = FALSE;
      }
      s_advapi->FreeSid(admgroup);
    }
  }
  return isadmin;
}

