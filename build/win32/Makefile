# Copyright (C) 2008-2009  The Tor Project, Inc.
# See LICENSE file for rights and terms.

SHELL=/bin/bash
export SHELL
DLCMD=scripts/download.sh

ifeq (,$(ISODIR))
	ISODIR=iso
else
	override ISODIR:=$(realpath $(ISODIR))
endif
override UNZIP:=unzip -qq

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

MSYSCRYPT_F=crypt-1.1-1-MSYS-1.0.11-1.tar.bz2
MSYSCRYPT_URL=http://downloads.sourceforge.net/mingw/crypt-1.1-1-MSYS-1.0.11-1.tar.bz2?modtime=1206481779&big_mirror=1
MSYSCRYPT_SUM=072e9a158d41e73aa5a766df016494dab74085e4

M4_F=m4-1.4.7-MSYS.tar.bz2
M4_URL=http://prdownloads.sourceforge.net/mingw/m4-1.4.7-MSYS.tar.bz2?download
M4_SUM=a00ed987824456946777d92f600612f344211dcc

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

ACONF_F=autoconf2.5-2.61-1-bin.tar.bz2
ACONF_URL=http://downloads.sourceforge.net/mingw/autoconf2.5-2.61-1-bin.tar.bz2?modtime=1190249465&big_mirror=1
ACONF_SUM=7f05699dfeb0fea12b490da7204eeb89644a3bf7

AMAKE_F=automake1.9-1.9.6-2-bin.tar.bz2
AMAKE_URL=http://downloads.sourceforge.net/mingw/automake1.9-1.9.6-2-bin.tar.bz2?modtime=1190249841&big_mirror=1
AMAKE_SUM=685dd354868ebb6fca148a300f20b55e1c036fca

PERL_F=perl-5.6.1-MSYS-1.0.11-1.tar.bz2
PERL_URL=http://downloads.sourceforge.net/mingw/perl-5.6.1-MSYS-1.0.11-1.tar.bz2?modtime=1206455338&big_mirror=1
PERL_SUM=ff1f07a061c7958bc9043f0085a55ff0101b1be4

GROFF_F=groff-1.19.2.tar.gz
GROFF_URL=http://ftp.gnu.org/gnu/groff/groff-1.19.2.tar.gz
GROFF_SUM=cfad99a5c52933a6a9180e394bd94fbaec0ac956

CMAKE_F=cmake-2.6.2.tar.gz
CMAKE_URL=http://www.cmake.org/files/v2.6/cmake-2.6.2.tar.gz
CMAKE_SUM=48d33afaf23d40f2ea9aa2f5018c7213983222c2

SSLSRC_F=openssl-0.9.8j.tar.gz
SSLSRC_URL=http://www.openssl.org/source/openssl-0.9.8j.tar.gz
SSLSRC_SUM=f70f7127a26e951e8a0d854c0c9e6b4c24df78e4

PY2EXE_F=py2exe-0.6.9.zip
PY2EXE_TGZ=py2exe.tar.gz
PY2EXE_URL=http://downloads.sourceforge.net/py2exe/py2exe-0.6.9.zip?modtime=1226789941&big_mirror=1
PY2EXE_SUM=f1a2cdfbf6d142f6ff54edfcf9d127e2d07169d5

WIX_F=wix3.0.4721.0-binaries.zip
WIX_TGZ=wix.tar.gz
WIX_URL=http://downloads.sourceforge.net/wix/wix3.0.4721.0-binaries.zip?modtime=1227270606&big_mirror=1
WIX_SUM=3035442aeab28a01f1cb032c610f375f763a0f88

GNURX_F=mingw-libgnurx-2.5.1-src.tar.gz
GNURX_URL=http://downloads.sourceforge.net/mingw/mingw-libgnurx-2.5.1-src.tar.gz?modtime=1178542293&big_mirror=1
GNURX_SUM=f1e4af2541645dac82362b618aaa849658cd4988

NSIS_VER=2.42
NSIS_F=nsis-$(NSIS_VER).zip
NSIS_URL=http://downloads.sourceforge.net/nsis/nsis-2.42.zip?modtime=1229784735&big_mirror=1
NSIS_SUM=a1aef12a33f16273ac9109fde28713399f6a8016
NSIS_TGZ=nsis-2.42.tar.gz

