
#include "ppfilter.h"
#include "pplib.h"

ppstream * ppstream_create (ppdoc *pdf, ppdict *dict, size_t offset)
{
	ppstream *stream;
	stream = (ppstream *)ppheap_take(&pdf->heap, sizeof(ppstream));
	stream->dict = dict;
	stream->offset = offset;
	//if (!ppdict_rget_uint(dict, "Length", &stream->length)) // may be indirect pointing PPNONE at this moment
	//  stream->length = 0;
	stream->length = 0;
	stream->input = &pdf->input;
	stream->I = NULL;
	stream->cryptkey = NULL;
	stream->flags = 0;
	return stream;
}

/* codecs */

enum {
  PPSTREAM_UNKNOWN = -1,
  PPSTREAM_BASE16 = 0,
  PPSTREAM_BASE85,
  PPSTREAM_RUNLENGTH,
  PPSTREAM_FLATE,
  PPSTREAM_LZW,
  PPSTREAM_CCITT,
  PPSTREAM_DCT,
  PPSTREAM_JBIG2,
  PPSTREAM_JPX,
  PPSTREAM_CRYPT
};

static int ppstream_codec_type (ppname name)
{ // one of those places where some hash wuld be nice..
  switch (name[0])
  {
    case 'A':
      if (ppname_is(name, "ASCIIHexDecode")) return PPSTREAM_BASE16;
      if (ppname_is(name, "ASCII85Decode")) return PPSTREAM_BASE85;
      break;
    case 'R':
      if (ppname_is(name, "RunLengthDecode")) return PPSTREAM_RUNLENGTH;
      break;
    case 'F':
      if (ppname_is(name, "FlateDecode")) return PPSTREAM_FLATE;
      break;
    case 'L':
      if (ppname_is(name, "LZWDecode")) return PPSTREAM_LZW;
      break;
    case 'D':
      if (ppname_is(name, "DCTDecode")) return PPSTREAM_DCT;
      break;
    case 'C':
      if (ppname_is(name, "CCITTFaxDecode")) return PPSTREAM_CCITT;
      if (ppname_is(name, "Crypt")) return PPSTREAM_CRYPT;
      break;
    case 'J':
      if (ppname_is(name, "JPXDecode")) return PPSTREAM_JPX;
      if (ppname_is(name, "JBIG2Decode")) return PPSTREAM_JBIG2;
      break;
  }
  return PPSTREAM_UNKNOWN;
}

static iof * ppstream_predictor (ppdict *params, iof *N)
{
  ppint predictor, rowsamples, components, samplebits;

  if (!ppdict_get_int(params, "Predictor", &predictor) || predictor <= 1)
    return N;
  if (!ppdict_get_int(params, "Columns", &rowsamples) || rowsamples == 0) // sanity, filter probably expects >0
    rowsamples = 1;;
  if (!ppdict_get_int(params, "Colors", &components) || components == 0) // ditto
    components = 1;
  if (!ppdict_get_int(params, "BitsPerComponent", &samplebits) || samplebits == 0)
    samplebits = 8;
  return iof_filter_predictor_decoder(N, (int)predictor, (int)rowsamples, (int)components, (int)samplebits);
}

static iof * ppstream_decoder (ppstream *stream, int codectype, ppdict *params, iof *N)
{
  int flags;
  iof *F, *P;
  ppint earlychange;
  ppstring cryptkey;

  switch (codectype)
  {
    case PPSTREAM_BASE16:
      return iof_filter_base16_decoder(N);
    case PPSTREAM_BASE85:
      return iof_filter_base85_decoder(N);
    case PPSTREAM_RUNLENGTH:
      return iof_filter_runlength_decoder(N);
    case PPSTREAM_FLATE:
      if ((F = iof_filter_flate_decoder(N)) != NULL)
      {
        if (params != NULL)
        {
          if ((P = ppstream_predictor(params, F)) != NULL)
            return P;
          iof_close(F);
          break;
        }
        return F;
      }
      break;
    case PPSTREAM_LZW:
      flags = LZW_DECODER_DEFAULTS;
      if (params != NULL && ppdict_get_int(params, "EarlyChange", &earlychange) && earlychange == 0) // integer, not boolean
        flags &= ~LZW_EARLY_INDEX;
      if ((F = iof_filter_lzw_decoder(N, flags)) != NULL)
      {
        if (params != NULL)
        {
          if ((P = ppstream_predictor(params, F)) != NULL)
            return P;
          iof_close(F);
          break;
        }
        return F;
      }
      break;
    case PPSTREAM_CRYPT:
      if ((cryptkey = stream->cryptkey) == NULL)
        return N; // /Identity crypt
      if (stream->flags & PPSTREAM_ENCRYPTED_AES)
        return iof_filter_aes_decoder(N, cryptkey, ppstring_size(cryptkey));
      if (stream->flags & PPSTREAM_ENCRYPTED_RC4)
        return iof_filter_rc4_decoder(N, cryptkey, ppstring_size(cryptkey));
      return NULL; // if neither AES or RC4 but cryptkey present, something went wrong; see ppstream_info()
    case PPSTREAM_CCITT:
    case PPSTREAM_DCT:
    case PPSTREAM_JBIG2:
    case PPSTREAM_JPX:
    case PPSTREAM_UNKNOWN:
      break;
  }
  return NULL;
}

