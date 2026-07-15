# Fundamentação bibliográfica e decisões de implementação

## 1. Papel do oxímetro no I Blue It

A arquitetura 123-SGR trata sinais fisiológicos involuntários como uma modalidade necessária para **segurança**, complementar às modalidades conscientes usadas para controlar o jogo. Nessa arquitetura, frequência cardíaca e saturação podem informar adaptações ou interrupções, mas devem passar por tratamento de sinal, combinação, grade de adaptação e armazenamento.

A tese de Dias (2024) amplia essa ideia no Flow Psicofisiológico: o sistema considera segurança por biossinais, conforto físico, dimensão psíquica e diversão. O oxímetro, portanto, não deve ser apenas um número serial; ele deve produzir um dado qualificado, rastreável e apropriado para consumo pelo Mixer/DeepDDA.

Decisão derivada:

```text
PPG bruto → qualidade → HR/SpO₂ → confiança → health_frame → política do jogo
```

O DeepDDA e o módulo de segurança não devem consumir diretamente valores brutos ou estimativas sem validade.

## 2. Continuidade com o desenvolvimento do I Blue It

O I Blue It foi concebido como jogo sério de reabilitação respiratória com envolvimento de especialistas e estímulos respiratórios avaliados iterativamente. A evolução posterior incorporou multimodalidade, monitoramento e Flow Psicofisiológico.

A prova de conceito atual preserva essa abordagem incremental:

- não substitui PITACO, MANO-BD ou calibração respiratória;
- adiciona uma modalidade fisiológica inconsciente;
- produz dados de monitoramento independentes do controle respiratório;
- prepara a integração com estados da sessão, persistência e dashboard;
- mantém separadas a estimação do biossinal e a decisão terapêutica.

## 3. Fundamento óptico do cálculo de SpO₂

A fotopletismografia mede variações ópticas associadas ao volume sanguíneo. Em cada canal, o sinal é decomposto em:

- **DC:** componente média, associada ao caminho óptico e tecidos;
- **AC:** componente pulsátil, associada à variação arterial.

A razão dos quocientes usada na POC é:

```text
R = (AC_RED / DC_RED) / (AC_IR / DC_IR)
```

Depois, uma curva empírica converte `R` em SpO₂:

```text
SpO₂ = aR² + bR + c
```

Essa estrutura é coerente com a documentação técnica da indústria para oximetria reflexiva. Entretanto, os coeficientes não são universais.

## 4. Por que a curva não pode ser considerada pronta

A curva depende de fatores como:

- comprimentos de onda e tolerâncias dos LEDs;
- fotodiodo e ganho;
- corrente dos LEDs;
- distância e geometria entre emissor e receptor;
- janela e material de proteção;
- encapsulamento e bloqueio óptico;
- posição e pressão de contato;
- local do corpo;
- movimento, perfusão e luz ambiente;
- população e protocolo de referência.

Consequência: copiar uma fórmula de outro módulo pode produzir números plausíveis sem demonstrar exatidão. Por isso, a implementação separa:

- `valid`: a matemática e o sinal permitem uma estimativa experimental;
- `calibrated`: há curva rastreável para o conjunto final;
- `clinical_valid`: o frame pode ser considerado validado para finalidade clínica definida.

Na configuração entregue:

```c
.calibrated = false
.allow_uncalibrated_estimate = true
.version = 0
```

Isso permite concluir a POC de software sem ocultar a limitação científica.

## 5. Fundamento do HR

A frequência cardíaca é estimada pela periodicidade do PPG IR:

1. remoção de tendência;
2. suavização curta;
3. limiar adaptativo;
4. detecção de picos;
5. cálculo dos intervalos entre batimentos;
6. mediana do IBI;
7. conversão `HR = 60000 / IBI_ms`;
8. rejeição fora da faixa e cálculo de estabilidade.

Esse método é apropriado como baseline interpretável para POC. Ele não substitui algoritmos industriais com filtros adaptativos, acelerômetro e modelos de artefato de movimento.

