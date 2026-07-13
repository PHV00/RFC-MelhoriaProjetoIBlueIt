# Análise Minuciosa do Repositório — Firmware de Oximetria

## Visão geral

Esta revisão estática considera o firmware disponível em `spo2-wrist-sensor/main`.

A arquitetura está organizada em módulos e o componente ESP-IDF compila as diferentes camadas do sistema. Porém, o fluxo executado atualmente ainda funciona principalmente como um **protótipo de aquisição e transmissão de dados brutos**.

No fluxo atual, o firmware:

1. inicializa o barramento I²C;
2. inicializa e configura o sensor;
3. lê uma amostra RED/IR por ciclo;
4. armazena a amostra no buffer;
5. imprime a amostra pela serial.

Os módulos de qualidade, frequência cardíaca, SpO₂, confiança e telemetria de frame existem e são compilados, mas ainda não estão integrados ao fluxo principal executado pelo controlador.

> Esta é uma análise estática do código. Não inclui compilação, execução em hardware ou validação clínica.

---

## Resumo geral

| Classificação | Quantidade aproximada |
|---|---:|
| Problemas críticos | 9 |
| Problemas altos | 12 |
| Problemas médios | 13 |
| Melhorias arquiteturais | 8 |

---

# 1. Problemas críticos

## 1.1 O cálculo de SpO₂ não existe

A função `spo2_estimator_compute(...)` ignora o buffer:

```c
(void)buffer;
```

O resultado é zerado e a função sempre retorna `false`.

Portanto, o firmware atualmente:

- coleta RED e IR;
- não calcula a razão dos quocientes;
- não aplica curva de calibração;
- não produz uma estimativa de SpO₂ válida.

### Impacto

O firmware ainda não pode ser considerado um oxímetro funcional.

---

## 1.2 O cálculo de frequência cardíaca não existe

A função `hr_estimator_compute(...)` também:

- ignora o buffer;
- inicializa o resultado como inválido;
- retorna `false`.

Assim:

```text
hr.valid = false
hr.bpm = 0
```

### Impacto

Nenhuma frequência cardíaca é produzida no fluxo atual.

---

## 1.3 Os módulos de processamento não são executados

Embora o controlador inclua:

```c
signal_quality.h
hr_estimator.h
spo2_estimator.h
confidence_engine.h
```

essas funções não são chamadas em `app_controller_step()`.

O fluxo real é:

```text
MAX3010x
   ↓
ppg_sampler
   ↓
sample_buffer
   ↓
serial RAW
```

O fluxo arquitetural desejado seria:

```text
MAX3010x
   ↓
ppg_sampler
   ↓
sample_buffer
   ↓
signal_quality
   ↓
hr_estimator
   ↓
spo2_estimator
   ↓
confidence_engine
   ↓
health_frame
   ↓
telemetria
```

### Impacto

Mesmo os módulos já existentes não afetam o comportamento atual do firmware.

---

## 1.4 Incompatibilidade entre produção e consumo da FIFO

O sensor está configurado para uma taxa equivalente a aproximadamente 100 amostras por segundo.

O controlador executa a coleta aproximadamente a cada 50 ms:

```text
1000 ms / 50 ms = aproximadamente 20 leituras por segundo
```

Cada chamada retira apenas uma amostra.

Fluxo provável:

```text
sensor produz aproximadamente 100 amostras/s
firmware consome aproximadamente 20 amostras/s
diferença acumulada: aproximadamente 80 amostras/s
```

### Impacto

A FIFO pode:

- encher rapidamente;
- perder amostras;
- acumular atraso;
- fornecer dados antigos;
- comprometer a forma de onda.

---

## 1.5 O driver lê a FIFO sem verificar se há dados

A função `max3010x_read_sample()` lê diretamente seis bytes de `REG_FIFO_DATA`.

Ela não consulta:

- `FIFO_WR_PTR`;
- `FIFO_RD_PTR`;
- `OVF_COUNTER`;
- registradores de interrupção;
- quantidade de amostras disponíveis.

### Impacto

O firmware não sabe:

- se existe uma amostra nova;
- quantas amostras estão disponíveis;
- se houve overflow;
- se ocorreu perda;
- se a leitura está atrasada.

---

## 1.6 Overflow da FIFO não é detectado

