# G1 — Gate de Integridade: guia técnico e científico

## 1. Objetivo

O G1 é o primeiro filtro formal do SQI. Sua pergunta é deliberadamente limitada:

> Esta janela RAW possui integridade mínima de aquisição/contato para justificar processamento fisiológico posterior?

Ele não decide se a onda é fisiologicamente boa. Movimento, periodicidade ruim e morfologia degradada pertencem a G2/G3/G4.

## 2. Entrada e saída

Entrada:

```c
typedef struct {
    const ppg_sample_t *samples;
    size_t count;
    float expected_sample_rate_hz;
} sqi_window_t;
```

O ponteiro é `const`: G1 não altera RED, IR ou timestamps.

Saída principal:

```c
typedef struct {
    bool passed;
    uint32_t failure_mask;
    sqi_fail_reason_t primary_reason;
    uint32_t red_min, red_max, red_range;
    float red_mean, red_clipping_fraction;
    uint32_t ir_min, ir_max, ir_range;
    float ir_mean, ir_clipping_fraction;
    float continuity_fraction;
    uint32_t discontinuity_count;
    uint32_t duplicate_timestamp_count;
} g1_integrity_result_t;
```

## 3. Lógica pura em ASCII

```text
RAW RED + RAW IR + timestamps
              │
              ▼
       validar argumentos/config
              │ inválido
              ├──────────────→ SQI_EVAL_ERROR
              ▼
     varrer TODA a janela uma vez
              │
              ├─ RED: min/max/soma/near-rail
              ├─ IR : min/max/soma/near-rail
              └─ tempo: dt, contínuo, gap, duplicado
              │
              ▼
      calcular features G1
              │
              ├─ red_mean / ir_mean
              ├─ red_range / ir_range
              ├─ red_clip / ir_clip
              └─ continuity_fraction
              │
              ▼
   aplicar TODAS as regras e montar mask
              │
              ├─ continuity < min ? bit 1
              ├─ red_mean < min ? bit 2
              ├─ ir_mean < min ? bit 4
              ├─ red_range < min ? bit 8
              ├─ ir_range < min ? bit 16
              ├─ red_clip > max ? bit 32
              └─ ir_clip > max ? bit 64
              │
              ▼
         mask == 0 ?
          /        \
        SIM        NÃO
        │           │
        ▼           ▼
     G1 PASS      G1 FAIL
        │           │
        │           ├─ preservar mask completa
        │           ├─ escolher primary_reason
        │           └─ fail-fast: HR/SpO₂ não executam
        ▼
   continuar para G2 (futuro)
```

## 4. Regras exatamente implementadas

### 4.1 Continuidade

```c
expected_dt_ms = 1000.0f / Fs;
min_dt = expected_dt_ms * (1 - tolerance);
max_dt = expected_dt_ms * (1 + tolerance);
continuity = continuous_intervals / (N - 1);

if (continuity < minimum_continuity_fraction)
    failure_mask |= G1_FAILURE_DISCONTINUITY;
```

A comparação é estrita: exatamente no limite de continuidade passa.

### 4.2 Presença óptica

```c
if (red_mean < minimum_mean_level)
    mask |= G1_FAILURE_NO_SIGNAL_RED;
if (ir_mean < minimum_mean_level)
    mask |= G1_FAILURE_NO_SIGNAL_IR;
```

RED e IR são avaliados independentemente.

### 4.3 Flatline

```c
red_range = red_max - red_min;
ir_range  = ir_max - ir_min;

if (red_range < minimum_raw_range)
    mask |= G1_FAILURE_FLATLINE_RED;
if (ir_range < minimum_raw_range)
    mask |= G1_FAILURE_FLATLINE_IR;
```

`minimum_raw_range` deve permanecer conservador. G1 detecta sinal praticamente travado; baixa pulsatilidade fisiológica pertence ao G2.

### 4.4 Clipping / rails

```c
low_limit  = adc_min + rail_margin_counts;
high_limit = adc_max - rail_margin_counts;
near_rail = value <= low_limit || value >= high_limit;
clip_fraction = near_rail_count / N;

if (clip_fraction > maximum_clipping_fraction)
    mask |= G1_FAILURE_CLIPPING_*;
```

Exatamente no limite de clipping passa; acima dele falha.

## 5. Máscara de falhas

```text
1   DISCONTINUITY
2   NO_SIGNAL_RED
4   NO_SIGNAL_IR
8   FLATLINE_RED
16  FLATLINE_IR
32  CLIPPING_RED
64  CLIPPING_IR
```

A máscara é cumulativa. Exemplo: ausência de sinal em ambos os canais = `2+4=6`; flatline em ambos = `8+16=24`.

`primary_reason` usa prioridade determinística:

```text
DISCONTINUITY
> CLIPPING_RED
> CLIPPING_IR
> NO_SIGNAL_RED
> NO_SIGNAL_IR
> FLATLINE_RED
> FLATLINE_IR
```

A prioridade serve à telemetria; a máscara continua contendo todas as falhas.

## 6. Integração fail-fast

```text
signal_quality_evaluate_window()
        │
        ├─ WAITING → não estima
        ├─ ERROR   → não estima
        └─ COMPLETE
             │
             ├─ G1 INVALID → HR/SpO₂ = LOW_QUALITY
             └─ G1 VALID   → estimadores atuais podem executar
```

Quando G2–G4 existirem, o caminho correto será:

```c
if (!g1.passed) reject();
if (!g2.passed) reject();
if (!g3.passed) reject();
if (!g4.passed) reject();
quality.state = PPG_QUALITY_VALID;
```

## 7. O que o G1 cobre

| área | cobre? |
|---|---:|
| buffer/janela suficiente | parcialmente no orquestrador |
| perda/descontinuidade temporal | ✅ |
| ausência de nível óptico RED/IR | ✅ |
| sinal travado/quase constante | ✅ |
| clipping/saturação de ADC | ✅ |
| pulsatilidade | ❌ G2 |
| movimento/artefato morfológico | ❌ G2/G3 |
| largura/rise time dos pulsos | ❌ G3 |
| coerência RED↔IR | ❌ G4 |
| calibração de SpO₂ | ❌ estimador separado |
| regra terapêutica/jogo | ❌ camada superior |

## 8. Embasamento científico

- **Reddy et al.**: fundamenta rejeição precoce de nearly-zero amplitude/saturação, hierarquia e execução on-device; também alerta que thresholds de amplitude devem respeitar a faixa dinâmica do módulo.
- **Fischer et al.**: sustenta a detecção de clipping/artefatos antes de transformações que possam mascará-los e a preferência por regras simples embarcáveis.
- **Vadrevu & Manikandan**: apoia arquitetura hierárquica, janela de 5 s e separação entre testes precoces e análise pulsátil posterior.
- **MAX30102/fabricante**: fornece resolução/rails/configuração física; não fornece um SQI universal.

## 9. Pontos de ajuste

Não alterar o algoritmo do gate para “corrigir” HR/SpO₂ instáveis. Ajustes legítimos no G1 são somente os parâmetros descritos em `G1_PENDING_CALIBRATIONS.md`. Se uma janela íntegra, porém fisiologicamente ruim, ainda passar, a correção pertence a G2/G3/G4.
