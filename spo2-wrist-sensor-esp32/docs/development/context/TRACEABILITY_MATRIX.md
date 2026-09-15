> Pacote de contexto científico e de engenharia — I Blue It / oximetria
> Baseline de implementação auditada: commit `a74986d93098367b9e0b2085ed6d113f29ffa1d0`, em 2026-07-15.
> Natureza da auditoria: inspeção estática. Não houve build, execução em hardware, replay ou validação clínica nesta atualização.

# TRACEABILITY_MATRIX

## 1. Como interpretar os estados

- **confirmada:** a ligação está explícita, possui artefatos identificáveis e não conflita com o baseline auditado;
- **parcial:** a ligação existe, mas falta parte da implementação, evidência executável, critério ou resultado;
- **ausente:** não foi localizado artefato que materialize a ligação;
- **contraditória:** os artefatos ligados expressam comportamentos incompatíveis;
- **não verificável:** há uma alegação ou destino proposto, mas o artefato necessário não está disponível neste repositório.

Os estados descrevem o commit auditado e o material disponível. Uma ligação parcial ou ausente representa trabalho pendente, não uma limitação permanente nem uma decisão de abandonar o requisito.

## 2. Objetivo/hipótese → evidência → requisito → implementação → teste → métrica → artigo

Cada célula classifica a ligação com a célula imediatamente anterior.

| Objetivo ou hipótese | Evidência | Requisito | Implementação no baseline | Teste atual | Métrica | Seção do artigo |
|---|---|---|---|---|---|---|
| OBJ-001 / HYP-001 — aquisição, FIFO e tempo | **parcial** — EVD-013 registra o problema histórico; EVD-018 confirma correções estáticas, sem comprovação HIL | **confirmada** — SR-F-001 a 006; SR-NF-002/006/007/008 | **parcial** — lê IDs, verifica registradores, drena FIFO em lote, detecta overflow e reconstrói timestamps; ainda não seleciona variante explicitamente, não detecta gaps de origem e não preserva RAW | **ausente** — TST-001/TST-002 não possuem código ou relatório no repositório | **parcial** — MET-009/MET-010 estão definidas, sem resultados | **não verificável** — seção sugerida: Materiais e Métodos / hardware e aquisição; manuscrito não vinculado |
| OBJ-002 / HYP-002 — qualidade e falso válido | **parcial** — EVD-014 contém observação exploratória; EVD-018 confirma o gate no código, sem dataset rotulado | **confirmada** — SR-F-007 a 009 e 012; SR-NF-011/013 | **parcial** — há tendência, AC RMS, ruído residual, SNR, perfusão, correlação, clipping e continuidade; limiares/pesos ainda são heurísticos e não foram validados contra erro | **ausente** — TST-003/TST-004 não possuem implementação ou resultado | **parcial** — MET-001/MET-006 estão definidas, sem matriz de confusão | **não verificável** — seção sugerida: Métodos / qualidade e Resultados / cobertura; manuscrito não vinculado |
| OBJ-002 / HYP-003 — HR/SpO₂ contra referência | **ausente** — EVD-016 confirma que não há comparação do MAX30102 atual com referência | **confirmada** — SR-F-010 a 012; SR-NF-004/008 | **parcial** — HR possui baseline por picos/IBI; SpO₂ calcula razão dos quocientes, mas a curva está `calibrated=false` e permite estimativa experimental | **ausente** — TST-005 e RP-02 não foram executados nem possuem artefatos anexados | **parcial** — MET-002 a MET-005 estão definidas, sem critérios numéricos aprovados ou resultados | **não verificável** — seções sugeridas: Métodos / estimadores e Resultados / concordância; manuscrito não vinculado |
| OBJ-003 / HYP-004 — política de segurança | **parcial** — EVD-007 sustenta apenas uma regra de prova de conceito; não valida política clínica universal | **confirmada** — CR-003 a 006/008/010; SR-F-017/022 | **ausente** — a camada `safety` constrói confiança, mas não implementa persistência, histerese, limiar clínico, ação de sessão ou override | **ausente** — TST-006/TST-007/TST-010/TST-012 não possuem artefatos executáveis | **parcial** — MET-007 está definida; a matriz de ações e o custo de erro ainda não estão formalizados | **não verificável** — seção sugerida: Métodos / política de segurança e Resultados / cenários; manuscrito não vinculado |
| OBJ-004 / HYP-005 — rastreabilidade ponta a ponta | **parcial** — EVD-010 é plano e EVD-018 confirma apenas telemetria local parcial | **confirmada** — SR-F-006/015/016/018/020/021; SR-NF-008/009/015/016 | **contraditória** — o modelo interno separa `valid` e `clinical_valid`, mas a telemetria ativa omite `clinical_valid`, calibração, motivos, overflow e status; RAW está desativado e não há paciente/sessão/firmware | **ausente** — TST-008/TST-011 não possuem implementação ou relatório | **ausente** — taxa de completude/reconstrução ainda não possui ID MET nem resultado | **não verificável** — seção sugerida: Arquitetura e rastreabilidade; manuscrito não vinculado |
| OBJ-005 / HYP-006 — validação técnica e latência | **ausente** — EVD-016/EVD-019 registram ausência de validação e de testes atuais | **parcial** — os requisitos técnicos existem, mas orçamento aprovado e critérios de aceitação continuam abertos | **parcial** — há firmware PPG/HR/SpO₂ isolado; integração fim a fim com jogo, backend e dashboard não está neste repositório | **ausente** — TST-009 e execução integral de RP-01/RP-02/RP-03 não foram localizados | **parcial** — MET-008 está definida; 800 ms continua meta não validada | **não verificável** — seções sugeridas: Protocolo e validação técnica; manuscrito não vinculado |
| OBJ-006 / HYP-007 — viabilidade e eficácia clínica | **ausente** — EVD-017 registra ausência de eficácia clínica incremental | **parcial** — CR-001 a 012 existem, mas intended use, população, limiares e protocolo ainda exigem aprovação humana | **ausente** — não existe intervenção clínica implementada ou autorizada neste baseline | **ausente** — RP-03/RP-04/RP-05 não foram executados para o módulo atual | **ausente** — desfechos, amostra e critérios clínicos não estão definidos para teste de eficácia | **não verificável** — seção apropriada no estado atual: Discussão / trabalho futuro; não há seção de resultados clínicos sustentada |

