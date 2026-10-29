__constant uint MD5_K[64] = {
    0xd76aa478U,0xe8c7b756U,0x242070dbU,0xc1bdceeeU,0xf57c0fafU,0x4787c62aU,0xa8304613U,0xfd469501U,
    0x698098d8U,0x8b44f7afU,0xffff5bb1U,0x895cd7beU,0x6b901122U,0xfd987193U,0xa679438eU,0x49b40821U,
    0xf61e2562U,0xc040b340U,0x265e5a51U,0xe9b6c7aaU,0xd62f105dU,0x02441453U,0xd8a1e681U,0xe7d3fbc8U,
    0x21e1cde6U,0xc33707d6U,0xf4d50d87U,0x455a14edU,0xa9e3e905U,0xfcefa3f8U,0x676f02d9U,0x8d2a4c8aU,
    0xfffa3942U,0x8771f681U,0x6d9d6122U,0xfde5380cU,0xa4beea44U,0x4bdecfa9U,0xf6bb4b60U,0xbebfbc70U,
    0x289b7ec6U,0xeaa127faU,0xd4ef3085U,0x04881d05U,0xd9d4d039U,0xe6db99e5U,0x1fa27cf8U,0xc4ac5665U,
    0xf4292244U,0x432aff97U,0xab9423a7U,0xfc93a039U,0x655b59c3U,0x8f0ccc92U,0xffeff47dU,0x85845dd1U,
    0x6fa87e4fU,0xfe2ce6e0U,0xa3014314U,0x4e0811a1U,0xf7537e82U,0xbd3af235U,0x2ad7d2bbU,0xeb86d391U
};
__constant uint MD5_S[64] = {
    7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
    5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
    4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
    6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
};

inline uint md5_rotl(const uint x, const uint s) {
    return rotate(x, s);
}

inline void md5_candidate(const __private uchar* candidate, const uint length, __private uint* digest) {
    uint M[16];
    for (uint j = 0; j < 16; ++j) M[j] = 0U;
    for (uint j = 0; j < length; ++j) {
        M[j / 4U] |= ((uint)candidate[j]) << (8U * (j % 4U));
    }
    M[length / 4U] |= 0x80U << (8U * (length % 4U));
    M[14] = length * 8U;
    M[15] = 0U;

    uint a = 0x67452301U;
    uint b = 0xefcdab89U;
    uint c = 0x98badcfeU;
    uint d = 0x10325476U;
    const uint initial_a = a, initial_b = b, initial_c = c, initial_d = d;

    for (uint i = 0; i < 64U; ++i) {
        uint f;
        uint g;
        if (i < 16U) {
            f = (b & c) | ((~b) & d);
            g = i;
        } else if (i < 32U) {
            f = (d & b) | ((~d) & c);
            g = (5U * i + 1U) % 16U;
        } else if (i < 48U) {
            f = b ^ c ^ d;
            g = (3U * i + 5U) % 16U;
        } else {
            f = c ^ (b | (~d));
            g = (7U * i) % 16U;
        }
        const uint old_d = d;
        d = c;
        c = b;
        b = b + md5_rotl(a + f + MD5_K[i] + M[g], MD5_S[i]);
        a = old_d;
    }

    digest[0] = initial_a + a;
    digest[1] = initial_b + b;
    digest[2] = initial_c + c;
    digest[3] = initial_d + d;
}

__kernel void md5_search(__global const uchar* alphabet,
                         const uint alphabet_size,
                         __global const uint* target,
                         const uint length,
                         const ulong base_index,
                         const ulong total,
                         __global int* found_state) {
    const ulong gid = (ulong)get_global_id(0);
    const ulong index = base_index + gid;
    if (gid >= total) return;

    ulong rest = index;
    uchar candidate[16];
    for (int pos = (int)length - 1; pos >= 0; --pos) {
        const uint digit = (uint)(rest % (ulong)alphabet_size);
        rest /= (ulong)alphabet_size;
        candidate[pos] = alphabet[digit];
    }

    uint digest[4];
    md5_candidate(candidate, length, digest);
    if (digest[0] == target[0] && digest[1] == target[1]
        && digest[2] == target[2] && digest[3] == target[3]) {
        if (atomic_cmpxchg((volatile __global int*)&found_state[0], 0, 1) == 0) {
            found_state[1] = (int)(index & 0xffffffffUL);
            found_state[2] = (int)(index >> 32U);
        }
    }
}