## 6. Qualidade do sinal como pré-condição

A bibliografia e a prática de dispositivos ópticos mostram que uma estimativa numérica sem avaliação de sinal pode ser enganosa. O pacote usa:

- presença de sinal;
- AC RMS e DC;
- índice de perfusão;
- ruído residual e SNR;
- correlação RED/IR;
- continuidade temporal;
- clipping;
- domínio dos estimadores.

A qualidade não é “prova de acurácia”; ela funciona como gate técnico que evita processar condições claramente inadequadas.

## 7. Movimento e acelerômetro

Soluções industriais para HR/SpO₂ em dispositivos vestíveis frequentemente combinam PPG com acelerômetro para identificar ou compensar artefatos de movimento. O firmware atual não possui esse dado.

Consequência para o artigo e para a integração:

- declarar a limitação de movimento;
- medir o desempenho em repouso e em movimento separadamente;
- rejeitar janelas de baixa qualidade;
- considerar acelerômetro como evolução arquitetural, não como requisito para concluir a primeira POC estacionária.

## 8. O que cada referência sustenta

| Fonte | Sustenta | Não sustenta sozinha |
|---|---|---|
| Santos et al. (2018) | utilidade do I Blue It, participação de especialistas e mecânicas respiratórias | acurácia de oxímetro |
| Grimes (2018) | sistema biomédico, PITACO e natureza acadêmica/incremental do ecossistema | equivalência clínica dos sensores futuros |
| Nery et al. (2020) | multimodalidade, sinais involuntários, flexibilidade, complementariedade e segurança | limiares clínicos específicos |
| Dias et al. (2020) | lacuna no uso de IA e biofeedback em jogos para reabilitação respiratória | validação de um algoritmo específico de SpO₂ |
| Dias (2024) | Flow Psicofisiológico, DeepDDA, integração de biossinais e prova de conceito | calibração do conjunto óptico atual |
| Analog Devices/Maxim | princípio AC/DC, razão `R`, necessidade de calibração e aspectos do sensor | validação automática do módulo montado pelo projeto |
| Equipamento de referência | comparação experimental no protocolo definido | demonstração completa apenas por coincidência pontual de valores |

## 9. Formulação academicamente defensável

Durante a fase atual, a redação adequada é:

> Foi desenvolvida uma prova de conceito de aquisição e processamento fotopletismográfico baseada em sensor da família MAX3010x, capaz de estimar frequência cardíaca e calcular uma estimativa experimental de SpO₂ por razão dos quocientes. A curva de SpO₂ permanece não calibrada para o conjunto óptico final e, portanto, os resultados não possuem finalidade diagnóstica ou clínica.

Após calibração e validação, a redação pode descrever:

- protocolo;
- equipamento de referência;
- população/amostra;
- domínio;
- coeficientes;
- métricas de erro e concordância;
- limitações;
- versão do hardware e software.

## Referências

- DIAS, Claudinei. *Flow Psicofisiológico em Jogos Digitais: Inteligência Artificial em Jogos Sérios Multimodais para Reabilitação Respiratória*. UDESC, 2024.
- DIAS, C. et al. *Uso da Inteligência Artificial em Jogos Digitais aplicados à Reabilitação Respiratória: um mapeamento sistemático da literatura*. SBGames, 2020.
- GRIMES, Renato Hartmann. *Sistema Biomédico (com Jogo Sério e Dispositivo Especial) para Reabilitação Respiratória*. UDESC, 2018.
- NÉRY, J. T. C.; HENRIQUE, Y. A. M.; HOUNSELL, M. S. *123-SGR: Uma Arquitetura para Jogos Sérios Multimodais para Reabilitação*. SBGames, 2020.
- SANTOS, A. M. et al. *I Blue It: Um Jogo Sério para auxiliar na Reabilitação Respiratória*. SBGames, 2018.
- Analog Devices / Maxim Integrated. *Guidelines for SpO₂ Measurement* e documentação técnica da família MAX3010x.
