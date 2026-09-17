/*
 * GATE 01 - INTEGRIDADE DO SINAL RAW
 * ==================================
 *
 * PERGUNTA DO GATE
 * ----------------
 * "Existe sinal util nos dois canais e a aquisicao esta livre de condicoes
 *  obviamente invalidas, como sinal quase nulo/flatline ou saturacao?"
 *
 * FUNDAMENTACAO PRINCIPAL
 * -----------------------
 * [G1-01]
 * On-Device Integrated PPG Quality Assessment and Sensor
 * Disconnection/Saturation Detection System for IoT Health Monitoring.
 * Ano: 2020
 * DOI: 10.1109/TIM.2020.2971132
 *
 * Do trabalho usamos o PRINCIPIO de:
 * - detectar cedo sinal nearly-zero / desconexao;
 * - detectar saturacao;
 * - executar verificacoes baratas antes de processamento mais caro;
 * - avaliar o sinal em janelas.
 *
 * COMPLEMENTO
 * -----------
 * Fischer et al. - An Algorithm for Real-Time Pulse Waveform Segmentation
 * and Artifact Detection in Photoplethysmograms.
 * DOI: 10.1109/JBHI.2016.2518202
 *
 * Do trabalho usamos como reforco o principio de verificar clipping no RAW
 * antes de etapas de filtragem/processamento morfologico.
 *
 * NAO COPIAR THRESHOLDS DO ARTIGO
 * -------------------------------
 * O estudo de Reddy usa limites associados ao seu quantizador/dispositivo.
 * Valores como 0-3 e 1020-1023 NAO sao transferiveis diretamente ao MAX30102.
 * Os thresholds abaixo sao PROVISORIOS para a fase de bancada e devem ser
 * recalibrados com dados do nosso sensor, montagem e futuro pegador.
 *
 * O QUE ESTAMOS VALIDANDO
 * -----------------------
 *
 *        RAW RED --------------------+
 *          |                         |
 *          +--> sinal presente? -----+
 *          +--> range suficiente? ---+----> TODOS OK? ----> G1 PASS
 *          +--> saturado/clippado? --+          |
 *                                               +----------> G1 FAIL
 *        RAW IR ---------------------+                         |
 *          |                                                   v
 *          +--> sinal presente? -------------------------- INVALID
 *          +--> range suficiente?
 *          +--> saturado/clippado?
 *
 * Regra de projeto: RED e IR precisam passar. A SpO2 depende dos dois canais.
 */

// MAX30102 entrega amostras de ate 18 bits.
static const uint32_t G1_ADC_MAX_CODE = 262143UL;

/*
 * PARAMETROS PROVISORIOS DE BANCADA
 * ---------------------------------
 * Estes valores NAO sao thresholds publicados nos artigos.
 * Sao pontos de partida conservadores para caracterizacao no MAX30102.
 *
 * Nosso bring-up mostrou:
 * - sem dedo: valores tipicamente na ordem de centenas;
 * - com dedo: RED/IR na ordem de 1e5.
 *
 * Por isso, 5000 separa apenas uma condicao obviamente sem sinal util da
 * condicao de contato observada. Esse numero devera ser recalibrado.
 */
static const uint32_t G1_MIN_MEAN_SIGNAL = 5000UL;
static const uint32_t G1_MIN_DYNAMIC_RANGE = 500UL;
static const uint32_t G1_SATURATION_MARGIN = 2048UL;
static const uint32_t G1_SATURATION_LEVEL =
  G1_ADC_MAX_CODE - G1_SATURATION_MARGIN;

// Nesta primeira validacao qualquer amostra claramente saturada reprova.
static const uint16_t G1_MAX_SATURATED_SAMPLES = 0;

// Codigos internos de falha. Evitamos String para poupar SRAM no Uno.
static const uint8_t G1_OK = 0;
static const uint8_t G1_FAIL_NO_SAMPLES = 1;
static const uint8_t G1_FAIL_RED_LOW_SIGNAL = 2;
static const uint8_t G1_FAIL_IR_LOW_SIGNAL = 3;
static const uint8_t G1_FAIL_RED_FLATLINE = 4;
static const uint8_t G1_FAIL_IR_FLATLINE = 5;
static const uint8_t G1_FAIL_RED_SATURATION = 6;
static const uint8_t G1_FAIL_IR_SATURATION = 7;

// Estado incremental da janela. Nao armazenamos 5 s de RAW em RAM.
static uint32_t g1RedMin;
static uint32_t g1RedMax;
static uint32_t g1IrMin;
static uint32_t g1IrMax;
static uint32_t g1RedSum;
static uint32_t g1IrSum;
static uint16_t g1RedSaturatedCount;
static uint16_t g1IrSaturatedCount;
static uint16_t g1SampleCount;
static uint8_t g1FailReason;
static bool g1LastPass;

void gate1Reset()
{
  g1RedMin = G1_ADC_MAX_CODE;
  g1RedMax = 0;
  g1IrMin = G1_ADC_MAX_CODE;
  g1IrMax = 0;

  g1RedSum = 0;
  g1IrSum = 0;

  g1RedSaturatedCount = 0;
  g1IrSaturatedCount = 0;
  g1SampleCount = 0;

  g1FailReason = G1_OK;
  g1LastPass = false;
}

