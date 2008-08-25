# Copyright (C) 2008  The Tor Project, Inc.
# See LICENSE file for rights and terms.

SHELL=/bin/bash
export SHELL
DLCMD=scripts/download.sh

ifeq (,$(ISODIR))
	ISODIR=iso
else
	override ISODIR:=$(realpath $(ISODIR))
endif

# various utilities are downloaded and packaged into an ISO image
# with an autorun.inf that launches a build of all the win32 parts.
# 
ZLIB_F=zlib-1.2.3-bin.zip
ZLIB_URL=http://downloads.sourceforge.net/gnuwin32/zlib-1.2.3-bin.zip?modtime=1122324542&big_mirror=1
ZLIB_SUM=becbcaf5076e307e743b1edc6a5645849eba9ebc

GZIP_F=gzip-1.3.12-1-bin.zip
GZIP_URL=http://downloads.sourceforge.net/gnuwin32/gzip-1.3.12-1-bin.zip?modtime=1213401872&big_mirror=1
GZIP_SUM=69901b7a58e324e39653d4282deaf5ab5bb5c07f

BZIP2_F=bzip2-1.0.5-bin.zip
BZIP2_URL=http://downloads.sourceforge.net/gnuwin32/bzip2-1.0.5-bin.zip?modtime=1206045010&big_mirror=1
BZIP2_SUM=618ba51d3d6838e71a922febf717471b7d5f54e2

LIBARCH_F=libarchive-2.4.12-1-bin.zip
LIBARCH_URL=http://downloads.sourceforge.net/gnuwin32/libarchive-2.4.12-1-bin.zip?modtime=1215347767&big_mirror=1
LIBARCH_SUM=02eb2fe2200705d0067f03780c6d65f51ae69e42

BINUTILS_F=binutils-2.18.50-20080109-2.tar.gz
BINUTILS_URL=http://downloads.sourceforge.net/mingw/binutils-2.18.50-20080109-2.tar.gz?modtime=1206567544&big_mirror=1
BINUTILS_SUM=4a47e75604765f671ca233bb503f1ca67fd93af0

GCCORE_F=gcc-core-3.4.5-20060117-3.tar.gz
GCCORE_URL=http://downloads.sourceforge.net/mingw/gcc-core-3.4.5-20060117-3.tar.gz?modtime=1209046227&big_mirror=1
GCCORE_SUM=1ba77819f5749954fda080c0137af359a09760fc

GPP_F=gcc-g++-3.4.5-20060117-3.tar.gz
GPP_URL=http://downloads.sourceforge.net/mingw/gcc-g%2B%2B-3.4.5-20060117-3.tar.gz?modtime=1209046244&big_mirror=1
GPP_SUM=1d31f990d4027f171ab8c55374bec815a46c16dd

MINGWRUN_F=mingw-runtime-3.14.tar.gz
MINGWRUN_URL=http://downloads.sourceforge.net/mingw/mingw-runtime-3.14.tar.gz?modtime=1198749088&big_mirror=1
MINGWRUN_SUM=ebd523dff5cb5bc476124a283b3ba9781f907fea

MINGWUTIL_F=mingw-utils-0.3.tar.gz
MINGWUTIL_URL=http://downloads.sourceforge.net/mingw/mingw-utils-0.3.tar.gz?modtime=1077362266&big_mirror=1
MINGWUTIL_SUM=7ae32742ece1e89978784634aed824775cf47336

MAKE_F=mingw32-make-3.81-20080326-3.tar.gz
MAKE_URL=http://downloads.sourceforge.net/mingw/mingw32-make-3.81-20080326-3.tar.gz?modtime=1212658958&big_mirror=1
MAKE_SUM=86ab2ddffdd2df6245856bb0dd50fbd8ba372767

W32API_F=w32api-3.11.tar.gz
W32API_URL=http://downloads.sourceforge.net/mingw/w32api-3.11.tar.gz?modtime=1198748420&big_mirror=1
W32API_SUM=0969cde19c3a85012b4924cb49cf2e9ef3d1b6df

