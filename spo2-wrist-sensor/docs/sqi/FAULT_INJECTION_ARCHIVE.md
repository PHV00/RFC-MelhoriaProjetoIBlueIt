# Fault injection archive

O harness de fault injection usado para validar o G1 foi removido da linha de produção antes do merge do G1.

A implementação completa usada nos testes permanece preservada na branch:

```text
archive/sqi-g1-fault-injection
```

Commit preservado:

```text
411a5e653de7ddb2ecf7fb1bcb24abf31ac86de2
```

Essa branch contém os modos de injeção `FLATLINE_BOTH`, `CLIPPING_RED`, `CLIPPING_IR` e `DISCONTINUITY`, além do validador de telemetria usado na validação end-to-end.

Os resultados obtidos continuam registrados em `G1_VALIDATION_REPORT.md`. Caso seja necessário repetir os testes de fault injection no futuro, use a branch arquivada em vez de reintroduzir o harness diretamente na linha de produção.
