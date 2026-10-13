#requires -Version 5.1
[CmdletBinding()]
param(
    [string]$WorkspaceRoot = "",
    [switch]$InstallTools,
    [switch]$InstallDocker,
    [switch]$PinBaselines,
    [switch]$VerifyOnly
)
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
if (-not $WorkspaceRoot) {
    # The dev-environment directory lives inside RFC; clones go beside RFC.
    $WorkspaceRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
}
$WorkspaceRoot = [IO.Path]::GetFullPath($WorkspaceRoot)
$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot "repositories.json") -Raw | ConvertFrom-Json
if ($VerifyOnly) {
    & (Join-Path $PSScriptRoot "windows\verify.ps1") -WorkspaceRoot $WorkspaceRoot
    return
}
if ($InstallTools) {
    & (Join-Path $PSScriptRoot "windows\install-tools.ps1") -InstallDocker:$InstallDocker
}
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw "Git indisponivel no PATH. Instale Git, reinicie o PowerShell e tente novamente."
}
if (-not (Test-Path -LiteralPath $WorkspaceRoot)) {
    New-Item -ItemType Directory -Path $WorkspaceRoot -Force | Out-Null
}
Write-Host "Workspace: $WorkspaceRoot"
foreach ($entry in $manifest.repositories) {
    $destination = Join-Path $WorkspaceRoot $entry.folder
    if (Test-Path -LiteralPath $destination) {
        if (-not (Test-Path -LiteralPath (Join-Path $destination ".git"))) {
            throw "Destino ocupado por pasta sem Git: $destination. Nada foi sobrescrito."
        }
        $origin = (& git -C $destination remote get-url origin 2>$null | Out-String).Trim()
        if ($LASTEXITCODE -ne 0 -or $origin -notmatch ([regex]::Escape($entry.name) + '(\.git)?$')) {
            throw "Remote origin diferente do esperado em $destination : $origin"
        }
        Write-Host "[PRESERVADO] $($entry.name). Sem pull/reset/checkout/stash."
        continue
    }
    Write-Host "[CLONAR] $($entry.name)"
    & git clone --branch $entry.branch -- $entry.url $destination
    if ($LASTEXITCODE -ne 0) { throw "Falha ao clonar $($entry.name)" }
    if ($PinBaselines) {
        & git -C $destination checkout --detach $entry.baselineCommit
        if ($LASTEXITCODE -ne 0) { throw "Falha ao fixar baseline $($entry.name)" }
    }
}
& (Join-Path $PSScriptRoot "windows\verify.ps1") -WorkspaceRoot $WorkspaceRoot
Write-Host "Git preparado. Compilacao Unity, API, dashboard e hardware ainda requerem validacao." -ForegroundColor Cyan
