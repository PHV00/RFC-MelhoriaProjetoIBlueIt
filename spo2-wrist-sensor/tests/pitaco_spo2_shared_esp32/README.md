# Teste integrado PITACO + PPG no mesmo ESP32-C3

Este projeto de teste fica deliberadamente em `spo2-wrist-sensor/tests/` (ESP-IDF),
sem alterar o firmware Arduino e sem alterar o firmware principal.

## Objetivo

Provar, isoladamente, que o mesmo ESP32-C3 consegue manter ao mesmo tempo:

- leitura analógica do XGZP6897A no ADC1/GPIO0;
- leitura RED/IR do MAX3010x/MAX30102 usando o driver já existente no repositório;
- polling do FIFO do PPG a 100 Hz (configuração default do driver);
- amostragem/telemetria diagnóstica do pneumotacógrafo a cada 50 ms.

O teste reaproveita diretamente:

- `main/drivers/i2c_bus.c`;
- `main/drivers/max3010x_driver.c`;
- a abordagem de ADC já usada em `tests/basic_pneumo`.

## Fiação

### XGZP6897A

```text
XGZP OUT ---- R1 2,17 kΩ ----+---- GPIO0 / ADC1
                              |
                           R2 1,50 kΩ
                              |
                             GND
```

- VDD: 5 V / VBUS;
- GND: GND comum;
- R1 medido: aproximadamente 2,17 kΩ;
- R2 medido: aproximadamente 1,50 kΩ.

### MAX3010x/MAX30102

- SDA -> GPIO6;
- SCL -> GPIO7;
- GND -> GND comum;
- alimentação conforme a breakout usada no projeto.

## Por que o teste não calcula Pa

A faixa exata da unidade XGZP6897A disponível ainda não foi confirmada.
Por isso o teste não assume 0..10 kPa nem -5..+5 kPa.

Ele reporta:

- ADC RAW;
- tensão calibrada no GPIO0;
- tensão reconstruída no OUT do XGZP;
- DELTA em mV relativo ao zero obtido no boot.

Isso permite validar a integração elétrica antes de congelar a função tensão -> pressão.

## Build / flash

A partir desta pasta:

```bash
idf.py set-target esp32c3
idf.py build
idf.py flash monitor
```

## Saída

O monitor imprime CSV:

```text
ms,adc_raw,adc_mv,xgzp_mv,delta_mv,red,ir,ppg_samples,fifo_overflows,ppg_read_errors,ppg_ready
```

Critérios mínimos para considerar a PoC de coexistência aprovada:

1. `delta_mv` responde ao pneumotacógrafo sem travar o loop;
2. RED e IR mudam com o dedo sobre o MAX3010x;
3. `ppg_samples` cresce continuamente;
4. `fifo_overflows` permanece em zero em operação normal;
5. não aparecem falhas I2C recorrentes;
6. as duas aquisições continuam ativas simultaneamente.

## Escopo

Este teste valida coexistência de aquisição no mesmo ESP32-C3. Ele não valida
clinicamente pressão, fluxo, frequência cardíaca ou SpO2.

Também não executa G2 nesta etapa: em `spo2-wrist-sensor` os arquivos
`processing/sqi/gates/g2_pulsatility/` ainda estão vazios nesta branch. O G2
diagnóstico atualmente implementado está no baseline Arduino, portanto deve ser
integrado ao ESP-IDF em uma etapa separada para não misturar validação de hardware
com portabilidade do gate.