O registrador `REG_OVF_COUNTER` é zerado durante a configuração, mas não é lido novamente.

### Impacto

O firmware pode perder amostras sem:

- registrar o problema;
- alterar o estado;
- limpar a FIFO;
- sincronizar a coleta;
- avisar a aplicação.

---

## 1.7 Os timestamps não representam o instante real da medição

O timestamp é atribuído quando o ESP retira a amostra da FIFO:

```c
sample.timestamp_ms =
    (uint32_t)(esp_timer_get_time() / 1000ULL);
```

Ele não representa necessariamente o instante em que o sensor produziu a amostra.

Com acúmulo na FIFO:

```text
sensor produz a amostra
        ↓
amostra aguarda na FIFO
        ↓
ESP lê posteriormente
        ↓
timestamp atual é atribuído
```

### Impacto

Os intervalos temporais podem ficar incorretos, comprometendo:

- cálculo de frequência cardíaca;
- filtros;
- análise de periodicidade;
- tamanho real da janela;
- estimativas dependentes de tempo.

---

## 1.8 O sensor é marcado como inicializado sem identificação

`max3010x_init()` preenche a estrutura e define:

```c
dev->initialized = true;
```

Porém, não verifica:

- `PART_ID`;
- `REV_ID`;
- identidade do componente;
- variante;
- capacidades;
- resposta real do hardware.

### Impacto

O campo `initialized` significa apenas que a estrutura foi preenchida, não que o sensor foi validado.

---

## 1.9 Não existe recuperação operacional de erro

Quando `ppg_sampler_step()` falha, o controlador entra em `APP_STATE_ERROR`.

Na próxima iteração, ele tenta coletar novamente sem:

- reinicializar o sensor;
- limpar a FIFO;
- reiniciar o barramento;
- contar falhas consecutivas;
- aplicar atraso progressivo;
- diferenciar falha transitória de permanente.

Se a próxima leitura funcionar, o controlador volta diretamente para `IDLE`.

### Impacto

O estado de erro não representa uma recuperação controlada.

---

# 2. Máquina de estados

## 2.1 O módulo não é uma máquina de estados completa

O código atual permite:

```c
app_state_machine_set(APP_STATE_TRACKING);
```

sem validar:

- estado atual;
- próxima transição;
- condições necessárias;
- evento responsável.

### Impacto

Qualquer estado pode ser atribuído a qualquer momento.

---

## 2.2 Estado `IDLE` não corresponde ao comportamento real

Após coletar uma amostra, o sistema define:

```c
APP_STATE_IDLE
```

Entretanto, continua coletando periodicamente.

```text
estado informado: IDLE
comportamento real: aquisição contínua
```

### Impacto

A telemetria de estado pode ser enganosa.

---

## 2.3 Estados declarados não são utilizados

Os seguintes estados existem, mas não participam do fluxo real:

```text
SELF_TEST
SAMPLING
TRACKING
LOW_CONFIDENCE
```

---

## 2.4 Não existe autoteste real

O estado `APP_STATE_SELF_TEST` existe, mas o firmware não executa:

- leitura de Part ID;
- leitura de Revision ID;
- validação de registradores;
- teste dos LEDs;
- teste da FIFO;
- teste de canais;
- confirmação de sinal.

---

## 2.5 Não existe política formal de transições

Uma política coerente seria:

```text
BOOT → SELF_TEST
SELF_TEST → IDLE ou ERROR
IDLE → SAMPLING
SAMPLING → TRACKING ou LOW_CONFIDENCE
TRACKING ↔ LOW_CONFIDENCE
qualquer estado → ERROR
ERROR → recuperação ou BOOT
```

---

# 3. Driver MAX3010x

## 3.1 O driver não é realmente genérico

O nome sugere suporte à família MAX3010x, mas o código:

- não identifica a variante;
- não possui enum de modelo;
- não registra capacidades;
- não trata LED3;
- assume RED seguido de IR;
- assume seis bytes por amostra;
- configura um modo específico.

Uma estrutura genérica deveria considerar:

```c
typedef enum {
    MAX3010X_VARIANT_UNKNOWN,
    MAX30102_VARIANT,
    MAX30105_VARIANT
} max3010x_variant_t;
```

