#!/bin/bash
export PATH=.:/usr/local/bin:/usr/bin:/bin:/mingw/bin:/wix:/lib:/usr/local/lib:/usr/libexec:/c/WINDOWS/system32:/c/WINDOWS:/c/WINDOWS/System32/Wbem
export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/lib:/mingw/lib
export ddir=/c/Tor_VM
export libdir="${ddir}/lib"
export bindir="${ddir}/bin"
export statedir="${ddir}/state"
export brootdir=/c/Tor_Win32
export instdir=$broot/Installer
export thandir=$broot/Thandy
export bundledir=$broot/Bundle

export ZLIB_VER="1.2.3"
export ZLIB_DIR="zlib-${ZLIB_VER}"
export ZLIB_FILE="zlib-${ZLIB_VER}.tar.gz"

export WPCAP_DIR=/usr/src/WpcapSrc_4_1_beta4
export WPCAP_INCLUDE="-I${WPCAP_DIR}/wpcap/libpcap -I${WPCAP_DIR}/wpcap/libpcap/Win32/Include"
export WPCAP_LDFLAGS="-L${WPCAP_DIR}/wpcap/PRJ -L${WPCAP_DIR}/packetNtx/Dll/Project"

export TORSVN_DIR="tor-latest"
export TORSVN_FILE="tor-latest.tar.gz"


export OPENSSL_VER="0.9.8j"
export OPENSSL_DIR="openssl-${OPENSSL_VER}"
export OPENSSL_FILE="openssl-${OPENSSL_VER}.tar.gz"

export GROFF_VER="1.19.2"
export GROFF_DIR="groff-${GROFF_VER}"
export GROFF_FILE="groff-${GROFF_VER}.tar.gz"

export CMAKE_VER="2.6.2"
export CMAKE_DIR="cmake-${CMAKE_VER}"
export CMAKE_FILE="cmake-${CMAKE_VER}.tar.gz"
export CMAKEBIN="/c/Program\ Files/CMake/bin"
export PATH="${PATH}:${CMAKEBIN}:/src/$CMAKE_DIR/bin"

export QT_VER="4.4.3"
export QT_DIR="qt-${QT_VER}"
export QT_FILE="qt-${QT_VER}.tgz"
export QT_ROOT="/c/Qt/${QT_VER}"
export QT_BIN="${QT_ROOT}/bin"
export QTDIR="C:\Qt\4.4.3"
export QMAKESPEC=win32-g++
export PATH="$PATH:$QT_BIN:$QTDIR\bin"

export PYTHON_ROOT=/c/Python26
export PATH=$PATH:$PYTHON_ROOT

export VIDALIA_FILE=vidalia-latest.tar.gz
export VIDALIA_DIR=vidalia-latest

export GNURX_FILE=mingw-libgnurx-2.5.1-src.tar.gz
export GNURX_DIR=mingw-libgnurx-2.5.1

export POLIPO_FILE=polipo-20080907.tar.gz
export POLIPO_DIR=polipo-20080907

export TORBUTTON_FILE=torbutton-1.2.0.xpi

export NSIS_DIR=nsis-2.42
export 7ZIP_DIR="/c/Program Files/7-Zip"
export PATH="${PATH}:/${NSIS_DIR}/Bin:/${NSIS_DIR}:/${NSIS_DIR}/bin:${7ZIP_DIR}"

if [ -d "$VS80COMNTOOLS" ]; then
  export VSTOOLSDIR="$VS80COMNTOOLS"
  export VSTOOLSENV="$VS80COMNTOOLS\vsvars32.bat"
elif [ -d "$VS90COMNTOOLS" ]; then
  export VSTOOLSDIR="$VS90COMNTOOLS"
  export VSTOOLSENV="$VS90COMNTOOLS\vsvars32.bat"
else
  unset VSTOOLSDIR
  unset VSTOOLSENV
fi

