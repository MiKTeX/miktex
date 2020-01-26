#ifndef OTFTOTFM_OTFTOTFM_HH
#define OTFTOTFM_OTFTOTFM_HH
#include <lcdf/string.hh>
class Metrics;
class FontInfo;
class StringAccum;
class ErrorHandler;

String suffix_font_name(const String &font_name, const String &suffix);

String installed_metrics_font_name(const String &font_name, const String &secondary);

void output_metrics(Metrics &metrics, const String &ps_name, int boundary_char,
        const FontInfo &finfo,
        const String &encoding_name, const String &encoding_file,
        const String &font_name,
        String (*dvips_include)(const String &ps_name, const FontInfo &, ErrorHandler *),
        ErrorHandler *errh);

double font_cap_height(const FontInfo &, const Transform &);
double font_ascender(const FontInfo &, const Transform &);
double font_slant(const FontInfo &);

#endif
