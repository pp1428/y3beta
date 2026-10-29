// md5_kernel.cl
// Ядро OpenCL для брутфорса MD5.

// Перевод целого числа в строку (аналог sprintf)
int uint_to_str(ulong val, char *buf) {
    if (val == 0) {
        buf[0] = '0';
        return 1;
    }
    int len = 0;
    ulong temp = val;
    while (temp > 0) {
        temp /= 10;
        len++;
    }
    for (int i = len - 1; i >= 0; i--) {
        buf[i] = '0' + (val % 10);
        val /= 10;
    }
    return len;
}

// Быстрый MD5 для коротких строк (до 55 символов, влезает в один блок)
void md5_short(const char *msg, int len, uint *hash) {
    uint a = 0x67452301, b = 0xefcdab89, c = 0x98badcfe, d = 0x10325476;
    uint x[16] = {0};
    uchar block[64] = {0};

    for (int i = 0; i < len; i++) block[i] = msg[i];
    block[len] = 0x80;
    
    // Записываем длину в битах
    ulong bit_len = (ulong)len * 8;
    block[56] = (uchar)(bit_len & 0xFF);
    block[57] = (uchar)((bit_len >> 8) & 0xFF);
    block[58] = (uchar)((bit_len >> 16) & 0xFF);
    block[59] = (uchar)((bit_len >> 24) & 0xFF);

    for (int i = 0; i < 16; i++) {
        x[i] = ((uint)block[i*4]) | (((uint)block[i*4+1]) << 8) |
               (((uint)block[i*4+2]) << 16) | (((uint)block[i*4+3]) << 24);
    }

#define F(x,y,z) (((x) & (y)) | ((~x) & (z)))
#define G(x,y,z) (((x) & (z)) | ((y) & (~z)))
#define H(x,y,z) ((x) ^ (y) ^ (z))
#define I(x,y,z) ((y) ^ ((x) | (~z)))
#define ROTATE_LEFT(x,n) (((x) << (n)) | ((x) >> (32-(n))))
#define FF(a,b,c,d,x,s,ac) { (a) += F((b),(c),(d)) + (x) + (uint)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }
#define GG(a,b,c,d,x,s,ac) { (a) += G((b),(c),(d)) + (x) + (uint)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }
#define HH(a,b,c,d,x,s,ac) { (a) += H((b),(c),(d)) + (x) + (uint)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }
#define II(a,b,c,d,x,s,ac) { (a) += I((b),(c),(d)) + (x) + (uint)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }

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

    hash[0] = a + 0x67452301;
    hash[1] = b + 0xefcdab89;
    hash[2] = c + 0x98badcfe;
    hash[3] = d + 0x10325476;
}

__kernel void md5_bruteforce(__global const uint *target_hash,
                             const ulong start_idx,
                             const ulong max_range,
                             __global int *found_flag,
                             __global ulong *found_val) {
    ulong i = start_idx + get_global_id(0);
    if (i >= max_range) return;
    if (*found_flag) return; // Если другой поток уже нашел, выходим

    char buf[24];
    int len = uint_to_str(i, buf);
    uint hash[4];
    
    md5_short(buf, len, hash);

    // Сравниваем с целевым хэшем
    if (hash[0] == target_hash[0] && hash[1] == target_hash[1] &&
        hash[2] == target_hash[2] && hash[3] == target_hash[3]) {
        *found_flag = 1;
        *found_val = i;
    }
}