E armazenar capacidades como:

```text
tem RED?
tem IR?
tem GREEN?
quantos slots ativos?
qual a ordem da FIFO?
qual a resolução?
```

---

## 3.2 `REG_PART_ID` existe, mas não é usado

O código define:

```c
#define REG_PART_ID 0xFF
```

mas não o consulta.

A função `read_reg()` também não é usada no fluxo atual.

---

## 3.3 Reset validado apenas por atraso fixo

Após escrever o bit de reset, o driver espera um tempo fixo.

Ele não verifica se o bit de reset retornou a zero.

Uma abordagem melhor:

```text
escrever reset
consultar MODE_CONFIG
esperar bit reset limpar
aplicar timeout
```

---

## 3.4 Configuração não é verificada por leitura de retorno

Após escrever os registradores, o driver não confirma:

- modo;
- taxa;
- corrente dos LEDs;
- faixa ADC;
- largura de pulso;
- configuração da FIFO.

---

## 3.5 Números mágicos

Valores como:

```c
0x0F
0x27
0x24
0x03
```

não deixam claras as escolhas de configuração.

Seria melhor usar constantes nomeadas:

```c
MAX3010X_ADC_RANGE_4096 |
MAX3010X_SAMPLE_RATE_100 |
MAX3010X_PULSE_WIDTH_411
```

---

## 3.6 Configuração fragmentada

Parte dos parâmetros está em `config_repo`, outra parte em `main.c` e outra no driver.

Parâmetros fixos no driver incluem:

- sample rate;
- ADC range;
- pulse width;
- corrente do LED RED;
- corrente do LED IR;
- média da FIFO;
- rollover.

---

## 3.7 Ausência de tratamento de saturação e luz ambiente

O código não trata adequadamente:

- clipping;
- saturação do ADC;
- luz ambiente;
- flags de interrupção;
- proximidade;
- ajuste automático da corrente dos LEDs.

---

## 3.8 Ausência de funções de controle operacional

Faltam funções como:

```c
max3010x_shutdown()
max3010x_wakeup()
max3010x_flush_fifo()
max3010x_reconfigure()
```

---

# 4. Barramento I²C

## 4.1 Falta validação de argumentos na escrita

`i2c_bus_write()` não valida completamente:

- barramento inicializado;
- endereço;
- `data == NULL` com `len > 0`;
- falha ao criar o comando.

---

## 4.2 Leitura não verifica inicialização

`i2c_bus_read()` valida o buffer de saída, mas não verifica explicitamente se o barramento foi inicializado.

---

## 4.3 Timeout de um segundo é alto

```c
#define SPO2_I2C_TIMEOUT_MS 1000
```

Uma leitura pode bloquear a tarefa por até um segundo.

### Impacto

Pode causar:

- travamento aparente;
- atraso de telemetria;
- perda de amostras;
- quebra da periodicidade.

---

## 4.4 Ausência de mutex

O barramento não possui proteção para acesso concorrente.

No fluxo atual de uma tarefa, isso não é um erro imediato. Porém, será um risco caso outros módulos ou tarefas compartilhem o I²C.

---

## 4.5 `i2c_scan()` quebra a abstração

O controlador chama diretamente APIs do ESP-IDF:

```c
i2c_cmd_link_create()
i2c_master_cmd_begin()
I2C_NUM_0
```

Isso:

- acopla o controlador ao hardware;
- duplica lógica;
- ignora a abstração de `i2c_bus`;
- fixa o uso de `I2C_NUM_0`.

O scanner deveria pertencer à camada de driver.

---

## 4.6 O scan ocorre tarde demais

A sequência atual é aproximadamente:

```text
inicializa I²C
inicializa estrutura do sensor
reseta sensor
configura sensor
executa scan
```

Se o sensor não existir, o sistema pode falhar antes do diagnóstico.

---

# 5. Sampler e temporização

## 5.1 `sample_interval_ms` não é usado

A configuração contém:

```c
.sample_interval_ms = 50
```

Mas o `main.c` usa diretamente:

```c
vTaskDelay(pdMS_TO_TICKS(50));
```

### Impacto

Alterar o repositório de configuração não muda o intervalo real.

---

## 5.2 `vTaskDelay()` causa deriva temporal

O período real é:

```text
tempo de leitura
+ tempo de impressão
+ processamento
+ 50 ms
```

Portanto, o ciclo não é exatamente periódico.

Uma opção melhor seria:

```c
vTaskDelayUntil(...)
```

ou aquisição orientada por data-ready/interrupção.

---

## 5.3 A sequência não garante continuidade real

`seq` incrementa a cada leitura aceita pelo sampler.

Porém, não considera:

- overflow da FIFO;
- amostras perdidas;
- lacunas;
- leituras repetidas;
- diferença entre ponteiros.

Assim:

```text
seq=10 → seq=11
```

não garante que sejam amostras consecutivas produzidas pelo sensor.

---

## 5.4 Timestamp de 32 bits

O timestamp em milissegundos transborda em aproximadamente 49,7 dias.

Não é crítico para uma sessão curta, mas precisa ser considerado nos cálculos de diferença temporal.

---

## 5.5 `g_step_counter` não é usado

Existe uma variável global de contador no controlador que não é incrementada nem consultada.

Isso indica lógica incompleta ou código residual.

---

# 6. Sample buffer

A matemática do buffer circular está correta para uso em uma única tarefa.

## 6.1 Sobrescrita silenciosa

Quando cheio, o buffer substitui a amostra mais antiga sem registrar:

- perda;
- quantidade descartada;
- atraso do consumidor.

---

## 6.2 Ausência de snapshot

Se aquisição e processamento ocorrerem em tarefas diferentes, o conteúdo pode mudar durante a análise.

---

## 6.3 Ausência de sincronização

Não há:

- mutex;
- lock;
- região crítica.

No fluxo atual de uma tarefa isso não é um bug imediato.

---

## 6.4 Ausência de metadados

O buffer não informa:

- taxa nominal;
- taxa efetiva;
- timestamp inicial;
- timestamp final;
- lacunas;
- perdas;
- overflow da FIFO;
- duração da janela.

---

## 6.5 Tamanho não está ligado explicitamente ao tempo

O tamanho deveria representar:

\[
N = f_s \times T
\]

Sem essa relação:

```text
128 amostras a 100 Hz = 1,28 s
128 amostras a 20 Hz = 6,4 s
```

---

# 7. Qualidade do sinal

## 7.1 AC calculado por máximo menos mínimo

```c
ac_ir = max_ir - min_ir;
ac_red = max_red - min_red;
```

Esse cálculo é muito sensível a:

- outliers;
- movimento;
- clipping;
- amostras corrompidas;
- mudança lenta de pressão.

---

## 7.2 `noise = ac_red / dc_ir` não mede ruído

```c
noise = ac_red / dc_ir;
```

Problemas:

- `ac_red` inclui pulsação fisiológica;
- mistura AC de RED com DC de IR;
- não representa ruído residual;
- não possui justificativa clara como SNR.

### Impacto

A nota de ruído e a nota final ficam conceitualmente comprometidas.

---

## 7.3 Índice de perfusão depende de um AC frágil

```c
perfusion_index = ac_ir / dc_ir;
```

A estrutura `AC/DC` é coerente, mas o `AC` usado é vulnerável a outliers.

---

## 7.4 Mínimo de 20 amostras não representa duração fixa

```c
if (count < 20) {
    return false;
}
```

Exemplos:

```text
20 amostras a 20 Hz = 1 s
20 amostras a 100 Hz = 0,2 s
```

A janela deveria ser definida em segundos ou quantidade mínima de ciclos cardíacos.

---

## 7.5 Limiares arbitrários

```c
dc_ir > 1000.0f
ac_ir > 50.0f
perfusion_index * 40.0f
noise * 10.0f
```

Esses valores não estão:

- calibrados;
- centralizados;
- justificados;
- versionados.

---

## 7.6 Pesos arbitrários

```c
quality_score =
    0.6f * score_from_pi +
    0.4f * score_from_noise;
```

Os pesos de 60% e 40% não possuem justificativa experimental apresentada.

---

## 7.7 Uso de todas as amostras disponíveis

A função usa:

```c
count = sample_buffer_count(buffer);
```

Assim, a janela varia enquanto o buffer enche.

---

## 7.8 Ausência de remoção de tendência

