# G1 — calibrações pendentes e critério de fechamento

Este arquivo é o checklist operacional. **Todos os valores marcados como provisórios ainda podem mudar sem alterar a arquitetura do gate.**

| parâmetro | atual | status | dados necessários | procedimento | pronto quando |
|---|---:|---|---|---|---|
| `window_ms` | 5000 ms | baseline literatura | RAW rotulado | comparar 3/5/8 s ou manter 5 s se não houver motivo | latência/erro documentados |
| `step_ms` | 1000 ms | engenharia | perfil de CPU/FIFO/latência | medir custo e responsividade | atende tempo real sem overflow |
| `adc_min_value` | 0 | hardware | datasheet/config | nenhuma calibração | fixo enquanto formato não mudar |
| `adc_max_value` | 262143 | hardware | 18-bit config | nenhuma calibração | fixo enquanto ADC/pulse width não mudar |
| `rail_margin_counts` | 1 | **provisório** | saturação/bench | sweep de distância ao rail | margem separa nominal de saturado no hold-out |
| `minimum_mean_level` | 5000 | **provisório** | NO_CONTACT + contatos rotulados | distribuições + ROC/PR/critério de falsa aceitação | desempenho estável no hold-out |
| `minimum_raw_range` | 20 | **provisório** | flatline/stuck + contatos válidos | cauda inferior + falhas sintéticas | detecta stuck sem rejeitar sinal íntegro |
| `maximum_clipping_fraction` | 0.01 | **provisório** | sweep de clipping | 0–10% de corrupção + impacto downstream | tolerância justificada e validada |
| `minimum_continuity_fraction` | 0.95 | **provisório** | long run + faults | injetar perdas/duplicados crescentes | nominal aceito, falha prejudicial rejeitada |
| `maximum_interval_deviation_fraction` | 0.40 | **provisório** | distribuição de `dt` reconstruído | normal + carga + faults | envelope nominal documentado |
| `minimum_quality_score` | 0.55 | legado | G2–G4 completos | não calibrar agora | redefinido/removido após pipeline completo |

## Passo a passo de execução

```text
1. congelar perfil MAX30102 + geometria NO_GRIP
2. coletar sessões RAW rotuladas
3. gerar features G1 por janela
4. dividir por participante/sessão
5. construir distribuições e thresholds candidatos
6. medir erros no desenvolvimento
7. escolher candidato por critério explícito
8. rodar hold-out sem retuning
9. fazer análise de sensibilidade
10. congelar config_version + dataset + relatório
```

## Regras de não contaminação entre gates

- `minimum_mean_level`: presença óptica, não pulsatilidade;
- `minimum_raw_range`: flatline técnico, não amplitude fisiológica mínima;
- clipping: rail/corrupção ADC, não movimento;
- continuidade: aquisição/tempo, não qualidade morfológica;
- movimento, periodicidade e forma devem ser resolvidos em G2/G3/G4.

## Perfil WITH_GRIP

Somente após `NO_GRIP` estar congelado. Repetir o mesmo protocolo com o pegador e comparar distributions/false accept/false reject. Criar thresholds distintos apenas se houver diferença relevante e reproduzível.