void gate1AddSample(uint32_t red, uint32_t ir)
{
  if (red < g1RedMin) g1RedMin = red;
  if (red > g1RedMax) g1RedMax = red;

  if (ir < g1IrMin) g1IrMin = ir;
  if (ir > g1IrMax) g1IrMax = ir;

  /*
   * Em uma janela de 5 s, mesmo 500 amostras no valor maximo do ADC ficam
   * confortavelmente abaixo do limite de uint32_t (~131 milhoes < 4.29e9).
   */
  g1RedSum += red;
  g1IrSum += ir;

  if (red >= G1_SATURATION_LEVEL) g1RedSaturatedCount++;
  if (ir >= G1_SATURATION_LEVEL) g1IrSaturatedCount++;

  if (g1SampleCount < 65535U) g1SampleCount++;
}

bool gate1Evaluate()
{
  g1FailReason = G1_OK;
  g1LastPass = false;

  if (g1SampleCount == 0)
  {
    g1FailReason = G1_FAIL_NO_SAMPLES;
    return false;
  }

  const uint32_t redMean = g1RedSum / g1SampleCount;
  const uint32_t irMean = g1IrSum / g1SampleCount;

  const uint32_t redRange = g1RedMax - g1RedMin;
  const uint32_t irRange = g1IrMax - g1IrMin;

  // 1) Ha energia/sinal optico minimamente presente?
  if (redMean < G1_MIN_MEAN_SIGNAL)
  {
    g1FailReason = G1_FAIL_RED_LOW_SIGNAL;
    return false;
  }

  if (irMean < G1_MIN_MEAN_SIGNAL)
  {
    g1FailReason = G1_FAIL_IR_LOW_SIGNAL;
    return false;
  }

  // 2) O sinal nao esta praticamente constante/flatline?
  if (redRange < G1_MIN_DYNAMIC_RANGE)
  {
    g1FailReason = G1_FAIL_RED_FLATLINE;
    return false;
  }

  if (irRange < G1_MIN_DYNAMIC_RANGE)
  {
    g1FailReason = G1_FAIL_IR_FLATLINE;
    return false;
  }

  // 3) O ADC nao esta encostando no teto (clipping/saturacao)?
  if (g1RedSaturatedCount > G1_MAX_SATURATED_SAMPLES)
  {
    g1FailReason = G1_FAIL_RED_SATURATION;
    return false;
  }

  if (g1IrSaturatedCount > G1_MAX_SATURATED_SAMPLES)
  {
    g1FailReason = G1_FAIL_IR_SATURATION;
    return false;
  }

  g1LastPass = true;
  return true;
}

void gate1PrintFailReason()
{
  switch (g1FailReason)
  {
    case G1_OK:
      Serial.print(F("NONE"));
      break;
    case G1_FAIL_NO_SAMPLES:
      Serial.print(F("NO_SAMPLES"));
      break;
    case G1_FAIL_RED_LOW_SIGNAL:
      Serial.print(F("RED_LOW_SIGNAL"));
      break;
    case G1_FAIL_IR_LOW_SIGNAL:
      Serial.print(F("IR_LOW_SIGNAL"));
      break;
    case G1_FAIL_RED_FLATLINE:
      Serial.print(F("RED_FLATLINE"));
      break;
    case G1_FAIL_IR_FLATLINE:
      Serial.print(F("IR_FLATLINE"));
      break;
    case G1_FAIL_RED_SATURATION:
      Serial.print(F("RED_SATURATION"));
      break;
    case G1_FAIL_IR_SATURATION:
      Serial.print(F("IR_SATURATION"));
      break;
    default:
      Serial.print(F("UNKNOWN"));
      break;
  }
}

void gate1PrintReport()
{
  Serial.print(F("G1_RESULT="));
  Serial.print(g1LastPass ? F("PASS") : F("FAIL"));

  Serial.print(F(" reason="));
  gate1PrintFailReason();

  Serial.print(F(" samples="));
  Serial.print(g1SampleCount);

  if (g1SampleCount > 0)
  {
    const uint32_t redMean = g1RedSum / g1SampleCount;
    const uint32_t irMean = g1IrSum / g1SampleCount;
    const uint32_t redRange = g1RedMax - g1RedMin;
    const uint32_t irRange = g1IrMax - g1IrMin;

    Serial.print(F(" RED[min="));
    Serial.print(g1RedMin);
    Serial.print(F(" max="));
    Serial.print(g1RedMax);
    Serial.print(F(" mean="));
    Serial.print(redMean);
    Serial.print(F(" range="));
    Serial.print(redRange);
    Serial.print(F(" sat="));
    Serial.print(g1RedSaturatedCount);
    Serial.print(F("]"));

    Serial.print(F(" IR[min="));
    Serial.print(g1IrMin);
    Serial.print(F(" max="));
    Serial.print(g1IrMax);
    Serial.print(F(" mean="));
    Serial.print(irMean);
    Serial.print(F(" range="));
    Serial.print(irRange);
    Serial.print(F(" sat="));
    Serial.print(g1IrSaturatedCount);
    Serial.print(F("]"));
  }

  Serial.println();
}
