/*
 * GATE 02 - VADREVU 2019 / PORT ATmega328P - MODO DIAGNOSTICO
 * ============================================================
 *
 * FONTE PRIMARIA
 * --------------
 * Vadrevu, S.; Manikandan, M. S. (2019)
 * "Real-Time PPG Signal Quality Assessment System for Improving
 * Battery Life and False Alarms"
 * DOI: 10.1109/TCSII.2019.2891636
 *
 * OBJETIVO DESTA BRANCH
 * ---------------------
 * Implementar separadamente os modulos/regras descritos pelo artigo para
 * podermos validar cada um no MAX30102 antes de congelar o G2 final.
 *
 * Fluxo de referencia:
 *
 * RAW18 -> HP Butterworth 3a ordem / 0.5 Hz
 *       -> R1: Maximum Absolute Amplitude
 *       -> R2: Local Amplitude Maxima
 *       -> R3: Number of Threshold Crossings (NTC)
 *       -> Hamming + ACF
 *       -> R4: FZCP
 *       -> R5: Rmax + Kmax
 *       -> R6: NTC da primeira diferenca dPPG
 *
 * O artigo trabalha com janelas de 5 s, Fs=125 Hz e ADC de 10 bits.
 * Nosso port preserva os TEMPOS, mas usa a taxa efetiva observada no
 * MAX30102 configurado com sampleAverage=4: aproximadamente 25 amostras/s.
 * Assim, a janela do G2 tem 125 amostras (~5 s).
 *
 * ADAPTACOES EXPLICITAS DO PORT
 * -----------------------------
 * 1) lambda_MAA do artigo (=5 no sistema de 10 bits) NAO e copiado como
 *    threshold final. Nesta branch usamos somente um ponto de partida
 *    proporcional ao range digital:
 *
 *      lambda_MAA ~= 5/1023 * 262143 ~= 1281 counts
 *
 *    Esse valor e PROVISORIO e precisa ser calibrado no MAX30102.
 *
 * 2) Quadros locais de 100 ms do R2 sao representados por bins temporais
 *    de 100 ms. A 25 Hz cada bin contem 2 ou 3 amostras, alternadamente.
 *
 * 3) O artigo descreve de forma curta duas condicoes do R2. Para bancada,
 *    registramos as metricas completas e usamos uma interpretacao operacional
 *    marcada como PROVISORIA: <=2 frames acima de lambda_MAA, mais de 4
 *    frames consecutivos abaixo, ou uma das paridades de frames inteira abaixo.
 *
 * 4) O artigo define lambda_ntc2 do R6 como:
 *      numero estimado de pulsos * numero possivel de zero-crossings/pulso.
 *    Como o numero exato nao e explicitado na Regra 6, usamos 4 apenas como
 *    TRACE PROVISORIO, apoiado na discussao anterior do proprio artigo sobre
 *    ate quatro crossings por ciclo. Nao e threshold final.
 *
 * 5) Para caber no ATmega328P, o sinal high-pass e armazenado em int16_t
 *    apos divisao por 8. R1/R2/R3 sao extraidos ANTES dessa quantizacao.
 *    Hamming + ACF usam a versao quantizada. A escala nao altera idealmente
 *    a ACF normalizada, mas a quantizacao deve ser validada em bancada.
 *
 * 6) O artigo tem uma desigualdade tipograficamente inconsistente na Eq. 9.
 *    O texto define PPI de 0.2 a 2.0 s; portanto R5 usa esse intervalo.
 *
 * 7) "Maximum peak (Rmax) and its lag (Kmax)" e implementado como o maior
 *    MAXIMO LOCAL da ACF apos o FZCP, dentro de 0.2..2.0 s. Esta e uma
 *    interpretacao operacional necessaria para nao confundir a cauda
 *    descendente de R[0]=1 com um pico periodico. O artigo nomeia Rmax/Kmax,
 *    mas nao fornece pseudocodigo para a busca do pico.
 *
 * IMPORTANTE
 * ----------
 * Esta branch continua DIAGNOSTICA. As regras sao calculadas e impressas,
 * mas o orquestrador ainda NAO rejeita a janela pelo G2.
 */

#include <math.h>

#if defined(__AVR__)
#include <avr/pgmspace.h>
#endif

#ifndef ENABLE_GATE2_DIAGNOSTIC
#define ENABLE_GATE2_DIAGNOSTIC 0
#endif

