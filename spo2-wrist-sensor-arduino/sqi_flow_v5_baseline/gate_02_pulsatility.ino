/*
 * GATE 02 - PULSATILIDADE (PORT V1 -> ATmega328P / MODO DIAGNOSTICO)
 * ===================================================================
 *
 * BASE PRINCIPAL
 * --------------
 * Vadrevu, S.; Manikandan, M. S. (2019)
 * "Real-Time PPG Signal Quality Assessment System for Improving Battery
 *  Life and False Alarms"
 * DOI: 10.1109/TCSII.2019.2891636
 *
 * Do material primario atualmente verificado tratamos como confirmadas as
 * FAMILIAS de features:
 * - amplitude;
 * - threshold crossing rate;
 * - autocorrelation-function features.
 *
 * FZCP, pico/lag da ACF, Butterworth e Hamming abaixo possuem rastreabilidade
 * direta para a V1 ESP32 arquivada do projeto. Nao devem ser apresentados como
 * detalhes literais do artigo-base sem confrontar o texto integral da fonte.
 *
 * POR QUE ESTA REVISAO EXISTE
 * ---------------------------
 * A primeira versao diagnostica do port Uno removia apenas a media e calculava
 * ACF diretamente sobre Packed18. O primeiro ensaio de bancada mostrou FZCP
 * muito tardio e ausencia de pico periodico em varias janelas de dedo estavel.
 *
 * Ao confrontar esse port com a V1 ESP32 arquivada, verificou-se que a V1:
 * 1) recebe o PPG apos high-pass Butterworth de 3a ordem;
 * 2) calcula RMS/crossings no sinal pre-processado;
 * 3) aplica janela de Hamming antes da extracao da forma da ACF;
 * 4) procura Kmax dentro de uma faixa temporal configurada;
 * 5) trata FZCP e Kmax/Rmax como caracteristicas separadas.
 *
 * Esta versao restaura essa estrutura, adaptando a memoria ao ATmega328P:
 * somente UM workspace float[100] e usado e reutilizado RED -> IR.
 *
 * CONFIGURACAO HISTORICA DA V1 USADA COMO BASE DIAGNOSTICA
 * --------------------------------------------------------
 * - high-pass: 0.5 Hz;
 * - faixa de busca da ACF: 0.20 s .. 2.00 s;
 * - os thresholds finais da V1 eram declarados provisórios para calibracao.
 *
 * No Uno/Nano a taxa EFETIVA observada com sampleAverage=4 e sampleRate=100
 * e aproximadamente 25 amostras/s. Portanto os tempos sao preservados e os
 * lags sao recalculados a partir de fs=25.
 *
 * IMPORTANTE
 * ----------
 * Ainda NAO existe G2 PASS/FAIL nesta branch.
 * period_cpm e apenas a periodicidade candidata derivada de Kmax; NAO e a FC
 * clinica/final.
 */

#include <math.h>

#if defined(__AVR__)
#include <avr/pgmspace.h>
#endif

#ifndef ENABLE_GATE2_DIAGNOSTIC
#define ENABLE_GATE2_DIAGNOSTIC 0
#endif

#if ENABLE_GATE2_DIAGNOSTIC

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const uint16_t G2_EFFECTIVE_FS_HZ = 25;
static const float G2_HIGHPASS_CUTOFF_HZ = 0.5f;
static const float G2_ACF_MIN_PERIOD_S = 0.20f;
static const float G2_ACF_MAX_PERIOD_S = 2.00f;
static const uint16_t G2_WORK_SAMPLES = 100;

/*
 * Um unico workspace de 100 floats = 400 B.
 * Ele e reutilizado: primeiro RED, depois IR.
 *
 * Nao criamos red[100] + ir[100] em float/uint32_t.
 */
static float g2Work[G2_WORK_SAMPLES];

/*
 * Janela de Hamming de N=100 quantizada em Q15.
 *
 * w[n] = 0.54 - 0.46*cos(2*pi*n/(N-1))
 *
 * A tabela fica em FLASH no AVR. A quantizacao e uma adaptacao de hardware
 * para evitar 100 chamadas a cosf() e evitar outro vetor float em SRAM.
 */