MSYS_F=MSYS-1.0.11-20071204.tar.bz2
MSYS_URL=http://downloads.sourceforge.net/mingw/MSYS-1.0.11-20071204.tar.bz2?modtime=1196807934&big_mirror=1
MSYS_SUM=9a1207b04ffaf8880c58de643609e4deed3e1563

MSYSCORE_F=msysCORE-1.0.11-2007.01.19-1.tar.bz2
MSYSCORE_URL=http://downloads.sourceforge.net/mingw/msysCORE-1.0.11-2007.01.19-1.tar.bz2?modtime=1169214215&big_mirror=1
MSYSCORE_SUM=65870650914925dfd8451a9452bff3cdb8320221

BASH_F=bash-3.1-MSYS-1.0.11-1.tar.bz2
BASH_URL=http://downloads.sourceforge.net/mingw/bash-3.1-MSYS-1.0.11-1.tar.bz2?modtime=1195140582&big_mirror=1
BASH_SUM=a440bb5a6e2edfe703b67877c7e7e48504aad6dc

DIFFUTIL_F=diffutils-2.8.7-MSYS-1.0.11-1.tar.bz2
DIFFUTIL_URL=http://downloads.sourceforge.net/mingw/diffutils-2.8.7-MSYS-1.0.11-1.tar.bz2?modtime=1196332674&big_mirror=1
DIFFUTIL_SUM=a34f0470674b6bb807e505839a477f9b9c70facf

LIBTOOL_F=libtool1.5-1.5.25a-1-bin.tar.bz2
LIBTOOL_URL=http://downloads.sourceforge.net/mingw/libtool1.5-1.5.25a-1-bin.tar.bz2?modtime=1190250326&big_mirror=1
LIBTOOL_SUM=7e7206011471dbe79f5298d76ea9530288b2a6dd

MINIRES_F=minires-1.01-1-MSYS-1.0.11-1.tar.bz2
MINIRES_URL=http://downloads.sourceforge.net/mingw/minires-1.01-1-MSYS-1.0.11-1.tar.bz2?modtime=1206482214&big_mirror=1
MINIRES_SUM=c077e08cce446656ff0c0b3f30a327652f9c9986

MSYSZLIB_F=zlib-1.2.3-MSYS-1.0.11-1.tar.bz2
MSYSZLIB_URL=http://downloads.sourceforge.net/mingw/zlib-1.2.3-MSYS-1.0.11-1.tar.bz2?modtime=1206455079&big_mirror=1
MSYSZLIB_SUM=dff1a110b165d7ba4d7804b1c2f4a023823eec90

SSL_F=openssl-0.9.8g-1-MSYS-1.0.11-2-dll098.tar.gz
SSL_URL=http://downloads.sourceforge.net/mingw/openssl-0.9.8g-1-MSYS-1.0.11-2-dll098.tar.gz?modtime=1206734099&big_mirror=1
SSL_SUM=eba5f37fdec2982727556a3dbb5ea6fd04296b76

SSH_F=openssh-4.7p1-MSYS-1.0.11-1-bin.tar.gz
SSH_URL=http://downloads.sourceforge.net/mingw/openssh-4.7p1-MSYS-1.0.11-1-bin.tar.gz?modtime=1206734972&big_mirror=1
SSH_SUM=bf1617bf43de198cea7c197621494baec9520bcb

ZLIBSRC_F=zlib-1.2.3.tar.gz
ZLIBSRC_URL=http://www.zlib.net/zlib-1.2.3.tar.gz
ZLIBSRC_SUM=60faeaaf250642db5c0ea36cd6dcc9f99c8f3902

PTHREADS_F=pthreads-w32-2-8-0-release.tar.gz
PTHREADS_URL=ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-8-0-release.tar.gz
PTHREADS_SUM=da8371cb20e8e238f96a1d0651212f154d84a9ac

