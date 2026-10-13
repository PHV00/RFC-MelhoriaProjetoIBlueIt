#requires -Version 5.1
[CmdletBinding()]
param([switch]$StartMongo)
$ErrorActionPreference = "Stop"
$dockerDir = Join-Path (Split-Path -Parent $PSScriptRoot) "docker"
$envFile = Join-Path $dockerDir ".env"
if (-not (Test-Path -LiteralPath $envFile)) {
    $bytes = New-Object byte[] 24
    $rng = [Security.Cryptography.RandomNumberGenerator]::Create()
    try { $rng.GetBytes($bytes) } finally { $rng.Dispose() }
    $secret = -join ($bytes | ForEach-Object { $_.ToString("x2") })
    "MONGO_USER=iblueit_dev" + [Environment]::NewLine + "MONGO_PASSWORD=$secret" |
        Set-Content -LiteralPath $envFile -Encoding ASCII
    Write-Host "Arquivo docker/.env criado com senha local aleatoria (nao versionada)."
} else { Write-Host "docker/.env preservado." }
if ($StartMongo) {
    if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
        throw "Docker CLI ausente. Inicie/instale Docker Desktop."
    }
    & docker compose --env-file $envFile -f (Join-Path $dockerDir "compose.yaml") up -d mongo
    if ($LASTEXITCODE -ne 0) { throw "Nao foi possivel iniciar MongoDB." }
}
