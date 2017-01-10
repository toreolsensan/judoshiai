#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha.h"

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

static void encodeblock( unsigned char *in, unsigned char *out, int len )
{
    out[0] = (unsigned char) cb64[ (int)(in[0] >> 2) ];
    out[1] = (unsigned char) cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ (int)(in[2] & 0x3f) ] : '=');
}

static void decodeblock( unsigned char *in, unsigned char *out )
{
    out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

//https://tools.ietf.org/html/rfc6455
#define WEBSOCK_STR "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

int main(int argc, char *argv[])
{
	int i = 0;
	unsigned char buf[128], out[4];
	SHA_CTX context;
	sha1_byte digest[SHA1_DIGEST_LENGTH];

	SHA1_Init(&context);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s%s", argv[1], WEBSOCK_STR);
	//strcpy(buf, "2BA+jZj+o/ZzBjy/xr7CRw==" WEBSOCK_STR);
	SHA1_Update(&context, buf, strlen(buf));
	SHA1_Final(digest, &context);

	for (i = 0; i < SHA1_DIGEST_LENGTH; i++)
		printf(" %02x", digest[i]);
	printf("\n");

	for (i = 0; i < SHA1_DIGEST_LENGTH; i += 3) {
		encodeblock(&digest[i], out, SHA1_DIGEST_LENGTH - i);
		printf("%c%c%c%c", out[0], out[1], out[2], out[3]);
	}
	printf("\n");
}
