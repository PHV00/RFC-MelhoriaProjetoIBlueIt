#requires -Version 5.1
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$WorkspaceRoot)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$app = Join-Path $WorkspaceRoot 'PITACO_ESP32\arduino-pitaco-ble\ble_client_gui.py'
$root = Split-Path -Parent $PSScriptRoot
$venvDir = Join-Path $root '.local\pitaco-bridge\.venv'
$venvPython = Join-Path $venvDir 'Scripts\python.exe'
$requirements = Join-Path $PSScriptRoot 'pitaco-bridge-requirements.txt'

if (-not (Test-Path -LiteralPath $app)) {
    throw "PITACO Bridge ausente em $app. Confira o clone do repositorio privado."
}
if (-not (Test-Path -LiteralPath $venvPython)) {
    $pythonExecutable = $null
    $pythonPrefix = @()
    $launcher = Get-Command py -ErrorAction SilentlyContinue
    if ($launcher) {
        try {
            & $launcher.Source -3.11 -c 'import sys; assert sys.version_info[:2] == (3,11)' 2>$null | Out-Null
            if ($LASTEXITCODE -eq 0) {
                $pythonExecutable = $launcher.Source
                $pythonPrefix = @('-3.11')
            }
        } catch { }
    }
    if (-not $pythonExecutable) {
        $cmd = Get-Command python -ErrorAction SilentlyContinue
        if ($cmd) {
            try {
                & $cmd.Source -c 'import sys; assert sys.version_info[:2] == (3,11)' 2>$null | Out-Null
                if ($LASTEXITCODE -eq 0) { $pythonExecutable = $cmd.Source }
            } catch { }
        }
    }
    if (-not $pythonExecutable) {
        throw 'Python 3.11 nao encontrado. Execute .\setup.ps1 -InstallTools, reabra o PowerShell e tente novamente.'
    }
    Write-Host '[PITACO] Criando ambiente virtual fora do repositorio privado.'
    & $pythonExecutable @pythonPrefix -m venv $venvDir
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao criar o ambiente virtual PITACO.' }
} else {
    Write-Host '[PITACO] Ambiente virtual existente preservado.'
}
Write-Host '[PITACO] Instalando/verificando bibliotecas Python isoladas.'
& $venvPython -m pip install --disable-pip-version-check -r $requirements
if ($LASTEXITCODE -ne 0) { throw 'Falha ao instalar as dependencias Python do PITACO Bridge.' }
Write-Host '[PITACO] Preparacao concluida. Nenhum firmware ou driver foi instalado.'
Write-Host 'Abra o programa: .\windows\launch-pitaco-bridge.ps1'
