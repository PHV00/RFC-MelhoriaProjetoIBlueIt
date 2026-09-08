# Estratégia de testes do SQI

## Princípio

Cada gate deve ser validado em três níveis: **unidade**, **integração no target por fault injection** e **aquisição real**. Teste unitário demonstra a regra matemática; fault injection demonstra o caminho completo firmware→gate→fail-fast→telemetria; aquisição real demonstra comportamento no hardware sem adulteração.

## Estado do G1

### Unit tests host

Comando:

```bash
./tests/sqi/g1_integrity/run_host_tests.sh
```

Resultado validado:

```text
Host compiler: /usr/bin/cc
Host assembler: /usr/bin/as
G1 integrity tests: PASS (18 cases)
```

A suíte cobre sinal válido, no-signal, isolamento RED/IR, flatline, clipping RED/IR, limites exatos de thresholds, continuidade, timestamp duplicado, argumentos/configuração inválidos e imutabilidade da entrada.

### Fault injection no ESP32-C3

Branch de validação: `test/sqi-g1-final-validation`.

| modo | falha | resultado observado |
|---:|---|---|
| 1 | FLATLINE_BOTH | PASS; `FLATLINE_RED`, mask 24, HR/SpO₂ bloqueados |
| 2 | CLIPPING_RED | PASS; 14 frames, 0 violações, mask 32, `red_clip=0.0500`, `ir_clip=0.0000` |
| 3 | CLIPPING_IR | PASS; 14 frames, 0 violações, mask 64, `red_clip=0.0000`, `ir_clip=0.0500` |
| 4 | DISCONTINUITY | PASS; 13 frames, 0 violações, mask 1, `continuity=0.7996` |

O validador é:

```bash
python3 tests/sqi/g1_integrity/validate_fault_log.py <log> <modo>
```

### Sanity check em modo normal

Após recompilar com `SQI_G1_FAULT_MODE=0`, foi observado:

- sem dedo: `quality_state=INVALID`, `failed_gate=G1_INTEGRITY`, mask 6;
- dedo presente: G1 PASS, mask 0;
- retirada: transição gradual compatível com a janela de 5 s e posterior retorno a mask 6;
- continuidade real aproximadamente 0,99–1,00;
- `red_clip=0` e `ir_clip=0`;
- toda janela G1 INVALID manteve HR/SpO₂ inválidos.

## Matriz de cobertura do G1

| caso | unitário | hardware real | fault injection |
|---|---:|---:|---:|
| PASS normal | ✅ | ✅ | — |
| no-signal RED/IR | ✅ | ✅ | — |
| flatline | ✅ | — | ✅ |
| clipping RED | ✅ | tentativa física sem clipping | ✅ |
| clipping IR | ✅ | — | ✅ |
| descontinuidade | ✅ | continuidade nominal observada | ✅ |
| boundaries | ✅ | — | — |
| fail-fast | ✅/integração | ✅ | ✅ |

## Próximos testes — G2

Criar casos para pulso limpo, ruído, baixa amplitude pulsátil, periodicidade não fisiológica, um canal pulsátil e outro não, movimentos/transientes e sinais reais MAX30102. Datasets públicos podem ser usados para periodicidade/movimento, mas não para calibrar amplitudes RAW absolutas do nosso MAX30102.

## Próximos testes — G3

Testar beats estáveis, amplitude variável, largura/rise time anormais, beats deformados, quantidade insuficiente de beats e estabilidade intra/inter-beat.

## Próximos testes — G4

Testar RED/IR coerentes, períodos divergentes, contagens divergentes, desalinhamento temporal e degradação isolada de um canal.

## Critérios de regressão e ciência

Toda mudança de threshold deve registrar dataset, configuração do sensor, versão, divisão desenvolvimento/validação, taxa de falsa aceitação/rejeição, sensibilidade/especificidade quando aplicável e análise de sensibilidade. Janelas sobrepostas da mesma sessão nunca devem ser divididas entre calibração e teste.