if [ -f ~/.ssh/user ]; then
  export BUILD_SCP_USER=`cat ~/.ssh/user`
  export BUILD_SCP_HOST=`cat ~/.ssh/host`
  export BUILD_SCP_DIR=`cat ~/.ssh/dest`
  chmod 700 ~/.ssh >/dev/null 2>&1
  # if the identity key has a prefix, remove it
  mv ~/.ssh/*id_rsa ~/.ssh/id_rsa >/dev/null 2>&1
  chmod 600 ~/.ssh/id_rsa >/dev/null 2>&1
fi

# wrap the actual build process so we capture stdout/stderr
# and also transfer over the build log and shutdown, if needed.
if [[ "$1" != "dobuild" ]]; then
  if [[ "$1" == "shell" ]]; then
    exec /bin/bash -l
  fi
  export build_date=`date +%s`
  cd /usr/src
  /usr/src/buildall.sh dobuild 2>&1 | tee build.log
  if (( $? != 0 )); then
    echo "BUILD_FAILED" >> build.log
  else
    echo "BUILD_COMPLETE" >> build.log
  fi
  if [[ "$BUILD_SCP_USER" != "" ]]; then
    echo "Transferring build to destination ${BUILD_SCP_HOST}:${BUILD_SCP_DIR} ..."
    scp -o BatchMode=yes -o CheckHostIP=no -o StrictHostKeyChecking=no \
        build.log "${BUILD_SCP_USER}@${BUILD_SCP_HOST}:${BUILD_SCP_DIR}/build_${build_date}.log"
  fi
  if [[ "$AUTO_SHUTDOWN" == "TRUE" ]]; then
    echo "Invoking automated shutdown ..."
    shutdown.exe -f -s -t 1
  fi
else

for dir in $ddir $libdir $bindir $statedir $brootdir $instdir $thandir $bundledir; do
  if [ ! -d $dir ]; then
    mkdir -p $dir
  fi
done

# enforce particular versions of some build utils
cd /
tar xf /dl/m4*

if [ -d /usr/usr ]; then
  # ahh, gotta love the msys /usr <-> / equivalence hack...
  cd /usr/usr
  if [ -d local ]; then
    mv local ../
  fi
  if [ -d bin ]; then
    mv bin/* ../bin/
    rmdir bin
  fi
  cd ..
  rmdir usr
fi
cp /usr/local/bin/aclocal-* /bin/aclocal
cp /usr/local/bin/autoconf-* /bin/autoconf
cp /usr/local/bin/autoheader-* /bin/autoheader
cp /usr/local/bin/automake-* /bin/automake
cp /usr/local/bin/autom4te-* /bin/autom4te

# make sure that msys libz headers and static libs are not present
# we only need the dll for ssh and friends.
rm -f /include/zlib.h /include/zconf.h /lib/libz.a /lib/libz.dll.a 


echo "Building pthreads-w32 ..."
cd /usr/src
tar zxvf pthreads-w32-2-8-0-release.tar.gz
mv pthreads-w32-2-8-0-release pthreads-w32
cd pthreads-w32
make GC
if (( $? != 0 )); then
  echo "ERROR: pthreads-32 build failed." >&2
  exit 1
fi
cp pthreadGC2.dll $libdir/


echo "Building zlib ..."
cd /usr/src
tar zxvf $ZLIB_FILE
cd $ZLIB_DIR
./configure --prefix=/usr --enable-shared
if (( $? != 0 )); then
  echo "ERROR: zlib configure failed." >&2
  exit 1
fi
make
if (( $? != 0 )); then
  echo "ERROR: zlib build failed." >&2
  exit 1
fi
make install
if (( $? != 0 )); then
  echo "ERROR: zlib install failed." >&2
  exit 1
fi
make -f win32/Makefile.gcc
if (( $? != 0 )); then
  echo "ERROR: zlib dynamic build failed." >&2
  exit 1
fi


echo "Building SDL library ..."
cd /usr/src
tar zxvf SDL-1.2.13.tar.gz
mv SDL-1.2.13 SDL
cd SDL
./configure --prefix=/usr
if (( $? != 0 )); then
  echo "ERROR: SDL configure failed." >&2
  exit 1
fi
make
if (( $? != 0 )); then
  echo "ERROR: SDL build failed." >&2
  exit 1
fi
make install
cp /usr/bin/SDL.dll $libdir/


echo "Locating Windows Driver Development Kit ..."
# there has to be a better way to do this .... ugg
found=0
DDKNAME="WINDDK"
DDKDIR=""
DDKENV=""
DDKVER=""
DDKMAJORVER=""
checkdirs="C D E F G H I J K L M N O P Q R S T U V W X Y Z"
for cdir in $checkdirs; do
  if (( $found == 0 )); then
    if [ -d /$cdir/ ]; then
      tddkdir="/$cdir/$DDKNAME"
      if [ ! -d $tddkdir ]; then
        tddkdir=`find /$cdir/ -type d -name $DDKNAME 2>/dev/null`
      fi
      if [ -d $tddkdir ]; then
        envf=`find $tddkdir -type f -name setenv.bat 2>/dev/null`
	if [ -f $envf ]; then
	  ddkbase=`dirname $envf`
	  ddkbase=`dirname $ddkbase`
          wddkbase=`echo $ddkbase | sed 's/^...//' | sed 's/\//\\\/g'`
	  DDKDIR="${cdir}:\\${wddkbase}"
	  DDKENV="${DDKDIR}\bin\setenv.bat"
	  DDKVER=`grep '^Build' "${ddkbase}/Uninstall/Uninstall.ini" | sed 's/.*=//'`
	  DDKMAJORVER=`echo $DDKVER | sed 's/\..*//'`
          echo "Found DDK install at $DDKDIR";echo "  using env script $DDKENV"
	  cp "${ddkbase}/tools/devcon/i386/devcon.exe" $bindir
	  found=1
	fi
      fi
    fi
  fi