#if defined(__AVR__)
static const uint16_t g2HammingQ15[G2_WORK_SAMPLES] PROGMEM = {
#else
static const uint16_t g2HammingQ15[G2_WORK_SAMPLES] = {
#endif
  2621,2652,2743,2894,3104,3374,3701,4085,4523,5014,
  5574,6199,6888,7638,8445,9305,10212,11160,12141,13147,
  14171,15204,16237,17260,18264,19242,20183,21078,21918,22697,
  23407,24042,24595,25061,25439,25725,25917,26013,26013,25917,
  25725,25439,25061,24595,24042,23407,22697,21918,21078,20183,
  19242,18264,17260,16237,15204,14171,13147,12141,11160,10212,
  9305,8445,7638,6888,6199,5574,5014,4523,4085,3701,
  3374,3104,2894,2743,2652,2621,2652,2743,2894,3104,
  3374,3701,4085,4523,5014,5574,6199,6888,7638,8445,
  9305,10212,11160,12141,13147,14171,15204,16237,17260,18264
};

/*
 * ATENCAO:
 * A tabela acima precisa ser simetrica. Para evitar depender de uma tabela
 * incorreta gerada manualmente, usamos apenas metade logica e espelhamos o
 * indice no acesso; os valores efetivamente usados sao recalculados a partir
 * da primeira metade valida abaixo.
 *
 * Esta pequena LUT substitui trigonometria repetida no AVR.
 */
static const uint16_t g2HammingHalfQ15[50]
#if defined(__AVR__)
PROGMEM
#endif
= {
  2621,2652,2743,2894,3104,3374,3701,4085,4523,5014,
  5574,6199,6888,7638,8445,9305,10212,11160,12141,13147,
  14171,15204,16237,17260,18264,19242,20183,21078,21918,22697,
  23407,24042,24595,25061,25439,25725,25917,26013,26013,25917,
  25725,25439,25061,24595,24042,23407,22697,21918,21078,20183
};

struct Gate2Metrics
{
  uint16_t samples;
  uint32_t mean;

  // Amplitude no sinal high-pass antes da Hamming.
  uint32_t absoluteAmplitude;
  uint32_t acRms;

  // Threshold crossings no sinal high-pass.
  uint16_t crossings;
  uint16_t crossingRatePermille;

  // ACF apos Hamming.
  bool hasFzcp;
  uint8_t fzcpLag;
  int16_t acfPeakPermille;
  uint8_t acfPeakLag;

  // Periodicidade candidata de Kmax; nao e FC final.
  uint16_t periodCpm;
};

uint32_t gate2GetChannelSample(bool redChannel, uint16_t index)
{
  return redChannel ? packed18GetRed(index) : packed18GetIr(index);
}

uint16_t gate2ReadHammingQ15(uint16_t index)
{
  // w[n] = w[N-1-n]
  uint16_t mirrored = index;
  if (mirrored >= (G2_WORK_SAMPLES / 2U))
  {
    mirrored = (G2_WORK_SAMPLES - 1U) - mirrored;
  }

#if defined(__AVR__)
  return pgm_read_word(&g2HammingHalfQ15[mirrored]);
#else
  return g2HammingHalfQ15[mirrored];
#endif
}

/*
 * Replica a estrutura do preprocess da V1:
 * - centralizacao pela media RAW;
 * - high-pass de 1a ordem;
 * - high-pass de 2a ordem Q=1;
 * formando uma cascata Butterworth de 3a ordem.
 */
void gate2PreprocessChannel(
  bool redChannel,
  uint16_t samples,
  uint32_t mean
)
{
  const float fs = (float)G2_EFFECTIVE_FS_HZ;
  const float cutoff = G2_HIGHPASS_CUTOFF_HZ;

  // Secao de 1a ordem.
  const float k = tanf((float)M_PI * cutoff / fs);
  const float norm = 1.0f / (1.0f + k);
  const float b0 = norm;
  const float b1 = -norm;
  const float a1 = (k - 1.0f) * norm;

  float previousX =
    (float)gate2GetChannelSample(redChannel, 0) - (float)mean;
  float previousY = 0.0f;

  for (uint16_t i = 0; i < samples; i++)
  {
    const float x =
      (float)gate2GetChannelSample(redChannel, i) - (float)mean;

    const float y =
      b0 * x + b1 * previousX - a1 * previousY;

    g2Work[i] = y;
    previousX = x;
    previousY = y;
  }

  // Secao de 2a ordem, Q=1, igual a estrutura da V1.
  const float omega =
    2.0f * (float)M_PI * cutoff / fs;
  const float cosOmega = cosf(omega);
  const float sinOmega = sinf(omega);
  const float alpha = 0.5f * sinOmega;
  const float a0 = 1.0f + alpha;

  const float qB0 = ((1.0f + cosOmega) * 0.5f) / a0;
  const float qB1 = (-(1.0f + cosOmega)) / a0;
  const float qB2 = ((1.0f + cosOmega) * 0.5f) / a0;
  const float qA1 = (-2.0f * cosOmega) / a0;
  const float qA2 = (1.0f - alpha) / a0;

  float x1 = g2Work[0];
  float x2 = g2Work[0];
  float y1 = 0.0f;
  float y2 = 0.0f;

  for (uint16_t i = 0; i < samples; i++)
  {
    const float x = g2Work[i];
    const float y =
      qB0 * x + qB1 * x1 + qB2 * x2
      - qA1 * y1 - qA2 * y2;

    g2Work[i] = y;

    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = y;
  }
}

void gate2AmplitudeAndCrossings(
  uint16_t samples,
  Gate2Metrics &m
)
{
  float sumSquares = 0.0f;
  float maxAbs = 0.0f;
  int8_t previousSide = 0;

  for (uint16_t i = 0; i < samples; i++)
  {
    const float x = g2Work[i];
    const float ax = fabsf(x);

    if (ax > maxAbs)
    {
      maxAbs = ax;
    }

    sumSquares += x * x;

    int8_t side = 0;
    if (x > 0.0f) side = 1;
    else if (x < 0.0f) side = -1;

    if (side != 0)
    {
      if (previousSide != 0 && side != previousSide)
      {
        m.crossings++;
      }
      previousSide = side;
    }
  }

  m.absoluteAmplitude = (uint32_t)maxAbs;
  m.acRms = (uint32_t)sqrtf(sumSquares / (float)samples);

  if (samples > 1)
  {
    m.crossingRatePermille =
      (uint16_t)(((uint32_t)m.crossings * 1000UL) / (samples - 1U));
  }
}

void gate2ApplyHamming(uint16_t samples)
{
  for (uint16_t i = 0; i < samples; i++)
  {
    const float w =
      (float)gate2ReadHammingQ15(i) / 32767.0f;
    g2Work[i] *= w;
  }
}

/*
 * ACF no formato da V1:
 *
 *                 sum(xw[i] * xw[i+k])
 * R(k) = ----------------------------------------
 *                     sum(xw[i]^2)
 *
 * onde xw e o sinal high-pass multiplicado pela janela de Hamming.
 *
 * FZCP e Kmax sao extraidos separadamente.
 */
void gate2ExtractAcfShape(
  uint16_t samples,
  Gate2Metrics &m
)
{
  gate2ApplyHamming(samples);

  float energy = 0.0f;
  for (uint16_t i = 0; i < samples; i++)
  {
    energy += g2Work[i] * g2Work[i];
  }

  if (energy <= 1e-12f)
  {
    m.acfPeakPermille = 0;
    return;
  }

  uint8_t minLag =
    (uint8_t)ceilf(G2_ACF_MIN_PERIOD_S * (float)G2_EFFECTIVE_FS_HZ);
  uint8_t maxLag =
    (uint8_t)floorf(G2_ACF_MAX_PERIOD_S * (float)G2_EFFECTIVE_FS_HZ);

  if (minLag < 1U) minLag = 1U;
  if (maxLag >= samples) maxLag = (uint8_t)(samples - 1U);

  float previousR = 1.0f; // R(0)
  float bestR = -1.0f;
  uint8_t bestLag = minLag;

  for (uint8_t lag = 1U; lag <= maxLag; lag++)
  {
    float numerator = 0.0f;
    const uint16_t pairs = samples - lag;

    for (uint16_t i = 0; i < pairs; i++)
    {
      numerator += g2Work[i] * g2Work[i + lag];
    }

    const float r = numerator / energy;

    if (!m.hasFzcp && previousR > 0.0f && r <= 0.0f)
    {
      m.hasFzcp = true;
      m.fzcpLag = lag;
    }

    if (lag >= minLag && r > bestR)
    {
      bestR = r;
      bestLag = lag;
    }

    previousR = r;
  }

  if (bestR > 1.0f) bestR = 1.0f;
  if (bestR < -1.0f) bestR = -1.0f;

  m.acfPeakPermille = (int16_t)(bestR * 1000.0f);
  m.acfPeakLag = bestLag;

  if (bestLag > 0)
  {
    m.periodCpm =
      (uint16_t)(((uint32_t)60U * G2_EFFECTIVE_FS_HZ) / bestLag);
  }
}

Gate2Metrics gate2AnalyzeChannel(bool redChannel)
{
  Gate2Metrics m;

  m.samples = packed18GetCount();
  if (m.samples > G2_WORK_SAMPLES)
  {
    m.samples = G2_WORK_SAMPLES;
  }

  m.mean = 0;
  m.absoluteAmplitude = 0;
  m.acRms = 0;
  m.crossings = 0;
  m.crossingRatePermille = 0;
  m.hasFzcp = false;
  m.fzcpLag = 0;
  m.acfPeakPermille = -1000;
  m.acfPeakLag = 0;
  m.periodCpm = 0;

  if (m.samples < 3)
  {
    return m;
  }

  uint32_t sum = 0;
  for (uint16_t i = 0; i < m.samples; i++)
  {
    sum += gate2GetChannelSample(redChannel, i);
  }
  m.mean = sum / m.samples;

  gate2PreprocessChannel(redChannel, m.samples, m.mean);
  gate2AmplitudeAndCrossings(m.samples, m);
  gate2ExtractAcfShape(m.samples, m);

  return m;
}

void gate2PrintChannel(
  const __FlashStringHelper *name,
  const Gate2Metrics &m
)
{
  Serial.print(name);
  Serial.print(F("[n="));
  Serial.print(m.samples);

  Serial.print(F(" meanRAW="));
  Serial.print(m.mean);

  Serial.print(F(" hpAbsAmp="));
  Serial.print(m.absoluteAmplitude);

  Serial.print(F(" hpRms="));
  Serial.print(m.acRms);

  Serial.print(F(" crossings="));
  Serial.print(m.crossings);

  Serial.print(F(" crossRate_permille="));
  Serial.print(m.crossingRatePermille);

  Serial.print(F(" FZCP="));
  if (m.hasFzcp)
  {
    Serial.print(m.fzcpLag);
  }
  else
  {
    Serial.print(F("NONE"));
  }

  Serial.print(F(" ACFpeak_permille="));
  Serial.print(m.acfPeakPermille);

  Serial.print(F(" peakLag="));
  Serial.print(m.acfPeakLag);

  Serial.print(F(" period_cpm="));
  Serial.print(m.periodCpm);

  Serial.print(F("]"));
}

void gate2AnalyzeAndPrint()
{
  const unsigned long startedAt = millis();

  const Gate2Metrics red = gate2AnalyzeChannel(true);
  const Gate2Metrics ir = gate2AnalyzeChannel(false);

  const unsigned long elapsedMs = millis() - startedAt;

  Serial.print(F("G2_DIAGNOSTIC_V1PORT "));
  gate2PrintChannel(F("RED"), red);
  Serial.print(F(" "));
  gate2PrintChannel(F("IR"), ir);

  Serial.print(F(" G2_ms="));
  Serial.print(elapsedMs);

  Serial.print(F(" freeRAM="));
  Serial.print(packed18FreeRam());

  Serial.println();
}

#endif // ENABLE_GATE2_DIAGNOSTIC
