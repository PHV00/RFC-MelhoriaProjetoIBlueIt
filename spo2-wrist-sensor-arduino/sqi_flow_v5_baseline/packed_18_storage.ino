/*
 * PACKED 18-BIT STORAGE - MAX30102 / ARDUINO UNO
 * =================================================
 *
 * OBJETIVO
 * --------
 * Preservar EXATAMENTE os 18 bits entregues pelo MAX30102 usando 3 bytes
 * por amostra, em vez de 4 bytes de uint32_t.
 *
 * Isto NAO e "bit packing" denso de 18 bits. Cada amostra ocupa 24 bits:
 *
 *   byte 0: bits  0..7
 *   byte 1: bits  8..15
 *   byte 2: bits 16..17 (os 6 bits superiores ficam zerados)
 *
 * Consequencia:
 *   100 RED + 100 IR = 100 * 3 * 2 = 600 bytes
 *
 * Esta camada e usada NESTA ETAPA apenas para verificar que:
 *   RAW18 -> 3 bytes -> RAW18
 * e uma transformacao sem perda.
 *
 * O Gate 01 continua recebendo diretamente o RAW uint32_t original.
 */

#ifndef ENABLE_PACKED18_BUFFER
#define ENABLE_PACKED18_BUFFER 0
#endif

#ifndef ENABLE_PACKED18_STORAGE_TEST
#define ENABLE_PACKED18_STORAGE_TEST 0
#endif

#if ENABLE_PACKED18_BUFFER

struct Packed18
{
  uint8_t b0;
  uint8_t b1;
  uint8_t b2;
};

static const uint32_t PACKED18_MASK = 0x3FFFFUL; // 18 bits
static const uint16_t PACKED18_BUFFER_SAMPLES = 100;

static Packed18 packedRedBuffer[PACKED18_BUFFER_SAMPLES];
static Packed18 packedIrBuffer[PACKED18_BUFFER_SAMPLES];

static uint16_t packed18Count;
static uint16_t packed18ImmediateMismatchCount;
static uint16_t packed18RedAbove16Count;
static uint16_t packed18IrAbove16Count;

static uint32_t packed18ExpectedRedSum;
static uint32_t packed18ExpectedIrSum;

Packed18 packed18Encode(uint32_t value)
{
  value &= PACKED18_MASK;

  Packed18 out;
  out.b0 = (uint8_t)(value & 0xFFUL);
  out.b1 = (uint8_t)((value >> 8) & 0xFFUL);
  out.b2 = (uint8_t)((value >> 16) & 0x03UL);
  return out;
}

uint32_t packed18Decode(const Packed18 &value)
{
  return ((uint32_t)value.b0)
       | ((uint32_t)value.b1 << 8)
       | ((uint32_t)(value.b2 & 0x03U) << 16);
}

void packed18Reset()
{
  packed18Count = 0;
  packed18ImmediateMismatchCount = 0;
  packed18RedAbove16Count = 0;
  packed18IrAbove16Count = 0;

  packed18ExpectedRedSum = 0;
  packed18ExpectedIrSum = 0;
}

uint16_t packed18GetCount()
{
  return packed18Count;
}

uint32_t packed18GetRed(uint16_t index)
{
  if (index >= packed18Count) return 0;
  return packed18Decode(packedRedBuffer[index]);
}

uint32_t packed18GetIr(uint16_t index)
{
  if (index >= packed18Count) return 0;
  return packed18Decode(packedIrBuffer[index]);
}

void packed18StoreSample(uint32_t red, uint32_t ir)
{
  // O buffer de 100 amostras e apenas a janela de verificacao de storage.
  // O G1 continua processando TODAS as amostras da janela temporal de 5 s.
  if (packed18Count >= PACKED18_BUFFER_SAMPLES)
  {
    return;
  }

  const uint32_t red18 = red & PACKED18_MASK;
  const uint32_t ir18 = ir & PACKED18_MASK;

  packedRedBuffer[packed18Count] = packed18Encode(red18);
  packedIrBuffer[packed18Count] = packed18Encode(ir18);

  packed18ExpectedRedSum += red18;
  packed18ExpectedIrSum += ir18;

  if (red18 > 65535UL) packed18RedAbove16Count++;
  if (ir18 > 65535UL) packed18IrAbove16Count++;

  // Verificacao imediata de round-trip.
  if (packed18Decode(packedRedBuffer[packed18Count]) != red18)
  {
    packed18ImmediateMismatchCount++;
  }

  if (packed18Decode(packedIrBuffer[packed18Count]) != ir18)
  {
    packed18ImmediateMismatchCount++;
  }

  packed18Count++;
}

#if defined(__AVR__)
extern int __heap_start;
extern void *__brkval;

int packed18FreeRam()
{
  int stackTop;
  int heapTop = (__brkval == 0)
    ? (int)&__heap_start
    : (int)__brkval;

  return (int)&stackTop - heapTop;
}
#else
int packed18FreeRam()
{
  return -1;
}
#endif

#if ENABLE_PACKED18_STORAGE_TEST
void packed18PrintReport()
{
  uint32_t decodedRedSum = 0;
  uint32_t decodedIrSum = 0;

  for (uint16_t i = 0; i < packed18Count; i++)
  {
    decodedRedSum += packed18Decode(packedRedBuffer[i]);
    decodedIrSum += packed18Decode(packedIrBuffer[i]);
  }

  const bool exactRoundTrip =
    (packed18ImmediateMismatchCount == 0)
    && (decodedRedSum == packed18ExpectedRedSum)
    && (decodedIrSum == packed18ExpectedIrSum);

  Serial.print(F("PACK18_RESULT="));
  Serial.print(exactRoundTrip ? F("PASS") : F("FAIL"));

  Serial.print(F(" samples="));
  Serial.print(packed18Count);

  Serial.print(F(" bytes_per_sample="));
  Serial.print(sizeof(Packed18));

  Serial.print(F(" buffers_bytes="));
  Serial.print(sizeof(packedRedBuffer) + sizeof(packedIrBuffer));

  Serial.print(F(" mismatch="));
  Serial.print(packed18ImmediateMismatchCount);

  Serial.print(F(" RED>65535="));
  Serial.print(packed18RedAbove16Count);

  Serial.print(F(" IR>65535="));
  Serial.print(packed18IrAbove16Count);

  Serial.print(F(" freeRAM="));
  Serial.print(packed18FreeRam());

  Serial.println();
}
#endif // ENABLE_PACKED18_STORAGE_TEST

#endif // ENABLE_PACKED18_BUFFER
