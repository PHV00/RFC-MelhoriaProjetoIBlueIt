[SENSOR MAX3010x]
       |
       v
[drivers/]
  le hardware bruto
       |
       v
[sensing/]
  amostra + buffer + contato
       |
       v
[processing/]
  filtro + qualidade + FC + SpO2
       |
       v
[safety/]
  confiança + limiares + fail-safe
       |
       +------------------+
       |                  |
       v                  v
[transport/]          [storage/]
 BLE / Serial         config / calibração / logs
       |
       v
[app/] 
 orquestra estados e decisões do sistema

Sensor físico
    ↓
drivers
    ↓
sensing
    ↓
processing
    ↓
safety
    ↓
transport
    ↓
I Blue It / computador

////////////////////////////////////////////////

MAX30105
   ↓
i2c_bus
   ↓
max3010x_driver
   ↓
ppg_sampler
   ↓
sample_buffer
   ↓
serial_telemetry

//////////////////////////////////////////////////

main.c
└── chama app_controller_init()
    ├── config_repo_get()
    │   └── retorna pinos, frequência e endereço
    │
    ├── app_state_machine_set(BOOT)
    │
    ├── i2c_bus_init()
    │   └── configura o I²C do ESP32
    │
    ├── max3010x_init()
    │   └── cria representação do sensor
    │
    ├── max3010x_reset()
    │   └── reinicia o sensor
    │
    ├── max3010x_config_default()
    │   └── configura registradores
    │
    ├── sample_buffer_init()
    │   └── limpa o buffer
    │
    ├── ppg_sampler_init()
    │   ├── recebe endereço de g_sensor
    │   └── recebe endereço de g_buffer
    │
    └── app_state_machine_set(IDLE)

Durante a execução:

main.c
└── app_controller_step()
    ├── ppg_sampler_step()
    │   ├── max3010x_driver lê o sensor
    │   │   └── i2c_bus acessa o hardware
    │   │
    │   ├── cria ppg_sample_t
    │   └── sample_buffer insere a amostra
    │
    ├── sample_buffer_latest()
    │   └── devolve última amostra
    │
    └── serial_telemetry_print_sample()
        └── imprime RED e IR

drivers
    pegam a matéria-prima do hardware

sensing
    coleta e organiza a matéria-prima

processing
    transforma matéria-prima em informação

safety
    verifica se a informação é confiável

transport
    envia a informação

storage
    mantém configurações e, futuramente, dados

app_controller
    manda todas as áreas trabalharem na ordem certa


fluxo completo:

main.c
  ↓
app_controller
  ↓
ppg_sampler
  ↓
sample_buffer
  ↓
janela possui amostras suficientes?
  ├── não → continuar coletando
  └── sim
       ↓
signal_quality
       ↓
sinal utilizável?
  ├── não → LOW_CONFIDENCE
  └── sim
       ↓
hr_estimator
       ↓
spo2_estimator
       ↓
confidence_engine
       ↓
health_frame_t
       ↓
serial_telemetry
       ↓
PITACO / Unity / I Blue It / dashboard

