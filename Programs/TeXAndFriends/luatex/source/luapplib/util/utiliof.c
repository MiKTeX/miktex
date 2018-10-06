/* input/iutput stream */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utilmem.h"
#include "utillog.h"
#include "utiliof.h"

/* commons */

void * iof_copy_data (const void *data, size_t size)
{
  return memcpy(util_malloc(size), data, size);
}

uint8_t * iof_copy_file_data (const char *filename, size_t *psize)
{
  FILE *file;
  size_t size;
  uint8_t *data;
  if ((file = fopen(filename, "rb")) == NULL)
    return NULL;
  fseek(file, 0, SEEK_END);
  size = (size_t)ftell(file);
  data = (uint8_t *)util_malloc(size);
  fseek(file, 0, SEEK_SET);
  if ((*psize = fread(data, 1, size, file)) != size)
  {
    util_free(data);
    data = NULL;
  }
  fclose(file);
  return data;
}

uint8_t * iof_copy_file_handle_data (FILE *file, size_t *psize)
{
  size_t size;
  uint8_t *data;
  //long offset = ftell(file); // keep offset intact?
  fseek(file, 0, SEEK_END);
  size = (size_t)ftell(file);
  data = (uint8_t *)util_malloc(size);
  fseek(file, 0, SEEK_SET);
  if ((*psize = fread(data, 1, size, file)) != size)
  {
    util_free(data);
    data = NULL;
  }
  //fseek(file, offset, SEEK_SET)
  return data;
}

FILE * iof_get_file (iof *F)
{
  if (F->flags & IOF_FILE)
    return iof_file_get_file(F->iofile);
  if (F->flags & IOF_FILE_HANDLE)
    return F->file;
  return NULL;
}

const char * iof_status_kind (iof_status status)
{
  switch (status)
  {
    case IOFEOF:
      return "IOFEOF";
    case IOFERR:
      return "IOFERR";
    case IOFEMPTY:
      return "IOFEMPTY";
    case IOFFULL:
      return "IOFFULL";
    default:
      break;
  }
  return "(unknown)";
}

/* shared pseudofile */

#define IOF_FILE_DEFAULTS 0

iof_file * iof_file_new (FILE *file)
{
  iof_file *iofile = (iof_file *)util_malloc(sizeof(iof_file));
  iof_file_set_fh(iofile, file);
  iofile->offset = NULL;
  iofile->size = 0;
  iofile->name = NULL;
  iofile->refcount = 0;
  iofile->flags = IOF_FILE_DEFAULTS|IOF_ALLOC;
  return iofile;
}

iof_file * iof_file_init (iof_file *iofile, FILE *file)
{
  iof_file_set_fh(iofile, file);
  iofile->offset = NULL;
  iofile->size = 0;
  iofile->name = NULL;
  iofile->refcount = 0;
  iofile->flags = IOF_FILE_DEFAULTS;
  return iofile;
}

iof_file * iof_file_rdata (const void *data, size_t size)
{
  iof_file *iofile = (iof_file *)util_malloc(sizeof(iof_file));
  iofile->rbuf = iofile->rpos = (const uint8_t *)data;
  iofile->rend = iofile->rbuf + size;
  iofile->offset = NULL;
  iofile->size = 0;
  iofile->name = NULL;
  iofile->refcount = 0;
  iofile->flags = IOF_FILE_DEFAULTS|IOF_ALLOC|IOF_DATA;
  return iofile;
}

iof_file * iof_file_rdata_init (iof_file *iofile, const void *data, size_t size)
{
  iofile->rbuf = iofile->rpos = (const uint8_t *)data;
  iofile->rend = iofile->rbuf + size;
  iofile->offset = NULL;
  iofile->size = 0; // letse keep it consequently set to zero (only for user disposal)
  iofile->name = NULL;
  iofile->refcount = 0;
  iofile->flags = IOF_FILE_DEFAULTS|IOF_DATA;
  return iofile;
}

iof_file * iof_file_wdata (void *data, size_t size)
{
  return iof_file_rdata((const void *)data, size);
}

iof_file * iof_file_wdata_init (iof_file *iofile, void *data, size_t size)
{
  return iof_file_rdata_init(iofile, (const void *)data, size);
}

/* typical uses so far */

iof_file * iof_file_reader_from_file_handle (iof_file *iofile, const char *filename, FILE *file, int preload, int closefile)
{
  uint8_t *data;
  size_t size;

  if (preload)
  {
    if ((data = iof_copy_file_handle_data(file, &size)) == NULL)
    {
      if (closefile)
        fclose(file);
      return NULL;
    }
    if (iofile == NULL)
      iofile = iof_file_rdata(data, size);
    else
      iof_file_rdata_init(iofile, data, size);
    iofile->flags |= IOF_BUFFER_ALLOC;
    if (closefile)
      fclose(file);
  }
  else
  {
    if (iofile == NULL)
      iofile = iof_file_new(file);
    else
      iof_file_init(iofile, file);
    if (closefile)
      iofile->flags |= IOF_CLOSE_FILE;
  }
  if (filename != NULL)
    iof_file_set_name(iofile, filename);
  return iofile;
}

iof_file * iof_file_reader_from_file (iof_file *iofile, const char *filename, int preload)
{
  FILE *file;
  if ((file = fopen(filename, "rb")) == NULL)
    return NULL;
  return iof_file_reader_from_file_handle(iofile, filename, file, preload, 1);
}

iof_file * iof_file_reader_from_data (iof_file *iofile, const void *data, size_t size, int preload, int freedata)
{
  void *newdata;
  if (data == NULL)
    return NULL;
  if (preload)
  {
    newdata = iof_copy_data(data, size);
    if (iofile == NULL)
      iofile = iof_file_rdata(newdata, size);
    else
      iof_file_rdata_init(iofile, newdata, size);
    iofile->flags |= IOF_BUFFER_ALLOC;
    //if (freedata) // hardly makes sense...  we can't free const void *
    //  util_free((void *)data);
  }
  else
  {
    if (iofile == NULL)
      iofile = iof_file_rdata(data, size);
    else
      iof_file_rdata_init(iofile, data, size);
    if (freedata)
      iofile->flags |= IOF_BUFFER_ALLOC;
  }
  return iofile;
}

/*
iof_file * iof_file_writer_from_file (iof_file *iofile, const char *filename)
{
  FILE *file;
  if ((file = fopen(filename, "wb")) == NULL)
    return NULL;
  if (iofile == NULL)
    iofile = iof_file_new(file);
  else
    iof_file_init(iofile, file);
  iofile->flags |= IOF_CLOSE_FILE;
  iof_file_set_name(iofile, filename);
  return iofile;
}
*/

/*
Because of limited number of FILE* handles available, we may need to close contained handle
between accessing it. In applications so far (fonts, images) we typically need the source
to parse the file on creation and to rewrite or reload the data on dump. All iof_file api
functions assume that iofile has FILE* opened. Reopening it on every access (ftell, fseek,
read/write) makes no sense, as we would effectively loose control. If the caller invalidates
iofile by closing and nulling its file handle, it is also responsible to reopen when necessary.
*/

int iof_file_close_input (iof_file *iofile)
{
  FILE *file;
  if (iofile->flags & IOF_DATA)
    return 0;
  if ((file = iof_file_get_fh(iofile)) == NULL)
    return 0;
  fclose(file);
  iof_file_set_fh(iofile, NULL);
  iofile->flags &= ~IOF_RECLOSE_FILE;
  iofile->flags |= IOF_REOPEN_FILE;
  return 1;
}

int iof_file_reopen_input (iof_file *iofile)
{ // returns true if iofile readable
  FILE *file;
  const char *filename;
  if (iofile->flags & IOF_DATA)
    return 1;
  if ((file = iof_file_get_fh(iofile)) != NULL)
    return 1; // if present, assumed readable
  if ((filename = iofile->name) == NULL || (file = fopen(filename, "rb")) == NULL)
    return 0;
  iof_file_set_fh(iofile, file);
  iofile->flags &= ~IOF_REOPEN_FILE;
  iofile->flags |= IOF_RECLOSE_FILE;
  return 1;
}

/* freeing iof_file */

void iof_file_free (iof_file *iofile)
{
  FILE *file;
  if (iofile->flags & IOF_DATA)
  {
    if (iofile->flags & IOF_BUFFER_ALLOC)
    {
      iofile->flags &= ~IOF_BUFFER_ALLOC;
      if (iofile->buf != NULL)
      {
        util_free(iofile->buf);
        iofile->buf = iofile->pos = iofile->end = NULL;
      }
    }
  }
  else if ((file = iof_file_get_fh(iofile)) != NULL)
  {
    if (iofile->flags & IOF_CLOSE_FILE)
     	fclose(file);
    iof_file_set_fh(iofile, NULL);
  }
  iof_file_set_name(iofile, NULL);
  if (iofile->flags & IOF_ALLOC)
    util_free(iofile);
}

/* set filename for reopen */

void iof_file_set_name (iof_file *iofile, const char *name)
{
  if (iofile->name != NULL)
    util_free(iofile->name);
  if (name != NULL)
    iofile->name = iof_copy_data(name, strlen(name) + 1);
  else
    iofile->name = NULL;
}

/* seek */

int iof_file_seek (iof_file *iofile, long offset, int whence)
{
  if (iofile->flags & IOF_DATA)
  {
    switch (whence)
    {
      case SEEK_SET:
        if (offset >= 0 && iofile->buf + offset <= iofile->end)
        {
          iofile->pos = iofile->buf + offset;
          return 0;
        }
        return -1;
      case SEEK_CUR:
        if ((offset >= 0 && iofile->pos + offset <= iofile->end) || (offset < 0 && iofile->pos + offset >= iofile->buf))
        {
          iofile->pos += offset;
          return 0;
        }
        return -1;
      case SEEK_END:
        if (offset <= 0 && iofile->end + offset >= iofile->buf)
        {
          iofile->pos = iofile->end + offset;
          return 0;
        }
        return -1;
    }
    return -1;
  }
  return fseek(iof_file_get_fh(iofile), offset, whence);
}

/* */

long iof_file_tell (iof_file *iofile)
{
  return (iofile->flags & IOF_DATA) ? (long)(iofile->pos - iofile->buf) : ftell(iof_file_get_fh(iofile));
}

size_t iof_file_size (iof_file *iofile)
{ 
  long pos, size;
  FILE *file;
  if (iofile->flags & IOF_DATA)
    return (size_t)iof_space(iofile);
  file = iof_file_get_fh(iofile);
  pos = ftell(file);
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, pos, SEEK_SET);
  return size;
}

int iof_file_eof (iof_file *iofile)
{
  if (iofile->flags & IOF_DATA)
    return iofile->pos == iofile->end ? -1 : 0;
  return feof(iof_file_get_fh(iofile));
}

