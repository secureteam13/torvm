# Copyright (C) 2008  The Tor Project, Inc.
# See LICENSE file for rights and terms.

SHELL=/bin/bash
export SHELL

# setup your own local defaults outside svn or dist files here
# include local.mk

# set various build defaults
ifeq (,$(BUSER))
	BUSER=guest
endif
ifeq (,$(BGROUP))
	BGROUP=users
endif
ifeq (,$(TGTNAME))
	TGTNAME=x86-uclibc-vm
endif
ifeq (,$(DLDIR))
	DLDIR=./build/kamikaze/common/dl
endif
ifeq (,$(WDLDIR))
	WDLDIR=./build/win32/dl
endif
override DLDIR:=$(realpath $(DLDIR))
override WDLDIR:=$(realpath $(WDLDIR))

# OpenWRT version for build
override CVER:=11833

export BUSER
export BGROUP
export TGTNAME
export DLDIR
export WDLDIR

default all: prereq import buildtree buildkern buildvmiso buildw32src package

#XXX move this into configure
prereq: Makefile
	# check DL paths always, since these are more volatile than build user and tools
	@if [ ! -d $(DLDIR) ]; then \
		echo "Error: invalid DLDIR path given."; \
		echo "directory \"$(DLDIR)\" does not exist."; \
		exit 1; \
	fi;
	@if [ ! -d $(WDLDIR) ]; then \
		echo "Error: invalid WDLDIR path given."; \
		echo "directory \"$(WDLDIR)\" does not exist."; \
		exit 1; \
	fi;
	@if [ ! -f .build_prereqs_verified ]; then \
		echo "Verifying build prerequisites ..." >&2; \
		NOFOUND=""; \
		REQS="make gcc g++ gawk bison flex unzip bzip2 patch perl wget tar svn autoconf mkisofs sha1sum"; \
		for REQ in $$REQS; do \
			which $$REQ >/dev/null 2>&1; \
			if (( $$? != 0 )); then \
				export NOFOUND="$$REQ $$NOFOUND"; \
			fi; \
		done; \
		if [ "$$NOFOUND" != "" ]; then \
			echo "ERROR: Unable to locate the following mandatory applications: $$NOFOUND" >&2; \
			exit 1; \
		fi; \
		touch .test >/dev/null 2>&1 ; \
		if (( $$? != 0 )); then \
			echo "ERROR: Current build directory does not appear writable." >&2; \
			exit 1; \
		fi; \
		chown $(BUSER):$(BGROUP) .test >/dev/null 2>&1; \
		if (( $$? != 0 )); then \
			echo "ERROR: Build user $(BUSER) and group $(BGROUP) do not appear valid." >&2; \
			echo "       Please verify the settings you configured and set BUSER/BGROUP accordingly." >&2; \
			exit 1; \
		fi; \
		rm -f .test >/dev/null 2>&1; \
		touch .build_prereqs_verified; \
	fi

import: prereq
	@if [ ! -d import/kamikaze ]; then \
		echo "Checking out OpenWRT subversion tree revision $(CVER) ..." >&2; \
		cd import; \
		svn co -r $(CVER) https://svn.openwrt.org/openwrt/trunk/ kamikaze ; \
		if (( $$? != 0 )); then \
			echo "ERROR: Unable to download a copy of the OpenWRT subversion tree." >&2; \
			rm -rf kamikaze; \
			exit 1; \
		fi; \
	fi

buildtree: import
	@if [ ! -d build/kamikaze/$(TGTNAME) ]; then \
		echo "Creating Tor VM build tree ..."; \
		cd build/kamikaze; \
		svn export ../../import/kamikaze $(TGTNAME); \
		if (( $$? != 0 )); then \
			echo "ERROR: Unable to export working copy of local OpenWRT tree." >&2; \
			rm -rf $(TGTNAME); \
			exit 1; \
		fi; \
		cd $(TGTNAME); \
		for PFILE in $$(ls ../patches/); do \
			patch -p1 < ../patches/$$PFILE; \
			if (( $$? != 0 )); then \
				echo "ERROR: Unable to apply patch $$PFILE." >&2; \
				cd ..; \
				rm -rf $(TGTNAME); \
				exit 1; \
			fi; \
		done; \
		if [ -d $(DLDIR) ]; then \
			ln -s $(DLDIR) ./dl; \
		fi; \
		chown -R $(BUSER):$(BGROUP) . ; \
	fi

buildkern: buildtree
	@cd build/kamikaze/$(TGTNAME); \
	time su $(BUSER) -c "( $(MAKE) V=99 oldconfig && $(MAKE) V=99 world )"; \
	if (( $$? != 0 )); then \
		echo "ERROR: OpenWRT kernel build failed.  Check log for details." >&2; \
		exit 1; \
	fi

buildvmiso: buildkern
	@cd build/iso; \
	./buildiso; \
	if (( $$? != 0 )); then \
		echo "ERROR: Unable to create bootable ISO image." >&2; \
		exit 1; \
	fi


W32MK=WDLDIR=$(WDLDIR) all
ifneq (,$(BUILD_SCP_USER))
  W32MK:=BUILD_SCP_USER=$(BUILD_SCP_USER) BUILD_SCP_IDF=$(BUILD_SCP_IDF) BUILD_SCP_HOST=$(BUILD_SCP_HOST) BUILD_SCP_DIR=$(BUILD_SCP_DIR) $(W32MK)
endif
ifeq (TRUE,$(AUTO_SHUTDOWN))
  W32MK:=AUTO_SHUTDOWN=TRUE $(W32MK)
endif
ifneq (TRUE,$(DEBUG_NO_STRIP))
  W32MK:=DEBUG_NO_STRIP=TRUE $(W32MK)
endif
ifneq (,$(W32AUTO_BUILD_CMD))
  W32MK:=W32AUTO_BUILD_CMD="$(W32AUTO_BUILD_CMD)" $(W32MK)
endif

buildw32src: 
	@cd build/win32; \
	chown -R $(BUSER):$(BGROUP) . ; \
	time su $(BUSER) -c "( $(MAKE) $(W32MK) )"; \
	if (( $$? != 0 )); then \
		echo "ERROR: Unable to create win32 build ISO image." >&2; \
		exit 1; \
	fi

package: buildw32src buildvmiso
	@echo "Automated packaging of completed win32 build is not yet implemented."

.PHONY: clean prereq import buildkern buildvmiso buildw32src package

