/*
 * GATE 02 - PULSATILIDADE (REPLICACAO ADAPTADA / MODO DIAGNOSTICO)
 * =================================================================
 *
 * BASE PRINCIPAL
 * --------------
 * Vadrevu, S.; Manikandan, M. S. (2019)
 * "Real-Time PPG Signal Quality Assessment System for Improving Battery
 *  Life and False Alarms"
 * DOI: 10.1109/TCSII.2019.2891636
 *
 * O trabalho-base descreve um SQA hierarquico usando features simples:
 * - amplitude;
 * - threshold crossing rate;
 * - features da autocorrelacao (ACF).
 *
 * IMPORTANTE: do material primario atualmente verificado, tratamos como
 * confirmadas apenas as FAMILIAS amplitude, threshold crossing rate e features
 * de autocorrelacao. FZCP/pico/lag existem na V1 historica do projeto e em
 * descricoes secundarias do metodo, mas permanecem explicitamente marcados
 * como elementos a confirmar no texto integral da fonte primaria.
 *
 * IMPORTANTE SOBRE A REPLICACAO
 * -----------------------------
 * Este arquivo NAO copia thresholds do artigo.
 * Ele replica as FAMILIAS DE FEATURES e a logica de extracao, adaptando:
 * - armazenamento para Packed18 (ATmega328P / 2 KB SRAM);
 * - processamento para inteiros/float apenas temporarios;
 * - dois canais RED e IR do MAX30102;
 * - taxa efetiva observada de aproximadamente 25 amostras/s.
 *
 * Nesta fase NAO existe G2 PASS/FAIL. Primeiro medimos as features reais.
 *
 * RELACAO COM A V1 ESP32
 * ----------------------
 * A V1 arquivada do projeto ja possui:
 * - AC RMS;
 * - threshold crossings;
 * - ACF normalizada;
 * - FZCP;
 * - pico/lag da ACF;
 * - periodo.
 *
 * Aqui portamos esse nucleo de forma compativel com a SRAM do Uno/Nano.
 *
 * O QUE E DIRETAMENTE ALINHADO AO TRABALHO-BASE
 * ---------------------------------------------
 * - familia de amplitude;
 * - threshold crossing rate;
 * - familia de features da ACF.
 *
 * O QUE VEM DA V1 / AINDA REQUER CONFIRMACAO NA FONTE PRIMARIA
 * ------------------------------------------------------------
 * - FZCP;
 * - pico da ACF;
 * - lag do pico.
 *
 * O QUE E AUXILIAR DO NOSSO PORT
 * ------------------------------
 * - absoluteAmplitude = max |x-mean| como representacao diagnostica da amplitude;
 * - AC_RMS: preservado como metrica auxiliar da V1;
 * - period_cpm: derivado do lag para facilitar interpretacao;
 * - RED e IR calculados separadamente;
 * - tempo de execucao G2_ms e SRAM livre para medir custo no ATmega328P.
 *
 * PREPROCESSAMENTO DESTA VERSAO
 * -----------------------------
 * Apenas remocao da media:
 *
 *   x_ac[i] = x[i] - mean(x)
 *
 * O filtro Butterworth/Hamming existentes na V1 NAO sao assumidos aqui como
 * requisitos do artigo-base sem correspondencia documental confirmada.
 * Se forem reincorporados, isso sera registrado explicitamente.
 */

#include <math.h>

#ifndef ENABLE_GATE2_DIAGNOSTIC
#define ENABLE_GATE2_DIAGNOSTIC 0
#endif

#if ENABLE_GATE2_DIAGNOSTIC

static const uint16_t G2_EFFECTIVE_FS_HZ = 25;

// Para caracterizacao da ACF usamos ate metade da janela.
// Evita comparar lags com pouquissimos pares e nao impoe ainda faixa clinica.
static const uint8_t G2_MAX_LAG_DIVISOR = 2;

struct Gate2Metrics
{
  uint16_t samples;
  uint32_t mean;

  // Amplitude
  uint32_t absoluteAmplitude;
  uint32_t acRms;

  // Threshold crossings
  uint16_t crossings;
  uint16_t crossingRatePermille;

