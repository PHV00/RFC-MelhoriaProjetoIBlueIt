# Configuração do SQI

## Objetivo

Remover thresholds escondidos dos algoritmos e permitir calibração do MAX30102 sem alterar código de decisão.

## Estrutura alvo

```c
typedef struct {
    uint32_t window_ms;
    uint32_t step_ms;

    g1_integrity_config_t integrity;
    g2_pulsatility_config_t pulsatility;
    g3_morphology_config_t morphology;
    g4_channels_config_t channels;

    uint32_t config_version;
} sqi_config_t;
```

## Exemplos de grupos de parâmetros

### G1
- faixa mínima RAW por canal;
- margem de saturação;
- fração/contagem máxima de amostras saturadas;
- tolerância de intervalo temporal entre amostras.

### G2
- amplitude mínima;
- limites para threshold crossings;
- ACF mínima;
- faixa de lag/período aceitável.

### G3
- largura mínima/máxima;
- rise time mínimo/máximo;
- limites de variabilidade batimento a batimento;
- número mínimo de batimentos válidos.

### G4
- tolerância de diferença de período RED↔IR;
- tolerância de contagem;
- tolerância de alinhamento temporal.

## Regra importante

Thresholds publicados em artigos não devem ser copiados cegamente quando dependem de resolução ADC, sensor, taxa de amostragem, condicionamento ou protocolo diferentes.

O projeto deve registrar a origem de cada parâmetro:

- literatura;
- derivação matemática;
- limite do hardware;
- valor provisório;
- calibração experimental.

## Perfis

Prevê-se pelo menos:

```text
SQI_PROFILE_NO_GRIP
SQI_PROFILE_WITH_GRIP
```

A arquitetura e os gates permanecem os mesmos; apenas parâmetros podem mudar após caracterização mecânica/óptica.
