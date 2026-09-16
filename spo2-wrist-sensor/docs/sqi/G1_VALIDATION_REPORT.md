# Relatório de validação funcional — G1 Integridade

## Escopo

Este relatório fecha a **validação funcional da implementação V1 do G1**. Ele não fecha a calibração científica dos thresholds.

## 1. Unit tests host

Comando:

```bash
./tests/sqi/g1_integrity/run_host_tests.sh
```

Resultado:

```text
Host compiler: /usr/bin/cc
Host assembler: /usr/bin/as
G1 integrity tests: PASS (18 cases)
```

Cobertura: janela limpa, no-signal, isolamento por canal, flatline, clipping RED/IR, boundaries exatos, continuidade, timestamp duplicado, erro de configuração e imutabilidade do RAW.

## 2. Fault injection end-to-end

A branch de validação injeta falhas antes do `SampleBuffer`, exercitando:

```text
aquisição → SampleBuffer → SQI → G1 → fail-fast → telemetria
```

Resultados observados:

| teste | frames | violações | reason | mask | evidência específica |
|---|---:|---:|---|---:|---|
| flatline | aprovado | 0 | `FLATLINE_RED` | 24 | ranges RED/IR = 0; HR/SpO₂ bloqueados |
| clipping RED | 14 | 0 | `CLIPPING_RED` | 32 | `red_clip=0.0500`, `ir_clip=0.0000`, continuity 1.0000 |
| clipping IR | 14 | 0 | `CLIPPING_IR` | 64 | `red_clip=0.0000`, `ir_clip=0.0500`, continuity 1.0000 |
| discontinuity | 13 | 0 | `DISCONTINUITY` | 1 | continuity 0.7996, clipping 0 |

O script `validate_fault_log.py` também verifica `quality_state=INVALID`, `failed_gate=G1_INTEGRITY`, máscara esperada e `hr.valid=false` / `spo2.valid=false`.

## 3. Sanity check em aquisição real

Após recompilar com `SQI_G1_FAULT_MODE=0`:

- sem dedo, médias RAW ficaram muito abaixo de 5000 e G1 retornou mask 6;
- ao colocar o dedo, G1 passou com mask 0;
- durante retirada, a janela deslizante manteve G1 PASS por alguns segundos enquanto ainda continha amostras antigas; depois retornou mask 6;
- continuidade ficou ~0,99–1,00;
- clipping permaneceu 0 em ambos os canais;
- toda janela G1 INVALID bloqueou HR/SpO₂.

Exemplo da fronteira atual de presença durante retirada:

```text
ts=33512: red_mean=5201.2, ir_mean=5474.6 → G1 PASS
ts=34522: red_mean=772.7,  ir_mean=551.1  → G1 INVALID, mask 6
```

Isso demonstra que a implementação obedece ao threshold atual; **não demonstra que 5000 seja o valor cientificamente ótimo**.

## 4. Interpretação

O G1 está apto a detectar falhas técnicas grosseiras. Foram observadas janelas G1 PASS com HR/SpO₂ instáveis e `quality_score` legado alto. Isso é evidência da necessidade de G2/G3/G4 e não motivo para ampliar artificialmente o escopo do G1.

## 5. Conclusão

```text
lógica do G1                 VALIDADA
integração ESP32-C3          VALIDADA
fail-fast                    VALIDADO
telemetria                   VALIDADA
classes de falha testadas    VALIDADA
thresholds científicos       PENDENTES
```

Antes de qualquer teste fisiológico normal, garantir `SQI_G1_FAULT_MODE=0`.
