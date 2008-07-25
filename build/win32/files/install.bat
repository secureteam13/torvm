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
for %%f in (dl\*.tar) do bsdtar xvf %%f
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
