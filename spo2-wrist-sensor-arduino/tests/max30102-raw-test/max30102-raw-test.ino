/*
  Teste de bancada - Arduino Uno R3 + MAX30102

  Objetivo:
    - validar comunicacao I2C;
    - validar aquisicao dos canais RED e IR;
    - observar resposta a presenca do dedo;
    - nao calcular FC, SpO2 ou SQI nesta etapa.

  Ligacao usada no teste:
    MAX30102 VIN -> 3.3V do Arduino Uno
    MAX30102 GND -> GND
    MAX30102 SDA -> SDA (A4)
    MAX30102 SCL -> SCL (A5)

  Biblioteca:
    SparkFun MAX3010x Pulse and Proximity Sensor Library
*/

#include <Wire.h>
#include "MAX30105.h"

MAX30105 sensor;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Wire.begin();

  if (!sensor.begin(Wire, I2C_SPEED_STANDARD))
  {
    Serial.println("ERRO: MAX30102 nao encontrado.");
    while (1);
  }

  byte ledBrightness = 0x1F;
  byte sampleAverage = 4;
  byte ledMode = 2;      // RED + IR
  int sampleRate = 100;  // 100 Hz
  int pulseWidth = 411;  // 18 bits
  int adcRange = 4096;

  sensor.setup(
    ledBrightness,
    sampleAverage,
    ledMode,
    sampleRate,
    pulseWidth,
    adcRange
  );

  sensor.setPulseAmplitudeRed(0x1F);
  sensor.setPulseAmplitudeIR(0x1F);
  sensor.setPulseAmplitudeGreen(0);

  Serial.println("RED,IR");
}

void loop()
{
  sensor.check();

  while (sensor.available())
  {
    uint32_t red = sensor.getFIFORed();
    uint32_t ir  = sensor.getFIFOIR();

    Serial.print(red);
    Serial.print(",");
    Serial.println(ir);

    sensor.nextSample();
  }
}
