> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.


# EVIDENCE_MATRIX

## 1. Escala

- **E1 - externa revisada por pares/tese:** literatura fornecida.
- **E2 - documento técnico do projeto:** requisitos, arquitetura ou análise de código.
- **E3 - observação experimental interna:** log/teste sem protocolo completo.
- **E4 - hipótese/decisão:** não é evidência empírica.

## 2. Matriz de evidências

| ID | Afirmação | Classe | Fonte/local | Força | Limitações |
|---|---|---|---|---|---|
| EVD-001 | O I Blue It foi concebido como sistema biomédico com jogo e PITACO para RR. | literatura | SRC-LIT-001, PDF pp. 28, 74; SRC-LIT-002 | E1 | histórico; não valida módulo atual |
| EVD-002 | A avaliação histórica envolveu 106 atores, 85 UFE e 15 avaliações, com utilidade percebida positiva. | literatura/resultado | SRC-LIT-001, PDF pp. 119-121 | E1 | percepção de utilidade, não desfecho clínico |
| EVD-003 | O dispositivo histórico não buscava precisão/exatidão de pneumotacógrafo comercial e não reportou diagnóstico. | literatura/limitação | SRC-LIT-001, PDF p. 29 | E1 | reforça que novas alegações exigem novo protocolo |
| EVD-004 | O MSL de 2020 selecionou 28/7.918 estudos e encontrou IA explícita em 3. | literatura secundária | SRC-LIT-003, resumo/resultados | E1 | busca encerrada em 2020 |
| EVD-005 | A 123-SGR integra flexibilidade, complementaridade e segurança. | literatura arquitetural | SRC-LIT-004, seções I e IV | E1 | conceito/prova, não desempenho clínico |
| EVD-006 | A tese usa DeepDDA/PPO e monitoramento de SpO2 como prova de conceito. | literatura | SRC-LIT-005, PDF pp. 95, 119-123 | E1 | não é integração operacional definitiva do sensor atual |
| EVD-007 | A tese propõe reduzir velocidade abaixo de 95% e interromper abaixo de 89%. | literatura/regra proposta | SRC-LIT-005, PDF p. 119 | E1 | não demonstrada como regra universal; figura simplifica a lógica |
| EVD-008 | O treinamento do DeepDDA usou dados sintéticos baseados em um indivíduo hígido. | literatura/limitação | SRC-LIT-005, PDF pp. 125 e 149 | E1 | limita generalização clínica |
| EVD-009 | Foram descritos testes de unidade, integração, funcionais, sistema e avaliação por UFE. | literatura/resultado técnico | SRC-LIT-005, PDF pp. 132-137 | E1 | relatório detalhado, casos e artefatos não fornecidos |
| EVD-010 | O projeto atual pretende integrar SpO2, DeepDDA, dashboard e registro de decisões. | decisão do projeto | SRC-PRJ-002, objetivos/RF | E2 | plano, não execução confirmada |
| EVD-011 | O firmware é organizado em camadas drivers/sensing/processing/safety/transport/storage/app. | arquitetura interna | SRC-PRJ-002/003 | E2 | aderência real depende do código/commit |
| EVD-012 | Uma análise estática registrou estimadores ausentes e pipeline principal limitado a RAW. | observação de código | SRC-PRJ-003, problemas 1.1-1.3 | E2 | possivelmente desatualizada |
| EVD-013 | A mesma análise apontou risco de produção a 100 sps e consumo a ~20 sps, sem controle da FIFO. | observação de código | SRC-PRJ-003, problemas 1.4-1.7 | E2 | precisa ser revalidada no commit atual |
| EVD-014 | Logs posteriores exibiram HR/SpO2 e confiança variáveis. | observação interna | SRC-PRJ-004 | E3 | sem referência, configuração completa ou commit |
| EVD-015 | O módulo anterior foi identificado como PART_ID 0x15/REV_ID 0x06; o atual foi trocado para MAX30102. | observação interna | SRC-PRJ-001 | E3 | impede transferir validação entre variantes |
| EVD-016 | Não existe no material fornecido comparação do MAX30102 com equipamento homologado. | ausência de evidência | conjunto de fontes | E2 | lacuna central |
| EVD-017 | Não existe evidência fornecida de eficácia clínica incremental da oximetria/DeepDDA. | ausência de evidência | conjunto de fontes | E1/E2 | requer estudo futuro |

## 3. Evidências negativas preservadas

- Instabilidade de HR/SpO2 nos logs exploratórios.
- Primeira leitura inválida seguida de leituras válidas com confiança baixa/moderada.
- Risco histórico de FIFO, timestamp e processamento não integrados.
- Uso de dados sintéticos e de um único indivíduo hígido no DeepDDA.
- Falta de precisão comercial no PITACO original.
- Ausência de comparação metrológica do MAX30102 atual.

## 4. Conteúdo não verificável

| ID | Conteúdo | Motivo |
|---|---|---|
| NV-001 | estado exato do firmware que gerou os logs de 2026-07-13 | commit/build não anexado |
| NV-002 | identidade do sensor usado em cada log após a troca | log não inclui sempre PART_ID/REV_ID |
| NV-003 | validação clínica dos limiares 95%/89% para a população-alvo | não há protocolo/estudo no material |
| NV-004 | cumprimento dos KPIs de 800 ms, 85% e equivalência a homologado | metas sem relatório de medição |
| NV-005 | integração atual com Unity, MongoDB e dashboard | modelo/requisito, sem execução demonstrada nesta análise |

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
