# Firmware PPG/HR/SpO₂ — arquitetura e estado atual

## Escopo e baseline

Este documento descreve o código do commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0`, auditado estaticamente em 2026-07-15. O alvo de produto é um **MAX30102 conectado a um ESP32-C3**, preservando uma abstração de driver `MAX3010x`.

A presença de código não demonstra compilação, funcionamento em hardware, exatidão metrológica, segurança clínica ou eficácia. As pendências descritas abaixo são backlog evolutivo e devem ser reavaliadas quando houver correções e testes.

## Arquitetura implementada

```text
MAX3010x / I²C
      ↓
drivers/
  barramento, identificação, registradores, configuração e FIFO
      ↓
sensing/
  drenagem em lote, sequência, timestamps e buffer circular
      ↓
processing/
  qualidade → HR → estimativa experimental de SpO₂
      ↓
safety/
  composição de confiança e validade do health_frame
      ↓
transport/
  sensor_info, eventos técnicos e frame serial JSON
      ↓
app/
  inicialização, estados, orquestração e recuperação

storage/
  configuração compilada; ainda não há persistência
```

O código está organizado em C com ESP-IDF, FreeRTOS e CMake. O alvo versionado em `sdkconfig` é `esp32c3`. Apesar de o nome do projeto CMake ser `sp02_ble`, não há transporte BLE implementado neste baseline.

## Fluxo de inicialização

```text
app_main
  → app_controller_init
  → BOOT / SELF_TEST
  → i2c_bus_init
  → max3010x_init
  → max3010x_probe: lê PART_ID e REV_ID
  → max3010x_reset
  → max3010x_configure: FIFO, taxa, ADC, pulso e LEDs
  → readback dos registradores
  → sample_buffer_init / ppg_sampler_init
  → telemetria sensor_info
  → IDLE
```

O probe atual considera identificado qualquer `PART_ID` diferente de `0x00` e `0xFF`. Portanto, ele registra a identidade observada, mas ainda não comprova nem seleciona explicitamente a variante MAX30102. Essa é uma pendência de SR-F-002.

## Fluxo de aquisição e processamento

```text
app_controller_step
  → ppg_sampler_poll
     → consulta FIFO_WR_PTR, FIFO_RD_PTR e OVF_COUNTER
     → detecta overflow e limpa a FIFO
     → drena as amostras disponíveis em lote
     → reconstrói timestamps pela taxa nominal
     → insere RED/IR no buffer circular
  → a cada janela configurada:
     → signal_quality_evaluate_window
     → hr_estimator_compute
     → spo2_estimator_compute_with_calibration
     → confidence_engine_build_frame_ex
     → TRACKING ou LOW_CONFIDENCE
     → serial_telemetry_print_frame
```

Configuração atual relevante:

| Parâmetro | Valor no baseline | Interpretação |
|---|---:|---|
| taxa nominal | 100 Hz | precisa ser confirmada por medição efetiva |
| janela | 400 amostras | aproximadamente 4 s na taxa nominal |
| intervalo de processamento | 500 ms | janelas sobrepostas |
| polling configurado | 10 ms | ainda não é usado pelo loop principal |
| atraso do loop principal | 50 ms | valor fixo em `main.c` |
| qualidade mínima do frame | 0,55 | heurística ainda não validada |
| curva SpO₂ | `110 - 25R` | apenas demonstração de engenharia |
| calibração | `false`, versão 0 | não validada para o conjunto óptico final |
| estimativa não calibrada | permitida | somente para POC, sem finalidade clínica |

## Qualidade, HR e SpO₂

A qualidade usa remoção de tendência, AC RMS, ruído residual, SNR, índice de perfusão, correlação RED/IR, continuidade e clipping. Esses componentes melhoram a estrutura anterior, mas seus limiares e pesos ainda precisam ser relacionados ao erro real por dataset rotulado e referência.

HR é um baseline interpretável baseado em tendência, média móvel, detecção de picos, IBI mediano e estabilidade. SpO₂ usa a razão dos quocientes:

```text
R = (AC_RED / DC_RED) / (AC_IR / DC_IR)
SpO₂ = aR² + bR + c
```

O resultado interno separa:

- `spo2.valid`: valor numérico utilizável apenas na prova de conceito;
- `spo2.calibrated`: existência de curva validada para o conjunto final;
- `health_frame.valid`: sinal, HR e SpO₂ numericamente aceitos pela política técnica atual;
- `health_frame.clinical_valid`: exige também calibração.

No baseline, `clinical_valid` permanece falso porque a curva não é calibrada.

## Discrepância atual da telemetria

O modelo interno é mais completo que o frame serial ativo. A telemetria atual envia `valid`, qualidade, HR e SpO₂, mas omite:

- `clinical_valid`;
- `invalid_reasons`;
- overflow da FIFO;
- status dos estimadores;
- `calibrated` e versão de calibração;
- versão do firmware/configuração;
- paciente, sessão, dispositivo lógico e sequência do frame.

A função de telemetria RAW existe, mas seu `printf` está comentado. Portanto, RED/IR não são atualmente preservados ou exportados por esse caminho. Completar e testar esse contrato é prioridade antes de qualquer integração que possa interpretar `valid` como aptidão clínica.

## Estados e recuperação

Os estados são `BOOT`, `SELF_TEST`, `IDLE`, `SAMPLING`, `TRACKING`, `LOW_CONFIDENCE` e `ERROR`, com transições validadas. Após três falhas consecutivas de aquisição, o controlador entra em erro e tenta reconfigurar o sensor periodicamente.

Esse mecanismo é um baseline de recuperação. Ainda precisa de testes de desconexão, reconexão, FIFO, sinal congelado, LEDs, watchdog e comunicação do estado ao sistema de sessão.

## Testes atuais

Não foram localizados fontes, diretórios ou relatórios de testes automatizados/HIL neste repositório. O protocolo especifica TST-001 a TST-012, mas esses IDs representam testes planejados até que código, ambiente, execução e resultados sejam anexados.

## Próximas evoluções técnicas

1. completar o autoteste e a seleção explícita da variante MAX30102;
2. alinhar o período de polling à configuração e medir taxa efetiva, FIFO, gaps e timestamps;
3. preservar RED/IR e metadados para replay e validação;
4. completar o contrato de telemetria, especialmente validade clínica e motivos de invalidade;
5. adicionar testes unitários, replay determinístico e HIL;
6. calibrar e comparar HR/SpO₂ contra referência com critérios pré-especificados;
7. somente depois integrar política de segurança, terapeuta, DeepDDA, persistência e dashboard.

Esses itens representam a ordem segura sugerida no baseline atual; podem ser replanejados quando novas evidências e decisões arquiteturais forem registradas.
