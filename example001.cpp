
//***********************************************************************************
// 2018.04.01 created by Zexlus1126
//
// This is a simple demonstration on solving a block (#515961) from Block Explorer
//***********************************************************************************

#include <iostream>
#include <string>

#include <cstdio>
#include <cstring>

#include <cassert>

#include "sha256.h"


typedef struct _block
{
    unsigned int version;
    unsigned char prevhash[32];
    unsigned char merkle_root[32];
    unsigned int ntime;
    unsigned int nbits;
    unsigned int nonce;
}HashBlock;

//convert big/little endian
void swap_endian(unsigned char* byte, size_t len)
{
    for(size_t c = 0;c<len/2;++c)
    {
        byte[c] ^= byte[len-(c+1)];
        byte[len-(c+1)] ^= byte[c];
        byte[c] ^= byte[len-(c+1)];
    }
}

//convert one hex-codec char to binary
unsigned char decode(unsigned char c)
{
    switch(c)
    {
        case 'a':
            return 0x0a;
        case 'b':
            return 0x0b;
        case 'c':
            return 0x0c;
        case 'd':
            return 0x0d;
        case 'e':
            return 0x0e;
        case 'f':
            return 0x0f;
        case '0' ... '9':
            return c-'0';
    }
}


// convert hex string to binary
//
// in: input string
// string_len: the length of the input string
//      '\0' is not included in string_len!!!
// out: output bytes array
void convert_string_to_byte(unsigned char* out, unsigned char *in, size_t string_len)
{
    for(size_t s = 0, b = 0;s < string_len; s+=2, ++b)
    {
        out[b] = (unsigned char)(decode(in[s])<<4) + decode(in[s+1]);
    }
}

// print out binary array in the hex format
void print_hex(unsigned char* hex, size_t len)
{
    for(int i=0;i<len;++i)
    {
        printf("%02x", hex[i]);
    }
}

void double_sha256(SHA256 *sha256_ctx, unsigned char *bytes, size_t len)
{
    SHA256 tmp;
    sha256(&tmp, (BYTE*)bytes, len);
    sha256(sha256_ctx, (BYTE*)&tmp, sizeof(tmp));
}


int main(int argc, char **argv)
{
    // Block #515961
    // from Block Explorer
    unsigned char prevhash[] = "0000000000000000000aa3552278554fe86c84e75082f34235eccf8da8c1f5c8";  //big-endian
    unsigned char merkle_root[] = "81ae217a19ce1eb4b37fce79d29418ba8949f96ebe3867629ffa55cae3078903";  //big-endian
    unsigned char version[] = "20000000";  //big-endian
    unsigned char nbits[] = "17514a49";  //big-endian
    unsigned char ntime[] = "5abf51eb";  //big-endian  //2018-03-31 09:16:27
    unsigned char nonce[] = "ea87847b";  //big-endian

    //print block info
    printf("Block info (big): \n");
    printf("veresion:   %s\n", version);
    printf("prevhash:   %s\n", prevhash);
    printf("merkleroot: %s\n", merkle_root);
    printf("nbits:      %s\n", nbits);
    printf("ntime:      %s\n", ntime);
    printf("nonce:      %s\n", nonce);
    printf("\n");


    // Create Block
    HashBlock block;

    // convert to little endian
    convert_string_to_byte((unsigned char *)&block.version, version,   8);
    convert_string_to_byte(block.prevhash,                  prevhash,    64);
    convert_string_to_byte(block.merkle_root,               merkle_root, 64);
    convert_string_to_byte((unsigned char *)&block.nbits,   nbits,     8);
    convert_string_to_byte((unsigned char *)&block.ntime,   ntime,     8);
    convert_string_to_byte((unsigned char *)&block.nonce,   nonce,     8);

    // back up 
    HashBlock block_lit;
    memcpy(&block_lit, &block, sizeof(block));

    // convert all field from big endian to little endian
    swap_endian((unsigned char*)&block_lit.version, 4);
    swap_endian((unsigned char*) block_lit.prevhash, 32);
    swap_endian((unsigned char*) block_lit.merkle_root, 32);
    swap_endian((unsigned char*)&block_lit.ntime, 4);
    swap_endian((unsigned char*)&block_lit.nbits, 4);
    swap_endian((unsigned char*)&block_lit.nonce, 4);

    
    SHA256 sha256_ctx;

    //sha256d
    double_sha256(&sha256_ctx, (unsigned char*)&block_lit, sizeof(block_lit));


    // print result
    printf("Block #515961\n");
    //little-endian
    printf("hash(little): ");
    print_hex(sha256_ctx.b, 32);
    printf("\n");

    swap_endian(sha256_ctx.b, sizeof(sha256_ctx));

    //big-endian
    printf("hash(big):    ");
    print_hex(sha256_ctx.b, 32);
    printf("\n\n");
    printf("Block Explorer: \n");
    printf("https://blockexplorer.com/block/"
            "0000000000000000001d994821187aeb615a7eff7b2d2554400a6301667db68d\n\n");
    
    return 0;
}
