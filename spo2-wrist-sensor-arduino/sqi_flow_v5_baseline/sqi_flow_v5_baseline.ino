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
 * Nesta primeira versao SOMENTE O GATE 01 esta implementado e executado.
 * Gate 02, Gate 03, Gate 04, SpO2 e integracao com I Blue It ficam para etapas
 * posteriores, apos validarmos experimentalmente o Gate 01.
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

/*
 * VALIDACOES INDEPENDENTES
 * ------------------------
 * 0 = valida o Gate 01 isoladamente, sem reservar os 600 B do buffer 18-bit.
 * 1 = habilita tambem o teste de armazenamento lossless em 3 bytes/amostra.
 *
 * Para a revalidacao metodologica do G1, manter em 0. Depois, testar o
 * armazenamento separadamente mudando somente esta chave para 1.
 */
#define ENABLE_PACKED18_STORAGE_TEST 0

MAX30105 particleSensor;

// Baseline do projeto: janela temporal de 5 s.
// Reddy 2020 e Vadrevu 2019 utilizam janelas de 5 s em suas abordagens.
const unsigned long SQI_WINDOW_MS = 5000UL;

unsigned long sqiWindowStartedAt = 0;

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
#if ENABLE_PACKED18_STORAGE_TEST
  packed18Reset();
#endif
  sqiWindowStartedAt = millis();

  Serial.println(F("=== SQI FLOW V5 BASELINE / GATE 01 ==="));
  Serial.println(F("MAX30102 OK. Janela=5s. Somente G1 ativo."));
  Serial.println(F("Mantenha o dedo no sensor para validar o Gate 01."));
}

void loop()
{
  // A biblioteca SparkFun cuida da comunicacao/FIFO do MAX30102.
  particleSensor.check();

  while (particleSensor.available())
  {
    const uint32_t red = particleSensor.getFIFORed();
    const uint32_t ir  = particleSensor.getFIFOIR();

    // O Gate 01 trabalha diretamente com RAW: nada e filtrado antes dele.
    gate1AddSample(red, ir);

#if ENABLE_PACKED18_STORAGE_TEST
    // Teste independente de storage: preserva as primeiras 100 amostras
    // da janela em exatamente 3 bytes por canal/amostra.
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
      Serial.println(F("PIPELINE: G1 PASS -> aguardando implementacao do G2"));
    }
    else
    {
      Serial.println(F("PIPELINE: INVALID -> janela rejeitada no G1"));
    }

    Serial.println(F("--------------------------------------------------"));

    gate1Reset();
#if ENABLE_PACKED18_STORAGE_TEST
    packed18Reset();
#endif
    sqiWindowStartedAt = millis();
  }
}
