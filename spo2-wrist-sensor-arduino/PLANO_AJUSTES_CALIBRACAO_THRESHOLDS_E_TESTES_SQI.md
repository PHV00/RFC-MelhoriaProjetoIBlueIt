# Plano de Ajustes, Calibração de Thresholds e Testes da Pipeline SQI no Arduino Uno/Nano

## 1. Objetivo

Este documento registra a comparação entre duas rotas possíveis para a camada de qualidade PPG no ATmega328P, o estado atual da implementação, as variáveis que ainda precisam ser calibradas e os testes necessários com e sem dados reais de bancada.

O objetivo é evitar dois erros metodológicos:

- copiar thresholds publicados para sensor, resolução, taxa de amostragem e população diferentes;
- confundir decisões de engenharia do projeto com regras propostas pela literatura.

A pipeline continua tendo como objetivo rejeitar janelas de PPG de qualidade insuficiente antes do cálculo de FC/SpO2 e antes da integração com PITACO/I Blue It.

---

# 2. Estudos-base

## 2.1 Reddy et al. (2020)

Gangireddy Narendra Kumar Reddy, M. Sabarimalai Manikandan e N. V. L. Narasimha Murty.

On-Device Integrated PPG Quality Assessment and Sensor Disconnection/Saturation Detection System for IoT Health Monitoring

DOI: 10.1109/TIM.2020.2971132

Pontos usados neste projeto:

- janela de 5 s;
- Rule-01: nearly-zero amplitude;
- Rule-02: saturation;
- primeira diferença d[n] = s[n] - s[n-1];
- normalização do sinal diferenciado;
- adição de ruído controlado;
- first-order predictor coefficient, FOPC;
- regras hierárquicas para noise-free PPG, motion artifact e pulse-free noise;
- implementação embarcada;
- versão FOPC-DS sem HPF, beat detection, template matching e DTW.

## 2.2 Vadrevu e Manikandan (2019)

Real-Time PPG Signal Quality Assessment System for Improving Battery Life and False Alarms

DOI: 10.1109/TCSII.2019.2891636

Pontos usados para comparação:

- janela de 5 s;
- HPF Butterworth de 3ª ordem, 0,5 Hz;
- maximum absolute amplitude;
- local amplitude maxima em frames de 100 ms;
- number of threshold crossings;
- ACF com janela de Hamming;
- FZCP;
- pico máximo da ACF, Rmax;
- lag do pico, Kmax;
- NTC sobre o sinal diferenciado;
- seis regras hierárquicas.

## 2.3 Sukor, Redmond e Lovell (2011)

Signal quality measures for pulse oximetry through waveform morphology analysis

DOI: 10.1088/0967-3334/32/3/008

Pontos previstos para o futuro G3:

- filtragem PPG 0,5–5 Hz;
- segmentação em pulsos;
- pulse amplitude;
- trough depth difference;
- pulse width;
- distância Euclidiana em relação a um pulso médio;
- amplitude ratios.

## 2.4 Fischer et al.

An Algorithm for Real-Time Pulse Waveform Segmentation and Artifact Detection in Photoplethysmograms

DOI: 10.1109/JBHI.2016.2518202

Pontos usados:

- clipping deve ser verificado no RAW antes da filtragem;
- análise embarcada;
- rise time;
- pulse-wave duration;
- pulse-wave amplitude;
- comparações relativas beat-to-beat.

## 2.5 Orphanidou et al.

Signal-Quality Indices for the Electrocardiogram and Photoplethysmogram: Derivation and Applications to Wireless Monitoring

DOI: 10.1109/JBHI.2014.2338351

Alternativa/complemento:

- regras de viabilidade fisiológica;
- gap máximo entre pulsos;
- razão máximo/mínimo intervalo;
- template matching.

## 2.6 Karlen et al. (2012)

Photoplethysmogram signal quality estimation using repeated Gaussian filters and cross-correlation

