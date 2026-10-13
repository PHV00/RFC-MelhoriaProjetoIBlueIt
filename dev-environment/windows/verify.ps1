#requires -Version 5.1
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$WorkspaceRoot)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$manifest = Get-Content -LiteralPath (Join-Path $root "repositories.json") -Raw | ConvertFrom-Json
Write-Host "== Verificacao nao destrutiva =="
foreach ($name in @("git","code","python","docker","winget")) {
    $found = Get-Command $name -ErrorAction SilentlyContinue
    Write-Host ("{0}: {1}" -f $name, $(if ($found) { "localizado" } else { "ausente no PATH" }))
}
$unity = Join-Path $env:ProgramFiles "Unity\Hub\Editor\2022.3.4f1\Editor\Unity.exe"
Write-Host ("Unity 2022.3.4f1: " + $(if (Test-Path $unity) { "localizado" } else { "nao localizado no caminho padrao" }))
foreach ($entry in $manifest.repositories) {
    $dir = Join-Path $WorkspaceRoot $entry.folder
    if (-not (Test-Path -LiteralPath (Join-Path $dir ".git"))) {
        Write-Warning "Clone ausente: $($entry.name)"
        continue
    }
    if (Get-Command git -ErrorAction SilentlyContinue) {
        $head = (& git -C $dir rev-parse --short HEAD | Out-String).Trim()
        $status = (& git -C $dir status --porcelain | Out-String).Trim()
        Write-Host ("{0} : {1}{2}" -f $entry.folder, $head, $(if ($status) { " [mudancas locais preservadas]" } else { "" }))
    }
}
Write-Host "Esse teste nao confirma execucao Unity, API ou dispositivos." -ForegroundColor Cyan
