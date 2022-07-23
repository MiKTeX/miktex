/*
 *  KANJI Code conversion routines.
 */

#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include <kpathsea/config.h>
#include <kpathsea/c-memstr.h>
#include <kpathsea/variable.h>
#include <kpathsea/readable.h>
#include <kpathsea/c-limits.h>
#include <kpathsea/c-pathmx.h>

#include <ptexenc/c-auto.h>
#include <ptexenc/ptexenc.h>
#include <ptexenc/kanjicnv.h>
#include <ptexenc/unicode.h>
#include <ptexenc/unicode-jp.h>

#include <ctype.h>
#if defined(MIKTEX)
#include <miktex/Core/c/api.h>
#if !defined(PATH_MAX)
#define PATH_MAX 260
#endif
#define popen miktex_popen
#define pclose miktex_pclose
#if defined(MIKTEX_WINDOWS)
#undef _WIN32
#undef WIN32
#endif
#endif

#define ENC_UNKNOWN  0
#define ENC_JIS      1
#define ENC_EUC      2
#define ENC_SJIS     3
#define ENC_UTF8     4
#define ENC_UPTEX    5

static int default_kanji_enc;
static boolean UPTEX_enabled;
static boolean ptex_mode = false;
static boolean prior_file_enc = false;

#define ESC '\033'

#ifndef NOFILE
# ifndef OPEN_MAX
#  define OPEN_MAX 132 /* sup_max_in_open(127) +alpha */
# endif
# define NOFILE OPEN_MAX
#endif
#if defined(MIKTEX)
inline int filenoCheck(FILE* f)
{
    int fd = fileno(f);
    if (fd >= NOFILE)
    {
        fprintf(stderr, "too many open files\n");
        miktex_exit(1);
    }
    return fd;
}
#define fileno filenoCheck
#endif

const char *ptexenc_version_string = PTEXENCVERSION;
#if defined(WIN32)
FILE *Poptr;
int infile_enc_auto;
#else
static int infile_enc_auto = 1;
#endif

static int     file_enc = ENC_UNKNOWN;
static int internal_enc = ENC_UNKNOWN;
static int terminal_enc = ENC_UNKNOWN;

const_string enc_to_string(int enc)
{
    switch (enc) {
    case ENC_JIS:  return "jis";
    case ENC_EUC:  return "euc";
    case ENC_SJIS: return "sjis";
    case ENC_UTF8: return "utf8";
    case ENC_UPTEX: if (UPTEX_enabled) return "uptex";
    default:       return "?";
    }
}

static int string_to_enc(const_string str)
{
    if (str == NULL)                    return ENC_UNKNOWN;
    if (strcasecmp(str, "default")== 0) return default_kanji_enc;
    if (strcasecmp(str, "jis")    == 0) return ENC_JIS;
    if (strcasecmp(str, "euc")    == 0) return ENC_EUC;
    if (strcasecmp(str, "sjis")   == 0) return ENC_SJIS;
    if (strcasecmp(str, "utf8")   == 0) return ENC_UTF8;
    if (UPTEX_enabled && strcasecmp(str, "uptex")  == 0) return ENC_UPTEX;

    if (strcasecmp(str, "BINARY") == 0)      return ENC_JIS;
    if (strcasecmp(str, "ISO-2022-JP") == 0) return ENC_JIS;
    if (strcasecmp(str, "EUC-JP") == 0)      return ENC_EUC;
    if (strcasecmp(str, "Shift_JIS")   == 0) return ENC_SJIS;
    if (strcasecmp(str, "UTF-8")       == 0) return ENC_UTF8;
    return -1; /* error */
}

static int get_default_enc(void)
{
    /* kpse_var_value("PTEX_KANJI_ENC") aborts
       if 'kpse_program_name' is empty.  It typically occurs
       when 'ptex' and 'jmpost' print version messages. */
    string var = getenv("PTEX_KANJI_ENC");
    int enc = string_to_enc(var);
    if (enc < 0) {
        fprintf(stderr, "Warning: Unknown environment value "
                "PTEX_KANJI_ENC='%s'\n", var);
    } else if (enc != ENC_UNKNOWN) {
        return enc;
    }
    return default_kanji_enc;
}

static void set_file_enc(int enc)
{
    if (enc == ENC_UPTEX) file_enc = ENC_UTF8;
    else /* rest */       file_enc = enc;
}

static void set_internal_enc(int enc)
{
    if      (enc == ENC_SJIS)  internal_enc = ENC_SJIS;
    else if (UPTEX_enabled && enc == ENC_UPTEX) internal_enc = ENC_UPTEX;
    else /* EUC, JIS, UTF8 */  internal_enc = ENC_EUC;
}

static int get_file_enc(void)
{
    if (file_enc == ENC_UNKNOWN) set_file_enc(get_default_enc());
    return file_enc;
}

int get_internal_enc(void)
{
    if (internal_enc == ENC_UNKNOWN) set_internal_enc(get_default_enc());
    return internal_enc;
}