  // ACF
  bool hasFzcp;
  uint8_t fzcpLag;
  int16_t acfPeakPermille;
  uint8_t acfPeakLag;
  bool hasPeriodicPeak;

  // Derivada do lag, apenas para leitura diagnostica.
  // So e calculada quando existe FZCP seguido de pico positivo.
  uint16_t periodCpm;
};

uint32_t gate2GetChannelSample(bool redChannel, uint16_t index)
{
  return redChannel ? packed18GetRed(index) : packed18GetIr(index);
}

int32_t gate2CenteredSample(bool redChannel, uint16_t index, uint32_t mean)
{
  return (int32_t)gate2GetChannelSample(redChannel, index) - (int32_t)mean;
}

uint32_t gate2Abs32(int32_t value)
{
  return (value < 0) ? (uint32_t)(-(int64_t)value) : (uint32_t)value;
}

/*
 * ACF normalizada de referencia:
 *
 *               sum(a[i] * b[i])
 * R(k) = --------------------------------
 *        sqrt(sum(a[i]^2)) * sqrt(sum(b[i]^2))
 *
 * a[i] = x_ac[i]
 * b[i] = x_ac[i+k]
 *
 * Retornamos em permille:
 *   -1000 ~= -1.0
 *       0 ~=  0.0
 *    1000 ~= +1.0
 *
 * uint64_t/int64_t evitam overflow na soma dos produtos.
 * float e usado apenas na normalizacao/sqrt, sem buffers float.
 */
int16_t gate2AcfPermille(
  bool redChannel,
  uint16_t samples,
  uint32_t mean,
  uint8_t lag
)
{
  if (lag == 0 || lag >= samples)
  {
    return 0;
  }

  int64_t numerator = 0;
  uint64_t energyA = 0;
  uint64_t energyB = 0;

  const uint16_t pairs = samples - lag;

  for (uint16_t i = 0; i < pairs; i++)
  {
    const int32_t a = gate2CenteredSample(redChannel, i, mean);
    const int32_t b = gate2CenteredSample(redChannel, i + lag, mean);

    const int64_t a64 = (int64_t)a;
    const int64_t b64 = (int64_t)b;

    numerator += a64 * b64;
    energyA += (uint64_t)(a64 * a64);
    energyB += (uint64_t)(b64 * b64);
  }

  if (energyA == 0 || energyB == 0)
  {
    return 0;
  }

  // sqrt separado evita multiplicar duas energias grandes em inteiro.
  const float denominator =
    sqrtf((float)energyA) * sqrtf((float)energyB);

  if (denominator <= 0.0f)
  {
    return 0;
  }

  float r = (float)numerator / denominator;

  if (r > 1.0f) r = 1.0f;
  if (r < -1.0f) r = -1.0f;

  return (int16_t)(r * 1000.0f);
}

