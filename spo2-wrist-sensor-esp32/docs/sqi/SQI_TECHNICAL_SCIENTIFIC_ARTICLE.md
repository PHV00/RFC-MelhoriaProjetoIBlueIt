# Arquitetura Hierárquica de Qualidade de Sinal Fotopletismográfico para Oximetria Embarcada no I Blue It
## Fundamentação teórica, metodologia de gates, parametrização, calibração e correspondência com o firmware ESP32/MAX30102

**Projeto:** RFC-MelhoriaProjetoIBlueIt  
**Repositório de referência:** `PHV00/RFC-MelhoriaProjetoIBlueIt`  
**Módulo:** `spo2-wrist-sensor`  
**Sensor-alvo:** MAX30102 / família MAX3010x  
**Perfil experimental inicial:** `NO_GRIP` — aquisição sem pegador anatômico  
**Status arquitetural:** G1 implementado; G2–G4 planejados/scaffold; migração de score agregado para SQI hierárquico em andamento.

---

## Resumo

A integração de oximetria de pulso ao I Blue It introduz uma nova modalidade fisiológica involuntária cujo dado pode, futuramente, alimentar mecanismos de segurança, monitoramento e adaptação do exergame. Entretanto, valores de frequência cardíaca e saturação periférica de oxigênio (`SpO₂`) somente são úteis se o sinal fotopletismográfico que os origina possuir qualidade suficiente. O problema central deste módulo não é reconstruir um PPG degradado, mas decidir, de modo interpretável e computacionalmente viável em um ESP32, se uma janela de sinais RED/IR é utilizável para processamento fisiológico posterior.

O firmware adota uma arquitetura de Signal Quality Index (SQI) **rule-based, multimétrica, hierárquica e fail-fast**. A janela bruta de 5 s é preservada e submetida sequencialmente a quatro gates: **G1 — Integridade RAW**, **G2 — Pulsatilidade**, **G3 — Morfologia** e **G4 — Coerência RED↔IR**. Cada gate responde a uma hipótese distinta sobre o sinal e pode invalidar imediatamente a janela. Somente uma janela aprovada por todos os gates obrigatórios deve chegar aos estimadores de frequência cardíaca e SpO₂.

A organização deriva das necessidades do RFC, da arquitetura multimodal 123-SGR e da literatura de avaliação de qualidade de PPG. Reddy et al. sustentam a rejeição precoce de desconexão/saturação e a execução embarcada; Vadrevu e Manikandan fundamentam a decisão hierárquica baseada em amplitude, threshold crossings e autocorrelação; Sukor et al. e Fischer et al. sustentam a avaliação morfológica; Orphanidou et al. e Karlen et al. fornecem critérios complementares de plausibilidade e consistência batimento-a-batimento; Elgendi e Desquins et al. contextualizam diferentes famílias de SQI e a necessidade de não tratar uma única métrica como árbitro universal.

O presente documento descreve a lógica matemática e computacional dos gates, seus parâmetros configuráveis, os pontos em que calibração experimental é necessária, um protocolo de obtenção dos thresholds para o perfil `NO_GRIP`, o mapeamento entre teoria e código e os limites do sistema atual.

**Palavras-chave:** fotopletismografia; PPG; SQI; oximetria de pulso; MAX30102; qualidade de sinal; ESP32; jogos sérios; I Blue It; reabilitação respiratória.

---

# 1. Introdução

O I Blue It foi concebido como jogo sério para auxiliar a reabilitação respiratória, utilizando ações respiratórias como forma de interação e permitindo configuração terapêutica e registro dos resultados. Desde a concepção original, o sistema é orientado a um contexto acompanhado por profissional, com restrições explícitas de não fadigar o paciente e não exigir esforços inadequados.

A evolução multimodal do sistema amplia esse desenho. Na arquitetura 123-SGR, modalidades podem representar ações conscientes do paciente ou sinais involuntários associados ao seu estado físico. O oxímetro aparece explicitamente como dispositivo do fluxo inconsciente e como possível provedor de segurança. Na evolução posterior do I Blue It, Dias incorpora sinais fisiológicos ao conceito de Flow Psicofisiológico e ao DeepDDA, em que o estado do paciente pode influenciar ajustes de dificuldade.

Contudo, existe uma dependência lógica anterior a qualquer decisão terapêutica:

```text
"Existe um número de SpO₂?" não basta.

É necessário perguntar:

"O PPG que gerou esse número possui qualidade suficiente
 para que esse número possa sequer ser considerado?"
```

Esse é o papel do SQI.

O SQI deste projeto não é um algoritmo de restauração de sinal. Formalmente:

```text
SQI(PPG_RED, PPG_IR, tempo, configuração)
             |
             v
     VALID / INVALID / WAITING
```

e não:

```text
PPG degradado --> SQI --> PPG corrigido
```

Essa distinção é fundamental para evitar que filtragem, suavização ou interpolação ocultem evidências de problemas físicos na aquisição.

---

# 2. Questão de engenharia e questão científica

A questão de engenharia é:

> Como implementar no ESP32 um mecanismo de decisão que impeça sinais PPG inadequados de alimentar HR/SpO₂ e, posteriormente, o I Blue It?

A questão científica associada é:

> Quais propriedades observáveis do PPG são suficientemente informativas, explicáveis e computacionalmente acessíveis para classificar janelas como utilizáveis ou não, e como seus thresholds devem ser determinados para o conjunto real MAX30102 + dedo + geometria de montagem do projeto?

A literatura não fornece um único threshold universal. Ela fornece principalmente famílias de features, ordem de avaliação, estratégias de decisão e métodos de validação. Portanto:

```text
LITERATURA
   |
   | define "o que observar"
   v
FEATURES / ESTRUTURA
   |
   | hardware define limites físicos
   v
VALORES INICIAIS DE ENGENHARIA
   |
   | dados reais do projeto
   v
CALIBRAÇÃO NO_GRIP
   |
   | hold-out sem retuning
   v
THRESHOLDS CONGELADOS
```

---

# 3. Contexto dentro da arquitetura I Blue It / 123-SGR

A arquitetura 123-SGR separa o fluxo consciente do fluxo inconsciente. O oxímetro pertence ao fluxo fisiológico involuntário, associado à segurança.