static int get_terminal_enc(void)
{
    if (terminal_enc == ENC_UNKNOWN) {
#if defined(MIKTEX_WINDOWS)
        terminal_enc = ENC_UTF8;
#else
        char lang[16];  /* enough large space */
        const char *s    = getenv("LC_ALL");
        if (s == NULL) s = getenv("LC_MESSAGES");
        if (s == NULL) s = getenv("LANG");
        if (s == NULL) s = getenv("LANGUAGE");
        if (s == NULL) s = "";
        if (strrchr(s, '.') != NULL) s = strrchr(s, '.') + 1;
        strncpy(lang, s, sizeof(lang) - 1);
        lang[sizeof(lang) - 1] = '\0';
        if      (strcasecmp(lang, "euc")  == 0) terminal_enc = ENC_EUC;
        else if (strcasecmp(lang, "eucJP")== 0) terminal_enc = ENC_EUC;
        else if (strcasecmp(lang, "ujis") == 0) terminal_enc = ENC_EUC;
        else if (strcasecmp(lang, "sjis") == 0) terminal_enc = ENC_SJIS;
        else if (strcasecmp(lang, "utf8") == 0) terminal_enc = ENC_UTF8;
        else if (strcasecmp(lang, "UTF-8")== 0) terminal_enc = ENC_UTF8;
        else if (strcasecmp(lang, "jis")  == 0) terminal_enc = ENC_JIS;
        else if (strcasecmp(lang, "ISO-2022-JP")== 0) terminal_enc = ENC_JIS;
        else terminal_enc = get_file_enc();
#endif
    }
    return terminal_enc;
}

/* enable ptex mode (use flag 0x100 for Japanese char) */
void ptenc_ptex_mode (const boolean enable)
{
   //fprintf(stderr, "ptenc_ptex_mode is called! (%d)\n", enable);
   ptex_mode = enable;
}

/* enable/disable UPTEX */
void enable_UPTEX (boolean enable)
{
    UPTEX_enabled = enable;
    if (enable) {
        default_kanji_enc = ENC_UPTEX;
        internal_enc = ENC_UPTEX;
    } else {
#if defined(MIKTEX_WINDOWS) || defined(WIN32)
        default_kanji_enc = ENC_UTF8;
        internal_enc = ENC_SJIS;
#else
        default_kanji_enc = ENC_UTF8;
        internal_enc = ENC_EUC;
#endif
    }
}

void set_prior_file_enc(void)
{
    prior_file_enc = true;
}

const_string get_enc_string(void)
{
    static char buffer[20]; /* enough large space */

    if (get_file_enc() == get_internal_enc()) {
        return enc_to_string(get_file_enc());
    } else {
        sprintf(buffer, "%s.%s",
                enc_to_string(get_file_enc()),
                enc_to_string(get_internal_enc()));
        return buffer;
    }
}

boolean set_enc_string(const_string file_str, const_string internal_str)
{
    int file     = string_to_enc(file_str);
    int internal = string_to_enc(internal_str);

    if (file < 0 || internal < 0) return false; /* error */
    if (file     != ENC_UNKNOWN) {
        set_file_enc(file);
#if !defined(WIN32)
        infile_enc_auto = 0;
        nkf_disable();
#endif
    }
    if (internal != ENC_UNKNOWN) set_internal_enc(internal);
    return true;
}

boolean is_internalSJIS(void)
{
    return (internal_enc == ENC_SJIS);
}

boolean is_internalEUC(void)
{
    return (internal_enc == ENC_EUC);
}

boolean is_internalUPTEX(void)
{
    return (internal_enc == ENC_UPTEX);
}

boolean is_terminalUTF8(void)
{
#ifdef WIN32
    return false;
#else
    get_terminal_enc(); return (terminal_enc == ENC_UTF8);
#endif
}


/* check char range */
boolean ismultichr (int length, int nth, int c)
{
    if (is_internalUPTEX()) return isUTF8(length, nth, c);
    if (length == 2) {
        if (nth == 1) {
            if (is_internalSJIS()) return isSJISkanji1(c);
            /* EUC */              return isEUCkanji1(c);
        } else if (nth == 2) {
            if (is_internalSJIS()) return isSJISkanji2(c);
            /* EUC */              return isEUCkanji2(c);
        }
    }
    if ((length == 3 || length == 4) &&
        (0 < nth && nth <= length)) return false;
    fprintf(stderr, "ismultichr: unexpected param length=%d, nth=%d\n",
            length, nth);
    return false;
}

/* check char range (kanji 1st) */
boolean iskanji1(int c)
{
    if (is_internalUPTEX()) return (isUTF8(2,1,c) ||
                                    isUTF8(3,1,c) ||
                                    isUTF8(4,1,c));
    if (is_internalSJIS()) return isSJISkanji1(c);
    /* EUC */              return isEUCkanji1(c);
}