#define ppstream_image(type) (type == PPSTREAM_DCT || type == PPSTREAM_JBIG2 || PPSTREAM_JPX)

#define ppstream_source(stream) iof_filter_stream_coreader((iof_file *)((stream)->input), (size_t)((stream)->offset), (size_t)((stream)->length))
#define ppstream_auxsource(filename) iof_filter_file_reader(filename)

static ppname ppstream_filter_name (ppobj *filterobj, size_t index)
{
  if (filterobj->type == PPNAME)
    return index == 0 ? filterobj->name : NULL;
  if (filterobj->type == PPARRAY)
    return pparray_get_name(filterobj->array, index);
  return NULL;
}

static ppdict * ppstream_filter_params (ppobj *paramsobj, size_t index)
{
  if (paramsobj->type == PPDICT)
    return index == 0 ? paramsobj->dict : NULL;
  if (paramsobj->type == PPARRAY)
    return pparray_rget_dict(paramsobj->array, index);
  return NULL;
}

static const char * ppstream_aux_filename (ppobj *filespec)
{ // mockup, here we should decode the string
  if (filespec->type == PPSTRING)
  {
    return (const char *)(filespec->string);
  }
  // else might be a dict - todo
  return NULL;
}

iof * ppstream_read (ppstream *stream, int decode, int all)
{
  ppdict *dict;
  iof *I, *F;
  int codectype, external, owncrypt;
  ppobj *filterobj, *paramsobj, *filespecobj;
  ppname filter;
  ppdict *params;
  size_t index;
  const char *filename;

  if (ppstream_iof(stream) != NULL)
    return NULL; // usage error

  dict = stream->dict;
  if ((filespecobj = ppdict_rget_obj(dict, "F")) != NULL)
  {
    filename = ppstream_aux_filename(filespecobj);
    I = filename != NULL ? ppstream_auxsource(filename) : NULL;
    external = 1;
  }
  else
  {
    I = ppstream_source(stream);
    external = 0;
  }
  if (I == NULL)
    return NULL;
  /* If the stream is encrypted, decipher is the first to be applied */
  owncrypt = (stream->flags & PPSTREAM_ENCRYPTED_OWN) != 0;
  if (!owncrypt)
  {
    if (stream->cryptkey != NULL)
    { /* implied global crypt */
      if ((F = ppstream_decoder(stream, PPSTREAM_CRYPT, NULL, I)) == NULL)
        goto stream_error;
      I = F;
    } /* otherwise no crypt at all or /Identity */
  }
  if (decode || owncrypt)
  {
    filterobj = ppdict_rget_obj(dict, external ? "FFilter" : "Filter");
    if (filterobj != NULL)
    {
      paramsobj = ppdict_rget_obj(dict, external ? "FDecodeParms" : "DecodeParms");
      for (index = 0, filter = ppstream_filter_name(filterobj, 0); filter != NULL; filter = ppstream_filter_name(filterobj, ++index))
      {
        params = paramsobj != NULL ? ppstream_filter_params(paramsobj, index) : NULL;
        codectype = ppstream_codec_type(filter);
        if ((F = ppstream_decoder(stream, codectype, params, I)) != NULL)
        {
          I = F;
          if (owncrypt && !decode && codectype == PPSTREAM_CRYPT)
            break; // /Crypt filter should always be first, so in practise we return decrypted but compressed
          continue;
        }
        if (!ppstream_image(codectype)) // something unexpected
          goto stream_error;
        else // just treat image data (jpeg/jbig) as the target data
          break;
      }
    }
  }
  if (all)
    iof_load(I);
  else
    iof_input(I);
  stream->I = I;
  return I;
stream_error:
  iof_close(I);
  return NULL;
}

uint8_t * ppstream_first (ppstream *stream, size_t *size, int decode)
{
  iof *I;
  if ((I = ppstream_read(stream, decode, 0)) != NULL)
  {
    *size = (size_t)iof_left(I);
    return I->pos;
  }
  *size = 0;
  return NULL;
}

uint8_t * ppstream_next (ppstream *stream, size_t *size)
{
  iof *I;
  if ((I = ppstream_iof(stream)) != NULL)
  {
    I->pos = I->end;
    if ((*size = iof_input(I)) > 0)
      return I->pos;
  }
  *size = 0;
  return NULL;
}

uint8_t * ppstream_all (ppstream *stream, size_t *size, int decode)
{
  iof *I;
  if ((I = ppstream_read(stream, decode, 1)) != NULL)
  {
    *size = (size_t)iof_left(I);
    return I->pos;
  }
  *size = 0;
  return NULL;
}

void ppstream_done (ppstream *stream)
{
  iof *I;
  if ((I = ppstream_iof(stream)) != NULL)
  {
    iof_close(I);
    stream->I = NULL;
  }
}

/* */

void ppstream_init_buffers (void)
{
	iof_filters_init();
}

void ppstream_free_buffers (void)
{
	iof_filters_free();
}
