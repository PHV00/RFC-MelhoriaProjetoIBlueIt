/*
 * I BLUE IT - SQI FLOW V5 BASELINE - ARDUINO UNO + MAX30102
 * ===========================================================
 *
 * PAPEL DESTE ARQUIVO
 * -------------------
 * Este e o ORQUESTRADOR central da pipeline de qualidade PPG.
 * Ele deve permanecer simples: inicializa o MAX30102 pela biblioteca SparkFun,
 * recebe RED/IR da FIFO, alimenta os Gates na ordem e decide se a janela pode
 * seguir para a proxima etapa.
 *
 * ESTADO DESTA BRANCH
 * --------------------
 * Gate 01 ja foi validado em bancada.
 * Gate 02 esta implementado em modo diagnostico e agora e caracterizado antes
 * de receber thresholds de PASS/FAIL.
 * Gate 03 nao sera incorporado automaticamente: seu ganho incremental sera
 * avaliado posteriormente. Gate 04, SpO2 e integracao com I Blue It permanecem
 * como etapas seguintes.
 *
 * FUNDAMENTACAO DA ARQUITETURA
 * ---------------------------
 * [BASE-01]
 * Charlton et al. - The 2023 wearable photoplethysmography roadmap.
 * DOI: 10.1088/1361-6579/acead2
 * Uso no projeto: qualidade do PPG deve ser avaliada antes de interpretar
 * variaveis fisiologicas derivadas.
 *
 * [BASE-02]
 * Desquins et al. - A Survey of Photoplethysmography and Imaging
 * Photoplethysmography Quality Assessment Methods.
 * DOI: 10.3390/app12199582
 * Uso no projeto: fundamenta a estrategia rule-based, multimetrica e a
 * combinacao hierarquica de criterios de qualidade.
 *
 * [SPO2-01]
 * Berwal et al. - SpO2 Measurement: Non-Idealities and Ways to Improve
 * Estimation Accuracy in Wearable Pulse Oximeters.
 * DOI: 10.1109/JSEN.2022.3170069
 * Uso no projeto: a estimativa de SpO2 deve receber apenas uma janela cuja
 * qualidade tenha sido aprovada.
 *
 * IMPORTANTE
 * ----------
 * A sequencia de quatro Gates e uma arquitetura DE PROJETO apoiada pela
 * literatura acima. Ela nao e uma copia literal de um unico artigo.
 * Thresholds do MAX30102 permanecem configuraveis e devem ser caracterizados
 * experimentalmente no nosso hardware.
 *
 * FLUXO ALVO
 * ----------
 *
 *   MAX30102 / SparkFun
 *          |
 *          v
 *      RAW RED + IR
 *          |
 *          v
 *   +----------------+
 *   | GATE 01        |  <-- IMPLEMENTADO AGORA
 *   | Integridade    |
 *   +-------+--------+
 *       FAIL|PASS
 *           |  \
 *     INVALID   v
 *          +----------------+
 *          | GATE 02        |  <-- proxima etapa
 *          | Pulsatilidade  |
 *          +----------------+
 *                  |
 *                  v
 *          +----------------+
 *          | GATE 03        |
 *          | Morfologia     |
 *          +----------------+
 *                  |
 *                  v
 *          +----------------+
 *          | GATE 04        |
 *          | RED <-> IR     |
 *          +----------------+
 *                  |
 *                  v
 *              PPG VALID
 *                  |
 *                  v
 *             SpO2 / FC
 *
 * VALIDACAO ATUAL
 * ----------------
 * O firmware imprime um relatorio do Gate 01 a cada janela de 5 s.
 * Nao ha protocolo do I Blue It nesta etapa: a serial e somente de bancada.
 */

#include <Wire.h>
#include "MAX30105.h"

// Forward declarations needed by the Arduino 1.8.x .ino preprocessor when
// it auto-generates function prototypes across multiple .ino tabs.
struct Packed18;
struct Gate2Metrics;

/*
 * MODOS DE BANCADA
 * ----------------
 * O G1 ja foi validado. Nesta branch o G2 roda somente em modo DIAGNOSTICO:
 * calcula metricas, mas ainda NAO decide PASS/FAIL.
 *
 * O buffer Packed18 e necessario ao G2. O relatorio de round-trip pode ficar
 * desligado para reduzir ruido na serial; o armazenamento continua ativo.
 */
#define ENABLE_PACKED18_STORAGE_TEST 0
#define ENABLE_GATE2_DIAGNOSTIC 1

#if ENABLE_PACKED18_STORAGE_TEST || ENABLE_GATE2_DIAGNOSTIC
  #define ENABLE_PACKED18_BUFFER 1
#else
  #define ENABLE_PACKED18_BUFFER 0
#endif

MAX30105 particleSensor;

