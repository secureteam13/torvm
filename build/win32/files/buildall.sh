#!/bin/bash
# wrap the actual build process so we capture stdout/stderr
# and also transfer over the build log and shutdown, if needed.
if [[ "$1" != "dobuild" ]]; then
  # only set various environment settings on the first pass
  export PATH=.:/usr/local/bin:/usr/bin:/bin:/mingw/bin:/wix:/lib:/usr/local/lib:/usr/libexec:$PATH
  export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/lib:/mingw/lib:$LD_LIBRARY_PATH

  export KERNEL_IMAGE=/src/add/vmlinuz
  export VMHDD_IMAGE=/src/add/hdd.img
  
  # set sysdrive, ddir, and brootdir in parent env if needed.
  if [[ "$sysdrive" == "" ]]; then
    sysdrive=`echo $SYSTEMDRIVE | sed 's/:.*//'`
    if [ ! -d /$sysdrive ]; then
      # msys not happy about whatever odd location windows installed into.
      sysdrive=c
    fi
  fi
  # make sure we express sys drive as lower case because of msys pedantic'ness
  # or that of the sub scripts and whatever else get confused.
  # boy, wouldn't tr be nice? tr -t '[:upper:]' '[:lower:]'
  for DL in a b c d e f g h i j k l m n o p q r s t u v w x y z; do
    echo $sysdrive | grep -i "^[${DL}]" >/dev/null
    if (( $? == 0 )); then
      sysdrive=$DL
    fi
  done
  if [ ! -d /$sysdrive ]; then
    echo "Bogus sysdrive Windows root set: $sysdrive , using defaults ..."
    sysdrive=c
  fi
  echo "Using Windows system drive root /$sysdrive , ${sysdrive}:\\"
  export sysdrive
  if [[ "$ddir" == "" ]]; then
    ddir=/$sysdrive/Tor_VM
  fi
  if [[ "$brootdir" == "" ]]; then
    brootdir=/$sysdrive/Tor_Win32
  fi
  echo "Using Tor VM destination folder: $ddir"
  echo "Using Bundle destination folder: $brootdir"
  export ddir
  export brootdir
  
  # make sure some default windows paths are available too
  export PATH=$PATH:/$sysdrive/WINDOWS/system32:/$sysdrive/WINDOWS:/$sysdrive/WINDOWS/System32/Wbem
  
  # the build state file is solely used to toggle targets on or off
  # intended to be used when developing or automated via buildbot, etc.
  if [[ "$bstatefile" == "" ]]; then
    export bstatefile="/src/build.state"
  fi
  if [ -f $bstatefile ]; then
    source $bstatefile
  else
    echo '#!/bin/bash' > $bstatefile
    chmod +x $bstatefile
  fi
  
  export bdlibdir="${ddir}/lib"
  export bindir="${ddir}/bin"
  export statedir="${ddir}/state"
  export instdir="${brootdir}/Installer"
  export thandir="${brootdir}/Thandy"
  export bundledir="${brootdir}/Bundle"
  export licensedir="${brootdir}/License"
  
  if [[ "$SEVNZIP_INST" == "" ]]; then
    export SEVNZIP_INST=true
  fi
  SEVNZIP_DEF_INSTPATH="/${sysdrive}/Program Files/7-Zip"
  export PATH="$PATH:${SEVNZIP_DEF_INSTPATH}"
  
  export ZLIB_VER="1.2.3"
  export ZLIB_DIR="zlib-${ZLIB_VER}"
  export ZLIB_FILE="zlib-${ZLIB_VER}.tar.gz"

  export LIBEVENT_VER=1.4.8-stable
  export LIBEVENT_FILE="libevent-${LIBEVENT_VER}.tar.gz"
  export LIBEVENT_DIR="libevent-${LIBEVENT_VER}"
  
  export TOR_DIR="tor-latest"
  export TOR_FILE="tor-latest.tar.gz"
  
  export PTHREAD_VER=2-8-0
  export PTHREAD_DIR="pthreads-w32-${PTHREAD_VER}-release"
  export PTHREAD_FILE="${PTHREAD_DIR}.tar.gz"
  
  export OPENSSL_VER="0.9.8j"
  export OPENSSL_DIR="openssl-${OPENSSL_VER}"
  export OPENSSL_FILE="openssl-${OPENSSL_VER}.tar.gz"
  
  export GROFF_VER="1.19.2"
  export GROFF_DIR="groff-${GROFF_VER}"
  export GROFF_FILE="groff-${GROFF_VER}.tar.gz"

  export SDL_VER=1.2.13
  export SDL_DIR="SDL-${SDL_VER}"
  export SDL_FILE="${SDL_DIR}.tar.gz"

  export OPENVPN_VER=2.1_rc15
  export OPENVPN_DIR="openvpn-${OPENVPN_VER}"
  export OPENVPN_FILE="${OPENVPN_DIR}.tar.gz"

  export WPCAP_VER=4_1_beta5
  export WPCAP_DIR="WpcapSrc_${WPCAP_VER}"
  export WPCAP_FILE="${WPCAP_DIR}.tar.gz"
  export WPCAP_INCLUDE="-I/src/${WPCAP_DIR}/wpcap/libpcap -I/src/${WPCAP_DIR}/wpcap/libpcap/Win32/Include"
  export WPCAP_LDFLAGS="-L/src/${WPCAP_DIR}/wpcap/PRJ -L/src/${WPCAP_DIR}/packetNtx/Dll/Project"

  export QEMU_VER=0.9.1
  export QEMU_DIR="qemu-${QEMU_VER}"
  export QEMU_FILE="${QEMU_DIR}.tar.gz"

  export CMAKE_VER="2.6.2"
  export CMAKE_DIR="cmake-${CMAKE_VER}"
  export CMAKE_FILE="cmake-${CMAKE_VER}.tar.gz"
  export CMAKEBIN="/$sysdrive/Program Files/CMake/bin"
  export PATH="${PATH}:${CMAKEBIN}:/src/$CMAKE_DIR/bin"
  
  export QT_VER="4.5.0"
  export QT_DIR="qt-all-opensource-src-${QT_VER}"
  export QT_FILE="${QT_DIR}.tar.bz2"
  export QT_ROOT="/$sysdrive/Qt/${QT_VER}"
  export QT_BIN="${QT_ROOT}/bin"
  export QTDIR="${sysdrive}:\Qt\4.5.0"
  export QMAKESPEC=win32-g++
  export PATH="$PATH:$QT_BIN:$QTDIR\bin"
  
  export PYTHON_ROOT=/$sysdrive/Python26
  export PATH=$PATH:$PYTHON_ROOT
  
  export MARBLE_DIR=marble-latest
  export MARBLE_FILE="${MARBLE_DIR}.tar.gz"
  export MARBLE_DEST=/marble
  export MARBLE_OPTS="-DCMAKE_BUILD_TYPE=release -DQTONLY=ON -DCMAKE_INSTALL_PREFIX=${MARBLE_DEST} -DPACKAGE_ROOT_PREFIX=${MARBLE_DEST} -DMARBLE_DATA_PATH=data"
  
  export VIDALIA_FILE=vidalia-latest.tar.gz
  export VIDALIA_DIR=vidalia-latest
  # XXX need to resolve why this wont build against the installed marble, only the build tree
  export VIDALIA_OPTS="-DCMAKE_BUILD_TYPE=release"
  export VIDALIA_MARBLE_OPTS="-DUSE_MARBLE=1 -DMARBLE_LIBRARY_DIR=/src/${MARBLE_DIR}/src/lib -DMARBLE_DATA_DIR=/src/${MARBLE_DIR}/data -DMARBLE_INCLUDE_DIR=${MARBLE_DEST}/include/marble -DMARBLE_PLUGIN_DIR=/src/${MARBLE_DIR}/src/plugins"
  
  export GNURX_FILE=mingw-libgnurx-2.5.1-src.tar.gz
  export GNURX_DIR=mingw-libgnurx-2.5.1
  
  export POLIPO_FILE=polipo-20080907.tar.gz
  export POLIPO_DIR=polipo-20080907
  
  export TORBUTTON_FILE=torbutton-1.2.0.xpi
  
  export NSIS_DIR=nsis-2.42
  export PATH="${PATH}:/${NSIS_DIR}/Bin:/${NSIS_DIR}:/${NSIS_DIR}/bin"

  export WIXSRC_DIR=wixsrc
  export WIXSRC_FILE=wixsrc.tar.gz
  
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
  if [ -f "$VSTOOLSENV" ]; then
    echo "Using VisualStudio environment script at $VSTOOLSENV."
    export PATH="$PATH:$VSTOOLSDIR:$VSTOOLSDIR\Bin:$VSTOOLSDIR\VC\bin:$VSTOOLSDIR\Common7\IDE:$VSTOOLSDIR\SDK\v2.0\bin"
    echo "Using new PATH=$PATH"
  fi
  
  # always set these since we may need to copy over failure logs
  # and don't have a proper login shell env
  if [ -f ~/.ssh/user ]; then
    export BUILD_SCP_USER=`cat ~/.ssh/user`
    export BUILD_SCP_HOST=`cat ~/.ssh/host`
    export BUILD_SCP_DIR=`cat ~/.ssh/dest`
  fi

  # the shell option gives a login shell with all of the env setup properly
  if [[ "$1" == "shell" ]]; then
    exec /bin/bash -l
  fi
  export build_date=`date +%s`
  if [[ "$bld_dsub" == "" ]]; then
    export bld_dsub="${BUILD_SCP_DIR}/${build_date}"
    if [[ "$BUILD_SCP_NOSUBDIR" == "TRUE" ]]; then
      # dont include the date subdir in any copied destinations.
      export bld_dsub="${BUILD_SCP_DIR}"
    fi
  fi
  cd /usr/src
  /usr/src/buildall.sh dobuild 2>&1 | tee build.log
  if (( $? != 0 )); then
    echo "BUILD_FAILED" >> build.log
  else
    echo "BUILD_COMPLETE" >> build.log
  fi
  if [[ "$BUILD_SCP_USER" != "" ]]; then
    echo "Transferring build to destination ${BUILD_SCP_HOST}:${bld_dsub} ..."
    scp -o BatchMode=yes -o CheckHostIP=no -o StrictHostKeyChecking=no \
        build.log "${BUILD_SCP_USER}@${BUILD_SCP_HOST}:${bld_dsub}/win32build.log"
  fi
  if [[ "$AUTO_SHUTDOWN" == "TRUE" ]]; then
    echo "Invoking automated shutdown ..."
    shutdown.exe -f -s -t 1
  fi