Mudanças lentas de contato ou pressão podem ser interpretadas como componente pulsátil.

---

## 7.9 Ausência de análise temporal

A função reduz a janela a:

- média;
- mínimo;
- máximo.

Ela ignora:

- formato da onda;
- periodicidade;
- sequência;
- batimentos;
- frequência dominante.

Dois sinais diferentes podem ter os mesmos mínimo, máximo e média.

---

## 7.10 Ausência de coerência RED/IR

Não verifica:

- alinhamento dos pulsos;
- correlação;
- frequência semelhante;
- deslocamento;
- inversão;
- coerência fisiológica.

---

## 7.11 Ausência de clipping

Não existe rejeição explícita de valores próximos ao limite do ADC.

---

## 7.12 Saída pode manter dados antigos após falha

Se a função retorna `false` antes de preencher `out_quality`, a estrutura do chamador pode conservar dados anteriores.

Uma abordagem mais segura seria zerar ou invalidar a saída no início da função.

---

# 8. Frequência cardíaca e SpO₂

## 8.1 Retorno `false` é ambíguo

Pode significar:

- argumento inválido;
- qualidade insuficiente;
- janela incompleta;
- algoritmo não implementado;
- falha interna;
- resultado fisiológico indisponível.

Uma enumeração de status seria mais clara:

```c
typedef enum {
    ESTIMATOR_OK,
    ESTIMATOR_NOT_READY,
    ESTIMATOR_LOW_QUALITY,
    ESTIMATOR_INVALID_ARGUMENT,
    ESTIMATOR_INTERNAL_ERROR
} estimator_status_t;
```

---

## 8.2 Limiares diferentes e não centralizados

Foram identificados limiares diferentes:

```text
frame: 0.35
HR:    0.45
SpO₂:  0.55
```

Pode haver motivo para usar valores diferentes, mas isso precisa ser documentado e configurável.

---

## 8.3 Não existe calibração

Faltam:

- coeficientes;
- curva de SpO₂;
- versão da calibração;
- faixa válida;
- origem dos dados;
- persistência;
- incerteza.

---

## 8.4 Não existe razão dos quocientes

Ainda falta calcular:

\[
R =
\frac{AC_{RED}/DC_{RED}}
     {AC_{IR}/DC_{IR}}
\]

com AC e DC robustos.

---

## 8.5 Não há controle de domínio

O futuro cálculo deverá tratar:

- divisão por zero;
- `NaN`;
- infinito;
- valores negativos;
- `R` fora da faixa;
- SpO₂ fora do domínio calibrado;
- clipping.

---

# 9. Confidence engine

## 9.1 Não calcula confiança própria

O módulo apenas faz:

```c
out_frame->confidence = quality->quality_score;
```

Portanto, não combina:

- validade do HR;
- validade da SpO₂;
- estabilidade;
- ruído;
- movimento;
- overflow;
- coerência RED/IR;
- idade dos dados.

---

## 9.2 `frame.valid` ignora HR e SpO₂

A regra atual:

```c
out_frame->valid =
    quality->signal_present &&
    quality->quality_score >= 0.35f;
```

permite:

```text
frame.valid = true
hr.valid = false
spo2.valid = false
```

---

## 9.3 Limiar fixo e arbitrário

O valor `0.35f` é um número mágico e não está centralizado.

---

## 9.4 O módulo não é chamado

No fluxo atual, nenhum `health_frame_t` é produzido operacionalmente.

---

## 9.5 `finger_present` apenas copia `signal_present`

```c
out_frame->finger_present = quality->signal_present;
```

Mas sinal óptico não é necessariamente equivalente a dedo corretamente posicionado.

Objetos refletivos, luz externa ou movimento podem produzir sinal.

---

## 9.6 Mistura construção de frame e política de segurança

A função:

- copia dados;
- calcula validade;
- define presença de dedo;
- define confiança.

Essas responsabilidades poderiam ser separadas em:

```text
frame_builder
confidence_evaluator
validation_policy
```

---

# 10. Tipos e modelo de dados

## 10.1 `health_frame_t.valid` é ambíguo

Pode significar:

- frame construído;
- sinal válido;
- qualidade suficiente;
- HR válida;
- SpO₂ válida;
- resultado completo válido.

Seria melhor separar:

```c
bool signal_valid;
bool frame_complete;
```

mantendo também:

```c
hr.valid;
spo2.valid;
```

---

## 10.2 Falta motivo de invalidade

O consumidor não sabe se o resultado está inválido por:

- dedo ausente;
- movimento;
- baixa perfusão;
- janela curta;
- overflow;
- erro I²C;
- sensor desconectado;
- valor fora da curva;
- calibração ausente.

---

## 10.3 Falta idade da estimativa

O frame não informa quando HR e SpO₂ foram atualizadas pela última vez.

---

## 10.4 Falta incerteza

Não existem campos para:

- intervalo de confiança;
- erro estimado;
- variação entre janelas;
- estabilidade.

---

## 10.5 Falta rastreabilidade da configuração

O resultado não registra:

- taxa;
- corrente dos LEDs;
- faixa ADC;
- largura de pulso;
- versão do algoritmo;
- versão da calibração.

---

# 11. Telemetria

## 11.1 Apenas RAW é usado atualmente

A função de frame existe, mas não participa do fluxo executado.

---

## 11.2 `printf` em toda amostra interfere na temporização

A impressão serial pode:

- bloquear;
- aumentar jitter;
- reduzir a taxa efetiva;
- alterar a coleta;
- causar acúmulo adicional na FIFO.

---

## 11.3 Formato sem versionamento

Faltam:

- versão do protocolo;
- tipo formal de frame;
- CRC ou checksum;
- contador de pacote;
- timestamp do frame;
- unidades explícitas.

---

## 11.4 Logs e dados compartilham o mesmo canal

Mensagens como:

```text
[RAW]
[FRAME]
[APP]
```

são enviadas pela mesma serial.

Isso dificulta parsing automático robusto.

---

## 11.5 `state_str` pode divergir do estado verdadeiro

A função de telemetria aceita qualquer texto:

```c
const char *state_str
```

Seria mais seguro receber `app_state_t` e converter internamente.

---

# 12. Configuração e armazenamento

## 12.1 `config_repo` não é persistente

O módulo apenas expõe uma estrutura constante compilada no firmware.

Ele não:

- usa NVS;
- salva alterações;
- carrega calibração;
- persiste parâmetros.

---

## 12.2 Configuração parcialmente ignorada

`sample_interval_ms` não é usado na temporização real.

---

## 12.3 Configuração incompleta

Faltam parâmetros como:

- taxa do sensor;
- pulse width;
- ADC range;
- corrente RED;
- corrente IR;
- tamanho da janela;
- limiares;
- coeficientes de calibração;
- versão da calibração.

---

## 12.4 Não existe persistência de calibração

Não há implementação visível para armazenar ou recuperar parâmetros de calibração.

---

# 13. Inicialização e segurança

## 13.1 `ESP_ERROR_CHECK` causa falha abrupta

Durante a inicialização, erros podem provocar aborto ou reinicialização imediata.

Para um sistema integrado, seria melhor:

- registrar o erro;
- entrar em estado seguro;
- informar o receptor;
- tentar recuperação limitada;
- impedir uso de dados antigos.

---

## 13.2 Não há desligamento seguro dos LEDs

Em caso de erro, não existe um caminho explícito para colocar o sensor em shutdown.

---

## 13.3 Falta watchdog lógico de aquisição

Não há detecção de:

- sinal congelado;
- amostras repetidas;
- ausência de atualização;
- atraso excessivo;
- overflow recorrente;
- barramento travado.

---

## 13.4 A camada `safety` ainda não oferece política de segurança

Atualmente, a camada contém apenas o `confidence_engine`.

Ainda faltam:

- debounce;
- histerese;
- persistência temporal;
- confirmação por múltiplos frames;
- diferenciação entre perda de contato e dessaturação;
- fail-safe;
- pausa segura da sessão.

---

# 14. Engenharia e manutenção

## 14.1 Ausência de testes automatizados

Não foram identificados testes unitários claros para:

- buffer circular;
- estados;
- qualidade;
- FIFO;
- timestamps;
- cálculo de HR;
- cálculo de SpO₂;
- overflow.

---

## 14.2 Diretório `build` versionado

Artefatos de build normalmente deveriam ser ignorados pelo Git.

Problemas:

