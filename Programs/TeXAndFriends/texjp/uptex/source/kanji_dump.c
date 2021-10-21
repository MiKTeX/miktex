/*
 *  Dump/undump Kanji encoding for (e)upTeX.
 */

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#include "kanji.h"
#ifndef upTeX
#define upTeX
#endif
#include <texmfmp.h>

#if defined(MIKTEX) && !defined(FMT_COMPRESS)
void dump_kanji(FILE* fp)
#else
void dump_kanji (gzFile fp)
#endif
{
    char buffer[12];
    const char *p = get_enc_string ();
    size_t len = strlen (p);

    if (len > 11) {
        fprintf (stderr, "Kanji encoding string \"%s\" exceeds 11 bytes.\n", p);
        uexit(1);
    }

    strcpy (buffer, p);
    for (len++; len < 12; len++)
        buffer[len] = 0;
#if defined(MIKTEX)
#if !defined(FMT_COMPRESS)
    if (fwrite (buffer, 1, 12, fp) != 12)
    {
      fprintf (stderr, "! Could not dump kanji.\n");
      uexit (1);
    }
#else
    TODO();
#endif
#else
    do_dump (buffer, 1, 12, fp);
#endif
}

#if defined(MIKTEX) && !defined(FMT_COMPRESS)
void undump_kanji(FILE* fp)
#else
void undump_kanji (gzFile fp)
#endif
{
    char buffer[12];
    char *p;
    int i;

#if defined(MIKTEX)
#if !defined(FMT_COMPRESS)
    if (fread (buffer, 1, 12, fp) != 12)
    {
        fprintf (stderr, "! Could not undump kanji.\n");
        uexit (1);
    }
#else
    TODO();
#endif
#else
    do_undump (buffer, 1, 12, fp);
#endif
    buffer[11] = 0;  /* force string termination, just in case */

    p = strchr (buffer, '.');
    if (p)
        *p++ = 0;
    else
        p = buffer;

    i = get_internal_enc();

    /* Now BUFFER and P are the file and internal encoding strings.  */
    init_kanji (NULL, p);
    if (get_internal_enc() != i) {
        fprintf (stderr, "Kanji internal encoding incompatible with the preloaded format.\n");
        fprintf (stderr, "I'll stick to %s.\n", enc_to_string(get_internal_enc()));
    }
}