/* check char range (kanji 2nd) */
boolean iskanji2(int c)
{
    if (is_internalSJIS()) return isSJISkanji2(c);
    /* EUC */              return isEUCkanji2(c);
}

/* multi-byte char length in s[pos] */
#define DEFINE_MULTISTRLEN(SUFF,TYPE) \
int multistrlen ## SUFF(TYPE *s, int len, int pos) \
{ \
    s += pos; len -= pos; \
    if (is_internalUPTEX()) { \
        int ret = UTF8Slength ## SUFF(s, len); \
        if (ret < 0) return 1; \
        return ret; \
    } \
    if (len < 2) return 1; \
    if (is_internalSJIS()) { \
        if (isSJISkanji1(s[0]) && isSJISkanji2(s[1])) return 2; \
    } else { /* EUC */ \
        if (isEUCkanji1(s[0])  && isEUCkanji2(s[1]))  return 2; \
    } \
    return 1; \
}
DEFINE_MULTISTRLEN(,unsigned char);
DEFINE_MULTISTRLEN(short,unsigned short);

/* for outputting filename (*s) to the terminal */
int multistrlenfilename(unsigned short *s, int len, int pos)
{
    s += pos; len -= pos;
    if (terminal_enc == ENC_UTF8) {
        int ret = UTF8Slengthshort(s, len);
        if (ret < 0) return 1;
        return ret;
    }
    if (len < 2) return 1;
    if (terminal_enc == ENC_SJIS) {
        if (isSJISkanji1(s[0]) && isSJISkanji2(s[1])) return 2;
    } else { /* EUC */
        if (isEUCkanji1(s[0])  && isEUCkanji2(s[1]))  return 2;
    }
    return 1;
}

/* with not so strict range check */
int multibytelen (int first_byte)
{
    if (is_internalUPTEX()) {
        return UTF8length(first_byte);
    } else if (is_internalSJIS()) {
        if (isSJISkanji1(first_byte)) return 2;
    } else { /* EUC */
        if (isEUCkanji1(first_byte))  return 2;
    }
    return 1;
}

/* buffer (EUC/SJIS/UTF-8) to internal (EUC/SJIS/UPTEX) code conversion */
long fromBUFF(unsigned char *s, int len, int pos)
{
    s += pos; len -= pos;
    if (is_internalUPTEX()) {
        if (UTF8Slength(s, len) < 0) return s[0];
        return UCStoUPTEX(UTF8StoUCS(s));
    }
    if (len < 2) return s[0];
    if (is_internalSJIS()) {
        if (isSJISkanji1(s[0]) && isSJISkanji2(s[1])) return HILO(s[0], s[1]);
    } else { /* EUC */
        if (isEUCkanji1(s[0])  && isEUCkanji2(s[1]))  return HILO(s[0], s[1]);
    }
    return s[0];
}

long fromBUFFshort(unsigned short *s, int len, int pos)
{
    int i;
    unsigned char sc[6];
    s += pos; len -= pos;
    for (i=0;i<(len<6 ? len : 6);i++) sc[i]=0xFF&s[i];
    return fromBUFF(sc, (len<6 ? len : 6), 0);
}

/* internal (EUC/SJIS/UPTEX) to buffer (EUC/SJIS/UTF-8) code conversion */
long toBUFF(long kcode)
{
    if (is_internalUPTEX()) kcode = UCStoUTF8(UPTEXtoUCS(kcode));
    return kcode;
}

/* DVI (JIS/UCS) to internal (EUC/SJIS/UPTEX) code conversion */
long fromDVI (long kcode)
{
    if (is_internalUPTEX()) return UCStoUPTEX(kcode);
    if (is_internalSJIS())  return JIStoSJIS(kcode);
    /* EUC */               return JIStoEUC(kcode);
}

/* internal (EUC/SJIS/UPTEX) to DVI (JIS/UCS) code conversion */
long toDVI (long kcode)
{
    if (is_internalUPTEX()) return UPTEXtoUCS(kcode);
    if (is_internalSJIS())  return SJIStoJIS(kcode);
    /* EUC */               return EUCtoJIS(kcode);
}

/* JIS to internal (EUC/SJIS/UPTEX) code conversion */
long fromJIS(long kcode)
{
    if (is_internalUPTEX()) return UCStoUPTEX(JIStoUCS2(kcode));
    if (is_internalSJIS())  return JIStoSJIS(kcode);
    /* EUC */               return JIStoEUC(kcode);
}

/* internal (EUC/SJIS/UPTEX) to JIS code conversion */
long toJIS(long kcode)
{
    if (is_internalUPTEX()) return UCS2toJIS(UPTEXtoUCS(kcode));
    if (is_internalSJIS())  return SJIStoJIS(kcode);
    /* EUC */               return EUCtoJIS(kcode);
}


/* EUC to internal (EUC/SJIS/UPTEX) code conversion */
long fromEUC(long kcode)
{
    if (!is_internalUPTEX() && !is_internalSJIS()) return kcode;
    return fromJIS(EUCtoJIS(kcode));
}