DOI: 10.1088/0967-3334/33/10/1617

Alternativa/complemento para G3:

- segmentação;
- derivada;
- repeated Gaussian filters;
- cross-correlation entre pulsos consecutivos;
- SQI por similaridade morfológica.

## 2.7 Elgendi (2016)

Optimal Signal Quality Index for Photoplethysmogram Signals

DOI: 10.3390/bioengineering3040021

Referência comparativa:

- perfusion SQI;
- kurtosis;
- skewness;
- relative power;
- non-stationarity;
- zero crossing;
- entropy;
- matching de detectores.

Esses SQIs são alternativas. Eles não precisam ser todos implementados.

---

# 3. Estado atual da implementação no Uno/Nano

## 3.1 Aquisição MAX30102 — IMPLEMENTADA

Configuração atual:

~~~text
sampleRate = 100 Hz
sampleAverage = 4
pulseWidth = 411 us
ADC = 18 bits
RED + IR
~~~

Taxa efetiva observada:

~~~text
~25 amostras/s
~125 amostras em 5 s
~~~

Estado: validada funcionalmente em bancada. Isso não equivale a validação clínica ou populacional.

## 3.2 Packed18 — IMPLEMENTADO E VALIDADO

Cada amostra de 18 bits é armazenada em 3 bytes:

~~~text
byte 0 -> bits 0..7
byte 1 -> bits 8..15
byte 2 -> bits 16..17
~~~

Propriedade:

~~~text
decode(encode(x)) = x
~~~

Resultados obtidos:

~~~text
mismatch = 0
RED > 65535 = 100/100 em janelas estáveis
IR  > 65535 = 100/100 em janelas estáveis
~~~

Para 100 amostras por canal:

~~~text
uint32_t: 800 B
Packed18: 600 B
economia: 200 B
~~~

Se a janela for unificada em 5 s na taxa atual:

~~~text
125 RED + 125 IR
= 125 * 3 * 2
= 750 B
~~~

O aumento seria de 150 B. Antes de adotá-lo deve-se medir freeRAM.

---

# 4. Rota A — G1 Reddy/Fischer + G2 Vadrevu completo

Fluxo:

~~~text
MAX30102
   |
   v
G1 integridade
   |
   v
HPF 0,5 Hz
   |
   v
Maximum Absolute Amplitude
   |
   v
Local Amplitude Maxima / 100 ms
   |
   v
Threshold Crossings
   |
   v
Hamming + ACF
   |
   +--> FZCP
   +--> Rmax
   +--> Kmax
   |
   v
dPPG + NTC
   |
   v
Acceptable / Unacceptable
~~~

## Vantagens

- forte fidelidade a um algoritmo completo publicado;
- regras rastreáveis ao artigo;
- features interpretáveis;
- ACF fornece periodicidade explícita;
- implementação embarcada demonstrada pelos autores.

## Implicação 1 — resolução temporal

O trabalho original usa 125 Hz.

Em 100 ms:

~~~text
125 Hz -> 12,5 amostras
25 Hz  -> 2,5 amostras
~~~

Isso torna a regra de máximos locais muito mais grosseira.

O artigo também usa FZCP com limite temporal iniciando em aproximadamente 0,05 s.

Na taxa atual:

~~~text
0,05 s * 25 Hz = 1,25 amostras
~~~

Esse limite não é representável adequadamente.

## Implicação 2 — memória se aumentarmos Fs

Em 100 Hz efetivos por 5 s:

~~~text
500 amostras/canal
500 * 3 * 2 = 3000 B
~~~

Isso excede os 2 kB de SRAM do ATmega328P.

Em 50 Hz:

~~~text
250 * 3 * 2 = 1500 B
~~~

Restariam aproximadamente 548 B antes das demais variáveis e do PITACO.

## Implicação 3 — CPU

A ACF exige múltiplos lags, com custo aproximadamente quadrático na janela.