```text
 PACIENTE
    |
    | perfusão arterial modifica absorção óptica
    v
 MAX30102
    |
    | RED + IR
    v
 AQUISIÇÃO / FIFO / SampleBuffer
    |
    v
 +--------------------------------------+
 |       TRATAMENTO DE SINAIS / SQI     |
 |                                      |
 |  G1 -> G2 -> G3 -> G4                |
 |                                      |
 +--------------------------------------+
    |
    | somente dado aprovado
    v
 HR / SpO2
    |
    v
 confidence_engine
    |
    v
 TELEMETRIA
    |
    v
 I BLUE IT / camada fisiológica futura
    |
    +--> monitoramento
    +--> dashboard
    +--> regras terapêuticas
    +--> DeepDDA / Flow Psicofisiológico
```

Na terminologia da 123-SGR, o SQI materializa uma parte concreta do **Tratamento de Sinais** antes que o biossinal possa participar da **Grade de Adaptação**.

---

# 4. Princípio físico mínimo: por que existem dois canais

Na oximetria de pulso, RED e IR medem variações ópticas relacionadas à variação pulsátil de volume sanguíneo. Cada canal contém, de forma simplificada:

\[
PPG(t) = DC(t) + AC(t)
\]

- `DC(t)`: componente de base.
- `AC(t)`: componente pulsátil.

A estimativa clássica de oximetria utiliza:

\[
R = \frac{AC_{RED}/DC_{RED}}{AC_{IR}/DC_{IR}}
\]

e:

\[
SpO_2 = f(R)
\]

Logo:

```text
RED utilizável
    E
IR utilizável
    E
ambos descrevem o mesmo fenômeno pulsátil
```

---

# 5. Arquitetura do SQI adotada pelo repositório

```text
┌──────────────────────────────────────────────────────────────┐
│                    SampleBuffer RAW                          │
│          RED[n], IR[n], timestamp[n], seq[n]                 │
└──────────────────────────────┬───────────────────────────────┘
                               │
                               │ snapshot único
                               ▼
┌──────────────────────────────────────────────────────────────┐
│ G0 CONCEITUAL — SNAPSHOT 5 s                                │
│ Não é gate formal no enum.                                  │
│ Preserva RAW e fornece uma única janela a todos os gates.    │
└──────────────────────────────┬───────────────────────────────┘
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│ G1 — INTEGRIDADE RAW                                        │
│ timestamps | nível óptico | flatline | clipping              │
└──────────────────────────────┬───────────────────────────────┘
                    FAIL ──────┤
                     │         │ PASS
                     ▼         ▼
                  INVALID   cópia processada
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│ G2 — PULSATILIDADE RED / IR                                 │
│ amplitude | crossings | ACF | período plausível              │
└──────────────────────────────┬───────────────────────────────┘
                    FAIL ──────┤
                     │         │ PASS
                     ▼         ▼
                  INVALID   Beat Detector
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│ G3 — MORFOLOGIA                                             │
│ width | rise | amplitude B2B | IBI | estabilidade/template   │
└──────────────────────────────┬───────────────────────────────┘
                    FAIL ──────┤
                     │         │ PASS
                     ▼         ▼
                  INVALID
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│ G4 — COERÊNCIA RED ↔ IR                                     │
│ período | contagem | pareamento | alinhamento | correlação   │
└──────────────────────────────┬───────────────────────────────┘
                    FAIL ──────┤
                     │         │ PASS
                     ▼         ▼
                  INVALID    VALID
                               │
                               ▼
                         HR / SpO₂
```

\[
VALID = G1 \land G2 \land G3 \land G4
\]

\[
FAIL(G_i) \Rightarrow INVALID
\]

---

# 6. Por que uma arquitetura hierárquica

## Interpretabilidade

```text
"Por que esta janela foi rejeitada?"

- descontinuidade;
- ausência de contato;
- flatline;
- clipping;
- ausência de pulsatilidade;
- morfologia anômala;
- RED e IR incoerentes.
```

## Custo computacional

```text
G1 barato
 |
 | se falhou, parar
 v
G2
 |
 | se falhou, parar
 v
Beat detector + G3
 |
 v
G4
```

## Segurança lógica

```text
score agregado:
boa métrica A + boa métrica B + clipping grave
= ainda pode gerar score intermediário

fail-fast:
clipping grave -> G1 FAIL -> INVALID
```

---

# 7. Janela temporal

O firmware atual usa:

```c
window_ms = 5000
step_ms   = 1000
Fs        = 100 Hz
```

\[
N = F_s \cdot T = 100 \cdot 5 = 500
\]

```text
0s                  5s
|-------------------|  janela 1

    1s                  6s
    |-------------------|  janela 2

        2s                  7s
        |-------------------|  janela 3
```

---

# 8. G0 conceitual — Snapshot

```text
SampleBuffer
    |
    v
copy_latest(...)
    |
    v
s_samples[500]
    |
    +-----------> G1 RAW
    |
    +-----------> preprocessamento em cópia -> G2/G3/G4
```

Invariante:

\[
RAW_{antes} = RAW_{depois}
\]

Nenhum gate sobrescreve RED, IR ou timestamps do snapshot.

---

# 9. G1 — Integridade RAW

## Hipótese

> **H1:** a janela possui integridade mínima de aquisição e contato óptico para justificar processamento fisiológico posterior.

Entrada:

```c
typedef struct {
    const ppg_sample_t *samples;
    size_t count;
    float expected_sample_rate_hz;
} sqi_window_t;
```

---

# 10. Fluxo lógico do G1

```text
              ┌────────────────────┐
              │  janela RAW 5 s    │
              │ RED + IR + tempo   │
              └─────────┬──────────┘
                        │
                        ▼
             ┌───────────────────────┐
             │ validar config/args   │
             └─────────┬─────────────┘
                       │
                       ▼
        ┌─────────────────────────────────┐
        │ varrer toda a janela uma vez    │
        │ RED: min max sum rail_count     │
        │ IR : min max sum rail_count     │
        │ T  : dt / gaps / duplicados     │
        └───────────────┬─────────────────┘
                        │
                        ▼
       ┌────────────────────────────────────┐
       │ mean / range / clipping / continuity│
       └────────────────┬───────────────────┘
                        │
                        ▼
      ┌──────────────────────────────────────┐
      │ aplicar TODAS as regras              │
      └─────────────────┬────────────────────┘
                        │
           ┌────────────┴────────────┐
           ▼                         ▼
       mask == 0                 mask != 0
           │                         │
           ▼                         ▼
        G1 PASS                    G1 FAIL
           │                         │
           ▼                         ▼
        G2 futuro               INVALID
```

