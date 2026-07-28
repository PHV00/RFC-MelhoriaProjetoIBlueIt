# Estrutura atual do firmware

Baseline: commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0`, auditado estaticamente.

```text
main.c
└── app/
    ├── app_controller: orquestra inicialização, aquisição, processamento e recuperação
    ├── app_state_machine: valida transições de estado
    └── app_types: contratos de amostra, qualidade, HR, SpO₂ e health_frame
        ↓
drivers/
├── i2c_bus: I²C ESP-IDF com mutex e timeout
└── max3010x_driver: IDs, reset, configuração, readback e FIFO
        ↓
sensing/
├── ppg_sampler: drenagem em lote, sequência e timestamps
└── sample_buffer: buffer circular RED/IR
        ↓
processing/
├── signal_quality: tendência, AC/DC, ruído, SNR, perfusão, correlação e clipping
├── hr_estimator: picos, IBI e estabilidade
└── spo2_estimator: razão dos quocientes e curva experimental
        ↓
safety/
└── confidence_engine: frame, confiança, validade técnica e validade clínica
        ↓
transport/
└── serial_telemetry: sensor_info, eventos e frame JSON parcial

storage/
└── config_repo: configuração constante compilada; sem persistência atual
```

## Estado evolutivo

- O alvo é MAX30102/ESP32-C3, mas a seleção explícita por variante ainda está pendente.
- O pipeline de qualidade, HR e SpO₂ está integrado ao controlador.
- A curva de SpO₂ está marcada como não calibrada e serve somente à prova de conceito.
- A telemetria RAW está desativada e o frame ativo ainda omite metadados críticos.
- Não foram localizados testes automatizados ou HIL versionados.
- Integrações com Unity, DeepDDA, banco e dashboard não pertencem ao código presente neste repositório.

Essas pendências descrevem apenas o baseline auditado e devem ser atualizadas conforme forem corrigidas e testadas.
