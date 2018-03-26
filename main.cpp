#include <iostream>
#include <cstdio>
#include <cstring>

#include "sha256.h"

int main(int argc, char **argv)
{
	#define print_hash(x) printf("sha256 hash: "); for(int i=0;i<32;++i)printf("%02X", (x).hash8[i]);
	#define print_msg(x) printf("%s", ((x) ? "Pass":"Failed"))
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
	print_msg(!memcmp(abcans, ctx.hash8, 32));
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
	print_msg(!memcmp(len55ans, ctx.hash8, 32));
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
	print_msg(!memcmp(len290ans, ctx.hash8, 32));
	printf("\n\n");
	
	return 0;
}
