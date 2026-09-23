# Projeto Matemático do G2 — Reddy 2020 / FOPC-DS no ATmega328P

## 1. Objetivo

Este documento fecha a formulação matemática que deve orientar a próxima implementação do Gate 02 no Arduino Uno/Nano.

A base primária é:

> G. N. K. Reddy, M. S. Manikandan e N. V. L. Narasimha Murty,  
> "On-Device Integrated PPG Quality Assessment and Sensor Disconnection/Saturation Detection System for IoT Health Monitoring",  
> IEEE Transactions on Instrumentation and Measurement, DOI: 10.1109/TIM.2020.2971132.

A opção selecionada é o **FOPC-DS**: First-Order Predictor Coefficient extraído do **Differenced Sensor Signal**.

O objetivo não é copiar thresholds de 10 bits / 100 Hz sem validação, mas preservar a transformação matemática e a lógica hierárquica do método, adaptando a execução à SRAM/CPU do ATmega328P.

---

# 2. O que o artigo faz

A sequência publicada é:

~~~text
segmento de sensor s[n], 5 s
        |
        v
Rule-01: nearly-zero amplitude
        |
        v
Rule-02: saturation
        |
        v
d[n] = s[n] - s[n-1]
        |
        v
y[n] = d[n] / max(d[n])
        |
        v
z[n] = y[n] + a*w[n]
        |
        v
FOPC alpha
        |
        +--> alpha > 0.93          -> Noise-Free PPG
        +--> -0.5 < alpha < 0.93   -> Motion Artifact
        +--> alpha < -0.5          -> Pulse-Free signal
~~~

No estudo:

~~~text
janela = 5 s
Fs = 100 Hz na análise/validação do Reddy 2020
quantização = 10 bits
noise level selecionado = 0.10 * Vmax
~~~

Os thresholds de alpha foram obtidos a partir das distribuições de dados do estudo e não devem ser assumidos como universais.

---

# 3. Etapa matemática 1 — primeira diferença

O artigo define:

~~~text
d[n] = s[n] - s[n-1]
~~~

## Função

A diferença de primeira ordem:

- reduz a influência de baseline lentamente variável;
- reduz a influência de componentes de baixa frequência;
- enfatiza componentes de frequência mais alta;
- elimina a necessidade de HPF usada na alternativa FOPC-FS.

## No MAX30102

A amostra RAW é de 18 bits:

~~~text
0 <= s[n] <= 262143
~~~

Logo:

~~~text
-262143 <= d[n] <= +262143
~~~

Portanto:

- s[n] pode continuar como uint32_t;
- d[n] precisa de int32_t.

## Memória

Não é necessário armazenar um vetor d[500].

Para cada nova amostra:

~~~text
d = current - previous
previous = current
~~~

Somente a amostra anterior precisa permanecer em RAM.

---

# 4. Etapa matemática 2 — normalização de amplitude

O pseudocódigo do artigo escreve:

~~~text
y[n] = d[n] / max(d[n])
~~~

Defina:

~~~text
M = max(d[n])
~~~

Então:

~~~text
y[n] = d[n] / M
~~~

## Função

A normalização reduz dependência da escala absoluta do sensor antes da perturbação e da extração do predictor coefficient.

## Ponto de atenção

A fonte escreve max(d[n]), e não max(abs(d[n])). Portanto o port deve inicialmente reproduzir essa definição.

Qualquer mudança para max(abs(d[n])) precisa ser tratada como adaptação e comparada experimentalmente.

---

# 5. Etapa matemática 3 — adição de ruído

O artigo define:

~~~text
z[n] = y[n] + a*w[n]
~~~

onde:

- w[n] é uma sequência de ruído aleatório;
- a define o nível do ruído.

O trabalho avalia níveis de 5% a 100% do Vmax e seleciona 10% como compromisso com bom desempenho:

~~~text
a = 0.10
~~~

## Por que existe

Os autores mostram que componentes residuais lentas podem manter predictor coefficients elevados mesmo em sinais ruins.

A perturbação ajuda a aumentar a separação entre:

~~~text
NF = noise-free PPG
MA = motion-artifact PPG
PF = pulse-free signal
~~~

## Ponto ainda não fechado

O artigo textual fornecido não especifica suficientemente a distribuição/gerador exato de w[n].

Para reprodução bit-a-bit será necessário conferir o código-fonte suplementar indicado pelos autores.

Para o port de bancada, caso isso não seja recuperado, deve-se usar uma sequência pseudoaleatória:

- de distribuição documentada;
- amplitude conhecida;
- seed fixa durante calibração;
- reproduzível Arduino x referência.

---

# 6. Etapa matemática 4 — predição linear

O artigo define um preditor de ordem P:

~~~text
x_hat[n] = sum(k=1..P) alpha_k * x[n-k]
~~~

e os coeficientes ótimos minimizam o erro quadrático médio.

Em forma matricial:

~~~text
alpha = Rxx^-1 * rxx
~~~

Os autores usam Levinson-Durbin para resolver o sistema.

Para o método escolhido:

~~~text
P = 1
~~~

Logo existe somente:

~~~text
alpha_1 = alpha
~~~

e:

~~~text
z_hat[n] = alpha * z[n-1]
~~~

---

# 7. Derivação de engenharia para P=1

Esta seção é uma **derivação matemática do projeto**, não uma equação escrita explicitamente por Reddy.

Para ordem 1, a equação de Yule-Walker reduz-se a:

~~~text
R0 * alpha = R1
~~~

portanto:

~~~text
alpha = R1 / R0
~~~

com uma estimativa possível:

~~~text
R0 = sum(z[n]^2)
R1 = sum(z[n] * z[n-1])
~~~

Assim:

~~~text
             sum(z[n] * z[n-1])
alpha = -------------------------------
                sum(z[n]^2)
~~~

A convenção exata dos limites/normalização deve ser mantida igual na implementação Arduino e na referência de PC.

## Por que isso é útil no Uno

Para P=1 não é necessário manter:

~~~text
float z[500]
matrix R
vector r
~~~

Podemos acumular apenas:

~~~text
R0
R1
z_previous
~~~

e ao final:

~~~text
alpha = R1 / R0
~~~

Entretanto ainda existe um problema: z[n] depende de M = max(d[n]), que só é conhecido ao fim da janela.

---

# 8. Como evitar um buffer de 500 amostras

Existem três soluções possíveis.

## Solução A — duas passagens sobre um buffer

Passagem 1:

~~~text
calcular d[n]
descobrir M
~~~

Passagem 2:

~~~text
normalizar
adicionar ruído
calcular R0/R1
~~~

Problema:

A 100 Hz e 5 s:

~~~text
500 amostras/canal
~~~

Packed18 dual-channel exigiria:

~~~text
500 * 3 * 2 = 3000 B
~~~

inviável no ATmega328P.

---

## Solução B — armazenar apenas um canal

~~~text
500 * 3 = 1500 B
~~~

Tecnicamente pode caber, mas deixa pouca SRAM para:

- stack;
- serial;
- variáveis dos Gates;
- PITACO futuro.

Não é a opção preferida.

---

## Solução C — acumular estatísticas suficientes em uma única passagem

Esta é a opção mais promissora e deve ser validada matematicamente contra uma implementação de referência.

Como:

~~~text
z[n] = d[n]/M + a*w[n]
~~~

multiplicar toda a sequência por M não altera o predictor coefficient de primeira ordem:

~~~text
q[n] = M*z[n]
     = d[n] + a*M*w[n]
~~~

Logo:

~~~text
alpha(z) = alpha(q)
~~~

porque o fator global M cancela em R1/R0.

Defina:

~~~text
b = a*M
q[n] = d[n] + b*w[n]
~~~

Então:

~~~text
R1 = sum(q[n] * q[n-1])
~~~

Expansão:

~~~text
R1 =
sum(d[n]d[n-1])
+ b * sum(d[n]w[n-1] + w[n]d[n-1])
+ b^2 * sum(w[n]w[n-1])
~~~

E:

~~~text
R0 = sum(q[n]^2)
~~~

