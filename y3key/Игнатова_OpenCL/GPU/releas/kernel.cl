
#define uchar unsigned char
#define uint unsigned int
#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - (c)) ++b; a += c;
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))
#pragma OPENCL EXTENSION cl_khr_fp16 : enable










__kernel void Counting(__global char* hashh, int size, int ii) {

	
	char* sha256;
	char alfsixteen[17]="0123456789abcdef";
	uint a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
	
	uchar ctx__data[64];
	uint ctx__datalen;
	uint ctx__bitlen[2];
	uint ctx__state[8];
	
	//SHA256_CTX ctx__;
	
	uint k[64] = {
		0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
		0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
		0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
		0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
		0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
		0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
		0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
		0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
	};
		const int iiii = get_global_id(0);
	const int jjjj = get_global_id(1);
	int nomer = iiii*256+jjjj + ii*65536;
	//printf("%s\n", "z t,fk");
	char str[10] = "\0\0\0\0\0\0\0\0\0\0\0\0";
	char stri[10] = "\0\0\0\0\0\0\0\0\0\0\0\0";
	
	int bbb = nomer;
	char str_t[100] = "\0\0\0\0\0\0\0\0\0\0\0\0";
	int ost = 0;
	int len = 0;
	while (bbb != 0) {
		ost = bbb % 256;
		//printf("%d", ost);
		str_t[len] = ost;
		//printf("%c\n", str_t[0]);
		len++;
		bbb = bbb / 256;
	}
	//printf("!!\n");
	for (int iq = 0; iq < len; iq++) {
		str[iq] = str_t[len - iq - 1];
		//printf("%d", str[q]);
	}
	str[len] = '\0';
	//printf("%c%c%c end\n", str[0], str[1], str[2]);
	
	
	unsigned char hash[32];
	char hashStr[65] = "";
	
	
	//SHA256Init(&ctx);
	ctx__datalen = 0;
	ctx__bitlen[0] = 0;
	ctx__bitlen[1] = 0;
	ctx__state[0] = 0x6a09e667;
	ctx__state[1] = 0xbb67ae85;
	ctx__state[2] = 0x3c6ef372;
	ctx__state[3] = 0xa54ff53a;
	ctx__state[4] = 0x510e527f;
	ctx__state[5] = 0x9b05688c;
	ctx__state[6] = 0x1f83d9ab;
	ctx__state[7] = 0x5be0cd19;


	//SHA256Update(&ctx, data, len);
	for (uint ia = 0; ia < len; ++ia) {
		ctx__data[ctx__datalen] = str[ia];
		ctx__datalen++;
		if (ctx__datalen == 64) {
			





	
			for (i = 0, j = 0; i < 16; ++i, j += 4)
				m[i] = (ctx__data[j] << 24) | (ctx__data[j + 1] << 16) | (ctx__data[j + 2] << 8) | (ctx__data[j + 3]);
			for (; i < 64; ++i)
				m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
	
			a = ctx__state[0];
			b = ctx__state[1];
			c = ctx__state[2];
			d = ctx__state[3];
			e = ctx__state[4];
			f = ctx__state[5];
			g = ctx__state[6];
			h = ctx__state[7];
	
			for (i = 0; i < 64; ++i) {
				t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
				t2 = EP0(a) + MAJ(a, b, c);
				h = g;
				g = f;
				f = e;
				e = d + t1;
				d = c;
				c = b;
				b = a;
				a = t1 + t2;
			}

			ctx__state[0] += a;
			ctx__state[1] += b;
			ctx__state[2] += c;
			ctx__state[3] += d;
			ctx__state[4] += e;
			ctx__state[5] += f;
			ctx__state[6] += g;
			ctx__state[7] += h;


			DBL_INT_ADD(ctx__bitlen[0], ctx__bitlen[1], 512);
			ctx__datalen = 0;
		}
	}
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	

	//SHA256Final(&ctx, hash);
	uint is = ctx__datalen;

	if (ctx__datalen < 56) {
		ctx__data[is++] = 0x80;
		while (is < 56)
			ctx__data[is++] = 0x00;
	}
	else {
		ctx__data[is++] = 0x80;
		while (is < 64)
			ctx__data[is++] = 0x00;





		for (i = 0, j = 0; i < 16; ++i, j += 4)
			m[i] = (ctx__data[j] << 24) | (ctx__data[j + 1] << 16) | (ctx__data[j + 2] << 8) | (ctx__data[j + 3]);
		for (; i < 64; ++i)
			m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

		a = ctx__state[0];
		b = ctx__state[1];
		c = ctx__state[2];
		d = ctx__state[3];
		e = ctx__state[4];
		f = ctx__state[5];
		g = ctx__state[6];
		h = ctx__state[7];
	
		for (i = 0; i < 64; ++i) {
			t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
			t2 = EP0(a) + MAJ(a, b, c);
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}
	
		ctx__state[0] += a;
		ctx__state[1] += b;
		ctx__state[2] += c;
		ctx__state[3] += d;
		ctx__state[4] += e;
		ctx__state[5] += f;
		ctx__state[6] += g;
		ctx__state[7] += h;





		for (int zxc = 0; zxc < 56; zxc++){
			ctx__data[zxc] = 0;
		}
		//memset(ctx__data, 0, 56);
	}

	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	/////////////////////////////////////////
	 
	DBL_INT_ADD(ctx__bitlen[0], ctx__bitlen[1], ctx__datalen * 8);
	ctx__data[63] = ctx__bitlen[0];
	ctx__data[62] = ctx__bitlen[0] >> 8;
	ctx__data[61] = ctx__bitlen[0] >> 16;
	ctx__data[60] = ctx__bitlen[0] >> 24;
	ctx__data[59] = ctx__bitlen[1];
	ctx__data[58] = ctx__bitlen[1] >> 8;
	ctx__data[57] = ctx__bitlen[1] >> 16;
	ctx__data[56] = ctx__bitlen[1] >> 24;
	





	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (ctx__data[j] << 24) | (ctx__data[j + 1] << 16) | (ctx__data[j + 2] << 8) | (ctx__data[j + 3]);
	for (; i < 64; ++i)
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

	a = ctx__state[0];
	b = ctx__state[1];
	c = ctx__state[2];
	d = ctx__state[3];
	e = ctx__state[4];
	f = ctx__state[5];
	g = ctx__state[6];
	h = ctx__state[7];

	for (i = 0; i < 64; ++i) {
		t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
		t2 = EP0(a) + MAJ(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx__state[0] += a;
	ctx__state[1] += b;
	ctx__state[2] += c;
	ctx__state[3] += d;
	ctx__state[4] += e;
	ctx__state[5] += f;
	ctx__state[6] += g;
	ctx__state[7] += h;







	for (is = 0; is < 4; ++is) {
		hash[is] = (ctx__state[0] >> (24 - is * 8)) & 0x000000ff;
		hash[is + 4] = (ctx__state[1] >> (24 - is * 8)) & 0x000000ff;
		hash[is + 8] = (ctx__state[2] >> (24 - is * 8)) & 0x000000ff;
		hash[is + 12] = (ctx__state[3] >> (24 - is * 8)) & 0x000000ff;
		hash[is + 16] = (ctx__state[4] >> (24 - is * 8)) & 0x000000ff;
		hash[is + 20] = (ctx__state[5] >> (24 - is * 8)) & 0x000000ff;
		hash[is + 24] = (ctx__state[6] >> (24 - is * 8)) & 0x000000ff;
		hash[is + 28] = (ctx__state[7] >> (24 - is * 8)) & 0x000000ff;
	}





	//printf("%c%c%c end  %c%c%c%c%c%c endhash\n", str[0], str[1], str[2], hash[0], hash[1], hash[2], hash[3], hash[4], hash[5]);

	char s[3];
	for (int ias = 0; ias < 32; ias++) {
		//sprintf(s, "%02x", hash[i]);
		int dell =  hash[ias]/16;
		int proc = hash[ias]%16;
		//printf("%d end d\n", dell);
		hashStr[2*ias] = alfsixteen[dell];
		hashStr[2*ias+1] = alfsixteen[proc];
	}
	//printf("%c%c%c end   %c%c%c%c%c%c%c%c   endhash\n",str[0], str[1], str[2], hashStr[0], hashStr[1],hashStr[2], hashStr[3], hashStr[4], hashStr[5], hashStr[6], hashStr[7]);
	int flag = 1;
	for (int cmp = 0; cmp < 64; cmp++){
		if (hashh[cmp]!=hashStr[cmp]){
			flag=0;
			cmp=100;
		}
	}
	if (flag){
			hashh[70] = '1';
			hashh[71] = str[0];
			hashh[72] = str[1];
			hashh[73] = str[2];
			hashh[74] = str[3];
			hashh[75] = str[4];
			}
	
}



