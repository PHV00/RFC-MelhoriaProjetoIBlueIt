# Referências técnicas e científicas do SQI

Esta página registra **qual responsabilidade do software é sustentada por cada trabalho**. Thresholds dependentes de escala RAW não devem ser copiados cegamente entre sensores.

## Reddy, Manikandan & Murty — On-Device Integrated PPG Quality Assessment

Aplicação: **G1 e arquitetura hierárquica**.

Contribuições usadas: rejeição precoce de amplitude praticamente nula/saturação, hierarquia para economizar processamento, adaptação de thresholds à faixa dinâmica do módulo e execução embarcada.

## Vadrevu & Manikandan — Real-Time PPG Signal Quality Assessment System

Aplicação principal: **G2 — Pulsatilidade**; apoio à arquitetura global.

Contribuições usadas: janela de 5 s, remoção de baseline, amplitude, threshold crossings, autocorrelação, periodicidade e descarte hierárquico.

## Fischer et al. — Real-Time Pulse Waveform Segmentation and Artifact Detection

Aplicações: **G1 e G3**.

Contribuições usadas: detectar clipping/artefatos antes de filtragens que os escondam, segmentação em tempo real, amplitude, duração e rise time, regras simples adequadas a sistemas embarcados.

## Sukor, Redmond & Lovell — Signal Quality Measures for Pulse Oximetry through Waveform Morphology Analysis

Aplicação principal: **G3 — Morfologia**.

Contribuições usadas: amplitude, largura e estabilidade/consistência de pulsos para identificar segmentos cuja forma degrada a estimativa fisiológica.

## Karlen et al. — Photoplethysmogram Signal Quality Estimation using Repeated Gaussian Filters and Cross-Correlation

Aplicações: referência de **SQI contínuo/template/correlação** e metodologia de avaliação. Pode complementar G3/V2, sem substituir a decisão hierárquica.

## Orphanidou et al. — Signal-Quality Indices for ECG and PPG

Aplicações: plausibilidade temporal, qualidade binária e template matching. Reserva importante para validar regras de beats e critérios globais de qualidade.

## Elgendi — Optimal Signal Quality Index for PPG Signals

Aplicações: comparação de métricas estatísticas, skewness e discussão de SQIs. Serve como alternativa/experimento; perfusion index isolado não é usado como árbitro universal.

## MAX30102 — fabricante

O fabricante fundamenta limites físicos/configuração: resolução ADC, ranges, taxa, pulse width, LED e FIFO. Ele **não fornece um threshold universal de “PPG bom”**. Portanto `adc_min/max` podem ser hardware-derived; presença óptica, clipping tolerável e continuidade devem ser caracterizados no nosso sistema.

## Contexto I Blue It / 123-SGR

O SQI materializa a etapa técnica que protege a cadeia fisiológica antes de disponibilizar FC/SpO₂ às camadas superiores. Ele não substitui regras terapêuticas, o Flow Psicofisiológico ou a Grade de Adaptação.

## Mapeamento resumido

| área | referências principais |
|---|---|
| hierarquia/fail-fast/embedded | Reddy; Vadrevu |
| integridade RAW/clipping | Reddy; Fischer; fabricante MAX30102 |
| pulsatilidade/ACF/crossings | Vadrevu |
| morfologia de beats | Sukor; Fischer; Orphanidou |
| template/correlação/SQI contínuo | Karlen; Orphanidou |
| métricas estatísticas alternativas | Elgendi |
| integração em reabilitação/jogo | I Blue It; 123-SGR; trabalhos do projeto |
