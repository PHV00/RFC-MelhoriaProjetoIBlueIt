> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.


# OPEN_RESEARCH_QUESTIONS

## Questões abertas prioritárias

| ID | Questão | Por que importa | Evidência necessária | Prioridade |
|---|---|---|---|---|
| ORQ-001 | Qual é o intended use exato do módulo: monitoramento informativo, recomendação ou controle automático? | define risco, requisitos e validação | decisão clínica/regulatória | crítica |
| ORQ-002 | Qual configuração do MAX30102 maximiza qualidade sem saturar ADC ou exceder consumo? | afeta PPG e calibração | experimento fatorial/bancada | crítica |
| ORQ-003 | Como garantir consumo integral da FIFO e timestamps representativos? | base de todos os cálculos | TST-001/TST-002 | crítica |
| ORQ-004 | Quais features de qualidade predizem erro de HR/SpO2? | evita falso válido | dataset com referência e artefatos | crítica |
| ORQ-005 | Qual algoritmo e curva de calibração são apropriados ao módulo físico atual? | determina exatidão | referência simultânea e documentação | crítica |
| ORQ-006 | Qual tempo de estabilização e duração de persistência são necessários? | evita decisão precoce/transitória | análise temporal | alta |
| ORQ-007 | Como diferenciar queda fisiológica de movimento/baixa perfusão? | segurança | sinais rotulados e referência | alta |
| ORQ-008 | Quais limiares e ações são adequados por população/protocolo? | regra 95/89 pode não ser universal | revisão clínica e estudo | crítica |
| ORQ-009 | O DeepDDA deve executar ou apenas recomendar ações? | automação e responsabilidade | análise de risco/UFE | crítica |
| ORQ-010 | Qual combinação de SpO2, HR, fluxo, Borg e desempenho é informativa sem redundância? | Flow multimodal | estudo de modelagem | média |
| ORQ-011 | Como medir confiança de modo calibrado, não apenas heurístico? | interpretação do valor | reliability/calibration curves | alta |
| ORQ-012 | Qual taxa de falso alerta é aceitável sem induzir alarm fatigue? | usabilidade/segurança | RP-03 | alta |
| ORQ-013 | A latência de 800 ms é adequada para aquisição, dashboard e ação? | requisito atual arbitrário | medição e análise de risco | alta |
| ORQ-014 | Que dados brutos devem ser retidos em produção? | reprodutibilidade versus privacidade | arquitetura/governança | média |
| ORQ-015 | O monitoramento melhora segurança ou apenas detecta eventos? | alegação clínica | estudo prospectivo | futura |
| ORQ-016 | Há benefício em adesão/desfecho quando SpO2 alimenta o DeepDDA? | eficácia clínica | ensaio/estudo adequado | futura |
| ORQ-017 | Como o algoritmo se comporta em diferentes tons de pele/perfusão/idades? | generalização/equidade | amostra estratificada | crítica antes de alegação ampla |
| ORQ-018 | Quais padrões/regulamentos se aplicam ao produto final no Brasil? | qualidade e comercialização | análise regulatória atual | crítica |

## Questões de redação científica

- O artigo será de implementação e validação técnica do módulo, de integração ao I Blue It ou de avaliação clínica? Misturar os três sem dados correspondentes enfraquece a contribuição.
- Qual é a novidade: algoritmo de SpO2, engenharia do pipeline, integração multimodal/DeepDDA, rastreabilidade ou estudo de validação?
- Quais artefatos serão disponibilizados: código, dados brutos, protocolo, scripts, configuração e modelo?
- Como o trabalho se diferencia da prova de conceito da tese de 2024 sem alegar que ela já validou o hardware atual?

## Perguntas que devem ser respondidas antes de coleta com pacientes

1. intended use e operador;
2. risco e ações permitidas;
3. equipamento de referência;
4. critérios de inclusão/exclusão e interrupção;
5. erro aceitável e tratamento de inválidos;
6. aprovação ética e consentimento;
7. governança dos dados;
8. supervisão clínica e resposta a evento adverso.

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