#if ENABLE_GATE2_DIAGNOSTIC

// ---------------------------------------------------------------------------
// Configuracao temporal do port
// ---------------------------------------------------------------------------

static const uint16_t G2_EFFECTIVE_FS_HZ = 25U;
static const uint16_t G2_WINDOW_SECONDS = 5U;
static const uint16_t G2_WORK_SAMPLES =
  G2_EFFECTIVE_FS_HZ * G2_WINDOW_SECONDS; // 125

static const uint16_t G2_LOCAL_FRAME_MS = 100U;
static const uint8_t G2_EXPECTED_LOCAL_FRAMES =
  (G2_WINDOW_SECONDS * 1000U) / G2_LOCAL_FRAME_MS; // 50

static const uint16_t G2_FZCP_MIN_MS = 50U;
static const uint16_t G2_FZCP_MAX_MS = 1000U;

static const uint16_t G2_PERIOD_MIN_MS = 200U;
static const uint16_t G2_PERIOD_MAX_MS = 2000U;

static const int16_t G2_MIN_RMAX_PERMILLE = 500; // Rmax >= 0.5

// R3: 5 s / 0.2 s * 4 crossings = 100.
static const uint16_t G2_NTC1_MAX = 100U;

// ---------------------------------------------------------------------------
// Threshold de amplitude: adaptacao PROVISORIA 10 bits -> 18 bits
// ---------------------------------------------------------------------------

static const uint32_t G2_ARTICLE_ADC_MAX = 1023UL;
static const uint32_t G2_MAX30102_ADC_MAX = 262143UL;
static const uint32_t G2_ARTICLE_LAMBDA_MAA = 5UL;

static const uint32_t G2_LAMBDA_MAA_COUNTS =
  (G2_ARTICLE_LAMBDA_MAA * G2_MAX30102_ADC_MAX
   + (G2_ARTICLE_ADC_MAX / 2UL))
  / G2_ARTICLE_ADC_MAX; // ~1281; PROVISORIO

// ---------------------------------------------------------------------------
// Adaptacao de memoria
// ---------------------------------------------------------------------------

// 18-bit / 8 cabe aproximadamente em int16_t.
// O valor final do filtro e saturado se necessario.
static const float G2_STORE_DIVISOR = 8.0f;

// Um unico workspace e reutilizado RED -> IR: 125 * 2 = 250 B.
static int16_t g2Work[G2_WORK_SAMPLES];

