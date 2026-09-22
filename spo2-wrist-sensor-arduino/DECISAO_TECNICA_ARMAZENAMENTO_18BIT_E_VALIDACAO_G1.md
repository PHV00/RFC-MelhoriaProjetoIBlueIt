# Decisão técnica — preservação de amostras MAX30102 em 3 bytes e validação do Gate 01

## 1. Contexto

O MAX30102 pode operar com resolução de até 18 bits. A biblioteca SparkFun entrega as amostras RED e IR em `uint32_t`, pois não existe um tipo inteiro nativo de 18 bits em C/C++.

No Arduino Uno R3 (ATmega328P), a SRAM disponível é de apenas 2 KB. Armazenar 100 amostras de RED e 100 de IR em `uint32_t` consome 800 bytes:

```text
100 x 4 bytes x 2 canais = 800 bytes
```

Esse custo se soma aos buffers internos do algoritmo Maxim/SparkFun e ao restante do firmware, tornando a solução inadequada para o Uno quando usada de forma ingênua.

O baseline histórico também usa `uint16_t` no AVR, mas isso é insuficiente quando a aquisição está configurada em 18 bits e os valores ultrapassam 65535. O cast direto produz wrap/truncamento e pode introduzir descontinuidades artificiais no sinal.

## 2. Decisão

A decisão desta branch é preservar os 18 bits integralmente usando **3 bytes por amostra**, sem reduzir a resolução do sinal.

Cada amostra é representada como:

```text
byte 0 -> bits 0..7
byte 1 -> bits 8..15
byte 2 -> bits 16..17
```

Os seis bits superiores do terceiro byte permanecem zerados.

A representação ocupa:

```text
100 amostras RED x 3 bytes = 300 bytes
100 amostras IR  x 3 bytes = 300 bytes
TOTAL = 600 bytes
```

Isso reduz em 25% o custo dos buffers quando comparado com dois vetores `uint32_t[100]`, preservando exatamente os dados de 18 bits.

## 3. O que esta decisão NÃO significa

Esta decisão:

- não altera a resolução física do MAX30102;
- não converte 18 bits para 16 bits;
- não resolve, sozinha, todo o consumo de memória do algoritmo de SpO2;
- não obriga o Gate 01 a armazenar a forma de onda.

O Gate 01 continua trabalhando diretamente sobre o RAW entregue pela biblioteca, uma amostra por vez.

## 4. Arquitetura adotada

```text
MAX30102
   |
   v
RAW RED/IR em uint32_t temporário
   |
   +--------------------------+
   |                          |
   v                          v
Gate 01 streaming        armazenamento opcional
min/max/soma/range       3 bytes por amostra
saturação                para Gates posteriores/
                         SpO2 e testes de memória
```

O uso de `uint32_t` no ponto de leitura é intencional e barato: são apenas variáveis temporárias. O problema de SRAM ocorre quando grandes janelas são mantidas como arrays de `uint32_t`.

## 5. Implementação no código

### 5.1 Leitura RAW

Em `sqi_flow_v5_baseline.ino`:

```cpp
const uint32_t red = particleSensor.getFIFORed();
const uint32_t ir  = particleSensor.getFIFOIR();

gate1AddSample(red, ir);
```

O G1 recebe os 18 bits sem conversão.

### 5.2 Armazenamento em 3 bytes

Em `packed_18_storage.ino`:

```cpp
struct Packed18
{
  uint8_t b0;
  uint8_t b1;
  uint8_t b2;
};
```

A codificação aplica máscara de 18 bits e separa o valor em três bytes. A decodificação reconstrói o mesmo `uint32_t`.

### 5.3 Teste lossless

O firmware de armazenamento compara:

```text
RAW18 original
   ->
encode 3 bytes
   ->
decode
   ->
RAW18 reconstruído
```

A validação é aceita somente com:

```text
PACK18_RESULT=PASS
mismatch=0
bytes_per_sample=3
buffers_bytes=600
```

