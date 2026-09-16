# Documentação de desenvolvimento do SQI

Use estes arquivos como guia de implementação, validação e calibração.

- `G1_GATE_GUIDE.md` — lógica completa do Gate 01: código, pseudo-código, máscara, diagrama ASCII, limites de responsabilidade e referências.
- `G1_VALIDATION_REPORT.md` — evidência funcional: unit tests, fault injection e sanity check em hardware.
- `G1_CALIBRATION_PLAN.md` — metodologia científica para transformar thresholds provisórios em parâmetros calibrados.
- `G1_PENDING_CALIBRATIONS.md` — checklist operacional de cada valor ainda pendente e critério de fechamento.
- `PENDING_GATES_G2_G4.md` — funcionamento técnico/teórico planejado para G2, G3 e G4 e como seus parâmetros deverão ser obtidos.
- `FAULT_INJECTION_ARCHIVE.md` — localização da branch arquivada que preserva o harness end-to-end retirado da linha de produção.

Regra central: **validar a implementação da regra não equivale a calibrar cientificamente o threshold**. O G1 V1 está funcionalmente validado; a calibração de thresholds permanece uma etapa experimental separada.