7ZIP_VER=464
7ZIP_F=7z464.msi
7ZIP_URL=http://downloads.sourceforge.net/sevenzip/7z464.msi?modtime=1230949842&big_mirror=1
7ZIP_SUM=19a06e188a83452dc9273290dbef71220686c4c1
7ZIP_NAME=7zip.msi

SDL_F=SDL-1.2.13.tar.gz
SDL_URL=http://www.libsdl.org/release/SDL-1.2.13.tar.gz
SDL_SUM=51fcaa3e1d5c01fd813ea08688780f86b19cf539

WPCAPSRC_F=WpcapSrc_4_1_beta4.zip
WPCAPSRC_URL=http://www.winpcap.org/install/bin/WpcapSrc_4_1_beta4.zip
WPCAPSRC_SUM=1450c4d293a38c25ec8cd8a9be30e04858df36d9

QEMU_F=qemu-0.9.1.tar.gz
QEMU_URL=http://bellard.org/qemu/qemu-0.9.1.tar.gz
QEMU_SUM=630ea20b5989f0df00128de7f7be661c573ed041

OVPN_F=openvpn-2.1_rc10.tar.gz
OVPN_URL=http://openvpn.net/release/openvpn-2.1_rc10.tar.gz
OVPN_SUM=b7f4af94704caa8aa3279ccbc391b98cf7ed953e

WINDDK_F=1830_usa_ddk.iso
WINDDK_URL=http://download.microsoft.com/download/9/0/f/90f019ac-8243-48d3-91cf-81fc4093ecfd/1830_usa_ddk.iso
WINDDK_SUM=0d2154d88a5ee252cc908630c77863bb42777387

QT_VER=4.4.3
QT_F=qt-win-opensource-src-$(QT_VER).zip
QT_TGZ=qt-$(QT_VER).tgz
QT_URL=ftp://ftp.trolltech.com/qt/source/qt-win-opensource-src-$(QT_VER).zip
QT_SUM=669d494b9c37353f87c13765ed9f3ea97c0fa9e0

POLIPO_F=polipo-20080907.tar.gz
POLIPO_URL=http://www.pps.jussieu.fr/~jch/software/files/polipo/polipo-20080907.tar.gz
POLIPO_SUM=dda211e3027310891fb555c1fe5fa2f5b274aa0f

MARBLE_NAME=marble-latest
MARBLE_TGZ=$(MARBLE_NAME).tar.gz
MARBLE_SVN=https://svn.vidalia-project.net/svn/marble/
MARBLE_BR=trunk
MARBLE_VER=3516

VIDALIA_NAME=vidalia-latest
VIDALIA_TGZ=$(VIDALIA_NAME).tar.gz
VIDALIA_SVN=https://svn.vidalia-project.net/svn/vidalia/
VIDALIA_BR=trunk
VIDALIA_VER=3520

# for package resources
TORSVN_NAME=tor-latest
TORSVN_TGZ=$(TORSVN_NAME).tar.gz
TORSVN_SVN=https://tor-svn.freehaven.net/svn/tor/
TORSVN_BR=trunk
TORSVN_VER=18546

TORBUTTON_NAME=torbutton-latest
TORBUTTON_TGZ=$(TORBUTTON_NAME).tar.gz
TORBUTTON_SVN=https://tor-svn.freehaven.net/svn/torbutton/
TORBUTTON_BR=trunk
TORBUTTON_VER=18548

PYCRYPTO_NAME=pycrypto-latest
PYCRYPTO_TGZ=$(PYCRYPTO_NAME).tar.gz
PYCRYPTO_GIT=git://git.pycrypto.org:9419/crypto/pycrypto-2.x.git

THANDY_NAME=thandy-latest
THANDY_TGZ=$(THANDY_NAME).tar.gz
THANDY_GIT=git://git.torproject.org/git/thandy

# XXX: tmp equivalent of exit 0 when chaining cmds
# buildscripts need some dire clean up...
OK:=echo -n