---

# 11. Continuidade temporal do G1

\[
\Delta t_{esperado} = \frac{1000}{F_s}
\]

Para \(F_s=100 Hz\):

\[
\Delta t_{esperado}=10 ms
\]

Com tolerância relativa \(\tau\):

\[
\Delta t_{min} = \Delta t_{esperado}(1-\tau)
\]

\[
\Delta t_{max} = \Delta t_{esperado}(1+\tau)
\]

\[
continuity\_fraction =
\frac{N_{intervalos\ válidos}}{N-1}
\]

Regra:

```text
continuity_fraction < minimum_continuity_fraction
    => DISCONTINUITY
```

Pontos de ajuste:

```text
maximum_interval_deviation_fraction
minimum_continuity_fraction
```

---

# 12. Presença óptica no G1

\[
\mu = \frac{1}{N}\sum_{i=1}^{N}x_i
\]

```text
mean_RED < minimum_mean_level -> NO_SIGNAL_RED
mean_IR  < minimum_mean_level -> NO_SIGNAL_IR
```

A média RAW indica presença óptica grosseira, não qualidade fisiológica.

Ponto de ajuste:

```text
minimum_mean_level
```

Dependências:

```text
corrente LED
range ADC
posição do dedo
pressão
pele
luz ambiente
encapsulamento
geometria
```

---

# 13. Flatline no G1

\[
range = max(x)-min(x)
\]

```text
range_RED < minimum_raw_range -> FLATLINE_RED
range_IR  < minimum_raw_range -> FLATLINE_IR
```

Separação:

```text
range quase zero -> G1
pulsação pequena -> G2
```

---

# 14. Clipping / saturação no G1

\[
L_{low}=ADC_{min}+m
\]

\[
L_{high}=ADC_{max}-m
\]

Uma amostra está próxima ao rail se:

\[
x_i \le L_{low} \lor x_i \ge L_{high}
\]

\[
clip\_fraction=\frac{N_{near\_rail}}{N}
\]

```text
clip_fraction > maximum_clipping_fraction
    => CLIPPING
```

Visual:

```text
onda real:
      /\
     /  \
 ___/    \___

ADC saturado:
      ____
     |    |
 ___/      \___
```

Pontos de ajuste:

```text
rail_margin_counts
maximum_clipping_fraction
```

---

# 15. Máscara cumulativa do G1

```text
1   DISCONTINUITY
2   NO_SIGNAL_RED
4   NO_SIGNAL_IR
8   FLATLINE_RED
16  FLATLINE_IR
32  CLIPPING_RED
64  CLIPPING_IR
```

Exemplo:

```text
NO_SIGNAL_RED + NO_SIGNAL_IR = 2 + 4 = 6
```

`primary_reason` serve à telemetria; `failure_mask` preserva o diagnóstico completo.

---

# 16. Parâmetros atuais do G1

| Parâmetro | Valor atual | Origem | Status |
|---|---:|---|---|
| `adc_min_value` | 0 | hardware | estável enquanto ADC não mudar |
| `adc_max_value` | `0x03FFFF` | hardware/configuração | estável enquanto resolução não mudar |
| `rail_margin_counts` | 1 | engenharia | provisório |
| `minimum_mean_level` | 5000 | engenharia | provisório |
| `minimum_raw_range` | 20 | engenharia | provisório |
| `maximum_clipping_fraction` | 0,01 | engenharia | provisório |
| `minimum_continuity_fraction` | 0,95 | engenharia | provisório |
| `maximum_interval_deviation_fraction` | 0,40 | engenharia | provisório |

Objetivo:

```text
ENGINEERING_PROVISIONAL
        |
        v
EMPIRICALLY_CALIBRATED_NO_GRIP
```

---

# 17. Calibração científica do G1

Antes da coleta, congelar:

```text
sensor/placa
revisão
100 Hz
pulse width
ADC range
LED RED/IR
posição
óptica
firmware
perfil NO_GRIP
```

Classes sugeridas:

```text
NO_CONTACT
STABLE_CONTACT
LIGHT_CONTACT
PARTIAL_CONTACT
PRESSURE
AMBIENT_LIGHT
MOTION
TRANSITION_IN
TRANSITION_OUT
FLAT_FAULT
CLIP_LOW
CLIP_HIGH
TIMESTAMP_FAULT
```

Features exportadas:

```text
red_mean
ir_mean
red_min/max/range
ir_min/max/range
red_clip_fraction
ir_clip_fraction
continuity_fraction
discontinuity_count
duplicate_timestamp_count
```

---

# 18. Ajuste de `minimum_mean_level`

```text
NO_CONTACT -------------------- distribuição A
STABLE_CONTACT ---------------- distribuição B

               threshold ?
                   |
A: ========        |
B:          ====================
```

Método:

```text
distribuições
-> percentis
-> candidatos
-> ROC/PR ou restrição de falso aceite
-> development set
-> hold-out
```

Percentis ajudam, mas não são regra universal.

---

# 19. Ajuste de `minimum_raw_range`

Objetivo:

```text
detectar stuck/flatline
```

e não:

```text
rejeitar baixa pulsatilidade
```

Usar:

```text
fault injection + cauda inferior de janelas reais válidas
```

---

# 20. Ajuste de clipping

Sweep:

```text
rail_margin_counts:
1 2 4 8 16 32 64 128 ...

maximum_clipping_fraction:
0%
0,2%
0,5%
1%
2%
5%
10%
```

Selecionar limite que represente um orçamento de corrupção justificável antes de degradar features/estimadores posteriores.

---

# 21. G2 — Pulsatilidade

## Hipótese

> **H2:** a janela tecnicamente íntegra contém componente pulsátil periódica suficientemente forte e plausível em RED e IR.

```text
G1: "há aquisição íntegra?"
G2: "há fenômeno pulsátil plausível?"
```

Base principal:

```text
Vadrevu & Manikandan
Reddy et al.
Elgendi como apoio estatístico/perfusão
```

---

# 22. Pré-processamento do G2

```text
RAW preservado
   |
   +--------------------------+
   |                          |
   v                          v
evidência G1            cópia processada
                              |
                              v
                       detrending / HP
                              |
                              v
                    RED_ac(t), IR_ac(t)
```

