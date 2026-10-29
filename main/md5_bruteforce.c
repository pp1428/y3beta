// md5_bruteforce.c
// Реализация MD5 (RFC 1321) и базовый брутфорс на CPU.
// Сборка: gcc -O3 -o md5_bruteforce md5_bruteforce.c
// Или в VS Code через MSVC: cl /O2 md5_bruteforce.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// =========================================================
// Реализация MD5 (RFC 1321)
// =========================================================
typedef struct {
    uint32_t state[4];
    uint32_t count[2];
    unsigned char buffer[64];
} MD5_CTX;

#define F(x,y,z) (((x) & (y)) | ((~x) & (z)))
#define G(x,y,z) (((x) & (z)) | ((y) & (~z)))
#define H(x,y,z) ((x) ^ (y) ^ (z))
#define I(x,y,z) ((y) ^ ((x) | (~z)))
#define ROTATE_LEFT(x,n) (((x) << (n)) | ((x) >> (32-(n))))

#define FF(a,b,c,d,x,s,ac) { \
    (a) += F((b),(c),(d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a),(s)); \
    (a) += (b); \
}
#define GG(a,b,c,d,x,s,ac) { \
    (a) += G((b),(c),(d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a),(s)); \
    (a) += (b); \
}
#define HH(a,b,c,d,x,s,ac) { \
    (a) += H((b),(c),(d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a),(s)); \
    (a) += (b); \
}
#define II(a,b,c,d,x,s,ac) { \
    (a) += I((b),(c),(d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a),(s)); \
    (a) += (b); \
}

static void MD5_Transform(uint32_t state[4], const unsigned char block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t x[16];

    for (int i = 0, j = 0; j < 64; i++, j += 4) {
        x[i] = ((uint32_t)block[j]) | (((uint32_t)block[j+1]) << 8) |
               (((uint32_t)block[j+2]) << 16) | (((uint32_t)block[j+3]) << 24);
    }

    FF(a,b,c,d,x[0],7,0xd76aa478);   FF(d,a,b,c,x[1],12,0xe8c7b756);
    FF(c,d,a,b,x[2],17,0x242070db);  FF(b,c,d,a,x[3],22,0xc1bdceee);
    FF(a,b,c,d,x[4],7,0xf57c0faf);   FF(d,a,b,c,x[5],12,0x4787c62a);
    FF(c,d,a,b,x[6],17,0xa8304613);  FF(b,c,d,a,x[7],22,0xfd469501);
    FF(a,b,c,d,x[8],7,0x698098d8);   FF(d,a,b,c,x[9],12,0x8b44f7af);
    FF(c,d,a,b,x[10],17,0xffff5bb1); FF(b,c,d,a,x[11],22,0x895cd7be);
    FF(a,b,c,d,x[12],7,0x6b901122);  FF(d,a,b,c,x[13],12,0xfd987193);
    FF(c,d,a,b,x[14],17,0xa679438e); FF(b,c,d,a,x[15],22,0x49b40821);

    GG(a,b,c,d,x[1],5,0xf61e2562);   GG(d,a,b,c,x[6],9,0xc040b340);
    GG(c,d,a,b,x[11],14,0x265e5a51); GG(b,c,d,a,x[0],20,0xe9b6c7aa);
    GG(a,b,c,d,x[5],5,0xd62f105d);   GG(d,a,b,c,x[10],9,0x02441453);
    GG(c,d,a,b,x[15],14,0xd8a1e681); GG(b,c,d,a,x[4],20,0xe7d3fbc8);
    GG(a,b,c,d,x[9],5,0x21e1cde6);   GG(d,a,b,c,x[14],9,0xc33707d6);
    GG(c,d,a,b,x[3],14,0xf4d50d87);  GG(b,c,d,a,x[8],20,0x455a14ed);
    GG(a,b,c,d,x[13],5,0xa9e3e905);  GG(d,a,b,c,x[2],9,0xfcefa3f8);
    GG(c,d,a,b,x[7],14,0x676f02d9);  GG(b,c,d,a,x[12],20,0x8d2a4c8a);

    HH(a,b,c,d,x[5],4,0xfffa3942);   HH(d,a,b,c,x[8],11,0x8771f681);
    HH(c,d,a,b,x[11],16,0x6d9d6122); HH(b,c,d,a,x[14],23,0xfde5380c);
    HH(a,b,c,d,x[1],4,0xa4beea44);   HH(d,a,b,c,x[4],11,0x4bdecfa9);
    HH(c,d,a,b,x[7],16,0xf6bb4b60);  HH(b,c,d,a,x[10],23,0xbebfbc70);
    HH(a,b,c,d,x[13],4,0x289b7ec6);  HH(d,a,b,c,x[0],11,0xeaa127fa);
    HH(c,d,a,b,x[3],16,0xd4ef3085);  HH(b,c,d,a,x[6],23,0x04881d05);
    HH(a,b,c,d,x[9],4,0xd9d4d039);   HH(d,a,b,c,x[12],11,0xe6db99e5);
    HH(c,d,a,b,x[15],16,0x1fa27cf8); HH(b,c,d,a,x[2],23,0xc4ac5665);

    II(a,b,c,d,x[0],6,0xf4292244);   II(d,a,b,c,x[7],10,0x432aff97);
    II(c,d,a,b,x[14],15,0xab9423a7); II(b,c,d,a,x[5],21,0xfc93a039);
    II(a,b,c,d,x[12],6,0x655b59c3);  II(d,a,b,c,x[3],10,0x8f0ccc92);
    II(c,d,a,b,x[10],15,0xffeff47d); II(b,c,d,a,x[1],21,0x85845dd1);
    II(a,b,c,d,x[8],6,0x6fa87e4f);   II(d,a,b,c,x[15],10,0xfe2ce6e0);
    II(c,d,a,b,x[6],15,0xa3014314);  II(b,c,d,a,x[13],21,0x4e0811a1);
    II(a,b,c,d,x[4],6,0xf7537e82);   II(d,a,b,c,x[11],10,0xbd3af235);
    II(c,d,a,b,x[2],15,0x2ad7d2bb);  II(b,c,d,a,x[9],21,0xeb86d391);

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
}

