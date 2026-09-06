# Status de implementação — SQI MAX30102

## Estado atual

A arquitetura documental foi definida e a **Fase 0 — refatoração estrutural sem mudança intencional do algoritmo — foi concluída**.

O projeto possui agora a estrutura física que permitirá implementar o SQI hierárquico de forma incremental, preservando um ponto de regressão conhecido antes da introdução de G1–G4.

## O que foi concluído

### Arquitetura e documentação

- `ReadME.md`: fluxo funcional, princípios arquiteturais, organização alvo e documentação técnica;
- `ARCHITECTURE.md`: separação em camadas e direção de dependências;
- `SQI_ARCHITECTURE.md`: arquitetura hierárquica e fail-fast em quatro gates;
- `DECISIONS.md`: RAW imutável, configuração centralizada, separação SQI × confidence e migração incremental;
- `CONFIGURATION.md`: modelo de configuração centralizada/versionável e perfis futuros `NO_GRIP`/`WITH_GRIP`;
- `TESTING.md`: estratégia de testes por gate e integração;
- `ROADMAP.md`: fases de refatoração, implementação, integração e calibração;
- `REFERENCES.md`: mapeamento dos trabalhos científicos/técnicos para as responsabilidades do SQI.

### Fase 0 — estrutura de código

- criada a árvore `processing/sqi/`;
- criadas as áreas `gates/`, `features/` e `preprocess/`;
- criado o scaffolding de testes correspondente;
- `signal_quality.c/.h` atuais foram migrados de `processing/` para `processing/sqi/`;
- `CMakeLists.txt` e includes foram atualizados para o novo caminho;
- a implementação antiga no caminho anterior deixou de fazer parte do build;
- tipos compartilhados começaram a ser migrados de `app/app_types.h` para `common/measurement_types.h`;
- `signal_quality_types.h` foi criado como ponto de evolução do contrato do SQI;
- foi criada uma versão inicial de `sqi_config_t`;
- a configuração temporal da refatoração preserva o comportamento anterior (4 s de janela e 500 ms de passo) para evitar misturar mudança estrutural com mudança algorítmica.

### Build e smoke test

A nova estrutura foi compilada com ESP-IDF para ESP32-C3. O build concluiu e gerou o binário da aplicação, com `processing/sqi/signal_quality.c` incorporado ao componente principal.

Foi realizado smoke test com ESP32-C3 + MAX30102/MAX3010x:

- o sensor foi identificado e reportou taxa de amostragem de 100 Hz;
- o firmware permaneceu executando e emitindo telemetria;
- sem dedo, o sistema permaneceu em `LOW_CONFIDENCE`, com `finger=false` e resultados fisiológicos não utilizáveis;
- com dedo presente, o pipeline atual executou qualidade de sinal, estimador de FC, estimador de SpO₂ e `confidence_engine`;
- após retirar o dedo, o sistema retornou a `LOW_CONFIDENCE` e `finger=false`;
- não foram observados panic, watchdog ou loop de reset durante o teste informado.

Essa evidência fecha a Fase 0 como **regressão funcional/estrutural**. Não foi realizado teste de equivalência numérica bit a bit usando o mesmo conjunto RAW no firmware anterior e no refatorado.

## O que ainda não foi implementado

- o novo contrato `SQI_EVAL_*` / `PPG_QUALITY_*`;
- `failed_gate` e `fail_reason`;
- snapshot único e imutável da janela para todos os gates;
- configuração completa e independente para G1–G4;
- mudança da baseline para janela de 5 s e passo de 1 s;
- G1 Integridade;
- pré-processamento definitivo e G2 Pulsatilidade;
- beat detector compartilhado e G3 Morfologia;
- G4 de coerência RED ↔ IR;
- fail-fast hierárquico novo;
- bloqueio definitivo dos estimadores para qualquer janela classificada como `INVALID` pelo novo SQI;
- testes unitários dos novos gates;
- calibração experimental de thresholds;
- validação/calibração fisiológica do módulo de SpO₂;
- perfil `NO_GRIP` calibrado;
- comparação futura com `WITH_GRIP`.

Os valores instáveis de FC/SpO₂ observados no smoke test não são tratados como falha desta fase: a Fase 0 tinha como objetivo preservar o funcionamento do pipeline durante a reorganização. A melhoria da rejeição de transientes, artefatos e incoerências pertence às fases seguintes do SQI.

## Próxima etapa — Fase 1

A próxima etapa deve formalizar o contrato do novo SQI antes de implementar G1:

1. separar o status da avaliação (`WAITING`, `COMPLETE`, `ERROR`) do estado de qualidade (`UNKNOWN`, `VALID`, `INVALID`);
2. definir `failed_gate` e `fail_reason`;
3. definir uma janela/snapshot único e imutável para uma avaliação;
4. evoluir `sqi_config_t` para conter configurações por gate;
5. após esse contrato estar estável, adotar a baseline de 5 s e passo de 1 s;
6. somente então iniciar G1 Integridade.

Esse limite mantém a implementação incremental e permite atribuir cada mudança de comportamento a uma fase específica.
