# Matriz consolidada de lacunas e estado da prova de conceito

## Legenda

- **Concluído na POC:** implementação entregue e coberta ao menos por revisão estática ou teste de software.
- **Parcial:** o risco principal foi reduzido, mas falta validação, generalização ou função complementar.
- **Pendente:** depende de hardware, calibração, integração externa ou etapa posterior.
- **Não aplicável ao pacote:** questão de repositório/processo que não pode ser corrigida por esta sobreposição isolada.

## 1. Problemas críticos

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 1.1 Cálculo de SpO₂ inexistente | **Concluído na POC** | Implementada razão dos quocientes, domínio, curva configurável, status e confiança. A curva padrão é deliberadamente não calibrada. |
| 1.2 Cálculo de HR inexistente | **Concluído na POC** | Implementada estimativa por picos do IR, mediana de IBI, faixa fisiológica e confiança. |
| 1.3 Processamento não executado | **Concluído na POC** | O controlador chama qualidade, HR, SpO₂, confiança e telemetria de frame. |
| 1.4 Produção/consumo da FIFO incompatíveis | **Concluído na POC** | Polling de 10 ms e drenagem em lote de todas as amostras disponíveis. |
| 1.5 FIFO lida sem verificar dados | **Concluído na POC** | Leitura de ponteiros e cálculo de amostras disponíveis. |
| 1.6 Overflow não detectado | **Concluído na POC** | Leitura do contador, contabilização, flush e descarte da janela. |
| 1.7 Timestamp não representa medição | **Parcial** | Timestamps são reconstruídos conforme taxa nominal para cada lote. Falta timestamp de hardware ou sincronização por interrupção/data-ready. |
| 1.8 Sensor marcado sem identificação | **Concluído na POC** | Leitura de `PART_ID` e `REV_ID` antes da configuração. A distinção exata da variante permanece parcial. |
| 1.9 Sem recuperação operacional | **Concluído na POC** | Contagem de erros consecutivos, estado `ERROR` e tentativa periódica de reinicialização/configuração. |

## 2. Máquina de estados

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 2.1 Não é máquina de estados completa | **Concluído na POC** | Transições válidas são verificadas antes da mudança. |
| 2.2 `IDLE` diverge do comportamento | **Concluído na POC** | O sistema passa para `SAMPLING` quando recebe amostras. |
| 2.3 Estados não usados | **Concluído na POC** | Todos os estados participam do fluxo. |
| 2.4 Sem autoteste real | **Parcial** | Inclui identificação, reset e readback. Faltam teste óptico dos LEDs, canais, ruído e presença do dedo. |
| 2.5 Sem política de transições | **Concluído na POC** | Política formal codificada em `app_state_machine.c`. |

## 3. Driver MAX3010x

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 3.1 Driver não realmente genérico | **Parcial** | Probe e configuração comum à família; ainda assume RED+IR, 6 bytes e modo SpO₂. Falta suporte a GREEN, slots e capacidades por variante. |
| 3.2 `PART_ID` não usado | **Concluído na POC** | `PART_ID` e `REV_ID` são lidos e transmitidos. |
| 3.3 Reset por atraso fixo | **Concluído na POC** | Polling do bit de reset com timeout. |
| 3.4 Sem readback da configuração | **Concluído na POC** | Escrita e verificação de FIFO, SpO₂, LEDs e modo. |
| 3.5 Números mágicos | **Parcial** | Registradores e bits principais receberam nomes; correntes e alguns campos ainda são valores configuráveis em hexadecimal. |
| 3.6 Configuração fragmentada | **Parcial** | Taxa, LEDs e polling centralizados; ADC range, pulse width e média FIFO ainda são fixos no driver. |
| 3.7 Sem saturação/luz ambiente | **Parcial** | Clipping é detectado no processamento. Faltam interrupções de luz ambiente, proximidade e controle automático dos LEDs. |
| 3.8 Sem controle operacional | **Parcial** | Incluídos reset, flush e reconfiguração via `configure`; faltam shutdown/wakeup explícitos. |

## 4. Barramento I²C

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 4.1 Escrita sem validação | **Concluído na POC** | Estado, endereço, ponteiro, tamanho e alocação do comando são validados. |
| 4.2 Leitura sem verificar inicialização | **Concluído na POC** | Retorna `ESP_ERR_INVALID_STATE` antes de operar. |
| 4.3 Timeout de 1 s | **Concluído na POC** | Reduzido para 50 ms. |
| 4.4 Sem mutex | **Concluído na POC** | Mutex protege transações do barramento. |
| 4.5 Scan quebra abstração | **Concluído na POC** | O controlador não chama diretamente a API I²C; usa probe do driver. |
| 4.6 Scan tarde demais | **Concluído na POC** | O probe é realizado antes de reset/configuração. |

