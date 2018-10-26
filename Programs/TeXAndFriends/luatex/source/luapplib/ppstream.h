
#ifndef PP_STREAM_H
#define PP_STREAM_H

ppstream * ppstream_create (ppdoc *pdf, ppdict *dict, size_t offset);
iof * ppstream_read (ppstream *stream, int decode, int all);
#define ppstream_iof(stream) ((iof *)((stream)->I))
void ppstream_info (ppstream *stream, ppdoc *pdf);

#endif