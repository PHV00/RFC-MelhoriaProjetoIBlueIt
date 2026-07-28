# Instruções do projeto

## Escopo deste arquivo

Estas instruções se aplicam a todo o repositório, salvo quando um `AGENTS.md` mais específico existir em um subdiretório. Instruções locais podem especializar este arquivo, mas não podem reduzir requisitos de segurança, rastreabilidade, privacidade ou integridade científica.

O projeto envolve software de apoio à reabilitação respiratória e processamento de sinais fisiológicos. Funcionamento técnico, validade de medição, segurança operacional, utilidade clínica e eficácia clínica são conceitos diferentes e devem permanecer separados em código, testes, documentação e comunicação.

## Contexto obrigatório

Antes de planejar ou modificar código, leia, nesta ordem:

1. `docs/development/context/PROJECT_BRIEF.md`;
2. `docs/development/context/SOFTWARE_REQUIREMENTS.md`;
3. `ReadME.md` e `docs/development/dev/Structure.md`, equivalentes arquiteturais atuais;
4. `docs/development/context/TRACEABILITY_MATRIX.md`;
5. `docs/development/context/LIMITATIONS.md`;
6. `docs/development/context/OPEN_RESEARCH_QUESTIONS.md`.

`docs/development/context/DECISIONS.md` ainda não existe. Até que seja criado e revisado, referências a ADR/DM sem artefato são **não verificáveis** e não devem ser tratadas como decisões aceitas.

`docs/development/context/GLOSSARY.md` também ainda não existe. Quando um termo de domínio, sigla, estado, métrica ou identificador não estiver inequívoco, registre a ambiguidade em vez de presumir uma definição.

Antes de alterar protocolo experimental, critérios de aceitação, coleta com seres humanos, métricas de comparação ou alegações científicas, leia também:

- `docs/development/context/SCIENTIFIC_CONTEXT.md`;
- `docs/development/context/EVIDENCE_MATRIX.md`;
- `docs/development/context/HYPOTHESES.md`;
- `docs/development/context/RESEARCH_PROTOCOL.md`.

Não inicie trabalho com base apenas em conversas, issues, comentários ou trechos isolados. Use-os para localizar o contexto canônico e registre lacunas quando ele não existir.

## Autoridade das informações

A precedência depende do tipo de afirmação.

### Estado atual da implementação

Use esta ordem:

1. código e testes do commit atual;
2. configuração versionada e artefatos gerados pelo mesmo commit;
3. arquitetura documentada;
4. documentação histórica e conversas.

O código mostra o que o sistema faz, não o que ele deveria fazer nem o que é clinicamente aceitável.

### Comportamento normativo e critérios de aceitação

Use esta ordem:

1. requisitos clínicos e de segurança confirmados em `SOFTWARE_REQUIREMENTS.md`;
2. decisões marcadas como `aceita` em `DECISIONS.md`, quando esse documento existir;
3. requisitos funcionais e não funcionais confirmados;
4. arquitetura documentada;
5. protocolo de pesquisa aprovado;
6. hipóteses, resultados preliminares e conversas históricas.

Uma implementação existente não revoga silenciosamente um requisito ou uma decisão aceita. Uma hipótese não se torna requisito por estar implementada.

### Afirmações científicas

Use esta ordem:

1. resultado reproduzível vinculado a protocolo, dados, versão, teste e métrica;
2. evidência externa catalogada em `EVIDENCE_MATRIX.md`;
3. resultado preliminar identificado como tal;
4. observação interna do projeto;
5. hipótese científica;
6. conversa histórica.

Não trate conversas, logs isolados, testes de bancada ou opinião de especialista como evidência externa de eficácia clínica.

### Divergências

Quando código, testes, requisitos, arquitetura, decisões ou documentos divergirem:

1. não escolha silenciosamente;
2. identifique os artefatos e IDs em conflito;
3. determine se a divergência é de implementação, documentação, requisito, versão ou evidência;
4. preserve o comportamento mais seguro enquanto o conflito estiver aberto;
5. registre a questão em `OPEN_RESEARCH_QUESTIONS.md` ou a decisão em `DECISIONS.md`;
6. proponha a correção e seu impacto.

Conflitos que envolvam paciente, limiar fisiológico, validade do sinal, identidade do sensor, integridade de dados ou ação automática bloqueiam a conclusão da alteração até revisão humana apropriada.

## Rastreabilidade obrigatória

Use os identificadores estáveis já definidos:

