/*
 * GATE 02 - QUALIDADE TEMPORAL / FOPC-DS (REDDY 2020)
 * =====================================================
 *
 * STATUS
 * ------
 * IMPLEMENTACAO DIAGNOSTICA PARA VALIDACAO MATEMATICA E DE BANCADA.
 * AINDA NAO EXISTE PASS/FAIL NESTE GATE.
 *
 * FUNDAMENTACAO PRIMARIA
 * ----------------------
 * Gangireddy Narendra Kumar Reddy,
 * M. Sabarimalai Manikandan,
 * N. V. L. Narasimha Murty.
 *
 * "On-Device Integrated PPG Quality Assessment and Sensor
 *  Disconnection/Saturation Detection System for IoT Health Monitoring"
 * DOI: 10.1109/TIM.2020.2971132
 *
 * O artigo propoe, apos as regras de nearly-zero e saturation:
 *
 *   d[n] = s[n] - s[n-1]
 *
 *   y[n] = d[n] / max(d[n])
 *
 *   z[n] = y[n] + a*w[n]
 *
 * e extrai o First-Order Predictor Coefficient (FOPC) de z[n].
 *
 * O estudo utiliza a=0.10 como nivel de ruido selecionado e obtem thresholds
 * empiricos para NF/MA/PF. NESTA BRANCH esses thresholds NAO sao aplicados:
 * primeiro validamos a matematica e caracterizamos o MAX30102.
 *
 * ADAPTACAO PARA ATmega328P
 * -------------------------
 * Para P=1, a equacao de Yule-Walker reduz-se a:
 *
 *            R1
 *   alpha = ----
 *            R0
 *
 * onde:
 *
 *   R0 = sum(z[n]^2)
 *   R1 = sum(z[n] * z[n-1])
 *
 * Para evitar guardar toda a janela, usamos a invariancia de alpha a um
 * fator de escala global:
 *
 *   q[n] = M*z[n] = d[n] + a*M*w[n]
 *   alpha(q) = alpha(z)
 *
 * com M=max(d[n]).
 *
 * Expandindo R0 e R1, acumulamos estatisticas suficientes em streaming.
 * Essa reducao e uma DERIVACAO DE ENGENHARIA DO PROJETO e deve ser comparada
 * com uma implementacao de referencia antes de ser congelada.
 *
 * RUIDO DE VALIDACAO
 * ------------------
 * O artigo descreve adicao de ruido aleatorio, mas o texto primario que
 * possuimos nao especifica de forma suficiente o gerador/distribuicao.
 *
 * Para tornar Arduino e referencia PC exatamente reproduziveis, esta fase usa
 * um xorshift32 deterministico convertido para Q15 em [-1,1).
 * Isto e uma escolha de VALIDACAO DO PORT, nao uma afirmacao sobre o gerador
 * original de Reddy. A amplitude relativa a=0.10 e preservada.
 */

#include <math.h>

#ifndef ENABLE_GATE2_REDDY_DIAGNOSTIC
#define ENABLE_GATE2_REDDY_DIAGNOSTIC 0
#endif

#ifndef ENABLE_GATE2_REDDY_SELF_TEST
#define ENABLE_GATE2_REDDY_SELF_TEST 0
#endif

#if ENABLE_GATE2_REDDY_DIAGNOSTIC || ENABLE_GATE2_REDDY_SELF_TEST

static const float G2_REDDY_NOISE_LEVEL = 0.10f;
static const float G2_Q15_SCALE = 32768.0f;
static const uint32_t G2_NOISE_SEED = 0x12345678UL;

struct Gate2FopcState
{
  bool hasPreviousRaw;
  uint32_t previousRaw;

  bool hasPreviousDiff;
  int32_t previousDiff;
  int16_t previousNoiseQ15;

  uint32_t noiseState;

  uint16_t rawSamples;
  uint16_t diffSamples;

  // M = max(d[n]) conforme o pseudocodigo do artigo.
  int32_t maxDiff;

  // Estatisticas suficientes para R0/R1 apos adicao de ruido.
  uint64_t sumD2;
  int64_t sumDDPrev;

  int64_t sumDW;
  int64_t sumCrossDW;

  uint64_t sumW2;
  int64_t sumWWPrev;
};

struct Gate2FopcMetrics
{
  bool valid;
  uint16_t rawSamples;
  uint16_t diffSamples;
  int32_t maxDiff;

  float alphaNoNoise;
  float alphaNoise;
};

