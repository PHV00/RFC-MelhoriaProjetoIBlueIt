# Gates pendentes — funcionamento técnico, teoria e parametrização

## Visão geral

O log de validação mostrou janelas G1 PASS com FC/SpO₂ instáveis. Isso é esperado: G1 verifica integridade técnica. Os próximos gates devem responder, em ordem, se há pulsatilidade, se os beats têm morfologia estável e se RED/IR descrevem o mesmo fenômeno.

```text
G1 PASS
  ↓
PREPROCESS (cópia do RAW)
  ↓
G2 PULSATILITY
  ↓
BEAT DETECTOR
  ↓
G3 MORPHOLOGY
  ↓
G4 RED↔IR COHERENCE
  ↓
VALID FINAL
```

# G2 — Pulsatilidade

## Pergunta

Existe componente pulsátil periódica suficientemente forte e plausível em RED **e** IR?

## Base teórica

Vadrevu & Manikandan: amplitude, threshold crossings e autocorrelação em arquitetura hierárquica com janela de 5 s. Reddy reforça a estratégia de rejeitar cedo antes de algoritmos caros.

## Implementação proposta

1. criar cópia processada; RAW não muda;
2. remover baseline com HP/detrending, baseline inicial ~0,5 Hz;
3. por canal calcular amplitude/range/RMS pulsátil;
4. calcular threshold crossings em torno de nível robusto/zero após detrending;
5. calcular ACF em faixa de lags compatível com frequência cardíaca definida;
6. extrair pico/lag dominante, periodicidade e consistência;
7. exigir RED e IR aprovados; registrar métricas e motivo por canal.

Pseudo-fluxo:

```text
processed channel
   ├─ amplitude >= A_min ?
   ├─ crossings in [C_min,C_max] ?
   ├─ ACF_peak >= R_min ?
   └─ period in plausible range ?
         FAIL → G2 INVALID
```

## Parâmetros a obter cientificamente

- filtro/detrending definitivo;
- amplitude pulsátil mínima por canal;
- limites de crossings;
- ACF mínima;
- faixa de lag/período;
- regras para assimetria RED/IR.

## Como calibrar

Usar sinais sintéticos para verificar matemática; dados MAX30102 rotulados para thresholds dependentes de amplitude; datasets como Wrist PPG During Exercise/PPG-DaLiA para periodicidade e movimento. Selecionar thresholds no desenvolvimento e validar por sessão/participante hold-out.

# G3 — Morfologia

## Pergunta

Os beats detectados possuem forma e estabilidade compatíveis com uma onda PPG utilizável?

## Base teórica

Sukor et al.: morfologia do pulso, amplitude/largura e qualidade para pulse oximetry. Fischer et al.: segmentação em tempo real, amplitude, duração e rise time; regras simples embarcáveis. Orphanidou: plausibilidade/template como validação complementar.

## Implementação proposta

Criar beat detector compartilhado. Para cada beat extrair:

- tempo de início/pico/fim;
- amplitude;
- largura/duração;
- rise time;
- intervalo beat-to-beat;
- opcional: similaridade com template/beat mediano.

Agregar por janela com mediana, MAD/CV e número de beats válidos.

```text
beats
 ├─ quantidade suficiente?
 ├─ width plausível?
 ├─ rise time plausível?
 ├─ amplitude estável?
 └─ variabilidade B2B aceitável?
       FAIL → G3 INVALID
```

## Parâmetros a calibrar

Limites de duração/rise time, número mínimo de beats, dispersão máxima de amplitude/largura/intervalo, critérios de outlier e eventual template threshold.

## Como calibrar

Primeiro validar detector em sinais anotados/sintéticos; depois extrair distribuições de beats bons/ruins em MAX30102 e datasets públicos. Dividir por participante/sessão. Preferir intervalos fisiologicamente justificáveis + dados, sem overfit em uma única população.

# G4 — Coerência RED↔IR

## Pergunta

RED e IR representam o mesmo trem de pulsos?

## Base teórica

A oximetria depende de duas bandas ópticas observando o mesmo fenômeno pulsátil. A coerência temporal entre canais é, portanto, uma condição de uso antes do cálculo de SpO₂. Correlação pode apoiar, mas não deve ser a única regra.

## Implementação proposta

Com eventos detectados por canal:

- período/FC equivalente RED e IR;
- diferença relativa de período;
- número de beats por canal;
- diferença de contagem;
- pareamento de eventos/picos por janela temporal;
- offset/alinhamento temporal;
- opcional: correlação em sinal normalizado.

```text
RED beats ─┐
           ├─ periods agree?
IR beats ──┤  counts agree?
           └─ peaks align?
                FAIL → G4 INVALID
```

## Parâmetros a calibrar

Tolerância de período, diferença máxima de contagem, janela de pareamento temporal, fração mínima de eventos pareados e, se usado, threshold de correlação.

## Como calibrar

Criar casos sintéticos com delay, beats removidos/adicionados e períodos diferentes; usar MAX30102 real em contato estável/movimento; selecionar tolerâncias no desenvolvimento e validar em hold-out. Esses parâmetros dependem mais de fisiologia/timing que de escala RAW e, portanto, datasets públicos são mais transferíveis do que no G1.

# Decisão final e score legado

Quando G2–G4 forem implementados:

```c
if (!g1.passed) reject(G1);
if (!g2.passed) reject(G2);
if (!g3.passed) reject(G3);
if (!g4.passed) reject(G4);
accept();
```

O score contínuo deve virar informação auxiliar de confiança/diagnóstico. Um score alto nunca pode reabilitar uma janela rejeitada por gate obrigatório.

# Critério científico comum a G2–G4

1. especificar feature e hipótese antes de olhar o conjunto de teste;
2. unit tests/sinais sintéticos para correção matemática;
3. dataset de desenvolvimento para thresholds;
4. split por participante/sessão;
5. hold-out sem retuning;
6. sensitivity/specificity/false accept/false reject e análise de sensibilidade;
7. registrar versão de preprocessamento, feature e configuração;
8. repetir após mudanças materiais de sensor/geometria/população alvo.
