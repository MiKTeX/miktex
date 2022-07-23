/*
 *  KANJI Code conversion routines.
 *  (for pTeX and e-pTeX)
 */

#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include "kanji.h"
#define CS_TOKEN_FLAG   0xFFFF

/* TOKEN */
boolean check_kanji(integer c)
{
    if (c >= CS_TOKEN_FLAG) return false;
    else return is_char_kanji(c);
}

boolean is_char_ascii(integer c)
{
    return (0 <= c && c < 0x100);
}

boolean is_char_kanji(integer c)
{
    return (iskanji1(Hi(c)) && iskanji2(Lo(c)));
}

boolean ismultiprn(integer c)
{
    if (iskanji1(c) || iskanji2(c)) return true;
    return false;
}

#ifdef OLDSTYLE
integer calc_pos(integer c)
{
    int c1, c2;

    if(c<256) return(c<<1);
    c1 = c>>8;
    c2 = c & 0xff;
    if(c1) {
		if (is_internalSJIS()) 
			return((c2+(c2<<(c1-0x81)) & 0xff)<<1);
		else
			return((c2+(c2<<(c1-0xa1)) & 0xff)<<1);
    } else
        return(((c2+c2+1) & 0xff)<<1);
}
#else /* OLDSTYLE */
integer calc_pos(integer c)
{
    unsigned char c1, c2;

    if(c>=0 && c<=255) return(c);
    c1 = (c >> 8) & 0xff;
    c2 = c & 0xff;
    if(iskanji1(c1)) {
		if (is_internalSJIS()) {
			c1 = ((c1 - 0x81) % 4) * 64;  /* c1 = 0, 64, 128, 192 */
			c2 = c2 % 64;                 /* c2 = 0..63 */
		} else {
			c1 = ((c1 - 0xa1) % 4) * 64;  /* c1 = 0, 64, 128, 192 */
			c2 = c2 % 64;                 /* c2 = 0..63 */
		}
		return(c1 + c2);              /* ret = 0..255 */
    } else
        return(c2);
}
#endif /* OLDSTYLE */

integer kcatcodekey(integer c)
{
    return Hi(toDVI(c));
}

void init_kanji (const_string file_str, const_string internal_str)
{
    if (!set_enc_string (file_str, internal_str)) {
        fprintf (stderr, "Bad kanji encoding \"%s\" or \"%s\".\n",
                 file_str ? file_str  : "NULL",
                 internal_str ? internal_str : "NULL");
        uexit(1);
    }
}

void init_default_kanji (const_string file_str, const_string internal_str)
{
    char *p;

    enable_UPTEX (false); /* disable */

    init_kanji (file_str, internal_str);

    p = getenv ("PTEX_KANJI_ENC");
    if (p) {
        if (!set_enc_string (p, NULL))
            fprintf (stderr, "Ignoring bad kanji encoding \"%s\".\n", p);
    }

#if !defined(MIKTEX)
#ifdef WIN32
    p = kpse_var_value ("guess_input_kanji_encoding");
    if (p) {
        if (*p == '1' || *p == 'y' || *p == 't')
            infile_enc_auto = 1;
        free(p);
    }
#endif
#endif
}
