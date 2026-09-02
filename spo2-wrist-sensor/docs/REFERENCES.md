# Referências técnicas e científicas do SQI

Esta documentação resume como cada trabalho será usado na implementação. Ela não substitui a leitura dos artigos originais.

## Vadrevu & Manikandan — Real-Time PPG Signal Quality Assessment System

Uso no projeto:

- arquitetura hierárquica;
- janela de 5 s;
- remoção de baseline;
- amplitude;
- threshold crossing;
- autocorrelação;
- descarte precoce de segmentos ruins.

Aplicação principal: **G2 — Pulsatilidade**.

## Reddy, Manikandan & Murty — On-Device Integrated PPG Quality Assessment

Uso no projeto:

- detecção precoce de sinal praticamente nulo;
- detecção de saturação;
- hierarquia para economizar processamento;
- preocupação explícita com execução embarcada.

Aplicação principal: **G1 — Integridade**.

## Fischer et al. — Real-Time Pulse Waveform Segmentation and Artifact Detection

Uso no projeto:

- clipping antes da filtragem;
- implementação embarcada em tempo real;
- segmentação por pulso;
- duração, amplitude e rise time;
- listas de decisão simples.

Aplicações: **G1 e G3**.

## Sukor, Redmond & Lovell — Signal Quality Measures for Pulse Oximetry through Waveform Morphology Analysis

Uso no projeto:

- morfologia do pulso;
- amplitude;
- largura;
- comparação/estabilidade entre pulsos.

Aplicação principal: **G3 — Morfologia**.

## Karlen et al. — Photoplethysmogram Signal Quality Estimation using Repeated Gaussian Filters and Cross-Correlation

Uso no projeto:

- referência para SQI contínuo;
- segmentação de pulso;
- correlação entre pulsos consecutivos.

Status: **reserva para V2**, caso a abordagem baseline não seja suficiente.

## Orphanidou et al. — Signal-Quality Indices for ECG and PPG

Uso no projeto:

- referência para qualidade binária;
- plausibilidade temporal;
- template matching;
- ligação entre qualidade de PPG e confiabilidade da FC.

Status: **reserva/validação complementar**.

## Elgendi — Optimal Signal Quality Index for PPG Signals

Uso no projeto:

- comparação entre SQIs estatísticos;
- skewness como métrica candidata;
- evidência de que perfusion index isolado não deve ser tratado como árbitro universal de qualidade.

Status: **métrica experimental/V1.x**.

## Contexto I Blue It / 123-SGR

A arquitetura 123-SGR coloca sinais fisiológicos involuntários no fluxo voltado à segurança, passando por tratamento de sinais antes da Grade de Adaptação. O módulo SQI deste firmware materializa a etapa técnica de validação do PPG antes de disponibilizar SpO₂/FC ao restante do ecossistema.

O SQI não substitui regras terapêuticas nem o Flow Psicofisiológico. Ele responde apenas se o dado fisiológico possui qualidade técnica suficiente para ser usado pelas camadas superiores.
