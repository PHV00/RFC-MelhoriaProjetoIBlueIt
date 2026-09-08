# Roadmap — Refatoração, SQI hierárquico e calibração

## Status geral

- [x] arquitetura documental e modular;
- [x] Fase 0 — refatoração estrutural;
- [x] contrato de qualidade, snapshot e baseline 5 s / 1 s;
- [x] G1 — Integridade;
- [x] fail-fast antes de HR/SpO₂ para falhas do G1;
- [x] testes unitários e fault injection do G1;
- [ ] calibração científica dos thresholds do G1;
- [ ] G2 — Pulsatilidade;
- [ ] G3 — Morfologia;
- [ ] G4 — Coerência RED↔IR;
- [ ] congelar perfil `NO_GRIP`;
- [ ] comparar perfil futuro `WITH_GRIP`.

## Fase 0 — Refatoração estrutural — CONCLUÍDA

A estrutura `processing/sqi/`, scaffolding de gates/features/preprocess, tipos compartilhados e documentação base foram criados e validados por build/smoke test.

## Fase 1 — Contrato e tipos — CONCLUÍDA para o G1

- [x] `SQI_EVAL_WAITING/COMPLETE/ERROR`;
- [x] `PPG_QUALITY_UNKNOWN/VALID/INVALID`;
- [x] `failed_gate` / `fail_reason`;
- [x] `sqi_window_t` com snapshot RAW;
- [x] `g1_integrity_config_t`;
- [x] 5 s de janela / 1 s de passo.

A configuração ainda deverá crescer com estruturas específicas de G2–G4.

## Fase 2 — G1 Integridade — IMPLEMENTADO E FUNCIONALMENTE VALIDADO

- [x] continuidade temporal;
- [x] presença óptica por média RAW em RED e IR;
- [x] flatline por `raw_range`;
- [x] clipping/saturação próximo aos rails;
- [x] failure mask cumulativa;
- [x] motivo primário determinístico;
- [x] telemetria;
- [x] fail-fast;
- [x] 18 unit tests;
- [x] fault injection de flatline, clipping RED, clipping IR e descontinuidade;
- [x] sanity check de retorno ao modo real.

Pendência desta fase: **calibrar cientificamente** `rail_margin_counts`, `minimum_mean_level`, `minimum_raw_range`, `maximum_clipping_fraction`, `minimum_continuity_fraction` e `maximum_interval_deviation_fraction`.

## Fase 3 — Pré-processamento + G2 Pulsatilidade

Objetivo: rejeitar janelas tecnicamente íntegras, porém sem comportamento pulsátil confiável.

- [ ] criar visão processada sem alterar RAW;
- [ ] remoção de baseline / HP próximo de 0,5 Hz como baseline científica;
- [ ] amplitude/range pulsátil por canal;
- [ ] threshold crossings;
- [ ] autocorrelação e lag dominante;
- [ ] plausibilidade do período;
- [ ] regras RED e IR independentes;
- [ ] fail-fast G2;
- [ ] testes sintéticos, MAX30102 RAW e datasets externos;
- [ ] calibração dos thresholds em conjunto de desenvolvimento e validação independente.

Referências centrais: Vadrevu & Manikandan; Reddy et al. como inspiração hierárquica.

## Fase 4 — Beat detector + G3 Morfologia

Objetivo: avaliar se os pulsos detectados possuem forma e estabilidade compatíveis com PPG utilizável.

- [ ] detector de beats reutilizável;
- [ ] amplitude por beat;
- [ ] largura;
- [ ] rise time;
- [ ] quantidade de beats válidos;
- [ ] variabilidade batimento a batimento;
- [ ] regras de aceitação/rejeição;
- [ ] testes e calibração.

Referências centrais: Sukor et al.; Fischer et al.; Orphanidou et al. como validação complementar.

## Fase 5 — G4 Coerência RED↔IR

Objetivo: confirmar que RED e IR descrevem o mesmo evento pulsátil.

- [ ] período por canal;
- [ ] diferença relativa de período;
- [ ] contagem de beats por canal;
- [ ] diferença de contagem;
- [ ] alinhamento temporal de picos/eventos;
- [ ] correlação como métrica auxiliar, se útil;
- [ ] testes de canais coerentes e deliberadamente divergentes.

## Fase 6 — Decisão final e confidence

- [ ] `VALID` somente após G1→G4;
- [ ] score contínuo apenas como diagnóstico/confiança auxiliar;
- [ ] revisar `minimum_quality_score` legado;
- [ ] assegurar que `confidence_engine` não contradiga os gates obrigatórios.

## Fase 7 — Calibração NO_GRIP

- [ ] congelar configuração do MAX30102 e geometria óptica;
- [ ] coletar RAW rotulado por sessões/participantes;
- [ ] gerar features por janela de 5 s;
- [ ] separar desenvolvimento e validação por sessão/participante;
- [ ] selecionar thresholds por distribuição + ROC/PR/regra de erro aceitável;
- [ ] análise de sensibilidade;
- [ ] congelar e versionar perfil `NO_GRIP`.

## Fase 8 — Pegador anatômico

- [ ] repetir protocolo com pegador;
- [ ] comparar distribuições e erros;
- [ ] manter thresholds se equivalentes ou criar `WITH_GRIP` se necessário;
- [ ] medir redução de artefatos/falsas rejeições.
