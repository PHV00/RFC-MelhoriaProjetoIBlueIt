/*
 * GATE 02 - PULSATILIDADE (MODO DIAGNOSTICO)
 * ==========================================
 *
 * OBJETIVO DESTA ETAPA
 * --------------------
 * Ainda NAO decidir PASS/FAIL.
 *
 * Primeiro caracterizamos, no nosso MAX30102 + ATmega328P, tres propriedades
 * simples da janela que passou no G1:
 *
 * 1) AC/DC proxy:
 *    mede quanto o sinal varia em torno do nivel medio.
 *    Aqui usamos MAD/DC = mean(|x - mean|) / mean.
 *    Nao e uma definicao universal de AC/DC; e uma metrica de bancada barata,
 *    explicita e reprodutivel.
 *
 * 2) crossings:
 *    conta quantas vezes o sinal cruza a propria media.
 *    Uma janela pulsatile tende a produzir cruzamentos repetidos; transientes
 *    isolados tendem a produzir outra assinatura temporal.
 *
 * 3) autocorrelacao (ACF):
 *    procura repeticao da forma de onda em diferentes atrasos (lags).
 *    O melhor lag e o atraso com maior correlacao positiva normalizada.
 *
 * FUNDAMENTACAO
 * -------------
 * Vadrevu et al. - Real-Time PPG Signal Quality Assessment System for
 * Improving Battery Life and False Alarms.
 * DOI: 10.1109/TCSII.2019.2891636
 *
 * A literatura apoia o uso de caracteristicas temporais/periodicas para
 * avaliar qualidade PPG. Esta implementacao e uma adaptacao de engenharia
 * para o nosso hardware, nao uma copia literal dos thresholds do artigo.
 *
 * MEMORIA
 * -------
 * O G2 reutiliza o Packed18 ja existente. Nenhum uint32_t[100] adicional e
 * criado. Cada amostra e reconstruida sob demanda.
 *
 * JANELA
 * ------
 * O buffer possui 100 amostras. Com a taxa efetiva observada em bancada
 * (~25 amostras/s), isso representa aproximadamente 4 s dentro da janela
 * temporal de 5 s do G1.
 *
 * IMPORTANTE
 * ----------
 * - ainda nao existem thresholds de PASS/FAIL;
 * - ACF alta tambem pode ocorrer com movimento periodico;
 * - morfologia sera responsabilidade do G3;
 * - coerencia RED/IR sera avaliada no G4.
 */

#ifndef ENABLE_GATE2_DIAGNOSTIC
#define ENABLE_GATE2_DIAGNOSTIC 0
#endif

#if ENABLE_GATE2_DIAGNOSTIC

static const uint8_t G2_MIN_LAG = 6;
static const uint8_t G2_MAX_LAG = 38;
static const uint16_t G2_EFFECTIVE_FS_HZ = 25;

struct Gate2Metrics
{
  uint16_t samples;
  uint32_t mean;
  uint32_t mad;
  uint32_t acdcPermille;
  uint16_t crossings;
  uint8_t bestLag;
  int16_t bestAcfPermille;
  uint16_t periodCpm;
};

uint32_t gate2AbsDiff(uint32_t a, uint32_t b)
{
  return (a >= b) ? (a - b) : (b - a);
}

uint32_t gate2GetChannelSample(bool redChannel, uint16_t index)
{
  return redChannel ? packed18GetRed(index) : packed18GetIr(index);
}

