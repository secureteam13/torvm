#!/bin/bash
# Copyright (C) 2008  The Tor Project, Inc.
# See LICENSE file for rights and terms.

if (( $# != 3 )); then
  echo "Usage: `basename $0` SrcURL SHA1 DestPath" >&2
  exit 1
fi
SRCURL="$1"
SUMEXPECTED="$2"
SAVEAS="$3"
DLTMP="${SAVEAS}.dltmp"

# get an sha1 digest using sha1sum or gpg and store in $SHA1OUT
export ZEROSHA1=da39a3ee5e6b4b0d3255bfef95601890afd80709
cmdsum () {
  sha1sum=`which sha1sum`
  if (( $? != 0 )); then
    return 1
  fi
  SHA1OUT=`$sha1sum "$1" | sed 's/ .*//'`
  return 0
}

gpgsum () {
  gpgbin=`which gpg`
  if (( $? != 0 )); then
    return 1
  fi
  SHA1OUT=`$gpgbin --print-md sha1 "$1" 2>/dev/null | sed 's/.*: //' | sed 's/[^0-9A-F]//g' | tr -t '[:upper:]' '[:lower:]'`
  return 0
}

dfunc=
cmdsum /dev/null
if (( $? == 0 )); then
  if [[ "$SHA1OUT" == "$ZEROSHA1" ]]; then
    dfunc=cmdsum
  fi
fi
if [ -z "$dfunc" ]; then
  gpgsum /dev/null
  if (( $? == 0 )); then
    if [[ "$SHA1OUT" == "$ZEROSHA1" ]]; then
      dfunc=gpgsum
    fi
  fi 
fi
if [ -z "$dfunc" ]; then
  echo "ERROR: Unable to find suitable sha1sum utility.  Please install sha1sum or gpg." >&2
  exit 1
fi

echo "Retrieving $SRCURL ..."
wget --no-check-certificate -t5 --timeout=20 $WGET_OPTIONS -O "$DLTMP" "$SRCURL"
if (( $? != 0 )); then
  echo "ERROR: Could not retrieve file $SRCURL" >&2
  if [ -f "$DLTMP" ]; then
    rm -f "$DLTMP"
  fi
  exit 1
fi
$dfunc "$DLTMP"
if [[ "$SHA1OUT" != "$SUMEXPECTED" ]]; then
  echo "ERROR: Digest for file `basename $DLTMP` does not match." >&2
  echo "       Expected $SUMEXPECTED but got $SHA1OUT instead." >&2
  rm -f "$DLTMP"
  exit 1
fi
mv "$DLTMP" "$SAVEAS"
echo "SHA-1 Digest verified OK for `basename $SAVEAS`"
exit 0
