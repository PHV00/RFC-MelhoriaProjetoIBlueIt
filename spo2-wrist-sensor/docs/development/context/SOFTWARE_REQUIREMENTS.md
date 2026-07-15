> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.


# SOFTWARE_REQUIREMENTS

## 1. Requisitos clínicos

| ID | Requisito clínico | Origem | Verificação |
|---|---|---|---|
| CR-001 | O uso deve ocorrer sob supervisão de profissional/responsável conforme protocolo. | SRC-LIT-001, PDF p. 29; contexto do projeto | revisão clínica + teste de fluxo |
| CR-002 | O sistema não deve emitir diagnóstico ou substituir julgamento clínico. | escopo histórico + decisão metodológica | interface/documentação |
| CR-003 | O terapeuta deve poder pausar, interromper, retomar e sobrescrever ação automática. | segurança e intended use | TST-010 |
| CR-004 | Sinal inválido, ausente ou de baixa confiança deve ser exibido como desconhecido, nunca como normalidade. | segurança de medição | TST-004/TST-006 |
| CR-005 | Limiares e duração de persistência devem ser configuráveis por protocolo/perfil e aprovados clinicamente. | conflito dos limiares da tese | inspeção + teste |
| CR-006 | Alertas devem informar valor, qualidade, tendência, motivo, ação e timestamp. | rastreabilidade | TST-008/TST-011 |
| CR-007 | A associação paciente-sessão-dispositivo deve ser inequívoca. | SRC-PRJ-002, modelo de dados | teste de integridade |
| CR-008 | O sistema deve distinguir alerta técnico (sensor) de alerta fisiológico. | engenharia clínica | TST-012 |
| CR-009 | Dados sensíveis devem seguir minimização, acesso autorizado e política de retenção. | requisito de privacidade | revisão de segurança |
| CR-010 | A ação automática deve ser conservadora quando a medição é incerta, sem ocultar indisponibilidade. | fail-safe | cenários de falha |
| CR-011 | Mensagens e comportamento devem ser revisados por profissional da população-alvo. | necessidade de UFE | validação humana |
| CR-012 | Uso em pacientes só pode iniciar após aprovação do protocolo e critérios de interrupção independentes do protótipo. | ética/segurança | gate de pesquisa |

## 2. Requisitos funcionais

| ID | Requisito | Prioridade | Fonte/status |
|---|---|---|---|
| SR-F-001 | Ler e registrar PART_ID e REV_ID no boot. | MUST | falha apontada em SRC-PRJ-003 |
| SR-F-002 | Selecionar configuração compatível com MAX30102 e rejeitar variante desconhecida ou modo incompatível. | MUST | troca de sensor |
| SR-F-003 | Configurar sample rate, average, pulse width, ADC range e correntes LED de forma versionada. | MUST | processamento/calibração |
| SR-F-004 | Consultar ponteiros/overflow da FIFO e drenar todas as amostras disponíveis. | MUST | SRC-PRJ-003 |
| SR-F-005 | Reconstruir timestamps pelo contador/taxa efetiva e detectar gaps. | MUST | SRC-PRJ-003 |
| SR-F-006 | Armazenar RED/IR brutos antes de filtros, com metadados. | MUST | reprodutibilidade |
| SR-F-007 | Detectar dedo/contato sem usar um único limiar absoluto não calibrado. | MUST | falhas observadas |
| SR-F-008 | Aplicar filtros documentados sem distorcer frequência/amplitude relevante. | MUST | HYP-002 |
| SR-F-009 | Calcular escore de qualidade e lista de motivos de invalidade. | MUST | RNF06 do projeto |
| SR-F-010 | Estimar HR com janela, picos, estabilidade e número mínimo de batimentos. | MUST | OBJ-002 |
| SR-F-011 | Estimar SpO2 por algoritmo e curva de calibração versionados. | MUST | OBJ-002 |
| SR-F-012 | Produzir `valid`, `confidence`, faixa/erro estimado e status de estabilização separados do valor. | MUST | logs preliminares |
| SR-F-013 | Implementar máquina de estados com transições validadas: BOOT, SELF_TEST, IDLE, SAMPLING, TRACKING, LOW_CONFIDENCE, ERROR. | MUST | SRC-PRJ-003 |
| SR-F-014 | Recuperar falhas transitórias e registrar falhas permanentes, sem loop enganoso IDLE/coleta. | MUST | SRC-PRJ-003 |
| SR-F-015 | Emitir telemetria versionada para sensor_info, raw opcional, qualidade, oximetria e eventos. | MUST | arquitetura atual |
| SR-F-016 | Vincular cada frame a dispositivo, paciente, sessão, firmware e sequência. | MUST | CR-007 |
| SR-F-017 | Aplicar regra de segurança somente após validade, persistência e política configurada. | MUST | HYP-004 |
| SR-F-018 | Registrar decisão do DeepDDA: observação, ação, recompensa, parâmetros antes/depois e resultado. | MUST | SRC-PRJ-002 RF30-34 |
| SR-F-019 | Permitir replay de dados brutos/frames e injeção de dados simulados. | MUST | testabilidade |
| SR-F-020 | Exibir no dashboard valor, qualidade, tendência, eventos, indisponibilidade e proveniência. | SHOULD | objetivo do projeto |
| SR-F-021 | Exportar dados em formato documentado sem perder inválidos/alertas. | SHOULD | histórico/dashboard |
| SR-F-022 | Permitir override do terapeuta e registrar justificativa. | MUST | CR-003 |

