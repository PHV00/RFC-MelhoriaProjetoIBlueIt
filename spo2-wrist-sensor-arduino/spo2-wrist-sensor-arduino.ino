/*
  I Blue It v5 - Oximetria MAX30102 - Arduino Uno R3
  Compatibilidade mantida com o firmware oficial:
    Serial: 115200 baud
    'E'/'e' -> responde "echox"
    'R'/'r' -> inicia aquisição
    'F'/'f' -> interrompe aquisição
    Saída durante aquisição: FC,SpO2
    Exemplo: 72,97

  Pinagem Arduino Uno R3 (ATmega328P):
    MAX30102 SDA -> A4 (ou pino SDA dedicado; é o mesmo barramento)
    MAX30102 SCL -> A5 (ou pino SCL dedicado; é o mesmo barramento)
    MAX30102 GND -> GND
    MAX30102 VIN/VCC -> conforme o breakout utilizado
    INT -> não utilizado
    LED onboard de leitura -> D13

  Base funcional: firmware oficial do repositório
  UDESC-LARVA/iblueit-psychophysiological-flow.
*/

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

#define DEBUG false

MAX30105 particleSensor;

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
uint16_t irBuffer[100];
uint16_t redBuffer[100];
#else
uint32_t irBuffer[100];
uint32_t redBuffer[100];
#endif

int32_t bufferLength = 100;
int32_t spo2 = 0;
int8_t validSPO2 = 0;
int32_t heartRate = 0;
int8_t validHeartRate = 0;

const byte readLED = 13;

int cont = 0;
long sumHeartRate = 0;
long sumSPO2 = 0;

bool isSampling = false;
bool firstSample = true;

void listenCommand(char cmd) {
  if (cmd == 'e' || cmd == 'E') {
    Serial.println("echox");
  } else if (cmd == 'r' || cmd == 'R') {
    isSampling = true;
  } else if (cmd == 'f' || cmd == 'F') {
    isSampling = false;
  }
}

bool initializeMAX30102() {
  // No Arduino Uno R3, Wire usa SDA=A4 e SCL=A5.
  // Os pinos SDA/SCL dedicados da placa são eletricamente o mesmo barramento.
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    return false;
  }

  // Mantém a configuração usada pelo firmware oficial do I Blue It v5.
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(readLED, OUTPUT);
  digitalWrite(readLED, LOW);

  delay(2000);

  if (!initializeMAX30102()) {
    // Esta mensagem só aparece em falha; em operação normal o protocolo
    // permanece limpo para o I Blue It.
    Serial.println("ERRO_MAX30102");
    while (true) {
      digitalWrite(readLED, !digitalRead(readLED));
      delay(250);
    }
  }
}

void loop() {
#if DEBUG
  isSampling = true;
#endif

  if (Serial.available() > 0) {
    listenCommand((char)Serial.read());
  }

  if (isSampling) {
    heartRateAndSPO2();
  }
}

void heartRateAndSPO2() {
  if (firstSample) {
    bufferLength = 100;

    for (byte i = 0; i < bufferLength; i++) {
      while (!particleSensor.available()) {
        particleSensor.check();
      }

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();
    }

    maxim_heart_rate_and_oxygen_saturation(
      irBuffer, bufferLength, redBuffer,
      &spo2, &validSPO2, &heartRate, &validHeartRate
    );

    firstSample = false;
  }

  for (byte i = 25; i < 100; i++) {
    redBuffer[i - 25] = redBuffer[i];
    irBuffer[i - 25] = irBuffer[i];
  }

  for (byte i = 75; i < 100; i++) {
    while (!particleSensor.available()) {
      particleSensor.check();
    }

    digitalWrite(readLED, !digitalRead(readLED));

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();

    // Limiar mantido do firmware oficial.
    if (irBuffer[i] > 7000) {
      if (validHeartRate == 1 && validSPO2 == 1) {
        cont++;
        sumHeartRate += heartRate;
        sumSPO2 += spo2;
      }
    }
  }

  if (cont > 0) {
    Serial.print(sumHeartRate / cont);
    Serial.print(",");
    Serial.println(sumSPO2 / cont);
  } else {
    Serial.println("0,0");
  }

  cont = 0;
  sumHeartRate = 0;
  sumSPO2 = 0;

  maxim_heart_rate_and_oxygen_saturation(
    irBuffer, bufferLength, redBuffer,
    &spo2, &validSPO2, &heartRate, &validHeartRate
  );
}