done
if (( $found == 0 )); then
  echo "ERROR: Unable to locate the Windows Driver Development Kit." >&2
  echo "       Please provide the correct location to configure or"  >&2
  echo "       install the 1830_usa_ddk.iso software if needed." >&2
  exit 1
fi
export DDKDIR
export DDKENV
export DDKVER
export DDKMAJORVER


echo "Building openvpn tap-win32 driver ..."
cd /usr/src
tar zxvf openvpn-2.1_rc10.tar.gz
cd openvpn-2.1_rc10
patch -p1 < ../openvpn-tor-tap-win32-driver.patch 2>/dev/null
aclocal -I . && autoheader && autoconf && automake --add-missing --copy
if (( $? != 0 )); then
  echo "ERROR: openvpn autotools update failed." >&2
  exit 1
fi
MAN2HTML=/bin/true.exe ./configure --prefix=/usr \
 --with-cygwin-native \
 --disable-debug \
 --disable-lzo \
 --disable-ssl \
 --disable-crypto 
if (( $? != 0 )); then
  echo "ERROR: openvpn configure failed." >&2
  exit 1
fi
install-win32/maketap
cd tap-win32
TAPDIR=`pwd | sed 's/^.usr//' | sed 's/\//\\\/g'`
BPATH="${MSYSROOT}${TAPDIR}"
echo "call $DDKENV $DDKDIR wxp f" > dobuild.bat
echo "cd \"$BPATH\"" >> dobuild.bat
echo "build -cef" >> dobuild.bat
echo "exit" >> dobuild.bat
cmd.exe /k dobuild.bat
TAPDRVN=tortap91
if [ ! -f i386/${TAPDRVN}.sys ]; then
  echo "ERROR: openvpn tap-win32 driver build failed." >&2
  exit 1
fi
cp i386/${TAPDRVN}.sys $libdir/
cp i386/OemWin2k.inf $libdir/${TAPDRVN}.inf


echo "Building WinPcap ..."
cd /usr/src
tar zxvf WpcapSrc_4_1_beta4.tar.gz
cd WpcapSrc_4_1_beta4
wpbase=`pwd`
patch -p1 < ../winpcap-tor-device-mods.patch 2>/dev/null
cd packetNtx
PCAPDIR=`pwd | sed 's/^.usr//' | sed 's/\//\\\/g'`
BPATH="${MSYSROOT}${PCAPDIR}"
echo "call $DDKENV $DDKDIR w2k f" > dobuild.bat
echo "cd \"$BPATH\"" >> dobuild.bat
echo "call CompileDriver" >> dobuild.bat
echo "exit" >> dobuild.bat
cmd.exe /k dobuild.bat
if [ ! -f driver/bin/2k/i386/npf.sys ]; then
  echo "ERROR: WinPcap NPF.sys driver build failed." >&2
  exit 1
fi
cp driver/bin/2k/i386/npf.sys $libdir/tornpf.sys
cd Dll/Project
make
if (( $? != 0 )); then
  echo "ERROR: WinPcap Packet user space library build failed." >&2
  exit 1
fi
cp torpkt.dll $libdir/
cd $wpbase
cd wpcap/PRJ
make
if (( $? != 0 )); then
  echo "ERROR: WinPcap libwpcap user space library build failed." >&2
  exit 1
fi
cp torpcap.dll $libdir/

echo "Building qemu ..."
cd /usr/src
tar zxvf qemu-0.9.1.tar.gz
cd qemu-0.9.1
patch -p1 < ../qemu-kernel-cmdline-from-stdin.patch 2> /dev/null
if (( $? != 0 )); then
  echo "ERROR: Qemu cmdline via stdin patch failed." >&2
  exit 1
