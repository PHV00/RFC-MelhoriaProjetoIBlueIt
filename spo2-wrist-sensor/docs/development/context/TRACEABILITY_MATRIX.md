> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.


# TRACEABILITY_MATRIX

## 1. Objetivo -> hipótese -> requisito -> teste -> métrica -> artigo

| Objetivo | Hipótese | Evidência base | Requisitos | Testes | Métricas | Seção de artigo sugerida | Estado |
|---|---|---|---|---|---|---|---|
| OBJ-001 | HYP-001 | EVD-013/EVD-015 | SR-F-001 a 006, SR-NF-002/007 | TST-001/TST-002 | MET-009/MET-010 | Materiais e Métodos - hardware/aquisição | parcial |
| OBJ-002 | HYP-002/HYP-003 | EVD-006/EVD-014/EVD-016 | SR-F-007 a 012 | TST-003 a 005 | MET-001 a 006 | Métodos - processamento; Resultados - concordância | lacuna crítica |
| OBJ-003 | HYP-004 | EVD-007/EVD-010 | CR-003 a 006; SR-F-017/018/022 | TST-006/TST-007/TST-010 | MET-007 + matriz de ações | Métodos - política; Resultados - cenários | planejado |
| OBJ-004 | HYP-005 | EVD-010/EVD-011 | SR-F-015/016/018/020/021 | TST-008/TST-011 | completude/reconstrução | Arquitetura e rastreabilidade | planejado/parcial |
| OBJ-005 | HYP-001 a 006 | EVD-016 | todos os técnicos | RP-01/RP-02/RP-03 | MET-001 a 010 | Protocolo e validação | não executado |
| OBJ-006 | HYP-007 | EVD-002/EVD-008/EVD-017 | CR-001 a 012 | RP-03/RP-04/RP-05 | usabilidade, segurança, clínicos | Discussão/estudo futuro | não iniciado |

## 2. Catálogo de testes

| ID | Teste | Tipo | Oráculo | Artefato esperado |
|---|---|---|---|---|
| TST-001 | taxa produzida/consumida, FIFO e overflow | unidade/HIL | contadores esperados | log e gráfico |
| TST-002 | timestamp, sequência, gaps e replay | unidade/integração | sinal conhecido | relatório determinístico |
| TST-003 | contato, clipping, ruído, movimento e baixa perfusão | unidade/replay | rótulos pré-definidos | matriz de qualidade |
| TST-004 | falso válido e transição LOW_CONFIDENCE | sistema | casos inválidos | matriz de confusão |
| TST-005 | comparação HR/SpO2 com referência | validação técnica | equipamento de referência | dataset + análise |
| TST-006 | queda transitória/sustentada e sinal inválido | replay/sistema | política aprovada | ações esperadas/obtidas |
| TST-007 | regras e persistência por perfil | funcional | tabela de decisão | cobertura de regras |
| TST-008 | reconstrução de decisão DeepDDA | auditoria | evento original | relatório de completude |
| TST-009 | latência/carga/perda de frames | desempenho | orçamento aprovado | percentis e uso de recursos |
| TST-010 | override, pausa, interrupção e retomada | sistema/usabilidade | fluxo clínico | evidência de interface/log |
| TST-011 | dashboard, tendência e alertas | funcional/UFE | requisitos de interface | checklist + feedback |
| TST-012 | distinção de alerta técnico/fisiológico | funcional/UFE | cenários | taxa de interpretação correta |

## 3. Tabela final de lacunas

Legenda: `OK` existe; `PARC` existe parcialmente; `NAO` ausente; `NV` não verificável; `FUT` fase futura.

| Objetivo/Hipótese | Evidência | Requisito | Implementação | Teste | Métrica | Seção no artigo | Lacuna principal |
|---|---:|---:|---:|---:|---:|---:|---|
| OBJ-001 / HYP-001 | PARC | OK | NV/PARC | NAO | OK | PARC | fixar commit e demonstrar FIFO/timestamp |
| OBJ-002 / HYP-002 | PARC | OK | PARC | NAO | OK | PARC | validar qualidade e falso válido |
| OBJ-002 / HYP-003 | NAO | OK | PARC | NAO | OK | PARC | comparação contra referência e curva calibrada |
| OBJ-003 / HYP-004 | PARC | OK | NV/PARC | NAO | PARC | PARC | política clínica e cenários com oráculo independente |
| OBJ-004 / HYP-005 | PARC | OK | NV/PARC | NAO | PARC | PARC | comprovar persistência e reconstrução ponta a ponta |
| OBJ-005 / HYP-006 | NAO | PARC | NV | NAO | OK | PARC | medir latência/carga no sistema integrado |
| OBJ-006 / HYP-007 | NAO | PARC | FUT | FUT | FUT | FUT | protocolo clínico, ética, amostra e desfechos |
| Atualização do estado da arte | PARC | n/a | n/a | n/a | n/a | NAO | busca 2021-2026 não realizada neste pacote |
| Validação dos limiares 95/89 | PARC | PARC | PARC | NAO | NAO | PARC | justificativa e aplicabilidade por população |
| Segurança/privacidade/regulação | PARC | PARC | NV | NAO | NAO | PARC | intended use e análise de risco formal |
| Reprodutibilidade | PARC | OK | NV | NAO | PARC | PARC | dataset, scripts, commit e relatório executável |

## 4. Gate para submissão do artigo técnico

Antes de submissão como implementação de oxímetro integrado, devem estar `OK`: OBJ-001, HYP-001, HYP-002, HYP-003, testes TST-001 a TST-005, MET-001 a MET-010 aplicáveis, commit/dataset e discussão clara de não eficácia clínica.

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
