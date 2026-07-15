# Plano de validação da prova de conceito de HR e SpO₂

## 1. Objetivo

Demonstrar, em etapas separadas, que:

1. o firmware adquire PPG sem perdas silenciosas;
2. a qualidade rejeita sinal ausente, descontínuo, saturado ou incoerente;
3. HR é calculada de forma repetível dentro do domínio proposto;
4. `R` é calculado corretamente;
5. uma curva de SpO₂ pode ser aplicada e rastreada;
6. a curva final é calibrada e validada antes de qualquer uso de segurança;
7. o `health_frame` pode ser integrado ao I Blue It sem misturar validade técnica e decisão terapêutica.

## 2. Níveis de evidência

### Nível A — Teste de software

Já iniciado neste pacote.

Casos mínimos adicionais:

- HR sintética em 45, 60, 90, 120, 160 e 190 bpm;
- jitter de timestamp;
- perda de amostras;
- clipping parcial e total;
- baixa perfusão;
- RED/IR descorrelacionados;
- drift de DC;
- interferência de 50/60 Hz e ruído impulsivo;
- `R` nos limites e fora do domínio;
- wrap-around de timestamp.

Critério: cada caso deve retornar valor dentro da tolerância esperada ou motivo de invalidade correto.

### Nível B — Compilação e testes do ESP-IDF

- compilação limpa com a versão de ESP-IDF adotada pelo projeto;
- análise de warnings;
- teste unitário dos módulos independentes;
- mocks do I²C e registradores da FIFO;
- teste da máquina de estados e recuperação.

Critério: build sem erro e sem warning novo; cobertura dos fluxos de erro críticos.

### Nível C — Hardware-in-the-loop

Usar o módulo CJMCU-30105 identificado no projeto e o ESP32-C3.

Registrar:

- `PART_ID`, `REV_ID` e configuração lida de volta;
- amostras RED/IR brutas;
- ponteiros e overflow da FIFO;
- timestamps, sequência e continuidade;
- corrente dos LEDs e parâmetros do sensor;
- frames completos de qualidade, HR e SpO₂.

Ensaios:

1. sem dedo;
2. dedo estável;
3. pressão leve, média e alta;
4. luz ambiente controlada e intensa;
5. movimento do dedo/punho;
6. diferentes posições e dedos;
7. atraso artificial do firmware para forçar overflow;
8. desconexão/reconexão do sensor;
9. sessão prolongada para verificar estabilidade e wrap lógico.

Critério: nenhuma perda silenciosa; overflow detectado; frames inválidos durante condições inadequadas; recuperação controlada.

### Nível D — Validação de HR

Comparar simultaneamente com equipamento de referência apropriado.

Para cada sessão, armazenar pares sincronizados:

```text
(timestamp, HR_referência, HR_POC, qualidade, posição, condição)
```

Análises mínimas:

- erro médio absoluto;
- RMSE;
- viés e limites de concordância;
- percentual de frames válidos;
- tempo até a primeira estimativa;
- erro por faixa de HR e condição de movimento.

A aceitação da POC deve ser definida antes do ensaio. O resultado deve ser descrito como desempenho experimental, não equivalência clínica.

### Nível E — Calibração de SpO₂

A relação `R → SpO₂` depende do conjunto óptico final. Portanto:

1. congelar hardware, encapsulamento, posição, janela óptica e configuração do sensor;
2. registrar uma versão única do firmware e dos parâmetros;
3. obter dados pareados de `R` e referência adequada dentro de protocolo aprovado;
4. ajustar uma curva linear ou quadrática somente na faixa observada;
5. separar dados de ajuste e validação;
6. registrar coeficientes, domínio, população, condições, incerteza e versão;
7. repetir a validação após qualquer alteração óptica ou mecânica.

O pacote já oferece os campos:

```c
.a
.b
.c
.min_ratio_r
.max_ratio_r
.calibrated
.version
```

A flag `calibrated` só deve ser ativada após o protocolo ser concluído e documentado.

### Nível F — Integração com o I Blue It

O jogo deve consumir apenas `health_frame`, nunca RED/IR diretamente para decisões de segurança.

Fluxo recomendado:

```text
firmware
  → frame técnico válido
  → adaptador serial no Unity
  → monitor fisiológico
  → política terapêutica configurada
  → estado da sessão
  → pausa/retomada/interrupção
  → persistência e dashboard
```

Campos mínimos no Unity/API:

- timestamp e idade do frame;
- `valid` e `clinical_valid`;
- HR, SpO₂ e respectivas confianças;
- qualidade e motivos de invalidade;
- versão de calibração;
- contagem de overflow;
- estado do sensor.

## 3. Política de segurança futura

A decisão terapêutica deve ser um módulo distinto do estimador. Ela precisa considerar:

- limites definidos pelo profissional para cada paciente;
- duração mínima da condição, evitando reação a uma amostra isolada;
- histerese para retomada;
- frame obsoleto ou inválido;
- ausência do dedo;
- estado de comunicação;
- sequência de eventos;
- opção de confirmação manual;
- fail-safe quando não houver dado confiável.

Exemplo conceitual, sem limites clínicos fixos:

```text
TRACKING
  → condição de atenção persistente → PAUSED
  → condição crítica persistente → INTERRUPTED
PAUSED
  → recuperação estável + autorização → TRACKING
qualquer estado
  → dado ausente/obsoleto → SENSOR_UNAVAILABLE
```

Os valores numéricos de limiar não devem ser inventados no firmware. Devem vir do protocolo terapêutico e do perfil do paciente.

## 4. Evidências a preservar para o artigo

- versão/commit do firmware;
- esquema e fotos do hardware final;
- IDs e configuração do sensor;
- definição de todas as métricas;
- protocolo de coleta;
- critérios de inclusão/exclusão de frames;
- dados brutos e processados;
- código de análise;
- erro, viés, concordância e disponibilidade;
- limitações por movimento, perfusão, luz, posicionamento e população;
- distinção explícita entre POC, estimativa experimental e dispositivo médico.

## 5. Próximo marco técnico

O próximo marco é executar o pacote no ESP32-C3 com o módulo identificado e coletar um arquivo contínuo contendo `sensor_info`, PPG e `health_frame`. Esse arquivo permitirá ajustar os limiares de qualidade e avaliar o HR antes de iniciar qualquer calibração de SpO₂.