Além disso, seriam necessários:

- HPF;
- Hamming;
- ACF;
- dPPG;
- dois conjuntos de crossings.

## Conclusão da Rota A

É cientificamente forte, mas não encaixa naturalmente na configuração atual de 25 Hz, 2 kB SRAM e futura execução compartilhada com PITACO.

Ela continua apropriada se o objetivo principal for reproduzir Vadrevu com maior fidelidade ou se houver aumento de recursos de hardware.

---

# 5. Rota B — Reddy Rules 1–2 + FOPC-DS

Fluxo:

~~~text
MAX30102
   |
   v
Rule 01
Nearly-Zero Amplitude
   |
   v
Rule 02
Saturation
   |
   v
d[n] = s[n] - s[n-1]
   |
   v
normalização de d[n]
   |
   v
ruído controlado
   |
   v
FOPC
   |
   +--> noise-free
   +--> motion artifact
   +--> pulse-free noise
~~~

## Vantagens

Na versão FOPC-DS o estudo evita:

- high-pass filter;
- beat detection;
- template matching;
- DTW.

Para ordem P=1, o predictor coefficient pode ser implementado com poucos acumuladores.

Uma adaptação adequada ao Uno pode usar duas passagens sobre Packed18:

~~~text
PASSAGEM 1
decode
-> d[n]
-> estimar escala necessária

PASSAGEM 2
decode
-> d[n]
-> normalizar
-> ruído determinístico
-> acumular estatísticas do FOPC
~~~

Não é necessário criar arrays float completos.

## Implicação de memória

Com 125 amostras por canal:

~~~text
Packed18 dual-channel = 750 B
workspace FOPC = acumuladores + estado
~~~

Isso é mais compatível com o ATmega328P.

## Implicação de CPU

O custo pode permanecer aproximadamente linear com N, muito mais favorável à futura execução compartilhada com PITACO.

## Limitação científica

Os thresholds publicados:

~~~text
alpha > 0,93
-0,5 < alpha < 0,93
alpha < -0,5
~~~

não devem ser copiados diretamente.

O nosso:

- Fs é diferente;
- ADC é diferente;
- sensor é diferente;
- sample averaging é diferente;
- montagem é diferente.

O predictor coefficient depende fortemente da relação temporal entre amostras. Portanto os thresholds de alpha precisam ser recalibrados.

## Ruído controlado

O artigo usa amplitude relativa de 0,10 após normalização.

No port devem ser caracterizados pelo menos:

~~~text
a = 0,05
a = 0,10
a = 0,20
~~~

Durante calibração a sequência de ruído deve ser determinística para que o mesmo experimento possa ser repetido.

---

# 6. Comparação resumida

| Critério | Rota A — Vadrevu | Rota B — Reddy FOPC-DS |
|---|---|---|
| fidelidade a algoritmo completo | alta | alta |
| número de features | maior | menor |
| HPF | sim | não na versão FOPC-DS |
| ACF multi-lag | sim | não |
| Hamming | sim | não |
| custo CPU esperado | maior | menor |
| workspace adicional | maior | pequeno |
| adequação natural a 25 Hz | baixa para algumas regras temporais | possível, mas exige recalibrar alpha |
| compatibilidade com 2 kB SRAM | difícil se Fs subir | melhor |
| futura convivência com PITACO | mais arriscada | mais favorável |
| periodicidade explícita | sim | não diretamente |
| classificação MA/PF/NF | sim | sim |
| thresholds copiáveis diretamente | não | não |

## Decisão provisória recomendada

Para o hardware atual:

~~~text
ATmega328P
2 kB SRAM
16 MHz
MAX30102
futura execução com PITACO
~~~

a Rota B é a candidata preferencial.

Vadrevu permanece como alternativa, benchmark e referência importante de SQA hierárquico.

---

# 7. Classes de parâmetros

## Classe H — hardware

