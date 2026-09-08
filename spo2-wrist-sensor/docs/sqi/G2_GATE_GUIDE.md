# G2 — Pulsatilidade: guia técnico e científico

## 1. Objetivo do Gate 02

O G2 responde apenas à pergunta:

> **Existe componente pulsátil periódica, com energia e repetição suficientes, nos canais RED e IR após o G1 ter aprovado a integridade técnica da janela?**

O G2 não avalia ainda a forma detalhada de cada beat (G3) e não decide se RED e IR representam exatamente o mesmo trem de pulsos (G4).

```text
RAW MAX30102
    │
    ▼
G1 INTEGRITY
    │ PASS
    ▼
PREPROCESS em cópia
    │
    ├── RED processado
    └── IR processado
          │
          ▼
       G2 PULSATILITY
       ├── energia/amplitude AC
       ├── threshold crossings
       ├── autocorrelação
       └── período/lag plausível
          │
     ┌────┴────┐
    FAIL      PASS
     │          │
 INVALID       G3
```

## 2. Por que o G2 existe

A validação do G1 mostrou janelas tecnicamente íntegras que ainda apresentavam HR instável ou impossível. Isso é esperado: uma janela pode ter sinal, não estar flat, não estar saturada e possuir timestamps íntegros, mas ainda assim não conter uma sequência pulsátil confiável.

G1 e G2 não devem ser fundidos:

```text
G1: "os dados chegaram tecnicamente utilizáveis?"
G2: "há comportamento pulsátil periódico utilizável?"
```

Essa separação preserva a hierarquia fail-fast e evita que métricas fisiológicas mais caras sejam executadas em sinais estruturalmente inválidos.

## 3. Fundamentação

### Vadrevu & Manikandan

Referência principal do G2. O trabalho propõe SQA hierárquico em tempo real com janela de 5 s e processamento simples para plataforma embarcada. O sinal é pré-processado para remover baseline lentamente variável e as regras seguintes exploram amplitude, threshold crossings e autocorrelação.

Adotamos do trabalho:

- arquitetura hierárquica;
- janela de 5 s como baseline;
- remoção de baseline antes das features periódicas;
- threshold crossings como indicador de atividade excessiva/ruído;
- autocorrelação como indicador de periodicidade;
- rejeição antes de estimadores mais caros.

Não copiamos thresholds numéricos como universais. Eles dependem da escala, sensor, resolução e população/dataset.

### Reddy et al.

Reforça a arquitetura on-device e a estratégia de descartar cedo sinais não utilizáveis, reduzindo processamento e falsos alarmes. No nosso desenho, várias regras de desconexão/saturação ficam no G1; o princípio hierárquico continua no G2.

### Karlen et al.

Mostra que PPG confiável possui repetição de pulsos e que correlação entre pulsos pode quantificar qualidade. O trabalho também reforça que artefatos e baixa perfusão comprometem a derivação de sinais vitais. A comparação morfológica detalhada ficará principalmente no G3, mas o princípio de repetição sustenta o uso de periodicidade no G2.

### Orphanidou et al.

O SQI busca determinar se HR confiável pode ser derivada de PPG e usa regras fisiológicas/temporais e template. No nosso projeto, a plausibilidade periódica é separada no G2 e a avaliação beat/template é reservada para G3.

### Sukor et al. / Fischer et al.

Esses trabalhos sustentam principalmente G3 porque analisam morfologia beat-to-beat, amplitude, largura, rise time e estabilidade. São citados aqui para deixar explícita a fronteira: G2 não deve absorver morfologia detalhada.

## 4. Arquitetura de software

Estrutura preparada:

```text
processing/sqi/
├── preprocess/
│   ├── ppg_preprocess.c
│   └── ppg_preprocess.h
├── features/
│   ├── threshold_crossing.c/.h
│   ├── autocorrelation.c/.h
│   └── beat_detector.c/.h      # reservado para G3
└── gates/
    └── g2_pulsatility/
        ├── gate_pulsatility.c
        └── gate_pulsatility.h
```

Nesta primeira etapa, `threshold_crossing`, `autocorrelation` e o núcleo isolado de `gate_pulsatility` foram implementados na branch `feature/sqi-g2-pulsatility`. **O G2 ainda não está conectado ao pipeline de produção.**

Essa decisão é proposital: primeiro validamos a matemática isolada, depois fechamos o pré-processamento e somente então integramos após o G1.

## 5. Contrato inicial do gate

Entrada:

```c
const float *red_processed;
const float *ir_processed;
size_t count;
float sample_rate_hz;
const g2_pulsatility_config_t *config;
```

Saída por canal:

```text
AC RMS
crossing count
ACF peak
best lag
period BPM
```

A máscara inicial distingue RED e IR:

```text
bit 0   LOW_RMS_RED
bit 1   LOW_RMS_IR
bit 2   CROSSINGS_RED
bit 3   CROSSINGS_IR
bit 4   LOW_ACF_RED
bit 5   LOW_ACF_IR
bit 6   PERIOD_RED
bit 7   PERIOD_IR
```

O gate passa apenas quando `failure_mask == 0`.

## 6. Features

### 6.1 AC RMS

```text
RMS = sqrt(sum(x[n]^2) / N)
```

