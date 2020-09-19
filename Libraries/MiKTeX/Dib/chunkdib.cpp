/* chunkdib.cpp: test driver for the DibChunker interfaces

   Copyright (C) 2002-2020 Christian Schenk

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <memory>

#include <cstdlib>

#include <iostream>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/App/Application>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Graphics/DibChunker>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Graphics;
using namespace MiKTeX::Util;
using namespace std;

#define T_(x) MIKTEXTEXT(x)

class ChunkDib :
  public Application,
  public IDibChunkerCallback
{
public:
  size_t MIKTEXTHISCALL Read(void* data, size_t size) override;

public:
  void MIKTEXTHISCALL OnNewChunk(shared_ptr<DibChunk> chunk) override;

public:
  void Run(int argc, const char** argv);

private:
  FileStream stream;

private:
  unsigned long nChunks = 0;

private:
  unsigned long nBitmaps = 0;

private:
  string prefix;
};

size_t MIKTEXTHISCALL ChunkDib::Read(void* data, size_t size)
{
  return stream.Read(data, size);
}

void MIKTEXTHISCALL ChunkDib::OnNewChunk(shared_ptr<DibChunk> chunk)
{
  nChunks += 1;
  PathName fileName(fmt::format("{0}-{1}-{2}.bmp", prefix, nBitmaps, nChunks));
  FileStream bitmapFile(File::Open(fileName, FileMode::Create, FileAccess::Write, false));
  const BITMAPINFO* pBitmapInfo = chunk->GetBitmapInfo();
  unsigned long nBytesPerLine = (((pBitmapInfo->bmiHeader.biWidth * pBitmapInfo->bmiHeader.biBitCount) + 31) & ~31) >> 3;
  unsigned nColors = pBitmapInfo->bmiHeader.biBitCount == 24 ? 0 : 1 << pBitmapInfo->bmiHeader.biBitCount;
  BITMAPFILEHEADER header;
  header.bfType = 0x4d42;
  header.bfSize = sizeof(header) + sizeof(BITMAPINFOHEADER) + nColors * sizeof(RGBQUAD) + nBytesPerLine * pBitmapInfo->bmiHeader.biHeight;
  header.bfReserved1 = 0;
  header.bfReserved2 = 0;
  header.bfOffBits = sizeof(header) + sizeof(BITMAPINFOHEADER) + nColors * sizeof(RGBQUAD);
  bitmapFile.Write(&header, sizeof(header));
  bitmapFile.Write(&pBitmapInfo->bmiHeader, sizeof(BITMAPINFOHEADER));
  bitmapFile.Write(chunk->GetColors(), nColors * sizeof(RGBQUAD));
  bitmapFile.Write(chunk->GetBits(), nBytesPerLine * pBitmapInfo->bmiHeader.biHeight);
  bitmapFile.Close();
  cout << fmt::format(T_("chunk {0} written"), fileName) << endl;
}

void ChunkDib::Run(int argc, const char** argv)
{
  unsigned long chunkSize;
  unique_ptr<DibChunker> chunker(DibChunker::Create());
  if (argc > 1)
  {
    PathName fileName(argv[1]);
    prefix = fileName.GetFileNameWithoutExtension().ToString();
    chunkSize = static_cast<unsigned long>(File::GetSize(PathName(argv[1]))) / 5;
    stream.Attach(File::Open(fileName, FileMode::Open, FileAccess::Read, false));
  }
  else
  {
    prefix = "chunk";
    chunkSize = 2 * 1024 * 1024;
    stream.Attach(stdin);
  }
  nBitmaps = 0;
  do
  {
    ++nBitmaps;
    nChunks = 0;
  } while (chunker->Process(DibChunker::Default, chunkSize, this));
}

int main(int argc, char** argv)
{
  try
  {
    ChunkDib app;
    vector<char*> newargv{ argv, argv + argc + 1 };
    app.Init(newargv);
    app.Run(newargv.size() - 1, const_cast<const char**>(&newargv[0]));
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException& e)
  {
    Utils::PrintException(e);
    return 1;
  }
  catch (const exception& e)
  {
    Utils::PrintException(e);
    return 1;
  }
  catch (const char* message)
  {
    cerr << "fatal error: " << message << endl;
    return 1;
  }
}
