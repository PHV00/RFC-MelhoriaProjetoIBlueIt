# Decisões arquiteturais

## D-001 — SQI antes da oximetria
**Decisão:** SpO₂ e FC só são considerados utilizáveis quando a janela foi aprovada pelo SQI.

**Consequência:** `app_controller` deve interromper o fluxo para janelas `INVALID`.

## D-002 — `signal_quality` como orquestrador
**Decisão:** `signal_quality.c` deixa de concentrar todas as fórmulas e passa a coordenar os gates e módulos auxiliares.

## D-003 — Gates em subpastas
**Decisão:** criar uma árvore `processing/sqi/gates/` com uma subpasta para cada gate.

**Motivo:** rastreabilidade científica e leitura mais simples para desenvolvimento e manutenção.

## D-004 — Matemática separada de decisão
**Decisão:** autocorrelação, threshold crossing e beat detection ficam em `features/`; filtros e detrending ficam em `preprocess/`.

**Motivo:** evitar duplicação e permitir reutilização por múltiplos gates/estimadores.

## D-005 — Fail-fast
**Decisão:** cada gate pode encerrar a avaliação imediatamente.

**Motivo:** reduzir processamento desnecessário e impedir que sinais obviamente inválidos alcancem estágios posteriores.

## D-006 — RAW imutável
**Decisão:** nenhum filtro sobrescreve a janela RAW.

## D-007 — Configuração centralizada
**Decisão:** thresholds devem migrar de `#define` locais para estruturas de configuração versionáveis.

## D-008 — Janela definida em tempo
**Decisão:** duração da janela é especificada em ms/s e convertida em número de amostras usando `Fs`.

Baseline: 5 s.

## D-009 — Janela deslizante
**Decisão:** usar janela de 5 s com passo configurável; baseline inicial sugerido de 1 s.

## D-010 — Desenvolvimento inicial sem pegador anatômico
**Decisão:** thresholds e critérios serão calibrados primeiro para o cenário sem o pegador, aceitando maior variabilidade mecânica/óptica.

**Depois:** o pegador será adicionado como melhoria física sem exigir reescrita da arquitetura do SQI.

## D-011 — Separação SQI × confiança
**Decisão:** SQI decide qualidade técnica da janela. `confidence_engine` trata confiança/fail-safe da saída global.

## D-012 — Tipos compartilhados fora de `app/`
**Decisão:** reduzir dependência de `processing` em `app/app_types.h`, migrando tipos transversais para `common/` ou equivalente.

## D-013 — Migração incremental
**Decisão:** não apagar o algoritmo atual de uma vez.

Ordem:

```text
reorganizar estrutura
→ compilar sem alteração funcional
→ criar novo contrato
→ extrair G1
→ validar
→ adicionar G2
→ validar
→ adicionar G3
→ validar
→ adicionar G4
```
