#include "win32xdr.h"

#if defined(_WIN32) && defined(HAVE_LIBTIRPC)

void w32_xdrstdio_create(Win32XDR* xdrs, FILE* file, uint32_t op)
{
  xdrs->file = file;
  xdrs->fileMode = op;
}

void w32_xdr_destroy(Win32XDR* xdrs)
{
}

void w32_xdrmem_create(
  Win32XDR* xdrs, uint8_t* addr, size_t size,
  uint32_t op
)
{
  xdrs->file = nullptr;
  xdrs->fileMode = op;
  xdrs->nonFileMem.data = addr;
  xdrs->nonFileMem.memSize = size;
  xdrs->nonFileMem.dataCursor = addr;
  xdrs->fileMode = op;
}

size_t w32_xdr_getpos(Win32XDR* xdrs) {
  return xdrs->nonFileMem.dataCursor-xdrs->nonFileMem.data;
}

bool w32_xdr_setpos(Win32XDR* xdrs, size_t pos) {
  if(pos <= xdrs->nonFileMem.memSize) {
    xdrs->nonFileMem.dataCursor=xdrs->nonFileMem.data+pos;
    return true;
  }
  return false;
}

bool w32_xdr_int16_t(Win32XDR* xdrs, int16_t* ip)
{
  return w32_xdr_u_int16_t(xdrs, reinterpret_cast<uint16_t*>(ip));
}

bool w32_xdr_u_int16_t(Win32XDR* xdrs, uint16_t* ip)
{
  return w32_internal_xdr_u_type<uint16_t>(xdrs, ip);
}

bool w32_xdr_int32_t(Win32XDR* xdrs, int32_t* ip)
{
  return w32_xdr_u_int32_t(xdrs, reinterpret_cast<uint32_t*>(ip));
}

bool w32_xdr_u_int32_t(Win32XDR* xdrs, uint32_t* ip)
{
  return w32_internal_xdr_u_type<uint32_t>(xdrs, ip);
}

bool w32_xdr_int64_t(Win32XDR* xdrs, int64_t* ip)
{
  return w32_xdr_u_int64_t(xdrs, reinterpret_cast<uint64_t*>(ip));
}

bool w32_xdr_u_int64_t(Win32XDR* xdrs, uint64_t* ip)
{
  return w32_internal_xdr_u_type<uint64_t>(xdrs, ip);
}

bool w32_xdr_float(Win32XDR* xdrs, float* ip)
{
  return w32_internal_xdr_u_type<float>(xdrs, ip);
}

bool w32_xdr_double(Win32XDR* xdrs, double* ip)
{
  return w32_internal_xdr_u_type<double>(xdrs, ip);
}

bool w32_xdr_char(Win32XDR* xdrs, char* ip)
{
  static_assert(sizeof(char) == sizeof(int8_t));
  return w32_xdr_u_char(xdrs, reinterpret_cast<unsigned char*>(ip));
}

bool w32_xdr_u_char(Win32XDR* xdrs, unsigned char* ip)
{
  static_assert(sizeof(unsigned char) == sizeof(uint8_t));
  return w32_internal_xdr_u_type<uint8_t>(xdrs, ip);
}

#endif
