# Roadmap — Refatoração e implementação do SQI

## Status desta etapa documental

- [x] atualizar o `ReadME.md` com o fluxo funcional e a organização alvo;
- [x] documentar a arquitetura geral do firmware;
- [x] documentar a arquitetura hierárquica do SQI em quatro gates;
- [x] registrar decisões arquiteturais e de migração incremental;
- [x] documentar a estrutura de configuração e versionamento de thresholds;
- [x] documentar estratégia de testes por gate e integração;
- [x] documentar o roadmap de implementação;
- [x] registrar referências técnicas/científicas e o papel de cada trabalho;
- [x] registrar que a primeira calibração será realizada sem o pegador anatômico;
- [ ] refatorar a estrutura de código;
- [ ] implementar os gates;
- [ ] executar compilação/regressão da nova estrutura;
- [ ] calibrar thresholds com dados reais do MAX30102.

> Nesta etapa foram alterados apenas documentos. Nenhum `.c` ou `.h` foi modificado e nenhuma fase de implementação deve ser considerada concluída por causa desta PR.

## Fase 0 — Refatoração estrutural sem mudar o algoritmo

- [ ] mover `signal_quality.*` para `processing/sqi/`;
- [ ] criar `gates/`, `features/` e `preprocess/`;
- [ ] atualizar `CMakeLists.txt`;
- [ ] garantir compilação com comportamento equivalente ao atual;
- [x] documentar contratos e ownership de cada módulo.

## Fase 1 — Contrato e tipos

- [ ] criar `signal_quality_types.h`;
- [ ] separar estado de avaliação de estado de qualidade;
- [ ] criar `fail_reason` e `failed_gate`;
- [ ] iniciar migração de tipos compartilhados para `common/`;
- [ ] criar `sqi_config_t`.

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
