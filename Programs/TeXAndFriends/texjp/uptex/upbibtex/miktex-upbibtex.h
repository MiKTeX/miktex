/* miktex-upbibtex.h:

   Copyright (C) 2021 Christian Schenk

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

#pragma once

#include "miktex-upbibtex-config.h"

#include <iostream>

#define IMPLEMENT_TCX 1

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/FileType>
#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/WebAppInputLine>

#include "upbibtex.h"

namespace bibtex {
#include <miktex/bibtex.defaults.h>
}

#include <miktex/uptex.h>

extern UPBIBTEXPROGCLASS UPBIBTEXPROG;

class UPBIBTEXAPPCLASS :
    public MiKTeX::TeXAndFriends::WebAppInputLine
{
public:
    template<typename T> void Reallocate(T*& p, size_t n)
    {
        size_t amount = n * sizeof(T);
        void* p2 = realloc(p, amount);
        if (p2 == nullptr && amount > 0)
        {
            FatalError(MIKTEXTEXT("Virtual memory exhausted."));
        }
        p = reinterpret_cast<T*>(p2);
    }
  
public:
    template<typename T> void PascalReallocate(T*& p, size_t n)
    {
        return Reallocate(p, n + 1);
    }

public:
    template<typename T> void Allocate(T*& p, size_t n)
    {
        p = nullptr;
        Reallocate(p, n);
    }

public:
    template<typename T> void PascalAllocate(T*& p, size_t n)
    {
        Allocate(p, n + 1);
    }

public:
    template<typename T> void Free(T*& p)
    {
        free(p);
        p = nullptr;
    }

private:
    std::shared_ptr<MiKTeX::Core::Session> session;
  
private:
    MiKTeX::TeXAndFriends::CharacterConverterImpl<UPBIBTEXPROGCLASS> charConv{ UPBIBTEXPROG };

private:
    MiKTeX::TeXAndFriends::InitFinalizeImpl<UPBIBTEXPROGCLASS> initFinalize{ UPBIBTEXPROG };

private:
    MiKTeX::TeXAndFriends::InputOutputImpl<UPBIBTEXPROGCLASS> inputOutput{ UPBIBTEXPROG };

public:
    void Init(std::vector<char*>& args) override
    {
        SetCharacterConverter(&charConv);
        SetInitFinalize(&initFinalize);
        SetInputOutput(&inputOutput);
        WebAppInputLine::Init(args);
        session = GetSession();
#if defined(IMPLEMENT_TCX)
        EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
#endif
        UPBIBTEXPROG.entstrsize = session->GetConfigValue(MIKTEX_CONFIG_SECTION_BIBTEX, "ent_str_size", MiKTeX::Configuration::ConfigValue(bibtex::bibtex::ent_str_size())).GetInt();
        UPBIBTEXPROG.globstrsize = session->GetConfigValue(MIKTEX_CONFIG_SECTION_BIBTEX, "glob_str_size", MiKTeX::Configuration::ConfigValue(bibtex::bibtex::glob_str_size())).GetInt();
        UPBIBTEXPROG.maxstrings = session->GetConfigValue(MIKTEX_CONFIG_SECTION_BIBTEX, "max_strings", MiKTeX::Configuration::ConfigValue(bibtex::bibtex::max_strings())).GetInt();
        UPBIBTEXPROG.mincrossrefs = session->GetConfigValue(MIKTEX_CONFIG_SECTION_BIBTEX, "min_crossrefs", MiKTeX::Configuration::ConfigValue(bibtex::bibtex::min_crossrefs())).GetInt();
        UPBIBTEXPROG.hashsize = UPBIBTEXPROG.maxstrings;
        const int HASH_SIZE_MIN = 5000;
        if (UPBIBTEXPROG.hashsize < HASH_SIZE_MIN)
        {
            UPBIBTEXPROG.hashsize = HASH_SIZE_MIN;
        }
        UPBIBTEXPROG.hashmax = UPBIBTEXPROG.hashsize + UPBIBTEXPROG.hashbase - 1;
        UPBIBTEXPROG.endofdef = UPBIBTEXPROG.hashmax + 1;
        UPBIBTEXPROG.undefined = UPBIBTEXPROG.hashmax + 1;
        UPBIBTEXPROG.bufsize = UPBIBTEXPROG.bufsizedef;
        UPBIBTEXPROG.litstksize = UPBIBTEXPROG.litstksizedef;
        UPBIBTEXPROG.maxbibfiles = UPBIBTEXPROG.maxbibfilesdef;
        UPBIBTEXPROG.maxglobstrs = UPBIBTEXPROG.maxglobstrsdef;
        UPBIBTEXPROG.maxcites = UPBIBTEXPROG.maxcitesdef;
        UPBIBTEXPROG.maxentints = UPBIBTEXPROG.maxentintsdef;
        UPBIBTEXPROG.maxentstrs = UPBIBTEXPROG.maxentstrsdef;
        UPBIBTEXPROG.maxfields = UPBIBTEXPROG.maxfieldsdef;
        UPBIBTEXPROG.poolsize = UPBIBTEXPROG.poolsizedef;
        UPBIBTEXPROG.singlefnspace = UPBIBTEXPROG.singlefnspacedef;
        UPBIBTEXPROG.wizfnspace = UPBIBTEXPROG.wizfnspacedef;
        UPBIBTEXPROG.entryints = nullptr;
        UPBIBTEXPROG.entrystrs = nullptr;
        PascalAllocate(UPBIBTEXPROG.bibfile, UPBIBTEXPROG.maxbibfiles);
        PascalAllocate(UPBIBTEXPROG.biblist, UPBIBTEXPROG.maxbibfiles);
        PascalAllocate(UPBIBTEXPROG.buffer, UPBIBTEXPROG.bufsize);
        PascalAllocate(UPBIBTEXPROG.citeinfo, UPBIBTEXPROG.maxcites);
        PascalAllocate(UPBIBTEXPROG.citelist, UPBIBTEXPROG.maxcites);
        PascalAllocate(UPBIBTEXPROG.entryexists, UPBIBTEXPROG.maxcites);
        PascalAllocate(UPBIBTEXPROG.exbuf, UPBIBTEXPROG.bufsize);
        PascalAllocate(UPBIBTEXPROG.fieldinfo, UPBIBTEXPROG.maxfields);
        PascalAllocate(UPBIBTEXPROG.fntype, UPBIBTEXPROG.hashmax);
        PascalAllocate(UPBIBTEXPROG.glbstrend, UPBIBTEXPROG.maxglobstrs);
        PascalAllocate(UPBIBTEXPROG.glbstrptr, UPBIBTEXPROG.maxglobstrs);
        PascalAllocate(UPBIBTEXPROG.globalstrs, static_cast<size_t>(UPBIBTEXPROG.maxglobstrs) * (static_cast<size_t>(UPBIBTEXPROG.globstrsize) + 1));
        PascalAllocate(UPBIBTEXPROG.hashilk, UPBIBTEXPROG.hashmax);
        PascalAllocate(UPBIBTEXPROG.hashnext, UPBIBTEXPROG.hashmax);
        PascalAllocate(UPBIBTEXPROG.hashtext, UPBIBTEXPROG.hashmax);
        PascalAllocate(UPBIBTEXPROG.ilkinfo, UPBIBTEXPROG.hashmax);
        PascalAllocate(UPBIBTEXPROG.litstack, UPBIBTEXPROG.litstksize);
        PascalAllocate(UPBIBTEXPROG.litstktype, UPBIBTEXPROG.litstksize);
        PascalAllocate(UPBIBTEXPROG.namesepchar, UPBIBTEXPROG.bufsize);
        PascalAllocate(UPBIBTEXPROG.nametok, UPBIBTEXPROG.bufsize);
        PascalAllocate(UPBIBTEXPROG.outbuf, UPBIBTEXPROG.bufsize);
        PascalAllocate(UPBIBTEXPROG.spreamble, UPBIBTEXPROG.maxbibfiles);
        PascalAllocate(UPBIBTEXPROG.strpool, UPBIBTEXPROG.poolsize);
        PascalAllocate(UPBIBTEXPROG.strstart, UPBIBTEXPROG.maxstrings);
        PascalAllocate(UPBIBTEXPROG.svbuffer, UPBIBTEXPROG.bufsize);
        PascalAllocate(UPBIBTEXPROG.typelist, UPBIBTEXPROG.maxcites);
        PascalAllocate(UPBIBTEXPROG.wizfunctions, UPBIBTEXPROG.wizfnspace);
        UPBIBTEXPROG.computehashprime();
    }
  
public:
    void Finalize() override
    {
        Free(UPBIBTEXPROG.bibfile);
        Free(UPBIBTEXPROG.biblist);
        Free(UPBIBTEXPROG.buffer);
        Free(UPBIBTEXPROG.citeinfo);
        Free(UPBIBTEXPROG.citelist);
        Free(UPBIBTEXPROG.entryexists);
        Free(UPBIBTEXPROG.entryints);
        Free(UPBIBTEXPROG.entrystrs);
        Free(UPBIBTEXPROG.exbuf);
        Free(UPBIBTEXPROG.fieldinfo);
        Free(UPBIBTEXPROG.fntype);
        Free(UPBIBTEXPROG.glbstrend);
        Free(UPBIBTEXPROG.glbstrptr);
        Free(UPBIBTEXPROG.globalstrs);
        Free(UPBIBTEXPROG.hashilk);
        Free(UPBIBTEXPROG.hashnext);
        Free(UPBIBTEXPROG.hashtext);
        Free(UPBIBTEXPROG.ilkinfo);
        Free(UPBIBTEXPROG.litstack);
        Free(UPBIBTEXPROG.litstktype);
        Free(UPBIBTEXPROG.namesepchar);
        Free(UPBIBTEXPROG.nametok);
        Free(UPBIBTEXPROG.outbuf);
        Free(UPBIBTEXPROG.spreamble);
        Free(UPBIBTEXPROG.strpool);
        Free(UPBIBTEXPROG.strstart);
        Free(UPBIBTEXPROG.svbuffer);
        Free(UPBIBTEXPROG.typelist);
        Free(UPBIBTEXPROG.wizfunctions);
        WebAppInputLine::Finalize();
    }

#define OPT_KANJI 1000
#define OPT_MIN_CROSSREFS 1001
#define OPT_QUIET 1002

public:
    void AddOptions() override
    {
        WebAppInputLine::AddOptions();
        AddOption("kanji", MIKTEXTEXT("Set Japanese encoding (ENC=euc|jis|sjis|utf8)."), OPT_KANJI, POPT_ARG_STRING, "ENC");
        AddOption(MIKTEXTEXT("quiet\0Suppress all output (except errors)."), OPT_QUIET, POPT_ARG_NONE);
        AddOption("silent", "quiet");
        AddOption("terse", "quiet");
    }
  
public:
    MiKTeX::Core::FileType GetInputFileType() const override
    {
        return MiKTeX::Core::FileType::BIB;
    }

public:
    std::string MIKTEXTHISCALL GetUsage() const override
    {
        return MIKTEXTEXT("[OPTION...] AUXFILE");
    }

public:
    bool ProcessOption(int opt, const std::string& optArg) override
    {
        bool done = true;
        switch (opt)
        {
        case OPT_KANJI:
            set_prior_file_enc();
            if (!set_enc_string(optArg.c_str(), optArg.c_str()))
            {
                std::cerr << MIKTEXTEXT("Unknown encoding: ") << optArg << std::endl;
                throw 1;
            }
            break;
        case OPT_MIN_CROSSREFS:
            UPBIBTEXPROG.mincrossrefs = std::stoi(optArg);
            break;
        case OPT_QUIET:
            SetQuietFlag(true);
            break;
        default:
            done = WebAppInputLine::ProcessOption(opt, optArg);
            break;
        }
        return done;
    }
  
public:
    std::string MIKTEXTHISCALL TheNameOfTheGame() const override
    {
        return "BibTeX";
    }

public:
    void BufferSizeExceeded() const override
    {
        UPBIBTEXPROG.bufferoverflow();
    }

public:
    void SetNameOfFile(const MiKTeX::Util::PathName& fileName) override
    {
        MiKTeX::TeXAndFriends::IInputOutput* inputOutput = GetInputOutput();
        Reallocate(inputOutput->nameoffile(), fileName.GetLength() + 1);
        MiKTeX::Util::StringUtil::CopyString(inputOutput->nameoffile(), fileName.GetLength() + 1, fileName.GetData());
        inputOutput->namelength() = static_cast<C4P::C4P_signed32>(fileName.GetLength());
    }

public:
    template<class T> bool OpenBstFile(T& f) const
    {
        const char* fileName = GetInputOutput()->nameoffile();
        MIKTEX_ASSERT_STRING(fileName);
        MiKTeX::Util::PathName bstFileName(fileName);
        if (!bstFileName.HasExtension())
        {
            bstFileName.SetExtension(".bst");
        }
        MiKTeX::Util::PathName path;
        if (!session->FindFile(bstFileName.ToString(), MiKTeX::Core::FileType::BST, path))
        {
            return false;
        }
        FILE* file = session->OpenFile(path, MiKTeX::Core::FileMode::Open, MiKTeX::Core::FileAccess::Read, true);
        f.Attach(file, true);
#ifdef PASCAL_TEXT_IO
        get(f);
#endif
        return true;
    }
};

extern UPBIBTEXAPPCLASS UPBIBTEXAPP;

template<class T> inline void miktexbibtexalloc(T*& p, size_t n)
{
    UPBIBTEXAPP.PascalAllocate(p, n);
}

template<class T> inline void miktexbibtexrealloc(const char* varName, T*& p, size_t n)
{
    if (UPBIBTEXPROG.logfile != nullptr)
    {
        fprintf(UPBIBTEXPROG.logfile, "Reallocating '%s' (item size: %d) to %d items.\n",
            varName, static_cast<int>(sizeof(T)), static_cast<int>(n));
    }
    UPBIBTEXAPP.PascalReallocate(p, n);
}

template<class T> inline void miktexbibtexfree(T*& p)
{
    UPBIBTEXAPP.Free(p);
}

template<class T> inline bool miktexopenbstfile(T& f)
{
    return UPBIBTEXAPP.OpenBstFile(f);
}

inline bool miktexhasextension(const char* fileName, const char* extension)
{
    return MiKTeX::Util::PathName(fileName).HasExtension(extension);
}
