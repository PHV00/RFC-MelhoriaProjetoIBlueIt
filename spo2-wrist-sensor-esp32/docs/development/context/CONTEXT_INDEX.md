> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.
> Baseline de código auditado: commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0`, por inspeção estática.


# CONTEXT_INDEX

## 1. Como usar o pacote

1. Comece em `PROJECT_BRIEF.md` para escopo e estado.
2. Consulte `SCIENTIFIC_CONTEXT.md` para separar evidência de hipótese.
3. Use `EVIDENCE_MATRIX.md` para verificar a origem de afirmações.
4. Use `TRACEABILITY_MATRIX.md` antes de implementar ou escrever o artigo.
5. Use `RESEARCH_PROTOCOL.md` antes de qualquer validação formal.
6. Preserve IDs existentes. `DECISIONS.md` ainda não existe neste checkout; decisões não devem ser inferidas como aceitas a partir dessa ausência.

### Equivalentes disponíveis nesta árvore

- `docs/scientific/synthesis/SCIENTIFIC_CONTEXT.md` → `docs/development/context/SCIENTIFIC_CONTEXT.md`;
- `docs/scientific/synthesis/RESEARCH_PROTOCOL.md` → `docs/development/context/RESEARCH_PROTOCOL.md`;
- `docs/scientific/synthesis/EVIDENCE_MATRIX.md` → `docs/development/context/EVIDENCE_MATRIX.md`;
- `docs/product/SOFTWARE_REQUIREMENTS.md` → `docs/development/context/SOFTWARE_REQUIREMENTS.md`;
- `docs/traceability/TRACEABILITY_MATRIX.md` → `docs/development/context/TRACEABILITY_MATRIX.md`;
- `docs/engineering/ARCHITECTURE.md` → arquitetura corrente descrita em `spo2-wrist-sensor/ReadME.md`, `docs/development/dev/Structure.md` e no código. Não há ADR canônico equivalente.

## 2. Índice por categoria solicitada

| Categoria | Documento principal | IDs |
|---|---|---|
| evidências da literatura | SCIENTIFIC_CONTEXT, EVIDENCE_MATRIX | EVD-001 a EVD-009 |
| observações do projeto | PROJECT_BRIEF, EVIDENCE_MATRIX | EVD-010 a EVD-015 |
| hipóteses científicas | HYPOTHESES | HYP-001 a HYP-007 |
| decisões metodológicas | PROJECT_BRIEF; `DECISIONS.md` ausente | DM-001 a DM-005 não verificáveis como decisões aceitas |
| requisitos clínicos | SOFTWARE_REQUIREMENTS | CR-001 a CR-012 |
| requisitos de software | SOFTWARE_REQUIREMENTS | SR-F-001 a 022; SR-NF-001 a 018 |
| arquitetura de engenharia | `spo2-wrist-sensor/ReadME.md`, `docs/development/dev/Structure.md`, código | componentes; ADR-001 a ADR-012 continuam não verificáveis sem `DECISIONS.md` |
| resultados preliminares | PROJECT_BRIEF, EVIDENCE_MATRIX | EVD-002, 006, 009, 014 |
| resultados confirmados | PROJECT_BRIEF | confirmação documental, não clínica |
| dúvidas e conflitos | LIMITATIONS, OPEN_RESEARCH_QUESTIONS | CFLT-001 a 010; ORQ-001 a 018 |

## 3. Índice de IDs

- Objetivos: OBJ-001..006
- Pergunta de pesquisa: RQ-001
- Construtos: CST-001..008
- Hipóteses: HYP-001..007; H0-001..006
- Evidências: EVD-001..019
- Não verificáveis: NV-001..005
- Requisitos clínicos: CR-001..012
- Requisitos funcionais: SR-F-001..022
- Requisitos não funcionais: SR-NF-001..018
- Componentes: CMP-001..015
- Decisões referenciadas: ADR-001..012; ODR-001..008; DM-001..005 — artefato canônico ausente, portanto não verificáveis como aceitas
- Métricas: MET-001..010
- Testes: TST-001..012
- Conflitos: CFLT-001..011
- Questões abertas: ORQ-001..018

## 4. Estado geral

| Área | Estado | Evidência mínima faltante |
|---|---|---|
| fundamentação histórica do I Blue It | documentada | atualização de versões recentes |
| arquitetura multimodal | documentada conceitualmente | publicação 2023/código atual |
| aquisição MAX30102 | implementação parcial no commit auditado | identificação por variante, testes FIFO/timestamp e HIL |
| qualidade de sinal | protótipo implementado, não validado | dataset rotulado e falso válido |
| HR/SpO2 | código preliminar; SpO₂ não calibrada | comparação contra referência e critérios aprovados |
| segurança/DeepDDA | prova de conceito/planejado | política aprovada e replay |
| persistência/dashboard | proposto | teste ponta a ponta |
| telemetria/rastreabilidade | parcial e com discrepância de contrato | RAW, `clinical_valid`, calibração, motivos, IDs e teste de contrato |
| testes de firmware | ausentes no repositório auditado | unidade, replay, integração e HIL versionados |
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
| SRC-PRJ-003 | versão histórica de `docs/development/dev/MELHORIA.md` no commit `a74986d...` | análise estática interna histórica | rastrear problemas observados antes da atualização documental | diverge do código do mesmo baseline em vários itens; usar SRC-PRJ-005 para o estado atual |
| SRC-PRJ-004 | Logs de oximetria reproduzidos em conversa de 2026-07-13 | observação interna | comportamento preliminar do protótipo | sensor/commit/condições não integralmente preservados; não é validação metrológica |
| SRC-PRJ-005 | Código e configuração do commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0` | artefato versionado do projeto | estado estático atual da implementação e inventário de testes | não houve build, execução HIL ou comparação com referência nesta auditoria |

### Convenção de localização

As citações internas usam `Fonte, PDF p. N` para PDFs e `Fonte, seção/linhas` para documentos vivos. O número de página é o número do arquivo PDF, não necessariamente a paginação impressa. Afirmações de conversa usam data e tema; quando o trecho completo não estava disponível, recebem a marca **NÃO VERIFICÁVEL NESTA EXECUÇÃO**.
