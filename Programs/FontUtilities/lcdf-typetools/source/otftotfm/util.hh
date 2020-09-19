#ifndef OTFTOTFM_UTIL_HH
#define OTFTOTFM_UTIL_HH
#include <lcdf/string.hh>
#include <lcdf/globmatch.hh>
#include <stdio.h>
class ErrorHandler;

extern bool no_create;
extern bool verbose;
extern bool force;

enum { G_ENCODING = 1, G_METRICS = 2, G_VMETRICS = 4, G_TYPE1 = 8,
       G_PSFONTSMAP = 16, G_BINARY = 32, G_ASCII = 64, G_DOTLESSJ = 128,
       G_UPDMAP = 256, G_TRUETYPE = 512, G_TYPE42 = 1024, G_UPDMAP_USER = 2048 };

extern unsigned output_flags;

String read_file(String filename, ErrorHandler *, bool warn = false);
String printable_filename(const String &);
String pathname_filename(const String &);
bool same_filename(const String &a, const String &b);
String shell_quote(const String &);
int temporary_file(String &, ErrorHandler *);
int mysystem(const char *command, ErrorHandler *);
FILE* mypopen(const char* command, const char* type, ErrorHandler* errh);
bool parse_unicode_number(const char*, const char*, int require_prefix, uint32_t& result);

#ifdef WIN32
#define WEXITSTATUS(es) (es)
#endif

template <typename T>
void ignore_result(T result) {
    (void) result;
}

#endif