Baseline inicial do projeto:

```text
remoção de baseline / HP ~0,5 Hz
```

a ser validada.

---

# 23. Detrending

Modelo simples:

\[
x(t)=at+b+s(t)
\]

\[
\hat{x}_{baseline}(t)=\hat a t+\hat b
\]

\[
x_{AC}(t)=x(t)-\hat{x}_{baseline}(t)
\]

Critérios para escolher preprocessamento:

```text
custo
latência
fase
efeito morfológico
reprodutibilidade
```

---

# 24. Amplitude pulsátil no G2

\[
AC_{RMS}=
\sqrt{\frac{1}{N}\sum_{i=1}^{N}x_{AC,i}^{2}}
\]

e/ou:

\[
range_{AC}=max(x_{AC})-min(x_{AC})
\]

Regra abstrata:

```text
AC_RED >= RED_min
AND
AC_IR  >= IR_min
```

Pontos de ajuste:

```text
minimum_ac_rms_red
minimum_ac_rms_ir
minimum_pulsatile_range_red
minimum_pulsatile_range_ir
```

---

# 25. Perfusion Index no G2

\[
PI=\frac{AC}{DC}
\]

\[
PI_\%=100\frac{AC}{DC}
\]

No projeto:

```text
PI = feature auxiliar
```

não:

```text
PI = árbitro universal
```

---

# 26. Threshold crossings

```text
            /\        /\
           /  \      /  \
----------/----\----/----\------ threshold
         /      \  /      \
```

Poucos crossings:

```text
ausência de oscilação
```

Excessivos:

```text
ruído/oscilação rápida
```

Parâmetros:

```text
minimum_threshold_crossings
maximum_threshold_crossings
```

---

# 27. Autocorrelação no G2

\[
R_{xx}(k)=\sum_n x[n]x[n-k]
\]

Para um sinal periódico:

```text
ACF
1.0 |\
    | \
    |  \        /\
    |   \      /  \
    |    \____/    \____
0.0 +----------------------> lag
               ^
               |
        período dominante
```

Features:

```text
acf_peak
acf_peak_lag
period_s
periodicity_score
```

---

# 28. Região fisiologicamente plausível de lag

\[
T_{max}=\frac{60}{HR_{min}}
\]

\[
T_{min}=\frac{60}{HR_{max}}
\]

\[
lag_{min}=F_sT_{min}
\]

\[
lag_{max}=F_sT_{max}
\]

A faixa do SQI é uma faixa de plausibilidade algorítmica, não um limite terapêutico.

---

# 29. Fluxo do G2

```text
                 G1 PASS
                    |
                    v
            preprocessamento
          RED_ac           IR_ac
             |               |
             v               v
        amplitude        amplitude
             |               |
        crossings        crossings
             |               |
           ACF             ACF
             |               |
          período          período
             |               |
             +-------+-------+
                     |
                     v
       ambos possuem pulsatilidade?
              /             \
            não             sim
             |               |
             v               v
         G2 FAIL         G2 PASS
```

Estrutura sugerida:

```c
typedef struct {
    float highpass_cutoff_hz;
    float minimum_ac_rms_red;
    float minimum_ac_rms_ir;
    float minimum_pulsatile_range_red;
    float minimum_pulsatile_range_ir;
    uint16_t minimum_threshold_crossings;
    uint16_t maximum_threshold_crossings;
    float minimum_acf_peak;
    float minimum_period_s;
    float maximum_period_s;
} g2_pulsatility_config_t;
```

---

# 30. Calibração do G2

```text
sinais sintéticos
-> verificar matemática
-> MAX30102 GOOD/BAD
-> extrair amplitude/crossings/ACF
-> ajustar development
-> validar hold-out
-> análise de sensibilidade
-> congelar NO_GRIP
```

Amplitude absoluta é fortemente local. Periodicidade e ACF podem receber apoio maior de datasets externos.

---

# 31. Beat Detector — infraestrutura

Não é gate.

```text
sinal contínuo
      |
      v
beat_1 beat_2 beat_3 ...
```

Estrutura conceitual:

```c
typedef struct {
    uint32_t start_index;
    uint32_t peak_index;
    uint32_t end_index;
    float amplitude;
    float width_ms;
    float rise_time_ms;
    float ibi_ms;
} ppg_beat_t;
```

---

# 32. G3 — Morfologia

## Hipótese

> **H3:** os pulsos detectados apresentam forma e estabilidade compatíveis com uma onda PPG utilizável.

Pulsatilidade não garante boa morfologia:

```text
esperado:
      /\       /\       /\
 ____/  \_____/  \_____/  \____

artefato periódico:
 __/\_/\/\__/\/\_/\/\__/\/\____
```

Base:

```text
Sukor
Fischer
Orphanidou
Karlen
```

---

# 33. Features de G3

Amplitude:

\[
A_i=x_{peak,i}-x_{baseline,i}
\]

Largura:

\[
W_i=t_{end,i}-t_{start,i}
\]

Rise time:

\[
RT_i=t_{peak,i}-t_{start,i}
\]

Intervalo:

\[
IBI_i=t_{peak,i}-t_{peak,i-1}
\]

Frequência equivalente:

\[
HR_i=\frac{60000}{IBI_i}
\]

---

# 34. Agregação robusta

\[
median(x)
\]

\[
MAD=median(|x_i-median(x)|)
\]

Opcionalmente:

\[
CV=\frac{\sigma}{\mu}
\]

Mediana/MAD reduzem o efeito de um único beat corrompido.

---

# 35. Similaridade beat-to-beat

Após normalização:

\[
C_i=corr(beat_i,template)
\]

```text
beat A:       /\__
             /   \___

beat B:      /\__
            /   \___

corr alta
```

versus:

```text
beat C:     /\/\____

corr baixa
```

A contribuição de Karlen entra como feature complementar do G3, não como novo gate obrigatório.

---

# 36. Fluxo do G3

```text
        G2 PASS
           |
           v
      Beat Detector
           |
           v
  ┌─────────────────┐
  │ beats detectados│
  └───────┬─────────┘
          |
          +--> quantidade suficiente?
          +--> width plausível?
          +--> rise time plausível?
          +--> amplitude estável?
          +--> IBI estável?
          +--> template similarity? [opcional]
          |
          v
    fração de beats bons
          |
      ┌───┴────┐
      │        │
   baixa      suficiente
      │        │
      v        v
   G3 FAIL   G3 PASS
```

