# Testes unitários — G1 Integridade

Esta pasta contém a suíte unitária host que permanece junto da implementação de produção do `G1_INTEGRITY`.

## Executar

A partir de `spo2-wrist-sensor/`:

```bash
./tests/sqi/g1_integrity/run_host_tests.sh
```

Saída esperada:

```text
Host compiler: /usr/bin/cc
Host assembler: /usr/bin/as
G1 integrity tests: PASS (18 cases)
```

A suíte cobre janela limpa, no-signal, isolamento entre RED/IR, flatline, clipping RED/IR, fronteiras exatas dos thresholds, continuidade, timestamp duplicado, configuração/argumentos inválidos e imutabilidade da entrada RAW.

## Fault injection

O harness end-to-end usado para validar flatline, clipping RED, clipping IR e descontinuidade **não faz parte da linha de produção**.

Ele foi preservado para repetição futura na branch:

```text
archive/sqi-g1-fault-injection
```

Commit de referência:

```text
411a5e653de7ddb2ecf7fb1bcb24abf31ac86de2
```

Os resultados já obtidos estão documentados em:

```text
docs/sqi/G1_VALIDATION_REPORT.md
docs/sqi/FAULT_INJECTION_ARCHIVE.md
```

Caso seja necessário repetir os testes de integração, faça checkout da branch arquivada; não reintroduza fault injection no `ppg_sampler` da linha de produção apenas para manter o teste disponível.
