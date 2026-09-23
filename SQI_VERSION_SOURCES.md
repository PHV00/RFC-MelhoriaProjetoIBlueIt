# SQI — versões de referência e fontes preservadas

Este documento registra as versões de referência do SQI que devem ser preservadas para desenvolvimento futuro, mesmo após a limpeza das branches de trabalho.

## Objetivo

Evitar perda de conhecimento entre a implementação histórica em ESP32 e a nova implementação em Arduino Uno, mantendo pontos de referência claros para:

- arquitetura dos Gates;
- algoritmos e features;
- testes e fault injection;
- decisões de memória;
- documentação científica;
- migração incremental para o firmware conjunto PITACO + MAX30102.

---

## V1 — SQI ESP32

Branch arquivada:

```text
archive/sqi-v1-esp32-g1-g2
```

Commit preservado:

```text
d1ef31fcaad64587095de603740dd156e7c9d012
```

Origem:

```text
feature/sqi-g2-pulsatility
```

### O que a V1 preserva

- arquitetura modular do SQI;
- G1 de integridade com fail-fast;
- G2 de pulsatilidade;
- preprocessamento Butterworth high-pass;
- threshold crossings;
- autocorrelação;
- FZCP;
- busca de pico ACF;
- configuração do Gate 2;
- telemetria;
- testes host;
- testes de resposta em frequência;
- testes de equivalência da ACF;
- documentação científica/técnica do G2.

### Como deve ser usada

A V1 é **referência algorítmica e científica**, não código para copiar integralmente ao Arduino Uno.

Ao portar uma feature da V1 para o Uno, revisar obrigatoriamente:

1. custo de SRAM;
2. custo de CPU;
3. tamanho das janelas;
4. tipos numéricos;
5. scratch buffers;
6. dependências de ESP-IDF/FreeRTOS;
7. thresholds provisórios;
8. compatibilidade com a taxa efetiva de amostragem do MAX30102.

---

## V1 — harness de fault injection do G1

Branch arquivada:

```text
archive/sqi-v1-g1-fault-injection
```

Commit preservado:

```text
411a5e653de7ddb2ecf7fb1bcb24abf31ac86de2
```

Origem histórica:

```text
archive/sqi-g1-fault-injection
```

### Finalidade

Preservar os cenários sintéticos usados para validar:

- flatline;
- clipping RED;
- clipping IR;
- descontinuidade;
- fail-fast;
- bloqueio de HR/SpO2 quando G1 falha.

Esse código não faz parte da linha de produção, mas deve permanecer disponível para regressão e reprodução dos testes.

---

## V2 — SQI Arduino Uno

Branch arquivada:

```text
archive/sqi-v2-arduino-g1-18bit
```

Commit preservado:

```text
d4615aa1b05f70a949fd27eccab07abed3454dd8
```

Branch de desenvolvimento ativa no momento da criação deste documento:

```text
feature/arduino-sqi-gate1
```

### O que a V2 preserva

- port do MAX30102 para Arduino Uno;
- configuração explícita do sensor;
- arquitetura central + Gate 01;
- G1 incremental/streaming;
- uso temporário de `uint32_t` para a amostra RAW;
- decisão de não manter grandes buffers `uint32_t[100]`;
- armazenamento lossless de amostras de 18 bits em 3 bytes;
- separação entre validação do G1 e teste de armazenamento;
- documentação da decisão de memória.

### Configuração de aquisição de referência

```text
sampleAverage = 4
ledMode       = 2
sampleRate    = 100
pulseWidth    = 411 us
adcRange      = 4096
```

### Regra de desenvolvimento

A V2 é a base de destino para o Uno.

O desenvolvimento deve seguir:

```text
MAX30102 RAW
    |
    v
G1 — Integridade
    |
    v
G2 — Pulsatilidade
    |
    v
G3 — Morfologia
    |
    v
G4 — Coerência RED/IR
    |
    v
PPG VALID
    |
    v
FC / SpO2
```

Cada Gate deve ser portado separadamente e validado antes do próximo.

---

## Fonte de bring-up do MAX30102 no Uno

Commit importante:

```text
330a73cc33575bcc05069fcce76a4e7315442c0d
```

Esse commit contém o teste RAW mínimo usado para validar:

- conexão I2C;
- endereço 0x57;
- RED;
- IR;
- resposta ao colocar/remover o dedo.

Deve ser mantido como referência para separar problemas de hardware de problemas do SQI.

---

## Relação entre V1 e V2

```text
V1 — ESP32
algoritmos mais completos
G1 + G2
testes e documentação
        |
        | selecionar conceito/feature
        v
análise de custo SRAM/CPU
        |
        v
adaptação
        |
        v
V2 — Arduino Uno
G1 -> G2 -> G3 -> G4
        |
        v
PITACO + MAX30102
```

A V1 responde principalmente:

> Como o SQI foi estruturado e quais algoritmos já foram investigados/implementados?

A V2 responde:

> Como implementar a mesma finalidade dentro das restrições reais do ATmega328P?

---

## Política para futuras branches

Branches de trabalho devem ser curtas e associadas a uma etapa:

```text
feature/arduino-sqi-gate1
feature/arduino-sqi-gate2
feature/arduino-sqi-gate3
feature/arduino-sqi-gate4
feature/arduino-spo2
feature/pitaco-ppg-integration
```

Após merge:

1. preservar somente um snapshot se houver valor histórico/técnico;
2. registrar o commit neste documento;
3. remover a branch de trabalho;
4. iniciar a próxima feature a partir da `main`.

---

## Branches antigas que não são fontes

As branches abaixo são intermediárias e podem ser removidas quando a limpeza for executada:

```text
docs/sqi-architecture-status
docs/sqi-artigo-tecnico-cientifico
feature/sqi-g1-integrity
fix/remove-esp32-duplication
refactor/sqi-scaffold
test/sqi-g1-final-validation
feature/spo2-arduino-port
safety-module
```

A branch original `feature/sqi-g2-pulsatility` também pode ser removida depois da criação de `archive/sqi-v1-esp32-g1-g2`.

A branch original `archive/sqi-g1-fault-injection` pode ser removida depois da criação de `archive/sqi-v1-g1-fault-injection`.

---

## Estado esperado após a limpeza

Branches ativas:

```text
main
feature/arduino-sqi-gate1
```

Branches de arquivo:

```text
archive/sqi-v1-esp32-g1-g2
archive/sqi-v1-g1-fault-injection
archive/sqi-v2-arduino-g1-18bit
```

Depois que a V2/G1 for mergeada, `feature/arduino-sqi-gate1` também deve ser removida e a próxima etapa deve nascer da `main`.