Configuração sugerida:

```c
typedef struct {
    uint8_t minimum_valid_beats;
    float minimum_beat_width_ms;
    float maximum_beat_width_ms;
    float minimum_rise_time_ms;
    float maximum_rise_time_ms;
    float maximum_amplitude_cv;
    float maximum_width_cv;
    float maximum_ibi_cv;
    float maximum_outlier_fraction;
    bool use_template_similarity;
    float minimum_template_similarity;
} g3_morphology_config_t;
```

---

# 37. Metodologia de ajuste do G3

Parâmetros fisiologicamente informados:

```text
width
rise
IBI/HR plausível
```

Parâmetros dependentes do pipeline:

```text
CV
MAD
outlier fraction
template similarity
```

O segundo grupo requer calibração com o Beat Detector real.

---

# 38. G4 — Coerência RED ↔ IR

## Hipótese

> **H4:** RED e IR representam o mesmo fenômeno pulsátil durante a janela.

Exemplo incoerente:

```text
RED:
      /\        /\        /\
 ____/  \______/  \______/  \____

IR:
      /\             /\             /\
 ____/  \___________/  \___________/  \__
```

Cada canal isoladamente pode parecer periódico, mas não descreve o mesmo ciclo.

---

# 39. Features do G4

Períodos:

\[
T_{RED},T_{IR}
\]

Diferença relativa:

\[
D_T=
\frac{|T_{RED}-T_{IR}|}
{median(T_{RED},T_{IR})}
\]

Contagem:

\[
D_N=|N_{RED}-N_{IR}|
\]

Pareamento:

\[
|t_{RED,i}-t_{IR,j}|\le\Delta t_{match}
\]

\[
matched\_fraction=
\frac{N_{matched}}{\max(N_{RED},N_{IR})}
\]

Correlação auxiliar:

\[
\rho_{RED,IR}=corr(RED_{norm},IR_{norm})
\]

---

# 40. Fluxo do G4

```text
              G3 PASS
                 |
       ┌─────────┴─────────┐
       ▼                   ▼
   RED beats            IR beats
       |                   |
       +------ período -----+
       +------ count -------+
       +----- matching -----+
       +----- alignment ----+
       +-- correlação opc --+
                 |
                 v
       mesma série pulsátil?
          /             \
        não             sim
         |               |
         v               v
      G4 FAIL         G4 PASS
```

Configuração sugerida:

```c
typedef struct {
    float maximum_relative_period_difference;
    uint8_t maximum_beat_count_difference;
    float maximum_peak_alignment_ms;
    float minimum_matched_peak_fraction;
    bool use_correlation;
    float minimum_channel_correlation;
} g4_channels_config_t;
```

---

# 41. Decisão final

```c
if (!g1.passed) return reject(G1);
if (!g2.passed) return reject(G2);

detect_beats();

if (!g3.passed) return reject(G3);
if (!g4.passed) return reject(G4);

quality.state = PPG_QUALITY_VALID;
```

\[
Q=
\begin{cases}
1,&G_1=G_2=G_3=G_4=1\\
0,&caso\ contrário
\end{cases}
\]

---

# 42. Papel do score contínuo

Hoje há um score legado:

```text
quality_score =
      0.25 * PI_score
    + 0.25 * corr_score
    + 0.20 * SNR_score
    + 0.20 * continuity
    + 0.10 * clipping_score
```

e:

```text
minimum_quality_score = 0.55
```

Destino esperado:

```text
GATES -> decisão principal
SCORE -> diagnóstico/confiança
```

Nunca:

```text
gate obrigatório FAIL + score alto = VALID
```

---

# 43. SQI, estimadores e confidence

```text
                   ┌─────────────┐
                   │    SQI      │
                   │ G1 G2 G3 G4 │
                   └──────┬──────┘
                          |
                     VALID only
                          |
            ┌─────────────┴─────────────┐
            ▼                           ▼
       HR estimator                SpO2 estimator
            |                           |
            └─────────────┬─────────────┘
                          ▼
                  confidence_engine
                          |
                          ▼
                    health_frame_t
```

---

# 44. Semântica atual de VALID

Como somente G1 está implementado:

```text
PPG_QUALITY_VALID
```

significa hoje:

```text
passou todos os gates implementados
```

não:

```text
PPG definitivamente confiável para oximetria clínica
```

---

# 45. SpO₂ atual

A curva de engenharia atual é:

\[
SpO_2=110-25R
\]

com:

```text
calibrated = false
allow_uncalibrated_estimate = true
```

Logo:

```text
SQI válido != SpO2 clinicamente calibrada
```

São problemas distintos.

---

# 46. Rastreabilidade com o RFC

```text
RF04 — validar contato
   |
   +--> G1
   +--> G2

RF05 — validar qualidade
   |
   +--> G1 + G2 + G3 + G4

RF06 — ignorar leitura inválida
   |
   +--> fail-fast
   +--> estimadores só após VALID
   +--> confidence_engine
```

---

# 47. Metodologia geral de parametrização

```text
literatura
   |
   v
feature / intervalo candidato
   |
   v
dados development
   |
   v
θ1 θ2 θ3 ... θn
   |
   v
métricas
   |
   v
seleção
   |
   v
hold-out
   |
   v
θ*
```

Evitar:

```text
"funcionou no meu dedo" -> threshold definitivo
```

---

# 48. Split correto

Errado:

```text
sessão A:
window 1 -> treino
window 2 -> teste
window 3 -> treino
```

Correto:

```text
PARTICIPANTE / SESSÃO
        |
        +--> development
        OU
        +--> hold-out
```

Preferência: split por participante.

---

# 49. Métricas

\[
Sensitivity=\frac{TP}{TP+FN}
\]

\[
Specificity=\frac{TN}{TN+FP}
\]

\[
FAR=\frac{BAD\rightarrow ACCEPT}{BAD}
\]

\[
FRR=\frac{GOOD\rightarrow REJECT}{GOOD}
\]

\[
BalancedAccuracy=\frac{Sensitivity+Specificity}{2}
\]

```text
                         PREDIÇÃO
                  ACCEPT         REJECT
REAL GOOD           TP             FN
REAL BAD            FP             TN
                    ^
                    |
            sinal ruim aceito
```

No relatório final, a classe positiva deve ser explicitada.

---

# 50. Análise de sensibilidade

