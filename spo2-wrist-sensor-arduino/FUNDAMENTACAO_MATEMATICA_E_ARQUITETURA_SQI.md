# Fundamentação matemática e arquitetura do SQI — MAX30102 / Arduino Uno-Nano

## 1. Objetivo deste documento

Este documento concentra, em um único lugar, a relação entre:

1. a grandeza matemática utilizada;
2. o motivo de ela existir;
3. o Gate em que ela é aplicada;
4. o ponto do código em que ela aparece ou deverá aparecer;
5. a decisão arquitetural associada;
6. o que vem diretamente da literatura e o que é adaptação de engenharia deste projeto.

A regra metodológica é:

> nenhuma fórmula, threshold ou transformação deve aparecer no firmware sem que seja possível explicar o que ela mede, por que é necessária e em qual etapa da pipeline ela atua.

A arquitetura alvo permanece:

```text
MAX30102
   |
   v
RAW RED + IR (18 bits)
   |
   +--------------------------+
   |                          |
   v                          v
G1 — Integridade         Packed18
   |                     3 bytes/amostra
   | PASS                     |
   +--------------------------+
                              |
                              v
                       G2 — Pulsatilidade
                              |
                              v
                       G3 — Morfologia
                              |
                              v
                       G4 — RED <-> IR
                              |
                              v
                          PPG VALID
                              |
                              v
                           FC / SpO2
```

A sequência G1 -> G2 -> G3 -> G4 é uma **arquitetura do projeto**, apoiada em princípios e métodos da literatura. Ela não deve ser apresentada como uma sequência literal proposta por um único artigo.

---

# 2. Representação do sinal e armazenamento de 18 bits

## 2.1 Limite do ADC

Com resolução de 18 bits, o maior código possível é:

```text
ADC_MAX = 2^18 - 1
        = 262143
```

No código:

```cpp
static const uint32_t G1_ADC_MAX_CODE = 262143UL;
```

O uso de `uint32_t` na leitura é intencional: C/C++ não possui um tipo inteiro nativo de 18 bits.

## 2.2 Por que uint16_t não é suficiente

O maior valor de um `uint16_t` é:

```text
2^16 - 1 = 65535
```

Como as amostras reais do MAX30102 já ultrapassaram 65535 em bancada, armazenar diretamente o RAW 18-bit em `uint16_t` perde informação.

Exemplo observado no projeto:

```text
RAW = 100999

100999 - 65536 = 35463
```

Um cast direto para 16 bits pode produzir exatamente esse tipo de wrap/truncamento.

## 2.3 Packed18 — divisão em 3 bytes

Cada amostra `x` é limitada aos 18 bits válidos:

```text
x18 = x AND 0x3FFFF
```

A codificação usada é:

```text
b0 =  x        AND 0xFF
b1 = (x >> 8)  AND 0xFF
b2 = (x >> 16) AND 0x03
```

Assim:

```text
byte 0 -> bits  0..7
byte 1 -> bits  8..15
byte 2 -> bits 16..17
```

Os 6 bits superiores do terceiro byte ficam zerados.

A reconstrução é:

```text
x =
    b0
  OR (b1 << 8)
  OR ((b2 AND 0x03) << 16)
```

No código, isso está em:

```text
sqi_flow_v5_baseline/packed_18_storage.ino
```

nas funções:

```cpp
packed18Encode(...)
packed18Decode(...)
```

A propriedade que deve permanecer verdadeira é:

```text
decode(encode(x)) = x
```

para todo `x` no intervalo:

```text
0 <= x <= 262143
```

A validação experimental já obteve:

```text
PACK18_RESULT=PASS
mismatch=0
```

inclusive em janelas nas quais:

```text
RED>65535 = 100/100
IR>65535  = 100/100
```

## 2.4 Economia de SRAM

Para 100 amostras de RED e 100 de IR:

```text
uint32_t:
2 canais x 100 amostras x 4 B = 800 B

Packed18:
2 canais x 100 amostras x 3 B = 600 B
```

Economia absoluta:

```text
800 - 600 = 200 B
```

Economia relativa ao buffer:

```text
200 / 800 = 0,25 = 25%
```

Como o ATmega328P possui 2048 B de SRAM:

```text
200 / 2048 ~= 0,0977 = 9,8%
```

Portanto, a representação Packed18 economiza aproximadamente 9,8% de toda a SRAM física do microcontrolador sem reduzir a resolução da aquisição.

### Natureza acadêmica desta decisão

O Packed18 é uma **decisão de engenharia do projeto**, não uma exigência da literatura de SQI. Sua defesa é feita por:

- restrição objetiva de SRAM;
- preservação exata dos 18 bits;
- teste de round-trip com `mismatch=0`;
- ausência de alteração do valor reconstruído usado pelos Gates posteriores.

---

# 3. Gate 01 — Integridade do sinal RAW

## 3.1 Pergunta

> Existe sinal óptico minimamente utilizável nos dois canais e a aquisição está livre de condições obviamente inválidas?

O G1 não tenta reconhecer uma onda PPG fisiológica. Ele elimina cedo condições básicas inválidas.

Fundamentação principal atualmente registrada:

- Reddy et al. (2020), DOI `10.1109/TIM.2020.2971132`;
- Fischer et al., DOI `10.1109/JBHI.2016.2518202`.

Do primeiro usamos principalmente o princípio de detecção precoce de sinal nearly-zero/desconexão e saturação. Do segundo, o princípio de verificar clipping/artefatos no RAW antes de análises posteriores.

## 3.2 Média da janela

### Nome

Média aritmética / nível médio da janela.

### Fórmula

Para `N` amostras:

```text
mean = (1/N) * sum(x[i])
```

### Função no Gate

Detectar ausência clara de sinal óptico.

No código:

```cpp
redMean = g1RedSum / g1SampleCount;
irMean  = g1IrSum  / g1SampleCount;
```

Regra atual de bancada:

```text
mean < 5000 -> LOW_SIGNAL
```

O valor 5000 é um threshold provisório caracterizado na montagem atual; não é apresentado como threshold universal ou clínico.

## 3.3 Faixa dinâmica

### Nome

Range / amplitude pico-a-pico da janela.

### Fórmula

```text
range = max(x) - min(x)
```

### Função no Gate

Detectar sinal praticamente constante/flatline.

No código:

```cpp
redRange = g1RedMax - g1RedMin;
irRange  = g1IrMax  - g1IrMin;
```

Regra atual:

```text
range < 500 -> FLATLINE
```

Novamente, `500` é parâmetro provisório de bancada.

## 3.4 Saturação

### Nome

Detecção de clipping no limite superior.

### Fórmula

```text
SAT_LEVEL = ADC_MAX - SAT_MARGIN
```

Com os valores atuais:

```text
SAT_LEVEL = 262143 - 2048
          = 260095
```

Uma amostra é marcada como saturada quando:

```text
x >= 260095
```

No código:

```cpp
if (red >= G1_SATURATION_LEVEL) ...
if (ir  >= G1_SATURATION_LEVEL) ...
```

Na versão atual qualquer amostra saturada reprova a janela:

```text
G1_MAX_SATURATED_SAMPLES = 0
```

## 3.5 Arquitetura de memória do G1

O G1 é streaming.

Ele mantém apenas:

```text
min
max
soma
contador
contador de saturação
```

Não precisa armazenar a janela inteira.

Consequência:

```text
RAW uint32_t temporário
        |
        +--> G1 atualiza acumuladores
        |
        +--> Packed18 guarda janela para Gates posteriores
```

Isso separa uma verificação barata de integridade da análise temporal mais cara.

## 3.6 Estado experimental

Já observado:

```text
sem dedo:
mean ~500-600
=> FAIL / LOW_SIGNAL

dedo estável:
mean ~1e5
=> PASS

saturação normal:
sat = 0
```

Também foi observado que janelas de colocação/remoção do dedo podem passar no G1. Isso não é necessariamente erro do Gate: um transiente grande possui média e range suficientes. A identificação de estrutura pulsátil é responsabilidade do G2.

---

# 4. Gate 02 — Pulsatilidade

## 4.1 Pergunta

> A janela aprovada no G1 contém uma estrutura pulsátil/periódica compatível com PPG, em vez de apenas variação ou transientes?

A base selecionada para o G2 é:

- Vadrevu et al. (2019), *Real-Time PPG Signal Quality Assessment System for Improving Battery Life and False Alarms*, DOI `10.1109/TCSII.2019.2891636`.

Nossa V1 preservada no ESP32 também utiliza:

- amplitude AC/RMS;
- threshold crossings;
- autocorrelação;
- FZCP;
- pico de ACF;
- best lag;
- período estimado.

A estratégia atual é **replicar adequadamente o núcleo do método no nosso hardware**, e não inventar um novo G2.

## 4.2 Remoção do componente DC

### Nome

Centralização pela média.

### Fórmula

```text
mu = (1/N) * sum(x[i])

x_ac[i] = x[i] - mu
```

### Função

O PPG possui um grande nível DC e uma componente variável muito menor. O G2 interessa-se pela componente temporal/pulsátil.

Arquiteturalmente:

```text
Packed18
   |
decode x[i]
   |
   +--> calcula mu
   |
   +--> usa x[i] - mu sob demanda
```

Não é necessário manter um novo `uint32_t[100]`.

## 4.3 Amplitude AC por RMS

### Nome

RMS da componente AC.

### Fórmula

```text
AC_RMS =
sqrt(
  (1/N) * sum( x_ac[i]^2 )
)
```

### Função

Responder se existe energia/amplitude variável suficiente após a remoção do DC.

Duas janelas podem ter o mesmo nível óptico médio, mas amplitudes pulsáteis muito diferentes. O RMS ajuda a separar:

```text
DC alto + quase nenhuma variação
```

de:

```text
DC alto + componente pulsátil mensurável
```

### Estado do código

A V1/ESP32 já possui cálculo RMS.

Na branch diagnóstica atual do Uno o `MAD/DC` exploratório foi removido. O firmware agora calcula:

```text
absoluteAmplitude = max(|x[i] - mean|)
AC_RMS            = sqrt(mean((x[i] - mean)^2))
```

A primeira métrica representa a família de amplitude de forma explícita no port; `AC_RMS` é mantido como métrica auxiliar herdada da V1. Nenhum threshold de decisão está congelado.

## 4.4 Threshold crossings

### Nome

Contagem/taxa de cruzamentos de limiar.

Após centralização, o limiar natural inicial é zero:

```text
threshold = 0
```

Definimos o lado da amostra:

```text
side[i] =
  +1, se x_ac[i] > 0
  -1, se x_ac[i] < 0
   0, se x_ac[i] = 0
```

Um crossing ocorre quando amostras não nulas consecutivas trocam de lado:

```text
side[i] != side[i-1]
```

### Função

Detectar comportamento oscilatório repetido.

Um transiente único pode produzir range muito grande no G1, mas tende a apresentar uma assinatura de crossings diferente de uma onda pulsátil repetitiva.

### Código de referência

Na V1 preservada:

```text
processing/sqi/features/threshold_crossing.c
```

A implementação ignora pontos exatamente no threshold e conta mudanças de lado.

## 4.5 Autocorrelação normalizada

### Nome

Autocorrelação / correlação do sinal com uma versão atrasada de si mesmo.

Para um lag `k`, a formulação de referência usada na V1 é:

```text
              sum( x[i] * x[i+k] )
R(k) = -----------------------------------
       sqrt( sum(x[i]^2) * sum(x[i+k]^2) )
```

### Função

Medir repetição temporal.

Se a onda contém pulsos semelhantes separados aproximadamente pelo mesmo período, a correlação aumenta quando o lag se aproxima dessa distância.

Em termos intuitivos:

```text
lag pequeno/incorreto -> formas desalinhadas -> correlação menor

lag ~ período         -> pulsos alinhados    -> correlação maior
```

### Estado do código

A aproximação exploratória por:

```text
(energyA + energyB)/2
```

foi removida antes da caracterização experimental.

A branch diagnóstica atual do Uno usa a normalização:

```text
              sum(a[i] * b[i])
R(k) = --------------------------------
       sqrt(sum(a[i]^2)) * sqrt(sum(b[i]^2))
```

sem criar buffers `float[100]`. As energias e o numerador são acumulados em 64 bits; `float` é usado apenas na etapa de raiz/normalização. A saída é escalada para `[-1000,+1000]` para telemetria.