É uma métrica simples de energia da componente processada. No projeto ela é uma feature de engenharia para evitar tratar um sinal quase sem AC como pulsátil. O threshold final precisa ser calibrado no MAX30102.

### 6.2 Threshold crossings

Após baseline removida, contamos mudanças de lado em relação ao nível zero:

```text
negativo -> positivo = 1 crossing
positivo -> negativo = 1 crossing
```

Platôs exatamente no threshold são ignorados para não gerar dupla contagem.

Poucos crossings podem indicar ausência de oscilação; crossings excessivos podem indicar ruído de alta frequência/artefato. Os limites finais serão calibrados.

### 6.3 Autocorrelação

Para cada lag `k` na faixa permitida:

```text
R[k] = sum(x[n] x[n+k]) /
       sqrt(sum(x[n]^2) sum(x[n+k]^2))
```

O melhor lag fornece:

```text
period_bpm = 60 * Fs / best_lag
```

O `acf_peak` mede quão semelhante o sinal é a ele mesmo após um deslocamento correspondente a um possível período pulsátil.

### 6.4 Faixa de lag

A faixa de busca é derivada de `minimum_pulse_bpm` e `maximum_pulse_bpm` fornecidos pela configuração:

```text
min_lag = ceil(Fs * 60 / max_bpm)
max_lag = floor(Fs * 60 / min_bpm)
```

Os limites fisiológicos finais não serão fixados nesta preparação sem protocolo de calibração/validação.

## 7. Pré-processamento — pendência imediata

Vadrevu usa filtro Butterworth HP de terceira ordem com cutoff de 0,5 Hz antes das features. Essa é nossa baseline científica para investigação, mas ainda precisamos decidir e validar a implementação definitiva para `Fs=100 Hz` no ESP32.

Não devemos reutilizar silenciosamente o `detrend_linear()` legado como se fosse equivalente ao filtro do artigo.

Plano:

1. implementar filtro em `preprocess/ppg_preprocess.c` trabalhando em cópia;
2. validar resposta do filtro em sinais sintéticos;
3. comparar com referência offline;
4. verificar custo/memória no ESP32-C3;
5. fazer análise de sensibilidade do cutoff;
6. congelar versão do preprocessamento antes de calibrar thresholds do G2.

## 8. Parâmetros ainda científicos/experimentais

```text
minimum_ac_rms             PENDENTE
minimum_crossings          PENDENTE
maximum_crossings          PENDENTE
minimum_acf_peak           PENDENTE
minimum_pulse_bpm          PENDENTE
maximum_pulse_bpm          PENDENTE
preprocess filter/cutoff    PENDENTE
```

Nenhum valor usado em unit test deve ser tratado como threshold final. Os testes sintéticos usam valores exclusivamente para testar lógica.

## 9. Metodologia de calibração

### Dataset de desenvolvimento

Coletar/usar janelas rotuladas contendo pelo menos:

```text
STABLE_PPG
LOW_PULSATILITY
MOTION
RANDOM_NOISE
TRANSIENT
PERIODIC_NON_PPG
```

Complementar com datasets públicos de PPG com movimento para features independentes da escala RAW.

### Split

Nunca dividir janelas sobrepostas do mesmo trecho entre treino/calibração e teste. Dividir por participante ou sessão.

### Processo

```text
1. congelar preprocessamento
2. extrair RMS/crossings/ACF/lag
3. analisar distribuições por classe
4. escolher candidatos no conjunto de desenvolvimento
5. calcular sensibilidade/especificidade/FAR/FRR
6. avaliar ROC/PR quando aplicável
7. congelar thresholds candidatos
8. executar hold-out sem retuning
9. análise de sensibilidade
10. versionar configuração e dataset
```

## 10. Testes previstos

### Matemática host

- seno limpo em frequência conhecida -> PASS;
- amplitude quase zero -> LOW_RMS;
- sinal constante após preprocess -> falha de energia/crossings/ACF;
- ruído rápido -> crossings excessivos;
- periodicidade forte -> ACF alta e lag correto;
- frequência fora da faixa -> rejeição;
- RED bom / IR ruim e vice-versa -> isolamento por canal;
- argumentos/configuração inválidos -> erro de contrato;
- entrada imutável.

### Integração futura

```text
G1 PASS
 -> preprocess
 -> G2
 -> G2 FAIL bloqueia HR/SpO2
 -> telemetria mostra failed_gate=G2_PULSATILITY
```

Essa integração só será feita após a validação isolada das features e do preprocessamento.

## 11. Critério para considerar G2 funcionalmente fechado

- preprocessamento validado;
- features matematicamente testadas;
- gate RED/IR testado por unit tests;
- integrado após G1;
- fail-fast comprovado;
- telemetria G2 disponível;
- hardware real exercitado;
- fault cases determinísticos exercitados;
- thresholds explicitamente marcados como provisórios ou calibrados;
- documentação atualizada.

## 12. Relação com o I Blue It

O G2 faz parte do tratamento do sinal fisiológico antes que HR/SpO2 sejam usados como informação de segurança. Isso se alinha ao papel atribuído aos biosinais no fluxo inconsciente da arquitetura multimodal 123-SGR: sinais fisiológicos devem ser tratados e avaliados antes de alimentar decisões/adaptações do jogo.