/* internal (EUC/SJIS/UPTEX) to EUC code conversion */
static long toEUC(long kcode)
{
    if (!is_internalUPTEX() && !is_internalSJIS()) return kcode;
    return JIStoEUC(toJIS(kcode));
}


/* SJIS to internal (EUC/SJIS/UPTEX) code conversion */
long fromSJIS(long kcode)
{
    if (is_internalSJIS()) return kcode;
    return fromJIS(SJIStoJIS(kcode));
}

/* internal (EUC/SJIS/UPTEX) to SJIS code conversion */
static long toSJIS(long kcode)
{
    if (is_internalSJIS()) return kcode;
    return JIStoSJIS(toJIS(kcode));
}


/* KUTEN to internal (EUC/SJIS/UPTEX) code conversion */
long fromKUTEN(long kcode)
{
    return fromJIS(KUTENtoJIS(kcode));
}


/* UCS to internal (EUC/SJIS/UPTEX) code conversion */
long fromUCS(long kcode)
{
    if (is_internalUPTEX()) return UCStoUPTEX(kcode);
    kcode = UCS2toJIS(kcode);
    if (kcode == 0) return 0;
    return fromJIS(kcode);
}

/* internal (EUC/SJIS/UPTEX) to UCS code conversion */
long toUCS(long kcode)
{
    if (is_internalUPTEX()) return UPTEXtoUCS(kcode);
    return JIStoUCS2(toJIS(kcode));
}

/* internal (EUC/SJIS/UPTEX) to UTF-8 code conversion */
static long toUTF8 (long kcode)
{
    return UCStoUTF8(toUCS(kcode));
}

/* internal (EUC/SJIS/UPTEX) to 'enc' code conversion */
static long toENC(long kcode, int enc)
{
    switch (enc) {
    case ENC_UTF8: return toUTF8(kcode);
    case ENC_JIS:  return toJIS(kcode);
    case ENC_EUC:  return toEUC(kcode);
    case ENC_SJIS: return toSJIS(kcode);
    default:
        fprintf(stderr, "toENC: unknown enc (%d).\n", enc);
        return 0;
    }
}

#define KANJI_IN   LONG(0, ESC, '$', 'B')
#define KANJI_OUT  LONG(0, ESC, '(', 'B')

static int put_multibyte(long c, FILE *fp) {
#ifdef WIN32
    const int fd = fileno(fp);

    if ((fd == fileno(stdout) || fd == fileno(stderr)) && _isatty(fd)) {
       HANDLE hStdout;
       DWORD ret, wclen;
       UINT cp;
       wchar_t buff[2];
       char str[4];
       int mblen;

       if (fd == fileno(stdout))
           hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
       else
           hStdout = GetStdHandle(STD_ERROR_HANDLE);

       mblen=0;
       if (BYTE1(c) != 0) str[mblen++]=BYTE1(c);
       if (BYTE2(c) != 0) str[mblen++]=BYTE2(c);
       if (BYTE3(c) != 0) str[mblen++]=BYTE3(c);
       /* always */       str[mblen++]=BYTE4(c);

#define CP_UTF8    65001

       cp = CP_UTF8;
       if (MultiByteToWideChar(cp, 0, str, mblen, buff, 2) == 0)
           return EOF;

       wclen = mblen > 3 ? 2 : 1;
       if (WriteConsoleW(hStdout, buff, wclen, &ret, NULL) == 0)
           return EOF;

       return BYTE4(c);
    }
#endif

    if (BYTE1(c) != 0 && putc(BYTE1(c), fp) == EOF) return EOF;
    if (BYTE2(c) != 0 && putc(BYTE2(c), fp) == EOF) return EOF;
    if (BYTE3(c) != 0 && putc(BYTE3(c), fp) == EOF) return EOF;
    /* always */  return putc(BYTE4(c), fp);
}

static int flush (unsigned char *buff, int num, FILE *fp)
{
    int i, ret = EOF;

    /* fprintf(stderr, "putc2: unexpected chars. ( ");
       for (i=0; i<num; i++) fprintf(stderr, "%02X ", buff[i]);
       fprintf(stderr, ")\n");
    */
    for (i=0; i<num; i++) ret = putc(buff[i], fp);
    return ret;
}