- objetivos: `OBJ-*`;
- perguntas de pesquisa: `RQ-*`;
- hipóteses: `HYP-*` e `H0-*`;
- evidências: `EVD-*`;
- requisitos clínicos: `CR-*`;
- requisitos de software: `SR-F-*` e `SR-NF-*`;
- componentes: `CMP-*`;
- decisões: `ADR-*`, `ODR-*` e `DM-*`;
- testes: `TST-*`;
- métricas: `MET-*`;
- conflitos: `CFLT-*`;
- questões abertas: `ORQ-*`.

Antes de implementar uma alteração importante, determine pelo menos:

1. qual objetivo ela atende;
2. quais requisitos ela implementa ou modifica;
3. qual decisão arquitetural a restringe;
4. quais testes e métricas demonstrarão o resultado;
5. qual risco ou limitação permanece.

Não renumere IDs existentes. Para novos itens, use o próximo ID livre da categoria e registre-o nos documentos pertinentes e em `CONTEXT_INDEX.md`.

## Regras para alterações no firmware e processamento PPG

- O alvo atual é o MAX30102. Resultados históricos do MAX30105 não validam o hardware atual.
- Preserve a abstração MAX3010x, mas selecione configuração e capacidades por variante identificada.
- Leia e registre `PART_ID`, `REV_ID`, versão do firmware, configuração do sensor e commit.
- Corrija aquisição, FIFO, overflow, gaps e timestamps antes de calibrar ou otimizar SpO2.
- Não descarte perda de amostra, overflow, falha I2C, reset ou descontinuidade sem evento rastreável.
- Preserve RED/IR brutos e metadados antes dos filtros quando a execução fizer parte de teste, calibração, investigação ou validação.
- Separe valor, validade, confiança, estado de estabilização e motivo de invalidade.
- Sinal ausente ou inválido deve permanecer desconhecido. Não converta indisponibilidade em `0`, normalidade ou valor anterior sem marcador explícito.
- Processamento determinístico é o baseline. IA não substitui tratamento de sinal, qualidade, calibração nem fail-safe.
- Curvas de calibração, coeficientes, sample rate, averaging, pulse width, ADC range, corrente dos LEDs e limiares devem ser versionados e associados ao resultado.

## Regras para segurança clínica e DeepDDA

- O sistema é de apoio e não deve emitir diagnóstico nem substituir julgamento clínico.
- O terapeuta deve manter controle para pausar, interromper, retomar e sobrescrever ações automáticas.
- Diferencie alerta técnico de alerta fisiológico.
- Não acione regra fisiológica com amostra inválida, baixa confiança ou janela ainda não estabilizada.
- Valores como 95% e 89% são parâmetros provisórios da prova de conceito, não regras clínicas universais.
- Não hardcode limiares clínicos sem requisito, origem, versão de protocolo e aprovação humana registrada.
- Valide o pipeline contra referência antes de permitir que valores reais controlem ações clínicas do DeepDDA. Até lá, use simulação ou replay claramente identificados.
- Toda decisão automática deve registrar observação, qualidade, ação, motivo, parâmetros antes/depois, timestamp, versão e eventual override.
- Prefira comportamento conservador e indisponibilidade explícita a um falso estado seguro.

## Dados, privacidade e integridade

- Minimize dados pessoais e não inclua identificadores de paciente em logs de desenvolvimento, fixtures, commits ou exemplos públicos.
- Associe dados a paciente, sessão, dispositivo, firmware e sequência apenas por identificadores controlados e documentados.
- Preserve registros inválidos, negativos, falhas e desvios de protocolo; não limpe dados para melhorar métricas.
- Dados simulados, voluntários hígidos e pacientes devem ser armazenados e analisados como populações distintas.
- Não sobrescreva dados brutos nem resultados anteriores. Gere nova versão ou nova execução.
- Qualquer migração de esquema deve informar compatibilidade, rollback e impacto na rastreabilidade.

## Durante alterações

- Preserve decisões arquiteturais aceitas.
- Faça alterações pequenas, revisáveis e vinculadas aos IDs relevantes.
- Não adicione dependência de produção sem justificar necessidade, licença, manutenção, superfície de ataque, custo de memória/tempo e alternativa considerada.
- Não altere contrato público, frame de telemetria, unidade, estado, schema ou API sem indicar impacto e migração.
- Não silencie warning, erro ou teste para obter uma execução verde sem registrar a causa.
- Não remova teste que revele falha real; corrija o comportamento ou documente uma decisão de mudança.
- Registre novas dúvidas em `OPEN_RESEARCH_QUESTIONS.md`.
- Registre decisões duráveis em `DECISIONS.md`; não use o changelog como substituto de ADR.

