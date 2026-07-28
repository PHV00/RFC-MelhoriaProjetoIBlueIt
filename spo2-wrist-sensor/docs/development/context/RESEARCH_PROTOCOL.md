> Pacote de contexto científico e de engenharia - I Blue It / oximetria
> Gerado em: 2026-07-15
> Natureza: síntese rastreável de literatura, documentos do projeto, conversas e repositório público.
> Regra de interpretação: conversas e documentos de projeto são evidência interna do processo, não evidência científica externa nem validação clínica.
> Baseline de código auditado: commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0`, por inspeção estática.


# RESEARCH_PROTOCOL

## 1. Estado do protocolo

**PROTOCOLO PROPOSTO — ainda não executado integralmente.** O código atual contém partes necessárias para RP-01, como leitura de IDs, FIFO em lote, qualidade, HR, SpO₂ e confiança. Entretanto, não foram localizados testes automatizados, relatórios HIL, dataset completo, referência simultânea ou relatório de execução anexado. Presença de código não equivale à passagem de RP-01.

## 2. Objetivo primário

**OBJ-005:** estimar o desempenho técnico do módulo MAX30102 no pipeline I Blue It, comparando HR e SpO2 contra equipamento de referência em condições controladas e documentando falhas, sinal inválido, latência e rastreabilidade.

## 3. Fases

| Fase | Escopo | Participantes | Saída | Estado |
|---|---|---|---|---|
| RP-01 | validação de driver/FIFO/sinal com simulador e bancada | nenhum humano | logs determinísticos, testes unitários e HIL | implementação parcial; execução/evidência ausente |
| RP-02 | teste técnico em voluntários hígidos | adultos, mediante aprovação aplicável | pares referência-protótipo e qualidade | não iniciado formalmente |
| RP-03 | teste de usabilidade/segurança operacional com profissionais | fisioterapeutas/outros UFE | falhas de interface e decisões | não iniciado para módulo novo |
| RP-04 | viabilidade em população clínica | pacientes definidos por protocolo | segurança, completude e viabilidade | fora da fase atual |
| RP-05 | eficácia clínica | amostra e desenho apropriados | desfechos clínicos | fora do escopo atual |

## 4. Hipóteses cobertas

RP-01 testa HYP-001, HYP-002, HYP-005 e HYP-006. RP-02 testa HYP-003. RP-03 testa aspectos de HYP-004 e HYP-005. HYP-007 exige RP-04/RP-05.

## 5. Unidade de análise

- Janela PPG válida (`WIN-ID`) com início/fim, número de amostras e configuração.
- Par simultâneo protótipo-referência (`PAIR-ID`).
- Evento de qualidade/alerta/decisão (`EVT-ID`).
- Sessão completa (`SESSION-ID`).

## 6. Configuração mínima a registrar

`commit`, `build`, sensor/part/revision, placa, pinos I2C, endereço, sample rate, average, pulse width, ADC range, corrente RED/IR, modo FIFO, temperatura quando disponível, algoritmo e versão de calibração, filtros, tamanho/overlap da janela, posição do sensor, dispositivo de referência e versão, horário sincronizado.

## 7. Procedimento RP-01 - bancada e firmware

Baseline inicial para a próxima execução: commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0`. Antes do primeiro resultado, deve ser gerado um novo identificador de build e confirmada a identidade física do módulo. Se o código mudar, a execução deve registrar o novo commit; este baseline não deve ser reutilizado como se representasse versões futuras.

1. Fixar commit e gerar artefato de build identificável.
2. Executar autoteste: PART_ID, REV_ID, registradores, FIFO, LEDs e canais.
3. Verificar taxa produzida versus taxa consumida por pelo menos 30 minutos.
4. Forçar overflow, desconexão I2C, leituras vazias e recuperação.
5. Validar timestamps por contador de amostra/taxa configurada, não apenas horário de retirada da FIFO.
6. Alimentar sinais gravados/sintéticos para testar filtros, qualidade, detecção de pico, HR, SpO2, confiança e estados.
7. Confirmar que sinal inválido não gera ação clínica como se fosse normal.
8. Medir latência fim a fim e perda de frames.

## 8. Procedimento RP-02 - comparação técnica

### 8.1 Pré-condições

- aprovação ética/consentimento quando aplicável;
- critérios de inclusão/exclusão definidos por equipe clínica;
- equipamento de referência identificado e dentro da manutenção/calibração;
- sincronização temporal validada;
- protocolo de segurança e interrupção independente do protótipo.

### 8.2 Coleta

- período de estabilização antes de registrar;
- medições simultâneas em repouso e condições planejadas de variação segura;
- registrar mão/dedo, perfusão, movimento, luz ambiente, temperatura e artefatos;
- repetir medidas por participante e condição;
- manter também resultados inválidos e falhas.

### 8.3 Métricas

| ID | Métrica | Definição |
|---|---|---|
| MET-001 | cobertura válida | janelas válidas / janelas totais |
| MET-002 | MAE-SpO2 | média de `abs(SpO2_prot - SpO2_ref)` em pares válidos |
| MET-003 | viés-SpO2 | média de `SpO2_prot - SpO2_ref` |
| MET-004 | limites de concordância | viés +/- 1,96 DP das diferenças, quando pressupostos forem adequados |
| MET-005 | RMSE-HR | raiz do erro quadrático médio de HR |
| MET-006 | taxa de falso válido | saída marcada válida em janela tecnicamente inválida |
| MET-007 | taxa de falso alerta | alertas sem condição de referência predefinida |
| MET-008 | latência p95 | percentil 95 entre amostra válida e disponibilidade/ação |
| MET-009 | perda de amostras | diferença entre amostras esperadas e consumidas, incluindo overflow |
| MET-010 | recuperação | tempo e sucesso após desconexão/erro |

Critérios numéricos de aceitação para MET-002 a MET-005 não são inventados neste protocolo: devem ser aprovados antes da coleta com base no intended use, referência normativa aplicável e especialista clínico/metrológico.

## 9. Procedimento RP-03 - sistema e decisão

Usar cenários simulados e replay de sinais para verificar: normalidade, baixa perfusão, movimento, dedo ausente, queda transitória, queda sustentada, desconexão, atraso, valor impossível, conflito entre SpO2 e qualidade, ação manual do terapeuta e recuperação. O oráculo esperado deve ser definido por caso de teste, não aprendido a partir do resultado do próprio sistema.

## 10. Plano de análise

- Pré-registrar exclusões e nunca apagar dados negativos.
- Apresentar resultados por participante, condição, sensor, versão e faixa de SpO2 observada.
- Separar análise de janelas válidas da taxa de cobertura/invalidade.
- Não usar correlação isolada como evidência de concordância.
- Reportar intervalos de confiança quando amostra permitir.
- Publicar tabela de falhas e mudanças pós-teste.

## 11. Segurança e ética

O protótipo não deve conduzir exercício, reduzir ou interromper sessão de forma autônoma em estudo humano sem supervisão e protocolo aprovados. A decisão clínica final pertence ao profissional. Dados fisiológicos são sensíveis. O protocolo deve definir minimização, controle de acesso, retenção, anonimização/pseudonimização e procedimento de incidente.

## 12. Critério de passagem

RP-01 deve passar antes de RP-02; RP-02 antes de usar SpO2 em decisões em tempo real; RP-03 antes de RP-04. “Número plausível na serial” não é critério de passagem.

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
