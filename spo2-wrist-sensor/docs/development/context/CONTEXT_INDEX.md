> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.


# CONTEXT_INDEX

## 1. Como usar o pacote

1. Comece em `PROJECT_BRIEF.md` para escopo e estado.
2. Consulte `SCIENTIFIC_CONTEXT.md` para separar evidência de hipótese.
3. Use `EVIDENCE_MATRIX.md` para verificar a origem de afirmações.
4. Use `TRACEABILITY_MATRIX.md` antes de implementar ou escrever o artigo.
5. Use `RESEARCH_PROTOCOL.md` antes de qualquer validação formal.
6. Registre mudanças em `DECISIONS.md` e preserve IDs existentes.

## 2. Índice por categoria solicitada

| Categoria | Documento principal | IDs |
|---|---|---|
| evidências da literatura | SCIENTIFIC_CONTEXT, EVIDENCE_MATRIX | EVD-001 a EVD-009 |
| observações do projeto | PROJECT_BRIEF, EVIDENCE_MATRIX | EVD-010 a EVD-015 |
| hipóteses científicas | HYPOTHESES | HYP-001 a HYP-007 |
| decisões metodológicas | PROJECT_BRIEF, DECISIONS | DM-001 a DM-005 |
| requisitos clínicos | SOFTWARE_REQUIREMENTS | CR-001 a CR-012 |
| requisitos de software | SOFTWARE_REQUIREMENTS | SR-F-001 a 022; SR-NF-001 a 018 |
| decisões de engenharia | DECISIONS, ARCHITECTURE | ADR-001 a ADR-012 |
| resultados preliminares | PROJECT_BRIEF, EVIDENCE_MATRIX | EVD-002, 006, 009, 014 |
| resultados confirmados | PROJECT_BRIEF | confirmação documental, não clínica |
| dúvidas e conflitos | LIMITATIONS, OPEN_RESEARCH_QUESTIONS | CFLT-001 a 010; ORQ-001 a 018 |

## 3. Índice de IDs

- Objetivos: OBJ-001..006
- Pergunta de pesquisa: RQ-001
- Construtos: CST-001..008
- Hipóteses: HYP-001..007; H0-001..006
- Evidências: EVD-001..017
- Não verificáveis: NV-001..005
- Requisitos clínicos: CR-001..012
- Requisitos funcionais: SR-F-001..022
- Requisitos não funcionais: SR-NF-001..018
- Componentes: CMP-001..015
- Decisões: ADR-001..012; ODR-001..008; DM-001..005
- Métricas: MET-001..010
- Testes: TST-001..012
- Conflitos: CFLT-001..010
- Questões abertas: ORQ-001..018

## 4. Estado geral

| Área | Estado | Evidência mínima faltante |
|---|---|---|
| fundamentação histórica do I Blue It | documentada | atualização de versões recentes |
| arquitetura multimodal | documentada conceitualmente | publicação 2023/código atual |
| aquisição MAX30102 | em execução | testes FIFO/timestamp/commit |
| qualidade de sinal | parcial | dataset rotulado e falso válido |
| HR/SpO2 | preliminar | comparação contra referência |
| segurança/DeepDDA | prova de conceito/planejado | política aprovada e replay |
| persistência/dashboard | proposto | teste ponta a ponta |
| validação clínica | ausente | protocolo, ética e estudo |
| eficácia clínica | ausente | desenho clínico futuro |

## 5. Tabela de lacunas solicitada

A tabela detalhada está em `TRACEABILITY_MATRIX.md`, seção 3. A lacuna dominante é a ausência de uma cadeia completa `objetivo -> hipótese -> requisito -> implementação fixada -> teste -> métrica -> resultado -> seção do artigo` para a exatidão do MAX30102 e para decisões de segurança.

## 6. Artefatos que devem acompanhar a próxima versão

- hash do commit e binário;
- exportação de configuração do sensor;
- dataset PPG bruto + referência + metadados;
- resultados dos testes automatizados e HIL;
- protocolo aprovado e relatório de desvios;
- tabela de decisões clínicas aprovada;
- dicionário da telemetria/banco;
- versão do manuscrito com links para IDs deste pacote.

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
