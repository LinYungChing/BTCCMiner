#ifndef __SHA256_HEADER__
#define __SHA256_HEADER__

#include <stddef.h>

#ifdef __cplusplus
extern "C"{
#endif  //__cplusplus



typedef unsigned int WORD;
typedef unsigned char BYTE;

typedef union _sha256_ctx{
	WORD h[8];
	BYTE hash8[32];
}SHA256;

void sha256_transform(SHA256 *ctx, const BYTE *msg);
void sha256(SHA256 *ctx, const BYTE *msg, size_t len);


#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //__SHA256_HEADER__