## 3. Estado dos testes definidos

Não foram localizados diretórios ou fontes de testes automatizados para o firmware. Os IDs abaixo continuam sendo especificações de testes a implementar e executar.

| ID | Teste especificado | Tipo | Estado no baseline | Artefato necessário |
|---|---|---|---|---|
| TST-001 | taxa produzida/consumida, FIFO e overflow | unidade/HIL | ausente | código, log, configuração e gráfico |
| TST-002 | timestamp, sequência, gaps e replay | unidade/integração | ausente | sinal conhecido e relatório determinístico |
| TST-003 | contato, clipping, ruído, movimento e baixa perfusão | unidade/replay | ausente | dataset rotulado e matriz de qualidade |
| TST-004 | falso válido e LOW_CONFIDENCE | sistema | ausente | casos inválidos e matriz de confusão |
| TST-005 | comparação HR/SpO₂ com referência | validação técnica | ausente | dataset pareado e análise |
| TST-006 | queda transitória/sustentada e sinal inválido | replay/sistema | ausente | política aprovada e ações esperadas |
| TST-007 | regras e persistência por perfil | funcional | ausente | tabela de decisão aprovada |
| TST-008 | reconstrução de decisão DeepDDA | auditoria | ausente | evento original e relatório de completude |
| TST-009 | latência, carga e perda de frames | desempenho | ausente | orçamento e percentis medidos |
| TST-010 | override, pausa, interrupção e retomada | sistema/usabilidade | ausente | fluxo clínico, interface e log |
| TST-011 | dashboard, tendência e alertas | funcional/UFE | ausente | sistema integrado e checklist |
| TST-012 | alerta técnico versus fisiológico | funcional/UFE | ausente | cenários e interpretação esperada |

## 4. Divergências de implementação que afetam a rastreabilidade

| ID | Requisito/decisão | Estado observado | Classificação | Próxima evidência necessária |
|---|---|---|---|---|
| CFLT-001 | SR-F-010/011 — estimadores | os estimadores existem e são chamados; a análise antiga dizia que eram stubs | contraditória entre versões documentais | manter a análise antiga marcada como histórica e testar o código atual |
| CFLT-002 | SR-F-002 — MAX30102 por variante | o probe aceita qualquer PART_ID diferente de `0x00/0xFF` | parcial | perfil de variante, mock de registradores e HIL |
| CFLT-008 | CR-004/SR-F-012 — validade inequívoca | `clinical_valid` existe internamente, mas não é transmitido no frame ativo | contraditória | teste de contrato e telemetria completa |
| CFLT-009 | SR-F-016/018/020 — ponta a ponta | Unity, DeepDDA, MongoDB e dashboard não estão neste repositório | não verificável | repositórios/commits fixados e teste integrado |

## 5. Gate evolutivo para artigo técnico

Antes de submeter um artigo como implementação validada de oxímetro integrado, devem existir resultados reproduzíveis para TST-001 a TST-005 e MET-001 a MET-010 aplicáveis, com commit, binário, hardware, configuração, dataset, referência e critérios pré-especificados. O estado atual sustenta descrição de arquitetura e prova de conceito em código, não exatidão metrológica, segurança clínica ou eficácia.

As lacunas desta matriz devem ser reavaliadas a cada implementação ou teste; não devem ser copiadas como limitações permanentes depois que a evidência correspondente existir.