SDL_F=SDL-1.2.13.tar.gz
SDL_URL=http://www.libsdl.org/release/SDL-1.2.13.tar.gz
SDL_SUM=51fcaa3e1d5c01fd813ea08688780f86b19cf539

WPCAP_F=WinPcap_4_1_beta4.exe
WPCAP_URL=http://www.winpcap.org/install/bin/WinPcap_4_1_beta4.exe
WPCAP_SUM=25a56a70f9f285bc029220393f01177da5b02bb6

WPCAPRT_F=WinPcap-4.1-files.tar.gz
WPCAPRT_URL=http://peertech.org/files/WinPcap-4.1-files.tar.gz
WPCAPRT_SUM=57d2eab8b1afcfdf65101849cfe4e4110ee3f121

WPD_F=WpdPack_4_1_beta4.zip
WPD_URL=http://www.winpcap.org/install/bin/WpdPack_4_1_beta4.zip
WPD_SUM=fa5255dc1a36ff47999d5b60aeadcf8ff9fb44cd

QEMU_F=qemu-0.9.1.tar.gz
QEMU_URL=http://bellard.org/qemu/qemu-0.9.1.tar.gz
QEMU_SUM=630ea20b5989f0df00128de7f7be661c573ed041


ALLFILES=$(ZLIB_F) $(GZIP_F) $(BZIP2_F) $(LIBARCH_F) $(BINUTILS_F) $(GCCORE_F) $(GPP_F) $(MINGWRUN_F) $(MINGWUTIL_F) $(MAKE_F) $(W32API_F) $(MSYS_F) $(MSYSCORE_F) $(BASH_F) $(DIFFUTIL_F) $(LIBTOOL_F) $(MINIRES_F) $(MSYSZLIB_F) $(SSL_F) $(SSH_F) $(ZLIBSRC_F) $(PTHREADS_F) $(SDL_F) $(WPCAP_F) $(WPCAPRT_F) $(WPD_F) $(QEMU_F)

default all: downloads extract buildiso runinstall