## 4.6 Melhor lag

### Nome

Best lag / atraso dominante.

### Fórmula

```text
k_best = arg max R(k)
```

dentro da faixa de lags permitida.

### Função

Encontrar o atraso no qual a forma de onda mais se repete.

## 4.7 Período e periodicidade por minuto

Se a taxa efetiva é `fs` amostras/s:

```text
T = k_best / fs
```

onde `T` é o período dominante em segundos.

A periodicidade correspondente por minuto é:

```text
cycles_per_minute = 60 * fs / k_best
```

Exemplo:

```text
fs = 25 amostras/s
k_best = 25

T = 25/25 = 1 s

cycles/min = 60*25/25 = 60
```

### Função

Verificar se a estrutura repetitiva encontrada está em uma faixa temporal plausível.

Esse valor **não é ainda a FC clínica/final**. É a periodicidade dominante detectada pelo Gate de qualidade.

## 4.8 FZCP — First Zero-Crossing Point da ACF

A nossa V1 preservada possui uma métrica de primeiro cruzamento por zero da autocorrelação.

Conceitualmente:

```text
FZCP = primeiro k > 0 em que R(k) muda de positivo para <= 0
```

e:

```text
FZCP_s = FZCP_lag / fs
```

### Estado metodológico

A métrica existe na implementação V1. Fontes secundárias que descrevem o método de Vadrevu indicam que o primeiro zero-crossing da ACF, o pico máximo e o lag desse pico são usados para caracterizar a ACF.

Por isso, a branch diagnóstica atual passou a extrair:

```text
FZCP
ACF peak
peak lag
```

mas ainda **sem thresholds de PASS/FAIL**. O próximo passo é caracterizar essas grandezas no nosso MAX30102.

## 4.9 Arquitetura de memória do G2

O G2 deve reutilizar:

```text
Packed18 RED[100] = 300 B
Packed18 IR [100] = 300 B
```

A leitura lógica de uma amostra será:

```text
Packed18[i]
   |
decode
   |
uint32_t temporário
   |
cálculo
   |
descarta temporário
```

Regra arquitetural:

> não recriar `uint32_t red[100]` e `uint32_t ir[100]`.

A economia obtida no armazenamento somente é preservada se os Gates posteriores operarem sobre a janela compacta ou sobre pequenos workspaces controlados.

---

# 5. Gate 03 — Morfologia

## 5.1 Pergunta

> Os pulsos detectados apresentam forma e estabilidade beat-to-beat compatíveis com uma onda PPG utilizável?

Fundamentação atualmente selecionada:

- Sukor et al. (2011), DOI `10.1088/0967-3334/32/3/008`;
- Fischer et al., DOI `10.1109/JBHI.2016.2518202`.

A documentação atual prevê:

- amplitude do pulso;
- largura do pulso;
- rise time;
- duração;
- estabilidade/similaridade entre batimentos.

## 5.2 Estado matemático

**Ainda não existe uma formulação matemática final adotada no código Arduino.**

Portanto, para não transformar uma intenção em falsa decisão metodológica, as expressões abaixo são apenas definições naturais das grandezas planejadas e devem ser confrontadas com os artigos antes da implementação.

### Amplitude do pulso — definição candidata

```text
A_pulse = x_peak - x_foot
```

### Largura/duração — definição candidata

```text
width = t_end - t_start
```

### Rise time — definição candidata

```text
rise_time = t_peak - t_start
```

### Estabilidade beat-to-beat

Ainda deve ser escolhida a métrica concreta. Possibilidades incluem comparação de amplitude/largura entre pulsos ou uma medida de similaridade entre formas.

### Regra

Nenhuma dessas fórmulas será considerada parte definitiva do G3 até que:

1. o G2 esteja validado;
2. o método selecionado de Sukor/Fischer seja confrontado com a implementação desejada;
3. o custo de memória/CPU seja avaliado no ATmega328P.

---

# 6. Gate 04 — Coerência RED <-> IR

## 6.1 Pergunta

> Os dois canais descrevem uma estrutura temporal compatível para permitir o uso conjunto na oximetria?

Fundamentação geral:

- Desquins et al. (2022), DOI `10.3390/app12199582`.

O G4 é uma **adaptação arquitetural do projeto** para o caso dual-channel do MAX30102. Não afirmamos que a survey proponha um “Gate 04” nessa forma.

## 6.2 Quantidade mínima prevista

Como o G2 gera um lag dominante para cada canal:

```text
k_RED
k_IR
```

uma medida direta de discordância é:

```text
delta_k = |k_RED - k_IR|
```

ou, em período:

```text
delta_T = |T_RED - T_IR|
```

### Função

Verificar se RED e IR estão descrevendo aproximadamente a mesma periodicidade.

### Estado matemático

A fórmula de diferença é trivial; o que **não está definido** é a tolerância de aceitação.

Não existe atualmente um threshold universal RED<->IR adotado no projeto.

A tolerância deverá ser caracterizada experimentalmente no MAX30102 e poderá ser expressa, por exemplo, de forma absoluta ou relativa. Essa escolha ainda não está congelada.

## 6.3 Papel arquitetural

O G4 não substitui G2 nem G3.

```text
RED bom isoladamente ----                          +--> coerentes entre si? --> PPG VALID
IR bom isoladamente -----/
```

A coerência só faz sentido depois que cada canal já possui evidência suficiente de integridade, pulsatividade e morfologia.

---

# 7. Relação matemática entre os Gates

A hierarquia pode ser lida como um aumento progressivo da complexidade da pergunta:

```text
G1
"o sinal existe e nao esta obviamente corrompido?"
   |
   | mean, range, saturation
   v

G2
"o sinal varia de modo pulsatile e repetitivo?"
   |
   | AC/RMS, crossings, ACF, lag, periodo
   v

G3
"os pulsos encontrados possuem forma estavel/aceitavel?"
   |
   | amplitude, largura, rise time, estabilidade
   v

G4
"RED e IR descrevem o mesmo evento temporal?"
   |
   | comparacao de lag/periodo e outras metricas a definir
   v

PPG VALID
```

A ordem também é computacional:

```text
barato --------------------------------------------------> mais caro
G1 streaming -> G2 temporal -> G3 beat-to-beat -> G4 dual-channel
```

Uma janela rejeitada cedo não precisa executar processamento posterior.

---

# 8. Mapa fórmula -> código

| Gate/camada | Grandeza | Fórmula/resumo | Código atual |
|---|---|---|---|
| aquisição | ADC max | `2^18-1=262143` | `gate_01_integrity.ino` |
| Packed18 | encode | bytes 0..7, 8..15, 16..17 | `packed_18_storage.ino::packed18Encode` |
| Packed18 | decode | OR dos três bytes deslocados | `packed_18_storage.ino::packed18Decode` |
| G1 | média | `sum/N` | `gate1Evaluate` |
| G1 | range | `max-min` | `gate1Evaluate` |
| G1 | saturação | `ADC_MAX-margin` | `gate1AddSample` / `gate1Evaluate` |
| G2 | centralização | `x-mean` | a portar da V1 |
| G2 | AC RMS | `sqrt(mean(x_ac^2))` | V1/ESP32; ainda não final no Uno |
| G2 | crossings | mudanças de lado de zero | V1/ESP32; a portar |
| G2 | ACF | correlação normalizada em lag `k` | V1/ESP32; versão Uno atual é provisória |
| G2 | best lag | `argmax R(k)` | V1 + diagnóstico Uno |
| G2 | período | `k/fs` | V1 + diagnóstico Uno |
| G2 | ciclos/min | `60*fs/k` | V1 + diagnóstico Uno |
| G3 | amplitude | `peak-foot` | planejado, não congelado |
| G3 | width | `t_end-t_start` | planejado, não congelado |
| G3 | rise time | `t_peak-t_start` | planejado, não congelado |
| G4 | diferença de lag | `abs(k_RED-k_IR)` | planejado, threshold não definido |

---

# 9. O que é literatura e o que é projeto

## Diretamente apoiado pelos trabalhos selecionados

