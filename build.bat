@echo off
setlocal

:: 1. Validate Developer Environment
if "%VCINSTALLDIR%"=="" (
    echo Error: Visual Studio developer environment not detected.
    echo Please run this script from a Developer Command Prompt.
    exit /b 1
)

:: 2. Determine Architecture
set ARCH=%VSCMD_ARG_TGT_ARCH%
if "%ARCH%"=="" (
    if "%Platform%"=="x64" set ARCH=x64
    if "%Platform%"=="X64" set ARCH=x64
    if "%Platform%"=="x86" set ARCH=x86
    if "%Platform%"=="X86" set ARCH=x86
)
if "%ARCH%"=="" set ARCH=x64

:: 3. Define Directories (Aligned with BuildAll.ps1 structure)
set BUILD_ROOT=build\Batch\%ARCH%\Release
set DIST_ROOT=dist\%ARCH%\Release
set INTERMEDIATE_DIR=obj\Batch\%ARCH%\Release

if not exist "%BUILD_ROOT%" mkdir "%BUILD_ROOT%"
if not exist "%DIST_ROOT%" mkdir "%DIST_ROOT%"
if not exist "%INTERMEDIATE_DIR%" mkdir "%INTERMEDIATE_DIR%"

echo --- Building StackyNG [Batch Method] ---
echo Target: %ARCH% | Config: Release

:: 4. Compile Resources
rc /nologo /fo "%INTERMEDIATE_DIR%\stacky.res" "src\stacky.rc"

:: 5. Compile Sources
echo Compiling sources...
set SOURCES=stacky.cpp app.cpp cache_manager.cpp bmp.cpp config.cpp util.cpp logger.cpp
for %%f in (%SOURCES%) do (
    cl /nologo /c /O2 /MP /W3 /EHsc /std:c++17 /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS /Fo"%INTERMEDIATE_DIR%\\" "src\%%f"
)

:: 6. Link
echo Linking...
link /nologo "%INTERMEDIATE_DIR%\*.obj" "%INTERMEDIATE_DIR%\stacky.res" /OUT:"%BUILD_ROOT%\stacky.exe" /SUBSYSTEM:WINDOWS user32.lib gdi32.lib shell32.lib ole32.lib comctl32.lib windowscodecs.lib advapi32.lib uxtheme.lib dwmapi.lib

:: 7. Deployment & Compression
if %ERRORLEVEL% equ 0 (
    echo Build successful. Deploying...
    copy "%BUILD_ROOT%\stacky.exe" "%DIST_ROOT%\stacky.exe" >nul
    
    echo Compressing...
    powershell -Command "Compress-Archive -Path '%DIST_ROOT%\stacky.exe' -DestinationPath '%DIST_ROOT%\stacky-%ARCH%.zip' -Force"
    echo Done. Files available in %DIST_ROOT%
) else (
    echo Build failed.
)

endlocal