@echo off
SETLOCAL

git submodule update --init
git submodule update --init --recursive

set GENERATOR="Visual Studio 17 2022"
set PLATFORM=x64
set CONFIGURATION=Debug
set BUILD_FOLDER=%APPVEYOR_BUILD_FOLDER%

set CMAKE_BIN_x86="C:\Program Files (x86)\CMake\bin\cmake.exe"
set CMAKE_BIN_x64="C:\Program Files\CMake\bin\cmake.exe"
IF EXIST %CMAKE_BIN_x64% (
	echo CMake 64-bit detected
	set CMAKE_BIN=%CMAKE_BIN_x64%
) ELSE (
	IF EXIST %CMAKE_BIN_x86% (
		echo CMake 32-bit detected
		set CMAKE_BIN=%CMAKE_BIN_x86%
	) ELSE (
		echo Cannot detect either %CMAKE_BIN_x86% or
		echo %CMAKE_BIN_x64% make sure CMake is installed
		EXIT /B 1
	)
)
echo Using CMake at %CMAKE_BIN%

cd %BUILD_FOLDER%
mkdir %BUILD_FOLDER%\build
cd %BUILD_FOLDER%\build
echo --- Running CMake configure ---
%CMAKE_BIN% -G %GENERATOR% -A %PLATFORM% %BUILD_FOLDER% -D DCCLITE_GUI_TOOLS=true
%CMAKE_BIN% -G %GENERATOR% -A %PLATFORM% %BUILD_FOLDER%

echo --- Building DCCLite ---
%CMAKE_BIN% --build . --config %CONFIGURATION%
%CMAKE_BIN% --build . --target install --config %CONFIGURATION%

echo Done!

ENDLOCAL