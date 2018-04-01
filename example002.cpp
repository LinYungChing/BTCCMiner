//***********************************************************************************
// 2018.04.01 created by Zexlus1126
//
//    Example 002
// This is a simple demonstration on calculating merkle root from merkle branch 
// and solving a block (#286819) which the information is downloaded from Block Explorer 
//***********************************************************************************

#include <iostream>
#include <string>

#include <cstdio>
#include <cstring>

#include <cassert>

#include "sha256.h"

#include "offline_demo.mkrt" //merkle branch data (#286819)

typedef struct _block
{
    unsigned int version;
    unsigned char prevhash[32];
    unsigned char merkle_root[32];
    unsigned int ntime;
    unsigned int nbits;
    unsigned int nonce;
    unsigned char padding[];
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

// calculate merkle root from several merkle branches
// sha256_ctx: output hash will store here (little-endian)
// branch: merkle branch  (big-endian)
// count: total number of merkle branch
// coinbase: hashed coinbase (little-endian)
void calc_merkle_root(unsigned char *root, unsigned char branch[][65], size_t count, 
                        unsigned char *hashed_coinbase)
{
    size_t total_count = count + 1; // merkle branch + coinbase
    unsigned char *raw_list = new unsigned char[(total_count+1)*32];
    unsigned char **list = new unsigned char*[total_count+1];
    
    list[0] = raw_list;
    
    // copy hashed_coinbase to the first element of the merkle branch list
    memcpy(list[0], hashed_coinbase, 32);

    // copy each branch to the list
    for(int i=1;i<total_count; ++i)
    {
        list[i] = raw_list + i * 32;
        convert_string_to_byte(list[i], branch[i-1], 64);  //convert hex string to bytes array and store them to the list
        swap_endian(list[i], 32);  //big-endian to little-endian
    }

    list[total_count] = raw_list + total_count*32;


    // calculate merkle root
    while(total_count > 1)
    {
        
        // hash each pair
        int i, j;
        for(i=0, j=0;i<total_count-1;i+=2, ++j)
        {
            // this part is slightly tricky,
            //   because of the implementation of the double_sha256,
            //   we can avoid the memory begin overwritten during our sha256d calculation
            // double_sha:
            //     tmp = hash(list[0]+list[1])
            //     list[0] = hash(tmp)
            double_sha256((SHA256*)list[j], list[i], 64);
        }

        if(total_count % 2 == 1)  //odd,
        {
            memcpy(list[i+1], list[i], 32);
            double_sha256((SHA256*)list[j], list[i], 64);
            ++j;
        }
        total_count = j;
    }

    memcpy(root, list[0], 32);

    delete[] raw_list;
    delete[] list;
}

int main(int argc, char **argv)
{

    // ******** Calculate Merkle Root *********

    //unsigned char hashed_coinbase[65];
    //unsigned char merkle_branch[98][65];
    size_t branch_count = 98;
    unsigned char coinbase_bin[32];
    unsigned char merkle_root[32];

    // convert coinbase to byte & swap endian
    convert_string_to_byte(coinbase_bin, hashed_coinbase, 64);
    swap_endian(coinbase_bin, 32);

    // calculate merkle root
    calc_merkle_root(merkle_root, merkle_branch, branch_count, coinbase_bin);
    
    // print little-endian
    printf("merkle root(little): ");
    print_hex(merkle_root, 32);
    printf("\n");

    swap_endian(merkle_root, 32);

    // print big-endian
    printf("merkle root(big):    ");
    print_hex(merkle_root, 32);
    printf("\n\n");


    
    
    // ********* Same as Example001 **********
    
    
    // Block #286819
    // from Block EXplorer
    unsigned char prevhash[] = "000000000000000117c80378b8da0e33559b5997f2ad55e2f7d18ec1975b9717";  //big-endian
    //unsigned char merkle_root[] = "871714dcbae6c8193a2bb9b2a69fe1c0440399f38d94b3a0f1b447275a29978a";  //big-endian
    unsigned char version[] = "00000002";  //big-endian
    unsigned char nbits[] =   "19015f53";  //big-endian
    unsigned char ntime[] =   "53058b35";  //big-endian  //2014-02-20 04:57:25
    unsigned char nonce[] =   "33087548";  //big-endian
    
    //print block info
    printf("Block info (big): \n");
    printf("  veresion:   %s\n", version);
    printf("  prevhash:   %s\n", prevhash);
    printf("  merkleroot: "); print_hex(merkle_root, 32); printf("\n");
    printf("  nbits:      %s\n", nbits);
    printf("  ntime:      %s\n", ntime);
    printf("  nonce:      %s\n", nonce);
    printf("\n");
    
    HashBlock block;

    // convert to little endian
    convert_string_to_byte((unsigned char *)&block.version, version,   8);
    convert_string_to_byte(block.prevhash,                  prevhash,    64);
    memcpy(block.merkle_root, merkle_root, 32);
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
    printf("Block #286819\n");
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
                "0000000000000000e067a478024addfecdc93628978aa52d91fabd4292982a50\n\n");

    return 0;
}

