#pragma once

#if defined(_WIN32)

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(HAVE_LIBTIRPC)
#include <cstdint>
#include <cstring>
#include <cstdio>

#include <Winsock2.h>

uint32_t constexpr W32_XDR_DECODE = 0U;
uint32_t constexpr W32_XDR_ENCODE = 1U;

struct Win32XDR
{
  FILE* file;
  struct
  {
    uint8_t* data;
    uint8_t* dataCursor;
    size_t memSize;
  } nonFileMem;
  uint32_t fileMode;
};

void w32_xdrstdio_create(Win32XDR* xdrs, FILE* file, uint32_t op);

void w32_xdr_destroy(Win32XDR* xdrs);

void w32_xdrmem_create(
  Win32XDR* xdrs, uint8_t* addr, size_t size,
  uint32_t op
);

size_t w32_xdr_getpos(Win32XDR* xdrs);
bool w32_xdr_setpos(Win32XDR* xdrs, size_t pos);

template<typename T>
bool w32_internal_xdr_write_raw_bytes(Win32XDR* xdr, T& data)
{
  if (xdr->file != nullptr)
  {
    auto written = fwrite(&data, sizeof(T), 1, xdr->file);
    return written == 1;
  }
  else
  {
    memcpy(xdr->nonFileMem.dataCursor, reinterpret_cast<char*>(&data), sizeof(T));
    xdr->nonFileMem.dataCursor += sizeof(T);
    return true;
  }
}

template<typename T>
bool w32_internal_xdr_read_raw_bytes(Win32XDR* xdr, T& result)
{
  if (xdr->file != nullptr)
  {
    auto readNum = fread(&result, sizeof(T), 1, xdr->file);
    return readNum == 1;
  }
  else
  {
    result = *reinterpret_cast<T*>(xdr->nonFileMem.dataCursor);
    xdr->nonFileMem.dataCursor += sizeof(T);
    return xdr->nonFileMem.dataCursor <= xdr->nonFileMem.data + xdr->nonFileMem.memSize;
  }
}

template<typename THostType>
struct w32_internal_xdr_conv_fn {};

template<>
struct w32_internal_xdr_conv_fn<uint8_t>
{
  typedef uint8_t NetType;
  static uint8_t identity(uint8_t in)
  {
    return in;
  }

  static auto constexpr host2NetFn = identity;
  static auto constexpr net2HostFn = identity;

};

template<>
struct w32_internal_xdr_conv_fn<uint16_t>
{
  typedef u_short NetType;
  static auto constexpr host2NetFn = htons;
  static auto constexpr net2HostFn = ntohs;
};

template<>
struct w32_internal_xdr_conv_fn<uint32_t>
{
  typedef unsigned __int32 NetType;
  static auto constexpr host2NetFn = htonl;
  static auto constexpr net2HostFn = ntohl;
};

template<>
struct w32_internal_xdr_conv_fn<uint64_t>
{
  typedef unsigned __int64 NetType;
  static auto constexpr host2NetFn = htonll;
  static auto constexpr net2HostFn = ntohll;
};

template<>
struct w32_internal_xdr_conv_fn<double>
{
  typedef unsigned __int64 NetType;
  static auto constexpr host2NetFn = htond;
  static auto constexpr net2HostFn = ntohd;
};

template<>
struct w32_internal_xdr_conv_fn<float>
{
  typedef unsigned __int32 NetType;
  static auto constexpr  host2NetFn = htonf;
  static auto constexpr net2HostFn = ntohf;
};


template<typename THostType>
bool w32_internal_xdr_u_type(Win32XDR* xdrs, THostType* ip)
{
  typedef w32_internal_xdr_conv_fn<THostType>::NetType NetType;
  static_assert(sizeof(NetType) == sizeof(THostType));
  if (xdrs->fileMode == W32_XDR_DECODE)
  {
    NetType rawValue;
    auto const result = w32_internal_xdr_read_raw_bytes<NetType>(xdrs, rawValue);
    if (result)
    {
      *ip = w32_internal_xdr_conv_fn<THostType>::net2HostFn(rawValue);
    }
    return result;
  }
  else
  {
    NetType translated = w32_internal_xdr_conv_fn<THostType>::host2NetFn(*ip);
    return w32_internal_xdr_write_raw_bytes<NetType>(xdrs, translated);
  }
}

bool w32_xdr_int32_t(Win32XDR* xdrs, int32_t* ip);
bool w32_xdr_u_int32_t(Win32XDR* xdrs, uint32_t* ip);

bool w32_xdr_int64_t(Win32XDR* xdrs, int64_t* ip);
bool w32_xdr_u_int64_t(Win32XDR* xdrs, uint64_t* ip);

bool w32_xdr_int16_t(Win32XDR* xdrs, int16_t* ip);
bool w32_xdr_u_int16_t(Win32XDR* xdrs, uint16_t* ip);

bool w32_xdr_char(Win32XDR* xdrs, char* ip);
bool w32_xdr_u_char(Win32XDR* xdrs, unsigned char* ip);

bool w32_xdr_float(Win32XDR* xdrs, float* ip);
bool w32_xdr_double(Win32XDR* xdrs, double* ip);

typedef Win32XDR XDR;
#define xdrstdio_create w32_xdrstdio_create
#define xdr_destroy w32_xdr_destroy
#define xdrmem_create w32_xdrmem_create

#define xdr_getpos w32_xdr_getpos
#define xdr_setpos w32_xdr_setpos

#define xdr_int16_t w32_xdr_int16_t
#define xdr_u_int16_t w32_xdr_u_int16_t
#define xdr_int32_t w32_xdr_int32_t
#define xdr_u_int32_t w32_xdr_u_int32_t
#define xdr_int64_t w32_xdr_int64_t
#define xdr_u_int64_t w32_xdr_u_int64_t

#define xdr_char w32_xdr_char
#define xdr_u_char w32_xdr_u_char

#define xdr_float w32_xdr_float
#define xdr_double w32_xdr_double

#define XDR_DECODE W32_XDR_DECODE
#define XDR_ENCODE W32_XDR_ENCODE

#endif
#endif
