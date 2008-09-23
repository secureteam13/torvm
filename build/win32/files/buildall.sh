#!/bin/bash
export PATH=.:/usr/local/bin:/usr/bin:/bin:/mingw/bin:/c/WINDOWS/system32:/c/WINDOWS:/c/WINDOWS/System32/Wbem
export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/lib:/mingw/lib
export ddir=/c/Tor_VM
export libdir="${ddir}/lib"
export bindir="${ddir}/bin"
export confdir="${ddir}/config"
export statedir="${ddir}/state"

foreach dir in $ddir $libdir $bindir $confdir $statedir; do
  if [ ! -d $dir ]; then
    mkdir $dir
  fi
done

if [ -d /usr/usr ]; then
  # ahh, gotta love the msys /usr <-> / equivalence hack...
  cd /usr/usr
  mv local ../
  rmdir usr
fi
cp /usr/local/bin/aclocal-* /bin/aclocal
cp /usr/local/bin/autoconf-* /bin/autoconf
cp /usr/local/bin/autoheader-* /bin/autoheader
cp /usr/local/bin/automake-* /bin/automake

cp /bin/msys-z.dll $libdir/
cp /bin/msys-1.0.dll $libdir/


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


echo "Building zlib ..."
cd /usr/src
tar zxvf zlib-1.2.3.tar.gz
cd zlib-1.2.3
./configure --prefix=/usr
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
          echo "Found DDK install at $DDKDIR , using env script $DDKENV"
	  cp "${$ddkbase}/tools/devcon/i386/devcon.exe" $bindir
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
aclocal && autoheader && autoconf && automake --add-missing --copy
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
cd tap-win32
echo "call $DDKENV $DDKDIR fre WXP" > dobuild.bat
echo "build -cef" >> dobuild.bat
echo "exit 0" >> dobuild.bat
cmd.exe /k dobuild.bat
if (( $? != 0 )); then
  echo "ERROR: openvpn tap-win32 build failed." >&2
  exit 1
fi


echo "Building WinPcap ..."
cd /usr/src
tar zxvf WpcapSrc_4_1_beta4.tar.gz
cd WpcapSrc_4_1_beta4
patch -p1 < ../winpcap-tor-device-mods.patch 2>/dev/null
echo "KILL ME";sleep 3600


echo "Building qemu ..."
cd /usr/src
tar zxvf qemu-0.9.1.tar.gz
cd qemu-0.9.1
patch -p1 < ../qemu-winpcap-0.9.1.patch 2> /dev/null
if (( $? != 0 )); then
  echo "ERROR: Qemu patch failed." >&2
  exit 1
fi
./configure --prefix=/usr --interp-prefix=qemu-%M \
  --enable-uname-release="Tor VM 2.6-alpha i386" \
  --disable-werror \
  --disable-kqemu \
  --disable-system \
  --disable-vnc-tls \
  --extra-cflags="-DHAVE_INTSZ_TYPES -I. -I.. -I/usr/include -I/usr/local/include -I/usr/src/WpdPack/Include -I/usr/src/pthreads-w32 -I/usr/include/SDL" \
  --extra-ldflags="-L/usr/lib -L/usr/local/lib -L/usr/src/WpdPack/Lib -L/usr/src/pthreads-w32" \
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
cp /usr/src/add/* $libdir/
#cp /usr/src/WinPcap-4.1-files/* $ddir/

echo "DONE."
exit 0