downloads: Makefile
	@if [ ! -d $(WDLDIR) ]; then mkdir -p $(WDLDIR); fi
	@if [ ! -e $(WDLDIR)/$(ZLIB_F) ]; then $(DLCMD) "$(ZLIB_URL)" $(ZLIB_SUM) "$(WDLDIR)/$(ZLIB_F)"; fi
	@if [ ! -e $(WDLDIR)/$(GZIP_F) ]; then $(DLCMD) "$(GZIP_URL)" $(GZIP_SUM) "$(WDLDIR)/$(GZIP_F)"; fi
	@if [ ! -e $(WDLDIR)/$(BZIP2_F) ]; then $(DLCMD) "$(BZIP2_URL)" $(BZIP2_SUM) "$(WDLDIR)/$(BZIP2_F)"; fi
	@if [ ! -e $(WDLDIR)/$(LIBARCH_F) ]; then $(DLCMD) "$(LIBARCH_URL)" $(LIBARCH_SUM) "$(WDLDIR)/$(LIBARCH_F)"; fi
	@if [ ! -e $(WDLDIR)/$(BINUTILS_F) ]; then $(DLCMD) "$(BINUTILS_URL)" $(BINUTILS_SUM) "$(WDLDIR)/$(BINUTILS_F)"; fi
	@if [ ! -e $(WDLDIR)/$(GCCORE_F) ]; then $(DLCMD) "$(GCCORE_URL)" $(GCCORE_SUM) "$(WDLDIR)/$(GCCORE_F)"; fi
	@if [ ! -e $(WDLDIR)/$(GPP_F) ]; then $(DLCMD) "$(GPP_URL)" $(GPP_SUM) "$(WDLDIR)/$(GPP_F)"; fi
	@if [ ! -e $(WDLDIR)/$(MINGWRUN_F) ]; then $(DLCMD) "$(MINGWRUN_URL)" $(MINGWRUN_SUM) "$(WDLDIR)/$(MINGWRUN_F)"; fi
	@if [ ! -e $(WDLDIR)/$(MINGWUTIL_F) ]; then $(DLCMD) "$(MINGWUTIL_URL)" $(MINGWUTIL_SUM) "$(WDLDIR)/$(MINGWUTIL_F)"; fi
	@if [ ! -e $(WDLDIR)/$(MAKE_F) ]; then $(DLCMD) "$(MAKE_URL)" $(MAKE_SUM) "$(WDLDIR)/$(MAKE_F)"; fi
	@if [ ! -e $(WDLDIR)/$(W32API_F) ]; then $(DLCMD) "$(W32API_URL)" $(W32API_SUM) "$(WDLDIR)/$(W32API_F)"; fi
	@if [ ! -e $(WDLDIR)/$(MSYS_F) ]; then $(DLCMD) "$(MSYS_URL)" $(MSYS_SUM) "$(WDLDIR)/$(MSYS_F)"; fi
	@if [ ! -e $(WDLDIR)/$(MSYSCORE_F) ]; then $(DLCMD) "$(MSYSCORE_URL)" $(MSYSCORE_SUM) "$(WDLDIR)/$(MSYSCORE_F)"; fi
	@if [ ! -e $(WDLDIR)/$(BASH_F) ]; then $(DLCMD) "$(BASH_URL)" $(BASH_SUM) "$(WDLDIR)/$(BASH_F)"; fi
	@if [ ! -e $(WDLDIR)/$(DIFFUTIL_F) ]; then $(DLCMD) "$(DIFFUTIL_URL)" $(DIFFUTIL_SUM) "$(WDLDIR)/$(DIFFUTIL_F)"; fi
	@if [ ! -e $(WDLDIR)/$(LIBTOOL_F) ]; then $(DLCMD) "$(LIBTOOL_URL)" $(LIBTOOL_SUM) "$(WDLDIR)/$(LIBTOOL_F)"; fi
	@if [ ! -e $(WDLDIR)/$(MINIRES_F) ]; then $(DLCMD) "$(MINIRES_URL)" $(MINIRES_SUM) "$(WDLDIR)/$(MINIRES_F)"; fi
	@if [ ! -e $(WDLDIR)/$(MSYSZLIB_F) ]; then $(DLCMD) "$(MSYSZLIB_URL)" $(MSYSZLIB_SUM) "$(WDLDIR)/$(MSYSZLIB_F)"; fi
	@if [ ! -e $(WDLDIR)/$(SSL_F) ]; then $(DLCMD) "$(SSL_URL)" $(SSL_SUM) "$(WDLDIR)/$(SSL_F)"; fi
	@if [ ! -e $(WDLDIR)/$(SSH_F) ]; then $(DLCMD) "$(SSH_URL)" $(SSH_SUM) "$(WDLDIR)/$(SSH_F)"; fi
	@if [ ! -e $(WDLDIR)/$(ZLIBSRC_F) ]; then $(DLCMD) "$(ZLIBSRC_URL)" $(ZLIBSRC_SUM) "$(WDLDIR)/$(ZLIBSRC_F)"; fi
	@if [ ! -e $(WDLDIR)/$(PTHREADS_F) ]; then $(DLCMD) "$(PTHREADS_URL)" $(PTHREADS_SUM) "$(WDLDIR)/$(PTHREADS_F)"; fi
	@if [ ! -e $(WDLDIR)/$(SDL_F) ]; then $(DLCMD) "$(SDL_URL)" $(SDL_SUM) "$(WDLDIR)/$(SDL_F)"; fi
	@if [ ! -e $(WDLDIR)/$(WPCAP_F) ]; then $(DLCMD) "$(WPCAP_URL)" $(WPCAP_SUM) "$(WDLDIR)/$(WPCAP_F)"; fi
	@if [ ! -e $(WDLDIR)/$(WPCAPRT_F) ]; then $(DLCMD) "$(WPCAPRT_URL)" $(WPCAPRT_SUM) "$(WDLDIR)/$(WPCAPRT_F)"; fi
	@if [ ! -e $(WDLDIR)/$(WPD_F) ]; then $(DLCMD) "$(WPD_URL)" $(WPD_SUM) "$(WDLDIR)/$(WPD_F)"; fi
	@if [ ! -e $(WDLDIR)/$(QEMU_F) ]; then $(DLCMD) "$(QEMU_URL)" $(QEMU_SUM) "$(WDLDIR)/$(QEMU_F)"; fi

