/*
 * aes.c  –  AES-128 pure-software implementation
 *
 * Target : MicroBlaze soft-core on Nexys A7 100T
 * Build  : mb-gcc -O2 -c aes.c
 *
 * Implements FIPS 197 (AES) with:
 *   • Full AES-128 key schedule
 *   • ECB encrypt / decrypt (one 16-byte block)
 *   • CBC encrypt / decrypt (arbitrary multiple of 16 bytes)
 *   • PKCS#7 pad / unpad helpers
 *
 * All tables are stored in ROM-friendly const arrays so the
 * MicroBlaze linker can place them in LUT/BRAM.
 */

#include "aes.h"
#include <string.h>   /* memcpy, memset */

/* ===================================================================
 * AES S-Box and inverse S-Box (FIPS 197, Fig. 7 & 14)
 * =================================================================== */
static const uint8_t SBOX[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

static const uint8_t INV_SBOX[256] = {
    0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
    0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
    0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
    0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
    0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
    0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
    0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
    0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
    0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
    0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
    0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
    0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
    0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
    0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
    0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
    0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
};

/* Round constants for key schedule (only first 10 needed for AES-128) */
static const uint8_t RCON[11] = {
    0x00, /* unused (rounds are 1-indexed) */
    0x01, 0x02, 0x04, 0x08, 0x10,
    0x20, 0x40, 0x80, 0x1b, 0x36
};

/* ===================================================================
 * GF(2^8) multiply by 2 (xtime) and by 3
 * =================================================================== */
static inline uint8_t xtime(uint8_t x) {
    return (uint8_t)((x << 1) ^ ((x & 0x80) ? 0x1b : 0x00));
}

static inline uint8_t gf_mul(uint8_t x, uint8_t y) {
    uint8_t r = 0;
    while (y) {
        if (y & 1) r ^= x;
        x = xtime(x);
        y >>= 1;
    }
    return r;
}

/* ===================================================================
 * Key Expansion  (FIPS 197 §5.2)
 * =================================================================== */
void AES_init(AES_CTX *ctx, const uint8_t key[AES_KEY_SIZE]) {
    uint8_t *rk = ctx->round_key;
    uint8_t  tmp[4];
    int i;

    /* Copy original key into first round key slot */
    memcpy(rk, key, AES_KEY_SIZE);

    for (i = AES_KEY_SIZE; i < AES_BLOCK_SIZE * (AES_NUM_ROUNDS + 1); i += 4) {
        tmp[0] = rk[i - 4];
        tmp[1] = rk[i - 3];
        tmp[2] = rk[i - 2];
        tmp[3] = rk[i - 1];

        if ((i % AES_KEY_SIZE) == 0) {
            /* RotWord then SubWord then XOR Rcon */
            uint8_t t = tmp[0];
            tmp[0] = SBOX[tmp[1]] ^ RCON[i / AES_KEY_SIZE];
            tmp[1] = SBOX[tmp[2]];
            tmp[2] = SBOX[tmp[3]];
            tmp[3] = SBOX[t];
        }

        rk[i + 0] = rk[i - AES_KEY_SIZE + 0] ^ tmp[0];
        rk[i + 1] = rk[i - AES_KEY_SIZE + 1] ^ tmp[1];
        rk[i + 2] = rk[i - AES_KEY_SIZE + 2] ^ tmp[2];
        rk[i + 3] = rk[i - AES_KEY_SIZE + 3] ^ tmp[3];
    }
}

/* ===================================================================
 * Internal helpers – state is a 4×4 byte array (column-major)
 * state[col][row]
 * =================================================================== */
typedef uint8_t State[4][4];

/* Load 16-byte block into state (column-major) */
static void load_state(State s, const uint8_t blk[16]) {
    int r, c;
    for (c = 0; c < 4; c++)
        for (r = 0; r < 4; r++)
            s[c][r] = blk[c * 4 + r];
}

/* Store state back to 16-byte block */
static void store_state(uint8_t blk[16], const State s) {
    int r, c;
    for (c = 0; c < 4; c++)
        for (r = 0; r < 4; r++)
            blk[c * 4 + r] = s[c][r];
}

/* XOR round key into state */
static void add_round_key(State s, const uint8_t *rk) {
    int r, c;
    for (c = 0; c < 4; c++)
        for (r = 0; r < 4; r++)
            s[c][r] ^= rk[c * 4 + r];
}

/* SubBytes – apply S-Box to every byte */
static void sub_bytes(State s) {
    int r, c;
    for (c = 0; c < 4; c++)
        for (r = 0; r < 4; r++)
            s[c][r] = SBOX[s[c][r]];
}

/* InvSubBytes */
static void inv_sub_bytes(State s) {
    int r, c;
    for (c = 0; c < 4; c++)
        for (r = 0; r < 4; r++)
            s[c][r] = INV_SBOX[s[c][r]];
}

/* ShiftRows – left-rotate row i by i positions */
static void shift_rows(State s) {
    uint8_t t;
    /* Row 1: shift left 1 */
    t = s[0][1]; s[0][1] = s[1][1]; s[1][1] = s[2][1]; s[2][1] = s[3][1]; s[3][1] = t;
    /* Row 2: shift left 2 */
    t = s[0][2]; s[0][2] = s[2][2]; s[2][2] = t;
    t = s[1][2]; s[1][2] = s[3][2]; s[3][2] = t;
    /* Row 3: shift left 3 (= right 1) */
    t = s[3][3]; s[3][3] = s[2][3]; s[2][3] = s[1][3]; s[1][3] = s[0][3]; s[0][3] = t;
}

/* InvShiftRows */
static void inv_shift_rows(State s) {
    uint8_t t;
    /* Row 1: shift right 1 */
    t = s[3][1]; s[3][1] = s[2][1]; s[2][1] = s[1][1]; s[1][1] = s[0][1]; s[0][1] = t;
    /* Row 2: shift right 2 */
    t = s[0][2]; s[0][2] = s[2][2]; s[2][2] = t;
    t = s[1][2]; s[1][2] = s[3][2]; s[3][2] = t;
    /* Row 3: shift right 3 (= left 1) */
    t = s[0][3]; s[0][3] = s[1][3]; s[1][3] = s[2][3]; s[2][3] = s[3][3]; s[3][3] = t;
}

/* MixColumns – FIPS 197 §5.1.3 */
static void mix_columns(State s) {
    int c;
    for (c = 0; c < 4; c++) {
        uint8_t s0 = s[c][0], s1 = s[c][1], s2 = s[c][2], s3 = s[c][3];
        s[c][0] = gf_mul(s0,2) ^ gf_mul(s1,3) ^ s2             ^ s3;
        s[c][1] = s0             ^ gf_mul(s1,2) ^ gf_mul(s2,3) ^ s3;
        s[c][2] = s0             ^ s1             ^ gf_mul(s2,2) ^ gf_mul(s3,3);
        s[c][3] = gf_mul(s0,3) ^ s1             ^ s2             ^ gf_mul(s3,2);
    }
}

/* InvMixColumns – FIPS 197 §5.3.3 */
static void inv_mix_columns(State s) {
    int c;
    for (c = 0; c < 4; c++) {
        uint8_t s0 = s[c][0], s1 = s[c][1], s2 = s[c][2], s3 = s[c][3];
        s[c][0] = gf_mul(s0,0x0e)^gf_mul(s1,0x0b)^gf_mul(s2,0x0d)^gf_mul(s3,0x09);
        s[c][1] = gf_mul(s0,0x09)^gf_mul(s1,0x0e)^gf_mul(s2,0x0b)^gf_mul(s3,0x0d);
        s[c][2] = gf_mul(s0,0x0d)^gf_mul(s1,0x09)^gf_mul(s2,0x0e)^gf_mul(s3,0x0b);
        s[c][3] = gf_mul(s0,0x0b)^gf_mul(s1,0x0d)^gf_mul(s2,0x09)^gf_mul(s3,0x0e);
    }
}

/* ===================================================================
 * AES Encrypt – one 16-byte block (ECB)
 * =================================================================== */
void AES_encrypt_block(const AES_CTX *ctx,
                       const uint8_t  in[AES_BLOCK_SIZE],
                       uint8_t        out[AES_BLOCK_SIZE])
{
    State s;
    int round;

    load_state(s, in);
    add_round_key(s, ctx->round_key);

    for (round = 1; round < AES_NUM_ROUNDS; round++) {
        sub_bytes(s);
        shift_rows(s);
        mix_columns(s);
        add_round_key(s, ctx->round_key + round * AES_BLOCK_SIZE);
    }

    /* Final round (no MixColumns) */
    sub_bytes(s);
    shift_rows(s);
    add_round_key(s, ctx->round_key + AES_NUM_ROUNDS * AES_BLOCK_SIZE);

    store_state(out, s);
}

/* ===================================================================
 * AES Decrypt – one 16-byte block (ECB)
 * =================================================================== */
void AES_decrypt_block(const AES_CTX *ctx,
                       const uint8_t  in[AES_BLOCK_SIZE],
                       uint8_t        out[AES_BLOCK_SIZE])
{
    State s;
    int round;

    load_state(s, in);
    add_round_key(s, ctx->round_key + AES_NUM_ROUNDS * AES_BLOCK_SIZE);

    for (round = AES_NUM_ROUNDS - 1; round >= 1; round--) {
        inv_shift_rows(s);
        inv_sub_bytes(s);
        add_round_key(s, ctx->round_key + round * AES_BLOCK_SIZE);
        inv_mix_columns(s);
    }

    /* Final round */
    inv_shift_rows(s);
    inv_sub_bytes(s);
    add_round_key(s, ctx->round_key);

    store_state(out, s);
}

/* ===================================================================
 * CBC Encrypt
 * =================================================================== */
void AES_CBC_encrypt(const AES_CTX *ctx,
                     const uint8_t *iv,
                     const uint8_t *plaintext,
                     uint8_t       *ciphertext,
                     size_t         len)
{
    uint8_t feedback[AES_BLOCK_SIZE];
    uint8_t tmp[AES_BLOCK_SIZE];
    size_t i, b;

    memcpy(feedback, iv, AES_BLOCK_SIZE);

    for (b = 0; b < len; b += AES_BLOCK_SIZE) {
        /* XOR plaintext block with feedback (previous ciphertext / IV) */
        for (i = 0; i < AES_BLOCK_SIZE; i++)
            tmp[i] = plaintext[b + i] ^ feedback[i];

        AES_encrypt_block(ctx, tmp, ciphertext + b);

        /* Next feedback = this ciphertext block */
        memcpy(feedback, ciphertext + b, AES_BLOCK_SIZE);
    }
}

/* ===================================================================
 * CBC Decrypt
 * =================================================================== */
void AES_CBC_decrypt(const AES_CTX *ctx,
                     const uint8_t *iv,
                     const uint8_t *ciphertext,
                     uint8_t       *plaintext,
                     size_t         len)
{
    uint8_t feedback[AES_BLOCK_SIZE];
    uint8_t tmp[AES_BLOCK_SIZE];
    size_t i, b;

    memcpy(feedback, iv, AES_BLOCK_SIZE);

    for (b = 0; b < len; b += AES_BLOCK_SIZE) {
        AES_decrypt_block(ctx, ciphertext + b, tmp);

        /* XOR with feedback to recover plaintext */
        for (i = 0; i < AES_BLOCK_SIZE; i++)
            plaintext[b + i] = tmp[i] ^ feedback[i];

        /* Next feedback = this ciphertext block */
        memcpy(feedback, ciphertext + b, AES_BLOCK_SIZE);
    }
}

/* ===================================================================
 * PKCS#7 Padding helpers
 * =================================================================== */
size_t AES_pkcs7_pad(const uint8_t *in, size_t in_len,
                     uint8_t *out_buf, size_t out_buf_size)
{
    size_t pad_len = AES_BLOCK_SIZE - (in_len % AES_BLOCK_SIZE);
    size_t padded  = in_len + pad_len;

    if (padded > out_buf_size) return 0;   /* buffer too small */

    memcpy(out_buf, in, in_len);
    memset(out_buf + in_len, (uint8_t)pad_len, pad_len);
    return padded;
}

size_t AES_pkcs7_unpad(const uint8_t *buf, size_t padded_len)
{
    uint8_t pad_byte;
    size_t i;

    if (padded_len == 0 || (padded_len % AES_BLOCK_SIZE) != 0) return 0;

    pad_byte = buf[padded_len - 1];
    if (pad_byte == 0 || pad_byte > AES_BLOCK_SIZE) return 0;

    for (i = padded_len - pad_byte; i < padded_len; i++)
        if (buf[i] != pad_byte) return 0;

    return padded_len - pad_byte;
}