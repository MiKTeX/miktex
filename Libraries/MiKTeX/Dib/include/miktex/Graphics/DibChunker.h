/* miktex/Graphics/DibChunker.h:                        -*- C++ -*-

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of the MiKTeX DibChunker Library.

   The MiKTeX DibChunker Library is free software; you can
   redistribute it and/or modify it under the terms of the GNU General
   Public License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DibChunker Library is distributed in the hope that it
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX DibChunker Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(EA12EA6239A13E40B589B41B84360852)
#define EA12EA6239A13E40B589B41B84360852

// DLL import/export switch
#if !defined(CA71604DC6CF21439A421D76D5C2E5D7)
#  define MIKTEXDIBEXPORT MIKTEXDLLIMPORT
#endif

// API decoration for exported member functions
#define MIKTEXDIBCEEAPI(type) MIKTEXDIBEXPORT type MIKTEXCEECALL

#define MIKTEX_GRAPHICS_BEGIN_NAMESPACE         \
  namespace MiKTeX {                            \
    namespace Graphics {

#define MIKTEX_GRAPHICS_END_NAMESPACE           \
    }                                           \
  }

#include <cstddef>

#include <memory>

MIKTEX_GRAPHICS_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE DibChunk
{
public:
  virtual MIKTEXTHISCALL ~DibChunk() = 0;

public:
  virtual int MIKTEXTHISCALL GetX() const = 0;

public:
  virtual int MIKTEXTHISCALL GetY() const = 0;

public:
  virtual BITMAPINFOHEADER MIKTEXTHISCALL GetBitmapInfoHeader() const = 0;

public:
  virtual const RGBQUAD * MIKTEXTHISCALL GetColors() const = 0;

public:
  virtual const void * MIKTEXTHISCALL GetBits() const = 0;

public:
  virtual const BITMAPINFO * MIKTEXTHISCALL GetBitmapInfo() const = 0;

public:
  virtual std::size_t MIKTEXTHISCALL GetSize() const = 0;
};

class MIKTEXNOVTABLE IDibChunkerCallback
{
public:
  virtual std::size_t MIKTEXTHISCALL Read(void * pBuf, std::size_t size) = 0;

public:
  virtual void MIKTEXTHISCALL OnNewChunk(std::shared_ptr<DibChunk> chunk) = 0;
};

class MIKTEXNOVTABLE DibChunker
{
public:
  virtual MIKTEXTHISCALL ~DibChunker() = 0;

public:
  enum {
    RemoveBlankLines = 1,
    CreateChunks = 2,
    Crop = 4,
    Default = 7
  };

public:
  virtual bool MIKTEXTHISCALL Process(unsigned long flags, unsigned long chunkSize, IDibChunkerCallback * pCallback) = 0;

public:
  static MIKTEXDIBCEEAPI(std::unique_ptr<DibChunker>) Create();
};

MIKTEX_GRAPHICS_END_NAMESPACE;

#undef MIKTEX_GRAPHICS_BEGIN_NAMESPACE
#undef MIKTEX_GRAPHICS_END_NAMESPACE

#endif
