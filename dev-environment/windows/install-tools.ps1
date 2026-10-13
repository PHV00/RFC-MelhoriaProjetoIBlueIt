#requires -Version 5.1
[CmdletBinding()]
param([switch]$InstallDocker)
$ErrorActionPreference = "Stop"
if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
    throw "WinGet ausente: atualize App Installer no Windows 11."
}
$ids = @("Git.Git", "Microsoft.VisualStudioCode", "Unity.UnityHub", "Python.Python.3.11")
if ($InstallDocker) { $ids += "Docker.DockerDesktop" }
foreach ($id in $ids) {
    Write-Host "[WinGet] $id"
    & winget install --exact --id $id --source winget --accept-package-agreements --accept-source-agreements
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "WinGet retornou $LASTEXITCODE para $id; confira instalacao."
    }
}
Write-Warning "Reabra PowerShell para recarregar PATH. Instale o Editor 2022.3.4f1 no Unity Hub e faca login."
if ($InstallDocker) { Write-Warning "Docker Desktop exige WSL2 e pode solicitar reinicializacao." }