## 5. Sampler e temporização

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 5.1 Intervalo configurado não usado | **Concluído na POC** | `acquisition_poll_ms` controla o laço principal. |
| 5.2 Deriva de `vTaskDelay()` | **Concluído na POC** | Substituído por `vTaskDelayUntil()`. |
| 5.3 `seq` não garante continuidade | **Parcial** | Overflow é detectado e a janela invalidada; não existe contador de amostra fornecido pelo sensor. |
| 5.4 Timestamp de 32 bits | **Parcial** | Subtrações usam aritmética modular; o campo ainda transborda em aproximadamente 49,7 dias. |
| 5.5 `g_step_counter` sem uso | **Concluído na POC** | Removido do fluxo refeito. |

## 6. Sample buffer

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 6.1 Sobrescrita silenciosa | **Concluído na POC** | Contador `overwritten_samples`. |
| 6.2 Sem snapshot | **Concluído na POC** | `sample_buffer_copy_latest()` copia a janela em ordem temporal. |
| 6.3 Sem sincronização | **Parcial** | O fluxo atual permanece em uma tarefa. Será necessário lock se aquisição e processamento forem separados. |
| 6.4 Sem metadados | **Parcial** | Frame possui taxa, duração, continuidade e overflow; faltam metadados completos de sessão/configuração. |
| 6.5 Tamanho sem relação temporal | **Concluído na POC** | 512 amostras a 100 Hz; janela configurada de 400 amostras/4 s. |

## 7. Qualidade do sinal

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 7.1 AC por máximo-mínimo | **Concluído na POC** | AC calculado por RMS após detrending. |
| 7.2 Métrica de ruído incorreta | **Concluído na POC** | Ruído residual no IR após suavização; cálculo de SNR. |
| 7.3 PI baseado em AC frágil | **Concluído na POC** | PI usa AC RMS/DC. |
| 7.4 Mínimo de 20 amostras | **Concluído na POC** | Janela fixa de 4 s. |
| 7.5 Limiares arbitrários | **Parcial** | Limiares foram nomeados e centralizados localmente, mas ainda precisam ser ajustados com dados reais. |
| 7.6 Pesos arbitrários | **Parcial** | Score é explícito e testável, porém os pesos continuam heurísticos até validação experimental. |
| 7.7 Uso de todas as amostras | **Concluído na POC** | Apenas a janela fixa mais recente é processada. |
| 7.8 Sem remoção de tendência | **Concluído na POC** | Detrending linear em RED e IR. |
| 7.9 Sem análise temporal | **Parcial** | Continuidade e periodicidade entram no pipeline de HR; falta análise espectral e classificação robusta de artefatos. |
| 7.10 Sem coerência RED/IR | **Concluído na POC** | Correlação entre canais e limiar mínimo. |
| 7.11 Sem clipping | **Concluído na POC** | Fração de amostras próximas aos limites do ADC. |
| 7.12 Saída antiga após falha | **Concluído na POC** | Todas as saídas são zeradas no início. |

## 8. HR e SpO₂

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 8.1 `false` ambíguo | **Concluído na POC** | `estimator_status_t` diferencia indisponível, baixa qualidade, sem sinal, domínio e erro. |
| 8.2 Limiares não centralizados | **Parcial** | Qualidade global está na configuração; limiares específicos de HR e SpO₂ ainda são constantes internas documentadas. |
| 8.3 Sem calibração | **Parcial** | Estrutura, coeficientes, domínio, versão e flag implementados. Falta obter a curva experimental do hardware final. |
| 8.4 Sem razão dos quocientes | **Concluído na POC** | Cálculo de `R` implementado. |
| 8.5 Sem controle de domínio | **Concluído na POC** | Divisões, finitude, domínio de `R`, faixa de HR e faixa numérica de SpO₂ são verificadas. |

## 9. Confidence engine

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 9.1 Não calcula confiança própria | **Concluído na POC** | Combina qualidade, confiança de HR e confiança de SpO₂. |
| 9.2 `frame.valid` ignora estimadores | **Concluído na POC** | HR e SpO₂ válidos são obrigatórios. |
| 9.3 Limiar fixo | **Parcial** | Limiar global configurável; pesos e faixas devem ser validados. |
| 9.4 Módulo não chamado | **Concluído na POC** | Integrado ao controlador. |
| 9.5 `finger_present` simplista | **Parcial** | Ainda deriva da presença de sinal; deve receber lógica dedicada com histerese e DC/PI. |
| 9.6 Mistura frame e segurança | **Parcial** | O frame contém validade técnica. A política clínica de pausar/interromper continua separada e pendente. |

