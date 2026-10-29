// kernel_bf.cl - параллельный перебор паролей и вычисление SHA-256 на GPU.
// Каждый work-item формирует собственного кандидата по своему глобальному индексу
// (get_global_id), вычисляет для него SHA-256 и сравнивает с искомым хэшем.

#define uchar unsigned char
#define uint  unsigned int
#define ulong unsigned long

#define ROTR(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22))
#define EP1(x) (ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25))
#define SIG0(x) (ROTR(x,7) ^ ROTR(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x,17) ^ ROTR(x,19) ^ ((x) >> 10))

__constant uint k_const[64] = {
0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

// Вычисление SHA-256 одного блока данных (сообщение дополняется битом 1, нулями и длиной)
void sha256(const uchar* msg, int len, uchar* hashOut) {
    uint state[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
                      0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    uchar block[64];
    for (int i = 0; i < len; i++) block[i] = msg[i];
    block[len] = 0x80;
    for (int i = len + 1; i < 56; i++) block[i] = 0;
    ulong bitlen = (ulong)len * 8;
    for (int i = 0; i < 8; i++) block[63 - i] = (uchar)(bitlen >> (i * 8));

    uint m[64];
    for (int i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (block[j] << 24) | (block[j+1] << 16) | (block[j+2] << 8) | block[j+3];
    for (int i = 16; i < 64; ++i)
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    uint a=state[0],b=state[1],c=state[2],d=state[3],e=state[4],f=state[5],g=state[6],h=state[7];
    for (int i = 0; i < 64; ++i) {
        uint t1 = h + EP1(e) + CH(e,f,g) + k_const[i] + m[i];
        uint t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    state[0]+=a; state[1]+=b; state[2]+=c; state[3]+=d;
    state[4]+=e; state[5]+=f; state[6]+=g; state[7]+=h;

    for (int i = 0; i < 8; i++) {
        hashOut[i*4+0] = (state[i] >> 24) & 0xff;
        hashOut[i*4+1] = (state[i] >> 16) & 0xff;
        hashOut[i*4+2] = (state[i] >> 8) & 0xff;
        hashOut[i*4+3] = state[i] & 0xff;
    }
}

// targetHash - 32 байта искомого хэша; passLen - длина перебираемого пароля;
// found - флаг/индекс найденного кандидата (атомарно записывается первым нашедшим потоком)
__kernel void bruteforce(__global const uchar* targetHash,
                          const int passLen,
                          __global volatile long* foundIndex,
                          const ulong offset) {
    ulong gid = offset + get_global_id(0) + get_global_id(1) * get_global_size(0);

    __constant char alphabet[36] = "0123456789abcdefghijklmnopqrstuvwxyz";
    uchar candidate[16];
    ulong idx = gid;
    for (int i = passLen - 1; i >= 0; i--) {
        candidate[i] = alphabet[idx % 36];
        idx /= 36;
    }

    uchar hash[32];
    sha256(candidate, passLen, hash);

    bool match = true;
    for (int i = 0; i < 32; i++) if (hash[i] != targetHash[i]) { match = false; break; }
    if (match) *foundIndex = (long)gid;
}
