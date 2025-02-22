#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(HAVE_LIBTIRPC)
#include "xstream.h"

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
  buf=fopen(filename,"r");
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
  buf=fopen(filename,(mode & app) ? "a" : "w");
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

  // DANGER: There's a /severe/ issue with certain systems (though such cases should be rare)
  // about a potential memory leak.

  // See https://github.com/Kreijstal/fmem/issues/6

  // Right now, we have no reasonable way to determine if tmpfile implementation is being used
  // or not, so we cannot have a way to conditionally free the memory.

  // In most systems, we have open_memstream and Windows tmpfile API
  // which the allocation/mapping is handled by the system and hence
  // no need to free the pointer ourselves, but tmpfile implementation uses malloc that doesn't
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
    // for sanity check, always want to make sure we have a vector of bytes
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
memixstream::memixstream(char* data, size_t length, bool singleprecision)
  : ixstream(singleprecision)
{
  xdrmem_create(&xdri,data,length,XDR_DECODE);
}
memixstream::memixstream(std::vector<char>& data, bool singleprecision)
  : memixstream(data.data(), data.size(), singleprecision)
{
}

memixstream::~memixstream()
{
  xdr_destroy(&xdri);
}
void memixstream::close()
{
  xdr_destroy(&xdri);
}

void memixstream::open(const char* filename, open_mode openMode)
{
}

// ioxstream

void ioxstream::open(const char* filename, open_mode mode)
{
  clear();
  if(mode & app)
    buf=fopen(filename,"a+");
  else if(mode & trunc)
    buf=fopen(filename,"w+");
  else if(mode & out) {
    buf=fopen(filename,"r+");
    if(!buf)
      buf=fopen(filename,"w+");
  } else
    buf=fopen(filename,"r");
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


// ixstream + oxstream operator implementation

#define IXSTREAM_CC_DECL(T) ixstream& ixstream::operator >> (T& x)
#define IXSTREAM_IMPL(T,N) IXSTREAM_CC_DECL(T) {if(!xdr_##N(&xdri, &x)) set(eofbit); return *this;}

IXSTREAM_IMPL(int, int);
IXSTREAM_IMPL(unsigned int, u_int);
IXSTREAM_IMPL(long, long);
IXSTREAM_IMPL(unsigned long, u_long);
IXSTREAM_IMPL(long long, longlong_t);
IXSTREAM_IMPL(unsigned long long, u_longlong_t);
IXSTREAM_IMPL(short, short);
IXSTREAM_IMPL(unsigned short, u_short);
IXSTREAM_IMPL(char, char);
#ifndef _CRAY
IXSTREAM_IMPL(unsigned char, u_char);
#endif
IXSTREAM_IMPL(float, float);

#define OXSTREAM_CC_DECL(T) oxstream& oxstream::operator << (T x)
#define OXSTREAM_IMPL(T,N) OXSTREAM_CC_DECL(T) {if(!xdr_##N(&xdro, &x)) set(badbit); return *this;}

OXSTREAM_IMPL(int, int);
OXSTREAM_IMPL(unsigned int, u_int);
OXSTREAM_IMPL(long, long);
OXSTREAM_IMPL(unsigned long, u_long);
OXSTREAM_IMPL(long long, longlong_t);
OXSTREAM_IMPL(unsigned long long, u_longlong_t);
OXSTREAM_IMPL(short, short);
OXSTREAM_IMPL(unsigned short, u_short);
OXSTREAM_IMPL(char, char);

#ifndef _CRAY
OXSTREAM_IMPL(unsigned char, u_char);
#endif
OXSTREAM_IMPL(float, float);

oxstream& endl(oxstream& s) { s.flush(); return s; }
oxstream& flush(oxstream& s) {s.flush(); return s;}


}
#endif
