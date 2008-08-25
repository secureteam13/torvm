#!/bin/bash
export PATH=.:/usr/local/bin:/usr/bin:/bin:/mingw/bin:/c/WINDOWS/system32:/c/WINDOWS:/c/WINDOWS/System32/Wbem
export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/lib:/mingw/lib
export ddir=/c/Tor_VM
mkdir $ddir

cp /usr/bin/msys-z.dll $ddir/
cp /usr/bin/msys-1.0.dll $ddir/

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
cp pthreadGC2.dll $ddir/


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
cp /usr/bin/SDL.dll $ddir/


echo "Extracting WinPcap runtime files ..."
cd /usr/src
tar zxvf WinPcap-4.1-files.tar.gz

echo "Extracting WinPcap developer files ..."
cd /usr/src
tar zxvf WpdPack_4_1_beta4.tar.gz


echo "Building zlib ..."
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
cp i386-softmmu/qemu.exe $ddir/
cp pc-bios/bios.bin $ddir/
cp pc-bios/vgabios.bin $ddir/
cp pc-bios/vgabios-cirrus.bin $ddir/
cp /usr/src/add/* $ddir/

# still need to handle WinPcap installation; perhaps include link to 4.1 installer exe
# or wget it at runtime if not present.
# cp /usr/src/WinPcap-4.1-files/* $ddir/

echo "DONE."
exit 0