- avaliação da qualidade antes de utilizar variáveis fisiológicas;
- detecção de ausência/saturação/clipping no início da pipeline;
- uso de características de amplitude/pulsatilidade;
- threshold crossings;
- autocorrelação para estrutura temporal;
- análise morfológica da onda;
- avaliação multimétrica da qualidade.

## Adaptações/decisões do projeto

- exatamente quatro Gates;
- ordem concreta G1 -> G2 -> G3 -> G4;
- Packed18 em 3 bytes;
- execução streaming do G1;
- reaproveitamento do Packed18 pelos Gates posteriores;
- thresholds do MAX30102;
- G4 dedicado à coerência RED/IR;
- tamanho exato de janela usado em cada etapa;
- tipos inteiros/escala fixa adotados para caber no ATmega328P.

Essas decisões não são um problema acadêmico desde que sejam explicitamente apresentadas como adaptações de engenharia e validadas experimentalmente.

---

# 10. Estado atual e regra para o próximo passo

## Fechado/validado em bancada

```text
MAX30102 RAW
G1 de integridade
Packed18 lossless
```

## Em revisão antes da validação

```text
G2 de pulsatilidade
```

O port diagnóstico foi alinhado antes da coleta de thresholds. Atualmente ele extrai, sem decidir qualidade:

```text
absolute amplitude
AC RMS auxiliar
threshold crossing count/rate
ACF normalizada
FZCP
ACF peak
peak lag
periodicidade derivada
tempo de execução no ATmega328P
```

A validação experimental deve agora responder duas perguntas independentes:

1. as features separam adequadamente dedo estável de transientes/movimento?
2. o custo de CPU é compatível com a FIFO e com a futura execução conjunta com PITACO?

## Ainda não implementado

```text
G3
G4
FC/SpO2 integrado à pipeline
integração final com PITACO / I Blue It
```

A próxima implementação do G2 deve partir deste documento e da V1 preservada, marcando para cada elemento:

```text
IGUAL AO MÉTODO-BASE
ADAPTADO PARA ATmega328P
REMOVIDO / ADIADO
```

Isso permite rastrear a origem matemática de cada decisão e evita que otimizações de hardware sejam confundidas com requisitos publicados.


---

# 11. Matriz de rastreabilidade específica do G2 — Vadrevu -> V1 -> Uno

| Elemento | Evidência no trabalho-base / literatura de apoio | V1 ESP32 | Port Uno/Nano atual | Classificação |
|---|---|---|---|---|
| decisão hierárquica | descrita no trabalho-base | sim | G1 antes de G2; G2 ainda diagnóstico | **ADAPTADO** |
| amplitude | feature explicitamente descrita | AC RMS | `absoluteAmplitude` + AC RMS auxiliar | **REPLICADO/ADAPTADO** |
| threshold crossing rate | feature explicitamente descrita | crossing count | count + taxa em permille | **REPLICADO** |
| autocorrelação | feature explicitamente descrita | ACF normalizada | ACF normalizada sob demanda | **REPLICADO** |
| primeiro zero-crossing da ACF | descrito em fontes secundárias do método; existe na V1 | FZCP | FZCP | **REPLICADO COM RASTREABILIDADE SECUNDÁRIA** |
| pico máximo da ACF | descrito em fontes secundárias; existe na V1 | peak correlation | `ACFpeak_permille` | **REPLICADO** |
| lag do pico | descrito em fontes secundárias; existe na V1 | best lag | `peakLag` | **REPLICADO** |
| período derivado | consequência matemática do lag | BPM/período | `period_cpm` diagnóstico | **DERIVADO** |
| Butterworth da V1 | presente na implementação histórica | sim | não nesta versão | **ADIADO** |
| Hamming da V1 | presente na implementação histórica da ACF | sim | não nesta versão | **ADIADO** |
| Packed18 | não pertence ao método-base | não | sim | **ADAPTAÇÃO DE HARDWARE** |
| RED + IR separados | não assumido como requisito universal do método-base | disponível no projeto | ambos processados | **ADAPTAÇÃO PARA OXIMETRIA** |
| thresholds finais | dependem do método/dados | configuráveis | ainda inexistentes | **A CARACTERIZAR** |

A regra de escrita acadêmica é citar a origem de cada elemento e não atribuir ao artigo escolhas que pertencem ao port para ATmega328P.
