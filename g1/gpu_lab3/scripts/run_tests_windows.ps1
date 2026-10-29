$ErrorActionPreference = 'Stop'
Set-Location (Split-Path -Parent $PSScriptRoot)
$exe = Join-Path (Get-Location) 'build\Release\lab3.exe'
if (!(Test-Path $exe)) { $exe = Join-Path (Get-Location) 'build\lab3.exe' }
if (!(Test-Path $exe)) { throw 'Сначала запустите scripts\build_windows.ps1' }
& $exe test
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
