# SpO₂ Wrist Sensor — I Blue It

Firmware embarcado para aquisição de PPG com MAX3010x/MAX30102, avaliação de qualidade do sinal (SQI), estimação de frequência cardíaca e SpO₂, aplicação de regras de confiança e transmissão de dados ao ecossistema I Blue It.

## Estado atual do projeto

O projeto está entrando na fase de **refatoração estrutural do módulo de qualidade de sinal**. A implementação existente de `signal_quality.c` será preservada como ponto de partida, mas sua responsabilidade será reduzida para **orquestração** de uma arquitetura SQI hierárquica, multimétrica e fail-fast.

A arquitetura alvo adota quatro gates sequenciais:

1. **G1 — Integridade:** valida o sinal bruto, continuidade, ausência de flatline e saturação/clipping.
2. **G2 — Pulsatilidade:** verifica se RED e IR apresentam estrutura pulsátil consistente usando amplitude, threshold crossings e autocorrelação.
3. **G3 — Morfologia:** avalia batimentos detectados por amplitude, largura, rise time e estabilidade batimento a batimento.
4. **G4 — Coerência RED ↔ IR:** verifica se os dois canais representam o mesmo fenômeno pulsátil em período, contagem e alinhamento temporal.

Somente janelas aprovadas pelos quatro gates podem chegar aos estimadores fisiológicos.

## Fluxo funcional

```text
MAX30102
   ↓
drivers/
   ↓
sensing/
   ├─ ppg_sampler
   └─ sample_buffer
   ↓
processing/sqi/
   ├─ G1 integridade RAW
   ├─ preprocessamento
   ├─ G2 pulsatilidade
   ├─ detecção de batimentos
   ├─ G3 morfologia
   └─ G4 coerência RED↔IR
   ↓
   ├─ INVALID → não calcular SpO₂/FC utilizáveis
   └─ VALID
        ↓
processing/hr_estimator
processing/spo2_estimator
        ↓
safety/confidence_engine
        ↓
health_frame_t
        ↓
transport/
        ↓
I Blue It / PITACO / dashboard
```

## Princípios arquiteturais congelados

- O sinal **RAW nunca é sobrescrito** pelo pipeline de processamento.
- Uma janela `INVALID` **não produz SpO₂ utilizável**.
- Thresholds do SQI não devem ficar espalhados em `#define` dentro dos algoritmos; devem pertencer a uma configuração centralizada e versionável.
- `signal_quality.c` é o **orquestrador** do SQI, não o local onde todas as métricas são implementadas.
- Features matemáticas reutilizáveis ficam separadas das regras de decisão dos gates.
- O SQI responde se o sinal é tecnicamente utilizável; o `confidence_engine` trata a confiança/estado final da saída do sistema.
- A primeira versão é dimensionada **sem o pegador anatômico**, portanto os thresholds devem permanecer configuráveis e sujeitos a calibração posterior.

## Organização alvo

```text
main/
├── drivers/
├── sensing/
├── processing/
│   ├── sqi/
│   │   ├── signal_quality.c
│   │   ├── signal_quality.h
│   │   ├── signal_quality_types.h
│   │   ├── gates/
│   │   │   ├── g1_integrity/
│   │   │   ├── g2_pulsatility/
│   │   │   ├── g3_morphology/
│   │   │   └── g4_channels/
│   │   ├── preprocess/
│   │   └── features/
│   ├── hr_estimator.*
│   └── spo2_estimator.*
├── safety/
├── storage/
├── transport/
└── app/
```

## Janela de processamento

Baseline atual:

- janela SQI: **5 s**;
- passo inicial sugerido: **1 s**;
- número de amostras: derivado de `Fs`, não fixado manualmente.

```text
N = Fs × janela_em_segundos
```

## Documentação

- [Arquitetura do firmware](docs/ARCHITECTURE.md)
- [Arquitetura do SQI](docs/SQI_ARCHITECTURE.md)
- [Decisões arquiteturais](docs/DECISIONS.md)
- [Configuração](docs/CONFIGURATION.md)
- [Estratégia de testes](docs/TESTING.md)
- [Roadmap de implementação](docs/ROADMAP.md)
- [Referências técnicas e científicas](docs/REFERENCES.md)
