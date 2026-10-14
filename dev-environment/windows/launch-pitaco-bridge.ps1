#requires -Version 5.1
[CmdletBinding()]
param([string]$WorkspaceRoot = '')

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
if (-not $WorkspaceRoot) {
    $WorkspaceRoot = Split-Path -Parent (Split-Path -Parent $root)
}
$appDir = Join-Path $WorkspaceRoot 'PITACO_ESP32\arduino-pitaco-ble'
$app = Join-Path $appDir 'ble_client_gui.py'
$python = Join-Path $root '.local\pitaco-bridge\.venv\Scripts\python.exe'
if (-not (Test-Path -LiteralPath $app)) {
    throw "Bridge nao encontrado. Execute .\setup.ps1 -WithPitacoBridge."
}
if (-not (Test-Path -LiteralPath $python)) {
    throw "Ambiente Python do Bridge ausente. Execute .\setup.ps1 -WithPitacoBridge."
}
Write-Warning 'O aplicativo original pode pedir elevacao de administrador para instalar portas virtuais. Revise a solicitacao antes de aceitar.'
Write-Warning 'Este instalador nao instala o driver com0com nem atualiza firmware.'
Push-Location $appDir
try {
    & $python $app
    if ($LASTEXITCODE -ne 0) { throw "Bridge finalizou com codigo $LASTEXITCODE." }
} finally {
    Pop-Location
}
