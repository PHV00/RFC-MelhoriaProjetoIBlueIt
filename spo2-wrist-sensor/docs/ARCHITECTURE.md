# Arquitetura do firmware

## Objetivo

Separar aquisição, processamento de sinais, avaliação de qualidade, estimação fisiológica, segurança e transporte, evitando acoplamento entre hardware e algoritmos científicos.

## Camadas

### `drivers/`
Acesso direto ao hardware e barramento. Deve conhecer registradores, I²C e particularidades do MAX3010x, mas não deve conhecer SQI, SpO₂, FC ou regras do jogo.

### `sensing/`
Coleta e organiza amostras. Contém `ppg_sampler` e `sample_buffer`. É responsável por produzir uma sequência temporal consistente de `RED`, `IR` e timestamp.

### `processing/`
Transforma o PPG em informação técnica e fisiológica. A ordem lógica é:

```text
RAW → SQI → FC/SpO₂
```

O SQI deve aprovar a janela antes dos estimadores fisiológicos.

### `safety/`
Recebe resultados já avaliados e decide o estado de confiança/fail-safe do sistema. Não deve reimplementar o SQI.

### `storage/`
Mantém configurações, parâmetros de calibração, versões de configuração e, futuramente, logs persistentes.

### `transport/`
Serial/BLE e demais protocolos. Não deve recalcular métricas.

### `app/`
Orquestra as camadas. Não deve conter fórmulas de SQI, filtros, autocorrelação ou regras fisiológicas detalhadas.

## Fluxo de execução alvo

```text
app_controller_step()
    ↓
ppg_sampler_step()
    ↓
sample_buffer
    ↓
janela suficiente?
    ├─ não → aguardar
    └─ sim
         ↓
      signal_quality_evaluate_window()
         ├─ WAITING → retornar
         ├─ ERROR   → fail-safe/log
         ├─ INVALID → não chamar estimadores
         └─ VALID
              ↓
           hr_estimator
              ↓
           spo2_estimator
              ↓
           confidence_engine
              ↓
           health_frame_t
              ↓
           telemetry
```

## Dependências

A camada `processing/` não deve depender de `app/`. Tipos compartilhados devem migrar para uma área comum, por exemplo:

```text
main/common/measurement_types.h
```

ou equivalente.

O objetivo é impedir a direção de dependência indesejada:

```text
processing → app
```

A dependência correta é:

```text
app → sensing/processing/safety/transport
```
