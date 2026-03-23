/*
 * main.c  –  AES-128 demo for MicroBlaze on Nexys A7 100T
 *
 * Prints results over UART using xil_printf (Vitis / Xilinx SDK).
 * If you are using a bare-metal Hello World project, just drop
 * aes.h and aes.c into the src/ folder alongside this file.
 *
 * Build: mb-gcc -O2 -o aes_demo.elf main.c aes.c
 */
 
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "xil_printf.h"   /* Xilinx UART print helper */
#include "aes.h"
 
/* ---------------------------------------------------------------
 * Helper: print a byte array as hex
 * --------------------------------------------------------------- */
static void print_hex(const char *label, const uint8_t *data, size_t len) {
    size_t i;
    xil_printf("%s: ", label);
    for (i = 0; i < len; i++)
        xil_printf("%02X ", data[i]);
    xil_printf("\r\n");
}
 
/* ---------------------------------------------------------------
 * FIPS-197 Known-Answer Test vectors (Appendix B)
 * Key:        2b 7e 15 16 28 ae d2 a6 ab f7 15 88 09 cf 4f 3c
 * Plaintext:  32 43 f6 a8 88 5a 30 8d 31 31 98 a2 e0 37 07 34
 * Ciphertext: 39 25 84 1d 02 dc 09 fb dc 11 85 97 19 6a 0b 32
 * --------------------------------------------------------------- */
static const uint8_t KAT_KEY[16] = {
    0x2b,0x7e,0x15,0x16, 0x28,0xae,0xd2,0xa6,
    0xab,0xf7,0x15,0x88, 0x09,0xcf,0x4f,0x3c
};
static const uint8_t KAT_PT[16] = {
    0x32,0x43,0xf6,0xa8, 0x88,0x5a,0x30,0x8d,
    0x31,0x31,0x98,0xa2, 0xe0,0x37,0x07,0x34
};
static const uint8_t KAT_CT[16] = {
    0x39,0x25,0x84,0x1d, 0x02,0xdc,0x09,0xfb,
    0xdc,0x11,0x85,0x97, 0x19,0x6a,0x0b,0x32
};
 
/* ---------------------------------------------------------------
 * CBC demo data
 * --------------------------------------------------------------- */
static const uint8_t CBC_KEY[16] = {
    0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f
};
static const uint8_t CBC_IV[16] = {
    0x00,0x11,0x22,0x33, 0x44,0x55,0x66,0x77,
    0x88,0x99,0xaa,0xbb, 0xcc,0xdd,0xee,0xff
};
 
/* ---------------------------------------------------------------
 * Entry point
 * --------------------------------------------------------------- */
int main(void) {
 
    AES_CTX ctx;
    uint8_t enc_buf[16], dec_buf[16];
    int kat_pass;
 
    xil_printf("\r\n===== AES-128 Demo on MicroBlaze (Nexys A7 100T) =====\r\n\r\n");
 
    /* ============================================================
     * TEST 1: FIPS-197 Known-Answer Test (ECB)
     * ============================================================ */
    xil_printf("--- TEST 1: FIPS-197 ECB Known-Answer Test ---\r\n");
 
    AES_init(&ctx, KAT_KEY);
 
    print_hex("Key      ", KAT_KEY, 16);
    print_hex("Plaintext", KAT_PT,  16);
 
    AES_encrypt_block(&ctx, KAT_PT, enc_buf);
    print_hex("Encrypted", enc_buf, 16);
    print_hex("Expected ", KAT_CT,  16);
 
    kat_pass = (memcmp(enc_buf, KAT_CT, 16) == 0);
    xil_printf("Encrypt KAT: %s\r\n\r\n", kat_pass ? "PASS" : "FAIL");
 
    AES_decrypt_block(&ctx, KAT_CT, dec_buf);
    print_hex("Decrypted", dec_buf, 16);
    print_hex("Expected ", KAT_PT,  16);
    xil_printf("Decrypt KAT: %s\r\n\r\n",
               (memcmp(dec_buf, KAT_PT, 16) == 0) ? "PASS" : "FAIL");
 
    /* ============================================================
     * TEST 2: CBC round-trip with PKCS#7 padding
     * ============================================================ */
    xil_printf("--- TEST 2: CBC Encrypt / Decrypt Round-Trip ---\r\n");
 
    {
        /* Message that is NOT a multiple of 16 bytes – pad it first */
        const char *msg = "Hello MicroBlaze!";   /* 17 bytes */
        uint8_t padded[32];
        uint8_t cipher[32];
        uint8_t plain[32];
        size_t  padded_len, orig_len;
 
        padded_len = AES_pkcs7_pad((const uint8_t *)msg, strlen(msg),
                                   padded, sizeof(padded));
 
        xil_printf("Message    : \"%s\" (%u bytes)\r\n", msg, (unsigned)strlen(msg));
        print_hex("Padded     ", padded, padded_len);
 
        AES_init(&ctx, CBC_KEY);
 
        AES_CBC_encrypt(&ctx, CBC_IV, padded, cipher, padded_len);
        print_hex("Ciphertext ", cipher, padded_len);
 
        AES_CBC_decrypt(&ctx, CBC_IV, cipher, plain, padded_len);
 
        orig_len = AES_pkcs7_unpad(plain, padded_len);
        plain[orig_len] = '\0';   /* null-terminate for printing */
 
        xil_printf("Decrypted  : \"%s\" (%u bytes)\r\n", (char *)plain, (unsigned)orig_len);
        xil_printf("CBC Test   : %s\r\n\r\n",
                   (strcmp((char *)plain, msg) == 0) ? "PASS" : "FAIL");
    }
 
    /* ============================================================
     * TEST 3: Custom key / plaintext – change these to your values
     * ============================================================ */
    xil_printf("--- TEST 3: Custom Encrypt/Decrypt ---\r\n");
 
    {
        uint8_t my_key[16]       = { 0xAA,0xBB,0xCC,0xDD,
                                     0xEE,0xFF,0x00,0x11,
                                     0x22,0x33,0x44,0x55,
                                     0x66,0x77,0x88,0x99 };
        uint8_t my_plaintext[16] = { 0xDE,0xAD,0xBE,0xEF,
                                     0xCA,0xFE,0xBA,0xBE,
                                     0x01,0x23,0x45,0x67,
                                     0x89,0xAB,0xCD,0xEF };
        uint8_t my_cipher[16], my_plain[16];
 
        AES_init(&ctx, my_key);
 
        print_hex("My Key     ", my_key,       16);
        print_hex("My PT      ", my_plaintext, 16);
 
        AES_encrypt_block(&ctx, my_plaintext, my_cipher);
        print_hex("My CT      ", my_cipher,    16);
 
        AES_decrypt_block(&ctx, my_cipher, my_plain);
        print_hex("My PT(dec) ", my_plain,     16);
 
        xil_printf("Round-trip : %s\r\n\r\n",
                   (memcmp(my_plain, my_plaintext, 16) == 0) ? "PASS" : "FAIL");
    }
 
    xil_printf("===== All tests done =====\r\n");
    return 0;
}