/* putc() with code conversion */
int putc2(int c, FILE *fp)
/*
  c in [0,255]:  writes the character c, without code conversion
  c in [256,511]: writes the character c-256, with code conversion
*/
{
    static int num[NOFILE];
        /* 0    : not in Kanji
           1..4 : in JIS Kanji and num[] bytes are in store[][]
           -1   : in JIS Kanji and store[][] is empty */
    static unsigned char store[NOFILE][4];
    const int fd = fileno(fp);
    int ret = c, output_enc;
#if defined(MIKTEX_WINDOWS) || defined(WIN32)
    if ((fp == stdout || fp == stderr) && (_isatty(fd) || !prior_file_enc)) {
        output_enc = ENC_UTF8;
     } else
        output_enc = get_file_enc();
#else
    if ((fp == stdout || fp == stderr) && !prior_file_enc) {
        output_enc = get_terminal_enc();
    } else
        output_enc = get_file_enc();
#endif
    if (ptex_mode && (c<256)) {
        if (num[fd] < 0 && output_enc == ENC_JIS) {
            put_multibyte(KANJI_OUT, fp);
        }
        ret = putc(c, fp);
        num[fd] = 0;
    } else {
        c &= 0xFF;
        if (num[fd] > 0) {        /* multi-byte char */
            if (is_internalUPTEX() && iskanji1(c)) { /* error */
                ret = flush(store[fd], num[fd], fp);
                num[fd] = 0;
            }
            store[fd][num[fd]] = c;
            num[fd]++;
            if (multistrlen(store[fd], num[fd], 0) == num[fd]) {
                long i = fromBUFF(store[fd], num[fd], 0);
                ret = put_multibyte(toENC(i, output_enc), fp);
                num[fd] = -1;
            } else if ((is_internalUPTEX() && num[fd] == 4) ||
                (!is_internalUPTEX() && num[fd] == 2)) { /* error */
                ret = flush(store[fd], num[fd], fp);
                num[fd] = -1;
            }
        } else if (iskanji1(c)) { /* first multi-byte char */
            if (num[fd] == 0 && output_enc == ENC_JIS) {
                ret = put_multibyte(KANJI_IN, fp);
            }
            store[fd][0] = c;
            num[fd] = 1;
        } else {                  /* ASCII */
            if (num[fd] < 0 && output_enc == ENC_JIS) {
                put_multibyte(KANJI_OUT, fp);
            }
            ret = putc(c, fp);
            num[fd] = 0;
        }
    }
    return ret;
}

/* fputs() with code conversion */
int fputs2(const char *s, FILE *fp)
{
    while (*s != '\0') {
        int ret = putc2((unsigned char)*s, fp);
        if (ret == EOF) return EOF;
        s++;
    }
    return 1;
}


static struct unget_st {
    int size;
    int buff[4];
} ungetbuff[NOFILE];

static int getc4(FILE *fp)
{
    struct unget_st *p = &ungetbuff[fileno(fp)];

    if (p->size == 0)
#ifdef WIN32
    {
        const int fd = fileno(fp);
        HANDLE hStdin;
        DWORD ret;
        wchar_t wc[2];
        long c;
        static wchar_t wcbuf = L'\0';

        if (!(fd == fileno(stdin) && _isatty(fd) && is_internalUPTEX()))
            return getc(fp);

        hStdin = GetStdHandle(STD_INPUT_HANDLE);
        if (wcbuf) {
            wc[0] = wcbuf;
            wcbuf = L'\0';
        }
        else if (ReadConsoleW(hStdin, wc, 1, &ret, NULL) == 0)
            return EOF;
        if (0xd800<=wc[0] && wc[0]<0xdc00) {
            if (ReadConsoleW(hStdin, wc+1, 1, &ret, NULL) == 0)
                return EOF;
            if (0xdc00<=wc[1] && wc[1]<0xe000) {
                c = UTF16StoUTF32(wc[0], wc[1]);
            } else {
                wcbuf = wc[1];
                c = U_REPLACEMENT_CHARACTER;  /* illegal upper surrogate pair */
            }
        } else if (0xdc00<=wc[0] && wc[0]<0xe000) {
            c = U_REPLACEMENT_CHARACTER;      /* illegal lower surrogate pair */
        } else {
            c = wc[0];
        }
        c = UCStoUTF8(c);
        /* always */       p->buff[p->size++]=BYTE4(c);
        if (BYTE3(c) != 0) p->buff[p->size++]=BYTE3(c);
        if (BYTE2(c) != 0) p->buff[p->size++]=BYTE2(c);
        if (BYTE1(c) != 0) p->buff[p->size++]=BYTE1(c);
    }
#else
        return getc(fp);
#endif
    return p->buff[--p->size];
}

static int ungetc4(int c, FILE *fp)
{
    struct unget_st *p = &ungetbuff[fileno(fp)];

    if (p->size >= 4) return EOF;
    return p->buff[p->size++] = c;
}


static unsigned char *buffer;
static long first, last;
static boolean combin_voiced_sound(boolean semi)
{
    int i, mblen;

    mblen = is_internalUPTEX() ? 3 : 2;
    if (last-mblen < first) return false;
    if (multistrlen(buffer,last,last-mblen) != mblen) return false;
    i = toUCS(fromBUFF(buffer,last,last-mblen));
    i = get_voiced_sound(i, semi);
    if (i == 0) return false;
    i = toBUFF(fromUCS(i));
    if (BYTE2(i) != 0) buffer[last-3] = BYTE2(i);
    /* always */       buffer[last-2] = BYTE3(i);
    /* always */       buffer[last-1] = BYTE4(i);
    return true;
}