else


# SECOND PASS - begin build process
  
function pkgbuilt () {
  echo "export $1=yes" >> $bstatefile
}

if [[ "$MSYS_SETUP" != "yes" ]]; then
  echo "Setting up MSYS build environment ..."
  if [ -f ~/.ssh/user ]; then
    chmod 700 ~/.ssh >/dev/null 2>&1
    if [ ! -f ~/.ssh/id_rsa ]; then
      # if the identity key has a prefix, remove it
      mv ~/.ssh/*id_rsa ~/.ssh/id_rsa >/dev/null 2>&1
    fi
    chmod 600 ~/.ssh/id_rsa >/dev/null 2>&1
  fi

  for dir in $ddir $bdlibdir $bindir $statedir $brootdir $instdir $thandir $bundledir $licensedir; do
    if [ ! -d $dir ]; then
      mkdir -p $dir
    fi
  done

  # enforce particular versions of some build utils
  cd /
  tar xf /dl/m4*

  if [ -d /usr/usr ]; then
    # ahh, gotta love the msys /usr <-> / equivalence hack.
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

  # if WiX sources available extract into expected location for
  # localization support.
  cd /src
  if [ -d /$WIXSRC_DIR ]; then
    mv /$WIXSRC_DIR ./
  fi

  pkgbuilt MSYS_SETUP
fi

if [[ "$PKGS_INSTALLED" != "yes" ]]; then
  anyfail=0
  echo "Checking for any packages to install ..."
  if [[ "$SEVNZIP_INST" == "true" ]]; then
    if [ ! -f "/dl/${SEVNZIP_PKG}" ]; then
      echo "ERROR: Unable to locate expected 7zip package for install at location: /${SEVNZIP_PKG}"
      anyfail=1
    else
      echo "Attempting to install ${MSYSROOT}\\dl\\${SEVNZIP_PKG} ..."
      $COMSPEC /k "msiexec /i ${MSYSROOT}\\dl\\${SEVNZIP_PKG} /qn" < /dev/null
      # XXX need to check for failure to install properly via exit code and package status.
    fi
  fi
  if (( $anyfail == 0 )); then
    pkgbuilt PKGS_INSTALLED
  fi
fi

if [[ "$IGNORE_DDK" != "yes" ]]; then
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
fi


if [[ "$PTHREADS_BUILT" != "yes" ]]; then
  echo "Building pthreads-w32 ..."
  cd /usr/src
  tar zxf $PTHREAD_FILE
  cd $PTHREAD_DIR
  make GC
  if (( $? != 0 )); then
    echo "ERROR: pthreads-32 build failed." >&2
    exit 1
  fi
  # XXX make install not quite sane yet
  cp *.a /lib/
  cp *.dll /bin/
  cp *.h /usr/include/
  cp pthreadGC2.dll $bdlibdir/
  cp COPYING.LIB $licensedir/pthreads-w32-COPYING-LIB.txt
  cp COPYING $licensedir/pthreads-w32-COPYING.txt

  pkgbuilt PTHREADS_BUILT
fi


if [[ "$ZLIB_BUILT" != "yes" ]]; then
  echo "Building zlib ..."
  cd /usr/src
  tar zxf $ZLIB_FILE
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
  cp *.dll $bdlibdir/
  cp README $licensedir/zlib-README.txt

  pkgbuilt ZLIB_BUILT
fi


if [[ "$LIBEVENT_BUILT" != "yes" ]]; then
  echo "Building libevent ..."
  cd /usr/src
  tar zxf $LIBEVENT_FILE
  cd $LIBEVENT_DIR
  ./configure --prefix=/usr --enable-static --disable-shared
  if (( $? != 0 )); then
    echo "ERROR: libevent configure failed." >&2
    exit 1
  fi
  make
  if (( $? != 0 )); then
    echo "ERROR: libevent build failed." >&2
    exit 1
  fi
  make install
  # XXX license info is per file

  pkgbuilt LIBEVENT_BUILT
fi


if [[ "$SDL_BUILT" != "yes" ]]; then
  echo "Building SDL library ..."
  cd /usr/src
  tar zxf $SDL_FILE
  cd $SDL_DIR
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
  cp /usr/bin/SDL.dll $bdlibdir/
  cp COPYING $licensedir/SDL-COPYING.txt

  pkgbuilt SDL_BUILT
fi


if [[ "$OPENVPN_BUILT" != "yes" ]]; then
  echo "Building openvpn tap-win32 driver ..."
  cd /usr/src
  tar zxf $OPENVPN_FILE
  cd $OPENVPN_DIR
  if [ -f ../openvpn-tor-tap-win32-driver.patch ]; then
    echo "Patching OpenVPN sources ..."
    patch -p1 < ../openvpn-tor-tap-win32-driver.patch
  fi
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
  $COMSPEC /k dobuild.bat
  TAPDRVN=tortap91
  if [ ! -f i386/${TAPDRVN}.sys ]; then
    echo "ERROR: openvpn tap-win32 driver build failed." >&2
    exit 1
  fi
  cp i386/${TAPDRVN}.sys $bdlibdir/
  cp i386/OemWin2k.inf $bdlibdir/${TAPDRVN}.inf
  cp COPYING $licensedir/openvpn-COPYING.txt
  cp COPYRIGHT.GPL $licensedir/openvpn-COPYRIGHT-GPL.txt

  pkgbuilt OPENVPN_BUILT
fi
  

if [[ "$WPCAP_BUILT" != "yes" ]]; then
  echo "Building WinPcap ..."
  cd /usr/src
  tar zxf $WPCAP_FILE
  cd $WPCAP_DIR
  wpbase=`pwd`
  if [ -f ../winpcap-tor-device-mods.patch ]; then
    echo "Patching WinPcap sources ..."
    patch -p1 < ../winpcap-tor-device-mods.patch
  fi
  cd packetNtx
  PCAPDIR=`pwd | sed 's/^.usr//' | sed 's/\//\\\/g'`
  BPATH="${MSYSROOT}${PCAPDIR}"
  echo "call $DDKENV $DDKDIR w2k f" > dobuild.bat
  echo "cd \"$BPATH\"" >> dobuild.bat
  echo "call CompileDriver" >> dobuild.bat
  echo "exit" >> dobuild.bat
  $COMSPEC /k dobuild.bat
  NPFDRV_F=driver/bin/i386/npf.sys
  if [ ! -f $NPFDRV_F ]; then
    echo "ERROR: WinPcap npf.sys driver build failed." >&2
    exit 1
  fi
  cp $NPFDRV_F $bdlibdir/tornpf.sys
  cd Dll/Project
  make
  if (( $? != 0 )); then
    echo "ERROR: WinPcap Packet user space library build failed." >&2
    exit 1
  fi
  cp torpkt.dll $bdlibdir/
  cd $wpbase
  cd wpcap/PRJ
  make
  if (( $? != 0 )); then
    echo "ERROR: WinPcap libwpcap user space library build failed." >&2
    exit 1
  fi
  cp torpcap.dll $bdlibdir/

  pkgbuilt WPCAP_BUILT
fi


if [[ "$QEMU_BUILT" != "yes" ]]; then
  echo "Building qemu ..."
  cd /usr/src
  tar zxf $QEMU_FILE
  cd $QEMU_DIR
  if [ -f ../qemu-kernel-cmdline-from-stdin.patch ]; then
    echo "Patching Qemu sources ..."
    patch -p1 < ../qemu-kernel-cmdline-from-stdin.patch
  fi
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

  pkgbuilt QEMU_BUILT
fi


if [[ "$W32CTL_BUILT" != "yes" ]]; then
  echo "Building torvm-w32 controller ..."
  cd /usr/src
  tar zxf torvm-w32.tgz
  cd torvm-w32
  make
  if (( $? != 0 )); then
    echo "ERROR: torvm-w32 build failed." >&2
    exit 1
  fi
  cp torvm.exe $ddir/

  pkgbuilt W32CTL_BUILT
fi


if [[ "$GROFF_BUILT" != "yes" ]]; then
  echo "Building groff ..."
  cd /usr/src
  tar zxf $GROFF_FILE
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

  pkgbuilt GROFF_BUILT
fi
  

if [[ "$OPENSSL_BUILT" != "yes" ]]; then
  echo "Building openssl ..."
  cd /usr/src
  tar zxf $OPENSSL_FILE
  cd $OPENSSL_DIR
  # XXX there should be a way to do this without patching despite recursive make invocations.
  if [ -f ../openssl-0.9.8-mingw-shared.patch ]; then
    echo "Patching openssl for shared mingw builds"
    patch -p1 < ../openssl-0.9.8-mingw-shared.patch
  fi
  ./Configure --prefix=/usr no-idea no-rc5 no-mdc2 no-hw no-sse2 zlib-dynamic threads shared mingw
  if (( $? != 0 )); then
    echo "ERROR: openssl configure failed." >&2
    exit 1
  fi
  echo "Configuring OpenSSL header files for build ..."
  find crypto -name "*.h" -exec cp {} include/openssl/ \;
  find ssl -name "*.h" -exec cp {} include/openssl/ \;
  find fips -name "*.h" -exec cp {} include/openssl/ \;
  cp *.h include/openssl/
  make
  if (( $? != 0 )); then
    # XXX Poor workaround for initial pass missing DLL dependencies during linkage
    cp *.a /lib
    find crypto -name "*.h" -exec cp {} include/openssl/ \;
    find ssl -name "*.h" -exec cp {} include/openssl/ \;
    find fips -name "*.h" -exec cp {} include/openssl/ \;
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

  pkgbuilt OPENSSL_BUILT
fi


if [[ "$TOR_BUILT" != "yes" ]]; then
  echo "Building Tor stand alone ..."
  cd /usr/src
  tar zxf $TOR_FILE
  cd $TOR_DIR
  if [ ! -f configure ]; then
    echo "Attempting autogen ..."
    aclocal
    autoheader
    autoconf
    automake --add-missing --copy
  fi
  echo "Invoking configure"
  (
    export CFLAGS="-I /src/$ZLIB_DIR -I/src/$LIBEVENT_DIR -I/src/$OPENSSL_DIR"
    ./configure --prefix=/src/$TOR_DIR \
            --with-zlib-dir=/src/$ZLIB_DIR \
            --with-libevent-dir=/src/$LIBEVENT_DIR \
            --with-openssl-dir=/src/$OPENSSL_DIR \
            --enable-shared \
            --enable-threads \
            --enable-local-appdata \
            --disable-transparent
    if (( $? != 0 )); then
      echo "ERROR: torsvn autoconf configure failed." >&2
      exit 1
    fi
    make
    if (( $? != 0 )); then
      echo "ERROR: make autoconf torsvn failed." >&2
      exit 1
    fi
    make install
  ) 
  # prepare files needed for WiX MSI package build at expected locations
  cp /src/$OPENSSL_DIR/cryptoeay32-0.9.8.dll bin/
  cp /src/$OPENSSL_DIR/ssleay32-0.9.8.dll bin/
  # line conversion prior to MSI packaging and other documentation prep
  ( 
    # this has to run detached or else troff tries to eat our controlling tty
    # resulting in a hung build without user interaction.
    groff -m man -F /share -M /share/tmac -T html doc/tor.1 > Usage.html &
    i=20
    pc=0
    while (( $i > 0 )); do
      sleep 1
      i=$(expr $i - 1)
      if [ -f Usage.html ]; then
        cc=$(cat Usage.html | wc -c)
        if (( $cc > $pc )); then
          pc=$cc
        else
          # all done generating
          i=0
        fi
      fi
    done
  )
  mkdir pkgdocs
  ls -lh Usage.html
  for FILE in README ChangeLog LICENSE Authors src/config/torrc.sample; do
    if [ -f $FILE ]; then
      unix2dos $FILE
    else
      # must have the file present or the WiX MSI build fails
      echo > $FILE
    fi
    cp $FILE pkgdocs/
  done

  pkgbuilt TOR_BUILT
fi


if [[ "$PYCRYPTO_BUILT" != "yes" ]]; then
  echo "Building PyCrypto ..."
  cd /usr/src
  tar zxf pycrypto-latest.tar.gz
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

  pkgbuilt PYCRYPTO_BUILT
fi


if [[ "$PY2EXE_BUILT" != "yes" ]]; then
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

  pkgbuilt PY2EXE_BUILT
fi


if [[ "$THANDY_BUILT" != "yes" ]]; then
  echo "Building Thandy ..."
  cd /usr/src
  tar zxf thandy-latest.tar.gz
  cd thandy-latest
  echo "Starting python Thandy build ..."
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

  pkgbuilt THANDY_BUILT
fi


if [[ "$CMAKE_BUILT" != "yes" ]]; then
  echo "Building CMake ..."
  cd /usr/src
  tar zxf $CMAKE_FILE
  cd $CMAKE_DIR
  ./bootstrap --no-qt-gui
  if (( $? != 0 )); then
    echo "ERROR: CMake bootstrap / configure failed."
    exit 1
  fi
  make
  if (( $? != 0 )); then
    echo "ERROR: CMake build failed."
    exit 1
  fi 
  make install
  if (( $? != 0 )); then
    echo "ERROR: CMake install failed."
    exit 1
  fi

  pkgbuilt CMAKE_BUILT
fi


if [[ "$QT_BUILT" != "yes" ]]; then
  echo "Building Qt ..."
  cd /usr/src
  mkdir /$sysdrive/Qt
  tar jxf $QT_FILE
  mv $QT_DIR /$sysdrive/Qt/$QT_VER
  cd /$sysdrive/Qt/$QT_VER
  if [ -f /src/qt-mingwssl.patch ]; then
    patch -p1 < /src/qt-mingwssl.patch
  fi 
  QT_CONF="-confirm-license -release -shared -fast"
  QT_CONF="$QT_CONF -no-dbus -no-phonon -no-webkit -no-qdbus -no-opengl -no-qt3support -no-xmlpatterns"
  QT_CONF="$QT_CONF -qt-style-windowsxp -no-style-windowsvista"
  QT_CONF="$QT_CONF -no-sql-sqlite -no-sql-sqlite2 -no-sql-odbc"
  QT_CONF="$QT_CONF -openssl -no-libmng -no-libtiff -qt-libpng -qt-libjpeg -qt-gif"
  ./configure.exe $QT_CONF
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

  # XXX: so it seems some things (marble) want to try and load
  # image plugins that are not plugins, namely the .a libtool
  # hooks for linking DLL's in mingw.  this is a hammer to
  # prevent such mistakes that will halt an automated build with
  # "Error not a library" warnings when a static lib is passed
  # to OpenLibrary.
  find plugins/imageformats -name \*.a -exec rm {} \;

  pkgbuilt QT_BUILT
fi


if [[ "$GNUREGEX_BUILT" != "yes" ]]; then
  echo "Building GNU regex ..."
  cd /usr/src
  tar zxf $GNURX_FILE
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

  pkgbuilt GNUREGEX_BUILT
fi


if [[ "$POLIPO_BUILT" != "yes" ]]; then
  echo "Building polipo ..."
  cd /usr/src
  tar zxf $POLIPO_FILE
  cd $POLIPO_DIR
  if [ -f ../polipo-mingw.patch ]; then
    echo "Patching polipo sources ..."
    patch -p1 < ../polipo-mingw.patch
  fi
  make
  if (( $? != 0 )); then
    echo "ERROR: polipo build failed."
  fi

  pkgbuilt POLIPO_BUILT
fi


if [[ "$MARBLE_BUILT" != "yes" ]]; then
  echo "Building Marble widget for Qt ..."
  cd /usr/src
  tar zxf $MARBLE_FILE
  cd $MARBLE_DIR
  cmake $MARBLE_OPTS -G "MSYS Makefiles" .
  if [ ! -f Makefile ]; then
    echo "ERROR: Marble cmake failed."
  fi
  make
  if (( $? != 0 )); then
    echo "ERROR: Marble build failed."
  fi
  make install
  if (( $? != 0 )); then
    echo "ERROR: Marble build failed."
  fi

  pkgbuilt MARBLE_BUILT
fi


if [[ "$VIDALIA_BUILT" != "yes" ]]; then
  echo "Building Vidalia ..."
  cd /usr/src
  tar zxf $VIDALIA_FILE
  cd $VIDALIA_DIR
  if [ -f ../vidalia-torvm.patch ]; then
    echo "Applying torvm patch to sources ..."
    patch -p1 < ../vidalia-torvm.patch
  fi
  if [ ! -d bin ]; then
    mkdir bin
  fi
  VIDALIA_EXE=src/vidalia/vidalia.exe

  # build one vidalia with the old 2D interface ...
  echo "Building Vidalia with standard 2D map widget support ..."
  cmake $VIDALIA_OPTS -DOPENSSL_LIBRARY_DIR=/src/$OPENSSL_DIR -G "MSYS Makefiles" .
  if [ ! -f Makefile ]; then
    echo "ERROR: Vidalia cmake failed."
  fi
  make
  if (( $? != 0 )); then
    echo "ERROR: Vidalia build failed."
  fi
  if [ -f $VIDALIA_EXE ]; then
    cp $VIDALIA_EXE bin/vidalia-2d.exe
  fi

  # and another with the new Marble UI
  echo "Building Vidalia with new 3D Marble map support ..."
  make clean >/dev/null 2>/dev/null
  cmake $VIDALIA_OPTS $VIDALIA_MARBLE_OPTS -DOPENSSL_LIBRARY_DIR=/src/$OPENSSL_DIR -G "MSYS Makefiles" .
  if [ ! -f Makefile ]; then
    echo "ERROR: Vidalia cmake failed."
  fi
  make
  if (( $? != 0 )); then
    echo "ERROR: Vidalia build failed."
  fi
  if [ -f $VIDALIA_EXE ]; then
    cp $VIDALIA_EXE bin/vidalia-marble.exe
  fi

  pkgbuilt VIDALIA_BUILT
fi


# don't forget the kernel and virtual disk
cp $KERNEL_IMAGE $bdlibdir/
cp $VMHDD_IMAGE $bdlibdir/


# Microsoft Installer package build
TOR_WXS_DIR=contrib
WIX_UI=/wix/WixUIExtension.dll
WIXSRC_WXLDIR=/src/$WIXSRC_DIR/src/ext/UIExtension/wixlib
DEF_WXL_LANG=en-us
WXL_LANGS="cs-cz de-de es-es fr-fr hu-hu it-it ja-jp nl-nl pl-pl ru-ru uk-ua en-us"
# XXX currently problems with WiX handling of: zh_CN zh_TW
VIDALIA_LANGS="cs de es fa fi fr he it nl pl pt ro ru sv en"
WIX_ALL_LOC_LINK=""
for LANG in $WXL_LANGS; do
  WIX_ALL_LOC_LINK="${WIX_ALL_LOC_LINK} -loc WixUI_${LANG}.wxl"
done
for LANG in $VIDALIA_LANGS; do
  WIX_ALL_LOC_LINK="${WIX_ALL_LOC_LINK} -loc vidalia_${LANG}.wxl"
done

# XXX building all languages eats up too much time and space
BUILD_IND_LANGS=yes

if [[ "$PACKAGES_BUILT" != "yes" ]]; then
  echo "Building bundle packages ..."
  echo "Expanding package dir ..."
  cd /src
  tar zxf pkg.tgz
  if [ -f /usr/src/$VIDALIA_DIR/src/vidalia/vidalia.exe ]; then
    echo "Creating Vidalia MSI package ..."
    cd /src/$VIDALIA_DIR
    for FILE in QtCore4.dll QtGui4.dll QtNetwork4.dll QtXml4.dll QtSvg4.dll; do
      cp /$sysdrive/Qt/$QT_VER/bin/$FILE bin/
    done
    cp /bin/mingwm10.dll bin/
    cp /src/$OPENSSL_DIR/ssleay32-0.9.8.dll bin/
    cp /src/$OPENSSL_DIR/cryptoeay32-0.9.8.dll bin/
    cp /src/$ZLIB_DIR/*.dll bin/
    cp /bin/pthreadGC2.dll bin/
    cp /bin/libgnurx-0.dll bin/
    cp /src/$POLIPO_DIR/polipo.exe bin/
    cp pkg/win32/polipo.conf bin/
    if [ -d $MARBLE_DEST ]; then
      cp $MARBLE_DEST/libmarblewidget.dll bin/
      mkdir -p plugins/imageformats
      cp $MARBLE_DEST/plugins/*.dll plugins/
      cp $MARBLE_DEST/plugins/*.dll bin/
      cp /$sysdrive/Qt/$QT_VER/plugins/imageformats/*.dll plugins/imageformats/
      cp /$sysdrive/Qt/$QT_VER/plugins/imageformats/*.dll bin/
    fi
    if [[ "$DEBUG_NO_STRIP" == "" ]]; then
      echo "Stripping debug symbols from binaries and libraries ..."
      strip $bdlibdir/*.dll
      strip $bindir/*.exe
      strip $ddir/*.exe
      strip bin/*.dll
      strip bin/*.exe
      strip plugins/*.dll
      strip plugins/imageformats/*.dll
    fi

    # typical work flow using generated component fragments from heat.exe:
    # heat.exe dir $pkgdir -gg -ke -sfrag -nologo -out "${pkgdir}.wxs" -template:product
    # tail +4c $out > $tmp, dos2unix $tmp
    # wixtool.exe splice -i prod.wxs -o out.wxs Directory:DirName=heat.wxs:Directory:dirname
    # wixtool.exe splice -i out.wxs -o final.wxs Feature:MainApplication=heat.wxs:Feature:ProductFeature
    # light.exe -sloc -out vid.wixout -xo -cc cabcache WixUI_Custom.wixobj vidalia.wixobj -ext /wix/WixUIExtension.dll
    # light.exe "-cultures:es-es;de-de;en-us" -loc WixUI_es-es.wxl -loc WixUI_de-de.wxl -loc WixUI_en-us.wxl -loc vidalia_es.wxl -loc vidalia_de.wxl -loc vidalia_en.wxl -out test.msi -cc cabcache -reusecab vid.wixout

    cp pkg/win32/*.vbs ./
    cp $WIXSRC_WXLDIR/*.wxl ./
    cp pkg/win32/*.wxl ./
    candle.exe pkg/win32/*.wxs
    light.exe -out vidalia.msi vidalia.wixobj WixUI_Custom.wixobj $WIX_ALL_LOC_LINK -ext $WIX_UI
    if [ -f vidalia.msi ]; then
      cp vidalia.msi $bundledir
      cp vidalia.msi ../pkg/
      cp -a bin ../pkg/
      ls -l vidalia.msi
    else
      echo "ERROR: unable to build vidalia MSI installer."
    fi
    # the null LCID/codepage should actually be run through ascii filter to be sure.
    cp WixUI_en-us.wxl WixUI_nullcp.wxl
    cp vidalia_en.wxl vidalia_nullcp.wxl
    light.exe -out vidalia-intl.msi vidalia.wixobj WixUI_Custom.wixobj -loc WixUI_nullcp.wxl -loc vidalia_nullcp.wxl -ext $WIX_UI
    if [ -f vidalia-intl.msi ]; then
      export BASEMSI=vidalia-intl.msi
    fi
    if [[ "$BUILD_IND_LANGS" == "yes" ]]; then
      for LANG in $VIDALIA_LANGS; do
        WIXCULTURE=$DEF_WXL_LANG
        for WIXLANG in $WXL_LANGS; do
          pre=$(echo $WIXLANG | sed 's/-.*//')
          if [[ "$pre" == "$LANG" ]]; then
            WIXCULTURE=$WIXLANG
          fi
        done
        outfile="vidalia-${LANG}.msi"
        echo "Linking localized $outfile ..."
        light.exe -out $outfile vidalia.wixobj WixUI_Custom.wixobj -cultures:$WIXCULTURE -loc "vidalia_${LANG}.wxl" -ext $WIX_UI
        if [ -f $outfile ]; then
          cp $outfile $bundledir
          cp $outfile ../pkg/
          ls -l $outfile
          if [ -f $BASEMSI ]; then
            echo "Adding language $LANG as transform against minimal MSI package ..."
            cscript.exe mktransform.vbs "$LANG" "$BASEMSI" "$outfile"
          fi
        else
          echo "ERROR: unable to link localized $outfile vidalia MSI installer."
        fi
      done
      if [ -f $BASEMSI ]; then
        cp $BASEMSI ../pkg/
      fi
    fi
  fi
  cd /src/pkg
  echo "Copying various package dependencies into place ..."
  cp $WIXSRC_WXLDIR/*.wxl ./
  cp /src/$VIDALIA_DIR/src/tools/wixtool/wixtool.exe ./
  cp /src/$VIDALIA_DIR/pkg/win32/*.vbs ./
  cp /src/$VIDALIA_DIR/pkg/win32/*.wxs ./
  cp /src/$VIDALIA_DIR/pkg/win32/*.wxl ./
  cp ../torvm-w32/tor-icon-32.ico ./torvm.ico
  cp ../torvm-w32/tor-icon-32.ico ./tor.ico
  # DONT STRIP PY2EXEs!
  cp $thandir/Thandy.exe bin/
  cp /src/$TOR_DIR/bin/*.exe bin/
  cp /src/$TOR_DIR/contrib/*.ico ./
  cp /src/$TOR_DIR/share/tor/geoip ./
  cp /src/$TOR_DIR/src/config/torrc.sample ./
  for FNAME in README Usage.html Authors ChangeLog LICENSE; do
    cp /src/$TOR_DIR/$FNAME ./
  done
  cp -a $ddir ./
  # XXX replace this with Matt's torbutton NSIS magic
  candle.exe *.wxs

  echo "Building Tor Vidalia bundle license docs package ..."
  cp -a $licensedir ./
  find License -type f -exec unix2dos {} \;
  heat.exe dir License -gg -ke -sfrag -nologo -out license-dir.wxs -template:product
  if [ ! -f license-dir.wxs ]; then
    echo "Failed to generate directory tree component for $licensedir ."
  else
    # whatever WiX is putting in those first four bytes causes parser havoc
    tail +4c license-dir.wxs > license-dir.wxs.tmp; dos2unix license-dir.wxs.tmp; cat license-dir.wxs.tmp > license-dir.wxs; rm -f license-dir.wxs.tmp
    wixtool.exe splice -i license.wxs -o license-tmpdir.wxs Directory:ProgramsInstDir=license-dir.wxs:Directory:License
    wixtool.exe splice -i license-tmpdir.wxs -o license-all.wxs Feature:MainApplication=license-dir.wxs:Feature:ProductFeature
    rm -f license-tmpdir.wxs
    candle.exe license-all.wxs
    echo "Linking Tor Vidalia bundle license docs package ..."
    light.exe -out license.msi WixUI_Custom.wixobj license-all.wixobj $WIX_ALL_LOC_LINK -ext $WIX_UI
  fi

  echo "Linking torvm MSI installer package ..."
  light.exe -out torvm.msi WixUI_Custom.wixobj torvm.wixobj $WIX_ALL_LOC_LINK -ext $WIX_UI
  if [ -f torvm.msi ]; then
    cp torvm.msi $bundledir
    ls -l torvm.msi
  else
    echo "ERROR: unable to build Tor VM MSI installer."
  fi
  echo "Linking minimal zero codepage MSI installer package ..."
  light.exe -out torvm-intl.msi WixUI_Custom.wixobj torvm.wixobj -loc WixUI_nullcp.wxl -loc vidalia_nullcp.wxl -ext $WIX_UI
  if [ -f torvm-intl.msi ]; then
    export BASEMSI=torvm-intl.msi
  else
    echo "ERROR: unable to link minimal zero codepage Tor VM MSI installer."
    export BASEMSI=""
  fi
  if [[ "$BUILD_IND_LANGS" == "yes" ]]; then
    for LANG in $VIDALIA_LANGS; do
      WIXCULTURE=$DEF_WXL_LANG
      for WIXLANG in $WXL_LANGS; do
        pre=$(echo $WIXLANG | sed 's/-.*//')
        if [[ "$pre" == "$LANG" ]]; then
          WIXCULTURE=$WIXLANG
        fi 
      done 
      outfile="torvm-${LANG}.msi"
      echo "Linking localized $outfile ..."
      light.exe -out $outfile torvm.wixobj WixUI_Custom.wixobj -cultures:$WIXCULTURE -loc "vidalia_${LANG}.wxl" -ext $WIX_UI
      if [ -f $outfile ]; then
        cp $outfile $bundledir
        ls -l $outfile
        if [ -f "$BASEMSI" ]; then
          echo "Adding language $LANG as transform against minimal MSI package ..."
          cscript.exe mktransform.vbs "$LANG" "$BASEMSI" "$outfile"
        fi
      else
        echo "ERROR: unable to link localized $outfile MSI installer."
      fi
    done
  fi
  if [ -f "$BASEMSI" ]; then
    cp "$BASEMSI" $bundledir
  fi

  echo "Linking tor MSI installer package ..."
  light.exe -out tor.msi WixUI_Custom.wixobj tor.wixobj $WIX_ALL_LOC_LINK -ext $WIX_UI
  if [ -f tor.msi ]; then
    cp tor.msi $bundledir
    ls -l tor.msi
  else
    echo "ERROR: unable to build Tor MSI installer."
  fi
  echo "Linking minimal zero codepage MSI installer package ..."
  light.exe -out tor-intl.msi WixUI_Custom.wixobj tor.wixobj -loc WixUI_nullcp.wxl -loc vidalia_nullcp.wxl -ext $WIX_UI
  if [ -f tor-intl.msi ]; then
    export BASEMSI=tor-intl.msi
  else
    echo "ERROR: unable to link minimal zero codepage Tor VM MSI installer."
    export BASEMSI=""
  fi
  if [[ "$BUILD_IND_LANGS" == "yes" ]]; then
    for LANG in $VIDALIA_LANGS; do
      WIXCULTURE=$DEF_WXL_LANG
      for WIXLANG in $WXL_LANGS; do
        pre=$(echo $WIXLANG | sed 's/-.*//')
        if [[ "$pre" == "$LANG" ]]; then
          WIXCULTURE=$WIXLANG
        fi 
      done 
      outfile="tor-${LANG}.msi"
      echo "Linking localized $outfile ..."
      light.exe -out $outfile tor.wixobj WixUI_Custom.wixobj -cultures:$WIXCULTURE -loc "vidalia_${LANG}.wxl" -ext $WIX_UI
      if [ -f $outfile ]; then
        cp $outfile $bundledir
        ls -l $outfile
        if [ -f "$BASEMSI" ]; then
          echo "Adding language $LANG as transform against minimal MSI package ..."
          cscript.exe mktransform.vbs "$LANG" "$BASEMSI" "$outfile"
        fi
      else
        echo "ERROR: unable to link localized $outfile MSI installer."
      fi
    done
  fi
  if [ -f "$BASEMSI" ]; then
    cp "$BASEMSI" $bundledir
  fi

  echo "Linking polipo MSI installer package ..."
  light.exe -out polipo.msi WixUI_Custom.wixobj polipo.wixobj $WIX_ALL_LOC_LINK -ext $WIX_UI
  if [ -f polipo.msi ]; then
    cp polipo.msi $bundledir
    ls -l polipo.msi
  else
    echo "ERROR: unable to build polipo MSI installer."
  fi
  echo "Linking minimal zero codepage MSI installer package ..."
  light.exe -out polipo-intl.msi WixUI_Custom.wixobj polipo.wixobj -loc WixUI_nullcp.wxl -loc vidalia_nullcp.wxl -ext $WIX_UI
  if [ -f polipo-intl.msi ]; then
    export BASEMSI=polipo-intl.msi
  else
    echo "ERROR: unable to link minimal zero codepage Polipo MSI installer."
    export BASEMSI=""
  fi
  if [[ "$BUILD_IND_LANGS" == "yes" ]]; then
    for LANG in $VIDALIA_LANGS; do
      WIXCULTURE=$DEF_WXL_LANG
      for WIXLANG in $WXL_LANGS; do
        pre=$(echo $WIXLANG | sed 's/-.*//')
        if [[ "$pre" == "$LANG" ]]; then
          WIXCULTURE=$WIXLANG
        fi
      done 
      outfile="polipo-${LANG}.msi"
      echo "Linking localized $outfile ..."
      light.exe -out $outfile polipo.wixobj WixUI_Custom.wixobj -cultures:$WIXCULTURE -loc "vidalia_${LANG}.wxl" -ext $WIX_UI
      if [ -f $outfile ]; then
        cp $outfile $bundledir
        ls -l $outfile
        if [ -f "$BASEMSI" ]; then
          echo "Adding language $LANG as transform against minimal MSI package ..."
          cscript.exe mktransform.vbs "$LANG" "$BASEMSI" "$outfile"
        fi
      else
        echo "ERROR: unable to link localized $outfile MSI installer."
      fi
    done
  fi
  if [ -f "$BASEMSI" ]; then
    cp "$BASEMSI" $bundledir
  fi

  if [ -f /src/$TORBUTTON_FILE ]; then
    cp /src/$TORBUTTON_FILE torbutton.xpi
    echo "Linking torbutton MSI installer package ..."
    light.exe -out torbutton.msi WixUI_Custom.wixobj torbutton.wixobj $WIX_ALL_LOC_LINK -ext $WIX_UI
    if [ -f torbutton.msi ]; then
      cp torbutton.msi $bundledir
      ls -l torbutton.msi
    else
      echo "ERROR: unable to build torbutton MSI installer."
    fi
    echo "Linking minimal zero codepage MSI installer package ..."
    light.exe -out torbutton-intl.msi WixUI_Custom.wixobj torbutton.wixobj -loc WixUI_nullcp.wxl -loc vidalia_nullcp.wxl  -ext $WIX_UI
    if [ -f torbutton-intl.msi ]; then
      export BASEMSI=torbutton-intl.msi
    else
      echo "ERROR: unable to link minimal zero codepage TorButton MSI installer."
      export BASEMSI=""
    fi
    if [[ "$BUILD_IND_LANGS" == "yes" ]]; then
      for LANG in $VIDALIA_LANGS; do
        WIXCULTURE=$DEF_WXL_LANG 
        for WIXLANG in $WXL_LANGS; do
          pre=$(echo $WIXLANG | sed 's/-.*//')
          if [[ "$pre" == "$LANG" ]]; then
            WIXCULTURE=$WIXLANG
          fi
        done
        outfile="torbutton-${LANG}.msi"
        echo "Linking localized $outfile ..."
        light.exe -out $outfile torbutton.wixobj WixUI_Custom.wixobj -cultures:$WIXCULTURE -loc "vidalia_${LANG}.wxl" -ext $WIX_UI
        if [ -f $outfile ]; then
          cp $outfile $bundledir
          ls -l $outfile
          if [ -f "$BASEMSI" ]; then
            echo "Adding language $LANG as transform against minimal MSI package ..."
            cscript.exe mktransform.vbs "$LANG" "$BASEMSI" "$outfile"
          fi
        else
          echo "ERROR: unable to link localized $outfile MSI installer."
        fi
      done
    fi
    if [ -f "$BASEMSI" ]; then
      cp "$BASEMSI" $bundledir
    fi
  fi

  echo "Linking thandy MSI installer package ..."
  light.exe -out thandy.msi WixUI_Custom.wixobj thandy.wixobj $WIX_ALL_LOC_LINK -ext $WIX_UI
  if [ -f thandy.msi ]; then
    cp thandy.msi $bundledir
    ls -l thandy.msi
  else
    echo "ERROR: unable to build Thandy MSI installer."
  fi
  echo "Linking minimal zero codepage MSI installer package ..."
  light.exe -out thandy-intl.msi WixUI_Custom.wixobj thandy.wixobj -loc WixUI_nullcp.wxl -loc vidalia_nullcp.wxl -ext $WIX_UI
  if [ -f thandy-intl.msi ]; then
    export BASEMSI=thandy-intl.msi
  else
    echo "ERROR: unable to link minimal zero codepage Thandy MSI installer."
    export BASEMSI=""
  fi
  if [[ "$BUILD_IND_LANGS" == "yes" ]]; then
    for LANG in $VIDALIA_LANGS; do
      WIXCULTURE=$DEF_WXL_LANG 
      for WIXLANG in $WXL_LANGS; do
        pre=$(echo $WIXLANG | sed 's/-.*//')
        if [[ "$pre" == "$LANG" ]]; then
          WIXCULTURE=$WIXLANG
        fi
      done
      outfile="thandy-${LANG}.msi"
      echo "Linking localized $outfile ..."
      light.exe -out $outfile thandy.wixobj WixUI_Custom.wixobj -cultures:$WIXCULTURE -loc "vidalia_${LANG}.wxl" -ext $WIX_UI
      if [ -f $outfile ]; then
        cp $outfile $bundledir
        ls -l $outfile
        if [ -f "$BASEMSI" ]; then
          echo "Adding language $LANG as transform against minimal MSI package ..."
          cscript.exe mktransform.vbs "$LANG" "$BASEMSI" "$outfile"
        fi
      else
        echo "ERROR: unable to link localized $outfile MSI installer."
      fi
    done
  fi
  if [ -f "$BASEMSI" ]; then
    cp "$BASEMSI" $bundledir
  fi

  echo "Creating Tor VM bundle installer executable ..."
  makensis.exe bundle.nsi
  if [ -f TorVMBundle.exe ]; then
    cp TorVMBundle.exe $bundledir
    ls -l TorVMBundle.exe
  else
    echo "ERROR: unable to build Tor VM executable bundle installer."
  fi

  echo "Creating Tor VM network installer executable ..."
  makensis.exe netinst.nsi
  if [ -f TorVMNetInstaller.exe ]; then
    cp TorVMNetInstaller.exe $bundledir
    ls -l TorVMNetInstaller.exe
  else
    echo "ERROR: unable to build Tor VM executable network installer."
  fi

  echo "Creating self-extracting Tor VM archive ..."
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

  pkgbuilt PACKAGES_BUILT
fi


if [[ "$BUILD_SCP_USER" != "" ]]; then
  echo "Transferring build to destination ${BUILD_SCP_HOST}:${bld_dsub} ..."
  scp -o BatchMode=yes -o CheckHostIP=no -o StrictHostKeyChecking=no \
      -r $ddir $brootdir "${BUILD_SCP_USER}@${BUILD_SCP_HOST}:${bld_dsub}/"
fi

echo "DONE."
exit 0

fi
