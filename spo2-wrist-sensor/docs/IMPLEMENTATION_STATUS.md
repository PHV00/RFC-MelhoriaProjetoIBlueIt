# Status da etapa — Arquitetura e documentação do SQI

## Objetivo desta etapa

Formalizar a arquitetura técnica que guiará a refatoração do módulo de qualidade de sinal do MAX30102 antes de modificar o comportamento do firmware.

## O que foi concluído

### Documentação principal

- `ReadME.md`: atualizado com estado atual, fluxo funcional, princípios arquiteturais, organização alvo e links para a documentação técnica.
- `ARCHITECTURE.md`: criada a separação em camadas (`drivers`, `sensing`, `processing`, `safety`, `storage`, `transport` e `app`) e a direção correta de dependências.
- `SQI_ARCHITECTURE.md`: definida a arquitetura hierárquica e fail-fast do SQI em quatro gates.
- `DECISIONS.md`: registradas decisões como RAW imutável, configuração centralizada, janela temporal, separação SQI × confidence e migração incremental.
- `CONFIGURATION.md`: definido o modelo de configuração centralizada/versionável e a futura separação de perfis `NO_GRIP` e `WITH_GRIP`.
- `TESTING.md`: descritos cenários mínimos de teste para cada gate e critérios de regressão.
- `ROADMAP.md`: dividido o trabalho em fases de refatoração, implementação, integração e calibração.
- `REFERENCES.md`: associado cada trabalho científico/técnico às responsabilidades de G1–G4 e às métricas candidatas.

### Decisões técnicas registradas

- SQI deve ser executado antes dos estimadores de FC e SpO₂.
- Uma janela `INVALID` não deve gerar SpO₂/FC utilizáveis.
- `signal_quality.c` deve se tornar o orquestrador do SQI.
- G1 valida integridade do sinal bruto antes da filtragem.
- G2 avalia pulsatilidade em RED e IR.
- G3 avalia morfologia por batimento.
- G4 verifica coerência entre RED e IR.
- O RAW não será sobrescrito pelo processamento.
- Features matemáticas e regras de decisão serão separadas.
- Thresholds serão centralizados e calibráveis.
- A baseline inicial usa janela de 5 s e passo sugerido de 1 s.
- A primeira calibração será realizada sem o pegador anatômico.
- A introdução futura do pegador não deve exigir reescrita da arquitetura.

## O que não foi feito nesta etapa

- nenhuma movimentação de arquivos `.c`/`.h`;
- nenhuma criação efetiva das pastas de código `gates/`, `features/` ou `preprocess/`;
- nenhuma alteração de `CMakeLists.txt`;
- nenhuma implementação de G1, G2, G3 ou G4;
- nenhuma alteração nos estimadores atuais de FC/SpO₂;
- nenhuma compilação específica da nova arquitetura;
- nenhum teste unitário novo;
- nenhuma calibração experimental de thresholds.

## Próxima etapa recomendada

Executar a **Fase 0** do roadmap como uma refatoração estrutural sem mudança funcional:

1. mover `signal_quality.*` para `processing/sqi/`;
2. criar a árvore de diretórios planejada;
3. atualizar includes e `CMakeLists.txt`;
4. compilar e verificar equivalência funcional;
5. somente depois iniciar a extração de G1.

Essa sequência preserva um ponto de regressão claro e evita misturar reorganização estrutural com mudanças algorítmicas.
