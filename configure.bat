@echo off
set VCVARS="E:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"

if exist %VCVARS% (
    echo Setting up MSVC Environment...
    call %VCVARS% x64 > nul
) else (
    echo "Error: vcvarsall.bat not found at %VCVARS%"
    exit /b 1
)

echo Configuring NexusScript (Node.js Source Build)...
cmake -G Ninja -B build -S .