```text
θ baixo -------------------------------- θ alto
   |
   +--> aceita muito            rejeita muito
```

Procurar plateau:

```text
performance
   ^
   |           _________
   |          /         \
   |_________/           \____
              ^^^^^
              região robusta
```

---

# 51. Proveniência de parâmetros

Classes:

```text
HARDWARE
LITERATURE_BASELINE
ENGINEERING_PROVISIONAL
EMPIRICALLY_CALIBRATED
THERAPIST_CONFIGURED
DERIVED
```

Exemplo:

```yaml
name: g1.minimum_mean_level
value: 5000
unit: adc_counts
origin: ENGINEERING_PROVISIONAL
profile: NO_GRIP
status: not_frozen
```

Após validação:

```yaml
origin: EMPIRICALLY_CALIBRATED
dataset: no_grip_v1
validation_split: subject_holdout
status: frozen
```

---

# 52. Perfis NO_GRIP e WITH_GRIP

```text
SEM PEGADOR
   |
   v
medir variabilidade real
   |
   v
calibrar NO_GRIP
   |
   v
congelar perfil
```

Depois:

```text
COM PEGADOR
   |
   v
repetir protocolo
   |
   v
comparar distribuições
   |
   +--> manter thresholds?
   +--> criar WITH_GRIP?
```

Hipótese possível:

> O pegador anatômico reduz rejeições por contato, pulsatilidade, morfologia ou incoerência RED/IR sem elevar a falsa aceitação.

---

# 53. Relação literatura ↔ gates

```text
                 DESQUINS 2022
             survey / taxonomia geral
                       |
        +--------------+--------------+
        |              |              |
        v              v              v

  REDDY 2020      VADREVU 2019    ELGENDI 2016
     |                  |              |
     v                  v              |
    G1                 G2 <------------+
     |
     +------+
            |
            v
      FISCHER 2017
            |
            +-------------------+
                                |
                                v
                            G3 MORFOLOGIA
                              ^       ^
                              |       |
                         SUKOR 2011   ORPHANIDOU
                              \       /
                               \     /
                                v   v
                             KARLEN 2012
                         similaridade opcional

G4:
dual wavelength + eventos G2/G3 + correlação auxiliar
```

---

# 54. Papel específico das referências

| Trabalho | Papel no projeto | Não interpretar como |
|---|---|---|
| Reddy et al. | G1, desconexão/saturação, hierarquia embarcada | counts RAW universais |
| Vadrevu & Manikandan | G2, amplitude/crossings/ACF, 5 s | thresholds universais |
| Fischer et al. | clipping/artefato, segmentação/morfologia | algoritmo obrigatório integral |
| Sukor et al. | morfologia de pulse oximetry | classificador pronto |
| Karlen et al. | consistência B2B/template | novo gate obrigatório |
| Orphanidou et al. | plausibilidade/template/metodologia | limite clínico |
| Elgendi | comparação de SQIs estatísticos | PI como único árbitro |
| Desquins et al. | survey e taxonomia | algoritmo específico |
| MAX30102 | limites/configuração física | SQI universal |
| 123-SGR | posição do biossinal no sistema | algoritmo de SQI |
| Dias 2024 | uso fisiológico no Flow/DeepDDA | validação do sensor atual |

---

# 55. Correspondência com o firmware

```text
spo2-wrist-sensor/main/
│
├── app/
│   ├── app_controller.c
│   └── app_state_machine.c
│
├── common/
│   ├── measurement_types.h
│   └── sqi_types.h
│
├── drivers/
│   └── max3010x_driver.*
│
├── sensing/
│   ├── ppg_sampler.*
│   └── sample_buffer.*
│
├── processing/
│   ├── hr_estimator.*
│   ├── spo2_estimator.*
│   └── sqi/
│       ├── signal_quality.*
│       ├── preprocess/
│       ├── features/
│       └── gates/
│           ├── g1_integrity/
│           ├── g2_pulsatility/
│           ├── g3_morphology/
│           └── g4_channels/
│
├── safety/
│   └── confidence_engine.*
│
├── storage/
│   └── config_repo.*
│
└── transport/
    └── serial_telemetry.*
```

---

# 56. Fluxo de execução atual

```text
app_controller_step()
      |
      v
ppg_sampler_poll()
      |
      +--> overflow? -> LOW_CONFIDENCE
      |
      v
SampleBuffer
      |
      | a cada step_ms
      v
signal_quality_evaluate_window()
      |
      +--> WAITING
      +--> ERROR
      +--> COMPLETE
                |
                v
       quality.state ?
          /         \
     INVALID        VALID
        |             |
        |             +--> hr_estimator
        |             +--> spo2_estimator
        |                     |
        +----------------> confidence_engine
                              |
                              v
                         health_frame
                              |
                              v
                       serial telemetry
```

---

# 57. Estrutura futura de configuração

```c
typedef struct {
    uint32_t window_ms;
    uint32_t step_ms;

    g1_integrity_config_t   g1_integrity;
    g2_pulsatility_config_t g2_pulsatility;
    g3_morphology_config_t  g3_morphology;
    g4_channels_config_t    g4_channels;

    float minimum_quality_score; // legado temporário
} sqi_config_t;
```

---

# 58. Telemetria necessária para calibração

```text
window_id
timestamp_start/end

G1:
mean/range/clip/continuity/mask

G2:
AC RMS
crossings
ACF peak
period

G3:
beat count
width
rise
IBI
variabilidade
template similarity

G4:
period diff
count diff
matched fraction
alignment
correlation

FINAL:
state
failed_gate
fail_reason
```

---

# 59. Dataset científico

```csv
participant_id,
session_id,
profile_id,
window_id,
label,
sensor_config_version,
firmware_commit,
features...,
g1_pass,
g2_pass,
g3_pass,
g4_pass,
final_quality
```

Preservar o RAW integral em arquivo separado.

---

# 60. Unit test ≠ calibração

```text
UNIT TEST:
"o código aplicou corretamente threshold 0,95?"

CALIBRAÇÃO:
"0,95 é o threshold adequado ao sistema real?"
```

São evidências diferentes.

---

# 61. Estratégia de teste por gate

```text
G1:
flatline / rail / timestamps / gaps / nível baixo

G2:
senoides / PPG sintético / ruído / amplitude / frequências

G3:
width / rise / beat removido / outlier / template

G4:
RED=IR / delay / beat extra / período divergente
```

