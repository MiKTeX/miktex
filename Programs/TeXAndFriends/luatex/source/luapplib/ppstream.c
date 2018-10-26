
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
	stream->filespec = NULL;
	stream->filter.filters = NULL;
	stream->filter.params = NULL;
	stream->filter.count = 0;
	stream->input = &pdf->input;
	stream->I = NULL;
	stream->cryptkey = NULL;
	stream->flags = 0;
	return stream;
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

static iof * ppstream_decoder (ppstream *stream, ppstreamtp filtertype, ppdict *params, iof *N)
{
  int flags;
  iof *F, *P;
  ppint earlychange;
  ppstring cryptkey;

  switch (filtertype)
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
      break;
  }
  return NULL;
}

#define ppstream_source(stream) iof_filter_stream_coreader((iof_file *)((stream)->input), (size_t)((stream)->offset), (size_t)((stream)->length))
#define ppstream_auxsource(filename) iof_filter_file_reader(filename)

static ppname ppstream_get_filter_name (ppobj *filterobj, size_t index)
{
  if (filterobj->type == PPNAME)
    return index == 0 ? filterobj->name : NULL;
  if (filterobj->type == PPARRAY)
    return pparray_get_name(filterobj->array, index);
  return NULL;
}

static ppdict * ppstream_get_filter_params (ppobj *paramsobj, size_t index)
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

#define ppstream_image_filter(fcode) (fcode == PPSTREAM_DCT || fcode == PPSTREAM_CCITT || fcode == PPSTREAM_JBIG2 || fcode == PPSTREAM_JPX)

