/*
 *  KANJI Code conversion routines.
 */

#ifndef PTEXENC_PTEXENC_H
#define PTEXENC_PTEXENC_H

#include <kpathsea/types.h>

#if defined (WIN32) && !defined (__MINGW32__) && !defined (NO_PTENC_DLL)
#define PTENC_DLL 1
#endif /* WIN32 && !__MINGW32__ && !NO_PTENC_DLL */

#if defined (PTENC_DLL) && (defined (WIN32) || defined (__CYGWIN__))
#ifdef MAKE_PTENC_DLL
#define PTENCDLL __declspec(dllexport)
#else /* ! MAKE_PTENC_DLL */
#define PTENCDLL __declspec(dllimport)
#endif
#else /* ! (PTENC_DLL && (WIN32 || __CYGWIN__)) */
#define PTENCDLL
#endif
#if defined(MIKTEX) && defined(__cplusplus)
extern "C" {
#endif

extern PTENCDLL const char *ptexenc_version_string;
#if defined(WIN32)
extern PTENCDLL FILE *Poptr;
extern PTENCDLL int infile_enc_auto;
#endif

#define KANJI_OPTS "{jis|euc|sjis|utf8}"

/* enable/disable UPTEX */
extern PTENCDLL void enable_UPTEX (boolean enable);
extern PTENCDLL void set_prior_file_enc(void);
extern PTENCDLL void ptenc_ptex_mode(const boolean enable);

/* get/set Kanji encoding by string */
extern PTENCDLL const_string get_enc_string(void);
extern PTENCDLL boolean set_enc_string(const_string file, const_string inter);
#define getencstring  get_enc_string
#define setencstring  set_enc_string

/* get internal Kanji encode (used in kanji_dump.c) */
extern PTENCDLL const_string enc_to_string(int enc);
extern PTENCDLL int get_internal_enc(void);

/* decide if internal Kanji encode is SJIS/UPTEX or not */
extern PTENCDLL boolean  is_internalSJIS(void);
extern PTENCDLL boolean  is_internalEUC(void);
extern PTENCDLL boolean  is_internalUPTEX(void);
#define isinternalSJIS  is_internalSJIS
#define isinternalEUC   is_internalEUC
#define isinternalUPTEX is_internalUPTEX

extern PTENCDLL boolean  is_terminalUTF8(void);
#define isterminalUTF8 is_terminalUTF8

/* check char range */
extern PTENCDLL boolean ismultichr (int length, int nth, int c);
extern PTENCDLL boolean iskanji1(int c);
extern PTENCDLL boolean iskanji2(int c);

/* internal (EUC/SJIS/UPTEX) from/to buffer (EUC/SJIS/UTF-8) code conversion */
extern PTENCDLL int multistrlen(unsigned char *s, int len, int pos);
extern PTENCDLL int multistrlenshort(unsigned short *s, int len, int pos);
extern PTENCDLL int multibytelen (int first_byte);
extern PTENCDLL long fromBUFF(unsigned char *s, int len, int pos);
extern PTENCDLL long fromBUFFshort(unsigned short *s, int len, int pos);
extern PTENCDLL long toBUFF(long inter);

/* for outputting filename (*s) to the terminal */
extern PTENCDLL int multistrlenfilename(unsigned short *s, int len, int pos);

/* internal (EUC/SJIS/UPTEX) from/to DVI (JIS/UCS) code conversion */
extern PTENCDLL long toDVI (long kcode);
extern PTENCDLL long fromDVI (long kcode);

/* JIS/EUC/SJIS/KUTN/UCS to internal (EUC/SJIS/UPTEX) code conversion */
/* (only for \euc primitive, etc.) */
extern PTENCDLL long toJIS(long kcode);
extern PTENCDLL long fromJIS(long jis);
extern PTENCDLL long fromEUC(long euc);
extern PTENCDLL long fromSJIS(long sjis);
extern PTENCDLL long fromKUTEN(long kuten);
extern PTENCDLL long fromUCS (long ucs);
extern PTENCDLL long toUCS(long kcode);

/* fputs/putc with encoding conversion */
extern PTENCDLL int putc2(int c, FILE *fp);
extern PTENCDLL int fputs2(const char *s, FILE *fp);

/* input line with encoding conversion */
extern PTENCDLL long input_line2(FILE *fp, unsigned char *buff, unsigned char *buff2,
                                long pos, const long buffsize, int *lastchar);

/* set current encoding */
extern PTENCDLL boolean setinfileenc(FILE *fp, const char *str);
extern PTENCDLL boolean setstdinenc(const char *str);

#if !defined(MIKTEX_WINDOWS) && defined(WIN32)
extern PTENCDLL void clear_infile_enc(FILE *fp);
#else
/* open/close through nkf */
extern PTENCDLL void nkf_disable(void);
extern PTENCDLL FILE *nkf_open(const char *path, const char *mode);
extern PTENCDLL int nkf_close(FILE *fp);
extern PTENCDLL unsigned char *ptenc_from_utf8_string_to_internal_enc(const unsigned char *is);
extern PTENCDLL unsigned char *ptenc_from_internal_enc_string_to_utf8(const unsigned char *is);
extern PTENCDLL int ptenc_get_command_line_args(int *p_ac, char ***p_av);
#endif

#if defined(MIKTEX) && defined(__cplusplus)
}
#endif
#endif /* PTEXENC_PTEXENC_H */
