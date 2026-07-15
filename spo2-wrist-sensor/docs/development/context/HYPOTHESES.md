> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.


# HYPOTHESES

## 1. Hipóteses técnicas e científicas

| ID | Hipótese | Variáveis | Métricas | Teste | Estado |
|---|---|---|---|---|---|
| HYP-001 | Corrigir consumo da FIFO, timestamps e taxa efetiva reduzirá perda de amostras e instabilidade do PPG. | política FIFO, taxa, overflow | MET-009, jitter, cobertura | TST-001/TST-002 | não testada formalmente |
| HYP-002 | Um gate de qualidade multicomponente reduzirá saídas falsamente válidas sem eliminar de forma excessiva janelas utilizáveis. | thresholds de qualidade, contato, ruído | MET-001, MET-006, sensibilidade/especificidade do gate | TST-003/TST-004 | protótipo parcial |
| HYP-003 | Após calibração documentada, a estimativa de SpO2 apresentará erro e concordância dentro de critérios pré-especificados contra referência. | algoritmo, curva, corrente, janela | MET-002 a MET-004 | TST-005 | não testada |
| HYP-004 | Usar somente SpO2 válida e persistente em um controlador de segurança reduzirá decisões inadequadas em cenários simulados de risco. | qualidade, persistência, thresholds | MET-007, matriz de confusão de ações | TST-006/TST-007 | não testada |
| HYP-005 | Registrar estado, ação, recompensa, parâmetros e resultado permitirá reconstruir decisões do DeepDDA. | completude de logs | taxa de reconstrução/auditoria | TST-008 | planejada |
| HYP-006 | A integração de oximetria não aumentará a latência fim a fim além do limite aprovado para gameplay/segurança. | carga CPU, telemetria, algoritmo | MET-008, dropped frames | TST-009 | meta de 800 ms não validada |
| HYP-007 | O monitoramento com oximetria/DeepDDA melhora segurança, adesão ou resultados terapêuticos. | intervenção clínica | eventos adversos, adesão, desfechos | estudo clínico futuro | não testável na fase atual |

## 2. Hipóteses nulas

- H0-001: a correção da FIFO não altera perda/jitter de modo mensurável.
- H0-002: o gate de qualidade não reduz falsos válidos ou reduz excessivamente a cobertura.
- H0-003: o protótipo não atinge o critério de erro/concordância definido previamente.
- H0-004: o controlador não melhora a classificação de ações esperadas em replay/simulação.
- H0-005: decisões não podem ser reconstruídas de modo completo a partir dos registros.
- H0-006: a integração excede o orçamento de latência ou degrada coleta/gameplay.

## 3. Hipóteses retiradas ou rebaixadas

| ID | Formulação anterior implícita | Tratamento atual |
|---|---|---|
| RH-001 | “Valores plausíveis indicam que o oxímetro funciona.” | rejeitada; plausibilidade não mede exatidão |
| RH-002 | “Acurácia igual a equipamento homologado.” | reescrita como HYP-003 com métrica e critério a definir |
| RH-003 | “Acurácia da IA >85%.” | suspensa; classes, dataset, oráculo e custo de erro não definidos |
| RH-004 | “SpO2 <95 reduz e <89 interrompe para qualquer paciente.” | rebaixada a regra de prova de conceito/configuração, sujeita a revisão clínica |
| RH-005 | “Utilidade percebida prova eficácia terapêutica.” | rejeitada; construtos distintos |

## 4. Dependências

HYP-003 depende de HYP-001 e HYP-002. HYP-004 depende de HYP-003. HYP-007 depende de todas as anteriores e de protocolo clínico independente.

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
