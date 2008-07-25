REM
set ISODRV=D:\
set DDRV=C:\
set DDIR=MinGW
set MDIR=msys
set MVER=1.0

IF EXIST %DDRV%%DDIR% GOTO NOINSTALL

cd /d %DDRV%
md %DDIR%
cd %DDIR%
md bin
cd /d %ISODRV%
cd bin
copy *.* %DDRV%%DDIR%\bin\
cd /d %DDRV%
set PATH=%DDRV%%DDIR%\bin;%PATH%
md %MDIR%
cd %MDIR%
md %MVER%
cd %MVER%
md dl
cd /d %ISODRV%
cd dl
copy *.* %DDRV%%MDIR%\%MVER%\dl\
cd /d %DDRV%
cd %MDIR%\%MVER%\dl\
bzip2 -d *.bz2
gzip -d *.gz
cd /d %DDRV%
cd %MDIR%\%MVER%
bsdtar xvf dl\msysCORE-1.0.11-2007.01.19-1.tar
bsdtar xvf dl\mingw-runtime-3.14.tar
bsdtar xvf dl\bash-3.1-MSYS-1.0.11-1.tar
bsdtar xvf dl\mingw32-make-3.81-20080326-3.tar
bsdtar xvf dl\binutils-2.18.50-20080109-2.tar
bsdtar xvf dl\diffutils-2.8.7-MSYS-1.0.11-1.tar
bsdtar xvf dl\gcc-core-3.4.5-20060117-3.tar
bsdtar xvf dl\gcc-g++-3.4.5-20060117-3.tar
bsdtar xvf dl\libtool1.5-1.5.25a-1-bin.tar
bsdtar xvf dl\w32api-3.11.tar
cd /d %ISODRV%
cd bin
copy fstab %DDRV%%MDIR%\%MVER%\etc\
cd /d %DDRV%
cd %MDIR%\%MVER%\
md src
cd /d %ISODRV%
cd dl\src
copy *.* %DDRV%%MDIR%\%MVER%\src\
cd %MDIR%\%MVER%
set PATH=%DDRV%%MDIR%\%MVER%\bin;%DDRV%%MDIR%\%MVER%;%PATH%
set BUILDER=/usr/src/buildall.sh
set WD=C:\msys\1.0\bin\
set PATH=%WD%;%PATH%

%WD%bash %BUILDER%
EXIT

:NOINSTALL
ECHO "Found existing install directories.  Delete any previous install targets and try again."
EXIT
