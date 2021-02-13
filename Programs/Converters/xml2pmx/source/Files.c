
#include "obx.h"

void FmtInt(FILE *fp, int n, int w) {
     fprintf(fp, "%*d", w, n);
}

void FmtLong(FILE *fp, longint n, int w) {
#ifdef __MINGW32__
     const char *fmt = "%*I64d";
#else
     const char *fmt = "%*lld";
#endif
     fprintf(fp, fmt, w, n);
}

void FmtReal(FILE *fp, float x) {
     fprintf(fp, "%#G", x);
}

void FmtLongReal(FILE *fp, double x) {
     fprintf(fp, "%#.12G", x);
}

void FmtFixed(FILE *fp, double x, int width, int dec) {
     fprintf(fp, "%*.*f", width, dec, x);
}

void FmtString(FILE *fp, char *s, int len) {
     fprintf(fp, "%.*s", len, s);
}

void InitFiles(value *in, value *out, value *err) {
     put_long(in, (ptrtype) stdin); 
     put_long(out, (ptrtype) stdout);
     put_long(err, (ptrtype) stderr);
}