// G1 e G2 trabalham sobre a mesma janela temporal de 5 s.
// Com sampleAverage=4 e sampleRate=100, observamos ~25 amostras/s;
// por isso o Packed18 reserva 125 amostras para o G2 Vadrevu.
const unsigned long SQI_WINDOW_MS = 5000UL;

unsigned long sqiWindowStartedAt = 0;

/*
 * Priming somente de STARTUP.
 *
 * O primeiro ensaio do G2 mostrou zeros dentro da primeira janela, produzidos
 * na inicializacao/FIFO. Esses zeros nao representam uma decisao fisiologica.
 * Para nao inventar um "warm-up de X segundos", descartamos apenas amostras
 * iniciais enquanto qualquer canal ainda vier exatamente zero. Assim que RED
 * e IR entregam uma primeira amostra nao-zero, iniciamos uma janela nova de
 * 5 s. Depois disso, zeros reais permanecem visiveis ao G1 normalmente.
 */
bool sqiAcquisitionPrimed = false;

void setup()
{
  Serial.begin(115200);
  delay(500);

  Wire.begin();

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD))
  {
    Serial.println(F("ERRO: MAX30102 nao encontrado."));
    Serial.println(F("Verifique 3.3V, GND, SDA e SCL."));
    while (1) {}
  }

  /*
   * Configuracao conservadora ja usada no bring-up do MAX30102 no Uno.
   * sampleAverage=4 e mantido por enquanto para preservar a configuracao
   * testada em bancada. A janela e controlada por TEMPO (millis), nao por uma
   * contagem fixa de amostras, evitando assumir uma taxa efetiva incorreta.
   */
  byte ledBrightness = 0x1F;
  byte sampleAverage = 4;
  byte ledMode = 2;       // RED + IR
  int sampleRate = 100;
  int pulseWidth = 411;   // 18 bits
  int adcRange = 4096;

  particleSensor.setup(
    ledBrightness,
    sampleAverage,
    ledMode,
    sampleRate,
    pulseWidth,
    adcRange
  );

  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);
  particleSensor.setPulseAmplitudeGreen(0);

  gate1Reset();
#if ENABLE_PACKED18_BUFFER
  packed18Reset();
#endif
  sqiWindowStartedAt = millis();
  sqiAcquisitionPrimed = false;

  Serial.println(F("=== SQI FLOW V5 / G1 + G2 DIAGNOSTIC ==="));
  Serial.println(F("MAX30102 OK. G1=5s; G2 Vadrevu=125 amostras (~5s)."));
  Serial.println(F("G2 trace R1..R6; ainda sem decisao na pipeline."));
}

void loop()
{
  // A biblioteca SparkFun cuida da comunicacao/FIFO do MAX30102.
  particleSensor.check();

  while (particleSensor.available())
  {
    const uint32_t red = particleSensor.getFIFORed();
    const uint32_t ir  = particleSensor.getFIFOIR();

    if (!sqiAcquisitionPrimed)
    {
      if (red == 0UL || ir == 0UL)
      {
        particleSensor.nextSample();
        continue;
      }

      // Primeira amostra fisicamente adquirida nos dois canais: comeca uma
      // janela limpa. Isto ocorre apenas uma vez apos o reset.
      sqiAcquisitionPrimed = true;
      gate1Reset();
#if ENABLE_PACKED18_BUFFER
      packed18Reset();
#endif
      sqiWindowStartedAt = millis();
      Serial.println(F("ACQ_PRIMED: primeira janela inicia sem zeros de startup."));
    }

    // O Gate 01 trabalha diretamente com RAW: nada e filtrado antes dele.
    gate1AddSample(red, ir);

#if ENABLE_PACKED18_BUFFER
    // Preserva as primeiras 125 amostras (~5 s) em 3 bytes/canal.
    // O G2 reutiliza esse bloco sem criar uint32_t[125].
    packed18StoreSample(red, ir);
#endif

    particleSensor.nextSample();
  }

  if ((millis() - sqiWindowStartedAt) >= SQI_WINDOW_MS)
  {
    const bool gate1Passed = gate1Evaluate();

    gate1PrintReport();

#if ENABLE_PACKED18_STORAGE_TEST
    packed18PrintReport();
#endif

    if (gate1Passed)
    {
#if ENABLE_GATE2_DIAGNOSTIC
      gate2AnalyzeAndPrint();
      Serial.println(F("PIPELINE: G1 PASS -> G2 DIAGNOSTIC (sem decisao ainda)"));
#else
      Serial.println(F("PIPELINE: G1 PASS -> G2 desabilitado"));
#endif
    }
    else
    {
      Serial.println(F("PIPELINE: INVALID -> janela rejeitada no G1; G2 nao executado"));
    }

    Serial.println(F("--------------------------------------------------"));

    gate1Reset();
#if ENABLE_PACKED18_BUFFER
    packed18Reset();
#endif
    sqiWindowStartedAt = millis();
  }
}
