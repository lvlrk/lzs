#ifndef _LZSS_H
#define _LZSS_H

typedef unsigned char u8;

typedef struct {
	int EI;
	int EJ;
	int P;
	int rless;
	int init_chr;
	int THRESHOLD;
} lzss_config;

static const lzss_config lzss0 = { 12, 4, 2, 2, 0, 2 };
static const lzss_config lzss = { 12, 4, 2, 2, ' ', 2 };
static const lzss_config okumura = { 11, 4, 1, 1, ' ', 2 };
static const lzss_config lz770 = { 12, 4, 2, 2, ' ', 0 };
static const lzss_config lz77 = { 12, 4, 2, 2, 0, 0 };

int lzss_decompress(unsigned char *src, int srclen, unsigned char *dst, int dstlen, lzss_config cfg);
int lzss_compress(u8 *in, int insz, u8 *out, int outsz, lzss_config cfg);

#endif