int iof_file_flush (iof_file *iofile)
{
  if (iofile->flags & IOF_DATA)
    return 0;
  return fflush(iof_file_get_fh(iofile));
}

size_t iof_file_read (void *ptr, size_t size, size_t items, iof_file *iofile)
{
  if (iofile->flags & IOF_DATA)
  {
    size_t bytes = size * items;
    if (bytes > (size_t)iof_left(iofile))
      bytes = (size_t)iof_left(iofile);
    memcpy(ptr, iofile->pos, bytes);
    iofile->pos += bytes;
    return bytes / size; // number of elements read
  }
  return fread(ptr, size, items, iof_file_get_fh(iofile));
}

static size_t iof_file_data_resizeto (iof_file *iofile, size_t space)
{
  uint8_t *newbuf;
  size_t size;
  size = iof_size(iofile);
  if (iofile->flags & IOF_BUFFER_ALLOC)
  {
    newbuf = (uint8_t *)util_realloc(iofile->buf, space);
  }
  else
  {
    newbuf = (uint8_t *)util_malloc(space);
    if (size > 0)
      memcpy(newbuf, iofile->buf, size);
    iofile->flags |= IOF_BUFFER_ALLOC;
  }
  iofile->buf = newbuf;
  iofile->pos = newbuf + size;
  iofile->end = newbuf + space;
  return space - size;
}

#define iof_file_data_resize(iofile) iof_file_data_resizeto(iofile, iof_space(iofile) << 1)

size_t iof_file_write (const void *ptr, size_t size, size_t items, iof_file *iofile)
{
  if (iofile->flags & IOF_DATA)
  {
    size_t space, sizesofar, bytes;
    bytes = size * items;
    if (bytes > (size_t)iof_left(iofile))
    {      
      if ((space = iof_space(iofile)) == 0) // allow iofile->buf/end initially NULL
        space = BUFSIZ;
      for (sizesofar = iof_size(iofile), space <<= 1; sizesofar + bytes > space; space <<= 1)
        ;
      if (iof_file_data_resizeto(iofile, space) == 0)
        return 0;
    }
    memcpy(iofile->pos, ptr, bytes);
    iofile->pos += bytes;
    return bytes / size;
  }
  return fwrite(ptr, size, items, iof_file_get_fh(iofile));
}

size_t iof_file_ensure (iof_file *iofile, size_t bytes)
{
  if (iofile->flags & IOF_DATA)
  {
    size_t space, sizesofar, left;
    left = (size_t)iof_left(iofile);
    if (bytes > left)
    {      
      if ((space = iof_space(iofile)) == 0) // allow iofile->buf/end initially NULL
        space = BUFSIZ;
      for (sizesofar = iof_size(iofile), space <<= 1; sizesofar + bytes > space; space <<= 1);
      return iof_file_data_resizeto(iofile, space);
    }
    return left;  
  }
  return 0;
}

int iof_file_getc (iof_file *iofile)
{
  if (iofile->flags & IOF_DATA)
    return iofile->pos < iofile->end ? *iofile->pos++ : IOFEOF;
  return fgetc(iof_file_get_fh(iofile));
}

int iof_file_putc (iof_file *iofile, int c)
{
  if (iofile->flags & IOF_DATA)
  {
    if (iofile->pos >= iofile->end)
      if (iof_file_data_resize(iofile) == 0)
        return IOFEOF;
    *iofile->pos++ = (uint8_t)c;
    return c;
  }
  return fputc(c, iof_file_get_fh(iofile));
}

static int iof_file_sync (iof_file *iofile, size_t *offset)
{
  if (iofile->offset != offset)
  {
    if (iofile->offset != NULL)
      *iofile->offset = iof_file_tell(iofile);
    iofile->offset = offset;
    if (offset) // let offset be NULL
      return iof_file_seek(iofile, (long)*offset, SEEK_SET);
  }
  return 0;
}

//#define iof_file_unsync(iofile, poffset) (void)((iofile)->offset == poffset && (((iofile)->offset = NULL), 0))
#define iof_file_unsync(iofile, poffset) ((void)poffset, (iofile)->offset = NULL)

/* iof seek */

#define iof_reader_reset(I) ((I)->pos = (I)->end = (I)->buf)
#define iof_reader_reseek_file(I, offset, whence) (fseek((I)->file, offset, whence) == 0 ? (iof_reader_reset(I), 0) : -1)
#define iof_reader_reseek_iofile(I, offset, whence) (iof_file_seek((I)->iofile, offset, whence) == 0 ? (iof_reader_reset(I), 0) : -1)

#define iof_writer_reset(O) ((O)->pos = (O)->buf)
#define iof_writer_reseek_file(O, offset, whence) (iof_flush(O), (fseek((O)->file, offset, whence) == 0 ? (iof_writer_reset(O), 0) : -1))
#define iof_writer_reseek_iofile(O, offset, whence) (iof_flush(O), (iof_file_seek((O)->iofile, offset, whence) == 0 ? (iof_writer_reset(O), 0) : -1))

static int iof_reader_seek_data (iof *I, long offset, int whence)
{
  switch (whence)
  {
    case SEEK_SET:
      if (offset >= 0 && I->buf + offset <= I->end)
      {
        I->pos = I->buf + offset;
        return 0;
      }
      return -1;
    case SEEK_CUR:
      if ((offset >= 0 && I->pos + offset <= I->end) || (offset < 0 && I->pos + offset >= I->buf))
      {
        I->pos += offset;
        return 0;
      }
      return -1;
    case SEEK_END:
      if (offset <= 0 && I->end + offset >= I->buf)
      {
        I->pos = I->end + offset;
        return 0;
      }
      return -1;
  }
  return -1;
}

static int iof_reader_seek_iofile (iof *I, long offset, int whence)
{
  long fileoffset;
  switch (whence)
  {
    case SEEK_SET:
      fileoffset = iof_file_tell(I->iofile);
      if (offset <= fileoffset && offset >= fileoffset - iof_space(I))
      {
        I->pos = I->end - (fileoffset - offset);
        return 0;
      }
      return iof_reader_reseek_iofile(I, offset, SEEK_SET);
    case SEEK_CUR:
      if ((offset >= 0 && I->pos + offset <= I->end) || (offset < 0 && I->pos + offset >= I->buf))
      {
        I->pos += offset;
        return 0;
      }
      return iof_reader_reseek_iofile(I, offset, SEEK_CUR);
    case SEEK_END:
      return iof_reader_reseek_iofile(I, offset, SEEK_END); // can we do better?
  }
  return -1;
}

static int iof_reader_seek_file (iof *I, long offset, int whence)
{
  long fileoffset;
  switch (whence)
  {
    case SEEK_SET:
      fileoffset = ftell(I->file);
      if (offset <= fileoffset && offset >= fileoffset - iof_space(I))
      {
        I->pos = I->end - (fileoffset - offset);
        return 0;
      }
      return iof_reader_reseek_file(I, offset, SEEK_SET);
    case SEEK_CUR:
      if ((offset >= 0 && I->pos + offset <= I->end) || (offset < 0 && I->pos + offset >= I->buf))
      {
        I->pos += offset;
        return 0;
      }
      return iof_reader_reseek_file(I, offset, SEEK_CUR);
    case SEEK_END:
      return iof_reader_reseek_file(I, offset, SEEK_END); // can we do better?
  }
  return -1;
}

int iof_reader_seek (iof *I, long offset, int whence)
{
  I->flags &= ~IOF_STOPPED;
  if (I->flags & IOF_FILE)
    return iof_reader_seek_iofile(I, offset, whence);
  if (I->flags & IOF_FILE_HANDLE)
    return iof_reader_seek_file(I, offset, whence);
  if (I->flags & IOF_DATA)
    return iof_reader_seek_data(I, offset, whence);
  return -1;
}

int iof_reader_reseek (iof *I, long offset, int whence)
{
  I->flags &= ~IOF_STOPPED;
  if (I->flags & IOF_FILE)
    return iof_reader_reseek_iofile(I, offset, whence);
  if (I->flags & IOF_FILE_HANDLE)
    return iof_reader_reseek_file(I, offset, whence);
  if (I->flags & IOF_DATA)
    return iof_reader_seek_data(I, offset, whence);
  return -1;
}

static int iof_writer_seek_data (iof *O, long offset, int whence)
{
  /*
  fseek() allows to seek after the end of file. Seeking does not increase the output file.
  No byte is written before fwirte(). It seems to fill the gap with zeros. Until we really need that,
  no seeking out of bounds for writers.
  */
  O->flags &= ~IOF_STOPPED;
  return iof_reader_seek_data(O, offset, whence);
}

static int iof_writer_seek_iofile (iof *O, long offset, int whence)
{
  long fileoffset;
  switch (whence)
  {
    case SEEK_SET:
      fileoffset = iof_file_tell(O->iofile);
      if (offset >= fileoffset && offset <= fileoffset + iof_space(O))
      {
        O->pos = O->buf + (offset - fileoffset);
        return 0;
      }
      return iof_writer_reseek_iofile(O, offset, SEEK_SET);
    case SEEK_CUR:
      if ((offset >=0 && O->pos + offset <= O->end) || (offset < 0 && O->pos + offset >= O->buf))
      {
        O->pos += offset;
        return 0;
      }
      return iof_writer_reseek_iofile(O, offset, SEEK_CUR);
    case SEEK_END:
      return iof_writer_reseek_iofile(O, offset, SEEK_END);
  }
  return -1;
}

static int iof_writer_seek_file (iof *O, long offset, int whence)
{
  long fileoffset;
  switch (whence)
  {
    case SEEK_SET:
      fileoffset = ftell(O->file);
      if (offset >= fileoffset && offset <= fileoffset + iof_space(O))
      {
        O->pos = O->buf + (offset - fileoffset);
        return 0;
      }
      return iof_writer_reseek_file(O, offset, SEEK_SET);
    case SEEK_CUR:
      if ((offset >=0 && O->pos + offset <= O->end) || (offset < 0 && O->pos + offset >= O->buf))
      {
        O->pos += offset;
        return 0;
      }
      return iof_writer_reseek_file(O, offset, SEEK_CUR);
    case SEEK_END:
      return iof_writer_reseek_file(O, offset, SEEK_END);
  }
  return -1;
}

int iof_writer_seek (iof *I, long offset, int whence)
{
  I->flags &= ~IOF_STOPPED;
  if (I->flags & IOF_FILE)
    return iof_writer_seek_iofile(I, offset, whence);
  if (I->flags & IOF_FILE_HANDLE)
    return iof_writer_seek_file(I, offset, whence);
  if (I->flags & IOF_DATA)
    return iof_writer_seek_data(I, offset, whence);
  return -1;
}

