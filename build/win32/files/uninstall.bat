@ECHO OFF
IF EXIST "%USERPROFILE%\Local Settings\Application Data\Thandy\TorVM Updates" (
  cd "%USERPROFILE%\Local Settings\Application Data\Thandy\TorVM Updates"
  for %%f in (*.msi) do msiexec /x %%f /qn
  for %%f in (*.msi) do del /F %%f
  cd ..
  rmdir "TorVM Updates"
)
IF EXIST "%USERPROFILE%\Local Settings\Application Data\Thandy\Tor Updates" (
  cd "%USERPROFILE%\Local Settings\Application Data\Thandy\Tor Updates"
  for %%f in (*.msi) do msiexec /x %%f /qn
  for %%f in (*.msi) do del /F %%f
  cd ..
  rmdir "Tor Updates"
)
IF EXIST "%USERPROFILE%\Local Settings\Application Data\Thandy\Polipo Updates" (
  cd "%USERPROFILE%\Local Settings\Application Data\Thandy\Polipo Updates"
  for %%f in (*.msi) do msiexec /x %%f /qn
  for %%f in (*.msi) do del /F %%f
  cd ..
  rmdir "Polipo Updates"
)
IF EXIST "%USERPROFILE%\Local Settings\Application Data\Thandy\TorButton Updates" (
  cd "%USERPROFILE%\Local Settings\Application Data\Thandy\TorButton Updates"
  for %%f in (*.msi) do msiexec /x %%f /qn
  for %%f in (*.msi) do del /F %%f
  cd ..
  rmdir "TorButton Updates"
)
IF EXIST "%USERPROFILE%\Local Settings\Application Data\Thandy\Vidalia Updates" (
  cd "%USERPROFILE%\Local Settings\Application Data\Thandy\Vidalia Updates"
  for %%f in (*.msi) do msiexec /x %%f /qn
  for %%f in (*.msi) do del /F %%f
  cd ..
  rmdir "Vidalia Updates"
)
IF EXIST "%USERPROFILE%\Local Settings\Application Data\Thandy\Vidalia Marble Updates" (
  cd "%USERPROFILE%\Local Settings\Application Data\Thandy\Vidalia Marble Updates"
  for %%f in (*.msi) do msiexec /x %%f /qn
  for %%f in (*.msi) do del /F %%f
  cd ..
  rmdir "Vidalia Marble Updates"
)
IF EXIST %PROGRAMFILES%\TorInstPkgs (
  cd %PROGRAMFILES%\TorInstPkgs
  for %%f in (*.msi) do msiexec /x %%f /qn
  for %%f in (*.msi) do del /F %%f
  cd ..
  rmdir TorInstPkgs
)
IF EXIST "%USERPROFILE%\Local Settings\Application Data\TorInstPkgs" (
  cd "%USERPROFILE%\Local Settings\Application Data\TorInstPkgs"
  for %%f in (*.msi) do msiexec /x %%f /qn
  for %%f in (*.msi) do del /F %%f
  cd ..
  rmdir TorInstPkgs
)
IF EXIST "%USERPROFILE%\Desktop\Uninstall_Tor.bat" (
  del /F "%USERPROFILE%\Desktop\Uninstall_Tor.bat"
)