static void MD5_Init(MD5_CTX *ctx) {
    ctx->count[0] = ctx->count[1] = 0;
    ctx->state[0] = 0x67452301; ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe; ctx->state[3] = 0x10325476;
}

static void MD5_Update(MD5_CTX *ctx, const unsigned char *input, unsigned int inputLen) {
    unsigned int i, index, partLen;
    index = (unsigned int)((ctx->count[0] >> 3) & 0x3F);
    if ((ctx->count[0] += ((uint32_t)inputLen << 3)) < ((uint32_t)inputLen << 3))
        ctx->count[1]++;
    ctx->count[1] += ((uint32_t)inputLen >> 29);
    partLen = 64 - index;

    if (inputLen >= partLen) {
        memcpy(&ctx->buffer[index], input, partLen);
        MD5_Transform(ctx->state, ctx->buffer);
        for (i = partLen; i + 63 < inputLen; i += 64)
            MD5_Transform(ctx->state, &input[i]);
        index = 0;
    } else { i = 0; }
    memcpy(&ctx->buffer[index], &input[i], inputLen - i);
}

static void MD5_Final(unsigned char digest[16], MD5_CTX *ctx) {
    unsigned char bits[8];
    unsigned int index, padLen;
    static unsigned char PADDING[64] = { 0x80 };

    for (int i = 0; i < 8; i++) bits[i] = (unsigned char)((ctx->count[i >> 2] >> ((i & 3) << 3)) & 0xff);
    index = (unsigned int)((ctx->count[0] >> 3) & 0x3F);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5_Update(ctx, PADDING, padLen);
    MD5_Update(ctx, bits, 8);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            digest[i * 4 + j] = (unsigned char)((ctx->state[i] >> (j * 8)) & 0xff);
}

// Утилита: хэширование строки и вывод в hex
void md5_string(const char *str, char *output) {
    MD5_CTX ctx;
    unsigned char digest[16];
    MD5_Init(&ctx);
    MD5_Update(&ctx, (const unsigned char*)str, (unsigned int)strlen(str));
    MD5_Final(digest, &ctx);
    for (int i = 0; i < 16; i++) sprintf(output + (i * 2), "%02x", digest[i]);
    output[32] = '\0';
}

// =========================================================
// Брутфорс
// =========================================================

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// Ищем число x, такое что MD5(x) == target_hash
// Проверяем диапазон от 0 до max_range.
int brute_force_md5(const char *target_hash, uint64_t max_range, uint64_t *found_x) {
    char buf[32];
    char hash[33];

    for (uint64_t i = 0; i < max_range; i++) {
        sprintf(buf, "%llu", (unsigned long long)i);
        md5_string(buf, hash);
        
        if (strcmp(hash, target_hash) == 0) {
            *found_x = i;
            return 1; // Нашли!
        }
    }
    return 0; // Не нашли
}

int main(void) {
    printf("=== MD5 Brute-force (CPU) ===\n");

    // Сгенерируем целевой хэш (загадаем число 123456)
    char target_hash[33];
    md5_string("123456", target_hash);
    printf("Target hash: %s (generated from '123456')\n", target_hash);

    // Проверяем поиск
    uint64_t found = 0;
    double t0 = now_sec();
    int success = brute_force_md5(target_hash, 200000, &found);
    double t1 = now_sec();

    if (success) {
        printf("Found! x = %llu, time = %.4f s\n", (unsigned long long)found, t1 - t0);
    } else {
        printf("Not found.\n");
    }

    // Бенчмарк: проверяем скорость на разных диапазонах
    printf("\n=== Benchmark (CPU, one thread) ===\n");
    printf("%12s %12s %14s\n", "range", "time, s", "hashes/sec");

    uint64_t sizes[] = {100000, 500000, 1000000, 5000000};
    // Хэш от заведомо большого числа, чтобы поиск прошёл весь диапазон
    md5_string("999999999999999", target_hash); 

    for (int i = 0; i < (int)(sizeof(sizes)/sizeof(sizes[0])); i++) {
        uint64_t range = sizes[i];
        
        t0 = now_sec();
        brute_force_md5(target_hash, range, &found);
        t1 = now_sec();

        double elapsed = t1 - t0;
        printf("%12llu %12.4f %14.0f\n", 
            (unsigned long long)range, elapsed, (double)range / elapsed);
    }

    return 0;
}
