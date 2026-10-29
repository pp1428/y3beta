// Последовательный перебор (брутфорс) паролей по заданному хэшу SHA-256 на CPU
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sha256.h"

static const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
#define ALPH_LEN 36

// Преобразует число idx в строку длины len в системе счисления ALPH_LEN
static void idx_to_str(unsigned long long idx, int len, char* out) {
    for (int i = len - 1; i >= 0; i--) {
        out[i] = alphabet[idx % ALPH_LEN];
        idx /= ALPH_LEN;
    }
    out[len] = 0;
}

int main(void) {
    int lengths[] = {1,2,3,4};
    int nl = sizeof(lengths)/sizeof(lengths[0]);
    printf("len;space;time_ms;found;candidate\n");
    for (int li = 0; li < nl; li++) {
        int len = lengths[li];
        unsigned long long space = 1;
        for (int i = 0; i < len; i++) space *= ALPH_LEN;

        // Целевой пароль — последний элемент перебираемого пространства (худший случай)
        char target[16];
        idx_to_str(space - 1, len, target);
        char targetHash[65];
        sha256_hex(target, len, targetHash);

        char buf[16]; char h[65];
        clock_t t0 = clock();
        unsigned long long found = (unsigned long long)-1;
        for (unsigned long long idx = 0; idx < space; idx++) {
            idx_to_str(idx, len, buf);
            sha256_hex(buf, len, h);
            if (strcmp(h, targetHash) == 0) { found = idx; break; }
        }
        clock_t t1 = clock();
        double ms = 1000.0 * (double)(t1 - t0) / CLOCKS_PER_SEC;
        printf("%d;%llu;%.3f;%llu;%s\n", len, space, ms, found, target);
    }
    return 0;
}