Expansão:

~~~text
R0 =
sum(d[n]^2)
+ 2b * sum(d[n]w[n])
+ b^2 * sum(w[n]^2)
~~~

Portanto, durante a aquisição podemos acumular:

~~~text
A0 = sum(d[n]^2)
A1 = sum(d[n]d[n-1])

B0 = sum(d[n]w[n])
B1 = sum(d[n]w[n-1] + w[n]d[n-1])

C0 = sum(w[n]^2)
C1 = sum(w[n]w[n-1])

M  = max(d[n])
~~~

Ao final:

~~~text
b = a*M

R0 = A0 + 2*b*B0 + b^2*C0
R1 = A1 + b*B1 + b^2*C1

alpha = R1/R0
~~~

## Consequência

É possível implementar o FOPC-DS em **O(1) SRAM por canal**, mesmo em 100 Hz, se essa derivação produzir o mesmo alpha que a implementação de referência.

Isso é extremamente importante porque remove a aparente necessidade de armazenar 500 amostras por canal.

---

# 9. Tipos numéricos

Para 18 bits:

~~~text
|d[n]| <= 262143
~~~

Logo:

~~~text
d[n]^2 <= aproximadamente 6.87e10
~~~

Em 500 amostras:

~~~text
sum(d[n]^2) <= aproximadamente 3.44e13
~~~

Isso:

- excede uint32_t;
- cabe com folga em uint64_t.

Portanto os acumuladores quadráticos devem usar 64 bits ou representação fixa equivalente validada.

A implementação deve verificar também os termos envolvendo ruído antes de escolher a escala fixa definitiva.

---

# 10. O que já existe no firmware

## Aquisição

Já existe:

~~~text
MAX30102
RED + IR
RAW em uint32_t
janela temporal de 5 s
~~~

## G1

Já existe:

~~~text
mean
range
upper saturation
PASS/FAIL hierárquico
~~~

Isso implementa o princípio de rejeição precoce, mas ainda não é a Rule-01/Rule-02 de Reddy de forma literal.

## Packed18

Já existe e foi validado:

~~~text
RAW18 -> 3 bytes -> RAW18
mismatch = 0
~~~

O Packed18 continua útil para Gates que realmente precisem da forma de onda.

Mas, se o G2 FOPC-DS for implementado por acumuladores suficientes, ele não precisa usar Packed18.

## G2 atual

A branch atual possui um diagnóstico baseado em Vadrevu:

~~~text
amplitude
crossings
ACF
FZCP
peak lag
~~~

Esse código deve ser tratado como experimento/branch histórica e não como o G2 final se a Rota Reddy for adotada.

---

# 11. O que precisa ser implementado

## G1 / pré-G2

Antes do FOPC:

1. revisar Rule-01 nearly-zero;
2. acrescentar lower saturation;
3. manter upper saturation;
4. decidir se mean/range continuam como features auxiliares.

## G2 FOPC-DS diagnóstico

Implementar inicialmente sem PASS/FAIL:

~~~text
d[n]
M = max(d[n])
ruído w[n] reprodutível
estatísticas suficientes
R0
R1
alpha
tempo de execução
freeRAM
~~~

Saída de diagnóstico sugerida:

~~~text
G2_FOPC
RED[alpha=... M=...]
IR[alpha=... M=...]
samples=...
G2_ms=...
freeRAM=...
~~~

## Não implementar ainda

Não ativar ainda:

~~~text
alpha > 0.93 -> PASS
-0.5 ... -> MA
alpha < -0.5 -> PF
~~~

Os valores publicados devem ser impressos apenas como referência até a calibração.

---

# 12. Taxa de amostragem — decisão crítica

Reddy usa:

~~~text
Fs = 100 Hz
janela = 5 s
N = 500
~~~

Nosso firmware atual produz aproximadamente:

~~~text
Fs efetiva = 25 Hz
N ~= 125
~~~

O predictor coefficient mede relação entre amostras consecutivas.

Logo mudar Fs muda o significado temporal de:

~~~text
z[n] versus z[n-1]
~~~