## 3. Requisitos não funcionais

| ID | Categoria | Requisito/critério |
|---|---|---|
| SR-NF-001 | segurança | falha de sensor não pode ser interpretada como condição fisiológica segura |
| SR-NF-002 | confiabilidade | nenhuma perda/overflow pode ocorrer silenciosamente |
| SR-NF-003 | desempenho | latência p95 deve ser medida; 800 ms é meta de projeto, não resultado confirmado |
| SR-NF-004 | determinismo | replay da mesma janela/configuração deve gerar o mesmo resultado, salvo componentes explicitamente estocásticos |
| SR-NF-005 | modularidade | drivers/sensing/processing/safety/transport/storage/app com interfaces testáveis |
| SR-NF-006 | portabilidade | abstração MAX3010x, mas configuração e capacidades por variante |
| SR-NF-007 | observabilidade | contadores de FIFO, gaps, resets, falhas I2C, qualidade e tempos devem estar disponíveis |
| SR-NF-008 | rastreabilidade | todo resultado deve apontar para versão de algoritmo/calibração e dados de origem |
| SR-NF-009 | integridade | registros não podem ser sobrescritos nem associados a paciente/sessão errados |
| SR-NF-010 | privacidade | criptografia, autenticação e minimização conforme arquitetura final e legislação aplicável |
| SR-NF-011 | usabilidade | estados conectado/desconectado/estabilizando/inválido/alerta devem ser inequívocos |
| SR-NF-012 | manutenibilidade | parâmetros clínicos e de sensor fora do código, com versionamento e validação |
| SR-NF-013 | testabilidade | módulos devem aceitar dados gravados/simulados e mocks de I2C/tempo |
| SR-NF-014 | compatibilidade | integração não pode quebrar captura respiratória, jogo ou dashboard existentes |
| SR-NF-015 | auditabilidade | decisões automáticas e manuais devem ser reconstruíveis |
| SR-NF-016 | documentação | protocolo de frame, unidades, campos, erros e versões devem ser publicados no repositório |
| SR-NF-017 | qualidade | análise estática, testes automatizados e revisão devem acompanhar mudanças críticas |
| SR-NF-018 | segurança operacional | watchdog e recuperação não podem gerar reinício silencioso com sessão clínica continuando sem aviso |

## 4. Critérios de aceitação não definidos

Os seguintes requisitos ainda precisam de valores aprovados: erro máximo de SpO2/HR, tempo de estabilização, cobertura válida mínima, falso válido máximo, latência de ação, persistência de queda e política por população. Não foram preenchidos artificialmente.

## Catálogo de fontes

| ID | Fonte | Tipo | Uso permitido neste pacote | Estado/revisão |
|---|---|---|---|---|
| SRC-LIT-001 | Grimes, 2018, *Sistema Biomédico (com Jogo Sério e Dispositivo Especial) para Reabilitação Respiratória* | dissertação | fundamento histórico, escopo e resultados de utilidade percebida | fundacional; hardware e software possivelmente superados |
| SRC-LIT-002 | Santos et al., 2018, *I Blue It: Um Jogo Sério para auxiliar na Reabilitação Respiratória* | artigo em evento | síntese da concepção e avaliação do I Blue It | fundacional; parcialmente redundante com SRC-LIT-001 |
| SRC-LIT-003 | Dias et al., 2020, *Uso da Inteligência Artificial em Jogos Digitais aplicados à Reabilitação Respiratória* | mapeamento sistemático | evidência secundária sobre lacunas até maio de 2020 | desatualizado para estado da arte atual; protocolo ainda útil |
| SRC-LIT-004 | Nery et al., 2020, *123-SGR: Uma Arquitetura para Jogos Sérios Multimodais para Reabilitação* | artigo em evento | arquitetura conceitual e prova de conceito multimodal | requer comparação com a versão 2023 citada na tese, não fornecida |
| SRC-LIT-005 | Dias, 2024, *Flow Psicofisiológico em Jogos Digitais* | tese | fundamento do DeepDDA, requisitos, prova de conceito, testes e limitações | fonte mais recente fornecida; não equivale a ensaio clínico |
| SRC-PRJ-001 | Conversas do projeto, 2026-07-03 a 2026-07-15 | registro interno | decisões, observações, logs e dúvidas do desenvolvimento | não revisado por pares; trechos completos não estavam todos disponíveis nesta execução |
| SRC-PRJ-002 | Repositório `PHV00/RFC-MelhoriaProjetoIBlueIt`, README, acessado em 2026-07-15 | documento vivo do projeto | objetivos, requisitos, KPIs e modelo de dados proposto | não fixado por hash nesta execução; conteúdo pode mudar |
| SRC-PRJ-003 | `spo2-wrist-sensor/MELHORIA.md`, acessado em 2026-07-15 | análise estática interna | falhas identificadas no firmware e fluxo arquitetural desejado | pode estar desatualizado em relação ao binário/branch usado nos logs |
| SRC-PRJ-004 | Logs de oximetria reproduzidos em conversa de 2026-07-13 | observação interna | comportamento preliminar do protótipo | sensor/commit/condições não integralmente preservados; não é validação metrológica |

### Convenção de localização

As citações internas usam `Fonte, PDF p. N` para PDFs e `Fonte, seção/linhas` para documentos vivos. O número de página é o número do arquivo PDF, não necessariamente a paginação impressa. Afirmações de conversa usam data e tema; quando o trecho completo não estava disponível, recebem a marca **NÃO VERIFICÁVEL NESTA EXECUÇÃO**.
