/*

Copyright 2024 Japanese TeX Development Community <issue@texjp.org>
Distributed under the 3-Clause BSD License.

*/

#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <kpathsea/getopt.h>
#include <kpathsea/progname.h>
#include <kpathsea/lib.h>
#ifdef _WIN32
#include <kpathsea/knj.h>
#ifndef CP_UTF8
#define CP_UTF8 65001
#endif
#endif
#include <ptexenc/ptexenc.h>
#include <c-auto.h>

static char opt_input_enc[5];
static char opt_output_enc[5];
static int flg_guess_enc;
static int flg_output_buffer;

#define GUESS_ONLY        -1
#define GUESS_AND_OUTPUT  +1

#define BUFFERLEN 4096
static char buff[BUFFERLEN];

static const char *optstr = "hvgbjseuGJSEU";

static struct option long_options[] = {
  {"help",    0, 0, 'h'},
  {"version", 0, 0, 'v'},
  {"guess",   0, 0, 'g'},
  {"buffer",  0, 0, 'b'},
  {0, 0, 0, 0}
};

#define MY_VERSION   "20240201"
#define BUG_ADDRESS  "issue@texjp.org"

static void show_version(void)
{
  printf("ptekf  ver." MY_VERSION " (%s) (%s, %s)\n", get_enc_string(), ptexenc_version_string, TL_VERSION);
  printf("    Copyright (C) 2024 Japanese TeX Development Community\n");
}

static void show_usage(void)
{
  printf("Usage: ptekf -[OPTION] [--] in_file1 [in_file2 ...]\n");
  printf("  j/s/e/u  Specify output encoding ISO-2022-JP, Shift_JIS, EUC-JP, UTF-8\n");
  printf("  J/S/E/U  Specify input encoding ISO-2022-JP, Shift_JIS, EUC-JP, UTF-8\n");
  printf("  G        Guess the input encoding and output to stdout or files\n");
  printf("  --guess    -g  Guess the input encoding (no conversion)\n");
  printf("  --buffer   -b  Output internal buffer without code conversion\n");
  printf("  --version  -v  Print version number\n");
  printf("  --help     -h  Print this help\n");
  printf("Default input/output encoding depends on kpathsearch parameters PTEX_KANJI_ENC, guess_input_kanji_encoding\n");
  printf("\nEmail bug reports to %s.\n", BUG_ADDRESS);
}

const char  *infname;
char        *outfname;
FILE        *infp, *outfp;

#if defined(_WIN32)
#define R_MODE "rb"
#define W_MODE "wb"
#else
#define R_MODE "r"
#define W_MODE "w"
#endif

static char *mfgets(char *buff, int size, FILE *fp)
{
  int c, len;

  if ((len = input_line2(fp, (unsigned char *)buff, NULL, 0, size, &c)) == 0
      && c != '\r' && c != '\n') return NULL;
  if (c == '\n' || c == '\r') {
    if (len+1 < size) strcat(buff+len, "\n");
    else ungetc(c, fp);
  }
  if (c == EOF) return NULL;
  return buff;
}


int
main (int argc,  char **argv)
{
  int c;
  int opt_ienc=0, opt_oenc=0;
  int (*fputs__)(const char*, FILE*);

  if (argc<=1) {
    show_version();
    show_usage();
    return 0;
  }
  kpse_set_program_name(argv[0], "ptekf");
#if defined(_WIN32)
  {
    int ac;
    char **av;
    file_system_codepage = CP_UTF8;
    is_cp932_system = 0;
    if (get_command_line_args_utf8("utf-8", &ac, &av)) {
      argv = av;
      argc = ac;
    }
  }
#endif
  enable_UPTEX(true);

  while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
    switch (c) {

    case 'G': /* guess and output to stdout or files */
      flg_guess_enc = GUESS_AND_OUTPUT;
      opt_ienc++;
      break;

    case 'g': /* guess */
      flg_guess_enc = GUESS_ONLY;
      opt_ienc++;
      break;

    case 'b': /* output internal buffer */
      flg_output_buffer = 1;
      opt_oenc++;
      break;

    case 'h': /* help */
      show_version();
      show_usage();
      return 0;

    case 'v': /* version */
      show_version();
      return 0;

    /* input encoding */
    case 'J': strcpy(opt_input_enc,  "jis")  ; opt_ienc++; break;
    case 'S': strcpy(opt_input_enc,  "sjis") ; opt_ienc++; break;
    case 'E': strcpy(opt_input_enc,  "euc")  ; opt_ienc++; break;
    case 'U': strcpy(opt_input_enc,  "utf8") ; opt_ienc++; break;

    /* output encoding */
    case 'j': strcpy(opt_output_enc, "jis")  ; opt_oenc++; break;
    case 's': strcpy(opt_output_enc, "sjis") ; opt_oenc++; break;
    case 'e': strcpy(opt_output_enc, "euc")  ; opt_oenc++; break;
    case 'u': strcpy(opt_output_enc, "utf8") ; opt_oenc++; break;
    }
  }

  if (opt_ienc>1) {
    fprintf(stderr, "Option (--guess or -g), -G, -J, -S, -E and -U are mutually exclusive\n");
    exit(16);
  }
  if (opt_oenc>1) {
    fprintf(stderr, "Option (--buffer or -b), -j, -s, -e and -u are mutually exclusive\n");
    exit(16);
  }

  fputs__ = flg_output_buffer ? fputs : fputs2; /* without/with code conversion */

  while (optind < argc) {
    char *genc, *ret;

    infname = argv[optind++];
    if ((infp = fopen(infname, R_MODE)) == 0) {
      fprintf(stderr, "ERROR: fail to open input file [%s].", infname);
      exit(32);
    }
    if (flg_guess_enc) {
      genc = ptenc_guess_enc(infp, 1);
      printf("%s: %s\n", infname, genc);
      setinfileenc(infp, genc);
      free(genc);
    }

    if ((opt_ienc && flg_guess_enc!=GUESS_ONLY) || opt_oenc) {
      if (opt_oenc) {
        outfname = xmalloc(strlen(infname)+5);
        strcpy(outfname, infname);
        strcat(outfname, ".out");

        if ((outfp = fopen(outfname, W_MODE)) == 0) {
          fprintf(stderr, "ERROR: fail to open output file [%s].", outfname);
          exit(32);
        }
      }
      else
        outfp = stdout;

      if (strlen(opt_input_enc)) {
        setinfileenc(infp, opt_input_enc);
      }
      if (strlen(opt_output_enc)) {
        setfileenc(opt_output_enc);
      }
      while ((ret = mfgets(buff, BUFFERLEN, infp)) != NULL) {
        (*fputs__)(buff, outfp);
      }
      if (fclose(infp)) {
        fprintf(stderr, "ERROR: fail to close input file [%s].", infname);
        exit(32);
      }
      if (outfp != stdout && fclose(outfp)) {
        fprintf(stderr, "ERROR: fail to close output file [%s].", outfname);
        exit(32);
      }
      if (outfname)
        free(outfname);
    }
  }

  return 0;
}