## 10. Tipos e modelo de dados

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 10.1 `valid` ambíguo | **Concluído na POC** | Separados `valid` e `clinical_valid`. |
| 10.2 Sem motivo de invalidade | **Concluído na POC** | Máscara `ppg_invalid_reason_t`. |
| 10.3 Sem idade da estimativa | **Parcial** | Frame possui timestamp; consumidor ainda precisa calcular idade ou receber campo explícito. |
| 10.4 Sem incerteza | **Parcial** | Há confiança de 0 a 1, mas não intervalo de confiança/metrologia. |
| 10.5 Sem rastreabilidade de configuração | **Parcial** | Versão da calibração e IDs do sensor são enviados. Falta hash/versão completa da configuração e firmware. |

## 11. Telemetria

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 11.1 Apenas RAW | **Concluído na POC** | `health_frame` inclui qualidade, HR, SpO₂, confiança e estados. |
| 11.2 `printf` em toda amostra | **Concluído na POC** | RAW limitado a 5 Hz; processamento/frame a 2 Hz. |
| 11.3 Sem versionamento | **Concluído na POC** | Campo `v=1`. |
| 11.4 Logs e dados no mesmo canal | **Parcial** | Continuam na serial, porém cada mensagem possui `type`; a integração final deve separar ou rotear por tipo. |
| 11.5 Estado pode divergir | **Concluído na POC** | O texto é gerado diretamente a partir da máquina de estados. |

## 12. Configuração e armazenamento

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 12.1 Configuração não persistente | **Pendente** | Migrar para NVS e incluir validação/versionamento. |
| 12.2 Configuração ignorada | **Concluído na POC** | Polling, taxa, LEDs, janela, intervalo e qualidade são consumidos. |
| 12.3 Configuração incompleta | **Parcial** | Parâmetros essenciais foram adicionados; faltam ADC, pulse width, FIFO average, filtros e política de segurança. |
| 12.4 Sem persistência da calibração | **Pendente** | Implementar armazenamento seguro após definição do processo de calibração. |

## 13. Inicialização e segurança

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 13.1 Falha abrupta por `ESP_ERROR_CHECK` | **Concluído na POC** | Inicialização retorna ao estado de erro e tenta recuperação. |
| 13.2 Sem desligamento seguro dos LEDs | **Pendente** | Adicionar shutdown/wakeup e desligar LEDs em erro permanente ou encerramento. |
| 13.3 Sem watchdog lógico | **Parcial** | Há contador de erros I²C. Falta watchdog para ausência de amostras, frame obsoleto e travamento sem erro do barramento. |
| 13.4 Camada safety sem política | **Pendente** | O pacote produz frame confiável; regras terapêuticas, histerese, pausa, retomada e interrupção são a próxima camada. |

## 14. Engenharia e manutenção

| Ponto | Estado | Tratamento realizado / pendência |
|---|---|---|
| 14.1 Sem testes automatizados | **Parcial** | Testes host de pipeline, janela curta e ausência de sinal. Faltam ESP-IDF, driver simulado, HIL e falhas injetadas. |
| 14.2 Diretório `build` versionado | **Não aplicável ao pacote** | Corrigir `.gitignore` e remover artefatos no repositório principal. |
| 14.3 API I²C no controlador | **Concluído na POC** | Toda comunicação passa por driver/barramento. |
| 14.4 Código não utilizado | **Concluído na POC** | Fluxo principal usa todos os módulos entregues; deve-se aplicar análise estática no repositório completo. |

## 15. Pontos não imediatamente errados

| Ponto | Estado | Observação |
|---|---|---|
| 15.1 Buffer circular | **Mantido e aprimorado** | Estrutura adequada para POC; recebeu snapshot e métricas. |
| 15.2 Sequência antes do `push` | **Risco residual baixo** | O `push` só falha por argumento inválido; pode ser refinado para incrementar após confirmação. |
| 15.3 Concorrência | **Adequado ao fluxo atual** | O buffer é usado em uma tarefa; adicionar lock ao separar tarefas. |

# Resultado por fase recomendada

| Fase | Resultado atual |
|---|---|
| 1 — Aquisição/FIFO/temporização | **Concluída para POC**, pendente validação física. |
| 2 — Integração do pipeline | **Concluída para POC**. |
| 3 — Qualidade do sinal | **Implementada**, pendente ajuste com dados reais e movimento. |
| 4 — HR e SpO₂ | **Implementados como estimadores experimentais**; SpO₂ ainda não calibrada. |
| 5 — Segurança e estados | **Estados e validade técnica implementados**; política terapêutica ainda pendente. |

# Conclusão

O firmware deixa de ser apenas um transmissor de RED/IR bruto e passa a ser uma prova de conceito funcional de aquisição e processamento. O principal bloqueio para transformar a estimativa de SpO₂ em uma medida defendível academicamente não é mais a ausência do algoritmo, mas a ausência de **calibração e validação do conjunto óptico final em hardware real**. O principal bloqueio para utilizar os dados na segurança do I Blue It é a definição e validação das **regras clínicas de pausa/interrupção**, que devem consumir apenas frames tecnicamente válidos e nunca a curva demonstrativa não calibrada.