e pode alterar significativamente alpha.

## Estratégia recomendada

Se a implementação O(1) SRAM for validada, testar o MAX30102 com:

~~~text
sampleAverage = 1
sampleRate = 100 Hz
~~~

para aproximar a condição temporal usada por Reddy sem exigir 3000 B de buffer.

Primeiro, porém, isso deve ser validado isoladamente:

- FIFO sem perda;
- CPU suficiente;
- coexistência com G1;
- freeRAM;
- tempo de processamento;
- qualidade RAW.

A configuração atual de 25 Hz não deve ser descartada antes da comparação. Ela pode funcionar, mas exigirá thresholds próprios.

---

# 13. Validação matemática antes de bancada

Criar uma implementação de referência no PC/Python.

Para vetores conhecidos:

~~~text
s[n]
-> d[n]
-> y[n]
-> w[n]
-> z[n]
-> Levinson-Durbin P=1 / R1-R0
-> alpha_ref
~~~

Executar o mesmo vetor no Arduino:

~~~text
alpha_uno
~~~

Critério:

~~~text
|alpha_uno - alpha_ref| <= epsilon
~~~

O epsilon deve ser escolhido com base na representação numérica utilizada.

Também comparar:

~~~text
implementação direta com arrays
versus
implementação O(1) por estatísticas suficientes
~~~

A implementação O(1) só pode ser adotada quando ambas produzirem o mesmo alpha dentro do erro definido.

---

# 14. Validação de bancada

Após a matemática:

Categorias mínimas:

~~~text
NF — dedo estável
MA — movimento com contato
PF — sem dedo / sem pulso
~~~

Subtipos MA:

~~~text
movimento aleatório
movimento periódico
mudança de pressão
deslocamento lateral
colocação/remoção
~~~

Para cada janela:

~~~text
alpha_RED
alpha_IR
M_RED
M_IR
Fs efetiva
sample_count
G2_ms
freeRAM
label experimental
~~~

Comparar duas configurações:

~~~text
A. 25 Hz efetivos
B. 100 Hz efetivos
~~~

Somente depois escolher:

- Fs final;
- canal do SQI;
- amplitude de ruído;
- thresholds de alpha.

---

# 15. Relação com a arquitetura de Gates

A arquitetura pode ficar:

~~~text
MAX30102
   |
   +--> RAW sample
          |
          +--> G1 streaming
          |    nearly-zero / clipping / integridade
          |
          +--> G2 streaming
               d[n]
               sufficient statistics
               FOPC alpha
               |
               +--> qualidade temporal
~~~

Packed18 passa a ser uma camada paralela, reservada para etapas que precisem da forma da onda:

~~~text
RAW
 |
 +--> G1 streaming
 +--> G2 FOPC streaming
 +--> Packed18 / armazenamento controlado
        |
        +--> G3 morfologia
        +--> SpO2 conforme necessidade
~~~

Isso preserva o ganho de memória e permite considerar Fs=100 Hz sem armazenar 5 s completos de RED+IR.

---

# 16. Decisões ainda abertas

Antes de codificar a versão final:

1. confirmar o gerador/distribuição de w[n] do código suplementar de Reddy;
2. validar a derivação O(1) contra a referência;
3. comparar 25 Hz e 100 Hz;
4. definir se FOPC roda em IR apenas ou RED+IR;
5. revisar G1 para aproximá-lo das Rules 01–02;
6. calibrar thresholds apenas após os dados de bancada.

---

# 17. Conclusão

A principal mudança arquitetural obtida desta análise é:

> O G2 FOPC-DS não precisa necessariamente de um buffer de 5 s.

Para ordem 1, a informação necessária pode ser reduzida a acumuladores e estado da amostra anterior, desde que a normalização e a adição de ruído sejam tratadas por uma formulação matematicamente equivalente e validada contra uma implementação de referência.

Isso abre a possibilidade de utilizar a taxa de 100 Hz de Reddy no ATmega328P sem reservar 3000 B para RED+IR, mantendo o G1 e o G2 como estágios streaming e deixando a memória de forma de onda para os Gates posteriores.