static void write_multibyte(long i)
{
    if (BYTE1(i) != 0) buffer[last++] = BYTE1(i);
    if (BYTE2(i) != 0) buffer[last++] = BYTE2(i);
    /* always */       buffer[last++] = BYTE3(i);
    /* always */       buffer[last++] = BYTE4(i);
}

static void write_hex(int i)
{
    sprintf((char *) buffer + last, "^^%02x", i);
    last += 4;
}

/* getc() with check of broken encoding of UTF-8 */
static int getcUTF8(FILE *fp)
{
    int c = getc4(fp);

    if (isUTF8(2,2,c)) return c;
    ungetc4(c, fp);
    return EOF;
}

static void get_utf8(int i, FILE *fp)
{
    long u = 0, j;
    int i2 = EOF, i3 = EOF, i4 = EOF;

    switch (UTF8length(i)) {
    case 2:
        i2 = getcUTF8(fp); if (i2 == EOF) break;
        u = UTF8BtoUCS(i, i2);
        break;
    case 3:
        i2 = getcUTF8(fp); if (i2 == EOF) break;
        i3 = getcUTF8(fp); if (i3 == EOF) break;
        u = UTF8CtoUCS(i, i2, i3);
        if (u == U_BOM) return; /* just ignore */
        if (u == U_VOICED      && combin_voiced_sound(false)) return;
        if (u == U_SEMI_VOICED && combin_voiced_sound(true))  return;
        break;
    case 4:
        i2 = getcUTF8(fp); if (i2 == EOF) break;
        i3 = getcUTF8(fp); if (i3 == EOF) break;
        i4 = getcUTF8(fp); if (i4 == EOF) break;
        u = UTF8DtoUCS(i, i2, i3, i4);
        break;
    default:
        u = U_REPLACEMENT_CHARACTER;
        break;
    }

    j = (u != 0) ? toBUFF(fromUCS(u)) : 0;
    if (j == 0) { /* can't represent (typically umlaut o in EUC) */
        write_hex(i);
        if (i2 != EOF) write_hex(i2);
        if (i3 != EOF) write_hex(i3);
        if (i4 != EOF) write_hex(i4);
    } else {
        write_multibyte(j);
    }
}

static void get_euc(int i, FILE *fp)
{
    int j = getc4(fp);

    if (isEUCkanji2(j)) {
        write_multibyte(toBUFF(fromEUC(HILO(i,j))));
    } else {
        buffer[last++] = i;
        ungetc4(j, fp);
    }
}        

static void get_sjis(int i, FILE *fp)
{
    int j = getc4(fp);

    if (isSJISkanji2(j)) {
        write_multibyte(toBUFF(fromSJIS(HILO(i,j))));
    } else {
        buffer[last++] = i;
        ungetc4(j, fp);
    }
}        

static boolean is_tail(long *c, FILE *fp)
{
    if (*c == EOF) return true;
    if (*c == '\n') return true;
    if (*c == '\r') {
        int d = getc4(fp);
        if (d == '\n') *c = d;
        else ungetc4(d, fp);
        return true;
    }
    return false;
}

#define MARK_LEN 4
/* if stream begins with BOM + 7bit char */
static boolean isUTF8Nstream(FILE *fp)
{
    int i;
    int c[MARK_LEN];
    int bom_u[MARK_LEN] = { 0xEF, 0xBB, 0xBF, 0x7E };
    int bom_l[MARK_LEN] = { 0xEF, 0xBB, 0xBF, 0 };

    for (i=0; i<MARK_LEN; i++) {
        c[i] = getc4(fp);
        if (!(bom_l[i] <= c[i] && c[i] <= bom_u[i])) {
            do { ungetc4(c[i], fp); } while (i-- > 0);
            return false;
        }
    }
    ungetc4(c[MARK_LEN-1], fp);
    return true;
}

static int infile_enc[NOFILE]; /* ENC_UNKNOWN (=0): not determined
                                  other: determined */