iof * ppstream_read (ppstream *stream, int decode, int all)
{
  iof *I, *F;
  ppstreamtp *filtertypes, filtertype;
  int owncrypt;
  ppdict **filterparams, *fparams;
  size_t index, filtercount;
  const char *filename;

  if (ppstream_iof(stream) != NULL)
    return NULL; // usage error

  if (stream->filespec != NULL)
  {
    filename = ppstream_aux_filename(stream->filespec); // mockup, basic support
    I = filename != NULL ? ppstream_auxsource(filename) : NULL;
  }
  else
  {
    I = ppstream_source(stream);
  }
  if (I == NULL)
    return NULL;

  /* If the stream is encrypted, decipher is the first to be applied */
  owncrypt = (stream->flags & PPSTREAM_ENCRYPTED_OWN) != 0;
  if (!owncrypt)
  {
    if (stream->cryptkey != NULL && stream->filespec == NULL)
    { /* implied global crypt; does not apply to external files (pdf psec page 115), except for embedded file streams (not supported so far) */
      if ((F = ppstream_decoder(stream, PPSTREAM_CRYPT, NULL, I)) == NULL)
        goto stream_error;
      I = F;
    } /* otherwise no crypt at all or /Identity */
  }

  if (decode || owncrypt)
  {
    if ((filtercount = stream->filter.count) > 0)
    {
      filtertypes = stream->filter.filters;
      filterparams = stream->filter.params;
      for (index = 0; index < filtercount; ++index)
      {
        fparams = filterparams != NULL ? filterparams[index] : NULL;
        filtertype = filtertypes[index];
        if ((F = ppstream_decoder(stream, filtertype, fparams, I)) != NULL)
        {
          I = F;
          if (owncrypt && !decode && filtertype == PPSTREAM_CRYPT)
            break; // /Crypt filter should always be first, so in practise we return decrypted but compressed
          continue;
        }
        if (!ppstream_image_filter(filtertype))
          goto stream_error; // failed to create non-image filter, something unexpected
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

/* fetching stream info
PJ20190916: revealed it makes sense to do a lilbit more just after parsing stream entry to simplify stream operations
and extend ppstream api
*/

/* stream filters */

const char * ppstream_filter_name[] = {
  "ASCIIHexDecode",
  "ASCII85Decode",
  "RunLengthDecode",
  "FlateDecode",
  "LZWDecode",
  "CCITTFaxDecode",
  "DCTDecode",
  "JBIG2Decode",
  "JPXDecode",
  "Crypt"
};

int ppstream_filter_type (ppname name, ppstreamtp *filtertype)
{
  switch (name[0])
  {
    case 'A':
      if (ppname_is(name, "ASCIIHexDecode")) { *filtertype = PPSTREAM_BASE16; return 1; }
      if (ppname_is(name, "ASCII85Decode")) { *filtertype = PPSTREAM_BASE85; return 1; }
      break;
    case 'R':
      if (ppname_is(name, "RunLengthDecode")) { *filtertype = PPSTREAM_RUNLENGTH; return 1; }
      break;
    case 'F':
      if (ppname_is(name, "FlateDecode")) { *filtertype = PPSTREAM_FLATE; return 1; }
      break;
    case 'L':
      if (ppname_is(name, "LZWDecode")) { *filtertype = PPSTREAM_LZW; return 1; }
      break;
    case 'D':
      if (ppname_is(name, "DCTDecode")) { *filtertype = PPSTREAM_DCT; return 1; }
      break;
    case 'C':
      if (ppname_is(name, "CCITTFaxDecode")) { *filtertype = PPSTREAM_CCITT; return 1; }
      if (ppname_is(name, "Crypt")) { *filtertype = PPSTREAM_CRYPT; return 1; }
      break;
    case 'J':
      if (ppname_is(name, "JPXDecode")) { *filtertype = PPSTREAM_JPX; return 1; }
      if (ppname_is(name, "JBIG2Decode")) { *filtertype = PPSTREAM_JBIG2; return 1; }
      break;
  }
  return 0;
}

void ppstream_info (ppstream *stream, ppdoc *pdf)
{ // called in ppdoc_load_entries() for every stream, but after loading non-stream objects (eg. /Length..)
  ppdict *dict, *fparams;
  ppobj *fobj, *pobj;
  ppname fname, tname, owncryptfilter = NULL;
  ppcrypt *crypt;
  ppref *ref;
  size_t i;
  int cflags;

  ppstreamtp *filtertypes = NULL, filtertype;
  ppdict **filterparams = NULL;
  size_t filtercount = 0, farraysize = 0;

  const char *filterkey, *paramskey;

  dict = stream->dict;
  ppdict_rget_uint(dict, "Length", &stream->length);

  if ((stream->filespec = ppdict_get_obj(dict, "F")) != NULL)
  {
    stream->flags |= PPSTREAM_NOT_SUPPORTED;
    filterkey = "FFilter", paramskey = "FDecodeParms";
  }
  else
    filterkey = "Filter", paramskey = "DecodeParms";

  if ((fobj = ppdict_rget_obj(dict, filterkey)) != NULL)
  {
    switch (fobj->type)
    {
      case PPNAME:
        farraysize = 1;
        break;
      case PPARRAY:
        farraysize = fobj->array->size;
        break;
      default:
        break;
    }
    if (farraysize > 0)
    {
      filtertypes = ppheap_take(&pdf->heap, farraysize * sizeof(ppstreamtp));
      if ((pobj = ppdict_rget_obj(dict, paramskey)) != NULL)
        filterparams = ppheap_take(&pdf->heap, farraysize * sizeof(ppdict *));
      for (i = 0; i < farraysize; ++i)
      {
        if ((fname = ppstream_get_filter_name(fobj, i)) != NULL && ppstream_filter_type(fname, &filtertype))
        {
          filtertypes[filtercount] = filtertype;
          if (pobj != NULL)
          {
            fparams = ppstream_get_filter_params(pobj, i);
            filterparams[filtercount] = fparams;
          }
          else
            fparams = NULL;
          switch (filtertype)
          {
            case PPSTREAM_BASE16:
            case PPSTREAM_BASE85:
            case PPSTREAM_RUNLENGTH:
            case PPSTREAM_FLATE:
            case PPSTREAM_LZW:
              stream->flags |= PPSTREAM_FILTER;
              break;
            case PPSTREAM_CCITT:
            case PPSTREAM_DCT:
            case PPSTREAM_JBIG2:
            case PPSTREAM_JPX:
              stream->flags |= PPSTREAM_IMAGE;
              break;
            case PPSTREAM_CRYPT:
              stream->flags |= PPSTREAM_ENCRYPTED_OWN;
              owncryptfilter = fparams != NULL ? ppdict_get_name(fparams, "Name") : NULL; // /Type /CryptFilterDecodeParms /Name ...
              if (i != 0) // we assume it is first
                stream->flags |= PPSTREAM_NOT_SUPPORTED;
              break;
          }
          ++filtercount;
        }
        else
        {
          stream->flags |= PPSTREAM_NOT_SUPPORTED;
        }
      }
    }
  }
  stream->filter.filters = filtertypes;
  stream->filter.params = filterparams;
  stream->filter.count = filtercount;

  if ((crypt = pdf->crypt) == NULL || (ref = crypt->ref) == NULL)
    return;
  if (stream->flags & PPSTREAM_ENCRYPTED_OWN)
  {
    /* Seems a common habit to use just /Crypt filter name with no params, which defaults to /Identity.
       A real example with uncompressed metadata: <</Filter[/Crypt]/Length 4217/Subtype/XML/Type/Metadata>> */
    if (owncryptfilter != NULL && !ppname_is(owncryptfilter, "Identity") && stream->filespec == NULL) // ?
    {
      if (crypt->map != NULL && ppcrypt_type(crypt, owncryptfilter, NULL, &cflags))
      {
        if (cflags & PPCRYPT_INFO_AES)
          stream->flags |= PPSTREAM_ENCRYPTED_AES;
        else if (cflags & PPCRYPT_INFO_RC4)
          stream->flags |= PPSTREAM_ENCRYPTED_RC4;
      }
    }
  }
  else
  {
    if ((crypt->flags & PPCRYPT_NO_METADATA) && (tname = ppdict_get_name(dict, "Type")) != NULL && ppname_is(tname, "Metadata"))
      ; /* special treatment of metadata stream; we assume that explicit /Filter /Crypt setup overrides document level setup of EncryptMetadata. */
    else if (stream->filespec == NULL) /* external files are not encrypted, expect embedded files (not supported yet) */
    {
      if (crypt->flags & PPCRYPT_STREAM_RC4)
        stream->flags |= PPSTREAM_ENCRYPTED_RC4;
      else if (crypt->flags & PPCRYPT_STREAM_AES)
        stream->flags |= PPSTREAM_ENCRYPTED_AES;
    }
  }

  /* finally, if the stream is encrypted with non-identity crypt (implicit or explicit), make and save the crypt key */
  if (stream->flags & PPSTREAM_ENCRYPTED)
    stream->cryptkey = ppcrypt_stmkey(crypt, ref, ((stream->flags & PPSTREAM_ENCRYPTED_AES) != 0), &pdf->heap);
}

void ppstream_filter_info (ppstream *stream, ppstream_filter *info, int decode)
{
  size_t from, index;
  ppstreamtp filtertype;
  ppdict *params;

  *info = stream->filter;
  if (info->count > 0)
  {
    from = (stream->flags & PPSTREAM_ENCRYPTED_OWN) && info->filters[0] == PPSTREAM_CRYPT ? 1 : 0;
    if (decode)
    {
      for (index = from; index < info->count; ++index)
      {
        filtertype = info->filters[index];
        if (ppstream_image_filter(filtertype))
        {
          break;
        }
      }
    }
    else
    {
      index = from;
    }
    if (index > 0) {
      info->count -= index;
      if (info->count > 0)
      {
        info->filters += index;
        if (info->params != NULL)
        {
          info->params += index;
          for (index = 0, params = NULL; index < info->count; ++index)
            if ((params = info->params[index]) != NULL)
              break;
          if (params == NULL)
            info->params = NULL;
        }
      }
      else
      {
        info->filters = NULL;
        info->params = NULL;
      }
    }
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
