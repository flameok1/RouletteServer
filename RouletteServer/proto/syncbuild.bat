@echo off

set "sourceFolder=D:\git\RouletteWeb\public\proto"
set "destinationFolder=D:\git\RouletteServer\RouletteServer\proto"

echo Copying files from "%sourceFolder%" to "%destinationFolder%".
xcopy "%sourceFolder%" "%destinationFolder%" /e /i /y

echo Copying complete.
echo.

echo Running protoc on files in "%destinationFolder%".
for /f "delims=" %%i in ('dir /b "%destinationFolder%\*.proto"') do (
    ..\..\vcpkg_installed\x64-windows\tools\protobuf\protoc -I=. --cpp_out=. "%%i"
)

echo All tasks are complete.
pause