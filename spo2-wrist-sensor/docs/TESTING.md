# Estratégia de testes do SQI

## Princípio

Testar cada gate isoladamente antes do pipeline completo.

## Estrutura sugerida

```text
tests/
└── sqi/
    ├── g1_integrity/
    ├── g2_pulsatility/
    ├── g3_morphology/
    ├── g4_channels/
    ├── features/
    └── integration/
```

## Cenários mínimos

### G1
- sinal constante;
- sinal quase constante;
- saturação superior;
- saturação inferior;
- clipping parcial;
- timestamps descontínuos;
- canal RED válido e IR inválido;
- IR válido e RED inválido.

### G2
- pulso limpo;
- ruído aleatório;
- ruído de alta frequência;
- sinal de baixa amplitude;
- sinal periódico não fisiológico;
- canal único pulsátil.

### G3
- pulsos estáveis;
- amplitude fortemente variável;
- largura anormal;
- rise time anormal;
- pulsos deformados;
- número insuficiente de beats.

### G4
- RED/IR coerentes;
- períodos divergentes;
- contagem divergente;
- desalinhamento temporal;
- um canal degradado.

## Tipos de dados para teste

1. sinais sintéticos controlados;
2. sinais RAW gravados do MAX30102;
3. sessões sem pegador;
4. sessões futuras com pegador;
5. casos deliberados de mau contato/movimento.

## Critérios de regressão

Cada mudança em threshold ou feature deve registrar:

- conjunto de dados usado;
- versão de configuração;
- taxa de aprovação/rejeição;
- distribuição de falhas por gate;
- impacto sobre SpO₂/FC derivados.

O objetivo é evitar ajustar thresholds apenas observando casos individuais.
