#ifndef AES_H
#define AES_H

#include <stdint.h>
#include <stddef.h>

/* ---------------------------------------------------------------
 * AES-128 Pure-Software Implementation
 * Target : MicroBlaze soft-core on Nexys A7 100T (Xilinx/AMD)
 * Toolchain: Vitis / Xilinx SDK  (mb-gcc)
 * Block  : 128-bit  |  Key: 128-bit  |  Rounds: 10
 * Modes  : ECB (single block) and CBC (multi-block)
 * --------------------------------------------------------------- */

#define AES_BLOCK_SIZE  16          /* bytes */
#define AES_KEY_SIZE    16          /* bytes – AES-128 */
#define AES_NUM_ROUNDS  10

/* Holds expanded round keys produced by AES_init() */
typedef struct {
    uint8_t round_key[AES_BLOCK_SIZE * (AES_NUM_ROUNDS + 1)]; /* 176 bytes */
} AES_CTX;

/* ---------- Key Expansion ---------- */
void AES_init(AES_CTX *ctx, const uint8_t key[AES_KEY_SIZE]);

/* ---------- ECB – single 16-byte block ---------- */
void AES_encrypt_block(const AES_CTX *ctx,
                       const uint8_t  in[AES_BLOCK_SIZE],
                       uint8_t        out[AES_BLOCK_SIZE]);

void AES_decrypt_block(const AES_CTX *ctx,
                       const uint8_t  in[AES_BLOCK_SIZE],
                       uint8_t        out[AES_BLOCK_SIZE]);

/* ---------- CBC – arbitrary length (must be multiple of 16) ---------- */
void AES_CBC_encrypt(const AES_CTX *ctx,
                     const uint8_t *iv,          /* 16 bytes, not modified */
                     const uint8_t *plaintext,
                     uint8_t       *ciphertext,
                     size_t         len);         /* must be multiple of 16 */

void AES_CBC_decrypt(const AES_CTX *ctx,
                     const uint8_t *iv,           /* same iv used during encrypt */
                     const uint8_t *ciphertext,
                     uint8_t       *plaintext,
                     size_t         len);          /* must be multiple of 16 */

/* ---------- PKCS#7 Padding helpers ---------- */
/* Pads 'in' (in_len bytes) into out_buf; returns padded length, 0 on error. */
size_t AES_pkcs7_pad(const uint8_t *in, size_t in_len,
                     uint8_t *out_buf, size_t out_buf_size);

/* Returns original data length after stripping pad, 0 on bad padding. */
size_t AES_pkcs7_unpad(const uint8_t *buf, size_t padded_len);

#endif /* AES_H */