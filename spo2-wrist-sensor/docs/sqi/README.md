# Documentação de desenvolvimento do SQI

Use estes arquivos como guia de implementação, validação e calibração.

- `G1_GATE_GUIDE.md` — lógica completa do Gate 01: código, pseudo-código, máscara, diagrama ASCII, limites de responsabilidade e referências.
- `G1_VALIDATION_REPORT.md` — evidência funcional: unit tests, fault injection e sanity check em hardware.
- `G1_CALIBRATION_PLAN.md` — metodologia científica para transformar thresholds provisórios em parâmetros calibrados.
- `G1_PENDING_CALIBRATIONS.md` — checklist operacional de cada valor ainda pendente e critério de fechamento.
- `G2_GATE_GUIDE.md` — contrato, arquitetura, features, fundamentação, testes e calibração planejada do Gate 02 — Pulsatilidade.
- `PENDING_GATES_G2_G4.md` — visão de funcionamento técnico/teórico planejado para G2, G3 e G4 e como seus parâmetros deverão ser obtidos.
- `FAULT_INJECTION_ARCHIVE.md` — localização da branch arquivada que preserva o harness end-to-end retirado da linha de produção.

Regra central: **validar a implementação da regra não equivale a calibrar cientificamente o threshold**.

Estado atual:

```text
G1 — Integridade          funcionalmente validado; thresholds em calibração
G2 — Pulsatilidade        preparação/implementação isolada iniciada
G3 — Morfologia           planejado
G4 — Coerência RED↔IR     planejado
```

O G2 deve permanecer fora do pipeline produtivo até que preprocessamento, features e testes host sejam validados. Depois disso ele será integrado entre G1 e os estimadores.