ALLFILES=$(ZLIB_F) $(GZIP_F) $(BZIP2_F) $(LIBARCH_F) $(BINUTILS_F) $(GCCORE_F) $(GPP_F) $(MINGWRUN_F) $(MINGWUTIL_F) $(MAKE_F) $(W32API_F) $(MSYS_F) $(MSYSCORE_F) $(BASH_F) $(DIFFUTIL_F) $(LIBTOOL_F) $(MINIRES_F) $(MSYSZLIB_F) $(MSYSCRYPT_F) $(M4_F) $(SSL_F) $(SSH_F) $(ZLIBSRC_F) $(PTHREADS_F) $(AMAKE_F) $(ACONF_F) $(PERL_F) $(GROFF_F) $(CMAKE_F) $(QT_F) $(NSIS_F) $(7ZIP_F) $(SSLSRC_F) $(PY2EXE_F) $(WIX_F) $(GNURX_F) $(POLIPO_F) $(SDL_F) $(WPCAPSRC_F) $(QEMU_F) $(OVPN_F)

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
	@if [ ! -e $(WDLDIR)/$(MSYSCRYPT_F) ]; then $(DLCMD) "$(MSYSCRYPT_URL)" $(MSYSCRYPT_SUM) "$(WDLDIR)/$(MSYSCRYPT_F)"; fi
	@if [ ! -e $(WDLDIR)/$(M4_F) ]; then $(DLCMD) "$(M4_URL)" $(M4_SUM) "$(WDLDIR)/$(M4_F)"; fi
	@if [ ! -e $(WDLDIR)/$(SSL_F) ]; then $(DLCMD) "$(SSL_URL)" $(SSL_SUM) "$(WDLDIR)/$(SSL_F)"; fi
	@if [ ! -e $(WDLDIR)/$(SSH_F) ]; then $(DLCMD) "$(SSH_URL)" $(SSH_SUM) "$(WDLDIR)/$(SSH_F)"; fi
	@if [ ! -e $(WDLDIR)/$(ZLIBSRC_F) ]; then $(DLCMD) "$(ZLIBSRC_URL)" $(ZLIBSRC_SUM) "$(WDLDIR)/$(ZLIBSRC_F)"; fi
	@if [ ! -e $(WDLDIR)/$(PTHREADS_F) ]; then $(DLCMD) "$(PTHREADS_URL)" $(PTHREADS_SUM) "$(WDLDIR)/$(PTHREADS_F)"; fi
	@if [ ! -e $(WDLDIR)/$(ACONF_F) ]; then $(DLCMD) "$(ACONF_URL)" $(ACONF_SUM) "$(WDLDIR)/$(ACONF_F)"; fi
	@if [ ! -e $(WDLDIR)/$(AMAKE_F) ]; then $(DLCMD) "$(AMAKE_URL)" $(AMAKE_SUM) "$(WDLDIR)/$(AMAKE_F)"; fi
	@if [ ! -e $(WDLDIR)/$(PERL_F) ]; then $(DLCMD) "$(PERL_URL)" $(PERL_SUM) "$(WDLDIR)/$(PERL_F)"; fi
	@if [ ! -e $(WDLDIR)/$(GROFF_F) ]; then $(DLCMD) "$(GROFF_URL)" $(GROFF_SUM) "$(WDLDIR)/$(GROFF_F)"; fi
	@if [ ! -e $(WDLDIR)/$(CMAKE_F) ]; then $(DLCMD) "$(CMAKE_URL)" $(CMAKE_SUM) "$(WDLDIR)/$(CMAKE_F)"; fi
	@if [ ! -e $(WDLDIR)/$(QT_F) ]; then $(DLCMD) "$(QT_URL)" $(QT_SUM) "$(WDLDIR)/$(QT_F)"; fi
	@if [ ! -e $(WDLDIR)/$(NSIS_F) ]; then $(DLCMD) "$(NSIS_URL)" $(NSIS_SUM) "$(WDLDIR)/$(NSIS_F)"; fi
	@if [ ! -e $(WDLDIR)/$(7ZIP_F) ]; then $(DLCMD) "$(7ZIP_URL)" $(7ZIP_SUM) "$(WDLDIR)/$(7ZIP_F)"; fi
	@if [ ! -e $(WDLDIR)/$(SSLSRC_F) ]; then $(DLCMD) "$(SSLSRC_URL)" $(SSLSRC_SUM) "$(WDLDIR)/$(SSLSRC_F)"; fi
	@if [ ! -e $(WDLDIR)/$(PY2EXE_F) ]; then $(DLCMD) "$(PY2EXE_URL)" $(PY2EXE_SUM) "$(WDLDIR)/$(PY2EXE_F)"; fi
	@if [ ! -e $(WDLDIR)/$(WIX_F) ]; then $(DLCMD) "$(WIX_URL)" $(WIX_SUM) "$(WDLDIR)/$(WIX_F)"; fi
	@if [ ! -e $(WDLDIR)/$(GNURX_F) ]; then $(DLCMD) "$(GNURX_URL)" $(GNURX_SUM) "$(WDLDIR)/$(GNURX_F)"; fi
	@if [ ! -e $(WDLDIR)/$(POLIPO_F) ]; then $(DLCMD) "$(POLIPO_URL)" $(POLIPO_SUM) "$(WDLDIR)/$(POLIPO_F)"; fi
	@if [ ! -e $(WDLDIR)/$(SDL_F) ]; then $(DLCMD) "$(SDL_URL)" $(SDL_SUM) "$(WDLDIR)/$(SDL_F)"; fi
	@if [ ! -e $(WDLDIR)/$(WPCAPSRC_F) ]; then $(DLCMD) "$(WPCAPSRC_URL)" $(WPCAPSRC_SUM) "$(WDLDIR)/$(WPCAPSRC_F)"; fi
	@if [ ! -e $(WDLDIR)/$(OVPN_F) ]; then $(DLCMD) "$(OVPN_URL)" $(OVPN_SUM) "$(WDLDIR)/$(OVPN_F)"; fi
	@if [ ! -e $(WDLDIR)/$(QEMU_F) ]; then $(DLCMD) "$(QEMU_URL)" $(QEMU_SUM) "$(WDLDIR)/$(QEMU_F)"; fi
	@( cd $(SDLDIR); if [ -d $(TORSVN_NAME) ]; then cd $(TORSVN_NAME); svn up; $(OK); else svn co "$(TORSVN_SVN)$(TORSVN_BR)" $(TORSVN_NAME) ; fi )
	@( cd $(SDLDIR); if [ -d $(TORBUTTON_NAME) ]; then cd $(TORBUTTON_NAME); svn up; $(OK); else svn co "$(TORBUTTON_SVN)$(TORBUTTON_BR)" $(TORBUTTON_NAME) ; fi )
	@( cd $(SDLDIR); if [ -d $(VIDALIA_NAME) ]; then cd $(VIDALIA_NAME); svn up; $(OK); else svn co "$(VIDALIA_SVN)$(VIDALIA_BR)" $(VIDALIA_NAME) ; fi )
	@( cd $(SDLDIR); if [ -d $(MARBLE_NAME) ]; then cd $(MARBLE_NAME); svn up; $(OK); else svn co "$(MARBLE_SVN)$(MARBLE_BR)" $(MARBLE_NAME) ; fi )
	@( cd $(SDLDIR); if [ -d $(THANDY_NAME) ]; then cd $(THANDY_NAME); git pull; $(OK); else git clone $(THANDY_GIT) $(THANDY_NAME); fi ) 
	@( cd $(SDLDIR); if [ -d $(PYCRYPTO_NAME) ]; then cd $(PYCRYPTO_NAME); git pull; $(OK); else git clone $(PYCRYPTO_GIT) $(PYCRYPTO_NAME); fi ) 

