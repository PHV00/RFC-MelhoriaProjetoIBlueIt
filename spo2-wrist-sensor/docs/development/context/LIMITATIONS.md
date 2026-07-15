> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.


# LIMITATIONS

## 1. Limitações da literatura fornecida

- SRC-LIT-001/002 são de 2018 e representam versões iniciais. Demonstram utilidade percebida, não precisão clínica ou eficácia.
- SRC-LIT-003 encerrou busca em 2020; não sustenta sozinho alegação de lacuna atual.
- SRC-LIT-004 é versão 2020 da arquitetura; a tese cita versão 2023 não anexada.
- SRC-LIT-005 é prova de conceito, usa dados simulados de um indivíduo hígido e não captura variabilidade de pacientes (PDF p. 149).
- Referências citadas dentro dos PDFs não foram individualmente auditadas neste pacote; não foram inventados DOI/autores além dos próprios documentos.

## 2. Limitações do material de projeto

- O histórico completo de todos os chats não estava disponível como exportação; foram usados tópicos e trechos presentes no contexto desta execução.
- O repositório foi lido pela interface pública, sem clone local e sem hash fixado; conteúdo pode mudar.
- O commit/binário que gerou cada log não foi fornecido.
- Não há dataset bruto anexado, equipamento de referência ou protocolo simultâneo.
- A troca MAX30105 -> MAX30102 torna parte dos resultados anteriores obsoleta para a variante atual.

## 3. Conflitos

| ID | Conflito | Impacto | Resolução necessária |
|---|---|---|---|
| CFLT-001 | SRC-PRJ-003 diz que HR/SpO2 eram stubs; logs posteriores mostram valores. | estado de implementação incerto | auditar HEAD e commit do log |
| CFLT-002 | PART_ID 0x15/REV 0x06 (MAX30105 provável) versus MAX30102 atual. | resultados não transferíveis | autoteste e nova caracterização |
| CFLT-003 | texto da tese usa 95%/89%; figura central exibe principalmente decisão `SpO2 > 89%`. | ambiguidade de política | formalizar tabela de decisão completa |
| CFLT-004 | tese usa linguagem de “garantir eficácia/segurança”, mas treinamento foi sintético e sem pacientes. | risco de sobreafirmação | reescrever como prova técnica/potencial |
| CFLT-005 | KPI “acurácia igualada a homologado” não define métrica/tolerância. | não testável | definir MAE, viés, LoA e critérios |
| CFLT-006 | KPI “IA >85%” não define classes, dataset ou custo de erro. | métrica inválida | definir tarefa/oráculo ou remover |
| CFLT-007 | objetivo de software clínico versus escopo histórico não diagnóstico. | risco regulatório e de comunicação | fechar intended use |
| CFLT-008 | confiança nos logs é baixa/moderada, mas `valid=true` aparece cedo. | falso válido possível | estabilização e gate de qualidade |
| CFLT-009 | arquitetura proposta inclui MongoDB/DeepDDA/dashboard, mas implementação corrente do repo principal não foi demonstrada. | rastreabilidade não confirmada | teste ponta a ponta |
| CFLT-010 | metas de 800 ms aparecem sem análise de risco ou medição. | pode ser inadequado/excessivo | medir por função e justificar |

## 4. Limitações técnicas previsíveis

PPG de dedo é sensível a movimento, perfusão, posicionamento, luz, pressão de contato, pigmentação/tecido, temperatura e diferenças de hardware. Um único limiar de amplitude não basta. A curva de SpO2 depende do sistema óptico e precisa ser validada. Valores de HR podem sofrer detecção dupla/metade. Backlog de FIFO pode deslocar temporalmente o sinal.

## 5. Limitações clínicas

- SpO2 não é medida suficiente para caracterizar esforço, segurança ou adequação terapêutica.
- Limiares podem variar por população, prescrição e contexto; o pacote não define conduta clínica.
- Redução/interrupção automatizada pode causar tanto atraso de intervenção quanto interrupções desnecessárias.
- Utilidade percebida por especialista não mede eventos adversos, adesão longitudinal ou desfecho funcional.

## 6. Documentos possivelmente desatualizados

| Documento | Motivo | Ação |
|---|---|---|
| SRC-LIT-001/002 | versões 1.0/2018 | usar apenas histórico |
| SRC-LIT-003 | estado da arte até 2020 | atualizar revisão |
| SRC-LIT-004 | versão 2020; versão 2023 ausente | obter publicação mais recente |
| SRC-PRJ-003 | conflita com logs posteriores | reexecutar análise estática no commit atual |
| README/requisitos do repo | documento vivo e metas ainda propostas | fixar versão para artigo |

## 7. Revisão humana obrigatória

1. fisioterapeuta/pneumologista: intended use, limiares, persistência e mensagens;
2. engenheiro biomédico/metrologista: referência, calibração, erro e protocolo;
3. especialista em firmware: FIFO, timestamps, configuração MAX30102 e testes HIL;
4. segurança/privacidade: dados sensíveis e controle de acesso;
5. ética/regulação: estudos com humanos e classificação aplicável;
6. orientador/autores do ecossistema: compatibilidade com I Blue It 5.0/123-SGR mais recente.

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
