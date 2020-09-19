#ifndef OTFTOTFM_KPSEINTERFACE_H
#define OTFTOTFM_KPSEINTERFACE_H
#ifdef __cplusplus
extern "C" {
#endif

void kpsei_init(const char* argv0, const char* progname);
extern int kpsei_env_sep_char;
char* kpsei_path_expand(const char* path); /* free() result */
enum { KPSEI_FMT_WEB2C, KPSEI_FMT_ENCODING, KPSEI_FMT_TYPE1,
       KPSEI_FMT_OTHER_TEXT, KPSEI_FMT_MAP, KPSEI_FMT_TRUETYPE,
       KPSEI_FMT_OPENTYPE, KPSEI_FMT_TYPE42 };
char* kpsei_find_file(const char* name, int format);
void kpsei_set_debug_flags(unsigned flags);

#ifdef __cplusplus
}
#endif
#endif