// Hamming N=125 em Q15. Por simetria guardamos somente indices 0..62.
// w[n] = 0.54 - 0.46*cos(2*pi*n/(N-1))
#if defined(__AVR__)
static const uint16_t g2HammingHalfQ15[63] PROGMEM = {
#else
static const uint16_t g2HammingHalfQ15[63] = {
#endif
  2621, 2641, 2699, 2795, 2930, 3103, 3313, 3560, 3843, 4162,
  4515, 4903, 5323, 5775, 6258, 6770, 7310, 7876, 8468, 9084,
  9721, 10379, 11056, 11750, 12459, 13182, 13916, 14660, 15412, 16169,
  16931, 17694, 18458, 19219, 19977, 20728, 21472, 22206, 22929, 23638,
  24332, 25009, 25667, 26305, 26920, 27512, 28079, 28619, 29131, 29613,
  30065, 30486, 30873, 31227, 31545, 31829, 32076, 32286, 32458, 32593,
  32690, 32748, 32767
};

struct Gate2Metrics
{
  uint16_t samples;
  bool completeWindow;
  uint32_t meanRaw;

  // Preprocess / amplitude.
  uint32_t xmax;

  // R2: Local Amplitude Maxima.
  uint8_t localFrames;
  uint8_t localFramesAbove;
  uint8_t maxConsecutiveBelow;
  uint8_t evenFrames;
  uint8_t oddFrames;
  uint8_t evenFramesBelow;
  uint8_t oddFramesBelow;
  bool alternateLowPattern;

  // R3.
  uint16_t ntc1;

  // R4/R5.
  bool hasFzcp;
  uint8_t fzcpLag;
  uint16_t fzcpMs;
  int16_t rmaxPermille;
  uint8_t kmaxLag;
  uint16_t kmaxMs;
  uint16_t periodCpm;

  // R6.
  uint16_t dntc2;
  uint8_t estimatedPulses;
  uint16_t lambdaNtc2;

  // Resultados por regra.
  bool rule1Fail;
  bool rule2FailProvisional;
  bool rule3Fail;
  bool rule4Fail;
  bool rule5Fail;
  bool rule6FailProvisional;

  // Sumarios somente para caracterizacao.
  bool confirmedCorePass;
  bool provisionalFullPass;
};

uint32_t gate2GetChannelSample(bool redChannel, uint16_t index)
{
  return redChannel ? packed18GetRed(index) : packed18GetIr(index);
}

uint16_t gate2ReadHammingQ15(uint16_t index)
{
  uint16_t mirrored = index;

  if (mirrored > (G2_WORK_SAMPLES - 1U) / 2U)
  {
    mirrored = (G2_WORK_SAMPLES - 1U) - mirrored;
  }

#if defined(__AVR__)
  return pgm_read_word(&g2HammingHalfQ15[mirrored]);
#else
  return g2HammingHalfQ15[mirrored];
#endif
}

int16_t gate2QuantizeFiltered(float value)
{
  long q = lroundf(value / G2_STORE_DIVISOR);

  if (q > 32767L) q = 32767L;
  if (q < -32767L) q = -32767L;

  return (int16_t)q;
}

uint32_t gate2MeanRaw(bool redChannel, uint16_t samples)
{
  uint32_t sum = 0;

  for (uint16_t i = 0; i < samples; i++)
  {
    sum += gate2GetChannelSample(redChannel, i);
  }

  return samples > 0 ? (sum / samples) : 0;
}

void gate2Rule02AcceptFrame(
  Gate2Metrics &m,
  uint8_t frameIndex,
  uint32_t localMaximum
)
{
  m.localFrames++;

  const bool below = localMaximum < G2_LAMBDA_MAA_COUNTS;

  if (!below)
  {
    m.localFramesAbove++;
    m.maxConsecutiveBelow = m.maxConsecutiveBelow; // explicito: sem alteracao
  }

  static uint8_t currentLowRun = 0;
  // O estado static nao pode vazar entre canais/janelas. Por isso esta funcao
  // NAO usa currentLowRun para a decisao final; a sequencia e atualizada no
  // preprocess, onde o estado e local. Mantemos aqui apenas contagens por paridade.

  if ((frameIndex & 1U) == 0U)
  {
    m.evenFrames++;
    if (below) m.evenFramesBelow++;
  }
  else
  {
    m.oddFrames++;
    if (below) m.oddFramesBelow++;
  }

  (void)currentLowRun;
}

/*
 * PREPROCESSAMENTO
 * ----------------
 * Butterworth high-pass de 3a ordem / 0.5 Hz como cascata:
 * - uma secao de 1a ordem;
 * - uma secao de 2a ordem com Q=1.
 *
 * Durante o mesmo passe extraimos Xmax, maximos locais de 100 ms, NTC1 e
 * armazenamos o sinal final em int16_t para ACF/dPPG.
 */
void gate2PreprocessAndExtractCheapFeatures(
  bool redChannel,
  Gate2Metrics &m
)
{
  const float fs = (float)G2_EFFECTIVE_FS_HZ;
  const float cutoff = 0.5f;

  const float k = tanf(PI * cutoff / fs);
  const float norm = 1.0f / (1.0f + k);

  const float hp1B0 = norm;
  const float hp1B1 = -norm;
  const float hp1A1 = (k - 1.0f) * norm;

  const float omega = 2.0f * PI * cutoff / fs;
  const float cosOmega = cosf(omega);
  const float sinOmega = sinf(omega);
  const float alpha = 0.5f * sinOmega;
  const float a0 = 1.0f + alpha;

  const float hp2B0 = ((1.0f + cosOmega) * 0.5f) / a0;
  const float hp2B1 = (-(1.0f + cosOmega)) / a0;
  const float hp2B2 = hp2B0;
  const float hp2A1 = (-2.0f * cosOmega) / a0;
  const float hp2A2 = (1.0f - alpha) / a0;

  float hp1PrevX =
    (float)gate2GetChannelSample(redChannel, 0) - (float)m.meanRaw;
  float hp1PrevY = 0.0f;

  float hp2X1 = 0.0f;
  float hp2X2 = 0.0f;
  float hp2Y1 = 0.0f;
  float hp2Y2 = 0.0f;

  int8_t ntcPreviousSide = 0;

  uint8_t activeFrame = 0;
  uint32_t activeFrameMax = 0;
  bool hasActiveFrame = false;
  uint8_t currentLowRun = 0;

  for (uint16_t i = 0; i < m.samples; i++)
  {
    const float rawCentered =
      (float)gate2GetChannelSample(redChannel, i) - (float)m.meanRaw;

    const float hp1 =
      hp1B0 * rawCentered
      + hp1B1 * hp1PrevX
      - hp1A1 * hp1PrevY;

    hp1PrevX = rawCentered;
    hp1PrevY = hp1;

    const float hp2 =
      hp2B0 * hp1
      + hp2B1 * hp2X1
      + hp2B2 * hp2X2
      - hp2A1 * hp2Y1
      - hp2A2 * hp2Y2;

    hp2X2 = hp2X1;
    hp2X1 = hp1;
    hp2Y2 = hp2Y1;
    hp2Y1 = hp2;

    const float absHp = fabsf(hp2);
    const uint32_t absCounts = (uint32_t)lroundf(absHp);

    if (absCounts > m.xmax)
    {
      m.xmax = absCounts;
    }

    // R2: bins temporais nao sobrepostos de 100 ms.
    uint8_t frameIndex = (uint8_t)(
      ((uint32_t)i * 1000UL)
      / ((uint32_t)G2_EFFECTIVE_FS_HZ * G2_LOCAL_FRAME_MS)
    );

    if (frameIndex >= G2_EXPECTED_LOCAL_FRAMES)
    {
      frameIndex = G2_EXPECTED_LOCAL_FRAMES - 1U;
    }

    if (!hasActiveFrame)
    {
      activeFrame = frameIndex;
      activeFrameMax = absCounts;
      hasActiveFrame = true;
    }
    else if (frameIndex != activeFrame)
    {
      gate2Rule02AcceptFrame(m, activeFrame, activeFrameMax);

      if (activeFrameMax < G2_LAMBDA_MAA_COUNTS)
      {
        currentLowRun++;
        if (currentLowRun > m.maxConsecutiveBelow)
        {
          m.maxConsecutiveBelow = currentLowRun;
        }
      }
      else
      {
        currentLowRun = 0;
      }

      activeFrame = frameIndex;
      activeFrameMax = absCounts;
    }
    else if (absCounts > activeFrameMax)
    {
      activeFrameMax = absCounts;
    }

    // R3: NTC em torno de lambda_MAA, conforme Eq. 5.
    const int8_t ntcSide =
      hp2 >= (float)G2_LAMBDA_MAA_COUNTS ? 1 : -1;

    if (i > 0 && ntcSide != ntcPreviousSide)
    {
      m.ntc1++;
    }
    ntcPreviousSide = ntcSide;

    g2Work[i] = gate2QuantizeFiltered(hp2);
  }

  if (hasActiveFrame)
  {
    gate2Rule02AcceptFrame(m, activeFrame, activeFrameMax);

    if (activeFrameMax < G2_LAMBDA_MAA_COUNTS)
    {
      currentLowRun++;
      if (currentLowRun > m.maxConsecutiveBelow)
      {
        m.maxConsecutiveBelow = currentLowRun;
      }
    }
  }

  // Interpretacao operacional do "alternate maxima < lambda_MAA".
  m.alternateLowPattern =
    (m.evenFrames > 0 && m.evenFramesBelow == m.evenFrames)
    || (m.oddFrames > 0 && m.oddFramesBelow == m.oddFrames);
}

// ---------------------------------------------------------------------------
// R1 - Maximum Absolute Amplitude
// ---------------------------------------------------------------------------

void gate2Rule01Evaluate(Gate2Metrics &m)
{
  m.rule1Fail = m.xmax < G2_LAMBDA_MAA_COUNTS;
}

// ---------------------------------------------------------------------------
// R2 - Local Amplitude Maxima (interpretacao PROVISORIA)
// ---------------------------------------------------------------------------

void gate2Rule02Evaluate(Gate2Metrics &m)
{
  m.rule2FailProvisional =
    (m.localFramesAbove <= 2U)
    || (m.maxConsecutiveBelow > 4U)
    || m.alternateLowPattern;
}

// ---------------------------------------------------------------------------
// R3 - Number of Threshold Crossings
// ---------------------------------------------------------------------------

void gate2Rule03Evaluate(Gate2Metrics &m)
{
  m.rule3Fail = m.ntc1 > G2_NTC1_MAX;
}

// ---------------------------------------------------------------------------
// R6 feature extraction (antes da Hamming; decisao aplicada depois de R5)
// ---------------------------------------------------------------------------

void gate2ExtractDppgNtc(Gate2Metrics &m)
{
  m.dntc2 = 0;

  if (m.samples < 3)
  {
    return;
  }

  int16_t previousDiff = g2Work[1] - g2Work[0];
  int8_t previousSide = previousDiff >= 1 ? 1 : -1;

  for (uint16_t i = 2; i < m.samples; i++)
  {
    const int16_t diff = g2Work[i] - g2Work[i - 1U];
    const int8_t side = diff >= 1 ? 1 : -1;

    if (side != previousSide)
    {
      m.dntc2++;
    }

    previousSide = side;
  }
}

// ---------------------------------------------------------------------------
// Hamming + ACF (Eq. 7)
// ---------------------------------------------------------------------------

void gate2ApplyHammingInPlace(uint16_t samples)
{
  for (uint16_t i = 0; i < samples; i++)
  {
    const int32_t weighted =
      ((int32_t)g2Work[i] * (int32_t)gate2ReadHammingQ15(i));

    g2Work[i] = (int16_t)(weighted >> 15);
  }
}

int16_t gate2AcfPermilleAtLag(
  uint16_t samples,
  uint8_t lag,
  uint64_t totalEnergy
)
{
  if (lag >= samples || totalEnergy == 0)
  {
    return 0;
  }

  int64_t numerator = 0;
  const uint16_t pairs = samples - lag;

  for (uint16_t i = 0; i < pairs; i++)
  {
    numerator +=
      (int32_t)g2Work[i] * (int32_t)g2Work[i + lag];
  }

  int64_t scaled = (numerator * 1000LL) / (int64_t)totalEnergy;

  if (scaled > 1000LL) scaled = 1000LL;
  if (scaled < -1000LL) scaled = -1000LL;

  return (int16_t)scaled;
}

void gate2ExtractAcfFeatures(Gate2Metrics &m)
{
  gate2ApplyHammingInPlace(m.samples);

  uint64_t totalEnergy = 0;

  for (uint16_t i = 0; i < m.samples; i++)
  {
    const int32_t x = g2Work[i];
    totalEnergy += (uint64_t)((int64_t)x * (int64_t)x);
  }

  if (totalEnergy == 0)
  {
    m.rmaxPermille = 0;
    return;
  }

  uint8_t minLag = (uint8_t)(
    ((uint32_t)G2_PERIOD_MIN_MS * G2_EFFECTIVE_FS_HZ + 999UL) / 1000UL
  );

  uint8_t maxLag = (uint8_t)(
    ((uint32_t)G2_PERIOD_MAX_MS * G2_EFFECTIVE_FS_HZ) / 1000UL
  );

  if (minLag < 1U) minLag = 1U;
  if (maxLag >= m.samples) maxLag = (uint8_t)(m.samples - 1U);

  /*
   * R[0] = 1 e a ACF normalmente sai desse pico trivial descendo.
   *
   * O erro da versao anterior era escolher simplesmente o maior R[k] na faixa
   * 0.2..2.0 s. Isso frequentemente selecionava o PRIMEIRO lag permitido
   * (k=5 => 200 ms => 300 cpm), mesmo sem existir um pico ali.
   *
   * Agora detectamos picos locais:
   *
   *   R[k-1] < R[k] >= R[k+1]
   *
   * e, como interpretacao operacional da sequencia R4 -> R5 de Vadrevu,
   * aceitamos candidatos somente APOS o primeiro zero-crossing da ACF.
   */
  int16_t rPrev2 = 1000; // R[0]
  int16_t rPrev1 =
    gate2AcfPermilleAtLag(m.samples, 1U, totalEnergy);

  if (rPrev2 > 0 && rPrev1 <= 0)
  {
    m.hasFzcp = true;
    m.fzcpLag = 1U;
    m.fzcpMs = (uint16_t)(1000UL / G2_EFFECTIVE_FS_HZ);
  }

  int16_t bestPeakR = -1000;
  uint8_t bestPeakLag = 0;

  uint8_t scanEnd = maxLag;
  if ((uint16_t)maxLag + 1U < m.samples)
  {
    scanEnd = maxLag + 1U; // look-ahead para testar pico em maxLag
  }

  for (uint8_t lag = 2U; lag <= scanEnd; lag++)
  {
    const int16_t r =
      gate2AcfPermilleAtLag(m.samples, lag, totalEnergy);

    if (!m.hasFzcp && rPrev1 > 0 && r <= 0)
    {
      m.hasFzcp = true;
      m.fzcpLag = lag;
      m.fzcpMs = (uint16_t)(
        ((uint32_t)lag * 1000UL) / G2_EFFECTIVE_FS_HZ
      );
    }

    const uint8_t candidateLag = lag - 1U;
    const bool isLocalPeak =
      rPrev1 > rPrev2
      && rPrev1 >= r;

    const bool inPeriodRange =
      candidateLag >= minLag
      && candidateLag <= maxLag;

    const bool afterFzcp =
      m.hasFzcp
      && candidateLag > m.fzcpLag;

    if (isLocalPeak && inPeriodRange && afterFzcp)
    {
      if (bestPeakLag == 0 || rPrev1 > bestPeakR)
      {
        bestPeakR = rPrev1;
        bestPeakLag = candidateLag;
      }
    }

    rPrev2 = rPrev1;
    rPrev1 = r;
  }

  // Sem maximo local periodico: R5 deve falhar em vez de inventar Kmax=5.
  m.rmaxPermille = bestPeakLag > 0 ? bestPeakR : -1000;
  m.kmaxLag = bestPeakLag;

  if (bestPeakLag > 0)
  {
    m.kmaxMs = (uint16_t)(
      ((uint32_t)bestPeakLag * 1000UL) / G2_EFFECTIVE_FS_HZ
    );

    m.periodCpm = (uint16_t)(
      ((uint32_t)60U * G2_EFFECTIVE_FS_HZ) / bestPeakLag
    );
  }
}

// ---------------------------------------------------------------------------
// R4 - FZCP
// ---------------------------------------------------------------------------

void gate2Rule04Evaluate(Gate2Metrics &m)
{
  m.rule4Fail =
    !m.hasFzcp
    || m.fzcpMs < G2_FZCP_MIN_MS
    || m.fzcpMs > G2_FZCP_MAX_MS;
}

// ---------------------------------------------------------------------------
// R5 - Rmax + Kmax
// ---------------------------------------------------------------------------

void gate2Rule05Evaluate(Gate2Metrics &m)
{
  m.rule5Fail =
    m.rmaxPermille < G2_MIN_RMAX_PERMILLE
    || m.kmaxLag == 0
    || m.kmaxMs < G2_PERIOD_MIN_MS
    || m.kmaxMs > G2_PERIOD_MAX_MS;
}

// ---------------------------------------------------------------------------
// R6 - NTC da primeira diferenca (threshold PROVISORIO)
// ---------------------------------------------------------------------------

void gate2Rule06Evaluate(Gate2Metrics &m)
{
  if (m.kmaxLag == 0)
  {
    m.estimatedPulses = 0;
    m.lambdaNtc2 = 0;
    m.rule6FailProvisional = true;
    return;
  }

  m.estimatedPulses =
    (uint8_t)(m.samples / m.kmaxLag);

  // PROVISORIO: 4 crossings/pulso.
  m.lambdaNtc2 =
    (uint16_t)m.estimatedPulses * 4U;

  // Eq. 11: Acceptable se Dntc2 > lambda_ntc2.
  m.rule6FailProvisional =
    m.dntc2 <= m.lambdaNtc2;
}

Gate2Metrics gate2AnalyzeChannel(bool redChannel)
{
  Gate2Metrics m = {};

  m.samples = packed18GetCount();
  if (m.samples > G2_WORK_SAMPLES)
  {
    m.samples = G2_WORK_SAMPLES;
  }

  m.completeWindow = m.samples == G2_WORK_SAMPLES;
  m.rmaxPermille = -1000;

  if (m.samples < 3)
  {
    m.rule1Fail = true;
    m.rule2FailProvisional = true;
    m.rule3Fail = true;
    m.rule4Fail = true;
    m.rule5Fail = true;
    m.rule6FailProvisional = true;
    return m;
  }

  m.meanRaw = gate2MeanRaw(redChannel, m.samples);

  gate2PreprocessAndExtractCheapFeatures(redChannel, m);

  // Cada modulo e avaliado separadamente para permitir estudo de ablacao.
  gate2Rule01Evaluate(m);
  gate2Rule02Evaluate(m);
  gate2Rule03Evaluate(m);

  // R6 usa o sinal sem Hamming. Extraimos agora, mas aplicamos sua decisao
  // logicamente somente depois de R4/R5.
  gate2ExtractDppgNtc(m);

  // R4/R5 dependem da ACF.
  gate2ExtractAcfFeatures(m);
  gate2Rule04Evaluate(m);
  gate2Rule05Evaluate(m);

  gate2Rule06Evaluate(m);

  // "Core" = regras com equacoes/limites diretamente utilizaveis do artigo.
  // lambda_MAA ainda e uma adaptacao de hardware e deve ser recalibrada.
  m.confirmedCorePass =
    m.completeWindow
    && !m.rule1Fail
    && !m.rule3Fail
    && !m.rule4Fail
    && !m.rule5Fail;

  // Full provisional inclui as interpretacoes ainda nao congeladas de R2/R6.
  m.provisionalFullPass =
    m.confirmedCorePass
    && !m.rule2FailProvisional
    && !m.rule6FailProvisional;

  return m;
}

void gate2PrintRule(bool fail, bool provisional)
{
  Serial.print(fail ? F("F") : F("P"));
  if (provisional)
  {
    Serial.print(F("?"));
  }
}

void gate2PrintChannel(
  const __FlashStringHelper *name,
  const Gate2Metrics &m
)
{
  Serial.print(name);
  Serial.print(F("[n="));
  Serial.print(m.samples);

  Serial.print(F(" full5s="));
  Serial.print(m.completeWindow ? 1 : 0);

  Serial.print(F(" meanRAW="));
  Serial.print(m.meanRaw);

  Serial.print(F(" lambdaMAA="));
  Serial.print(G2_LAMBDA_MAA_COUNTS);

  Serial.print(F(" Xmax="));
  Serial.print(m.xmax);

  Serial.print(F(" R1="));
  gate2PrintRule(m.rule1Fail, true); // threshold de hardware ainda provisório

  Serial.print(F(" frames="));
  Serial.print(m.localFrames);

  Serial.print(F(" above="));
  Serial.print(m.localFramesAbove);

  Serial.print(F(" lowRun="));
  Serial.print(m.maxConsecutiveBelow);

  Serial.print(F(" altLow="));
  Serial.print(m.alternateLowPattern ? 1 : 0);

  Serial.print(F(" R2="));
  gate2PrintRule(m.rule2FailProvisional, true);

  Serial.print(F(" NTC1="));
  Serial.print(m.ntc1);

  Serial.print(F(" R3="));
  gate2PrintRule(m.rule3Fail, false);

  Serial.print(F(" FZCPms="));
  if (m.hasFzcp) Serial.print(m.fzcpMs);
  else Serial.print(F("NONE"));

  Serial.print(F(" R4="));
  gate2PrintRule(m.rule4Fail, false);

  Serial.print(F(" Rmax_permille="));
  Serial.print(m.rmaxPermille);

  Serial.print(F(" Kmax="));
  Serial.print(m.kmaxLag);

  Serial.print(F(" Kmax_ms="));
  Serial.print(m.kmaxMs);

  Serial.print(F(" period_cpm="));
  Serial.print(m.periodCpm);

  Serial.print(F(" R5="));
  gate2PrintRule(m.rule5Fail, false);

  Serial.print(F(" DNTC2="));
  Serial.print(m.dntc2);

  Serial.print(F(" lambdaNTC2="));
  Serial.print(m.lambdaNtc2);

  Serial.print(F(" R6="));
  gate2PrintRule(m.rule6FailProvisional, true);

  Serial.print(F(" core="));
  Serial.print(m.confirmedCorePass ? F("PASS") : F("FAIL"));

  Serial.print(F(" fullProv="));
  Serial.print(m.provisionalFullPass ? F("PASS") : F("FAIL"));

  Serial.print(F("]"));
}

void gate2AnalyzeAndPrint()
{
  const unsigned long startedAt = millis();

  const Gate2Metrics red = gate2AnalyzeChannel(true);
  const Gate2Metrics ir = gate2AnalyzeChannel(false);

  const unsigned long elapsedMs = millis() - startedAt;

  Serial.print(F("G2_VADREVU_TRACE "));
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