Camadas:

```text
unit tests
   ↓
sintético
   ↓
fault injection
   ↓
MAX30102 real
   ↓
hold-out
   ↓
comparação de SpO2 com referência
```

---

# 62. Ameaças à validade

- população pequena;
- dependência de hardware;
- janelas sobrepostas;
- rótulo GOOD/BAD subjetivo;
- ausência inicial de referência clínica;
- movimento real sub-representado;
- mudança futura de geometria com pegador.

---

# 63. Reprodutibilidade

Registrar:

```text
data
participante anônimo
sessão
hardware_revision
sensor_id
profile
firmware_commit
config_hash
sample_rate
LED_RED
LED_IR
ADC range
pulse width
label protocol
raw file
feature file
analysis script version
```

---

# 64. Exemplo de metadado de parâmetro

```yaml
parameter:
  id: g2.minimum_acf_peak
  gate: G2
  hypothesis: "sinal utilizável apresenta periodicidade detectável"
  feature: "maximum normalized ACF within physiological lag interval"
  unit: dimensionless
  origin: EMPIRICALLY_CALIBRATED
  literature:
    - Vadrevu_2019
  profile: NO_GRIP
  development_dataset: no_grip_dev_v2
  validation_dataset: no_grip_holdout_v2
  selected_value: 0.xx
  FAR: 0.xx
  FRR: 0.xx
  firmware_commit: "..."
```

---

# 65. Tabela final de calibração a preencher

| Gate | Parâmetro | Valor inicial | Fonte | Valor NO_GRIP | FAR | FRR | Status |
|---|---|---:|---|---:|---:|---:|---|
| G1 | `minimum_mean_level` | 5000 | engenharia | TBD | TBD | TBD | provisório |
| G1 | `minimum_raw_range` | 20 | engenharia | TBD | TBD | TBD | provisório |
| G1 | `max_clip_fraction` | 0,01 | engenharia | TBD | TBD | TBD | provisório |
| G2 | `minimum_ac_rms_red` | TBD | literatura+dados | TBD | TBD | TBD | pendente |
| G2 | `minimum_acf_peak` | TBD | Vadrevu | TBD | TBD | TBD | pendente |
| G3 | `minimum_valid_beats` | TBD | dados/fisiologia | TBD | TBD | TBD | pendente |
| G3 | `maximum_ibi_cv` | TBD | dados | TBD | TBD | TBD | pendente |
| G4 | `minimum_matched_peak_fraction` | TBD | dados | TBD | TBD | TBD | pendente |
| G4 | `max_alignment_ms` | TBD | dados/timing | TBD | TBD | TBD | pendente |

---

# 66. Quadro síntese

```text
╔══════════════════════════════════════════════════════════════════════╗
║ G1 — "A aquisição é tecnicamente íntegra?"                         ║
║ RAW, tempo, nível, flatline, clipping                              ║
║ Reddy / Fischer / MAX30102                                         ║
╠══════════════════════════════════════════════════════════════════════╣
║ G2 — "Há pulsatilidade plausível?"                                 ║
║ amplitude, crossings, ACF, período, PI auxiliar                    ║
║ Vadrevu / Reddy / Elgendi                                          ║
╠══════════════════════════════════════════════════════════════════════╣
║ G3 — "Os beats possuem morfologia estável?"                        ║
║ amplitude B2B, width, rise, IBI, template opcional                 ║
║ Sukor / Fischer / Orphanidou / Karlen                              ║
╠══════════════════════════════════════════════════════════════════════╣
║ G4 — "RED e IR observam o mesmo evento?"                           ║
║ período, contagem, matching, alignment, correlação auxiliar        ║
║ princípio da oximetria + eventos G2/G3                             ║
╚══════════════════════════════════════════════════════════════════════╝
```

---

# 67. Ordem de desenvolvimento

```text
G1 implementado
   |
   v
calibrar G1 NO_GRIP
   |
   v
preprocess
   |
   v
G2 + testes + calibração
   |
   v
Beat Detector
   |
   v
G3 + testes + calibração
   |
   v
G4 + testes + calibração
   |
   v
VALID = G1 && G2 && G3 && G4
   |
   v
score deixa de decidir
   |
   v
congelar NO_GRIP
   |
   v
validar SpO2
   |
   v
integrar ao I Blue It
   |
   v
WITH_GRIP
```

---

# 68. Critério de conclusão de cada gate

```text
[ ] hipótese definida
[ ] features definidas
[ ] equações documentadas
[ ] parâmetros externalizados
[ ] origem registrada
[ ] unit tests
[ ] sintético
[ ] dados reais
[ ] development split
[ ] hold-out
[ ] FAR/FRR
[ ] sensibilidade de threshold
[ ] telemetria
[ ] documentação
[ ] versionamento
```

---

# 69. Conclusão

A arquitetura de SQI adequada ao RFC e ao firmware é uma cadeia lógica em que cada gate responde a uma pergunta científica diferente:

```text
INTEGRIDADE
    |
    v
PULSATILIDADE
    |
    v
MORFOLOGIA
    |
    v
COERÊNCIA DUAL-CHANNEL
    |
    v
PPG UTILIZÁVEL
```

A literatura define propriedades relevantes, mas thresholds dependentes do hardware e do arranjo óptico precisam ser calibrados no próprio sistema.

\[
SQI:
(PPG_{RED},PPG_{IR},t,\theta)
\rightarrow
\{VALID,INVALID\}
\]

\[
USE_{SpO2}
=
G1 \land G2 \land G3 \land G4
\land ESTIMATOR_{valid}
\]

O resultado esperado não é um sistema que “conserte qualquer PPG”, mas um sistema que saiba **quando não confiar no sinal**.

---

# Referências

**DESQUINS, T.; BOUSEFSAF, F.; PRUSKI, A.; MAAOUI, C.** A Survey of Photoplethysmography and Imaging Photoplethysmography Quality Assessment Methods. *Applied Sciences*, 12(19), 9582, 2022. DOI: `10.3390/app12199582`.

**ELGENDI, M.** Optimal Signal Quality Index for Photoplethysmogram Signals. *Bioengineering*, 3(4), 21, 2016. DOI: `10.3390/bioengineering3040021`.

