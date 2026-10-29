@echo off
REM Сборка GPU-версий (OpenCL). Нужен MinGW-w64 gcc + заголовки/библиотека OpenCL.
REM
REM Если у вас уже есть папка с OpenCL SDK (например OLSDK из прошлых лаб,
REM как на скриншоте в проекте Игнатова_OpenCL), укажите путь к ней ниже
REM или передайте его переменной окружения OCL_SDK перед запуском скрипта:
REM     set OCL_SDK=C:\_tr\y3nogit\лб3япы\Игнатова_OpenCL\OLSDK
REM     scripts\build_gpu.bat
REM
REM Иначе скачайте и установите Khronos OpenCL SDK:
REM   https://github.com/KhronosGroup/OpenCL-SDK/releases
REM и укажите путь к распакованной папке ниже.

if "%OCL_SDK%"=="" set OCL_SDK=..\OLSDK

set INCLUDE_DIR=%OCL_SDK%\include
set LIB_DIR=%OCL_SDK%\lib\x86_64

cd /d "%~dp0\..\gpu"

echo Используется SDK: %OCL_SDK%
echo   include: %INCLUDE_DIR%
echo   lib:     %LIB_DIR%
echo.

echo === det_host.c ===
gcc -O2 -o det_host.exe det_host.c -I "%INCLUDE_DIR%" -L "%LIB_DIR%" -l OpenCL
if errorlevel 1 goto :err

echo === bf_host.c ===
gcc -O2 -o bf_host.exe bf_host.c -I "%INCLUDE_DIR%" -L "%LIB_DIR%" -l OpenCL
if errorlevel 1 goto :err

echo.
echo Готово. Запускайте ИЗ ПАПКИ gpu (важно - .cl файлы читаются по относительному пути!):
echo   cd gpu
echo   det_host.exe
echo   bf_host.exe
goto :eof

:err
echo.
echo СБОРКА ЗАВЕРШИЛАСЬ С ОШИБКОЙ.
echo Проверьте путь к OpenCL SDK (переменная OCL_SDK) - см. комментарии в начале файла.
exit /b 1