## 6. Separação metodológica dos testes

O teste de armazenamento e a validação do Gate 01 são independentes.

Por padrão:

```cpp
#define ENABLE_PACKED18_STORAGE_TEST 0
```

Assim, os 600 bytes dos buffers não são reservados e o Gate 01 pode ser revalidado isoladamente.

Para testar apenas a estratégia de armazenamento:

```cpp
#define ENABLE_PACKED18_STORAGE_TEST 1
```

## 7. Gate 01 — objetivo

O Gate 01 responde à pergunta:

> Existe sinal útil em RED e IR e a aquisição está livre de condições obviamente inválidas, como ausência de sinal, flatline ou saturação?

Ele verifica:

- presença de sinal por média;
- faixa dinâmica mínima;
- saturação/clipping;
- existência de amostras.

Ele não avalia ainda:

- periodicidade;
- morfologia;
- coerência entre canais;
- validade fisiológica de FC/SpO2.

Esses itens pertencem aos Gates 02, 03 e 04.

## 8. Thresholds atuais

Os thresholds atuais são provisórios para bancada:

```cpp
G1_MIN_MEAN_SIGNAL   = 5000
G1_MIN_DYNAMIC_RANGE = 500
G1_SATURATION_MARGIN = 2048
```

Eles não devem ser tratados como limites clínicos ou universais. A validação atual serve para caracterizar o comportamento do MAX30102, da montagem e do futuro pegador.

## 9. Protocolo de revalidação do Gate 01

Executar com:

```cpp
#define ENABLE_PACKED18_STORAGE_TEST 0
```

### Cenário A — sem dedo

Objetivo: verificar rejeição por ausência clara de sinal.

Esperado:

```text
G1_RESULT=FAIL
reason=RED_LOW_SIGNAL
```

ou falha equivalente no IR.

### Cenário B — dedo estável

Manter o dedo imóvel por pelo menos três janelas de 5 s.

Esperado:

```text
G1_RESULT=PASS
reason=NONE
sat=0
```

### Cenário C — contato parcial

Objetivo: observar médias e ranges quando o acoplamento óptico piora. Não há resultado predefinido nesta fase; os dados servirão para caracterização dos thresholds.

### Cenário D — pressão excessiva

Objetivo: verificar se o G1 permanece restrito à integridade básica. Uma janela pode ainda passar no G1 e ser rejeitada depois por baixa pulsatividade no G2.

## 10. Critério para encerrar a validação inicial do G1

A validação inicial de bancada é considerada satisfatória quando:

- ausência de dedo é rejeitada de forma consistente;
- dedo estável é aceito de forma consistente;
- não ocorre saturação em condições normais de uso;
- os relatórios apresentam médias e ranges coerentes entre janelas;
- nenhum problema de memória ou perda de FIFO é observado.

Após isso, os dados de contato parcial e pressão devem ser usados para refinar thresholds, antes de tratá-los como estáveis.

## 11. Próximos passos

1. Revalidar o Gate 01 isoladamente.
2. Testar o armazenamento de 18 bits em 3 bytes separadamente.
3. Refinar os thresholds do G1 com dados reais.
4. Implementar e validar o Gate 02.
5. Implementar e validar o Gate 03.
6. Implementar e validar o Gate 04.
7. Somente então integrar FC/SpO2 e o contrato final com o I Blue It.

## 12. Justificativa acadêmica

A representação em 3 bytes é uma decisão de engenharia motivada pela restrição de SRAM do ATmega328P. Ela não é apresentada como uma exigência da literatura.

A adequação da decisão deve ser demonstrada por evidência experimental:

```text
RAW18 original == RAW18 reconstruído
mismatch = 0
```

e por ausência de alteração nos resultados das etapas que utilizarem a amostra reconstruída.

A fundamentação científica do pipeline permanece na necessidade de avaliar qualidade do PPG antes de derivar variáveis fisiológicas e na arquitetura 123-SGR, em que o tratamento de sinais precede a utilização de dados pelo jogo.