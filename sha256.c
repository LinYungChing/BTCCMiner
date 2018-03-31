
// This sha256 implementation is based on sha256 wiki page
// please refer to:
//     https://en.wikipedia.org/wiki/SHA-2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha256.h"

// circular shift - wiki:
//     https://en.wikipedia.org/wiki/Circular_shift
#define _rotl(v, s) ((v)<<(s) | (v)>>(32-(s)))
#define _rotr(v, s) ((v)>>(s) | (v)<<(32-(s)))

#define _swap(x, y) (((x)^=(y)), ((y)^=(x)), ((x)^=(y)))

#ifdef __cplusplus
extern "C"{
#endif  //__cplusplus

static const WORD k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void sha256_transform(SHA256 *ctx, const BYTE *msg)
{
	WORD a, b, c, d, e, f, g, h;
	WORD i, j;
	
	// Create a 64-entry message schedule array w[0..63] of 32-bit words
	WORD w[64];
	// Copy chunk into first 16 words w[0..15] of the message schedule array
	for(i=0, j=0;i<16;++i, j+=4)
	{
		w[i] = (msg[j]<<24) | (msg[j+1]<<16) | (msg[j+2]<<8) | (msg[j+3]);
	}
	
	// Extend the first 16 words into the remaining 48 words w[16..63] of the message schedule array:
	for(i=16;i<64;++i)
	{
		WORD s0 = (_rotr(w[i-15], 7)) ^ (_rotr(w[i-15], 18)) ^ (w[i-15]>>3);
		WORD s1 = (_rotr(w[i-2], 17)) ^ (_rotr(w[i-2], 19))  ^ (w[i-2]>>10);
		w[i] = w[i-16] + s0 + w[i-7] + s1;
	}
	
	
	// Initialize working variables to current hash value
	a = ctx->h[0];
	b = ctx->h[1];
	c = ctx->h[2];
	d = ctx->h[3];
	e = ctx->h[4];
	f = ctx->h[5];
	g = ctx->h[6];
	h = ctx->h[7];
	
	// Compress function main loop:
	for(i=0;i<64;++i)
	{
		WORD S0 = (_rotr(a, 2)) ^ (_rotr(a, 13)) ^ (_rotr(a, 22));
		WORD S1 = (_rotr(e, 6)) ^ (_rotr(e, 11)) ^ (_rotr(e, 25));
		WORD ch = (e & f) ^ ((~e) & g);
		WORD maj = (a & b) ^ (a & c) ^ (b & c);
		WORD temp1 = h + S1 + ch + k[i] + w[i];
		WORD temp2 = S0 + maj;
		
		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}
	
	// Add the compressed chunk to the current hash value
	ctx->h[0] += a;
	ctx->h[1] += b;
	ctx->h[2] += c;
	ctx->h[3] += d;
	ctx->h[4] += e;
	ctx->h[5] += f;
	ctx->h[6] += g;
	ctx->h[7] += h;
	
}

void sha256(SHA256 *ctx, const BYTE *msg, size_t len)
{
	// Initialize hash values:
	// (first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19):
	ctx->h[0] = 0x6a09e667;
	ctx->h[1] = 0xbb67ae85;
	ctx->h[2] = 0x3c6ef372;
	ctx->h[3] = 0xa54ff53a;
	ctx->h[4] = 0x510e527f;
	ctx->h[5] = 0x9b05688c;
	ctx->h[6] = 0x1f83d9ab;
	ctx->h[7] = 0x5be0cd19;
	
	
	WORD i, j;
	size_t remain = len % 64;
	size_t total_len = len - remain;
	
	// Process the message in successive 512-bit chunks
	// For each chunk:
	for(i=0;i<total_len;i+=64)
	{
		sha256_transform(ctx, &msg[i]);
	}
	
	// Process remain data
	BYTE m[64] = {};
	for(i=total_len, j=0;i<len;++i, ++j)
	{
		m[j] = msg[i];
	}
	
	// Append a single '1' bit
	m[j++] = 0x80;  //1000 0000
	
	// Append K '0' bits, where k is the minimum number >= 0 such that L + 1 + K + 64 is a multiple of 512
	if(j > 56)
	{
		sha256_transform(ctx, m);
		memset(m, 0, sizeof(m));
		printf("true\n");
	}
	
	// Append L as a 64-bit bug-endian integer, making the total post-processed length a multiple of 512 bits
	unsigned long long L = len * 8;  //bits
	m[63] = L;
	m[62] = L >> 8;
	m[61] = L >> 16;
	m[60] = L >> 24;
	m[59] = L >> 32;
	m[58] = L >> 40;
	m[57] = L >> 48;
	m[56] = L >> 56;
	sha256_transform(ctx, m);
	
	// Produce the final hash value (little-endian to big-endian)
	// Swap 1st & 4th, 2nd & 3rd byte for each word
	for(i=0;i<32;i+=4)
	{
        _swap(ctx->b[i], ctx->b[i+3]);
        _swap(ctx->b[i+1], ctx->b[i+2]);
	}
}

// Unit test
#ifdef __SHA256_UNITTEST__
	#define print_hash(x) printf("sha256 hash: "); for(int i=0;i<32;++i)printf("%02X", (x).b[i]);
	#define print_msg(x) printf("%s", ((x) ? "Pass":"Failed"))

int main(int argc, char **argv)
{
	SHA256 ctx;
	
	// ------------------ Stage 1: abc
	printf("------- Stage 1 : abc -------\n");
	BYTE abc[] = "abc";
	BYTE abcans[] = {0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA, 
					 0x41, 0x41, 0x40, 0xDE, 0x5D, 0xAE, 0x22, 0x23, 
					 0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C, 
					 0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD};
	size_t abclen = sizeof(abc) - 1;
	sha256(&ctx, abc, abclen);
	print_hash(ctx);
	printf("\nResult: ");
	print_msg(!memcmp(abcans, ctx.b, 32));
	printf("\n\n");
	
	// ------------------ Stage 2: len55
	printf("------ Stage 2 : len55 ------\n");
	BYTE len55[] = "1234567890123456789012345678901234567890123456789012345";
	BYTE len55ans[] = {0x03, 0xC3, 0xA7, 0x0E, 0x99, 0xED, 0x5E, 0xEC, 
					   0xCD, 0x80, 0xF7, 0x37, 0x71, 0xFC, 0xF1, 0xEC, 
					   0xE6, 0x43, 0xD9, 0x39, 0xD9, 0xEC, 0xC7, 0x6F, 
					   0x25, 0x54, 0x4B, 0x02, 0x33, 0xF7, 0x08, 0xE9};
	size_t len55len = sizeof(len55) - 1;
	sha256(&ctx, len55, len55len);
	print_hash(ctx);
	printf("\nResult: ");
	print_msg(!memcmp(len55ans, ctx.b, 32));
	printf("\n\n");
	
	// ------------------ Stage 3: len290
	printf("----- Stage 3 : len290 ------\n");
	BYTE len290[] = "ads;flkjas;dlkfjads;flkjads;flkafdlkjhfdalkjgadslfkjhadsjhfveroi"
					"uhwerpiuhwerptoiuywerptoiuywterypoihslgkjhdxzflgknbzsfdlkgjhsdfp"
					"gikjhwepgoiuhywertpiuywerptiuywrtoiuhwserlkjhsfdlgkjbsfd,nkmbxcv"
					".bkmnxflkjbnfdslgkjhsgpoiuhserpiuywerpituywetrpoiuhywerlkjbsfd,g"
					"nkbxsflkdjbsdflkjhsgfdluhsdgliuher";
	BYTE len290ans[] = {0xBD, 0xB5, 0xD4, 0xC1, 0xFB, 0x45, 0x1A, 0xD2, 
						0xFC, 0x8E, 0x62, 0x26, 0xF9, 0x5C, 0x6B, 0x58, 
						0x31, 0x53, 0x90, 0x1B, 0xE3, 0x74, 0xC2, 0x60, 
						0xC8, 0xA7, 0x46, 0x09, 0xC6, 0x89, 0x24, 0x60};
	size_t len290len = sizeof(len290) - 1;
	sha256(&ctx, len290, len290len);
	print_hash(ctx);
	printf("\nResult: ");
	print_msg(!memcmp(len290ans, ctx.b, 32));
	printf("\n\n");
	
	return 0;
}
#endif  //__SHA256_UNITTEST__

#ifdef __cplusplus
}
#endif  //__cplusplus

#undef _rotl
#undef _rotr