/* input line with encoding conversion */
long input_line2(FILE *fp, unsigned char *buff, unsigned char *buff2,
                 long pos, const long buffsize, int *lastchar)
{
    long i = 0;
    static boolean injis = false;
    const int fd = fileno(fp);

    if (infile_enc[fd] == ENC_UNKNOWN) { /* just after opened */
        ungetbuff[fd].size = 0;
        if (isUTF8Nstream(fp)) infile_enc[fd] = ENC_UTF8;
        else                   infile_enc[fd] = get_file_enc();
    }
    buffer = buff;
    first = last = pos;

    while (last < buffsize-30 && (i=getc4(fp)) != EOF && i!='\n' && i!='\r') {
        /* 30 is enough large size for one char */
        /* attention: 4 times of write_hex() eats 16byte */
#ifdef WIN32
        if (i == 0x1a && first == last &&
            fd == fileno(stdin) && _isatty(fd)) { /* Ctrl+Z on console */
                i = EOF;
                break;
        } else
#endif
        if (i == ESC) {
            if ((i=getc4(fp)) == '$') { /* ESC '$' (Kanji-in) */
                i = getc4(fp);
                if (i == '@' || i == 'B') {
                    injis = true;
                } else {               /* broken Kanji-in */
                    buffer[last++] = ESC;
                    buffer[last++] = '$';
                    if (is_tail(&i, fp)) break;
                    buffer[last++] = i;
                }
            } else if (i == '(') {     /* ESC '(' (Kanji-out) */
                i = getc4(fp);
                if (i == 'J' || i == 'B' || i == 'H') {
                    injis = false;
                } else {               /* broken Kanji-out */
                    buffer[last++] = ESC;
                    buffer[last++] = '(';
                    if (is_tail(&i, fp)) break;
                    buffer[last++] = i;
                }
            } else { /* broken ESC */
                buffer[last++] = ESC;
                if (is_tail(&i, fp)) break;
                buffer[last++] = i;
            }
        } else { /* rather than ESC */
            if (injis) { /* in JIS */
                long j = getc4(fp);
                if (is_tail(&j, fp)) {
                    buffer[last++] = i;
                    i = j;
                    break;
                } else { /* JIS encoding */
                    i = fromJIS(HILO(i,j));
                    if (i == 0) i = fromUCS(U_REPLACEMENT_CHARACTER);
                    write_multibyte(toBUFF(i));
                }
            } else {  /* normal */
                if        (infile_enc[fd] == ENC_SJIS && isSJISkanji1(i)) {
                    get_sjis(i, fp);
                } else if (infile_enc[fd] == ENC_EUC  && isEUCkanji1(i)) {
                    get_euc(i, fp);
                } else if (infile_enc[fd] == ENC_UTF8 && UTF8length(i) > 1) {
                    get_utf8(i, fp);
                } else {
                    buffer[last++] = i;
                }
            }
        }
    }

    if (i != EOF || first != last) buffer[last] = '\0';
    if (i == EOF || i == '\n' || i == '\r') injis = false;
    if (lastchar != NULL) *lastchar = i;

    if (buff2!= NULL) for (i=pos; i<=last; i++) buff2[i] = 0;
    /* buff2 is initialized */

    return last;
}

/* set encode of stdin if fp = NULL */
boolean setinfileenc(FILE *fp, const char *str)
{
    int enc;
    enc = string_to_enc(str);
    if (enc < 0) return false;
    infile_enc[fileno(fp)] = enc;
    return true;
}

boolean setstdinenc(const char *str)
{
    int enc;
    enc = string_to_enc(str);
    if (enc < 0) return false;
    infile_enc[fileno(stdin)] = enc;
    return true;
}

#ifdef WIN32
void clear_infile_enc(FILE *fp)
{
    infile_enc[fileno(fp)] = ENC_UNKNOWN;
}
#else /* !WIN32 */
static const_string in_filter = NULL;
static FILE *piped_fp[NOFILE];
static int piped_num = 0;

void nkf_disable(void)
{
    in_filter = "";
}

#ifdef NKF_TEST
static void nkf_check(void)
{
    if (piped_num > 0) {
        fprintf(stderr, "nkf_check: %d nkf_open() did not closed.\n",
                piped_num);
    } else {
        fprintf(stderr, "nkf_check: nkf_open() OK.\n");
    }
}
#endif /* NKF_TEST */

/* 'mode' must be read */
FILE *nkf_open(const char *path, const char *mode) {
    char buff[PATH_MAX * 2 + 20];  /* 20 is enough gaps */
    char *name;
    FILE *fp;

    if (in_filter == NULL) {
        in_filter = kpse_var_value("PTEX_IN_FILTER");
        if (in_filter == NULL || strcasecmp(in_filter, "no") == 0) {
            nkf_disable();
        }
#ifdef NKF_TEST
        atexit(nkf_check);
#endif /* NKF_TEST */
    }

    if (in_filter[0] == '\0') return fopen(path, mode);
    name = xstrdup(path);
    if (kpse_readable_file(name) == NULL) {
        free(name);
        return NULL; /* can't read */
    }

    sprintf(buff, "%.*s < '%.*s'", PATH_MAX, in_filter, PATH_MAX, path);
    free(name);
    /* fprintf(stderr, "\n`%s`", buff); */
    fp = popen(buff , "r");
    if (piped_num < NOFILE) piped_fp[piped_num++] = fp;
    return fp;
}

/* we must close in stack order (FILO) or in queue order (FIFO) */
int nkf_close(FILE *fp) {
    infile_enc[fileno(fp)] = ENC_UNKNOWN;
    if (piped_num > 0) {
        if (fp == piped_fp[piped_num-1]) {  /* for FILO */
            piped_num--;
            return pclose(fp);
        }
        if (fp == piped_fp[0]) {  /* for FIFO */
            int i;
            piped_num--;
            for (i=0; i<piped_num; i++) piped_fp[i] = piped_fp[i+1];
            return pclose(fp);
        }
    }
    return fclose(fp);
}

