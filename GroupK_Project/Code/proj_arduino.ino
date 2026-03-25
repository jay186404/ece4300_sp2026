#include <Crypto.h>
#include <AES.h>

const int KEY_SETUP_ROUNDS = 5000;
const int BLOCK_ROUNDS = 10000;

byte plaintext[16] = {
  0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96,
  0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A
};

byte ciphertext[16];
byte recovered[16];

byte key128[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

byte key192[24] = {
  0x8E, 0x73, 0xB0, 0xF7, 0xDA, 0x0E, 0x64, 0x52,
  0xC8, 0x10, 0xF3, 0x2B, 0x80, 0x90, 0x79, 0xE5,
  0x62, 0xF8, 0xEA, 0xD2, 0x52, 0x2C, 0x6B, 0x7B
};

byte key256[32] = {
  0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE,
  0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81,
  0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61, 0x08, 0xD7,
  0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4
};

bool blocksEqual(byte *a, byte *b, int len) {
  for (int i = 0; i < len; i++) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

template <typename AESClass>
void benchmarkAES(const char *label, byte *key, int keyLen) {
  AESClass aes;

  Serial.println("========================================");
  Serial.print("Benchmarking ");
  Serial.println(label);

  unsigned long startMicros = micros();
  for (int i = 0; i < KEY_SETUP_ROUNDS; i++) {
    aes.setKey(key, keyLen);
  }
  unsigned long keySetupTotal = micros() - startMicros;

  aes.setKey(key, keyLen);

  startMicros = micros();
  for (int i = 0; i < BLOCK_ROUNDS; i++) {
    aes.encryptBlock(ciphertext, plaintext);
  }
  unsigned long encryptTotal = micros() - startMicros;

  startMicros = micros();
  for (int i = 0; i < BLOCK_ROUNDS; i++) {
    aes.decryptBlock(recovered, ciphertext);
  }
  unsigned long decryptTotal = micros() - startMicros;

  bool ok = blocksEqual(plaintext, recovered, 16);

  Serial.print("Key setup avg (us): ");
  Serial.println((float)keySetupTotal / KEY_SETUP_ROUNDS, 2);

  Serial.print("Encrypt avg/block (us): ");
  Serial.println((float)encryptTotal / BLOCK_ROUNDS, 2);

  Serial.print("Decrypt avg/block (us): ");
  Serial.println((float)decryptTotal / BLOCK_ROUNDS, 2);

  Serial.print("Verification: ");
  Serial.println(ok ? "PASS" : "FAIL");
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println();
  Serial.println("AES Benchmark on ESP32");
  Serial.println("Using Crypto library");
  Serial.println();

  benchmarkAES<AES128>("AES-128", key128, sizeof(key128));
  benchmarkAES<AES192>("AES-192", key192, sizeof(key192));
  benchmarkAES<AES256>("AES-256", key256, sizeof(key256));
}

void loop() {
}