## Testes mínimos

Execute os testes relevantes ao escopo. Quando inexistentes, adicione-os antes ou junto da alteração.

Para mudanças no sensor ou pipeline PPG, considere obrigatórios, conforme aplicável:

- testes unitários de registradores, configuração e conversão;
- testes de FIFO, overflow, drenagem, gaps e reconstrução temporal;
- replay determinístico de RED/IR gravados;
- testes de filtros e preservação das características relevantes;
- testes de detecção de contato e qualidade do sinal;
- testes de HR, SpO2, validade, confiança e estabilização;
- cenários de movimento, luz ambiente, saturação do ADC, dedo ausente e falha I2C;
- testes de integração da telemetria e persistência;
- testes HIL quando houver alteração de driver, timing ou configuração física.

Para mudanças em segurança ou DeepDDA, considere obrigatórios:

- sinal válido acima/abaixo do limiar configurado;
- baixa confiança e sinal ausente;
- persistência, histerese e recuperação;
- alerta técnico versus fisiológico;
- override do terapeuta;
- replay da decisão com os mesmos dados e versões;
- falha de transporte, banco ou componente da IA.

Registre comando, ambiente, resultado e testes não executados. Teste de software não demonstra eficácia clínica.

## Critério de conclusão

Uma alteração importante só está concluída quando:

1. o código está vinculado aos requisitos e decisões aplicáveis;
2. os testes relevantes passam ou as falhas remanescentes estão explicitadas;
3. logs e formatos preservam rastreabilidade e estados inválidos;
4. impactos em compatibilidade, segurança, desempenho e dados foram avaliados;
5. a documentação canônica foi atualizada;
6. limitações e revisão humana pendente foram declaradas;
7. nenhuma alegação excede a evidência produzida.

## Depois de alterações importantes

Atualize, quando necessário, os equivalentes existentes:

- `docs/development/context/SOFTWARE_REQUIREMENTS.md`;
- `ReadME.md` e `docs/development/dev/Structure.md` para arquitetura;
- `docs/development/context/TRACEABILITY_MATRIX.md`;
- `docs/development/context/LIMITATIONS.md`;
- `docs/development/context/OPEN_RESEARCH_QUESTIONS.md`;
- `docs/development/context/CONTEXT_INDEX.md`.

`DECISIONS.md`, `GLOSSARY.md` e `CONTEXT_CHANGELOG.md` devem entrar nessa lista quando forem criados; não invente seu conteúdo para preencher a ausência.

Atualize `EVIDENCE_MATRIX.md`, `HYPOTHESES.md` e `RESEARCH_PROTOCOL.md` quando a alteração produzir evidência, modificar hipótese, métrica, teste, protocolo ou critério de aceitação.

Quando criado, `CONTEXT_CHANGELOG.md` deve registrar o que mudou e por quê, mas não substituir histórico do Git, ADR, resultado de teste ou relatório experimental.

## Revisão humana obrigatória

Exija revisão do responsável adequado antes de concluir mudanças que envolvam:

- intended use, população-alvo ou alegação clínica;
- limiar fisiológico, critério de interrupção ou resposta a evento adverso;
- ação autônoma que altere exercício ou sessão;
- protocolo com seres humanos, consentimento ou ética;
- privacidade, retenção ou compartilhamento de dados;
- curva de calibração ou critério de equivalência com referência;
- classificação regulatória ou uso fora de pesquisa.

Marque explicitamente no PR e na documentação: `REVISÃO CLÍNICA PENDENTE`, `REVISÃO CIENTÍFICA PENDENTE`, `REVISÃO DE SEGURANÇA PENDENTE` ou `REVISÃO REGULATÓRIA PENDENTE`.

## Comunicação da alteração

Ao concluir, informe de forma verificável:

- arquivos alterados;
- IDs atendidos ou afetados;
- comportamento anterior e novo;
- testes executados e resultados;
- limitações, riscos e testes não executados;
- documentação atualizada;
- revisões humanas pendentes.

Não use frases como “clinicamente validado”, “seguro para pacientes”, “preciso” ou “equivalente a oxímetro clínico” sem protocolo, referência, amostra, métricas, resultado e aprovação correspondentes.

## Manutenção deste arquivo

Mantenha este arquivo curto o suficiente para orientar trabalho, sem copiar literatura, transcrições ou especificações extensas. O detalhe pertence aos documentos de contexto. Enquanto `CONTEXT_CHANGELOG.md` não existir, preserve alterações no histórico do Git e registre a lacuna em `CONTEXT_INDEX.md`.