extract: downloads $(addprefix $(WDLDIR)/, $(ALLFILES))
	@if [ -d $(ISODIR) ]; then \
		echo "Removing previous ISO build directory."; \
		rm -rf $(ISODIR) >/dev/null 2>&1; \
	fi; \
	mkdir -p $(ISODIR)/dl/src; \
	mkdir $(ISODIR)/bin; \
	mkdir $(ISODIR)/add; \
	if [ -e ../kamikaze/x86-uclibc-vm/bin/openwrt-x86-vmlinuz ]; then \
		cp ../kamikaze/x86-uclibc-vm/bin/openwrt-x86-vmlinuz $(ISODIR)/add/vmlinuz; \
	fi; \
	if [ -e $(BUILD_SCP_IDF) ]; then \
		mkdir $(ISODIR)/ssh; \
		cp "$(BUILD_SCP_IDF)" $(ISODIR)/ssh/; \
		echo "$(BUILD_SCP_USER)" > $(ISODIR)/ssh/user; \
		echo "$(BUILD_SCP_HOST)" > $(ISODIR)/ssh/host; \
		echo "$(BUILD_SCP_DIR)" > $(ISODIR)/ssh/dest; \
		echo "export BUILD_SCP_NOSUBDIR=$(BUILD_SCP_NOSUBDIR)" >> $(ISODIR)/bldopts; \
	fi; \
	if [[ "$(AUTO_SHUTDOWN)" != "" ]]; then \
		echo "export AUTO_SHUTDOWN=$(AUTO_SHUTDOWN)" >> $(ISODIR)/bldopts; \
	fi; \
	if [[ "$(DEBUG_NO_STRIP)" != "" ]]; then \
		echo "export DEBUG_NO_STRIP=$(DEBUG_NO_STRIP)" >> $(ISODIR)/bldopts; \
	fi; \
	echo "--- Copying packages for ISO image file system ..."; \
	cp files/autorun.inf files/install.bat $(ISODIR)/; \
	cp files/hdd.img.gz $(ISODIR)/add/; \
	gzip -d $(ISODIR)/add/hdd.img.gz; \
	cp files/fstab $(ISODIR)/bin/; \
	cp files/buildall.sh $(ISODIR)/dl/src; \
	cp patches/* $(ISODIR)/dl/src; \
	cp $(addprefix $(WDLDIR)/, $(ALLFILES)) $(ISODIR)/dl/; \
	$(UNZIP) $(WDLDIR)/$(ZLIB_F) -d $(ISODIR)/ bin/zlib1.dll; \
	$(UNZIP) $(WDLDIR)/$(GZIP_F) -d $(ISODIR)/ bin/gzip.exe; \
	$(UNZIP) $(WDLDIR)/$(BZIP2_F) -d $(ISODIR)/ bin/bzip2.exe bin/bzip2.dll; \
	tar jxf $(WDLDIR)/$(BASH_F) bin/bash.exe bin/sh.exe; mv bin/* $(ISODIR)/bin/;rmdir bin; \
	tar jxf $(WDLDIR)/$(MSYS_F) bin/msys-1.0.dll bin/mount.exe bin/ps.exe; mv bin/* $(ISODIR)/bin/;rmdir bin; \
	tar jxf $(WDLDIR)/$(MSYSZLIB_F) bin/msys-z.dll; mv bin/* $(ISODIR)/bin/;rmdir bin; \
	$(UNZIP) $(WDLDIR)/$(LIBARCH_F) -d $(ISODIR)/ bin/bsdtar.exe bin/libarchive2.dll; \
	mv $(ISODIR)/dl/$(PTHREADS_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(QEMU_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(GROFF_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(CMAKE_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(SSLSRC_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(GNURX_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(POLIPO_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(SDL_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(OVPN_F) $(ISODIR)/dl/src/; \
	mv $(ISODIR)/dl/$(ZLIBSRC_F) $(ISODIR)/dl/src/; \
	echo "--- Repacking WinPCAP archive ..."; \
	$(UNZIP) $(WDLDIR)/$(WPCAPSRC_F); mv winpcap WpcapSrc_4_1_beta4; tar cf $(ISODIR)/dl/src/WpcapSrc_4_1_beta4.tar WpcapSrc_4_1_beta4; gzip -9 $(ISODIR)/dl/src/WpcapSrc_4_1_beta4.tar; rm -rf WpcapSrc_4_1_beta4 $(ISODIR)/dl/$(WPCAPSRC_F); \
	echo "--- Repacking Py2Exe archive ..."; \
	$(UNZIP) $(WDLDIR)/$(PY2EXE_F) -d tmp; mv tmp/py2exe* py2exe;rmdir tmp; tar zcf $(ISODIR)/dl/$(PY2EXE_TGZ) py2exe; rm -rf py2exe $(ISODIR)/dl/$(PY2EXE_F); \
	echo "--- Repacking NSIS archive ..."; \
	$(UNZIP) $(WDLDIR)/$(NSIS_F); tar zcf $(ISODIR)/dl/$(NSIS_TGZ) nsis-$(NSIS_VER); rm -rf nsis-$(NSIS_VER) $(ISODIR)/dl/$(NSIS_F); \
	echo "--- Repacking WiX archive ..."; \
	$(UNZIP) $(WDLDIR)/$(WIX_F) -d wix; tar zcf $(ISODIR)/dl/$(WIX_TGZ) wix; rm -rf wix $(ISODIR)/dl/$(WIX_F); \
	echo "--- Repacking Qt archive ..."; \
	$(UNZIP) $(WDLDIR)/$(QT_F) -d qt-$(QT_VER); mv qt-$(QT_VER)/qt-*/* qt-$(QT_VER)/; rm -rf qt-$(QT_VER)/qt-*; tar zcf $(ISODIR)/dl/src/$(QT_TGZ) qt-$(QT_VER); rm -rf qt-$(QT_VER) $(ISODIR)/dl/$(QT_F); \
	echo "--- Exporting Vidalia from local mirror ..."; \
	rm -rf $(VIDALIA_NAME); svn export --revision $(VIDALIA_VER) $(SDLDIR)/$(VIDALIA_NAME) $(VIDALIA_NAME); \
	tar zcf $(ISODIR)/dl/src/$(VIDALIA_TGZ) $(VIDALIA_NAME); rm -rf $(VIDALIA_NAME); \
	echo "--- Exporting Marble from local mirror ..."; \
	rm -rf $(MARBLE_NAME); svn export --revision $(MARBLE_VER) $(SDLDIR)/$(MARBLE_NAME) $(MARBLE_NAME) ; \
	tar zcf $(ISODIR)/dl/src/$(MARBLE_TGZ) $(MARBLE_NAME); rm -rf $(MARBLE_NAME); \
	echo "--- Exporting Thandy from local mirror ..."; \
	( cd $(SDLDIR); tar zcf $(ISODIR)/dl/src/$(THANDY_TGZ) $(THANDY_NAME) ); \
	echo "--- Exporting Tor from local mirror ..."; \
	rm -rf $(TORSVN_NAME); svn export --revision $(TORSVN_VER) $(SDLDIR)/$(TORSVN_NAME) $(TORSVN_NAME) ; \
	tar zcf $(ISODIR)/dl/src/$(TORSVN_TGZ) $(TORSVN_NAME); rm -rf $(TORSVN_NAME); \
	echo "--- Exporting PyCrypto from local mirror ..."; \
	( cd $(SDLDIR); tar zcf $(ISODIR)/dl/src/$(PYCRYPTO_TGZ) $(PYCRYPTO_NAME) ); \
	echo "--- Creating source archives from win32 tree ..."; \
	( cd src && tar zcf ../$(ISODIR)/dl/src/torvm-w32.tgz torvm-w32 ); \
	( cd src && tar zcf ../$(ISODIR)/dl/src/pkg.tgz pkg ); \
	touch $(ISODIR)/VMDEVISO.TXT

buildiso: extract
	@if [ -f w32build.iso ]; then \
		rm -f w32build.iso; \
	fi; \
	export ODIR=`pwd`; \
	( 	cd $(ISODIR); \
		mkisofs -hide-rr-moved -pad -l -o $$ODIR/w32build.iso -r -J .; \
	); if (( $$? != 0 )); then exit 1; fi

ifeq (,$(W32AUTO_BUILD_CMD))
  W32BLD=echo "Win32 build ISO image ready.  Please load the ISO into a Windows systems to start the build."
else
  W32BLD=now=`date`;echo "Starting win32 automated build at $$now";$(W32AUTO_BUILD_CMD);now=`date`;echo "Build done at $$now";true
endif

runinstall: buildiso
	@$(W32BLD)

.PHONY: downloads extract buildiso runinstall
