#include <cassert>
#if !defined(MIKTEX)
#include "xstream.h"
#endif

#if defined(HAVE_CONFIG_H)
#include "config.h"
#else
#define HAVE_LIBTIRPC 1
#endif

#if defined(HAVE_LIBTIRPC)

namespace xdr
{

// xbyte
xbyte::xbyte() {}
xbyte::xbyte(unsigned char c0): c(c0) {}
xbyte::xbyte(int c0): c((unsigned char) c0) {}
xbyte::xbyte(unsigned c0): c((unsigned char) c0) {}

int xbyte::byte() const
{
  return c;
}

xbyte::operator unsigned char() const
{
  return c;
}

// xios
int xios::good() const { return _state == 0; }
int xios::eof() const { return _state & eofbit; }
int xios::fail() const { return !good();}
int xios::bad() const { return _state & badbit; }
void xios::clear(int state) {_state=state;}
void xios::set(int flag) {_state |= flag;}
xios::operator void*() const { return fail() ? (void*)0 : (void*)(-1); }
int xios::operator!() const { return fail(); }

// xstream
xstream::~xstream() {}
xstream::xstream(): xios(), buf(nullptr) {}
void xstream::precision(int) {}
xstream& xstream::seek(OffsetType pos, seekdir dir) {
  if(buf) {
    clear();
    if(fseeko(buf,pos,dir) != 0) set(failbit);
  }
  return *this;
}
OffsetType xstream::tell() {
  return ftello(buf);
}

// ixstream

ixstream::ixstream(bool singleprecision)
  : singleprecision(singleprecision)
{

}

void ixstream::open(const char* filename, open_mode)
{
  clear();
  buf=fopen(filename,"rb");
  if(buf) xdrstdio_create(&xdri,buf,XDR_DECODE);
  else set(badbit);
}

void ixstream::close()
{
  closeFile();
}

void ixstream::closeFile()
{
  if(buf) {
#ifndef _CRAY
    xdr_destroy(&xdri);
#endif
    fclose(buf);
    buf=nullptr;
  }
}

ixstream::ixstream(const char* filename, bool singleprecision)
  : singleprecision(singleprecision)
{
  ixstream::open(filename);
}

ixstream::ixstream(const char* filename, open_mode mode, bool singleprecision)
  : singleprecision(singleprecision)
{
  ixstream::open(filename,mode);
}

ixstream::~ixstream()
{
  ixstream::close();
}

ixstream& ixstream::operator>>(imanip func)
{
  return (*func)(*this);
}

ixstream& ixstream::operator>>(double& x)
{
  if(singleprecision)
  {
    float f;
    *this >> f;
    x=(double) f;
  } else
    if(!xdr_double(&xdri, &x)) set(eofbit);
  return *this;
}

ixstream& ixstream::operator>>(xbyte& x)
{
  int c=fgetc(buf);
  if(c != EOF) x=c;
  else set(eofbit);
  return *this;
}

// oxstream
oxstream::oxstream(bool singleprecision): singleprecision(singleprecision)
{

}
void oxstream::open(const char* filename, open_mode mode)
{
  clear();
  buf=fopen(filename,(mode & app) ? "ab" : "wb");
  if(buf) xdrstdio_create(&xdro,buf,XDR_ENCODE);
  else set(badbit);
}
void oxstream::close()
{
  closefile();
}
void oxstream::closefile()
{
  if(buf) {
#ifndef _CRAY
    xdr_destroy(&xdro);
#endif
    fclose(buf);
    buf=NULL;
  }
}
oxstream::oxstream(const char* filename, bool singleprecision)
  : singleprecision(singleprecision)
{
  oxstream::open(filename);
}

oxstream::oxstream(const char* filename, open_mode mode, bool singleprecision)
  : singleprecision(singleprecision)
{
  oxstream::open(filename,mode);
}

oxstream::~oxstream()
{
  closefile();
}

oxstream& oxstream::flush() {if(buf) fflush(buf); return *this;}
oxstream& oxstream::operator<<(omanip func) { return (*func)(*this); }
oxstream& oxstream::operator<<(double x)
{
  if(singleprecision)
    *this << (float) x;
  else
    if(!xdr_double(&xdro, &x)) set(badbit);
  return *this;
}
oxstream& oxstream::operator<<(xbyte x) {
  if(fputc(x.byte(),buf) == EOF) set(badbit);
  return *this;
}

memoxstream::memoxstream(bool singleprecision)
  : oxstream(singleprecision)
#if defined(_WIN32)
  ,fmInstance()
#endif
{
  clear();
#if defined(_WIN32)
  fmem_init(&fmInstance);
  buf=fmem_open(&fmInstance, "w+");
#else
  buf=open_memstream(&buffer,&size);
#endif
  if(buf)
    xdrstdio_create(&xdro,buf,XDR_ENCODE);
  else
    set(badbit);
}

memoxstream::~memoxstream()
{
  closefile();
#if defined(_WIN32)
  fmem_term(&fmInstance);
#else
    free(buffer);
#endif
}
std::vector<uint8_t> memoxstream::createCopyOfCurrentData() {
  auto flushResult = fflush(buf);
  if (flushResult != 0)
  {
    std::cerr << "cannot flush memory xstream";
    exit(EXIT_FAILURE);
  }
#if defined(_WIN32)
  size_t retSize=0;
  void* streamPtr=nullptr;

  // DANGER: There's a severe but rare issue with certain systems
  // involving a potential memory leak.

  // See https://github.com/Kreijstal/fmem/issues/6

  // Right now, we have no reasonable way to determine if a tmpfile
  // implementation is being used, so we cannot have a way to
  // conditionally free the memory.

  // On most systems, we have the open_memstream and Windows tmpfile API,
  // where the allocation/mapping is handled by the system; hence
  // there is no need to free the pointer ourselves.
  // But the tmpfile implementation uses malloc that doesn't
  // get freed, so it is our job to manually free it.
  fmem_mem(&fmInstance, &streamPtr, &retSize);

  if (streamPtr == nullptr)
  {
    return {};
  }

  auto* bytePtr = static_cast<uint8_t*>(streamPtr);
  std::vector ret(bytePtr, bytePtr + retSize);
  return ret;
#else
    // for sanity check, always ensure that we have a vector of bytes
    static_assert(sizeof(char) == sizeof(uint8_t));

    if (buffer == nullptr)
    {
      return {};
    }

    auto* retPtr = reinterpret_cast<uint8_t*>(buffer);
    return {retPtr, retPtr + size};
#endif
}

// memixstream
#if defined(_WIN32)
memixstream::memixstream(uint8_t* data, size_t length, bool singleprecision)
  : ixstream(singleprecision), data(data), length(length)
{
  clear();
  xdrmem_create(&xdri,data,length,XDR_DECODE);
}
#else
memixstream::memixstream(uint8_t* data, size_t length, bool singleprecision)
  : ixstream(singleprecision)
{
  clear();
  buf=fmemopen(data,length,"r");
  if(buf) xdrstdio_create(&xdri,buf,XDR_DECODE);
  else set(badbit);
}
#endif

memixstream::memixstream(std::vector<uint8_t>& data, bool singleprecision)
  : memixstream(data.data(), data.size(), singleprecision)
{
}

#if defined(_WIN32)
xstream& memixstream::seek(OffsetType pos, seekdir dir) {
  clear();
  if(!xdr_setpos(&xdri,pos))
    set(eofbit);
  return *this;
}

OffsetType memixstream::tell() {
  return xdr_getpos(&xdri);
}

ixstream& memixstream::operator>>(xbyte& x)
{
  size_t position=tell();
  if(position < length) {
    x=data[position++];
    seek(position);
  } else set(eofbit);
  return *this;
}

memixstream::~memixstream()
{
  xdr_destroy(&xdri);
}
#else
memixstream::~memixstream()
{
  if(buf) {
    fclose(buf);
    buf=nullptr;
  }
}
#endif

// ioxstream
void ioxstream::open(const char* filename, open_mode mode)
{
  clear();
  if(mode & app)
    buf=fopen(filename,"ab+");
  else if(mode & trunc)
    buf=fopen(filename,"wb+");
  else if(mode & out) {
    buf=fopen(filename,"rb+");
    if(!buf)
      buf=fopen(filename,"wb+");
  } else
    buf=fopen(filename,"rb");
  if(buf) {
    xdrstdio_create(&xdri,buf,XDR_DECODE);
    xdrstdio_create(&xdro,buf,XDR_ENCODE);
  } else set(badbit);
}
void ioxstream::close() {
  if(buf) {
#ifndef _CRAY
    xdr_destroy(&xdri);
    xdr_destroy(&xdro);
#endif
    fclose(buf);
    buf=NULL;
  }
}
ioxstream::ioxstream()
{
}
ioxstream::ioxstream(const char* filename)
{
  ioxstream::open(filename);
}
ioxstream::ioxstream(const char* filename, open_mode mode)
{
  ioxstream::open(filename,mode);
}
ioxstream::~ioxstream()
{
  ioxstream::close();
}

oxstream& endl(oxstream& s) { s.flush(); return s; }
oxstream& flush(oxstream& s) {s.flush(); return s;}


}
#endif