fi
patch -p1 < ../qemu-winpcap-0.9.1.patch 2> /dev/null
if (( $? != 0 )); then
  echo "ERROR: Qemu winpcap patch failed." >&2
  exit 1
fi
./configure --prefix=/usr --interp-prefix=qemu-%M \
  --enable-uname-release="Tor VM 2.6-alpha i386" \
  --disable-werror \
  --disable-kqemu \
  --disable-system \
  --disable-vnc-tls \
  --extra-cflags="-DHAVE_INTSZ_TYPES -I. -I.. -I/src/$ZLIB_DIR -I/usr/include -I/usr/local/include $WPCAP_INCLUDE -I/src/pthreads-w32 -I/usr/include/SDL" \
  --extra-ldflags="-L/src/$ZLIB_DIR -L/usr/lib -L/usr/local/lib $WPCAP_LDFLAGS -L/src/pthreads-w32" \
  --target-list=i386-softmmu
if (( $? != 0 )); then
  echo "ERROR: Qemu configure failed." >&2
  exit 1
fi
make
if (( $? != 0 )); then
  echo "ERROR: qemu build failed." >&2
  exit 1
fi
cp i386-softmmu/qemu.exe $bindir/
cp pc-bios/bios.bin $bindir/
cp pc-bios/vgabios.bin $bindir/
cp pc-bios/vgabios-cirrus.bin $bindir/

echo "Building torvm-w32 controller ..."
cd /usr/src
tar zxvf torvm-w32.tgz
cd torvm-w32
make
if (( $? != 0 )); then
  echo "ERROR: torvm-w32 build failed." >&2
  exit 1
fi
cp torvm.exe $ddir/


# don't forget the kernel and virtual disk
cp /usr/src/add/vmlinuz $libdir/
cp /usr/src/add/hdd.img $libdir/

