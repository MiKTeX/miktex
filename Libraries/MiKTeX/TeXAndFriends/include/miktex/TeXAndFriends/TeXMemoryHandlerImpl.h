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

template<class PROGRAM_CLASS> class TeXMemoryHandlerImpl :
  public TeXMFMemoryHandlerImpl<PROGRAM_CLASS>
{
public:
  TeXMemoryHandlerImpl(PROGRAM_CLASS& program, TeXMFApp& texmfapp) :
    TeXMFMemoryHandlerImpl(program, texmfapp)
  {
  }

public:
  void Allocate(const std::unordered_map<std::string, int>& userParams) override
  {
    program.membot = GetParameter("mem_bot", userParams, 0);

    if (program.membot < 0 || program.membot > 1)
    {
      MIKTEX_FATAL_ERROR(MIKTEXTEXT("mem_bot must be 0 or 1."));
    }

    TeXMFMemoryHandlerImpl::Allocate(userParams);

    program.maxinopen = GetCheckedParameter("max_in_open", program.infmaxinopen, program.supmaxinopen, userParams, texapp::texapp::max_in_open());
    program.nestsize = GetCheckedParameter("nest_size", program.infnestsize, program.supnestsize, userParams, texapp::texapp::nest_size());
    program.savesize = GetCheckedParameter("save_size", program.infsavesize, program.supsavesize, userParams, texapp::texapp::save_size());
#if defined(MIKTEX_OMEGA)
    program.triesize = GetParameter("trie_size", userParams, texapp::texapp::trie_size());
#else
    program.triesize = GetCheckedParameter("trie_size", program.inftriesize, program.suptriesize, userParams, texapp::texapp::trie_size());
#endif

#if !defined(MIKTEX_OMEGA)
    program.hashextra = GetParameter("hash_extra", userParams, texapp::texapp::hash_extra());
#endif

    AllocateArray("sourcefilenamestack", program.sourcefilenamestack, program.maxinopen);
    AllocateArray("linestack", program.linestack, program.maxinopen);
    AllocateArray("fullsourcefilenamestack", program.fullsourcefilenamestack, program.maxinopen);
    AllocateArray("inputfile", program.inputfile, program.maxinopen);
    AllocateArray("nest", program.nest, program.nestsize + 1);
    AllocateArray("savestack", program.savestack, program.savesize + 1);
    AllocateArray("triehash", program.triehash, program.triesize + 1);
    AllocateArray("triel", program.triel, program.triesize + 1);
    AllocateArray("trieo", program.trieo, program.triesize + 1);
    AllocateArray("trier", program.trier, program.triesize + 1);
    AllocateArray("trietaken", program.trietaken, program.triesize);

    AllocateArray("nameoffile", program.nameoffile, MiKTeX::Core::BufferSizes::MaxPath + 1);

#if !defined(MIKTEX_OMEGA)
    program.hyphsize = GetCheckedParameter("hyph_size", program.infhyphsize, program.suphyphsize, userParams, texapp::texapp::hyph_size());
    program.fontmax = GetParameter("font_max", userParams, texapp::texapp::font_max());
    program.fontmemsize = GetCheckedParameter("font_mem_size", program.inffontmemsize, program.supfontmemsize, userParams, texapp::texapp::font_mem_size());

    AllocateArray("trietrl", program.trietrl, program.triesize);
    AllocateArray("trietro", program.trietro, program.triesize);
    AllocateArray("trietrc", program.trietrc, program.triesize);

    AllocateArray("hyphword", program.hyphword, program.hyphsize);
    AllocateArray("hyphlist", program.hyphlist, program.hyphsize);
    AllocateArray("hyphlink", program.hyphlink, program.hyphsize);

    AllocateArray("bcharlabel", program.bcharlabel, program.fontmax - program.constfontbase);
    AllocateArray("charbase", program.charbase, program.fontmax - program.constfontbase);
    AllocateArray("depthbase", program.depthbase, program.fontmax - program.constfontbase);
    AllocateArray("extenbase", program.extenbase, program.fontmax - program.constfontbase);
    AllocateArray("fontarea", program.fontarea, program.fontmax - program.constfontbase);
    AllocateArray("fontbc", program.fontbc, program.fontmax - program.constfontbase);
    AllocateArray("fontbchar", program.fontbchar, program.fontmax - program.constfontbase);
    AllocateArray("fontcheck", program.fontcheck, program.fontmax - program.constfontbase);
    AllocateArray("fontdsize", program.fontdsize, program.fontmax - program.constfontbase);
    AllocateArray("fontec", program.fontec, program.fontmax - program.constfontbase);
    AllocateArray("fontfalsebchar", program.fontfalsebchar, program.fontmax - program.constfontbase);
    AllocateArray("fontglue", program.fontglue, program.fontmax - program.constfontbase);
    AllocateArray("fontname", program.fontname, program.fontmax - program.constfontbase);
    AllocateArray("fontparams", program.fontparams, program.fontmax - program.constfontbase);
    AllocateArray("fontsize", program.fontsize, program.fontmax - program.constfontbase);
    AllocateArray("fontused", program.fontused, program.fontmax - program.constfontbase);
    AllocateArray("heightbase", program.heightbase, program.fontmax - program.constfontbase);
    AllocateArray("hyphenchar", program.hyphenchar, program.fontmax - program.constfontbase);
    AllocateArray("italicbase", program.italicbase, program.fontmax - program.constfontbase);
    AllocateArray("kernbase", program.kernbase, program.fontmax - program.constfontbase);
    AllocateArray("ligkernbase", program.ligkernbase, program.fontmax - program.constfontbase);
    AllocateArray("parambase", program.parambase, program.fontmax - program.constfontbase);
    AllocateArray("skewchar", program.skewchar, program.fontmax - program.constfontbase);
    AllocateArray("triec", program.triec, program.triesize);
    AllocateArray("widthbase", program.widthbase, program.fontmax - program.constfontbase);

    if (texmfapp.IsInitProgram())
    {
      // memory allocated in tex-miktex-hash.ch
      program.yhash = 0;
      program.zeqtb = 0;
    }

    if (texmfapp.IsInitProgram() || !texmfapp.AmITeXCompiler() || texmfapp.AmI("omega"))
    {
      AllocateArray("fontinfo", program.fontinfo, program.fontmemsize);
    }
#endif // not Omega

#if defined(MIKTEX_OMEGA)
    program.trieopsize = GetParameter("trie_op_size", userParams, omega::omega::trie_op_size());

    AllocateArray("hyfdistance", program.hyfdistance, program.trieopsize);
    AllocateArray("hyfnext", program.hyfnext, program.trieopsize);
    AllocateArray("hyfnum", program.hyfnum, program.trieopsize);
    AllocateArray("trie", program.trie, program.triesize);
    AllocateArray("trieophash", program.trieophash, 2 * program.trieopsize);
    AllocateArray("trieoplang", program.trieoplang, program.trieopsize);
    AllocateArray("trieopval", program.trieopval, program.trieopsize);
#endif // Omega
  }

public:
  void Free() override
  {
    TeXMFMemoryHandlerImpl::Free();

    FreeArray("linestack", program.linestack);
    FreeArray("inputstack", program.inputstack);
    FreeArray("inputfile", program.inputfile);
    FreeArray("fullsourcefilenamestack", program.fullsourcefilenamestack);
    FreeArray("sourcefilenamestack", program.sourcefilenamestack);
    FreeArray("nest", program.nest);
    FreeArray("savestack", program.savestack);
    FreeArray("triec", program.triec);
    FreeArray("triehash", program.triehash);
    FreeArray("triel", program.triel);
    FreeArray("trieo", program.trieo);
    FreeArray("trier", program.trier);
    FreeArray("trietaken", program.trietaken);

    FreeArray("nameoffile", program.nameoffile);

#if !defined(MIKTEX_OMEGA)
    FreeArray("hyphword", program.hyphword);
    FreeArray("hyphlist", program.hyphlist);
    FreeArray("hyphlink", program.hyphlink);

    FreeArray("trietrl", program.trietrl);
    FreeArray("trietro", program.trietro);
    FreeArray("trietrc", program.trietrc);

    FreeArray("bcharlabel", program.bcharlabel);
    FreeArray("charbase", program.charbase);
    FreeArray("depthbase", program.depthbase);
    FreeArray("extenbase", program.extenbase);
    FreeArray("fontarea", program.fontarea);
    FreeArray("fontbc", program.fontbc);
    FreeArray("fontbchar", program.fontbchar);
    FreeArray("fontcheck", program.fontcheck);
    FreeArray("fontdsize", program.fontdsize);
    FreeArray("fontec", program.fontec);
    FreeArray("fontfalsebchar", program.fontfalsebchar);
    FreeArray("fontglue", program.fontglue);
    FreeArray("fontinfo", program.fontinfo);
    FreeArray("fontname", program.fontname);
    FreeArray("fontparams", program.fontparams);
    FreeArray("fontsize", program.fontsize);
    FreeArray("fontused", program.fontused);
    FreeArray("heightbase", program.heightbase);
    FreeArray("hyphenchar", program.hyphenchar);
    FreeArray("italicbase", program.italicbase);
    FreeArray("kernbase", program.kernbase);
    FreeArray("ligkernbase", program.ligkernbase);
    FreeArray("parambase", program.parambase);
    FreeArray("skewchar", program.skewchar);
    FreeArray("widthbase", program.widthbase);
#endif // not Omega

#if defined(MIKTEX_OMEGA)
    FreeArray("hyfdistance", program.hyfdistance);
    FreeArray("hyfnext", program.hyfnext);
    FreeArray("hyfnum", program.hyfnum);
    FreeArray("trie", program.trie);

    FreeArray("trieophash", program.trieophash);
    FreeArray("trieoplang", program.trieoplang);
    FreeArray("trieopval", program.trieopval);
#endif
  }

public:
  void Check() override
  {
    TeXMFMemoryHandlerImpl::Check();
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.linestack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.inputstack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.inputfile);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fullsourcefilenamestack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.sourcefilenamestack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.nest);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.savestack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.triec);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.triehash);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.triel);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trieo);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trier);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trietaken);

    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.nameoffile);

#if !defined(MIKTEX_OMEGA)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.hyphword);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.hyphlist);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.hyphlink);

    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trietrl);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trietro);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trietrc);

    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.bcharlabel);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.charbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.depthbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.extenbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontarea);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontbc);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontbchar);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontcheck);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontdsize);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontec);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontfalsebchar);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontglue);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontinfo);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontname);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontparams);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontsize);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontused);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.heightbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.hyphenchar);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.italicbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.kernbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.ligkernbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.parambase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.skewchar);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.widthbase);
#endif // not Omega

#if defined(MIKTEX_OMEGA)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.hyfdistance);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.hyfnext);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.hyfnum);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trie);

    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trieophash);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trieoplang);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.trieopval);
#endif
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