static Gate2FopcState g2Red;
static Gate2FopcState g2Ir;

uint32_t gate2XorShift32(uint32_t x)
{
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return x;
}

int16_t gate2NextNoiseQ15(uint32_t &state)
{
  state = gate2XorShift32(state);

  // Os 16 bits altos sao reinterpretados como signed Q15.
  return (int16_t)(state >> 16);
}

void gate2ResetChannel(Gate2FopcState &s)
{
  s.hasPreviousRaw = false;
  s.previousRaw = 0;

  s.hasPreviousDiff = false;
  s.previousDiff = 0;
  s.previousNoiseQ15 = 0;

  s.noiseState = G2_NOISE_SEED;

  s.rawSamples = 0;
  s.diffSamples = 0;

  s.maxDiff = 0;

  s.sumD2 = 0;
  s.sumDDPrev = 0;

  s.sumDW = 0;
  s.sumCrossDW = 0;

  s.sumW2 = 0;
  s.sumWWPrev = 0;
}

void gate2Reset()
{
  gate2ResetChannel(g2Red);
  gate2ResetChannel(g2Ir);
}

void gate2AddChannelSample(Gate2FopcState &s, uint32_t raw)
{
  if (s.rawSamples < 65535U)
  {
    s.rawSamples++;
  }

  if (!s.hasPreviousRaw)
  {
    s.previousRaw = raw;
    s.hasPreviousRaw = true;
    return;
  }

  const int32_t d =
    (int32_t)raw - (int32_t)s.previousRaw;

  s.previousRaw = raw;

  if (d > s.maxDiff)
  {
    s.maxDiff = d;
  }

  const int16_t wQ15 =
    gate2NextNoiseQ15(s.noiseState);

  const int64_t d64 = (int64_t)d;
  const int64_t w64 = (int64_t)wQ15;

  s.sumD2 += (uint64_t)(d64 * d64);
  s.sumDW += d64 * w64;
  s.sumW2 += (uint64_t)(w64 * w64);

  if (s.hasPreviousDiff)
  {
    const int64_t previousD64 =
      (int64_t)s.previousDiff;

    const int64_t previousW64 =
      (int64_t)s.previousNoiseQ15;

    s.sumDDPrev += d64 * previousD64;

    s.sumCrossDW +=
      (d64 * previousW64)
      + (w64 * previousD64);

    s.sumWWPrev +=
      w64 * previousW64;
  }

  s.previousDiff = d;
  s.previousNoiseQ15 = wQ15;
  s.hasPreviousDiff = true;

  if (s.diffSamples < 65535U)
  {
    s.diffSamples++;
  }
}

void gate2AddSample(uint32_t red, uint32_t ir)
{
#if ENABLE_GATE2_REDDY_DIAGNOSTIC
  gate2AddChannelSample(g2Red, red);
  gate2AddChannelSample(g2Ir, ir);
#else
  (void)red;
  (void)ir;
#endif
}

Gate2FopcMetrics gate2EvaluateChannel(
  const Gate2FopcState &s
)
{
  Gate2FopcMetrics m;

  m.valid = false;
  m.rawSamples = s.rawSamples;
  m.diffSamples = s.diffSamples;
  m.maxDiff = s.maxDiff;
  m.alphaNoNoise = 0.0f;
  m.alphaNoise = 0.0f;

  if (s.diffSamples < 2)
  {
    return m;
  }

  if (s.maxDiff <= 0)
  {
    return m;
  }

  if (s.sumD2 == 0)
  {
    return m;
  }

  /*
   * Referencia sem ruido:
   *
   * alpha = R1 / R0
   *
   * usando:
   * R0 = sum(d[n]^2)
   * R1 = sum(d[n]d[n-1])
   *
   * A normalizacao d/M cancela na razao.
   */
  m.alphaNoNoise =
    (float)s.sumDDPrev / (float)s.sumD2;

  /*
   * Com ruido:
   *
   * q[n] = d[n] + b*w[n]
   *
   * b = a*M
   *
   * O w acumulado esta em Q15, portanto:
   *
   * q[n] = d[n] + (b/Q)*wQ15[n]
   */
  const float b =
    G2_REDDY_NOISE_LEVEL * (float)s.maxDiff;

  const float bOverQ =
    b / G2_Q15_SCALE;

  const float bOverQ2 =
    bOverQ * bOverQ;

  const float r0 =
      (float)s.sumD2
    + (2.0f * bOverQ * (float)s.sumDW)
    + (bOverQ2 * (float)s.sumW2);

  const float r1 =
      (float)s.sumDDPrev
    + (bOverQ * (float)s.sumCrossDW)
    + (bOverQ2 * (float)s.sumWWPrev);

  if (fabsf(r0) < 1.0e-12f)
  {
    return m;
  }

  m.alphaNoise = r1 / r0;
  m.valid = true;

  return m;
}