if [[ "$DEBUG_NO_STRIP" == "" ]]; then
  echo "Stripping debug symbols from binaries and libraries ..."
  strip $libdir/*.dll
  strip $bindir/*.exe
  strip $bindir/*.dll
  strip $ddir/*.exe
fi

echo "Building groff ..."
cd /usr/src
tar zxvf $GROFF_FILE
cd $GROFF_DIR
./configure --prefix=/usr
if (( $? != 0 )); then
  echo "ERROR: groff configure failed." >&2
  exit 1
fi
for DEP in src/include src/libs/libgroff src/libs/libdriver src/preproc/soelim src/preproc/html src/devices/grotty src/devices/grops src/devices/grohtml src/roff/groff src/roff/troff font/devps font/devascii font/devhtml tmac; do
  make $DEP
  if (( $? != 0 )); then
    echo "ERROR: groff build for DEP $DEP failed." >&2
    exit 1
  fi 
done
cp src/preproc/soelim/soelim.exe src/preproc/html/pre-grohtml.exe src/roff/groff/groff.exe src/devices/grohtml/post-grohtml.exe src/devices/grotty/grotty.exe src/devices/grops/grops.exe src/roff/troff/troff.exe /bin/
cp -a font/devascii /share/
cp -a font/devhtml /share/
cp -a font/devps /share/
cp -a tmac /share/
  

echo "Building openssl ..."
cd /usr/src
tar zxvf $OPENSSL_FILE
cd $OPENSSL_DIR
patch -p1 < ../openssl-0.9.8i-mingw-shared.patch
./Configure --prefix=/usr no-idea no-rc5 no-mdc2 no-hw no-sse2 zlib-dynamic threads shared mingw
if (( $? != 0 )); then
  echo "ERROR: openssl configure failed." >&2
  exit 1
fi
echo "Configuring OpenSSL header files for build ..."
find crypto -name "*.h" -exec cp {} include/openssl/ \;
find ssl -name "*.h" -exec cp {} include/openssl/ \;
cp *.h include/openssl/
make
if (( $? != 0 )); then
  cp *.a /lib
  find crypto -name "*.h" -exec cp {} include/openssl/ \;
  find ssl -name "*.h" -exec cp {} include/openssl/ \;
  cp *.h include/openssl/
  make
  if (( $? != 0 )); then
    echo "ERROR: openssl build failed." >&2
    exit 1
  fi
fi
cp -f *.dll /lib/
cp *.a /lib/
cp -a include/openssl /usr/include/

echo "Extracting sources for Tor from svn ..."
cd /usr/src
tar zxvf $TORSVN_FILE
  
# Microsoft Installer package build
TOR_WXS=tor.wxs
TORUI_WXS=WixUI_Tor.wxs
TOR_WXS_DIR=contrib
TOR_MSI=tor.msi
WIX_UI=/wix/WixUIExtension.dll

echo "Building PyCrypto ..."
cd /usr/src
tar zxvf pycrypto-latest.tar.gz
cd pycrypto-latest
python setup.py build
if (( $? != 0 )); then
  echo "ERROR: PyCrypto build failed."
  exit 1
fi
python setup.py install
if (( $? != 0 )); then
  echo "ERROR: PyCrypto install failed."
  exit 1
fi

echo "Building py2exe ..."
cd /py2exe
python setup.py build
if (( $? != 0 )); then
  echo "ERROR: Thandy build failed."
  exit 1
fi
python setup.py install
if (( $? != 0 )); then 
  echo "ERROR: Thandy install failed."
  exit 1
fi

echo "Building Thandy ..."
cd /usr/src
tar zxvf thandy-latest.tar.gz
cd thandy-latest
echo "Starting build..."
python setup.py build
if (( $? != 0 )); then
  echo "ERROR: Thandy build failed."
  exit 1 
fi 
python setup.py py2exe
if (( $? != 0 )); then
  echo "ERROR: Thandy install failed."
  exit 1
fi
mv dist/ClientCLI.exe $thandir/Thandy.exe

echo "Building CMake ..."
cd /usr/src
tar zxvf $CMAKE_FILE
cd $CMAKE_DIR
./bootstrap --no-qt-gui
if (( $? != 0 )); then
  echo "ERROR: CMake bootstrap / configure failed."
#  exit 1
fi
make
if (( $? != 0 )); then
  echo "ERROR: CMake build failed."
#  exit 1
fi 
make install
if (( $? != 0 )); then
  echo "ERROR: CMake install failed."
#  exit 1
fi

echo "Building Qt ..."
cd /usr/src
mkdir /c/Qt
tar zxvf $QT_FILE
mv $QT_DIR /c/Qt/$QT_VER
cd /c/Qt/$QT_VER
if [ -f /src/qt-mingwssl.patch ]; then
  patch -p1 < /src/qt-mingwssl.patch
fi 
./configure.exe -confirm-license -release -no-dbus -no-phonon -no-webkit -no-qdbus -no-opengl -no-qt3support -no-xmlpatterns -no-sse2 -no-3dnow -qt-style-windowsxp -qt-style-windowsvista -no-sql-sqlite -no-sql-sqlite2 -no-sql-odbc -no-fast -openssl
echo "QT_BUILD_PARTS -= examples" >> .qmake.cache
echo "QT_BUILD_PARTS -= demos" >> .qmake.cache
echo "QT_BUILD_PARTS -= docs" >> .qmake.cache
unix2dos .qmake.cache
make
if (( $? != 0 )); then
  # seems to run out of memory???
  make
  if (( $? != 0 )); then
    echo "ERROR: Qt build failed."
  fi
fi

echo "Building GNU regex ..."
cd /usr/src
tar zxvf $GNURX_FILE
cd $GNURX_DIR
./configure --prefix=/usr
if (( $? != 0 )); then
  echo "ERROR: GNU regex configure failed."
fi
make
if (( $? != 0 )); then
  echo "ERROR: GNU regex build failed."
fi
make install
if (( $? != 0 )); then
  echo "ERROR: GNU regex install failed."
fi

echo "Building polipo ..."
cd /usr/src
tar zxvf $POLIPO_FILE
cd $POLIPO_DIR
if [ -f ../polipo-mingw.patch ]; then
  echo "Patching polipo sources ..."
  patch -p1 < ../polipo-mingw.patch
fi
make
if (( $? != 0 )); then
  echo "ERROR: polipo build failed."
fi

echo "Expanding package dir ..."
cd /usr/src
tar zxvf pkg.tgz

echo "Building Vidalia ..."
cd /usr/src
tar zxvf $VIDALIA_FILE
cd $VIDALIA_DIR
if [ -f ../vidalia-torvm.patch ]; then
  echo "Applying torvm patch to sources ..."
  patch -p1 < ../vidalia-torvm.patch
fi
cmake -DOPENSSL_LIBRARY_DIR=/src/$OPENSSL_DIR -DCMAKE_BUILD_TYPE=release -G "MSYS Makefiles" .
if [ ! -f Makefile ]; then
  echo "ERROR: Vidalia cmake failed."
fi
make
if (( $? != 0 )); then
  echo "ERROR: Vidalia build failed."
fi
if [ -f src/vidalia/vidalia.exe ]; then
  strip src/vidalia/vidalia.exe
  ls -l src/vidalia/vidalia.exe
  mkdir bin
  for FILE in QtCore4.dll QtGui4.dll QtNetwork4.dll QtXml4.dll QtSvg4.dll; do
    cp /c/Qt/$QT_VER/bin/$FILE bin/
  done
  cp /bin/mingwm10.dll bin/
  cp /src/$OPENSSL_DIR/ssleay32-0.9.8.dll bin/
  cp /src/$OPENSSL_DIR/cryptoeay32-0.9.8.dll bin/
  cp /src/$ZLIB_DIR/*.dll bin/
  cp /bin/pthreadGC2.dll bin/
  cp /bin/libgnurx-0.dll bin/
  cp /src/$POLIPO_DIR/polipo.exe bin/
  cp pkg/win32/polipo.conf bin/
  strip bin/*.dll
  strip bin/*.exe

  candle.exe pkg/win32/*.wxs
  light.exe -out vidalia.msi vidalia.wixobj WixUI_Tor.wixobj -ext $WIX_UI
  if [ -f vidalia.msi ]; then
    cp vidalia.msi $bundledir
    cp vidalia.msi ../pkg/
    cp -a bin ../pkg/
    ls -l vidalia.msi
  else
    echo "ERROR: unable to build vidalia MSI installer."
  fi
fi

echo "Building bundle packages ..."
cd /usr/src/pkg
# DONT STRIP PY2EXEs!
cp $thandir/Thandy.exe bin/
cp /src/$TORSVN_DIR/contrib/*.wxs ./
cp -a $ddir ./
if [ -f /src/$TORBUTTON_FILE ]; then
  cp /src/$TORBUTTON_FILE bin/torbutton.xpi
fi

cp /src/$TORBUTTON_FILE ./torbutton.xpi
touch tbcheck.bat
touch uninstall.bat
candle.exe *.wxs

light.exe -out torvm.msi WixUI_Tor.wixobj torvm.wixobj -ext $WIX_UI
if [ -f torvm.msi ]; then
  cp torvm.msi $bundledir
  ls -l torvm.msi
else
  echo "ERROR: unable to build Tor VM MSI installer."
fi

light.exe -out polipo.msi WixUI_Tor.wixobj polipo.wixobj -ext $WIX_UI
if [ -f polipo.msi ]; then
  cp polipo.msi $bundledir
  ls -l polipo.msi
else
  echo "ERROR: unable to build polipo MSI installer."
fi

light.exe -out torbutton.msi WixUI_Tor.wixobj torbutton.wixobj -ext $WIX_UI
if [ -f torbutton.msi ]; then
  cp torbutton.msi $bundledir
  ls -l torbutton.msi
else
  echo "ERROR: unable to build torbutton MSI installer."
fi

light.exe -out thandy.msi WixUI_Tor.wixobj thandy.wixobj -ext $WIX_UI
if [ -f thandy.msi ]; then
  cp thandy.msi $bundledir
  ls -l thandy.msi
else
  echo "ERROR: unable to build Thandy MSI installer."
fi

makensis.exe bundle.nsi
if [ -f TorVMBundle.exe ]; then
  cp TorVMBundle.exe $bundledir
  ls -l TorVMBundle.exe
else
  echo "ERROR: unable to build Tor VM executable bundle installer."
fi

makensis.exe netinst.nsi
if [ -f TorVMNetInstaller.exe ]; then
  cp TorVMNetInstaller.exe $bundledir
  ls -l TorVMNetInstaller.exe
else
  echo "ERROR: unable to build Tor VM executable network installer."
fi

export exename=Tor_VM.exe
if [ -f $exename ]; then
  rm -f $exename
fi
7z.exe a -sfx7z.sfx $exename Tor_VM
if [ -f $exename ]; then
  cp $exename $bundledir
  ls -l $exename
else
  echo "ERROR: unable to build self extracting Tor VM archive."
fi

if [[ "$BUILD_SCP_USER" != "" ]]; then
  echo "Transferring build to destination ${BUILD_SCP_HOST}:${BUILD_SCP_DIR} ..."
  scp -o BatchMode=yes -o CheckHostIP=no -o StrictHostKeyChecking=no \
      -r /c/Tor_VM "${BUILD_SCP_USER}@${BUILD_SCP_HOST}:${BUILD_SCP_DIR}/Tor_VM_${build_date}"
fi

echo "DONE."
exit 0

fi
