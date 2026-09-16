# Arquitetura do SQI

## Objetivo

Substituir a decisão baseada em um score agregado por um pipeline **rule-based, multimétrico, hierárquico e fail-fast**. Um score contínuo pode existir como diagnóstico, mas nunca deve superar a rejeição de um gate obrigatório.

## Pipeline alvo

```text
SampleBuffer RAW
      │
      └── snapshot único de 5 s
              ↓
      G1 — Integridade RAW          [IMPLEMENTADO]
              │ FAIL → INVALID → não chama HR/SpO₂
              ↓ PASS
      preprocessamento em cópia     [PENDENTE]
              ↓
      G2 — Pulsatilidade RED/IR      [PENDENTE]
              │ FAIL → INVALID
              ↓ PASS
      beat detector                  [PENDENTE]
              ↓
      G3 — Morfologia                [PENDENTE]
              │ FAIL → INVALID
              ↓ PASS
      G4 — Coerência RED↔IR          [PENDENTE]
              │ FAIL → INVALID
              └ PASS → VALID final
                           ↓
                       HR / SpO₂
                           ↓
                  confidence/safety
```

## G0 — Snapshot

`signal_quality_evaluate_window()` copia a janela uma única vez para um buffer interno. O G1 recebe `const sqi_window_t`; o RAW não é modificado. Essa decisão garante que todos os gates futuros analisem a mesma janela e impede que filtros destruam evidências de rail, clipping ou flatline.

## G1 — Integridade

G1 trabalha apenas com RED RAW, IR RAW e timestamps. Para cada canal calcula mínimo, máximo, média, range e fração próxima aos rails. Para tempo calcula `dt`, fração de intervalos contínuos, descontinuidades e timestamps duplicados.

Falhas formais: `DISCONTINUITY`, `NO_SIGNAL_RED`, `NO_SIGNAL_IR`, `FLATLINE_RED`, `FLATLINE_IR`, `CLIPPING_RED`, `CLIPPING_IR`. A máscara preserva todas as falhas; `primary_reason` usa prioridade determinística.

Detalhes de implementação, pseudo-código, diagrama e referências: `docs/sqi/G1_GATE_GUIDE.md`.

## G2 — Pulsatilidade

Responsabilidade: rejeitar sinais tecnicamente íntegros que não contenham uma componente pulsátil consistente. Baseline técnica:

- preprocessamento em cópia com remoção de baseline/HP ~0,5 Hz;
- amplitude pulsátil por canal;
- threshold crossings;
- autocorrelação;
- lag dominante e período fisiologicamente plausível.

Vadrevu & Manikandan é a principal referência de estrutura; a parametrização deve ser recalibrada para nossa taxa, sensor e população.

## G3 — Morfologia

Após detectar beats, avaliar amplitude, largura, rise time, quantidade de beats e estabilidade batimento a batimento. Sukor et al. e Fischer et al. fundamentam o uso de morfologia/segmentação para rejeitar segmentos degradados. A V1 deve preferir regras simples, explicáveis e embarcáveis.

## G4 — Coerência RED↔IR

Verifica se os dois canais representam o mesmo evento pulsátil: período, contagem de beats e alinhamento temporal devem ser compatíveis. Correlação pode ser métrica auxiliar, mas não deve substituir regras de coerência de eventos.

## Semântica de `VALID`

Enquanto apenas G1 existe, `PPG_QUALITY_VALID` significa “passou todos os gates implementados”, não “PPG fisiologicamente confiável”. Quando G2–G4 forem implementados, `VALID` deverá ser atribuído somente depois de todos passarem.

## Separação de responsabilidades

- **SQI**: qualidade técnica/fisiológica da janela PPG;
- **HR/SpO₂**: estimadores, somente após gates obrigatórios;
- **confidence_engine**: confiança/fail-safe da saída global;
- **aplicação/jogo**: decisões terapêuticas/adaptação.

Detalhes dos gates pendentes e da calibração: `docs/sqi/PENDING_GATES_G2_G4.md`.