#define break_if_bad_utf8_second(k) if ((k<0x80)||(k>0xBF)) { i--; k='\0'; break; }
#define write_hex_if_not_ascii(c) \
   if ((c>=0x20)&&(c<=0x7E)) buffer[last++]=c; else write_hex(c);
unsigned char *ptenc_from_utf8_string_to_internal_enc(const unsigned char *is)
{
    int i;
    long u = 0, j, len;
    int i1, i2, i3, i4;
    unsigned char *buf, *buf_bak;
    long first_bak, last_bak;

    if (terminal_enc != ENC_UTF8 || is_internalUPTEX()) return NULL;
    buf_bak = buffer;
    first_bak = first;
    last_bak = last;

    len = strlen(is)+1;
    buffer = buf = xmalloc(len);
    first = last = 0;

    for (i=0; i<strlen(is); i++) {
        i1 = is[i]; i2 = i3 = i4 = '\0';
        switch (UTF8length(i1)) {
        case 1:
            buffer[last++] = i1; /* ASCII */
            if (i1 == '\0') goto end;
            continue;
        case 2:
            i2 = is[++i]; break_if_bad_utf8_second(i2);
            u = UTF8BtoUCS(i1, i2);
            break;
        case 3:
            i2 = is[++i]; break_if_bad_utf8_second(i2);
            i3 = is[++i]; break_if_bad_utf8_second(i3);
            u = UTF8CtoUCS(i1, i2, i3);
            if (u == U_BOM) continue; /* just ignore */
            if (u == U_VOICED      && combin_voiced_sound(false)) continue;
            if (u == U_SEMI_VOICED && combin_voiced_sound(true))  continue;
            break;
        case 4:
            i2 = is[++i]; break_if_bad_utf8_second(i2);
            i3 = is[++i]; break_if_bad_utf8_second(i3);
            i4 = is[++i]; break_if_bad_utf8_second(i4);
            u = UTF8DtoUCS(i1, i2, i3, i4);
            break;
        default:
            u = U_REPLACEMENT_CHARACTER;
            break;
        }

        j = (u != 0) ? toBUFF(fromUCS(u)) : 0;
        if (j == 0) { /* can't represent in EUC/SJIS */
            if (last+16>=len) buffer = buf = xrealloc(buffer, len=last+64);
            write_hex_if_not_ascii(i1);
            if (i2 != '\0') write_hex_if_not_ascii(i2);
            if (i3 != '\0') write_hex_if_not_ascii(i3);
            if (i4 != '\0') write_hex_if_not_ascii(i4);
        } else {
            write_multibyte(j);
        }
    }
    buffer[last] = '\0';
 end:
    buffer = buf_bak;
    first = first_bak;
    last = last_bak;
    return buf;
}

unsigned char *ptenc_from_internal_enc_string_to_utf8(const unsigned char *is)
{
    int i;
    long u = 0, len;
    int i1 = EOF, i2 = EOF;
    unsigned char *buf, *buf_bak;
    long first_bak, last_bak;

    if (terminal_enc != ENC_UTF8 || is_internalUPTEX()) return NULL;
    buf_bak = buffer;
    first_bak = first;
    last_bak = last;

    len = strlen(is)+1;
    buffer = buf = xmalloc(len*4);
    first = last = 0;

    for (i=0; i<strlen(is); i++) {
        i1 = is[i];
        switch (multibytelen(i1)) {
        case 1:
            buffer[last++] = i1; /* ASCII */
            if (i1 == '\0') goto end;
            continue;
        case 2: /* i1: not ASCII */
            i2 = is[++i];
            if (i2 == '\0') {
              write_hex(i1); continue;
            } else {
              u = JIStoUCS2(toJIS(HILO(i1,i2)));
              if (u==0) {
                write_hex(i1); write_hex_if_not_ascii(i2); continue;
              }
            }
            break;
        default: /* reachable only if internal code is uptex */
            u = U_REPLACEMENT_CHARACTER;
            break;
        }
        write_multibyte(UCStoUTF8(u));
    }
    buffer[last] = '\0';
 end:
    buffer = buf_bak;
    first = first_bak;
    last = last_bak;
    return buf;
}

int ptenc_get_command_line_args(int *p_ac, char ***p_av)
{
    int i, argc;
    char **argv;

    get_terminal_enc();
    if (terminal_enc == ENC_UTF8 && !is_internalUPTEX()) {
        argc = *p_ac;
        argv = xmalloc(sizeof(char *)*(argc+1));
        for (i=0; i<argc; i++) {
            argv[i] = ptenc_from_utf8_string_to_internal_enc((*p_av)[i]);
#ifdef DEBUG
            fprintf(stderr, "Commandline arguments %d:(%s)\n", i, argv[i]);
#endif /* DEBUG */
        }
        argv[argc] = NULL;
        *p_av = argv;
         return terminal_enc;
    }
    return 0;
}

#endif /* !WIN32 */
