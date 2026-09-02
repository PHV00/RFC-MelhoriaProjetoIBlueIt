# Arquitetura do SQI

## Objetivo

Evoluir o `signal_quality` atual de uma avaliação heurística monolítica e baseada em score agregado para uma arquitetura **rule-based, multimétrica, hierárquica e fail-fast**.

## Pipeline

```text
snapshot RAW de 5 s
      ↓
G1 — Integridade
      ├─ FAIL → INVALID
      ↓
preprocessamento
      ↓
G2 — Pulsatilidade RED + IR
      ├─ FAIL → INVALID
      ↓
detecção de batimentos
      ↓
G3 — Morfologia
      ├─ FAIL → INVALID
      ↓
G4 — Coerência RED ↔ IR
      ├─ FAIL → INVALID
      └─ PASS → VALID
```

## G0 — Snapshot da janela

O `signal_quality` copia a janela uma única vez do `sample_buffer`. Todos os gates trabalham sobre o mesmo snapshot imutável.

Requisitos:

- preservar RED e IR brutos;
- preservar timestamps;
- derivar `N` a partir de frequência e duração;
- evitar releituras do ring buffer durante a mesma avaliação.

## G1 — Integridade

Entrada: RAW RED, RAW IR e timestamps.

Responsabilidades:

- janela temporal suficiente;
- continuidade temporal;
- flatline / amplitude praticamente nula;
- saturação inferior/superior;
- clipping;
- falhas óbvias de aquisição.

G1 deve ocorrer antes de filtros que alterem a forma ou a amplitude do sinal.

## Pré-processamento

O RAW permanece intacto. O pipeline pode produzir visões derivadas:

```text
RAW
 ├─ visão para periodicidade
 └─ visão para morfologia
```

Baseline científica inicial:

- remoção de baseline / HP próximo de 0,5 Hz para pulsatilidade;
- banda aproximada até 5 Hz para morfologia, sujeita à validação na implementação.

## G2 — Pulsatilidade

Avaliado em RED e IR separadamente.

Features baseline:

- amplitude/range;
- número de threshold crossings;
- autocorrelação;
- lag/periodicidade derivada da ACF.

Para SpO₂, os dois canais precisam apresentar pulsatilidade adequada.

## G3 — Morfologia

Depende de detecção de batimentos.

Features baseline:

- amplitude de pulso;
- largura do pulso;
- rise time;
- número de batimentos;
- estabilidade batimento a batimento.

A V1 implementa uma morfologia reduzida, sem reproduzir integralmente árvores de decisão complexas da literatura.

## G4 — Coerência RED ↔ IR

Objetivo: verificar se RED e IR representam o mesmo fenômeno pulsátil.

Métricas iniciais:

- período RED;
- período IR;
- diferença relativa/absoluta de período;
- contagem de batimentos por canal;
- diferença entre contagens;
- alinhamento temporal de eventos/picos.

Correlação direta de amplitude/forma RED↔IR pode ser mantida como métrica auxiliar, mas não é requisito obrigatório da primeira versão.

## Saída

A saída principal não é apenas um score numérico.

```c
typedef enum {
    PPG_QUALITY_UNKNOWN = 0,
    PPG_QUALITY_VALID,
    PPG_QUALITY_INVALID
} ppg_quality_state_t;
```

O resultado deve conter:

- estado final;
- gate que falhou;
- motivo da falha;
- métricas por gate;
- metadados da janela;
- versão da configuração.

Um score contínuo pode ser mantido como diagnóstico/experimento, mas não deve substituir a decisão hierárquica da V1.