**FISCHER, C.; DOMER, B.; WIBMER, T.; PENZEL, T.** An Algorithm for Real-Time Pulse Waveform Segmentation and Artifact Detection in Photoplethysmograms. *IEEE Journal of Biomedical and Health Informatics*, 21(2), 372–381, 2017. DOI: `10.1109/JBHI.2016.2518202`.

**KARLEN, W.; KOBAYASHI, K.; ANSERMINO, J. M.; DUMONT, G. A.** Photoplethysmogram signal quality estimation using repeated Gaussian filters and cross-correlation. *Physiological Measurement*, 33(10), 1617–1629, 2012. DOI: `10.1088/0967-3334/33/10/1617`.

**ORPHANIDOU, C.; BONNICI, T.; CHARLTON, P.; CLIFTON, D.; VALLANCE, D.; TARASSENKO, L.** Signal-Quality Indices for the Electrocardiogram and Photoplethysmogram: Derivation and Applications to Wireless Monitoring. *IEEE Journal of Biomedical and Health Informatics*, 19(3), 832–838, 2015. DOI: `10.1109/JBHI.2014.2338351`.

**REDDY, G. N. K.; MANIKANDAN, M. S.; MURTY, N. V. L. N.** On-Device Integrated PPG Quality Assessment and Sensor Disconnection/Saturation Detection System for IoT Health Monitoring. *IEEE Transactions on Instrumentation and Measurement*, 69(9), 6351–6361, 2020. DOI: `10.1109/TIM.2020.2971132`.

**SUKOR, J. A.; REDMOND, S. J.; LOVELL, N. H.** Signal quality measures for pulse oximetry through waveform morphology analysis. *Physiological Measurement*, 32(3), 369–384, 2011. DOI: `10.1088/0967-3334/32/3/008`.

**VADREVU, S.; MANIKANDAN, M. S.** Real-Time PPG Signal Quality Assessment System for Improving Battery Life and False Alarms. *IEEE Transactions on Circuits and Systems II: Express Briefs*, 66(11), 1910–1914, 2019. DOI: `10.1109/TCSII.2019.2891636`.

**ANALOG DEVICES / MAXIM INTEGRATED.** MAX30102: High-Sensitivity Pulse Oximeter and Heart-Rate Sensor for Wearable Health. Datasheet.

**SANTOS, A. M. et al.** I Blue It: Um Jogo Sério para auxiliar na Reabilitação Respiratória. *SBGames*, 2018.

**GRIMES, R. H.** Sistema biomédico (com jogo sério e dispositivo especial) para reabilitação respiratória. Dissertação, UDESC, 2018.

**NÉRY, J. T. C.; HENRIQUE, Y. A. M.; HOUNSELL, M. S.** 123-SGR: Uma Arquitetura para Jogos Sérios Multimodais para Reabilitação. *SBGames*, 2020.

**DIAS, C.** Flow Psicofisiológico em Jogos Digitais: Inteligência Artificial em Jogos Sérios Multimodais para Reabilitação Respiratória. Tese de Doutorado, UDESC, 2024.

---

# Apêndice A — Mapa teoria → implementação

```text
Desconexão/saturação ----------> G1
Integridade temporal -----------> G1

Amplitude pulsátil -------------> G2
Perfusion Index ----------------> G2 feature
Threshold crossings ------------> G2
Autocorrelação -----------------> G2

Segmentação de pulsos ----------> Beat Detector
Amplitude por beat -------------> G3
Width --------------------------> G3
Rise time ----------------------> G3
IBI ----------------------------> G3
Template/correlação B2B --------> G3 opcional

Dual wavelength ----------------> G4
Período RED vs IR --------------> G4
Pareamento de beats ------------> G4
Correlação RED/IR --------------> G4 auxiliar

Score contínuo -----------------> diagnóstico/confidence
Regra clínica ------------------> fora do SQI
DeepDDA ------------------------> camada superior futura
```

---

# Apêndice B — Mapa de pontos de ajuste

```text
MAX30102
 |
 +-- sample_rate_hz
 +-- pulse_width
 +-- ADC range
 +-- RED LED current
 +-- IR LED current
 |
 v
G1
 |
 +-- rail_margin_counts
 +-- minimum_mean_level
 +-- minimum_raw_range
 +-- maximum_clipping_fraction
 +-- minimum_continuity_fraction
 +-- maximum_interval_deviation_fraction
 |
 v
PREPROCESS
 |
 +-- método detrending
 +-- cutoff
 |
 v
G2
 |
 +-- min amplitude RED
 +-- min amplitude IR
 +-- crossings min/max
 +-- ACF min
 +-- period min/max
 |
 v
BEAT DETECTOR
 |
 +-- peak thresholding
 +-- refractory period
 +-- segment boundaries
 |
 v
G3
 |
 +-- minimum_valid_beats
 +-- width range
 +-- rise range
 +-- amplitude variability
 +-- IBI variability
 +-- outlier fraction
 +-- template threshold
 |
 v
G4
 |
 +-- period tolerance
 +-- count tolerance
 +-- match window
 +-- matched fraction
 +-- correlation threshold
 |
 v
VALID
```

---

# Apêndice C — Regra para qualquer nova feature

```text
1. Qual pergunta científica ela responde?
2. A qual gate pertence?
3. Qual literatura a sustenta?
4. Depende da escala RAW?
5. Altera o RAW?
6. É condição obrigatória ou diagnóstico?
7. Como será testada sinteticamente?
8. Como será calibrada?
9. Qual o risco de falso aceite?
10. Como será versionada?
```

---

# Apêndice D — Estado final esperado

```text
                 ┌──────────────────┐
                 │      RAW PPG     │
                 └────────┬─────────┘
                          v
                  [ G1 INTEGRITY ]
                          |
                    fail  |  pass
                    <-----+------>
                          v
                 [ G2 PULSATILITY ]
                          |
                    fail  |  pass
                    <-----+------>
                          v
                   [ BEAT DETECTOR ]
                          |
                          v
                  [ G3 MORPHOLOGY ]
                          |
                    fail  |  pass
                    <-----+------>
                          v
                  [ G4 RED ↔ IR ]
                          |
                    fail  |  pass
                    <-----+------>
                               |
                               v
                        PPG_QUALITY_VALID
                               |
                   +-----------+-----------+
                   v                       v
                 HR                    SpO₂
                   \                       /
                    \                     /
                     +---- confidence ---+
                               |
                               v
                          health_frame
                               |
                               v
                           I Blue It
```