int iof_writer_reseek (iof *I, long offset, int whence)
{
  I->flags &= ~IOF_STOPPED;
  if (I->flags & IOF_FILE)
    return iof_writer_reseek_iofile(I, offset, whence);
  if (I->flags & IOF_FILE_HANDLE)
    return iof_writer_reseek_file(I, offset, whence);
  if (I->flags & IOF_DATA)
    return iof_writer_seek_data(I, offset, whence);
  return -1;
}

int iof_seek (iof *F, long offset, int whence)
{
  return (F->flags & IOF_WRITER) ? iof_writer_seek(F, offset, whence) : iof_reader_seek(F, offset, whence);
}

int iof_reseek (iof *F, long offset, int whence)
{
  return (F->flags & IOF_WRITER) ? iof_writer_reseek(F, offset, whence) : iof_reader_reseek(F, offset, whence);
}

/* tell */

long iof_reader_tell (iof *I)
{
  if (I->flags & IOF_FILE)
    return iof_file_tell(I->iofile) - (long)iof_left(I);
  if (I->flags & IOF_FILE_HANDLE)
    return ftell(I->file) - (long)iof_left(I);
  //if (I->flags & IOF_DATA)
  return (long)iof_size(I);
}

long iof_writer_tell (iof *O)
{
  if (O->flags & IOF_FILE)
    return iof_file_tell(O->iofile) + (long)iof_size(O);
  if (O->flags & IOF_FILE_HANDLE)
    return ftell(O->file) + (long)iof_size(O);
  //if (I->flags & IOF_DATA)
  return (long)iof_size(O);
}

long iof_tell (iof *I)
{
  return (I->flags & IOF_WRITER) ? iof_writer_tell(I) : iof_reader_tell(I);
}

size_t iof_fsize (iof *I)
{
  size_t pos, size;
  if (I->flags & IOF_FILE)
    return iof_file_size(I->iofile);
  if (I->flags & IOF_FILE_HANDLE)
  {
    pos = (size_t)ftell(I->file);
    fseek(I->file, 0, SEEK_END);
    size = (size_t)ftell(I->file);
    fseek(I->file, (long)pos, SEEK_SET);
    return size;
  }
  //if (I->flags & IOF_DATA)
  return (size_t)iof_space(I);
}

/* save reader tail */

size_t iof_save_tail (iof *I)
{
  size_t size, left;
  size = iof_size(I);
  left = iof_left(I);
  if (size >= left)
    memcpy(I->buf, I->pos, left);
  else
    memmove(I->buf, I->pos, left);
  return left;
}

size_t iof_input_save_tail (iof *I, size_t back)
{
  size_t size;
  I->flags |= IOF_TAIL;
  I->pos -= back;
  size = iof_input(I);
  I->pos += back;
  I->flags &= ~IOF_TAIL;
  return size; // + back - back
}

/* read from file */

/* iof free*/

static size_t file_read (iof *I);
static size_t file_load (iof *I);

static size_t file_reader (iof *I, iof_mode mode)
{
  switch (mode)
  {
    case IOFREAD:
      return file_read(I);
    case IOFLOAD:
      return file_load(I);
    case IOFCLOSE:
      iof_free(I);
      return 0;
    default:
      return 0;
  }
}

iof * iof_setup_file_handle_reader (iof *I, void *buffer, size_t space, FILE *f)
{
  if (I == NULL)
    iof_setup_reader(I, buffer, space);
  else
    iof_reader_buffer(I, buffer, space);
  iof_setup_file(I, f);
  I->more = file_reader;
  return I;
}

iof * iof_setup_file_reader (iof *I, void *buffer, size_t space, const char *filename)
{
  FILE *f;
  if ((f = fopen(filename, "rb")) == NULL)
    return NULL;
  if (I == NULL)
    iof_setup_reader(I, buffer, space);
  else
    iof_reader_buffer(I, buffer, space);
  iof_setup_file(I, f);
  I->flags |= IOF_CLOSE_FILE;
  I->more = file_reader;
  return I;
}

/* write to file */

static size_t file_write (iof *O, int flush);

static size_t file_writer (iof *O, iof_mode mode)
{
  switch (mode)
  {
    case IOFWRITE:
      return file_write(O, 0);
    case IOFFLUSH:
      return file_write(O, 1);
    case IOFCLOSE:
      file_write(O, 1);
      iof_free(O);
      return 0;
    default:
      return 0;
  }
}

iof * iof_setup_file_handle_writer (iof *O, void *buffer, size_t space, FILE *f)
{
  if (O == NULL)
    iof_setup_writer(O, buffer, space);
  else
    iof_writer_buffer(O, buffer, space);
  iof_setup_file(O, f);
  O->more = file_writer;
  return O;
}

iof * iof_setup_file_writer (iof *O, void *buffer, size_t space, const char *filename)
{
  FILE *f;
  if ((f = fopen(filename, "wb")) == NULL)
    return NULL;
  if (O == NULL)
    iof_setup_writer(O, buffer, space);
  else
    iof_writer_buffer(O, buffer, space);
  iof_setup_file(O, f);
  O->flags |= IOF_CLOSE_FILE;
  O->more = file_writer;
  return O;
}

/* a dedicated handler for stdout/stderr */

static size_t stdout_writer (iof *O, iof_mode mode)
{
  switch(mode)
  {
    case IOFWRITE:
    {
      fwrite(O->buf, sizeof(uint8_t), iof_size(O), stdout);
      O->pos = O->buf;
      return O->space;
    }
    case IOFCLOSE:
    case IOFFLUSH:
    {
      fwrite(O->buf, sizeof(uint8_t), iof_size(O), stdout);
      fflush(stdout);
      O->pos = O->buf;
      return 0;
    }
    default:
      break;
  }
  return 0;
}

static size_t stderr_writer (iof *O, iof_mode mode)
{
  switch(mode)
  {
    case IOFWRITE:
    {
      fwrite(O->buf, sizeof(uint8_t), iof_size(O), stderr);
      O->pos = O->buf;
      return O->space;
    }
    case IOFCLOSE:
    case IOFFLUSH:
    {
      fwrite(O->buf, sizeof(uint8_t), iof_size(O), stderr);
      fflush(stderr);
      O->pos = O->buf;
      return 0;
    }
    default:
      break;
  }
  return 0;
}

static uint8_t iof_stdout_buffer[BUFSIZ];
iof iof_stdout = IOF_WRITER_STRUCT(stdout_writer, NULL, iof_stdout_buffer, BUFSIZ, 0);

static uint8_t iof_stderr_buffer[BUFSIZ];
iof iof_stderr = IOF_WRITER_STRUCT(stderr_writer, NULL, iof_stderr_buffer, BUFSIZ, 0);

/* read from somewhere */

iof * iof_reader (iof *I, void *link, iof_handler reader, const void *m, size_t bytes)
{
  I->space = 0;
  I->link = link;
  I->more = reader;
  I->flags = 0;
  I->refcount = 0;
  if (m != NULL)
  {
    I->rbuf = I->rpos = (const uint8_t *)m;
    I->rend = (const uint8_t *)m + bytes;
    return I;
  }
  return NULL;
}

iof * iof_string_reader (iof *I, const void *s, size_t bytes)
{
  I->space = 0;
  I->link = NULL;
  I->more = NULL;
  I->flags = 0; // iof_string() sets IOF_DATA
  I->refcount = 0;
  if (s != NULL)
    return iof_string(I, s, bytes);
  return NULL;
}

/* write somewhere */

iof * iof_writer (iof *O, void *link, iof_handler writer, void *m, size_t bytes)
{
  O->space = 0;
  O->link = link;
  O->more = writer;
  O->flags = 0;
  O->refcount = 0;
  if (m != NULL && bytes > 0)
  {
    O->buf = O->pos = (uint8_t *)m;
    O->end = (uint8_t *)m + bytes;
    return O;
  }
  // return iof_null(O);
  return NULL;
}

/* write to growing bytes buffer */

static size_t iof_mem_handler (iof *O, iof_mode mode)
{
  switch(mode)
  {
    case IOFWRITE:
      return iof_resize_buffer(O);
    case IOFCLOSE:
      iof_free(O);
      return 0;
    default:
      return 0;
  }
}

iof * iof_setup_buffer (iof *O, void *buffer, size_t space)
{
  if (O == NULL)
    iof_setup_writer(O, buffer, space);
  else
    iof_writer_buffer(O, buffer, space);
  O->link = NULL;
  O->flags |= IOF_DATA;
  O->more = iof_mem_handler;
  return O;
}

iof * iof_setup_buffermin (iof *O, void *buffer, size_t space, size_t min)
{
  if ((O = iof_setup_buffer(O, buffer, space)) != NULL && space < min) // just allocate min now to avoid further rewriting
  {
    O->buf = O->pos = (uint8_t *)util_malloc(min);
    O->flags |= IOF_BUFFER_ALLOC;
    O->end = O->buf + min;
  }
  return O;
}

iof * iof_buffer_create (size_t space)
{
  uint8_t *buffer;
  iof *O;
  space += sizeof(iof);
  buffer = util_malloc(space);
  if ((O = iof_setup_buffer(NULL, buffer, space)) != NULL)
    O->flags |= IOF_ALLOC;
  return O;
}

/* set/get */

int iof_getc (iof *I)
{
  if (iof_readable(I))
    return *I->pos++;
  return IOFEOF;
}

int iof_putc (iof *O, int u)
{
  if (iof_writable(O))
  {
    iof_set(O, u);
    return (uint8_t)u;
  }
  return IOFFULL;
}

size_t iof_skip (iof *I, size_t bytes)
{
  while (bytes)
  {
    if (iof_readable(I))
      ++I->pos;
    else
      break;
    --bytes;
  }
  return bytes;
}

/* from iof to iof */

iof_status iof_pass (iof *I, iof *O)
{
  size_t leftin, leftout;
  if ((leftin = iof_left(I)) == 0)
    leftin = iof_input(I);
  while (leftin)
  {
    if ((leftout = iof_left(O)) == 0)
      if ((leftout = iof_output(O)) == 0)
        return IOFFULL;
    while (leftin > leftout)
    {
      memcpy(O->pos, I->pos, leftout);
      I->pos += leftout;
      O->pos = O->end; /* eq. += leftout */
      leftin -= leftout;
      if ((leftout = iof_output(O)) == 0)
        return IOFFULL;
    }
    if (leftin)
    {
      memcpy(O->pos, I->pos, leftin);
      I->pos = I->end; /* eq. += leftin */
      O->pos += leftin;
    }
    leftin = iof_input(I);
  }
  return IOFEOF;
}