Gate2Metrics gate2AnalyzeChannel(bool redChannel)
{
  Gate2Metrics m;
  m.samples = packed18GetCount();
  m.mean = 0;
  m.mad = 0;
  m.acdcPermille = 0;
  m.crossings = 0;
  m.bestLag = 0;
  m.bestAcfPermille = -1000;
  m.periodCpm = 0;

  if (m.samples < 2)
  {
    return m;
  }

  uint32_t sum = 0;
  for (uint16_t i = 0; i < m.samples; i++)
  {
    sum += gate2GetChannelSample(redChannel, i);
  }
  m.mean = sum / m.samples;

  uint32_t madSum = 0;
  bool previousAbove = gate2GetChannelSample(redChannel, 0) >= m.mean;

  for (uint16_t i = 0; i < m.samples; i++)
  {
    const uint32_t x = gate2GetChannelSample(redChannel, i);
    madSum += gate2AbsDiff(x, m.mean);

    if (i > 0)
    {
      const bool currentAbove = x >= m.mean;
      if (currentAbove != previousAbove)
      {
        m.crossings++;
      }
      previousAbove = currentAbove;
    }
  }

  m.mad = madSum / m.samples;

  if (m.mean > 0)
  {
    m.acdcPermille =
      (uint32_t)(((uint64_t)m.mad * 1000ULL) / (uint64_t)m.mean);
  }

  const uint8_t maxLag =
    (m.samples > G2_MAX_LAG) ? G2_MAX_LAG : (uint8_t)(m.samples - 1);

  for (uint8_t lag = G2_MIN_LAG; lag <= maxLag; lag++)
  {
    int64_t numerator = 0;
    uint64_t energyA = 0;
    uint64_t energyB = 0;

    const uint16_t pairs = m.samples - lag;

    for (uint16_t i = 0; i < pairs; i++)
    {
      const int32_t a =
        (int32_t)gate2GetChannelSample(redChannel, i) - (int32_t)m.mean;
      const int32_t b =
        (int32_t)gate2GetChannelSample(redChannel, i + lag) - (int32_t)m.mean;

      numerator += (int64_t)a * (int64_t)b;
      energyA += (uint64_t)((int64_t)a * (int64_t)a);
      energyB += (uint64_t)((int64_t)b * (int64_t)b);
    }

    /*
     * Para evitar sqrt e float no AVR, normalizamos pela media das energias.
     * Pelo limite 2ab <= a^2+b^2, o resultado fica aproximadamente em
     * [-1000, +1000]. E um coeficiente ACF proxy, nao Pearson.
     */
    const uint64_t denominator = (energyA + energyB) / 2ULL;

    if (denominator == 0)
    {
      continue;
    }

    int64_t scaled = (numerator * 1000LL) / (int64_t)denominator;

    if (scaled > 1000) scaled = 1000;
    if (scaled < -1000) scaled = -1000;

    const int16_t acfPermille = (int16_t)scaled;

    if (acfPermille > m.bestAcfPermille)
    {
      m.bestAcfPermille = acfPermille;
      m.bestLag = lag;
    }
  }

  if (m.bestLag > 0)
  {
    m.periodCpm =
      (uint16_t)(((uint32_t)60U * G2_EFFECTIVE_FS_HZ) / m.bestLag);
  }

  return m;
}

void gate2PrintChannel(const __FlashStringHelper *name, const Gate2Metrics &m)
{
  Serial.print(name);
  Serial.print(F("[n="));
  Serial.print(m.samples);

  Serial.print(F(" mean="));
  Serial.print(m.mean);

  Serial.print(F(" MAD="));
  Serial.print(m.mad);

  Serial.print(F(" AC_DC_permille="));
  Serial.print(m.acdcPermille);

  Serial.print(F(" crossings="));
  Serial.print(m.crossings);

  Serial.print(F(" bestLag="));
  Serial.print(m.bestLag);

  Serial.print(F(" ACF_permille="));
  Serial.print(m.bestAcfPermille);

  Serial.print(F(" period_cpm="));
  Serial.print(m.periodCpm);

  Serial.print(F("]"));
}

void gate2AnalyzeAndPrint()
{
  const Gate2Metrics red = gate2AnalyzeChannel(true);
  const Gate2Metrics ir = gate2AnalyzeChannel(false);

  Serial.print(F("G2_DIAGNOSTIC "));
  gate2PrintChannel(F("RED"), red);
  Serial.print(F(" "));
  gate2PrintChannel(F("IR"), ir);
  Serial.println();
}

#endif // ENABLE_GATE2_DIAGNOSTIC
