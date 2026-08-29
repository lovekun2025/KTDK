@echo off
setlocal enabledelayedexpansion

REM ============================================================
REM  One-click packaging: build Host + BootApp, output to HostApp\
REM  Usage: double-click this file, then run HostApp\Host.exe
REM  If you change PC, update QT_DIR / MINGW_DIR below
REM ============================================================

set "QT_DIR=D:\application\worker\Qt_Creator\Qt5.12.8\download\5.12.8\mingw73_64"
set "MINGW_DIR=D:\application\worker\Qt_Creator\Qt5.12.8\download\Tools\mingw730_64"
set "PROJ_ROOT=%~dp0"
set "OUT_DIR=%PROJ_ROOT%HostApp"
set "PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%"

echo [1/5] Building Host.exe ...
mkdir "%PROJ_ROOT%build_tmp_host" 2>nul
pushd "%PROJ_ROOT%build_tmp_host"
qmake "%PROJ_ROOT%Data_Collect_Host computer\Host\Host.pro" -spec win32-g++ "CONFIG+=release" >nul
mingw32-make -j8 >nul
popd
if not exist "%PROJ_ROOT%build_tmp_host\release\Host.exe" (
    echo   ERROR: build Host failed.
    pause
    exit /b 1
)
echo   OK

echo [2/5] Building BootApp.exe ...
mkdir "%PROJ_ROOT%build_tmp_boot" 2>nul
pushd "%PROJ_ROOT%build_tmp_boot"
qmake "%PROJ_ROOT%UP_programe\Completed\SerialPortYmodem\SerialPortYmodem.pro" -spec win32-g++ "CONFIG+=release" >nul
mingw32-make -j8 >nul
popd
if not exist "%PROJ_ROOT%UP_programe\Completed\BootApp.exe" (
    echo   ERROR: build BootApp failed.
    pause
    exit /b 1
)
echo   OK

echo [3/5] Assembling package ...
mkdir "%OUT_DIR%" 2>nul
copy /y "%PROJ_ROOT%build_tmp_host\release\Host.exe" "%OUT_DIR%\" >nul
copy /y "%PROJ_ROOT%UP_programe\Completed\BootApp.exe" "%OUT_DIR%\" >nul
if not exist "%OUT_DIR%\Protocols" (
    xcopy /e /y /i "%PROJ_ROOT%Data_Collect_Host computer\Protocols" "%OUT_DIR%\Protocols" >nul
)
echo   OK

echo [4/5] Deploying Qt DLLs ...
windeployqt "%OUT_DIR%\Host.exe" --no-translations >nul
windeployqt "%OUT_DIR%\BootApp.exe" --no-translations >nul
copy /y "%QT_DIR%\bin\libgcc_s_seh-1.dll" "%OUT_DIR%\" >nul
copy /y "%QT_DIR%\bin\libgcc_s_dw2-1.dll" "%OUT_DIR%\" >nul
copy /y "%QT_DIR%\bin\libstdc++-6.dll" "%OUT_DIR%\" >nul
copy /y "%QT_DIR%\bin\libwinpthread-1.dll" "%OUT_DIR%\" >nul
REM bin->sou 转换工具
if not exist "%OUT_DIR%\文件转换" mkdir "%OUT_DIR%\文件转换" 2>nul
copy /y "%PROJ_ROOT%文件转换\CreateSoul.exe" "%OUT_DIR%\文件转换\" >nul 2>nul
copy /y "%PROJ_ROOT%文件转换\CreateSoul.bat" "%OUT_DIR%\文件转换\" >nul 2>nul
echo   OK

echo [5/5] Cleaning up ...
rmdir /s /q "%PROJ_ROOT%build_tmp_host" 2>nul
rmdir /s /q "%PROJ_ROOT%build_tmp_boot" 2>nul
echo   OK

echo.
echo ============================================================
echo   Packaging DONE!
echo   Output: %OUT_DIR%
echo   Run Host.exe, then menu Tools - Firmware Upgrade opens BootApp
echo ============================================================
pause