Exemplo:

~~~text
ADC_MAX = 2^18 - 1 = 262143
~~~

Não precisa ser aprendido experimentalmente.

## Classe L — literatura, dependente do estudo

Exemplos:

~~~text
Reddy alpha = 0,93 / -0,5
Reddy NZA = 10 em 10 bits
Vadrevu lambda_MAA = 5 em 10 bits
Vadrevu FZCP / Rmax / Kmax
~~~

São referências iniciais, não thresholds finais do MAX30102.

## Classe P — fisiológico/temporal

Exemplos:

~~~text
pulse period
pulse width
rise time
HR plausível
~~~

Devem considerar população, tarefa e resolução temporal real.

## Classe E — engenharia do projeto

Exemplos:

~~~text
G1_MIN_MEAN_SIGNAL = 5000
G1_MIN_DYNAMIC_RANGE = 500
G1_SATURATION_MARGIN = 2048
Packed18
tolerância RED/IR futura
~~~

Precisam de caracterização própria.

---

# 8. Gate 01 — Integridade RAW

## Fundamentação

Reddy 2020:

- nearly-zero amplitude;
- saturation;
- decisões baratas antes de processamento posterior.

Fischer:

- clipping no RAW antes da filtragem.

## Implementado

~~~text
mean RED/IR
range RED/IR
upper saturation RED/IR
~~~

Thresholds atuais:

~~~text
G1_MIN_MEAN_SIGNAL = 5000
G1_MIN_DYNAMIC_RANGE = 500
G1_SATURATION_MARGIN = 2048
G1_MAX_SATURATED_SAMPLES = 0
~~~

## O que calibrar e por quê

### Lower saturation

Falta detectar clipping próximo do limite inferior.

Não deve ser confundido com o baseline sem dedo observado na montagem.

### Nearly-zero

Comparar experimentalmente:

~~~text
mean < threshold
max amplitude < threshold
ambos combinados
~~~

Manter a alternativa que melhor separar ausência de contato de contato real.

### Dynamic range

Range < 500 é uma regra nossa.

Só deve permanecer se acrescentar poder discriminativo além de nearly-zero e saturation.

## Testes sem dados de bancada

Usar vetores sintéticos:

~~~text
zeros
constante baixa
constante média
constante próxima ao ADC_MAX
clipping inferior
clipping superior
rampa
onda periódica
~~~

Verificar fronteiras, overflow e motivo correto de falha.

## Testes com dados de bancada

Condições:

~~~text
sem dedo
dedo estável
contato parcial
pressão baixa
pressão elevada
colocação/remoção
movimento aleatório
movimento periódico
~~~

Registrar:

~~~text
min
max
mean
range
lower_sat_count
upper_sat_count
label experimental
~~~

Calibrar:

~~~text
MIN_MEAN_SIGNAL
MIN_DYNAMIC_RANGE
LOW_SAT_LEVEL
HIGH_SAT_LEVEL
MAX_SATURATED_SAMPLES
~~~

Uma variável só deve ser mantida se separar casos que as demais não separam.

---

# 9. Gate 02 recomendado — Reddy FOPC-DS

## Fundamentação

Reddy 2020.

## Fórmulas

Primeira diferença:

~~~text
d[n] = s[n] - s[n-1]
~~~

Normalização:

~~~text
y[n] = d[n] / escala(d)
~~~

Ruído:

~~~text
z[n] = y[n] + a*w[n]
~~~

Predição de primeira ordem:

~~~text
x_hat[n] = alpha * x[n-1]
~~~

O artigo calcula os predictor coefficients por linear prediction e Levinson-Durbin.

Para ordem 1, o port pode usar uma forma matematicamente equivalente baseada nas autocorrelações de lag 0 e lag 1, desde que seja comparada contra uma implementação de referência antes do uso.

## Variáveis a calibrar

### Sample rate

Atual:

