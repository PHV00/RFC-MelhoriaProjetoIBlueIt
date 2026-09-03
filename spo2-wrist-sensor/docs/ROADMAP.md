# Roadmap — Refatoração e implementação do SQI

## Status geral

- [x] atualizar o `ReadME.md` com o fluxo funcional e a organização alvo;
- [x] documentar a arquitetura geral do firmware;
- [x] documentar a arquitetura hierárquica do SQI em quatro gates;
- [x] registrar decisões arquiteturais e de migração incremental;
- [x] documentar a estrutura de configuração e versionamento de thresholds;
- [x] documentar estratégia de testes por gate e integração;
- [x] documentar o roadmap de implementação;
- [x] registrar referências técnicas/científicas e o papel de cada trabalho;
- [x] registrar que a primeira calibração será realizada sem o pegador anatômico;
- [x] refatorar a estrutura de código sem alterar o algoritmo do SQI atual;
- [x] executar build e smoke test da nova estrutura em ESP32-C3 + MAX30102;
- [ ] implementar os gates;
- [ ] calibrar thresholds com dados reais do MAX30102.

> A Fase 0 foi concluída como refatoração estrutural com regressão funcional. Isso não representa validação clínica, calibração do oxímetro ou implementação do novo SQI hierárquico.

## Fase 0 — Refatoração estrutural sem mudar o algoritmo — CONCLUÍDA

- [x] mover `signal_quality.*` para `processing/sqi/`;
- [x] criar `gates/`, `features/` e `preprocess/`;
- [x] criar o scaffolding de testes por gate, features e integração;
- [x] atualizar includes e `CMakeLists.txt`;
- [x] iniciar desacoplamento de tipos compartilhados de `app/` para `common/`;
- [x] criar configuração inicial `sqi_config_t` preservando o comportamento temporal existente;
- [x] compilar o projeto com ESP-IDF para ESP32-C3;
- [x] confirmar que `processing/sqi/signal_quality.c` entra no build e a localização antiga não é compilada;
- [x] executar smoke test no hardware com MAX30102;
- [x] confirmar inicialização do sensor e processamento contínuo sem crash/watchdog observado;
- [x] confirmar comportamento funcional com ausência, colocação e retirada do dedo;
- [x] documentar contratos e ownership de cada módulo.

### Evidência de regressão da Fase 0

- target: ESP32-C3;
- sensor: MAX30102/MAX3010x identificado pelo firmware;
- taxa de amostragem observada: 100 Hz;
- build: concluído com geração do binário da aplicação;
- execução: telemetria de qualidade, FC, SpO₂ e estado final permaneceu operacional;
- sem dedo: `LOW_CONFIDENCE`, `finger=false` e resultados fisiológicos não utilizáveis;
- dedo presente: pipeline atual produziu avaliações de qualidade e resultados dos estimadores;
- retirada do dedo: retorno a `LOW_CONFIDENCE` e `finger=false`;
- não foram observados panic, watchdog ou loop de reset durante o smoke test.

A equivalência validada nesta fase é funcional/estrutural. Não foi realizado teste de equivalência numérica bit a bit com reprodução do mesmo conjunto RAW antes e depois da refatoração.

## Fase 1 — Contrato e tipos

- [x] criar fisicamente `signal_quality_types.h` como ponto de evolução do contrato;
- [ ] separar estado de avaliação de estado de qualidade;
- [ ] criar `fail_reason` e `failed_gate`;
- [x] iniciar migração de tipos compartilhados para `common/`;
- [x] criar versão inicial de `sqi_config_t`;
- [ ] evoluir `sqi_config_t` para configurações específicas de G1–G4;
- [ ] definir snapshot único/imutável da janela (`sqi_window_t` ou equivalente);
- [ ] alterar a baseline temporal para 5 s e passo de 1 s após o marco de regressão da Fase 0.

> Os itens já marcados na Fase 1 representam apenas infraestrutura criada durante a refatoração. O novo contrato de decisão do SQI ainda não foi implementado.

## Fase 2 — G1 Integridade

- [ ] snapshot único da janela;
- [ ] continuidade temporal;
- [ ] flatline / amplitude quase nula;
- [ ] clipping/saturação RAW;
- [ ] testes unitários;
- [ ] telemetria do motivo da falha.

## Fase 3 — Pré-processamento + G2

- [ ] detrending/HP para baseline;
- [ ] threshold crossing;
- [ ] autocorrelação;
- [ ] pulsatilidade RED;
- [ ] pulsatilidade IR;
- [ ] fail-fast;
- [ ] testes unitários e integração.

## Fase 4 — Beat detector + G3

- [ ] detecção de beats;
- [ ] amplitude;
- [ ] largura;
- [ ] rise time;
- [ ] estabilidade entre beats;
- [ ] testes.

## Fase 5 — G4 RED ↔ IR

- [ ] comparação de período;
- [ ] comparação de contagem;
- [ ] alinhamento temporal;
- [ ] métricas auxiliares;
- [ ] testes.

## Fase 6 — Integração fisiológica

- [ ] impedir chamadas de estimadores em janelas inválidas;
- [ ] ajustar `confidence_engine` para consumir qualidade já decidida;
- [ ] atualizar `health_frame_t`;
- [ ] atualizar telemetria.

## Fase 7 — Calibração sem pegador

- [ ] coletar base RAW representativa;
- [ ] registrar falhas por gate;
- [ ] ajustar thresholds provisórios;
- [ ] congelar perfil `NO_GRIP`.

## Fase 8 — Pegador anatômico

- [ ] repetir coleta com pegador;
- [ ] comparar distribuição de métricas;
- [ ] criar perfil `WITH_GRIP` se necessário;
- [ ] avaliar redução de falsos negativos/artefatos.