/* read n-bytes */

size_t iof_read (iof *I, void *to, size_t size)
{
  size_t leftin, done = 0;
  char *s = (char *)to;
  
  if ((leftin = iof_left(I)) == 0)
    if ((leftin = iof_input(I)) == 0)
      return done;
  while (size > leftin)
  {
    memcpy(s, I->pos, leftin * sizeof(uint8_t));
    size -= leftin;
    done += leftin;
    s += leftin;
    I->pos = I->end;
    if ((leftin = iof_input(I)) == 0)
      return done;
  }
  if (size)
  {
    memcpy(s, I->pos, size * sizeof(uint8_t));
    I->pos += size;
    done += size;
  }
  return done;
}

/* rewrite FILE content (use fseek if needed) */

size_t iof_write_file_handle (iof *O, FILE *file)
{
  size_t leftout, size, readout;
  if ((leftout = iof_left(O)) == 0)
    if ((leftout = iof_output(O)) == 0)
      return 0;
  size = 0;
  do {
    readout = fread(O->pos, 1, leftout, file);    
    O->pos += readout;
    size += readout;
  } while(readout == leftout && (leftout = iof_output(O)) > 0);
  return size;
}

size_t iof_write_file (iof *O, const char *filename)
{
  FILE *file;
  size_t size;
  if ((file = fopen(filename, "rb")) == NULL)
    return 0;
  size = iof_write_file_handle(O, file);
  fclose(file);
  return size;
}

size_t iof_write_iofile (iof *O, iof_file *iofile, int savepos)
{
  long offset;
  size_t size;
  FILE *file;
  if (iofile->flags & IOF_DATA)
    return iof_write(O, iofile->pos, (size_t)(iofile->end - iofile->pos));
  file = iof_file_get_fh(iofile);
  if (savepos)
  {
    offset = ftell(file);  
    size = iof_write_file_handle(O, file);
    fseek(file, offset, SEEK_SET);
    return size;
  }
  return iof_write_file_handle(O, file);
}

/* write n-bytes */

size_t iof_write (iof *O, const void *data, size_t size)
{
  size_t leftout, done = 0;
  const char *s = (const char *)data;
  if ((leftout = iof_left(O)) == 0)
    if ((leftout = iof_output(O)) == 0)
      return done;
  while (size > leftout)
  {
    memcpy(O->pos, s, leftout * sizeof(uint8_t));
    size -= leftout;
    done += leftout;
    s += leftout;
    O->pos = O->end;
    if ((leftout = iof_output(O)) == 0)
      return done;
  }
  if (size)
  {
    memcpy(O->pos, s, size * sizeof(uint8_t));
    O->pos += size;
    done += size;
  }
  return done;
}

/* write '\0'-terminated string */

iof_status iof_puts (iof *O, const void *data)
{
  const char *s = (const char *)data;
  while (*s)
  {
    if (iof_writable(O))
      iof_set(O, *s++);
    else
      return IOFFULL;
  }
  return IOFEOF; // ?
}

size_t iof_put_string (iof *O, const void *data)
{
  const char *p, *s = (const char *)data;
  for (p = s; *p != '\0' && iof_writable(O); iof_set(O, *p++));
  return p - s;
}

/* write byte n-times */

/*
iof_status iof_repc (iof *O, char c, size_t bytes)
{
  while (bytes)
  {
    if (iof_writable(O))
      iof_set(O, c);
    else
      return IOFFULL;
    --bytes;
  }
  return IOFEOF; // ?
}
*/

size_t iof_repc (iof *O, char c, size_t bytes)
{
  size_t leftout, todo = bytes;
  if ((leftout = iof_left(O)) == 0)
    if ((leftout = iof_output(O)) == 0)
      return 0;
  while (bytes > leftout)
  {
    memset(O->pos, c, leftout);
    bytes -= leftout;
    O->pos = O->end;
    if ((leftout = iof_output(O)) == 0)
      return todo - bytes;
  }
  if (bytes)
  {
    memset(O->pos, c, bytes);
    O->pos += bytes;
  }
  return todo;
}

/* putfs */

#define IOF_FMT_SIZE 1024

size_t iof_putfs (iof *O, const char *format, ...)
{
  static char buffer[IOF_FMT_SIZE];
  va_list args;
  va_start(args, format);
  if (vsnprintf(buffer, IOF_FMT_SIZE, format, args) > 0)
  {
    va_end(args);
    return iof_put_string(O, buffer);
  }
  else
  {
    va_end(args);
    return iof_write(O, buffer, IOF_FMT_SIZE);
  }
}

/* integer from iof; return 1 on success, 0 otherwise */

int iof_get_int32 (iof *I, int32_t *number)
{
  int sign, c = iof_char(I);
  iof_scan_sign(I, c, sign);
  if (!base10_digit(c)) return 0;
  iof_read_integer(I, c, *number);
  if (sign) *number = -*number;
  return 1;
}

int iof_get_intlw (iof *I, intlw_t *number)
{
  int sign, c = iof_char(I);
  iof_scan_sign(I, c, sign);
  if (!base10_digit(c)) return 0;
  iof_read_integer(I, c, *number);
  if (sign) *number = -*number;
  return 1;
}

int iof_get_int64 (iof *I, int64_t *number)
{
  int sign, c = iof_char(I);
  iof_scan_sign(I, c, sign);
  if (!base10_digit(c)) return 0;
  iof_read_integer(I, c, *number);
  if (sign) *number = -*number;
  return 1;
}

int iof_get_uint32 (iof *I, uint32_t *number)
{
  int c = iof_char(I);
  if (!base10_digit(c)) return 0;
  iof_read_integer(I, c, *number);
  return 1;
}

int iof_get_uintlw (iof *I, uintlw_t *number)
{
  int c = iof_char(I);
  if (!base10_digit(c)) return 0;
  iof_read_integer(I, c, *number);
  return 1;
}

int iof_get_uint64 (iof *I, uint64_t *number)
{
  int c = iof_char(I);
  if (!base10_digit(c)) return 0;
  iof_read_integer(I, c, *number);
  return 1;
}

int iof_get_int32_radix (iof *I, int32_t *number, int radix)
{
  int sign, c = iof_char(I);
  iof_scan_sign(I, c, sign);
  if (!base10_digit(c)) return 0;
  iof_read_radix(I, c, *number, radix);
  if (sign) *number = -*number;
  return 1;

}

int iof_get_intlw_radix (iof *I, intlw_t *number, int radix)
{
  int sign, c = iof_char(I);
  iof_scan_sign(I, c, sign);
  if (!base10_digit(c)) return 0;
  iof_read_radix(I, c, *number, radix);
  if (sign) *number = -*number;
  return 1;
}

int iof_get_int64_radix (iof *I, int64_t *number, int radix)
{
  int sign, c = iof_char(I);
  iof_scan_sign(I, c, sign);
  if (!base10_digit(c)) return 0;
  iof_read_radix(I, c, *number, radix);
  if (sign) *number = -*number;
  return 1;
}

int iof_get_uint32_radix (iof *I, uint32_t *number, int radix)
{
  int c = iof_char(I);
  if (!base10_digit(c)) return 0;
  iof_read_radix(I, c, *number, radix);
  return 1;
}

int iof_get_uintlw_radix (iof *I, uintlw_t *number, int radix)
{
  int c = iof_char(I);
  if (!base10_digit(c)) return 0;
  iof_read_radix(I, c, *number, radix);
  return 1;
}

int iof_get_uint64_radix (iof *I, uint64_t *number, int radix)
{
  int c = iof_char(I);
  if (!base10_digit(c)) return 0;
  iof_read_radix(I, c, *number, radix);
  return 1;
}

/* get roman to uint16_t, cf. roman_to_uint16() from utilnumber.c*/

/* todo: some trick in place of this macro horror? */

#define roman1000(c) (c == 'M' || c == 'm')
#define roman500(c)  (c == 'D' || c == 'd')
#define roman100(c)  (c == 'C' || c == 'c')
#define roman50(c)   (c == 'L' || c == 'l')
#define roman10(c)   (c == 'X' || c == 'x')
#define roman5(c)    (c == 'V' || c == 'v')
#define roman1(c)    (c == 'I' || c == 'i')

#define roman100s(I, c) \
  (roman100(c) ? (100 + ((c = iof_next(I), roman100(c)) ? (100 + ((c = iof_next(I), roman100(c)) ? (c = iof_next(I), 100) : 0)) : 0)) : 0)
#define roman10s(I, c) \
  (roman10(c) ? (10 + ((c = iof_next(I), roman10(c)) ? (10 + ((c = iof_next(I), roman10(c)) ? (c = iof_next(I), 10) : 0)) : 0)) : 0)
#define roman1s(I, c) \
  (roman1(c) ? (1 + ((c = iof_next(I), roman1(c)) ? (1 + ((c = iof_next(I), roman1(c)) ? (c = iof_next(I), 1) : 0)) : 0)) : 0)

int iof_get_roman (iof *I, uint16_t *number)
{
  int c;
  /* M */
  for (*number = 0, c = iof_char(I); roman1000(c); *number += 1000, c = iof_next(I));
  /* D C */
  if (roman500(c))
  {
    c = iof_next(I);
    *number += 500 + roman100s(I, c);
  }
  else if (roman100(c))
  {
    c = iof_next(I);
    if (roman1000(c))
    {
      c = iof_next(I);
      *number += 900;
    }
    else if (roman500(c))
    {
      c = iof_next(I);
      *number += 400;
    }
    else
      *number += 100 + roman100s(I, c);
  }
  /* L X */
  if (roman50(c))
  {
    c = iof_next(I);
    *number += 50 + roman10s(I, c);
  }
  else if (roman10(c))
  {
    c = iof_next(I);
    if (roman100(c))
    {
      c = iof_next(I);
      *number += 90;
    }
    else if (roman50(c))
    {
      c = iof_next(I);
      *number += 40;
    }
    else
      *number += 10 + roman10s(I, c);
  }
  /* V I */
  if (roman5(c))
  {
    c = iof_next(I);
    *number += 5 + roman1s(I, c);
  }
  else if (roman1(c))
  {
    c = iof_next(I);
    if (roman10(c))
    {
      c = iof_next(I);
      *number += 9;
    }
    else if (roman5(c))
    {
      c = iof_next(I);
      *number += 4;
    }
    else
      *number += 1 + roman1s(I, c);
  }
  return 1;
}

/* double from iof; return 1 on success */