~~~text
~25 Hz
~~~

Primeiro testar se o FOPC separa NF, MA e PF nessa taxa.

Só depois avaliar sampleAverage 2 ou 1.

### Amplitude de ruído a

Testar:

~~~text
0,05
0,10
0,20
~~~

Mantendo 0,10 como referência do estudo.

### Thresholds de alpha

Referência:

~~~text
alpha > 0,93
-0,5 < alpha < 0,93
alpha < -0,5
~~~

Não congelar esses números no port.

### Janela

Usar inicialmente 5 s para alinhar G1 e G2.

Na taxa atual:

~~~text
125 amostras/canal
750 B dual-channel
~~~

Medir freeRAM.

### Canal de SQI

Comparar:

~~~text
IR apenas
RED apenas
RED e IR
~~~

Se IR sozinho fornecer classificação equivalente, pode reduzir CPU do G2 enquanto RED continua preservado para oximetria.

## Testes sem dados de bancada

Vetores artificiais:

~~~text
senoide limpa
trem de pulsos
ruído branco
constante
pulsos + ruído
pulsos + spikes
degrau
movimento periódico sintético
~~~

Validar:

- d[n];
- normalização;
- ruído determinístico;
- alpha;
- Arduino versus referência Python/PC;
- overflow;
- tempo;
- memória.

Critério matemático:

~~~text
alpha_Arduino ~= alpha_referencia
~~~

dentro de erro numérico definido antes do teste.

## Testes com dados de bancada

Classes controladas:

~~~text
NF = dedo estável
PF = sem dedo / sem contato
MA = movimento com dedo presente
~~~

Subtipos MA:

~~~text
aleatório
periódico
pressão
deslocamento lateral
colocação/remoção
~~~

Registrar:

~~~text
alpha_RED
alpha_IR
label
sample_count
freeRAM
G2_ms
~~~

Depois obter:

~~~text
mediana
mínimo
máximo
percentis
overlap entre classes
~~~

Somente então congelar thresholds.

---

# 10. Gate 02 alternativo — Vadrevu

Se a Rota A for retomada, devem ser implementados todos os elementos antes de alegar reprodução:

~~~text
HPF 0,5 Hz
Maximum Absolute Amplitude
Local Amplitude Maxima / 100 ms
NTC com threshold
Hamming
ACF da Eq. 7
FZCP
Rmax
Kmax
dPPG
NTC do dPPG
seis regras hierárquicas
~~~

Parâmetros a recalibrar:

~~~text
lambda_MAA
lambda_NTC1
FZCP limits
Rmax
Kmax
lambda_NTC2
~~~

Essa rota não deve ser validada na configuração atual de 25 Hz sem resolver primeiro a resolução temporal.

---

# 11. Gate 03 — Morfologia

## Fundamentação

Principal:

- Sukor 2011.

Complementares:

- Fischer;
- Orphanidou;
- Karlen.

## Ainda não implementado

Sukor sugere:

~~~text
pulse amplitude
trough depth difference
pulse width
Euclidean distance para pulso médio
amplitude ratios
~~~

Fischer acrescenta:

~~~text
rise time
pulse-wave duration
pulse-wave amplitude
mudanças beat-to-beat
~~~

## Principal variável a validar antes do G3

Sampling rate.

Na taxa atual:

~~~text
25 Hz
1 amostra = 40 ms
~~~

Um rise time de 80 ms ocupa somente duas amostras.

Antes de implementar G3 deve-se provar que a taxa atual localiza trough, peak, width e rise time com estabilidade suficiente.

## Testes sem bancada

Ondas sintéticas com:

~~~text
amplitude variável
width variável
rise time variável
dicrotic notch presente/ausente
spikes
pulsos deformados
~~~

## Testes com bancada

Comparar:

~~~text
dedo estável
pressão variável
movimento
contato instável
~~~

Nenhum threshold morfológico deve ser congelado antes disso.

