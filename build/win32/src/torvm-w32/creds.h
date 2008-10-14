/* Copyright (C) 2008  The Tor Project, Inc.
 * See LICENSE file for rights and terms.
 */
#ifndef __creds_h__
#define __creds_h__

#include "torvm.h"
#include <ntsecpkg.h>
#include <ntsecapi.h>

BOOL setdriversigning (BOOL sigcheck);
BOOL haveadminrights (void);

#endif /* creds_h */
