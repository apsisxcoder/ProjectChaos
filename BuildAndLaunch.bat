@echo off
setlocal

set UPROJECT="C:\Users\Apsisx\Documents\Unreal Projects\ProjectChaos\ProjectChaos.uproject"
set UE_ROOT=C:\Program Files\Epic Games\UE_5.7

echo ============================================
echo  Killing UnrealEditor if running...
echo ============================================
taskkill /IM UnrealEditor.exe /F >nul 2>&1
timeout /t 2 /nobreak >nul

echo ============================================
echo  Building ProjectChaosEditor (Win64, Development)
echo ============================================
call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" ProjectChaosEditor Win64 Development %UPROJECT% -waitmutex

if errorlevel 1 (
    echo.
    echo ============================================
    echo  BUILD FAILED - see errors above
    echo ============================================
    pause
    exit /b 1
)

echo ============================================
echo  Build succeeded. Launching UnrealEditor...
echo ============================================
start "" "%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" %UPROJECT%

exit /b 0
