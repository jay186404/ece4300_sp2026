#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstdint>

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>

using namespace std;
using namespace std::chrono;
using namespace CryptoPP;

const int KEY_SETUP_ROUNDS = 5000;
const int BLOCK_ROUNDS = 10000;

unsigned char plaintext[16] = {
    0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96,
    0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A
};

unsigned char ciphertext[16];
unsigned char recovered[16];

unsigned char key128[16] = {
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

unsigned char key192[24] = {
    0x8E, 0x73, 0xB0, 0xF7, 0xDA, 0x0E, 0x64, 0x52,
    0xC8, 0x10, 0xF3, 0x2B, 0x80, 0x90, 0x79, 0xE5,
    0x62, 0xF8, 0xEA, 0xD2, 0x52, 0x2C, 0x6B, 0x7B
};

unsigned char key256[32] = {
    0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE,
    0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81,
    0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61, 0x08, 0xD7,
    0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4
};

bool blocksEqual(const unsigned char* a, const unsigned char* b, int len) {
    for (int i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

void printBlock(const unsigned char* data, int len) {
    for (int i = 0; i < len; i++) {
        cout << hex << uppercase << setw(2) << setfill('0')
            << static_cast<int>(data[i]) << " ";
    }
    cout << dec << nouppercase << setfill(' ') << "\n";
}

void benchmarkAES(const char* label, const unsigned char* key, int keyLen) {
    ECB_Mode<AES>::Encryption enc;
    ECB_Mode<AES>::Decryption decObj;

    cout << "========================================\n";
    cout << "Benchmarking " << label << "\n";

    auto start = high_resolution_clock::now();
    for (int i = 0; i < KEY_SETUP_ROUNDS; i++) {
        enc.SetKey(key, keyLen);
        decObj.SetKey(key, keyLen);
    }
    auto end = high_resolution_clock::now();
    auto keySetupTotal = duration_cast<microseconds>(end - start).count();

    enc.SetKey(key, keyLen);
    decObj.SetKey(key, keyLen);

    start = high_resolution_clock::now();
    for (int i = 0; i < BLOCK_ROUNDS; i++) {
        enc.ProcessData(ciphertext, plaintext, 16);
    }
    end = high_resolution_clock::now();
    auto encryptTotal = duration_cast<microseconds>(end - start).count();

    start = high_resolution_clock::now();
    for (int i = 0; i < BLOCK_ROUNDS; i++) {
        decObj.ProcessData(recovered, ciphertext, 16);
    }
    end = high_resolution_clock::now();
    auto decryptTotal = duration_cast<microseconds>(end - start).count();

    bool ok = blocksEqual(plaintext, recovered, 16);

    cout << "Key size (bytes): " << keyLen << "\n";
    cout << "Block size (bytes): " << AES::BLOCKSIZE << "\n";
    cout << "State size (bytes): " << sizeof(enc) + sizeof(decObj) << "\n";
    cout << "\n";

    cout << fixed << setprecision(2);

    cout << "Key setup total (us): ";
    cout << keySetupTotal << "\n";
    cout << "Key setup avg (us):   ";
    cout << (double)keySetupTotal / KEY_SETUP_ROUNDS << "\n";
    cout << "\n";

    cout << "Encrypt total (us):   ";
    cout << encryptTotal << "\n";
    cout << "Encrypt avg/block:    ";
    cout << (double)encryptTotal / BLOCK_ROUNDS << " us\n";

    cout << "Decrypt total (us):   ";
    cout << decryptTotal << "\n";
    cout << "Decrypt avg/block:    ";
    cout << (double)decryptTotal / BLOCK_ROUNDS << " us\n";
    cout << "\n";

    cout << "Ciphertext sample:    ";
    printBlock(ciphertext, 16);

    cout << "Recovered sample:     ";
    printBlock(recovered, 16);

    cout << "Verification:         ";
    cout << (ok ? "PASS" : "FAIL") << "\n";
    cout << "\n";
}

int main() {
    try {
        cout << "AES Benchmark on Windows x86_64\n";
        cout << "Using Crypto++\n\n";

        benchmarkAES("AES-128", key128, sizeof(key128));
        benchmarkAES("AES-192", key192, sizeof(key192));
        benchmarkAES("AES-256", key256, sizeof(key256));

        cout << "Done.\n";
    }
    catch (const Exception& e) {
        cerr << "Crypto++ error: " << e.what() << "\n";
        return 1;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}