int iof_get_double (iof *I, double *number) // cf. string_to_double()
{
  int sign, exponent10, c = iof_char(I);
  iof_scan_sign(I, c, sign);
  iof_scan_decimal(I, c, *number);
  if (c == '.')
  {
    c = iof_next(I);
    iof_scan_fraction(I, c, *number, exponent10);
  }
  else
    exponent10 = 0;
  if (c == 'e' || c == 'E')
  {
    c = iof_next(I);
    iof_scan_exponent10(I, c, exponent10);
  }
  double_exp10(*number, exponent10);
  if (sign) *number = -*number;
  return 1;
}

int iof_get_float (iof *I, float *number) // cf. string_to_float() in utilnumber.c
{
  int sign, exponent10, c = iof_char(I);
  iof_scan_sign(I, c, sign);
  iof_scan_decimal(I, c, *number);
  if (c == '.')
  {
    c = iof_next(I);
    iof_scan_fraction(I, c, *number, exponent10);
  }
  else
    exponent10 = 0;
  if (c == 'e' || c == 'E')
  {
    c = iof_next(I);
    iof_scan_exponent10(I, c, exponent10);
  }
  float_exp10(*number, exponent10);
  if (sign) *number = -*number;
  return 1;
}

int iof_conv_double (iof *I, double *number) // cf. convert_to_double() in utilnumber.c
{
  int sign, exponent10, c = iof_char(I);
  iof_scan_sign(I, c, sign);
  iof_scan_decimal(I, c, *number);
  if (c == '.' || c == ',')
  {
    c = iof_next(I);
    iof_scan_fraction(I, c, *number, exponent10);
    if (exponent10 < 0)
      double_negative_exp10(*number, exponent10);
  }
  if (sign) *number = -*number;
  return 1;
}

int iof_conv_float (iof *I, float *number) // cf. convert_to_float()
{
  int sign, exponent10, c = iof_char(I);
  iof_scan_sign(I, c, sign);
  iof_scan_decimal(I, c, *number);
  if (c == '.' || c == ',')
  {
    c = iof_next(I);
    iof_scan_fraction(I, c, *number, exponent10);
    if (exponent10 < 0)
      float_negative_exp10(*number, exponent10);
  }
  if (sign) *number = -*number;
  return 1;
}

/* integer to iof; return a number of written bytes */

#define iof_copy_number_buffer(O, s, p) for (p = s; *p && iof_writable(O); iof_set(O, *p), ++p)

