> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.
> Baseline de código auditado: commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0`, por inspeção estática.


# PROJECT_BRIEF

## 1. Definição operacional do projeto

O projeto atual investiga e implementa a integração de oximetria de pulso ao ecossistema I Blue It, com foco inicial em funcionamento técnico confiável e rastreável. O alvo imediato é adquirir sinais PPG RED/IR por um MAX30102 conectado a um ESP32-C3, estimar frequência cardíaca e SpO2 com avaliação explícita de qualidade, transmitir e persistir os dados, e disponibilizá-los a mecanismos de segurança, ao DeepDDA e ao dashboard clínico.

A finalidade clínica pretendida ainda não está formalmente fechada. O material histórico descreve o I Blue It como ferramenta de apoio à reabilitação respiratória, sob supervisão profissional; o trabalho de 2018 excluiu resultados diagnósticos e declarou que o PITACO não buscava precisão de pneumotacógrafo comercial (SRC-LIT-001, PDF pp. 28-29). A tese de 2024 apresenta a oximetria e o DeepDDA como prova de conceito, com dados simulados baseados em um único indivíduo hígido e limitação explícita de generalização clínica (SRC-LIT-005, PDF pp. 125 e 149).

## 2. Objetivos estáveis

| ID | Objetivo | Tipo | Estado |
|---|---|---|---|
| OBJ-001 | Integrar o MAX30102 ao firmware do PITACO/ESP32-C3 com aquisição PPG sincronizada, sem perda silenciosa de FIFO. | engenharia | em execução |
| OBJ-002 | Produzir estimativas de HR e SpO2 somente quando a qualidade do sinal e a confiança forem suficientes. | técnico-científico | em execução |
| OBJ-003 | Operacionalizar decisões de segurança e ajuste do DeepDDA usando SpO2 válida, mantendo possibilidade de intervenção do terapeuta. | software clínico | planejado/parcial |
| OBJ-004 | Preservar rastreabilidade ponta a ponta: amostra, qualidade, estimativa, alerta, decisão da IA, parâmetros antes/depois e estado da sessão. | engenharia clínica | planejado/parcial |
| OBJ-005 | Validar tecnicamente o módulo contra equipamento de referência e cenários controlados, com métricas pré-especificadas. | pesquisa | planejado |
| OBJ-006 | Avaliar viabilidade clínica, usabilidade e segurança operacional antes de qualquer estudo de eficácia. | pesquisa clínica | não iniciado |

## 3. Separação das dez classes de informação

### 3.1 Evidências provenientes da literatura

- Jogos sérios e exergames podem apoiar engajamento e treinamento, mas o material fornecido não demonstra eficácia clínica do novo módulo de oximetria. SRC-LIT-001 e SRC-LIT-002 demonstram principalmente concepção e utilidade percebida por especialistas.
- O mapeamento de 2020 encontrou 7.918 registros, selecionou 28 estudos e identificou uso explícito de IA em apenas 3, indicando lacuna de pesquisa naquele recorte temporal (SRC-LIT-003, resumo e seção de resultados).
- A 123-SGR formaliza flexibilidade, complementaridade e segurança por fluxos consciente/inconsciente, tratamento, fusão, adaptação e fissão (SRC-LIT-004, seções I e IV).
- A tese de 2024 implementou uma prova de conceito DeepDDA com PPO, testes de software e validação por especialistas, mas usou dados simulados de um indivíduo hígido e reconheceu ausência de variabilidade clínica (SRC-LIT-005, PDF pp. 125-149).

### 3.2 Observações realizadas no projeto

- Logs de 2026-07-13 mostraram alternância rápida de HR e SpO2, inclusive uma primeira leitura `valid=false` e leituras subsequentes marcadas válidas com baixa confiança. Isso sugere que o critério de validade ainda não representa estabilidade clínica. Fonte: SRC-PRJ-004. **PRELIMINAR**.
- Um teste anterior identificou `part_id=0x15`, `revision_id=0x06`, compatível com MAX30105; posteriormente o hardware foi trocado para MAX30102. Resultados do módulo anterior não validam o atual. Fonte: SRC-PRJ-001. **CONFLITO TEMPORAL**.
- A análise histórica em `MELHORIA.md` declarou estimadores de HR/SpO2 não implementados e módulos não integrados. A inspeção do commit `a74986d...` confirma que esses módulos agora possuem implementação e são chamados pelo controlador. A divergência foi classificada como evolução entre versões; ainda faltam build, testes e validação em hardware. Fontes: SRC-PRJ-003 e SRC-PRJ-005. **ESTADO ESTÁTICO ATUALIZADO**.
- No baseline auditado, a curva de SpO₂ está explicitamente não calibrada e a telemetria ativa omite campos internos importantes, incluindo `clinical_valid`, calibração e motivos de invalidade. Isso deve ser tratado como pendência prioritária de implementação, não como contrato final do sistema. Fonte: SRC-PRJ-005. **PARCIAL**.

### 3.3 Hipóteses científicas

As hipóteses formais são HYP-001 a HYP-007 em `HYPOTHESES.md`. Nenhuma hipótese de eficácia clínica está confirmada.

### 3.4 Decisões metodológicas

- Executar primeiro validação técnica do pipeline PPG; só depois integrar limiares de segurança e DeepDDA.
- Separar dados simulados, testes em voluntários hígidos e estudos com pacientes.
- Comparar contra equipamento de referência com protocolo e métricas definidos antes da coleta.
- Tratar os valores 95% e 89% da tese como parâmetros de prova de conceito, não como regra clínica universal.

### 3.5 Requisitos clínicos

O sistema deve ser supervisionado, indicar sinal inválido como desconhecido, permitir override do terapeuta, não emitir diagnóstico, preservar associação paciente-sessão e proteger dados sensíveis. Ver CR-001 a CR-012 em `SOFTWARE_REQUIREMENTS.md`.

### 3.6 Requisitos de software

Incluem identificação do sensor, configuração variante-específica, drenagem da FIFO, timestamps coerentes, filtros, qualidade, HR/SpO2, confiança, telemetria versionada, persistência, alertas, simulação e testes. Ver SR-F-001 a SR-NF-018.

### 3.7 Decisões de engenharia

A arquitetura implementada no baseline está organizada em `drivers -> sensing -> processing -> safety -> transport/storage -> app`, com separação parcial entre sinal bruto, estimativa e decisão. Os supostos ADR-001 a ADR-012 continuam não verificáveis porque `DECISIONS.md` não existe; a arquitetura corrente deve ser conferida em `spo2-wrist-sensor/ReadME.md`, `docs/development/dev/Structure.md` e no código.

### 3.8 Resultados preliminares

- Utilidade percebida do I Blue It histórico: 4,1/5 na dissertação, com 106 atores, 85 especialistas e 15 avaliações; isso não é desfecho clínico (SRC-LIT-001, PDF pp. 119-121).
- DeepDDA: aprendizado e funcionamento de prova de conceito com dados sintéticos; não generalizável para pacientes (SRC-LIT-005, PDF pp. 125-149).
- Firmware atual: o código contém aquisição, qualidade, HR, SpO₂ experimental e confiança integrados ao controlador; logs anteriores sugerem execução, mas continuam sem referência e sem associação inequívoca ao commit auditado (SRC-PRJ-004/005).

### 3.9 Resultados confirmados

Neste pacote, “confirmado” significa reproduzível a partir de documentação fornecida, não eficácia clínica:

- O I Blue It e o PITACO foram implementados historicamente e avaliados quanto à percepção de utilidade (SRC-LIT-001/002).
- A arquitetura 123-SGR foi descrita e aplicada como prova de conceito (SRC-LIT-004).
- A tese implementou e testou tecnicamente uma prova de conceito DeepDDA, reconhecendo suas limitações (SRC-LIT-005).
- Não há resultado confirmado de acurácia do MAX30102, equivalência com oxímetro homologado, segurança em pacientes ou benefício terapêutico incremental.

### 3.10 Dúvidas e conflitos

Os conflitos CFLT-001 a CFLT-011 estão em `LIMITATIONS.md`. Os mais críticos são: identidade/versão do sensor, validade exposta pela telemetria, justificativa clínica dos limiares, intended use e ausência de protocolo de referência metrológica.

## 4. Critério de conclusão da fase atual

A fase técnica só deve ser considerada concluída quando houver: commit fixado; identificação automática do MAX30102; captura sem overflow/perda silenciosa; dados brutos preservados; critérios de qualidade testados; HR/SpO2 comparados a referência; incerteza reportada; falhas de sensor tratadas; telemetria e banco rastreáveis; suíte de testes reproduzível; e revisão humana clínica dos limiares e mensagens.

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
| SRC-PRJ-003 | versão histórica de `docs/development/dev/MELHORIA.md` no commit `a74986d...` | análise estática interna histórica | rastrear problemas observados antes da atualização documental | diverge do código do mesmo baseline em vários itens; usar SRC-PRJ-005 para o estado atual |
| SRC-PRJ-004 | Logs de oximetria reproduzidos em conversa de 2026-07-13 | observação interna | comportamento preliminar do protótipo | sensor/commit/condições não integralmente preservados; não é validação metrológica |
| SRC-PRJ-005 | Código e configuração do commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0` | artefato versionado do projeto | estado estático atual da implementação e inventário de testes | não houve build, execução HIL ou comparação com referência nesta auditoria |

### Convenção de localização

As citações internas usam `Fonte, PDF p. N` para PDFs e `Fonte, seção/linhas` para documentos vivos. O número de página é o número do arquivo PDF, não necessariamente a paginação impressa. Afirmações de conversa usam data e tema; quando o trecho completo não estava disponível, recebem a marca **NÃO VERIFICÁVEL NESTA EXECUÇÃO**.
