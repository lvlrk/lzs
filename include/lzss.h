#ifndef _LZSS_H
#define _LZSS_H

typedef unsigned char u8;

typedef struct {
	int EI;
	int EJ;
	int P;
	int rless;
	int init_chr;
} config_t;

static const config_t lzss0 = { 12, 4, 2, 2, 0 };
static const config_t lzss = { 12, 4, 2, 2, ' ' };
static const config_t okumura = { 11, 4, 1, 1, ' ' };

int lzss_decompress(unsigned char *src, int srclen, unsigned char *dst, int dstlen, config_t cfg);
int lzss_compress(u8 *in, int insz, u8 *out, int outsz, config_t cfg);

#endif