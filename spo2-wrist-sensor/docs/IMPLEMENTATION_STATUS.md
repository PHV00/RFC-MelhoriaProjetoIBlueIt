# Status de implementação — SQI MAX30102

## Estado atual

A refatoração estrutural foi concluída e o **G1 — Integridade** está implementado, integrado ao pipeline e funcionalmente validado no host e no ESP32-C3. O sistema já usa janela de 5 s com passo de 1 s, snapshot RAW imutável, `failed_gate`, `fail_reason`, `failure_mask` e fail-fast antes de HR/SpO₂.

A implementação atual ainda **não representa o SQI hierárquico completo**: G2, G3 e G4 permanecem pendentes, e os thresholds empíricos do G1 ainda precisam de calibração científica no perfil real `NO_GRIP`.

## Concluído

- [x] arquitetura `processing/sqi/` e separação por gates;
- [x] contrato `SQI_EVAL_*` / `PPG_QUALITY_*`;
- [x] `failed_gate` e `fail_reason`;
- [x] snapshot único da janela antes do G1;
- [x] baseline temporal de 5 s / passo de 1 s;
- [x] `g1_integrity_config_t` centralizado;
- [x] G1 com continuidade, presença óptica, flatline e clipping por canal;
- [x] máscara cumulativa de falhas e motivo primário determinístico;
- [x] telemetria JSON v2 com métricas do G1;
- [x] bloqueio de HR/SpO₂ quando G1 retorna `INVALID`;
- [x] 18 testes unitários host;
- [x] fault injection end-to-end para flatline, clipping RED, clipping IR e descontinuidade;
- [x] sanity check final em aquisição real após retornar `SQI_G1_FAULT_MODE=0`.

## Evidência de validação do G1

| Caso | Resultado |
|---|---|
| unit tests host | `PASS (18 cases)` |
| flatline ambos os canais | PASS; esperado mask 24 |
| clipping RED | PASS; 14 frames, 0 violações, mask 32, `red_clip=0.0500`, `ir_clip=0.0000` |
| clipping IR | PASS; 14 frames, 0 violações, mask 64, `red_clip=0.0000`, `ir_clip=0.0500` |
| descontinuidade | PASS; 13 frames, 0 violações, mask 1, `continuity=0.7996` |
| aquisição normal sem dedo | G1 INVALID, mask 6 (`NO_SIGNAL_RED|NO_SIGNAL_IR`) |
| dedo presente | G1 PASS, mask 0 |
| retirada do dedo | atraso esperado devido à janela deslizante de 5 s; depois mask 6 |
| fail-fast | toda janela G1 INVALID manteve `hr.valid=false` e `spo2.valid=false` |

## O que o G1 garante — e o que não garante

O G1 responde apenas se a janela possui **integridade técnica mínima para continuar**. Ele não garante que a forma de onda seja fisiologicamente utilizável. É possível existir `quality_state=VALID` no estágio atual e HR/SpO₂ instáveis, porque ainda faltam as decisões de pulsatilidade, morfologia e coerência entre canais.

Assim, `quality_score` legado não deve ser interpretado como árbitro final de qualidade. O estado final só poderá significar “PPG utilizável” quando G1→G4 estiverem implementados.

## Pendências de implementação

- [ ] pré-processamento definitivo para G2/G3;
- [ ] G2 — pulsatilidade RED e IR;
- [ ] beat detector compartilhado;
- [ ] G3 — morfologia batimento a batimento;
- [ ] G4 — coerência RED↔IR;
- [ ] redefinir/remover papel decisório do `minimum_quality_score` legado;
- [ ] versão completa de configuração por gate;
- [ ] testes unitários/integrados de G2–G4;
- [ ] calibração científica dos thresholds do G1;
- [ ] validação/calibração fisiológica do SpO₂;
- [ ] congelar perfil `NO_GRIP` e futuramente comparar `WITH_GRIP`.

## Próxima etapa

A próxima etapa de software é G2 — Pulsatilidade. Em paralelo deve iniciar a coleta de dados RAW para calibrar os thresholds provisórios do G1 conforme `docs/sqi/G1_CALIBRATION_PLAN.md` e `docs/sqi/G1_PENDING_CALIBRATIONS.md`.