Gate2Metrics gate2AnalyzeChannel(bool redChannel)
{
  Gate2Metrics m;

  m.samples = packed18GetCount();
  m.mean = 0;
  m.absoluteAmplitude = 0;
  m.acRms = 0;
  m.crossings = 0;
  m.crossingRatePermille = 0;
  m.hasFzcp = false;
  m.fzcpLag = 0;
  m.acfPeakPermille = -1000;
  m.acfPeakLag = 0;
  m.hasPeriodicPeak = false;
  m.periodCpm = 0;

  if (m.samples < 3)
  {
    return m;
  }

  /*
   * 1) DC / media.
   * 100 * 262143 = 26.214.300, cabe em uint32_t.
   */
  uint32_t sum = 0;

  for (uint16_t i = 0; i < m.samples; i++)
  {
    sum += gate2GetChannelSample(redChannel, i);
  }

  m.mean = sum / m.samples;

  /*
   * 2) Amplitude absoluta + AC RMS.
   *    absoluteAmplitude = max |x - mean|
   *    AC_RMS = sqrt(mean((x-mean)^2))
   *
   * AC_RMS e auxiliar da V1. A amplitude absoluta e mantida separada para
   * aproximar a familia de amplitude descrita no trabalho-base.
   */
  uint64_t sumSquares = 0;

  /*
   * 3) Threshold crossings.
   * Igual a V1: pontos exatamente no threshold sao ignorados.
   */
  int8_t previousSide = 0;

  for (uint16_t i = 0; i < m.samples; i++)
  {
    const int32_t centered = gate2CenteredSample(redChannel, i, m.mean);
    const uint32_t absCentered = gate2Abs32(centered);

    if (absCentered > m.absoluteAmplitude)
    {
      m.absoluteAmplitude = absCentered;
    }

    const int64_t c64 = (int64_t)centered;
    sumSquares += (uint64_t)(c64 * c64);

    int8_t side = 0;
    if (centered > 0) side = 1;
    else if (centered < 0) side = -1;

    if (side != 0)
    {
      if (previousSide != 0 && side != previousSide)
      {
        m.crossings++;
      }

      previousSide = side;
    }
  }

  m.acRms = (uint32_t)sqrtf(
    (float)sumSquares / (float)m.samples
  );

  m.crossingRatePermille =
    (uint16_t)(((uint32_t)m.crossings * 1000UL) / (m.samples - 1U));

  /*
   * 4) ACF features.
   *
   * R(0)=1 por definicao. Procuramos:
   * - primeiro cruzamento positivo -> <=0 (FZCP);
   * - maior pico positivo APOS o FZCP;
   * - lag desse pico.
   *
   * Se nao houver FZCP na metade inicial da janela, mantemos o maior R(k)
   * observado apos lag=1 apenas como diagnostico, mas hasFzcp=false deixa
   * explicito que a forma esperada da ACF nao foi encontrada.
   */
  uint8_t maxLag = (uint8_t)(m.samples / G2_MAX_LAG_DIVISOR);

  if (maxLag >= m.samples)
  {
    maxLag = (uint8_t)(m.samples - 1U);
  }

  int16_t previousR = 1000; // R(0)
  int16_t fallbackPeak = -1000;
  uint8_t fallbackLag = 0;

  for (uint8_t lag = 1; lag <= maxLag; lag++)
  {
    const int16_t r = gate2AcfPermille(
      redChannel,
      m.samples,
      m.mean,
      lag
    );

    if (r > fallbackPeak)
    {
      fallbackPeak = r;
      fallbackLag = lag;
    }

    if (!m.hasFzcp && previousR > 0 && r <= 0)
    {
      m.hasFzcp = true;
      m.fzcpLag = lag;
    }
    else if (m.hasFzcp && r > m.acfPeakPermille)
    {
      m.acfPeakPermille = r;
      m.acfPeakLag = lag;
    }

    previousR = r;
  }

  if (!m.hasFzcp || m.acfPeakLag == 0)
  {
    m.acfPeakPermille = fallbackPeak;
    m.acfPeakLag = fallbackLag;
  }

  /*
   * Nao transformar um fallback de ACF em "periodo" quando a estrutura minima
   * esperada nao foi encontrada. O periodo diagnostico so existe quando:
   * - houve FZCP;
   * - o pico selecionado ocorre depois do FZCP;
   * - esse pico e positivo.
   *
   * Isto nao e um threshold fisiologico; e apenas uma condicao estrutural para
   * evitar period_cpm enganoso (por exemplo, lag=1 em sinais suaves).
   */
  m.hasPeriodicPeak =
    m.hasFzcp
    && (m.acfPeakLag > m.fzcpLag)
    && (m.acfPeakPermille > 0);

  if (m.hasPeriodicPeak)
  {
    m.periodCpm =
      (uint16_t)(((uint32_t)60U * G2_EFFECTIVE_FS_HZ) / m.acfPeakLag);
  }

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

  Serial.print(F(" mean="));
  Serial.print(m.mean);

  Serial.print(F(" absAmp="));
  Serial.print(m.absoluteAmplitude);

  Serial.print(F(" acRms="));
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

  Serial.print(F(" periodicPeak="));
  Serial.print(m.hasPeriodicPeak ? 1 : 0);

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

  Serial.print(F("G2_DIAGNOSTIC "));
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
