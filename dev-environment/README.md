# I Blue It: ambiente reproduzivel para Windows 11

Esta estrutura coordena os cinco repositorios por **Git direto**, preservando repositorios, historicos, permissoes, branches e commits. Nao e monorepo nem copia codigo dos outros projetos.

## Preparacao inicial do Windows

No PowerShell, instale apenas Git para poder obter o instalador:

~~~powershell
winget install --id Git.Git --exact --source winget
~~~

Reabra o PowerShell e execute:

~~~powershell
New-Item -ItemType Directory -Force C:\dev\iblueit
cd C:\dev\iblueit
git clone --branch feat/windows-dev-environment https://github.com/PHV00/RFC-MelhoriaProjetoIBlueIt.git
cd .\RFC-MelhoriaProjetoIBlueIt\dev-environment
.\setup.ps1 -InstallTools -InstallDocker
~~~

Se a politica de PowerShell bloquear arquivos, revise o script e use apenas nesta sessao: Set-ExecutionPolicy -Scope Process RemoteSigned. Pode ser necessario reiniciar o PowerShell apos instalar ferramentas.

O script clona os cinco repositorios como pastas irmas. NUNCA executa pull, reset, checkout ou stash em diretorio ja existente. Uma instalacao repetida preserva trabalho local. Use -WorkspaceRoot para alterar a pasta destino.

O manifesto registra branches e commits exatos. A opcao -PinBaselines clona e fixa os novos repositorios no commit de referencia em HEAD destacado (antes de desenvolver, crie ou selecione uma branch). Sem esta opcao, clona a branch atual e registra o baseline para comparacao. Atualizacoes de cada projeto sao deliberadamente manuais.

## Dependencias

WinGet: Git, VS Code, Unity Hub e Python 3.11; Docker Desktop somente se -InstallDocker. Unity Editor **2022.3.4f1 (35713cd46cd7)** deve ser instalado no Hub com login/licenca e modulos adequados. Arduino IDE/CLI, placas e drivers USB dependem do hardware escolhido.

Use **iblueit-psychophysiological-flow** como referencia Windows. Mantenha **iblueit-psychophysiological-flow-linux-port** separado. O original usa caminhos absolutos do ML-Agents no manifest; isso ainda requer correcao controlada numa branch Windows, sem alterar o modelo ONNX/DeepDDA.

O servidor usa Node.js 12.16.1, Azure Functions 3 e MongoDB; sao dependencias antigas. Nao instale Node 12 globalmente no Windows. O dashboard usa React 16 com react-scripts 5. Faltam identificar um repositorio do Bridge PITACO 2.0 Python e o modulo de predicao/IA mencionado na documentacao upstream; nao estao cobertos pelos cinco repositorios.

## Docker local: primeira fase

~~~powershell
.\windows\init-docker.ps1 -StartMongo
~~~

Isso cria docker/.env com senha aleatoria nao versionada e inicia somente MongoDB local (127.0.0.1:27017). Use dados de teste ficticios, nunca dados de pacientes.

O compose tambem contem os perfis EXPERIMENTAIS legacy-api e dashboard, ainda nao validados. A API upstream pode requerer ajustes adicionais de Azure Storage, IA externa e versoes. Nao usar em producao.

~~~powershell
docker compose --env-file docker/.env -f docker/compose.yaml --profile legacy-api up --build -d
docker compose --env-file docker/.env -f docker/compose.yaml --profile dashboard up -d
~~~

## Diagnostico

~~~powershell
.\setup.ps1 -VerifyOnly
~~~

Verifica ferramentas e repositorios de forma nao destrutiva, mas NAO comprova que Unity, backend, oximetria, PITACO ou gameplay funcionem.

## Estrutura esperada

~~~text
C:\dev\iblueit\
  RFC-MelhoriaProjetoIBlueIt\dev-environment\
  iblueit-psychophysiological-flow\
  iblueit-psychophysiological-flow-linux-port\
  iblueit-server-side\
  iblueit-health-Infocharts\
~~~

## Proximas fases

1. Validar scripts num Windows 11 real, Unity e estado das dependencias.
2. Containerizar e testar API/dashboard de maneira segura; falta IA/predicao do setup.
3. Identificar e integrar a fonte do Bridge, firmware e drivers usados na bancada.
4. Testes de ponta a ponta, equivalencia de sinais fisiologicos e distribuicao nativa.

A fase atual e infraestrutura inicial, **nao porte concluido nem ambiente testado**.
