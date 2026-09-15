# spo2-wrist-sensor-arduino

Porta inicial do módulo de oximetria do I Blue It para **Arduino Nano R3 (ATmega328P)** usando o sensor **MAX30102**.

## Objetivo desta branch

Esta implementação é uma etapa de teste de hardware e compatibilidade com o contrato histórico do I Blue It. Ela não substitui ainda a arquitetura SQI hierárquica implementada no firmware ESP32; o módulo ESP32 foi preservado separadamente em `spo2-wrist-sensor-esp32`.

## Pinagem — Arduino Nano R3

| MAX30102 | Arduino Nano R3 | Função |
|---|---|---|
| SDA | A4 | I2C dados |
| SCL | A5 | I2C clock |
| GND | GND | terra comum |
| VIN/VCC | conforme o breakout | alimentação |
| INT | não conectado | não utilizado nesta versão |

O LED onboard D13 é usado como indicador de leitura de amostra.

> Atenção: breakouts MAX30102 variam. Não assuma que qualquer placa aceita 5 V diretamente em VCC ou níveis lógicos de 5 V no I2C. Confirme o modelo do breakout antes da montagem definitiva.

## Biblioteca

Instale pela Arduino Library Manager:

`SparkFun MAX3010x Pulse and Proximity Sensor Library`

O sketch usa:

- `MAX30105.h`
- `heartRate.h`
- `spo2_algorithm.h`

A biblioteca `MAX30105` da SparkFun também suporta o MAX30102.

## Contrato serial mantido

Baud rate: `115200`

| Comando | Resposta/efeito |
|---|---|
| `E` / `e` | responde `echox` |
| `R` / `r` | inicia aquisição |
| `F` / `f` | interrompe aquisição |

Durante aquisição válida, a saída permanece:

```text
FC,SpO2
```

Exemplo:

```text
72,97
```

Quando a janela não produz valor válido:

```text
0,0
```

## Arquivo principal

Abra no Arduino IDE/VS Code:

`spo2-wrist-sensor-arduino.ino`

## Observação sobre o LED do MAX30102

O canal infravermelho é invisível ao olho humano e o LED vermelho é pulsado. Além disso, este firmware mantém a amplitude vermelha baixa (`0x0A`) como no código de referência do I Blue It. Portanto, não enxergar um LED continuamente aceso não significa, por si só, que o sensor não está funcionando.

O teste elétrico recomendado é verificar se o MAX30102 responde no barramento I2C (endereço típico `0x57`) antes de diagnosticar o algoritmo de FC/SpO2.

## Relação com `spo2-wrist-sensor-esp32`

Nesta branch:

```text
spo2-wrist-sensor-esp32/
    firmware atual ESP32 preservado

spo2-wrist-sensor-arduino/
    adaptação inicial Arduino Nano/MAX30102
```

A validação Arduino deve ocorrer primeiro em bancada. Depois disso, os Gates/SQI do firmware ESP32 podem ser portados de forma incremental, respeitando as limitações de SRAM e processamento do ATmega328P.
