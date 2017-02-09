/* miktex/TeXAndFriends/TeXMemoryHandlerImpl.h:         -*- C++ -*-

   Copyright (C) 2017 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(F751C3C43D1A411F806871C3A791B48B)
#define F751C3C43D1A411F806871C3A791B48B

#include <miktex/TeXAndFriends/config.h>

#include "TeXMFMemoryHandlerImpl.h"

MIKTEXMF_BEGIN_NAMESPACE;

namespace texapp {
#include <miktex/texapp.defaults.h>
}

template<class PROGRAM_CLASS> class TeXMemoryHandlerImpl :
  public TeXMFMemoryHandlerImpl<PROGRAM_CLASS>
{
public:
  TeXMemoryHandlerImpl(PROGRAM_CLASS& program, TeXMFApp& texmfapp) :
    TeXMFMemoryHandlerImpl<PROGRAM_CLASS>(program, texmfapp)
  {
  }

public:
  void Allocate(const std::unordered_map<std::string, int>& userParams) override
  {
    this->program.membot = this->GetParameter("mem_bot", userParams, 0);

    if (this->program.membot < 0 || this->program.membot > 1)
    {
      MIKTEX_FATAL_ERROR(MIKTEXTEXT("mem_bot must be 0 or 1."));
    }

    TeXMFMemoryHandlerImpl<PROGRAM_CLASS>::Allocate(userParams);

    this->program.maxinopen = this->GetCheckedParameter("max_in_open", this->program.infmaxinopen, this->program.supmaxinopen, userParams, texapp::texapp::max_in_open());
    this->program.nestsize = this->GetCheckedParameter("nest_size", this->program.infnestsize, this->program.supnestsize, userParams, texapp::texapp::nest_size());
    this->program.savesize = this->GetCheckedParameter("save_size", this->program.infsavesize, this->program.supsavesize, userParams, texapp::texapp::save_size());
#if defined(MIKTEX_OMEGA)
    this->program.triesize = this->GetParameter("trie_size", userParams, texapp::texapp::trie_size());
#else
    this->program.triesize = this->GetCheckedParameter("trie_size", this->program.inftriesize, this->program.suptriesize, userParams, texapp::texapp::trie_size());
#endif

#if !defined(MIKTEX_OMEGA)
    this->program.hashextra = this->GetParameter("hash_extra", userParams, texapp::texapp::hash_extra());
#endif

    this->AllocateArray("sourcefilenamestack", this->program.sourcefilenamestack, this->program.maxinopen);
    this->AllocateArray("linestack", this->program.linestack, this->program.maxinopen);
    this->AllocateArray("fullsourcefilenamestack", this->program.fullsourcefilenamestack, this->program.maxinopen);
    this->AllocateArray("inputfile", this->program.inputfile, this->program.maxinopen);
    this->AllocateArray("nest", this->program.nest, this->program.nestsize + 1);
    this->AllocateArray("savestack", this->program.savestack, this->program.savesize + 1);
    this->AllocateArray("triehash", this->program.triehash, this->program.triesize + 1);
    this->AllocateArray("triel", this->program.triel, this->program.triesize + 1);
    this->AllocateArray("trieo", this->program.trieo, this->program.triesize + 1);
    this->AllocateArray("trier", this->program.trier, this->program.triesize + 1);
    this->AllocateArray("trietaken", this->program.trietaken, this->program.triesize);

    this->AllocateArray("nameoffile", this->program.nameoffile, MiKTeX::Core::BufferSizes::MaxPath + 1);

#if !defined(MIKTEX_OMEGA)
    this->program.hyphsize = this->GetCheckedParameter("hyph_size", this->program.infhyphsize, this->program.suphyphsize, userParams, texapp::texapp::hyph_size());
    this->program.fontmax = this->GetParameter("font_max", userParams, texapp::texapp::font_max());
    this->program.fontmemsize = this->GetCheckedParameter("font_mem_size", this->program.inffontmemsize, this->program.supfontmemsize, userParams, texapp::texapp::font_mem_size());

    this->AllocateArray("trietrl", this->program.trietrl, this->program.triesize);
    this->AllocateArray("trietro", this->program.trietro, this->program.triesize);
    this->AllocateArray("trietrc", this->program.trietrc, this->program.triesize);

    this->AllocateArray("hyphword", this->program.hyphword, this->program.hyphsize);
    this->AllocateArray("hyphlist", this->program.hyphlist, this->program.hyphsize);
    this->AllocateArray("hyphlink", this->program.hyphlink, this->program.hyphsize);

    this->AllocateArray("bcharlabel", this->program.bcharlabel, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("charbase", this->program.charbase, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("depthbase", this->program.depthbase, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("extenbase", this->program.extenbase, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontarea", this->program.fontarea, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontbc", this->program.fontbc, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontbchar", this->program.fontbchar, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontcheck", this->program.fontcheck, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontdsize", this->program.fontdsize, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontec", this->program.fontec, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontfalsebchar", this->program.fontfalsebchar, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontglue", this->program.fontglue, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontname", this->program.fontname, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontparams", this->program.fontparams, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontsize", this->program.fontsize, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("fontused", this->program.fontused, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("heightbase", this->program.heightbase, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("hyphenchar", this->program.hyphenchar, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("italicbase", this->program.italicbase, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("kernbase", this->program.kernbase, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("ligkernbase", this->program.ligkernbase, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("parambase", this->program.parambase, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("skewchar", this->program.skewchar, this->program.fontmax - this->program.constfontbase);
    this->AllocateArray("triec", this->program.triec, this->program.triesize);
    this->AllocateArray("widthbase", this->program.widthbase, this->program.fontmax - this->program.constfontbase);

    if (this->texmfapp.IsInitProgram())
    {
      // memory allocated in tex-miktex-hash.ch
      this->program.yhash = nullptr;
      this->program.zeqtb = nullptr;
    }

    if (this->texmfapp.IsInitProgram() || !this->texmfapp.AmITeXCompiler() || this->texmfapp.AmI("omega"))
    {
      this->AllocateArray("fontinfo", this->program.fontinfo, this->program.fontmemsize);
    }
#endif // not Omega
  }

public:
  void Free() override
  {
    TeXMFMemoryHandlerImpl<PROGRAM_CLASS>::Free();

    this->FreeArray("linestack", this->program.linestack);
    this->FreeArray("inputstack", this->program.inputstack);
    this->FreeArray("inputfile", this->program.inputfile);
    this->FreeArray("fullsourcefilenamestack", this->program.fullsourcefilenamestack);
    this->FreeArray("sourcefilenamestack", this->program.sourcefilenamestack);
    this->FreeArray("nest", this->program.nest);
    this->FreeArray("savestack", this->program.savestack);
    this->FreeArray("triec", this->program.triec);
    this->FreeArray("triehash", this->program.triehash);
    this->FreeArray("triel", this->program.triel);
    this->FreeArray("trieo", this->program.trieo);
    this->FreeArray("trier", this->program.trier);
    this->FreeArray("trietaken", this->program.trietaken);

    this->FreeArray("nameoffile", this->program.nameoffile);

#if !defined(MIKTEX_OMEGA)
    this->FreeArray("hyphword", this->program.hyphword);
    this->FreeArray("hyphlist", this->program.hyphlist);
    this->FreeArray("hyphlink", this->program.hyphlink);

    this->FreeArray("trietrl", this->program.trietrl);
    this->FreeArray("trietro", this->program.trietro);
    this->FreeArray("trietrc", this->program.trietrc);

    this->FreeArray("bcharlabel", this->program.bcharlabel);
    this->FreeArray("charbase", this->program.charbase);
    this->FreeArray("depthbase", this->program.depthbase);
    this->FreeArray("extenbase", this->program.extenbase);
    this->FreeArray("fontarea", this->program.fontarea);
    this->FreeArray("fontbc", this->program.fontbc);
    this->FreeArray("fontbchar", this->program.fontbchar);
    this->FreeArray("fontcheck", this->program.fontcheck);
    this->FreeArray("fontdsize", this->program.fontdsize);
    this->FreeArray("fontec", this->program.fontec);
    this->FreeArray("fontfalsebchar", this->program.fontfalsebchar);
    this->FreeArray("fontglue", this->program.fontglue);
    this->FreeArray("fontinfo", this->program.fontinfo);
    this->FreeArray("fontname", this->program.fontname);
    this->FreeArray("fontparams", this->program.fontparams);
    this->FreeArray("fontsize", this->program.fontsize);
    this->FreeArray("fontused", this->program.fontused);
    this->FreeArray("heightbase", this->program.heightbase);
    this->FreeArray("hyphenchar", this->program.hyphenchar);
    this->FreeArray("italicbase", this->program.italicbase);
    this->FreeArray("kernbase", this->program.kernbase);
    this->FreeArray("ligkernbase", this->program.ligkernbase);
    this->FreeArray("parambase", this->program.parambase);
    this->FreeArray("skewchar", this->program.skewchar);
    this->FreeArray("widthbase", this->program.widthbase);
#endif // not Omega
  }

public:
  void Check() override
  {
    TeXMFMemoryHandlerImpl<PROGRAM_CLASS>::Check();

    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.linestack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.inputstack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.inputfile);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fullsourcefilenamestack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.sourcefilenamestack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.nest);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.savestack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.triec);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.triehash);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.triel);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.trieo);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.trier);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.trietaken);

    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.nameoffile);

#if !defined(MIKTEX_OMEGA)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.hyphword);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.hyphlist);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.hyphlink);

    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.trietrl);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.trietro);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.trietrc);

    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.bcharlabel);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.charbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.depthbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.extenbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontarea);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontbc);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontbchar);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontcheck);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontdsize);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontec);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontfalsebchar);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontglue);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontinfo);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontname);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontparams);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontsize);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.fontused);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.heightbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.hyphenchar);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.italicbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.kernbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.ligkernbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.parambase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.skewchar);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.widthbase);
#endif // not Omega
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