#if defined(__AVR__)
extern int __heap_start;
extern void *__brkval;

int gate2FreeRam()
{
  int stackTop;
  int heapTop = (__brkval == 0)
    ? (int)&__heap_start
    : (int)__brkval;

  return (int)&stackTop - heapTop;
}
#else
int gate2FreeRam()
{
  return -1;
}
#endif

void gate2PrintMetrics(
  const __FlashStringHelper *name,
  const Gate2FopcMetrics &m
)
{
  Serial.print(name);
  Serial.print(F("[raw="));
  Serial.print(m.rawSamples);

  Serial.print(F(" diff="));
  Serial.print(m.diffSamples);

  Serial.print(F(" M="));
  Serial.print(m.maxDiff);

  Serial.print(F(" alpha0="));
  Serial.print(m.alphaNoNoise, 6);

  Serial.print(F(" alphaNoise="));
  if (m.valid)
  {
    Serial.print(m.alphaNoise, 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("]"));
}

void gate2PrintReport()
{
#if ENABLE_GATE2_REDDY_DIAGNOSTIC
  const unsigned long startedUs = micros();

  const Gate2FopcMetrics red =
    gate2EvaluateChannel(g2Red);

  const Gate2FopcMetrics ir =
    gate2EvaluateChannel(g2Ir);

  const unsigned long evalUs =
    micros() - startedUs;

  Serial.print(F("G2_REDDY_DIAGNOSTIC "));

  gate2PrintMetrics(F("RED"), red);
  Serial.print(F(" "));
  gate2PrintMetrics(F("IR"), ir);

  Serial.print(F(" noise=0.10"));
  Serial.print(F(" seed=0x12345678"));

  Serial.print(F(" eval_us="));
  Serial.print(evalUs);

  Serial.print(F(" freeRAM="));
  Serial.print(gate2FreeRam());

  Serial.println();
#endif
}

/*
 * SELF-TEST MATEMATICO
 * --------------------
 * Vetor senoidal discreto fixo, usado tambem pela referencia Python.
 *
 * Esperado para esta implementacao/seed:
 *   M ~= 1545
 *   alpha0 ~= 0.902312
 *   alphaNoise ~= 0.903414
 *
 * O teste NAO valida fisiologia. Ele valida apenas:
 *   vetor -> d[n] -> estatisticas -> alpha.
 */
void gate2RunSelfTest()
{
#if ENABLE_GATE2_REDDY_SELF_TEST
  static const uint32_t testVector[] =
  {
    100000, 101545, 102938, 104045, 104755,
    105000, 104755, 104045, 102938, 101545,
    100000,  98455,  97062,  95955,  95245,
     95000,  95245,  95955,  97062,  98455,
    100000, 101545, 102938, 104045, 104755,
    105000, 104755, 104045, 102938, 101545,
    100000,  98455,  97062,  95955,  95245,
     95000,  95245,  95955,  97062,  98455
  };

  Gate2FopcState testState;
  gate2ResetChannel(testState);

  const uint16_t n =
    sizeof(testVector) / sizeof(testVector[0]);

  for (uint16_t i = 0; i < n; i++)
  {
    gate2AddChannelSample(testState, testVector[i]);
  }

  const Gate2FopcMetrics m =
    gate2EvaluateChannel(testState);

  const bool pass =
       m.valid
    && (m.maxDiff == 1545)
    && (fabsf(m.alphaNoNoise - 0.902312f) < 0.000010f)
    && (fabsf(m.alphaNoise - 0.903414f) < 0.000010f);

  Serial.print(F("G2_REDDY_SELFTEST="));
  Serial.print(pass ? F("PASS") : F("FAIL"));

  Serial.print(F(" M="));
  Serial.print(m.maxDiff);

  Serial.print(F(" alpha0="));
  Serial.print(m.alphaNoNoise, 6);

  Serial.print(F(" alphaNoise="));
  Serial.print(m.alphaNoise, 6);

  Serial.println();
#endif
}

#endif // ENABLE_GATE2_REDDY_DIAGNOSTIC || ENABLE_GATE2_REDDY_SELF_TEST
