# Plano científico de calibração dos thresholds do G1

## 1. Princípio

O G1 separa **integridade técnica** de pulsatilidade/morfologia. A literatura orienta quais features e arquitetura usar, mas não fornece um conjunto universal de contagens RAW para o nosso MAX30102. Valores absolutos dependem de ADC, range, LED, encapsulamento, pressão, dedo, iluminação e geometria.

A metodologia adotada é:

```text
literatura → escolhe feature/estrutura
hardware   → fixa limites físicos
engenharia → fornece valor inicial seguro
nossos dados → calibram threshold final
hold-out   → valida generalização
```

## 2. Congelar o perfil de aquisição

Antes da coleta registrar e manter fixos:

- sensor/placa e revisão;
- 100 Hz;
- pulse width/resolução ADC;
- ADC range;
- corrente RED e IR;
- posição do sensor;
- encapsulamento/óptica;
- firmware/config version;
- ausência do pegador: perfil inicial `NO_GRIP`.

Mudanças materiais nesses itens exigem reavaliação dos thresholds RAW.

## 3. Dataset local mínimo

Registrar RAW RED, RAW IR, timestamp/seq e metadados. Rotular blocos/sessões como:

`NO_CONTACT`, `STABLE_CONTACT`, `LIGHT_CONTACT`, `PARTIAL_CONTACT`, `PRESSURE`, `AMBIENT_LIGHT`, `MOTION`, `TRANSITION_IN`, `TRANSITION_OUT` e falhas controladas `FLAT_FAULT`, `CLIP_LOW/HIGH`, `TIMESTAMP_FAULT`.

Idealmente coletar múltiplas sessões e participantes. Janelas de 5 s podem ser extraídas com passo de 1 s, mas janelas sobrepostas da mesma sessão devem permanecer no mesmo split.

## 4. Features a exportar por janela

- `red_mean`, `ir_mean`;
- `red_min/max/range`, `ir_min/max/range`;
- `red_clip_fraction`, `ir_clip_fraction`;
- `continuity_fraction`;
- descontinuidades e timestamps duplicados;
- classe/label e identificador de sessão/participante.

## 5. Divisão dos dados

Preferência: split por participante. Se o número de participantes for pequeno, split por sessão. Nunca randomizar janelas sobrepostas individualmente entre desenvolvimento e teste.

```text
desenvolvimento → histogramas/thresholds candidatos
hold-out         → avaliação final sem retuning
```

## 6. Seleção por parâmetro

### `minimum_mean_level`

Comparar `NO_CONTACT`/contato inválido com contato utilizável, por canal. Inspecionar percentis e sobreposição; testar candidatos em ROC/PR ou por uma restrição explícita de falsa aceitação. Um procedimento possível é buscar região entre cauda superior de `NO_CONTACT` e cauda inferior de `STABLE_CONTACT`, mas percentis como P99/P01 são estratégia de engenharia, não regra universal.

### `minimum_raw_range`

Manter o objetivo estrito de detectar flatline/stuck. Usar falhas sintéticas/bench e a cauda inferior de janelas reais válidas. Não aumentar o valor para rejeitar baixa pulsatilidade: isso é G2.

### `rail_margin_counts`

Executar sweep de proximidade aos rails e ensaios de saturação óptica/eletrônica. Testar margens 1, 2, 4, 8, 16, 32, 64, 128... e observar quais códigos aparecem quando o sensor já está claramente saturado.

### `maximum_clipping_fraction`

Aplicar sweep controlado, por exemplo 0%, 0,2%, 0,5%, 1%, 2%, 5%, 10%, e medir impacto nas fases posteriores/estimadores. Selecionar um orçamento máximo de corrupção justificável.

### `minimum_continuity_fraction`

Executar long runs normais e injetar taxas crescentes de intervalos ausentes/duplicados. Escolher limite que retenha aquisição nominal e rejeite perda suficiente para prejudicar análise temporal posterior.

### `maximum_interval_deviation_fraction`

Caracterizar a distribuição real de `dt` reconstruído pelo firmware e ensaios sob carga. Como o MAX30102 não fornece timestamp individual, esta métrica mede a continuidade do fluxo reconstruído, não jitter instantâneo do ADC.

### `window_ms` e `step_ms`

5 s é a baseline científica de desenvolvimento; 1 s é escolha de engenharia. Fazer análise de sensibilidade de latência e erro antes de qualquer mudança.

## 7. Critérios de seleção

Para cada candidato reportar, conforme o problema binário rotulado: sensibilidade, especificidade, false accept, false reject, matriz de confusão e intervalos de confiança quando a amostra permitir. A escolha deve privilegiar um plateau estável, não o melhor ponto de uma única sessão.

## 8. Validação e congelamento

Executar o candidato no hold-out, sem reajuste. Registrar versão de sensor/configuração, dataset, métricas e thresholds. Só então mudar o status de `provisional` para `empirically calibrated` e congelar `NO_GRIP`.

## 9. Datasets públicos

Bancos como Wrist PPG During Exercise, PPG-DaLiA, MIMIC/BIDMC podem apoiar G2/G3/G4 e validação algorítmica, mas não devem definir diretamente `minimum_mean_level`, `minimum_raw_range` ou rails do nosso MAX30102 devido a hardware/escala diferentes.

## 10. Referências metodológicas

Reddy e Vadrevu demonstram que thresholds associados à amplitude/faixa dinâmica precisam ser adaptados ao módulo. Karlen/Orphanidou apoiam avaliação quantitativa de qualidade e validação em dados separados. O fabricante MAX30102 define limites físicos, não um threshold de “PPG bom”.