---

# 12. Gate 04 — Coerência RED/IR

## Fundamentação atual

A oximetria depende dos dois comprimentos de onda e movimento pode alterar as componentes AC de RED e IR.

Entretanto, os estudos selecionados não fornecem um Gate RED/IR equivalente ao nosso nem threshold universal de coerência.

Portanto G4 continua sendo adaptação de projeto.

## Variáveis candidatas

~~~text
delta_period = |T_RED - T_IR|
delta_lag = |K_RED - K_IR|
correlação RED/IR
estabilidade relativa de amplitude
~~~

Nenhuma está congelada.

## O que falta

Pesquisa específica por:

~~~text
inter-channel PPG quality
red infrared signal quality pulse oximetry
RED IR coherence PPG
~~~

Depois decidir se G4 permanece Gate próprio ou vira condição interna do cálculo de SpO2.

---

# 13. Protocolo geral de calibração

## Etapa 1 — sem bancada

Objetivo: provar que o código calcula corretamente as fórmulas.

Executar:

~~~text
test vectors
Arduino x referência Python/PC
boundary tests
overflow
memory
timing
~~~

Isso não prova qualidade fisiológica.

## Etapa 2 — caracterização controlada de bancada

Cada janela deve registrar:

~~~text
índice/timestamp
condição experimental
features
decisão dos Gates
sample count
freeRAM
processing time
~~~

Não ajustar threshold olhando uma única janela.

## Etapa 3 — calibração e validação separadas

Preferir:

~~~text
sessão A -> calibração
sessão B -> validação
~~~

Posteriormente, participantes diferentes serão necessários para qualquer alegação de generalização populacional.

## Etapa 4 — análise de erro

Registrar:

~~~text
TP
TN
FP
FN
sensibilidade
especificidade
accuracy
~~~

A definição de classe positiva deve ser fixa e documentada.

---

# 14. Ordem recomendada

~~~text
1. Não calibrar o G2 Vadrevu atual.

2. Corrigir G1:
   lower saturation
   revisar nearly-zero
   confirmar necessidade do range

3. Avaliar Packed18 de 125 amostras e medir SRAM.

4. Implementar G2 Reddy FOPC-DS em modo diagnóstico.

5. Validar Arduino x referência.

6. Coletar NF, MA e PF em bancada.

7. Calibrar:
   amplitude de ruído
   alpha thresholds
   canal usado

8. Habilitar G2 PASS/FAIL.

9. Reavaliar sampling rate antes do G3.

10. Implementar G3.

11. Pesquisar fundamento específico do G4.

12. Integrar FC/SpO2.

13. Integrar PITACO/I Blue It.
~~~

---

# 15. Critério para congelar um Gate

Um Gate só deve ser marcado como baseline congelada quando houver:

~~~text
A. origem científica documentada
B. fórmula e código conferidos
C. custo de CPU/SRAM medido
D. comportamento experimental caracterizado
E. thresholds validados em dados separados dos usados para calibração
~~~

Antes disso, usar:

~~~text
PROVISÓRIO
DIAGNÓSTICO
CARACTERIZAÇÃO DE BANCADA
~~~

e não:

~~~text
CLÍNICO
UNIVERSAL
VALIDADO PARA POPULAÇÃO
~~~

---

# 16. Decisão provisória registrada

~~~text
G1:
Reddy + Fischer
manter, com correções

G2:
priorizar Reddy FOPC-DS
substituindo o diagnóstico Vadrevu incompleto atual

Vadrevu:
manter como alternativa e benchmark

G3:
Sukor + Fischer
implementar após decisão sobre Fs

G4:
adaptação do projeto
ainda requer base científica específica

Packed18:
manter
decisão de engenharia validada
~~~

Essa decisão deve ser reavaliada se o microcontrolador, a taxa de amostragem, a arquitetura de memória ou os resultados de bancada mudarem.