extract: downloads $(addprefix $(WDLDIR)/, $(ALLFILES))
	@if [ -d $(ISODIR) ]; then \
		echo "Removing previous ISO build directory."; \
		rm -rf $(ISODIR) >/dev/null 2>&1; \
	fi; \
	mkdir -p $(ISODIR)/dl/src; \
	mkdir $(ISODIR)/bin; \
	mkdir $(ISODIR)/add; \
	if [ -e ../iso/vm.iso ]; then \
		cp ../iso/vm.iso $(ISODIR)/add; \
	fi; \
	if [ -e ../iso/vm-isosrc/kernels/boot ]; then \
		cp ../iso/vm-isosrc/kernels/boot $(ISODIR)/add/vmlinuz; \
	fi; \
	cp files/autorun.inf files/install.bat $(ISODIR)/; \
	cp files/hdd.img.gz $(ISODIR)/add/; \
	cp files/run.bat $(ISODIR)/add/; \
	gzip -d $(ISODIR)/add/hdd.img.gz; \
	cp files/fstab $(ISODIR)/bin/; \
	cp files/buildall.sh $(ISODIR)/dl/src; \
	cp patches/* $(ISODIR)/dl/src; \
	unzip $(WDLDIR)/$(ZLIB_F) -d $(ISODIR)/ bin/zlib1.dll; \
	unzip $(WDLDIR)/$(GZIP_F) -d $(ISODIR)/ bin/gzip.exe; \
	unzip $(WDLDIR)/$(BZIP2_F) -d $(ISODIR)/ bin/bzip2.exe bin/bzip2.dll; \
	tar jxf $(WDLDIR)/$(BASH_F) bin/bash.exe bin/sh.exe; mv bin/* $(ISODIR)/bin/;rmdir bin; \
	tar jxf $(WDLDIR)/$(MSYS_F) bin/msys-1.0.dll bin/mount.exe bin/ps.exe; mv bin/* $(ISODIR)/bin/;rmdir bin; \
	unzip $(WDLDIR)/$(LIBARCH_F) -d $(ISODIR)/ bin/bsdtar.exe bin/libarchive2.dll; \
	cp $(addprefix $(WDLDIR)/, $(ALLFILES)) $(ISODIR)/dl/; \
	unzip $(WDLDIR)/$(WPD_F); tar cf $(ISODIR)/dl/src/WpdPack_4_1_beta4.tar WpdPack; gzip -9 $(ISODIR)/dl/src/WpdPack_4_1_beta4.tar; rm -rf WpdPack; \
	mv $(ISODIR)/dl/$(WPCAPRT_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(PTHREADS_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(QEMU_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(SDL_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(ZLIBSRC_F) $(ISODIR)/dl/src/; \
	touch $(ISODIR)/VMDEVISO.TXT

buildiso: extract
	@if [ -f w32build.iso ]; then \
		rm -f w32build.iso; \
	fi; \
	export ODIR=`pwd`; \
	( 	cd $(ISODIR); \
		mkisofs -hide-rr-moved -pad -l -o $$ODIR/w32build.iso -r -J .; \
	); if (( $$? != 0 )); then exit 1; fi

runinstall: buildiso
	@echo "NOTICE: This feature is not yet automated.  Please load the ISO into a Windows systems to start the build."

.PHONY: downloads extract buildiso runinstall
