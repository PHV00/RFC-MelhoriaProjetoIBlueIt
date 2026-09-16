# Configuração do SQI

## Objetivo

Manter todos os thresholds explícitos, versionáveis e calibráveis. Um threshold não deve ficar escondido dentro de uma feature ou regra sem registrar sua origem.

## Configuração atual do G1

```c
.sqi = {
    .window_ms = 5000u,
    .step_ms = 1000u,
    .minimum_quality_score = 0.55f,
    .g1_integrity = {
        .adc_min_value = 0u,
        .adc_max_value = 0x03FFFFu,
        .rail_margin_counts = 1u,
        .minimum_mean_level = 5000u,
        .minimum_raw_range = 20u,
        .maximum_clipping_fraction = 0.01f,
        .minimum_continuity_fraction = 0.95f,
        .maximum_interval_deviation_fraction = 0.40f
    }
}
```

## Proveniência

| parâmetro | origem/status |
|---|---|
| `window_ms=5000` | baseline apoiada pela literatura de SQI hierárquico; manter e avaliar sensibilidade |
| `step_ms=1000` | escolha de engenharia para latência/custo |
| `adc_min_value=0` | hardware/formato ADC |
| `adc_max_value=0x03FFFF` | hardware: leitura em 18 bits |
| `rail_margin_counts=1` | provisório; calibrar em bancada |
| `minimum_mean_level=5000` | provisório; calibrar NO_CONTACT × contato utilizável |
| `minimum_raw_range=20` | provisório/conservador; calibrar apenas como flatline/stuck, sem invadir G2 |
| `maximum_clipping_fraction=0.01` | provisório; calibrar por sweep de corrupção |
| `minimum_continuity_fraction=0.95` | provisório; calibrar no fluxo de aquisição + falhas injetadas |
| `maximum_interval_deviation_fraction=0.40` | provisório; calibrar distribuição de `dt` reconstruído |
| `minimum_quality_score=0.55` | legado; não é threshold do G1 e será revisto após G2–G4 |

## Regra de origem

Cada parâmetro deve ser marcado como uma destas classes:

1. **hardware-derived** — fixado pela configuração/representação do MAX30102;
2. **literature-backed** — estrutura ou baseline suportada pela literatura, mas ainda sujeita à nossa implementação;
3. **engineering** — escolha de latência/custo/arquitetura;
4. **provisional** — valor inicial para permitir desenvolvimento;
5. **empirically calibrated** — valor obtido em dataset do perfil real e validado em hold-out.

## Estrutura alvo G1–G4

```c
typedef struct {
    uint32_t window_ms;
    uint32_t step_ms;
    g1_integrity_config_t g1_integrity;
    g2_pulsatility_config_t g2_pulsatility;
    g3_morphology_config_t g3_morphology;
    g4_channels_config_t g4_channels;
    uint32_t config_version;
} sqi_config_t;
```

G2 deverá parametrizar amplitude pulsátil, crossings, ACF e período; G3 largura/rise time/estabilidade/quantidade de beats; G4 diferenças RED↔IR de período, contagem e alinhamento.

## Perfis

A primeira calibração é `NO_GRIP`, deliberadamente mais tolerante à variabilidade mecânica. Depois o mesmo protocolo deve ser repetido com o pegador anatômico. Só criar `WITH_GRIP` se os dados mostrarem que thresholds distintos são justificáveis.

Detalhes: `docs/sqi/G1_CALIBRATION_PLAN.md`, `docs/sqi/G1_PENDING_CALIBRATIONS.md` e `docs/sqi/PENDING_GATES_G2_G4.md`.
