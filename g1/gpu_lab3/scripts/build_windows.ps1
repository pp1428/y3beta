$ErrorActionPreference = 'Stop'
Set-Location (Split-Path -Parent $PSScriptRoot)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
Write-Host "Готово. Запуск: .\build\Release\lab3.exe demo"
