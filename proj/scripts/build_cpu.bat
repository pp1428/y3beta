@echo off
REM Сборка CPU-версий (компилятор MinGW-w64 gcc должен быть в PATH)
cd /d "%~dp0\..\cpu"

echo === det_cpu.c ===
gcc -O2 -o det_cpu.exe det_cpu.c -lm
if errorlevel 1 goto :err

echo === bruteforce_cpu.c ===
gcc -O2 -o bruteforce_cpu.exe bruteforce_cpu.c -lm
if errorlevel 1 goto :err

echo.
echo Готово. Запускайте:
echo   cpu\det_cpu.exe
echo   cpu\bruteforce_cpu.exe
goto :eof

:err
echo СБОРКА ЗАВЕРШИЛАСЬ С ОШИБКОЙ
exit /b 1