size_t iof_put_int32 (iof *O, int32_t number)
{
  const char *s, *p;
  s = int32_to_string(number);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_intlw (iof *O, intlw_t number)
{
  const char *s, *p;
  s = intlw_to_string(number);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_int64 (iof *O, int64_t number)
{
  const char *s, *p;
  s = int64_to_string(number);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_uint32 (iof *O, uint32_t number)
{
  const char *s, *p;
  s = uint32_to_string(number);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_uintlw (iof *O, uintlw_t number)
{
  const char *s, *p;
  s = uintlw_to_string(number);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_uint64 (iof *O, uint64_t number)
{
  const char *s, *p;
  s = uint64_to_string(number);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_int32_radix (iof *O, int32_t number, int radix)
{
  const char *s, *p;
  s = int32_to_radix(number, radix);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_intlw_radix (iof *O, intlw_t number, int radix)
{
  const char *s, *p;
  s = intlw_to_radix(number, radix);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_int64_radix (iof *O, int64_t number, int radix)
{
  const char *s, *p;
  s = int64_to_radix(number, radix);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_uint32_radix (iof *O, uint32_t number, int radix)
{
  const char *s, *p;
  s = uint32_to_radix(number, radix);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_uintlw_radix (iof *O, uintlw_t number, int radix)
{
  const char *s, *p;
  s = uintlw_to_radix(number, radix);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_uint64_radix (iof *O, uint64_t number, int radix)
{
  const char *s, *p;
  s = uint64_to_radix(number, radix);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

/* roman numerals */

size_t iof_put_roman_uc (iof *O, uint16_t number)
{
  const char *s, *p;
  s = uint16_to_roman_uc(number);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_roman_lc (iof *O, uint16_t number)
{
  const char *s, *p;
  s = uint16_to_roman_lc(number);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

/* double/float to iof; return the number of written bytes */

size_t iof_put_double (iof *O, double number, int digits)
{
  const char *s, *p;
  s = double_to_string(number, digits);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

size_t iof_put_float (iof *O, float number, int digits)
{
  const char *s, *p;
  s = float_to_string(number, digits);
  iof_copy_number_buffer(O, s, p);
  return p - s;
}

/* iof to binary integer; pretty common */

int iof_get_be_uint2 (iof *I, uint32_t *pnumber)
{
  int c1, c2;
  if ((c1 = iof_get(I)) < 0 || (c2 = iof_get(I)) < 0)
    return 0;
  *pnumber = (c1<<8)|c2;
  return 1;
}

int iof_get_be_uint3 (iof *I, uint32_t *pnumber)
{
  int c1, c2, c3;
  if ((c1 = iof_get(I)) < 0 || (c2 = iof_get(I)) < 0 || (c3 = iof_get(I)) < 0)
    return 0;
  *pnumber = (c1<<16)|(c2<<8)|c3;
  return 1;
}

int iof_get_be_uint4 (iof *I, uint32_t *pnumber)
{
  int c1, c2, c3, c4;
  if ((c1 = iof_get(I)) < 0 || (c2 = iof_get(I)) < 0 || (c3 = iof_get(I)) < 0 || (c4 = iof_get(I)) < 0)
    return 0;
  *pnumber = (c1<<24)|(c2<<16)|(c3<<8)|c4;
  return 1;
}

int iof_get_le_uint2 (iof *I, uint32_t *pnumber)
{
  int c1, c2;
  if ((c1 = iof_get(I)) < 0 || (c2 = iof_get(I)) < 0)
    return 0;
  *pnumber = (c2<<8)|c1;
  return 1;
}

int iof_get_le_uint3 (iof *I, uint32_t *pnumber)
{
  int c1, c2, c3;
  if ((c1 = iof_get(I)) < 0 || (c2 = iof_get(I)) < 0 || (c3 = iof_get(I)) < 0)
    return 0;
  *pnumber = (c3<<16)|(c2<<8)|c1;
  return 1;
}

int iof_get_le_uint4 (iof *I, uint32_t *pnumber)
{
  int c1, c2, c3, c4;
  if ((c1 = iof_get(I)) < 0 || (c2 = iof_get(I)) < 0 || (c3 = iof_get(I)) < 0 || (c4 = iof_get(I)) < 0)
    return 0;
  *pnumber = (c4<<24)|(c3<<16)|(c2<<8)|c1;
  return 1;
}

/* iof input data */

uint8_t * iof_file_input_data (iof_file *iofile, size_t *psize, int *isnew)
{
  uint8_t *data;
  if (iofile->flags & IOF_DATA)
  {
    data = iofile->buf;
    *psize = iofile->end - iofile->buf;
    *isnew = 0;
    return data;
  }
  if (iof_file_reopen(iofile))
  {
    data = iof_copy_file_handle_data(iof_file_get_fh(iofile), psize);
    *isnew = 1;
    iof_file_reclose(iofile);
    return data;
  }
  return NULL;
}

/*
uint8_t * iof_file_reader_data (iof_file *iofile, size_t *size)
{
  uint8_t *data;
  if (!(iofile->flags & IOF_DATA) || iofile->pos == NULL || (*size = (size_t)iof_left(iofile)) == 0)
    return NULL;  
  if (iofile->flags & IOF_BUFFER_ALLOC)
  {
    data = iofile->buf; // iofile->pos; // returned must be freeable, makes sense when ->buf == ->pos
    iofile->flags &= ~IOF_BUFFER_ALLOC;
    iofile->buf = iofile->pos = iofile->end = NULL;
    return data;
  }
  data = (uint8_t *)util_malloc(*size);
  memcpy(data, iofile->buf, *size);
  return data;
}

uint8_t * iof_file_writer_data (iof_file *iofile, size_t *size)
{
  uint8_t *data;
  if (!(iofile->flags & IOF_DATA) || iofile->buf == NULL || (*size = (size_t)iof_size(iofile)) == 0)
    return NULL;  
  if (iofile->flags & IOF_BUFFER_ALLOC)
  {
    iofile->flags &= ~IOF_BUFFER_ALLOC;
    data = iofile->buf;
    iofile->buf = iofile->pos = iofile->end = NULL;
    return data;
  }
  data = (uint8_t *)util_malloc(*size);
  memcpy(data, iofile->buf, *size);
  return data;
}
*/

uint8_t * iof_reader_data (iof *I, size_t *psize)
{
  uint8_t *data;
  *psize = (size_t)iof_left(I);
  if (I->flags & IOF_BUFFER_ALLOC)
  {
    data = I->buf; // actually I->pos, but we have to return something freeable
    I->flags &= ~IOF_BUFFER_ALLOC;
    I->buf = NULL;
  }
  else
  {
    data = util_malloc(*psize);
    memcpy(data, I->pos, *psize);
  }
  iof_close(I);
  return data;
}


uint8_t * iof_writer_data (iof *O, size_t *psize)
{
  uint8_t *data;
  *psize = (size_t)iof_size(O);
  if (O->flags & IOF_BUFFER_ALLOC)
  {
    data = O->buf;
    O->flags &= ~IOF_BUFFER_ALLOC;
    O->buf = NULL;
  }
  else
  {
    data = util_malloc(*psize);
    memcpy(data, O->buf, *psize);
  }
  iof_close(O);
  return data;
}

size_t iof_reader_to_file_handle (iof *I, FILE *file)
{
  size_t size;
  for (size = 0; iof_readable(I); I->pos = I->end)
    size += fwrite(I->buf, sizeof(uint8_t), iof_left(I), file);
  return size;
}

size_t iof_reader_to_file (iof *I, const char *filename)
{
  FILE *file;
  size_t size;
  if ((file = fopen(filename, "wb")) == NULL)
    return 0;
  for (size = 0; iof_readable(I); I->pos = I->end)
    size += fwrite(I->buf, sizeof(uint8_t), iof_left(I), file);
  fclose(file);
  return size;
}

/* debug */

size_t iof_data_to_file (const void *data, size_t size, const char *filename)
{
  FILE *fh;
  if ((fh = fopen(filename, "wb")) == NULL)
    return 0;
  // size = fwrite(data, size, sizeof(uint8_t), fh); // WRONG, this always returns 1, as fwrite returns the number of elements successfully written out
  size = fwrite(data, sizeof(uint8_t), size, fh);
  fclose(fh);
  return size;
}

size_t iof_result_to_file_handle (iof *F, FILE *file)
{
  const void *data;
  size_t size;
  data = iof_result(F, size);
	return iof_data_to_file_handle(data, size, file);
}

size_t iof_result_to_file (iof *F, const char *filename)
{
  const void *data;
  size_t size;
  data = iof_result(F, size);
  return iof_data_to_file(data, size, filename);
}

void iof_debug (iof *I, const char *filename)
{
  FILE *file = fopen(filename, "wb");
  if (file != NULL)
  {
    fprintf(file, ">>> buf %p <<<\n", I->buf);
    fwrite(I->buf, sizeof(uint8_t), iof_size(I), file);
    fprintf(file, "\n>>> pos %p (%ld) <<<\n", I->pos, (long)iof_size(I));
    fwrite(I->pos, sizeof(uint8_t), iof_left(I), file);
    fprintf(file, "\n>>> end %p (%ld) <<<\n", I->end, (long)iof_left(I));
    fwrite(I->end, sizeof(uint8_t), I->space - iof_space(I), file);
    fprintf(file, "\n>>> end of buffer %p (%ld) <<<\n", I->buf + I->space, (long)(I->buf + I->space - I->end));
    fclose(file);
  }
}

/* common filters api */

/* sizes of filter states on x64
size of iof_filter: 640 (no longer used; sizeof(iof) + sizeof larger state)
size of file_state: 16
size of stream_state: 16
size of flate_state: 104
size of lzw_state: 56
size of predictor_state: 104
size of basexx_state: 48
size of basexx_state: 48
size of basexx_state: 48
size of eexec_state: 40
size of runlength_state: 24
size of rc4_state: 24
size of aes_state: 72
size of img_state: 576
size of img: 496
*/

typedef struct iof_heap iof_heap;

struct iof_heap {
  uint8_t *data, *pos;
  size_t size, space;
  iof_heap *next, *prev;
  int refcount;
};

typedef struct {
  iof_heap *heap;
} iof_heap_ghost;

static iof_heap * iof_buffers_heap = NULL;
static iof_heap * iof_filters_heap = NULL;

#define IOF_HEAP_FILTERS_COUNT 4
#define IOF_BUFFER_SIZE 262144 // (1<<18)
#define IOF_FILTER_SIZE 1024
// sizeof(iof_filter) on x64 is now 640, img_state 576, img 496, others 16-104
#define IOF_BUFFER_HEAP_SIZE (IOF_HEAP_FILTERS_COUNT * (IOF_BUFFER_SIZE + sizeof(iof_heap_ghost)))
#define IOF_FILTER_HEAP_SIZE (IOF_HEAP_FILTERS_COUNT * (IOF_FILTER_SIZE + sizeof(iof_heap_ghost)))

static iof_heap * iof_heap_new (size_t space)
{
  iof_heap *iofheap;
  iofheap = (iof_heap *)util_malloc(sizeof(iof_heap) + space);
  iofheap->data = iofheap->pos = (uint8_t *)(iofheap + 1);
  iofheap->size = iofheap->space = space;
  iofheap->next = NULL;
  iofheap->prev = NULL;
  iofheap->refcount = 0;
  return iofheap;
}

#define iof_heap_free(iofheap) util_free(iofheap)

void iof_filters_init (void)
{
  if (iof_buffers_heap == NULL)
    iof_buffers_heap = iof_heap_new(IOF_BUFFER_HEAP_SIZE);
  if (iof_filters_heap == NULL)
    iof_filters_heap = iof_heap_new(IOF_FILTER_HEAP_SIZE);
}

void iof_filters_free (void)
{
  iof_heap *heap, *next;
  for (heap = iof_buffers_heap; heap != NULL; heap = next)
  {
    next = heap->next;
    if (heap->refcount != 0)
      loggerf("not closed iof filters left (%d)", heap->refcount);
    if (next != NULL)
      loggerf("iof filters heap left");
    iof_heap_free(heap);
  }
  iof_buffers_heap = NULL;
  for (heap = iof_filters_heap; heap != NULL; heap = next)
  {
    next = heap->next;
    if (heap->refcount != 0)
      loggerf("not closed iof buffers left (%d)", heap->refcount);
    if (next != NULL)
      loggerf("iof buffers heap left");
    iof_heap_free(heap);
  }
  iof_filters_heap = NULL;
}

#if defined __arm__ || defined __ARM__ || defined ARM || defined __ARM || defined __arm || defined __ARM_ARCH ||defined __aarch64__ 
#define iof_heap_get(hp, ghost, data, siz) \
 (ghost = (iof_heap_ghost *)((void*)((hp)->pos)), \
  ghost->heap = hp, \
  data = (uint8_t *)(ghost + 1), \
  (hp)->pos += siz, \
  (hp)->size -= siz, \
  ++(hp)->refcount)
#else
#define iof_heap_get(hp, ghost, data, siz) \
 (ghost = (iof_heap_ghost *)((hp)->pos), \
  ghost->heap = hp, \
  data = (uint8_t *)(ghost + 1), \
  (hp)->pos += siz, \
  (hp)->size -= siz, \
  ++(hp)->refcount)

#endif


static void * iof_heap_take (iof_heap **pheap, size_t size)
{
  uint8_t *data;
  iof_heap_ghost *ghost;
  iof_heap *heap, *newheap, *next;

  heap = *pheap;
  size += sizeof(iof_heap_ghost);
  if (heap->size >= size)
  { /* take cheap mem from main heap */
    iof_heap_get(heap, ghost, data, size);
    return data;
  }
  if (size <= heap->space >> 1)
  { /* make new cheap heap, make it front */
    *pheap = newheap = iof_heap_new(heap->space);
    newheap->next = heap;
    heap->prev = newheap;
    iof_heap_get(newheap, ghost, data, size);
    return data;
  }
  /* size much larger than expected? should not happen.
     make a single-item heap, keep the front heap intact. */
  newheap = iof_heap_new(size);
  if ((next = heap->next) != NULL)
  {
    newheap->next = next;
    next->prev = newheap;
  }
  heap->next = newheap;
  newheap->prev = heap;
  iof_heap_get(newheap, ghost, data, size);
  return data;
}

void iof_heap_back (void *data)
{
  iof_heap_ghost *ghost;
  iof_heap *heap, *next, *prev;

  ghost = ((iof_heap_ghost *)data) - 1;
  heap = ghost->heap;
  if (heap->refcount == 0)
    loggerf("invalid use of iof heap, refcount < 0");
  if (--heap->refcount <= 0)
  {
    if ((prev = heap->prev) != NULL)
    { /* free the heap */
      if ((next = heap->next) != NULL)
        prev->next = next, next->prev = prev;
      else
        prev->next = NULL;
      iof_heap_free(heap);
    }
    else
    { /* this is the front heap, just reset */
      heap->pos = heap->data;
      heap->size = heap->space;
    }
  }
}

void * iof_filter_new (size_t size)
{ // to be removed
  void *data;

  iof_filters_init();
  data = iof_heap_take(&iof_filters_heap, size);
  return memset(data, 0, size);
}

static uint8_t * iof_filter_buffer_new (size_t *psize)
{
  iof_filters_init();
  *psize = IOF_BUFFER_SIZE;
  return iof_heap_take(&iof_buffers_heap, IOF_BUFFER_SIZE);
}

iof * iof_filter_reader_new (iof_handler handler, size_t statesize, void **pstate)
{
  iof *F;
  void *filter;
  uint8_t *buffer;
  size_t buffersize;

  iof_filters_init();
  filter = iof_heap_take(&iof_filters_heap, sizeof(iof) + statesize);
  F = (iof *)memset(filter, 0, sizeof(iof) + statesize);
  buffer = iof_filter_buffer_new(&buffersize);
  iof_reader_buffer(F, buffer, buffersize);
  F->flags |= IOF_HEAP|IOF_BUFFER_HEAP;
  F->more = handler;
  *pstate = (F + 1);
  return F;
}

iof * iof_filter_reader_with_buffer_new (iof_handler handler, size_t statesize, void **pstate, void *buffer, size_t buffersize)
{ // for filters that has own buffer (string, some image filters)
  iof *F;
  void *filter;
  iof_filters_init();
  filter = iof_heap_take(&iof_filters_heap, sizeof(iof) + statesize);
  F = (iof *)memset(filter, 0, sizeof(iof) + statesize);
  iof_reader_buffer(F, buffer, buffersize);
  F->flags |= IOF_HEAP;
  F->more = handler;
  *pstate = (F + 1);
  return F;
}

iof * iof_filter_writer_new (iof_handler handler, size_t statesize, void **pstate)
{
  iof *F;
  void *filter;
  uint8_t *buffer;
  size_t buffersize;

  iof_filters_init();
  filter = iof_heap_take(&iof_filters_heap, sizeof(iof) + statesize);
  F = (iof *)memset(filter, 0, sizeof(iof) + statesize);
  buffer = iof_filter_buffer_new(&buffersize);
  iof_writer_buffer(F, buffer, buffersize);
  F->flags |= IOF_HEAP|IOF_BUFFER_HEAP;
  F->more = handler;
  *pstate = (F + 1);
  return F;
}

iof * iof_filter_writer_with_buffer_new (iof_handler handler, size_t statesize, void **pstate, void *buffer, size_t size)
{
  iof *F;
  void *filter;
  size_t buffersize;

  iof_filters_init();
  filter = iof_heap_take(&iof_filters_heap, sizeof(iof) + statesize);
  F = (iof *)memset(filter, 0, sizeof(iof) + statesize);
  buffer = iof_filter_buffer_new(&buffersize);
  iof_writer_buffer(F, buffer, buffersize);
  F->flags |= IOF_HEAP;
  F->more = handler;
  *pstate = (F + 1);
  return F;
}

/* close */

#define iof_close_next(F) ((void)(iof_decref((F)->next), (F)->next = NULL, 0))
/* when filter creation fails, we should take care to destroy the filter but leave ->next intact */
#define iof_clear_next(F) ((void)(iof_unref((F)->next), (F)->next = NULL, 0))

#define iof_close_buffer(F) ((void)\
  ((F)->buf != NULL ? \
      ((F->flags & IOF_BUFFER_ALLOC) ? (util_free((F)->buf), (F)->buf = NULL, 0) : \
      ((F->flags & IOF_BUFFER_HEAP) ? (iof_filter_buffer_free((F)->buf), (F)->buf = NULL, 0) : ((F)->buf = NULL, 0))) : 0))

/* closing underlying file handle */

static void iof_close_file (iof *F)
{
  FILE *file;
  //if (F->flags & IOF_FILE_HANDLE)
  //{
    if ((file = F->file) != NULL)
    {
      if (F->flags & IOF_CLOSE_FILE)
        fclose(F->file);
      F->file = NULL;
    }
  //}
}

/* a very special variant for reader filters initiated with iof_file_reopen(). It also calls
   iof_file_reclose(), which takes an effect only if previously reopened, but better to keep
   all this thin ice separated. Used in filters: iofile_reader, iofile_stream_reader, image
   decoders. */

static void iof_close_iofile (iof *F)
{
  iof_file *iofile;
  //if (F->flags & IOF_FILE)
  //{
    if ((iofile = F->iofile) != NULL)
    {
      iof_file_unsync(iofile, NULL);
      iof_file_reclose(iofile); // takes an effect iff prevoiusly reopened
      iof_file_decref(iofile);
      F->iofile = NULL;
    }
  //}
}

void iof_free (iof *F)
{
  if (F->flags & IOF_FILE_HANDLE)
    iof_close_file(F);
  else if (F->flags & IOF_FILE)
    iof_close_iofile(F);
  else if (F->flags & IOF_NEXT)
    iof_close_next(F);
  iof_close_buffer(F);
  if (F->flags & IOF_HEAP)
    iof_filter_free(F);
  else if (F->flags & IOF_ALLOC)
    util_free(F);
}

void iof_discard (iof *F)
{ // so far used only on failed filters creation; as iof_free() but don't dare to release ->next
  if (F->flags & IOF_FILE_HANDLE)
    iof_close_file(F);
  else if (F->flags & IOF_FILE)
    iof_close_iofile(F);
  else if (F->flags & IOF_NEXT)
    iof_close_next(F);
  iof_close_buffer(F);
  if (F->flags & IOF_HEAP)
    iof_filter_free(F);
  else if (F->flags & IOF_ALLOC)
    util_free(F);
}

/* resizing buffer */

size_t iof_resize_buffer_to (iof *O, size_t space)
{
  uint8_t *buf;

  if (O->flags & IOF_BUFFER_ALLOC)
  {
    buf = (uint8_t *)util_realloc(O->buf, space);
  }
  else
  {
    buf = (uint8_t *)util_malloc(space);
    memcpy(buf, O->buf, iof_size(O));
    if (O->flags & IOF_BUFFER_HEAP)
    {
      iof_filter_buffer_free(O->buf);
      O->flags &= ~IOF_BUFFER_HEAP;
    }
    O->flags |= IOF_BUFFER_ALLOC;

  }
  O->pos = buf + iof_size(O);
  O->end = buf + space;
  O->buf = buf;
  O->space = space;
  return iof_left(O);
}

/* */

size_t iof_decoder_retval (iof *I, const char *type, iof_status status)
{
  switch (status)
  {
    case IOFERR:
    case IOFEMPTY:             // should never happen as we set state.flush = 1 on decoders init
      loggerf("%s decoder error (%d, %s)", type, status, iof_status_kind(status));
      I->flags |= IOF_STOPPED;
      return 0;
    case IOFEOF:               // this is the last chunk,
      I->flags |= IOF_STOPPED; // so stop it and fall
    case IOFFULL:              // prepare pointers to read from I->buf
      I->end = I->pos;
      I->pos = I->buf;
      return I->end - I->buf;
  }
  loggerf("%s decoder bug, invalid retval %d", type, status);
  return 0;
}

size_t iof_encoder_retval (iof *O, const char *type, iof_status status)
{
  switch (status)
  {
    case IOFERR:
    case IOFFULL:
      loggerf("%s encoder error (%d, %s)", type, status, iof_status_kind(status));
      return 0;
    case IOFEMPTY:
      O->pos = O->buf;
      O->end = O->buf + O->space;
      return O->space;
    case IOFEOF:
      return 0;
  }
  loggerf("%s encoder bug, invalid retval %d", type, status);
  return 0;
}

/* file/stream state */

typedef struct {
  size_t length;
  size_t offset;
} file_state;


#define file_state_init(state, off, len) ((state)->offset = off, (state)->length = len)

typedef struct {
  size_t length;
  size_t offset;
} stream_state;

#define stream_state_init(state, off, len) ((state)->offset = off, (state)->length = len)

static size_t file_read (iof *I)
{
  size_t bytes, tail;
  if (I->flags & IOF_STOPPED)
    return 0;
  tail = iof_tail(I);
  if ((bytes = tail + fread(I->buf + tail, sizeof(uint8_t), I->space - tail, I->file)) < I->space)
    I->flags |= IOF_STOPPED;
  I->pos = I->buf;
  I->end = I->buf + bytes;
  return bytes;
}

static size_t iofile_read (iof *I, size_t *poffset)
{
  size_t bytes, tail;
  if (I->flags & IOF_STOPPED)
    return 0;
  iof_file_sync(I->iofile, poffset);
  tail = iof_tail(I);
  if ((bytes = tail + iof_file_read(I->buf + tail, sizeof(uint8_t), I->space - tail, I->iofile)) < I->space)
  {
    I->flags |= IOF_STOPPED;
    iof_file_unsync(I->iofile, poffset);
  }
  I->pos = I->buf;
  I->end = I->buf + bytes;
  return bytes;
}

static size_t file_load (iof *I)
{
  size_t bytes, left, tail;
  if (I->flags & IOF_STOPPED)
    return 0;
  tail = iof_tail(I);
  I->pos = I->buf + tail;
  I->end = I->buf + I->space; /* don't assume its done when initializing the filter */
  left = I->space - tail;
  do {
    bytes = fread(I->pos, sizeof(uint8_t), left, I->file);
    I->pos += bytes;
  } while (bytes == left && (left = iof_resize_buffer(I)) > 0);
  I->flags |= IOF_STOPPED;
  return iof_loaded(I);
}

static size_t iofile_load (iof *I, size_t *poffset)
{
  size_t bytes, left, tail;
  if (I->flags & IOF_STOPPED)
    return 0;
  tail = iof_tail(I);
  I->pos = I->buf + tail;
  I->end = I->buf + I->space; /* don't assume its done when initializing the filter */
  left = I->space - tail;
  iof_file_sync(I->iofile, poffset);
  do {
    bytes = iof_file_read(I->pos, sizeof(uint8_t), left, I->iofile);
    I->pos += bytes;
  } while (bytes == left && (left = iof_resize_buffer(I)) > 0);
  I->flags |= IOF_STOPPED;
  iof_file_unsync(I->iofile, poffset);
  return iof_loaded(I);
}

static size_t filter_file_reader (iof *I, iof_mode mode)
{
  switch (mode)
  {
    case IOFREAD:
      return file_read(I);
    case IOFLOAD:
      return file_load(I);
    case IOFCLOSE:
      iof_free(I);
      return 0;
    default:
      return 0;
  }
}

static size_t filter_iofile_reader (iof *I, iof_mode mode)
{
  file_state *state;
  state = iof_filter_state(file_state *, I);
  switch (mode)
  {
    case IOFREAD:
      return iofile_read(I, &state->offset);
    case IOFLOAD:
      return iofile_load(I, &state->offset);
    case IOFCLOSE:
      iof_free(I);
      return 0;
    default:
      return 0;
  }
}

static size_t file_write (iof *O, int flush)
{
  size_t bytes;
  if ((bytes = iof_size(O)) > 0)
    if (bytes != fwrite(O->buf, sizeof(uint8_t), bytes, O->file))
      return 0;
  if (flush)
    fflush(O->file);
  O->end = O->buf + O->space; // remains intact actually
  O->pos = O->buf;
  return O->space;
}

static size_t iofile_write (iof *O, size_t *poffset, int flush)
{
  size_t bytes;
  iof_file_sync(O->iofile, poffset);
  if ((bytes = iof_size(O)) > 0)
  {
    if (bytes != iof_file_write(O->buf, sizeof(uint8_t), bytes, O->iofile))
    {
      iof_file_unsync(O->iofile, poffset);
      return 0;
    }
  }
  if (flush)
    iof_file_flush(O->iofile);
  O->end = O->buf + O->space; // remains intact actually
  O->pos = O->buf;
  return O->space;
}

static size_t filter_file_writer (iof *O, iof_mode mode)
{
  switch (mode)
  {
    case IOFWRITE:
      return file_write(O, 0);
    case IOFFLUSH:
      return file_write(O, 1);
    case IOFCLOSE:
      file_write(O, 1);
      iof_free(O);
      return 0;
    default:
      return 0;
  }
}

static size_t filter_iofile_writer (iof *O, iof_mode mode)
{
  file_state *state;
  state = iof_filter_state(file_state *, O);
  switch (mode)
  {
    case IOFWRITE:
      return iofile_write(O, &state->offset, 0);
    case IOFFLUSH:
      return iofile_write(O, &state->offset, 1);
    case IOFCLOSE:
      iofile_write(O, &state->offset, 1);
      iof_free(O);
      return 0;
    default:
      return 0;
  }
}

/* filter from FILE* */

iof * iof_filter_file_handle_reader (FILE *file)
{
  iof *I;
  file_state *state;
  if (file == NULL)
    return NULL;
  I = iof_filter_reader(filter_file_reader, sizeof(file_state), &state);
  iof_setup_file(I, file);
  file_state_init(state, 0, 0);
  return I;
}

iof * iof_filter_file_handle_writer (FILE *file)
{
  iof *O;
  file_state *state;
  if (file == NULL)
    return NULL;
  O = iof_filter_writer(filter_file_writer, sizeof(file_state), &state);
  iof_setup_file(O, file);
  file_state_init(state, 0, 0);
  return O;
}

/* filter from iof_file * */

iof * iof_filter_iofile_reader (iof_file *iofile, size_t offset)
{
  iof *I;
  file_state *state;
  if (!iof_file_reopen(iofile))
    return NULL;
  I = iof_filter_reader(filter_iofile_reader, sizeof(file_state), &state);
  iof_setup_iofile(I, iofile);
  file_state_init(state, offset, 0);
  return I;
}

iof * iof_filter_iofile_writer (iof_file *iofile, size_t offset)
{
  iof *O;
  file_state *state;
  O = iof_filter_writer(filter_iofile_writer, sizeof(file_state), &state);
  iof_setup_iofile(O, iofile);
  file_state_init(state, offset, 0);
  return O;
}

/* filter from filename */

iof * iof_filter_file_reader (const char *filename)
{
  iof *I;
  file_state *state;
  FILE *file;
  if ((file = fopen(filename, "rb")) == NULL)
    return NULL;
  I = iof_filter_reader(filter_file_reader, sizeof(file_state), &state);
  iof_setup_file(I, file);
  file_state_init(state, 0, 0);
  I->flags |= IOF_CLOSE_FILE;
  return I;
}

iof * iof_filter_file_writer (const char *filename)
{
  iof *O;
  file_state *state;
  FILE *file;
  if ((file = fopen(filename, "wb")) == NULL)
    return NULL;
  O = iof_filter_writer(filter_file_writer, sizeof(file_state), &state);
  iof_setup_file(O, file);
  file_state_init(state, 0, 0);
  O->flags |= IOF_CLOSE_FILE;
  return O;
}

/* from string */

static size_t dummy_handler (iof *I, iof_mode mode)
{
  switch (mode)
  {
    case IOFCLOSE:
      iof_free(I);
      return 0;
    default:
      return 0;
  }
}

iof * iof_filter_string_reader (const void *s, size_t length)
{
  iof *I;
  void *dummy;
  I = iof_filter_reader_with_buffer(dummy_handler, 0, &dummy, NULL, 0);
  I->rbuf = I->rpos = (const uint8_t *)s;
  I->rend = (const uint8_t *)s + length;
  // I->space = length;
  return I;
}

iof * iof_filter_string_writer (const void *s, size_t length)
{
  iof *O;
  void *dummy;
  O = iof_filter_reader_with_buffer(dummy_handler, 0, &dummy, NULL, 0);
  O->rbuf = O->rpos = (const uint8_t *)s;
  O->rend = (const uint8_t *)s + length;
  // O->space = length;
  return O;
}

iof * iof_filter_buffer_writer (size_t size)
{ // filter alternative of iof_buffer_create()
  iof *O;
  void *dummy;
  uint8_t *buffer;
  if (size > IOF_BUFFER_SIZE)
  {
    buffer = (uint8_t *)util_malloc(size);
    O = iof_filter_writer_with_buffer(iof_mem_handler, 0, &dummy, buffer, size);
    O->flags |= IOF_BUFFER_ALLOC;
    return O;
  }
	return iof_filter_writer(iof_mem_handler, 0, &dummy);
}

/* stream */

static size_t file_stream_read (iof *I, size_t *plength)
{
  size_t bytes, tail;
  if (I->flags & IOF_STOPPED || *plength == 0)
    return 0;
  tail = iof_tail(I);
  if (I->space - tail >= *plength)
  {
    bytes = tail + fread(I->buf + tail, sizeof(uint8_t), *plength, I->file);
    I->flags |= IOF_STOPPED;
    *plength = 0;
  }
  else
  {
    bytes = tail + fread(I->buf + tail, sizeof(uint8_t), I->space - tail, I->file);
    *plength -= bytes - tail;
  }
  I->pos = I->buf;
  I->end = I->buf + bytes;
  return bytes;
}

static size_t iofile_stream_read (iof *I, size_t *plength, size_t *poffset)
{
  size_t bytes, tail;
  if (I->flags & IOF_STOPPED || *plength == 0)
    return 0;
  tail = iof_tail(I);
  iof_file_sync(I->iofile, poffset);
  if (I->space - tail >= *plength)
  {
    bytes = tail + iof_file_read(I->buf + tail, sizeof(uint8_t), *plength, I->iofile);
    iof_file_unsync(I->iofile, poffset);
    I->flags |= IOF_STOPPED;
    *plength = 0;
  }
  else
  {
    bytes = tail + iof_file_read(I->buf + tail, sizeof(uint8_t), I->space - tail, I->iofile);
    *plength -= bytes - tail;
  }
  I->pos = I->buf;
  I->end = I->buf + bytes;
  return bytes;
}

static size_t file_stream_load (iof *I, size_t *plength)
{
  size_t bytes, tail;
  if (I->flags & IOF_STOPPED || *plength == 0)
    return 0;
  tail = iof_tail(I);
  if (I->space - tail < *plength)
    if (iof_resize_buffer_to(I, tail + *plength) == 0)
      return 0;
  bytes = tail + fread(I->buf + tail, sizeof(uint8_t), *plength, I->file);
  I->flags |= IOF_STOPPED;
  *plength = 0;
  I->pos = I->buf;
  I->end = I->buf + bytes;
  return bytes;
}

static size_t iofile_stream_load (iof *I, size_t *plength, size_t *poffset)
{
  size_t bytes, tail;
  if (I->flags & IOF_STOPPED || *plength == 0)
    return 0;
  iof_file_sync(I->iofile, poffset);
  tail = iof_tail(I);
  if (I->space - tail < *plength)
    if (iof_resize_buffer_to(I, tail + *plength) == 0)
      return 0;
  bytes = tail + iof_file_read(I->buf + tail, sizeof(uint8_t), *plength, I->iofile);
  iof_file_unsync(I->iofile, poffset);
  I->flags |= IOF_STOPPED;
  *plength = 0;
  I->pos = I->buf;
  I->end = I->buf + bytes;
  return bytes;
}

static size_t filter_file_stream_reader (iof *I, iof_mode mode)
{
  stream_state *state;
  state = iof_filter_state(stream_state *, I);
  switch(mode)
  {
    case IOFREAD:
      return file_stream_read(I, &state->length);
    case IOFLOAD:
      return file_stream_load(I, &state->length);
    case IOFCLOSE:
      iof_free(I);
      return 0;
    default:
      return 0;
  }
}

static size_t filter_iofile_stream_reader (iof *I, iof_mode mode)
{
  stream_state *state;
  state = iof_filter_state(stream_state *, I);
  switch(mode)
  {
    case IOFREAD:
      return iofile_stream_read(I, &state->length, &state->offset);
    case IOFLOAD:
      return iofile_stream_load(I, &state->length, &state->offset);
    case IOFCLOSE:
      iof_free(I);
      return 0;
    default:
      return 0;
  }
}

iof * iof_filter_stream_reader (FILE *file, size_t offset, size_t length)
{
  iof *I;
  stream_state *state;
  I = iof_filter_reader(filter_file_stream_reader, sizeof(stream_state), &state);
  iof_setup_file(I, file);
  stream_state_init(state, offset, length);
  fseek(file, (long)offset, SEEK_SET); // or perhaps it should be call in file_stream_read(), like iof_file_sync()?
  return I;
}

iof * iof_filter_stream_coreader (iof_file *iofile, size_t offset, size_t length)
{
  iof *I;
  stream_state *state;
  if (!iof_file_reopen(iofile))
    return NULL;
  I = iof_filter_reader(filter_iofile_stream_reader, sizeof(stream_state), &state);
  iof_setup_iofile(I, iofile);
  stream_state_init(state, offset, length);
  return I;
}

static size_t file_stream_write (iof *O, size_t *plength, int flush)
{
  size_t bytes;
  if ((bytes = iof_size(O)) > 0)
  {
    if (bytes != fwrite(O->buf, sizeof(uint8_t), bytes, O->file))
    {
      *plength += bytes;
      return 0;
    }
  }
  if (flush)
    fflush(O->file);
  *plength += bytes;
  O->end = O->buf + O->space; // remains intact
  O->pos = O->buf;
  return O->space;
}

static size_t iofile_stream_write (iof *O, size_t *plength, size_t *poffset, int flush)
{
  size_t bytes;
  if ((bytes = iof_size(O)) > 0)
  {
    iof_file_sync(O->iofile, poffset);
    if (bytes != iof_file_write(O->buf, sizeof(uint8_t), bytes, O->iofile))
    {
      *plength += bytes;
      iof_file_unsync(O->iofile, poffset);
      return 0;
    }
  }
  if (flush)
    iof_file_flush(O->iofile);
  *plength += bytes;
  O->end = O->buf + O->space; // remains intact
  O->pos = O->buf;
  return O->space;
}

static size_t filter_file_stream_writer (iof *O, iof_mode mode)
{
  stream_state *state;
  state = iof_filter_state(stream_state *, O);
  switch (mode)
  {
    case IOFWRITE:
      return file_stream_write(O, &state->length, 0);
    case IOFFLUSH:
      return file_stream_write(O, &state->length, 1);
    case IOFCLOSE:
      file_stream_write(O, &state->length, 1);
      iof_free(O);
      return 0;
    default:
      return 0;
  }
}

static size_t filter_iofile_stream_writer (iof *O, iof_mode mode)
{
  stream_state *state;
  state = iof_filter_state(stream_state *, O);
  switch (mode)
  {
    case IOFWRITE:
      return iofile_stream_write(O, &state->length, &state->offset, 0);
    case IOFFLUSH:
      return iofile_stream_write(O, &state->length, &state->offset, 1);
    case IOFCLOSE:
      iofile_stream_write(O, &state->length, &state->offset, 1);
      iof_free(O);
      return 0;
    default:
      return 0;
  }
}

iof * iof_filter_stream_writer (FILE *file)
{
  iof *O;
  stream_state *state;
  O = iof_filter_writer(filter_file_stream_writer, sizeof(stream_state), &state);
  iof_setup_file(O, file);
  stream_state_init(state, 0, 0);
  return O;
}

iof * iof_filter_stream_cowriter (iof_file *iofile, size_t offset)
{
  iof *O;
  stream_state *state;
  O = iof_filter_writer(filter_iofile_stream_writer, sizeof(stream_state), &state);
  iof_setup_iofile(O, iofile);
  stream_state_init(state, offset, 0);
  return O;
}

/* very specific for images; get input from already created strem filter, exchange the filter but keep the buffer */

FILE * iof_filter_file_reader_source (iof *I, size_t *poffset, size_t *plength)
{
  stream_state *sstate;
  file_state *fstate;
  if (I->more == filter_file_stream_reader) // I is the result of iof_filter_stream_reader()
  {
    sstate = iof_filter_state(stream_state *, I);
    *poffset = sstate->offset;
    *plength = sstate->length; // might be 0 but it is ok for file readers
    return I->file;
  }
  if (I->more == filter_file_reader)
  {
    fstate = iof_filter_state(file_state *, I);
    *poffset = fstate->offset;
    *plength = fstate->length; // might be 0 but it is ok for file readers
    return I->file;
  }
  return NULL;
}

iof_file * iof_filter_file_coreader_source (iof *I, size_t *poffset, size_t *plength)
{
  stream_state *sstate;
  file_state *fstate;
  if (I->more == filter_iofile_stream_reader) // I is the result of iof_filter_stream_coreader()
  {
    sstate = iof_filter_state(stream_state *, I);
    *poffset = sstate->offset;
    *plength = sstate->length;
    return I->iofile;
  }
  if (I->more == filter_iofile_reader)
  {
    fstate = iof_filter_state(file_state *, I);
    *poffset = fstate->offset;
    *plength = fstate->length;
    return I->iofile;
  }
  return NULL;
}

iof * iof_filter_reader_replacement (iof *P, iof_handler handler, size_t statesize, void **pstate)
{ // called after iof_filter_file_reader_source(), no need to check if F is filter from iof heap and if has buffer from iof heap
  iof *F;
  F = iof_filter_reader_with_buffer(handler, statesize, pstate, P->buf, P->space);
  F->flags |= IOF_BUFFER_HEAP;
  //iof_reader_buffer(P, NULL, 0);
  //P->flags &= ~IOF_BUFFER_HEAP;
  iof_filter_free(P);
  return F;
}























