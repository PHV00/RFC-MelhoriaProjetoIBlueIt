> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.
> Baseline de código auditado: commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0`, por inspeção estática.


# SCIENTIFIC_CONTEXT

## 1. Pergunta científica central

**RQ-001:** É possível integrar um módulo MAX30102 ao I Blue It de modo que a SpO2 e a frequência cardíaca sejam tecnicamente confiáveis, rastreáveis e adequadas para apoiar monitoramento e decisões de segurança durante exercícios respiratórios, sem confundir funcionamento técnico com eficácia clínica?

## 2. Contexto científico por cadeia de evidência

### 2.1 Jogos sérios e reabilitação respiratória

SRC-LIT-001 e SRC-LIT-002 descrevem o desenvolvimento do I Blue It e do PITACO com design participativo. O principal resultado é utilidade percebida e adequação funcional segundo especialistas, não comparação clínica contra terapia convencional. A própria dissertação restringiu o escopo ao contexto tecnológico, excluiu resultados diagnósticos e declarou que o dispositivo não buscava precisão/exatidão de pneumotacógrafo comercial (SRC-LIT-001, PDF p. 29).

**Classificação:** evidência de viabilidade e utilidade percebida; não evidência de eficácia clínica.

### 2.2 IA em jogos para reabilitação respiratória

SRC-LIT-003 realizou MSL com busca até maio de 2020. Dos 7.918 registros, 28 artigos foram selecionados e apenas 3 apresentaram IA explicitamente identificada, usando aprendizado de máquina para biofeedback respiratório. O estudo concluiu existir uma lacuna no uso de IA em jogos para terapia respiratória.

**Classificação:** evidência secundária histórica; precisa ser atualizada antes de sustentar alegação de estado da arte em artigo de 2026.

### 2.3 Multimodalidade e segurança

SRC-LIT-004 propõe a 123-SGR para unir flexibilidade, complementaridade e segurança. A arquitetura distingue fluxos conscientes e inconscientes; sinais fisiológicos podem acionar adaptações ou interrupções. O tratamento de sinais é explicitamente um núcleo arquitetural, mas o artigo não estabelece algoritmo de oximetria, desempenho metrológico ou protocolo clínico.

**Classificação:** evidência arquitetural/conceitual; não valida o sensor nem a decisão clínica.

### 2.4 Flow Psicofisiológico e DeepDDA

SRC-LIT-005 estende o Flow para dimensões psíquica e fisiológica. O DeepDDA usa aprendizado por reforço/PPO e combina desempenho, percepção de esforço e biossinais. O monitoramento de SpO2 proposto reduz velocidade abaixo de 95% e interrompe abaixo de 89% (PDF p. 119). A prova de conceito foi testada com dados sintetizados baseados em um indivíduo hígido, por limitações de recursos e para ajuste experimental de hiperparâmetros (PDF p. 125). A tese reconhece que isso não representa a variabilidade de pacientes e limita generalização clínica (PDF p. 149).

**Classificação:** prova de conceito técnico-metodológica; limiares e política não são validação clínica universal.

## 3. Cadeia causal proposta

`aquisição correta -> PPG com qualidade -> HR/SpO2 com incerteza conhecida -> classificação de risco válida -> ação de segurança rastreável -> potencial redução de exposição a esforço inadequado`

Cada seta exige evidência própria. Um sistema que calcula números plausíveis não necessariamente produz medições corretas; uma medição correta não prova que o limiar é apropriado; um limiar apropriado não prova benefício clínico.

## 4. Construtos e definições operacionais

| ID | Construto | Definição operacional neste projeto |
|---|---|---|
| CST-001 | PPG bruto | séries RED e IR com timestamp, configuração do sensor e taxa efetiva de amostragem |
| CST-002 | Qualidade de sinal | escore e motivos objetivos: contato, saturação do ADC, amplitude, ruído, clipping, movimento, estabilidade e periodicidade |
| CST-003 | SpO2 estimada | valor calculado por algoritmo documentado, acompanhado de validade, confiança, janela e versão de calibração |
| CST-004 | HR estimada | BPM derivado de pulsos detectados, acompanhado de qualidade, número de batimentos e estabilidade |
| CST-005 | Condição de risco | regra clínica/configurável aplicada somente a sinal válido; deve registrar limiar, duração e contexto |
| CST-006 | Ação do sistema | manter, reduzir, aumentar, pausar ou interromper; deve ser auditável e passível de override |
| CST-007 | Funcionamento técnico | aquisição, cálculo, latência, robustez, rastreabilidade e desempenho contra referência |
| CST-008 | Eficácia clínica | melhora de desfecho clínico/terapêutico atribuível ao sistema em desenho apropriado; não demonstrada |

## 5. Evidência interna do projeto

As conversas indicam evolução rápida do protótipo, troca de MAX30105 para MAX30102 e análise de logs. Essas informações orientam engenharia, mas são observações não controladas. O log com valores de SpO2/HR não informa referência simultânea, posição do dedo, corrente de LED, largura de pulso, movimento, temperatura, janela, algoritmo/calibração ou commit. Portanto, ele demonstra execução de alguma versão do software, não exatidão.

No commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0`, a inspeção estática confirma que o controlador chama avaliação de qualidade, estimativa de HR, estimativa de SpO₂ e construção de confiança. O driver também lê `PART_ID`/`REV_ID`, verifica registradores e drena amostras disponíveis da FIFO em lote. Essa constatação resolve a dúvida sobre a presença dos módulos no HEAD, mas não demonstra compilação, temporização real, comportamento no hardware ou desempenho contra referência (EVD-018).

A configuração permite uma estimativa experimental com curva marcada `calibrated=false`. O modelo interno separa `valid` de `clinical_valid`, porém a telemetria ativa não transmite `clinical_valid`, versão/calibração, motivos de invalidade, overflow ou status dos estimadores. Isso é uma discrepância atual de contrato e uma prioridade de correção; não é uma propriedade desejada ou permanente da arquitetura.

## 6. Alegações permitidas e proibidas no estado atual

### Permitidas

- “O código-fonte contém uma prova de conceito para adquirir RED/IR e produzir estimativas preliminares não calibradas.”
- “O pipeline está sendo estruturado com qualidade de sinal, confiança e estados explícitos de invalidade; a política fail-safe completa ainda está planejada.”
- “A literatura do ecossistema I Blue It sustenta a relevância de monitoramento multimodal e ajuste dinâmico.”

### Não sustentadas

- “O dispositivo é clinicamente preciso.”
- “O módulo aumenta a segurança do paciente.”
- “A SpO2 é equivalente a equipamento homologado.”
- “O DeepDDA melhora resultados terapêuticos.”
- “Os limiares 95%/89% são adequados para todos os pacientes.”

## 7. Documentos que precisam de revisão humana

- **Prioridade alta:** intended use, classificação regulatória, limiares, ações e mensagens ao terapeuta.
- **Prioridade alta:** algoritmo/calibração de SpO2 e protocolo de referência, por especialista em instrumentação/oximetria.
- **Prioridade alta:** revisão do código no commit usado nos logs, porque SRC-PRJ-003 conflita com observações posteriores.
- **Prioridade média:** atualização da revisão de literatura após 2020.
- **Prioridade média:** alinhamento entre 123-SGR de 2020 e a versão 2023 citada, mas não fornecida.

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
