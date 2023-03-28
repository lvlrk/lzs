#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>
#include "lzss.h"
#include "types.h"

typedef struct {
	char magic[4];
	int zero;
	u32 zsize;
	u32 size;
} sszl_header_t;

char *eistat = "cant stat INFILE";
char *eostat = "cant stat OUTFILE";
char *eiopen = "cant open INFILE";
char *eoopen = "cant open OUTFILE";
char *eread = "cant read from INFILE";
char *ewrite = "cant write to OUTFILE";
char *ealloc = "cant allocate memory";
char *erinfile = "INFILE required";
char *eroutfile = "OUTFILE required";
char *essize = "size too small";
char *ldecompress = "lzs: got .lzs file; decompressing";
char *lcompress = "lzs: file not recognized; compressing";
char *larc = "lzs: looks like a .arc file";
char *wcorrupt = "lzs warning: file may be corrupt";

const char *usage = "usage: lzs [-oN]... [INFILE]\n"
	"     --help          show this help information and exit\n"
	"     --version       show version information and exit\n"
	" -o, --outfile       set output file\n"
	" -N, --nmm-style     replace extension on compressed OUTFILE\n";

int main(int argc, char **argv) {
	char *infile = NULL, *outfile = NULL, zmagic[4], 
		magic[4], *tmp = NULL,
		*match = NULL, *err = NULL;
	u8 *zdata = NULL, *data = NULL;
	struct stat st;
	FILE *fp = NULL;
	sszl_header_t sh;
	int outfile_set = 0, nmm_style = 0, err_set = 0;

	if(argc < 2) {
		err_set = 1;
		goto quit;
	}

	infile = malloc(256);
	outfile = malloc(256);

	for(int i = 0; i < argc; i++) {
		if(!strcmp(argv[i], "--help")) {
			fputs(usage, stdout);
			return 0;
		} else if(!strcmp(argv[i], "--version")) {
			fputs("lzs-1.1 lvlrk\n", stdout);
			return 0;
		} else if(!strcmp(argv[i], "-N") || !strcmp(argv[i], "--nmm-style"))
			nmm_style = 1;
		else if(argv[i][0] != '-') {
			if(i > 0) {
				if(argv[i - 1][0] != '-') {
					if(stat(argv[i], &st) != -1) {
						strncpy(infile, argv[i], 128);
					} else {
						err_set = 1;
						err = eistat;
						goto quit;
					}
				}
			}
		} else if(!strcmp(argv[i], "-o") || !strcmp(argv[i], "--outfile")) {
			if(argv[i + 1]) {
				outfile_set = 1;

				strncpy(outfile, argv[i + 1], 128);
			} else {
				err_set = 1;
				err = eroutfile;
				goto quit;
			}
		}
	}

	if(!infile) {
		err_set = 1;
		err = erinfile;
		goto quit;
	}

	fp = fopen(infile, "rb");
	if(!fp) {
		err_set = 1;
		err = eiopen;
		goto quit;
	}
	
	if(fread(zmagic, 1, 4, fp) != 4) {
		err_set = 1;
		err = eread;
		goto quit;
	}

	fseek(fp, 0, SEEK_SET);
	
	if(!memcmp(zmagic, "SSZL", 4)) {
		puts(ldecompress);
		
		if(fread(&sh, 1, 16, fp) != 16) {
			err_set = 1;
			err = eread;
			goto quit;
		}

		if(sh.zero || (sh.zsize > sh.size) || !sh.zsize || !sh.size)
			puts(wcorrupt);

		zdata = malloc(sh.zsize);
		if(!zdata) {
			err_set = 1;
			err = ealloc;
			goto quit;
		}

		if(fread(zdata, 1, sh.zsize, fp) != sh.zsize) {
			err_set = 1;
			err = eread;
			goto quit;
		}

		fclose(fp);

		data = malloc(sh.size);
		if(!data) {
			err_set = 1;
			err = ealloc;
			goto quit;
		}

		lzss_decompress(zdata, sh.zsize, data, sh.size, lzss0);

		if(!outfile_set) {
			tmp = basename(infile);
			match = strstr(tmp, ".lzs");
			
			memcpy(magic, data, 4);
			if(!memcmp(magic, "VCRA", 4)) {
				puts(larc);

				strncpy(outfile, tmp, 128);

				if(!match) {
					err_set = 1;
					err = eroutfile;
					goto quit;
				} else {
					outfile[strlen(tmp) -
						strlen(match)] = 0;
				}
				
				strncat(outfile, ".arc", 5);
			} else {	
				strncpy(outfile, tmp, 128);

				if(!match) {
					err_set = 1;
					err = eroutfile;
					goto quit;
				} else {
					outfile[strlen(tmp) -
						strlen(match)] = 0;
				}
			}
		}
		fp = fopen(basename(outfile), "wb");
		if(!fp) {
			err_set = 1;
			err = eoopen;
			goto quit;
		}

		if(fwrite(data, 1, sh.size, fp) != sh.size) {
			err_set = 1;
			err = ewrite;
			goto quit;
		}
			
		fclose(fp);
	} else {
		puts(lcompress);
		
		memcpy(sh.magic, "SSZL", 4);
		sh.zero = 0;

		fseek(fp, 0, SEEK_END);
		sh.size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if(sh.size < 4) {
			err_set = 1;
			err = essize;
			goto quit;
		}

		data = malloc(sh.size);
		if(!data) {
			err_set = 1;
			err = ealloc;
			goto quit;
		}

		if(fread(data, 1, sh.size, fp) != sh.size) {
			err_set = 1;
			err = eread;
			goto quit;
		}

		fclose(fp);

		zdata = malloc(sh.size + 1);
		if(!zdata) {
			err_set = 1;
			err = ealloc;
			goto quit;
		}

		sh.zsize = lzss_compress(data, sh.size, zdata, sh.size + 1, lzss0);
		
		if(sh.zsize >= sh.size) {
			err_set = 1;
			err = essize;
			goto quit;
		}

		if(!outfile_set) {
			strncpy(outfile, infile, 128);
			if(nmm_style)
				outfile[strlen(outfile) - strlen(strrchr(outfile, '.'))] = 0;
			strncat(outfile, ".lzs", 5);
		}

		fp = fopen(outfile, "wb");
		if(!fp) {
			err_set = 1;
			err = eoopen;
			goto quit;
		}

		if(fwrite(&sh, 1, 16, fp) != 16) {
			err_set = 1;
			err = ewrite;
			goto quit;
		}

		if(fwrite(zdata, 1, sh.zsize, fp) != sh.zsize) {
			err_set = 1;
			err = ewrite;
			goto quit;
		}

		fclose(fp);
	}
	
	return 0;

quit:
	//if(fp != NULL)
		//fclose(fp);

	if(infile)
		free(infile);

	if(outfile)
		free(outfile);

	if(zdata)
		free(zdata);

	if(data)
		free(data);
		
	if(err != NULL)
		fprintf(stderr, "lzs error: %s\n", err);
		
	fputs(usage, stderr);
		
	return err_set;
}