- aumentam o repositório;
- podem conter caminhos locais;
- ficam obsoletos;
- geram alterações desnecessárias.

---

## 14.3 Dependência direta da API I²C no controlador

Mesmo existindo `i2c_bus`, o controlador ainda usa a API do ESP-IDF diretamente.

---

## 14.4 Código não utilizado

Foram identificados elementos sem uso no fluxo atual, como:

```text
g_step_counter
read_reg()
REG_PART_ID
REG_INTR_STATUS_1
REG_INTR_STATUS_2
```

---

# 15. Pontos que não são erros imediatos

## 15.1 Buffer circular

A lógica de:

- `head`;
- `count`;
- `latest`;
- `oldest`;
- acesso cronológico;

está coerente para uma única tarefa.

As limitações estão em:

- concorrência;
- metadados;
- sobrescrita silenciosa;
- janela temporal.

---

## 15.2 Sequência incrementada antes do `push`

Existe uma inconsistência teórica:

```c
sample.seq = ++sampler->seq;
return sample_buffer_push(...);
```

Porém, no código atual o `push` só falha por argumentos inválidos, previamente verificados.

É uma questão de baixa prioridade.

---

## 15.3 Concorrência

A ausência de mutex ainda não causa falha no fluxo atual, pois a aquisição ocorre em uma única tarefa.

Será um risco quando:

- coleta;
- processamento;
- telemetria;
- outros sensores;

forem executados em tarefas diferentes.

---

# 16. Ordem recomendada de correção

## Fase 1 — Corrigir aquisição

1. identificar a variante do sensor;
2. ler Part ID e Revision ID;
3. configurar taxa explicitamente;
4. ler ponteiros da FIFO;
5. detectar quantidade disponível;
6. ler amostras em lote;
7. detectar overflow;
8. reconstruir timestamps;
9. alinhar taxa do sensor e taxa de consumo;
10. retirar `printf` do caminho crítico.

---

## Fase 2 — Integrar o pipeline

```text
sampler
→ buffer
→ signal_quality
→ hr_estimator
→ spo2_estimator
→ confidence_engine
→ health_frame
→ telemetria
```

---

## Fase 3 — Refazer a avaliação de qualidade

Implementar:

- janela fixa em segundos;
- remoção de DC/tendência;
- filtragem;
- AC robusto;
- ruído residual real;
- análise de clipping;
- correlação RED/IR;
- regularidade temporal;
- movimento;
- estabilidade entre janelas.

---

## Fase 4 — Implementar SpO₂

Implementar:

- razão dos quocientes;
- curva de calibração versionada;
- faixa válida;
- rejeição de valores fora do domínio;
- suavização temporal;
- comparação com referência;
- documentação como estimativa não clínica até validação adequada.

---

## Fase 5 — Implementar segurança e estados

Implementar:

- máquina de estados baseada em eventos;
- regras formais de transição;
- recuperação de erro;
- histerese;
- debounce;
- confirmação temporal;
- motivo de invalidade;
- fail-safe;
- diferenciação entre perda de contato e possível dessaturação.

---

# 17. Prioridades principais

As três prioridades mais urgentes são:

1. corrigir o consumo da FIFO e a temporização;
2. conectar o pipeline de processamento ao controlador;
3. substituir as heurísticas frágeis de qualidade antes de confiar na estimativa de SpO₂.

---

# 18. Conclusão

A arquitetura de pastas está bem encaminhada e apresenta uma separação inicial de responsabilidades.

Pontos positivos:

- módulos separados;
- interfaces `.h`;
- driver isolado;
- sampler separado;
- buffer circular;
- tipos organizados;
- preparação para qualidade, HR, SpO₂ e confiança.

Entretanto, o firmware atual ainda deve ser descrito como:

> um coletor e transmissor serial de amostras ópticas RED e IR.

Ele ainda não deve ser descrito como oxímetro funcional porque:

- HR não é calculada;
- SpO₂ não é calculada;
- qualidade e confiança são heurísticas preliminares;
- os módulos não estão integrados;
- FIFO e temporização ainda estão inconsistentes;
- não há calibração;
- não há validação experimental ou clínica.

A base arquitetural é aproveitável, mas a aquisição precisa ser corrigida antes da implementação matemática do oxímetro.
