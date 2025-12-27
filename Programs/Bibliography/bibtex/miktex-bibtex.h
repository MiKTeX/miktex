/**
 * @file miktex-bibtex.h
 * @author Christian Schenk
 * @brief MiKTeX-BibTeX
 *
 * @copyright Copyright © 1996-2025 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include "miktex-bibtex-config.h"

#include "miktex-bibtex-version.h"

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/FileType>
#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/WebAppInputLine>
#include <miktex/W2C/Emulation>

#if !defined(MIKTEXHELP_BIBTEX)
#include <miktex/Core/Help>
#endif

#include "bibtex.h"

namespace bibtex {
#include <miktex/bibtex.defaults.h>
}

extern BIBTEXPROGCLASS BIBTEXPROG;

class BIBTEXAPPCLASS :
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

    template<typename T> void PascalReallocate(T*& p, size_t n)
    {
        return Reallocate(p, n + 1);
    }

    template<typename T> void Allocate(T*& p, size_t n)
    {
        p = nullptr;
        Reallocate(p, n);
    }

    template<typename T> void PascalAllocate(T*& p, size_t n)
    {
        Allocate(p, n + 1);
    }

    template<typename T> void Free(T*& p)
    {
        free(p);
        p = nullptr;
    }

    void Init(std::vector<char*>& args) override
    {
        SetCharacterConverter(&charConv);
        SetInitFinalize(&initFinalize);
        SetInputOutput(&inputOutput);
        WebAppInputLine::Init(args);
        session = GetSession();
        BIBTEXPROG.entstrsize = session->GetConfigValue(MIKTEX_CONFIG_SECTION_BIBTEX, "ent_str_size", MiKTeX::Configuration::ConfigValue(bibtex::bibtex::ent_str_size())).GetInt();
        BIBTEXPROG.globstrsize = session->GetConfigValue(MIKTEX_CONFIG_SECTION_BIBTEX, "glob_str_size", MiKTeX::Configuration::ConfigValue(bibtex::bibtex::glob_str_size())).GetInt();
        BIBTEXPROG.maxstrings = session->GetConfigValue(MIKTEX_CONFIG_SECTION_BIBTEX, "max_strings", MiKTeX::Configuration::ConfigValue(bibtex::bibtex::max_strings())).GetInt();
        BIBTEXPROG.mincrossrefs = session->GetConfigValue(MIKTEX_CONFIG_SECTION_BIBTEX, "min_crossrefs", MiKTeX::Configuration::ConfigValue(bibtex::bibtex::min_crossrefs())).GetInt();
        BIBTEXPROG.hashsize = BIBTEXPROG.maxstrings;
        const int HASH_SIZE_MIN = 5000;
        if (BIBTEXPROG.hashsize < HASH_SIZE_MIN)
        {
            BIBTEXPROG.hashsize = HASH_SIZE_MIN;
        }
        BIBTEXPROG.hashmax = BIBTEXPROG.hashsize + BIBTEXPROG.hashbase - 1;
        BIBTEXPROG.endofdef = BIBTEXPROG.hashmax + 1;
        BIBTEXPROG.undefined = BIBTEXPROG.hashmax + 1;
        BIBTEXPROG.bufsize = BIBTEXPROG.bufsizedef;
        BIBTEXPROG.litstksize = BIBTEXPROG.litstksizedef;
        BIBTEXPROG.maxbibfiles = BIBTEXPROG.maxbibfilesdef;
        BIBTEXPROG.maxglobstrs = BIBTEXPROG.maxglobstrsdef;
        BIBTEXPROG.maxcites = BIBTEXPROG.maxcitesdef;
        BIBTEXPROG.maxentints = BIBTEXPROG.maxentintsdef;
        BIBTEXPROG.maxentstrs = BIBTEXPROG.maxentstrsdef;
        BIBTEXPROG.maxfields = BIBTEXPROG.maxfieldsdef;
        BIBTEXPROG.poolsize = BIBTEXPROG.poolsizedef;
        BIBTEXPROG.singlefnspace = BIBTEXPROG.singlefnspacedef;
        BIBTEXPROG.wizfnspace = BIBTEXPROG.wizfnspacedef;
        BIBTEXPROG.entryints = nullptr;
        BIBTEXPROG.entrystrs = nullptr;
        PascalAllocate(BIBTEXPROG.bibfile, BIBTEXPROG.maxbibfiles);
        PascalAllocate(BIBTEXPROG.biblist, BIBTEXPROG.maxbibfiles);
        PascalAllocate(BIBTEXPROG.buffer, BIBTEXPROG.bufsize);
        PascalAllocate(BIBTEXPROG.citeinfo, BIBTEXPROG.maxcites);
        PascalAllocate(BIBTEXPROG.citelist, BIBTEXPROG.maxcites);
        PascalAllocate(BIBTEXPROG.entryexists, BIBTEXPROG.maxcites);
        PascalAllocate(BIBTEXPROG.exbuf, BIBTEXPROG.bufsize);
        PascalAllocate(BIBTEXPROG.fieldinfo, BIBTEXPROG.maxfields);
        PascalAllocate(BIBTEXPROG.fntype, BIBTEXPROG.hashmax);
        PascalAllocate(BIBTEXPROG.glbstrend, BIBTEXPROG.maxglobstrs);
        PascalAllocate(BIBTEXPROG.glbstrptr, BIBTEXPROG.maxglobstrs);
        PascalAllocate(BIBTEXPROG.globalstrs, static_cast<size_t>(BIBTEXPROG.maxglobstrs) * (static_cast<size_t>(BIBTEXPROG.globstrsize) + 1));
        PascalAllocate(BIBTEXPROG.hashilk, BIBTEXPROG.hashmax);
        PascalAllocate(BIBTEXPROG.hashnext, BIBTEXPROG.hashmax);
        PascalAllocate(BIBTEXPROG.hashtext, BIBTEXPROG.hashmax);
        PascalAllocate(BIBTEXPROG.ilkinfo, BIBTEXPROG.hashmax);
        PascalAllocate(BIBTEXPROG.litstack, BIBTEXPROG.litstksize);
        PascalAllocate(BIBTEXPROG.litstktype, BIBTEXPROG.litstksize);
        PascalAllocate(BIBTEXPROG.namesepchar, BIBTEXPROG.bufsize);
        PascalAllocate(BIBTEXPROG.nametok, BIBTEXPROG.bufsize);
        PascalAllocate(BIBTEXPROG.outbuf, BIBTEXPROG.bufsize);
        PascalAllocate(BIBTEXPROG.spreamble, BIBTEXPROG.maxbibfiles);
        PascalAllocate(BIBTEXPROG.strpool, BIBTEXPROG.poolsize);
        PascalAllocate(BIBTEXPROG.strstart, BIBTEXPROG.maxstrings);
        PascalAllocate(BIBTEXPROG.svbuffer, BIBTEXPROG.bufsize);
        PascalAllocate(BIBTEXPROG.typelist, BIBTEXPROG.maxcites);
        PascalAllocate(BIBTEXPROG.wizfunctions, BIBTEXPROG.wizfnspace);
        BIBTEXPROG.computehashprime();
    }

    void Finalize() override
    {
        Free(BIBTEXPROG.bibfile);
        Free(BIBTEXPROG.biblist);
        Free(BIBTEXPROG.buffer);
        Free(BIBTEXPROG.citeinfo);
        Free(BIBTEXPROG.citelist);
        Free(BIBTEXPROG.entryexists);
        Free(BIBTEXPROG.entryints);
        Free(BIBTEXPROG.entrystrs);
        Free(BIBTEXPROG.exbuf);
        Free(BIBTEXPROG.fieldinfo);
        Free(BIBTEXPROG.fntype);
        Free(BIBTEXPROG.glbstrend);
        Free(BIBTEXPROG.glbstrptr);
        Free(BIBTEXPROG.globalstrs);
        Free(BIBTEXPROG.hashilk);
        Free(BIBTEXPROG.hashnext);
        Free(BIBTEXPROG.hashtext);
        Free(BIBTEXPROG.ilkinfo);
        Free(BIBTEXPROG.litstack);
        Free(BIBTEXPROG.litstktype);
        Free(BIBTEXPROG.namesepchar);
        Free(BIBTEXPROG.nametok);
        Free(BIBTEXPROG.outbuf);
        Free(BIBTEXPROG.spreamble);
        Free(BIBTEXPROG.strpool);
        Free(BIBTEXPROG.strstart);
        Free(BIBTEXPROG.svbuffer);
        Free(BIBTEXPROG.typelist);
        Free(BIBTEXPROG.wizfunctions);
        WebAppInputLine::Finalize();
    }

#define OPT_MIN_CROSSREFS 1000
#define OPT_QUIET 1001

    void AddOptions() override
    {
        WebAppInputLine::AddOptions();
        AddOption(MIKTEXTEXT("min-crossrefs\0Include item after N cross-refs; default 2."), OPT_MIN_CROSSREFS, POPT_ARG_STRING, "N");
        AddOption(MIKTEXTEXT("quiet\0Suppress all output (except errors)."), OPT_QUIET, POPT_ARG_NONE);
        AddOption("silent", "quiet");
        AddOption("terse", "quiet");
    }

    MiKTeX::Core::FileType GetInputFileType() const override
    {
        return MiKTeX::Core::FileType::BIB;
    }

    std::string MIKTEXTHISCALL GetUsage() const override
    {
        return MIKTEXTEXT("[OPTION...] AUXFILE");
    }

    bool ProcessOption(int opt, const std::string& optArg) override
    {
        bool done = true;
        switch (opt)
        {
        case OPT_MIN_CROSSREFS:
            BIBTEXPROG.mincrossrefs = std::stoi(optArg);
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

    std::string MIKTEXTHISCALL TheNameOfTheGame() const override
    {
        return "BibTeX";
    }

    unsigned long MIKTEXTHISCALL GetHelpId() const override
    {
        return MIKTEXHELP_BIBTEX;
    }

    void BufferSizeExceeded() const override
    {
        BIBTEXPROG.bufferoverflow();
    }

    void SetNameOfFile(const MiKTeX::Util::PathName& fileName) override
    {
        MiKTeX::TeXAndFriends::IInputOutput* inputOutput = GetInputOutput();
        Reallocate(inputOutput->nameoffile(), fileName.GetLength() + 1);
        MiKTeX::Util::StringUtil::CopyCeeString(inputOutput->nameoffile(), fileName.GetLength() + 1, fileName.GetData());
        inputOutput->namelength() = static_cast<C4P::C4P_signed32>(fileName.GetLength());
    }

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

private:

    std::shared_ptr<MiKTeX::Core::Session> session;
    MiKTeX::TeXAndFriends::CharacterConverterImpl<BIBTEXPROGCLASS> charConv{ BIBTEXPROG };
    MiKTeX::TeXAndFriends::InitFinalizeImpl<BIBTEXPROGCLASS> initFinalize{ BIBTEXPROG };
    MiKTeX::TeXAndFriends::InputOutputImpl<BIBTEXPROGCLASS> inputOutput{ BIBTEXPROG };
};

extern BIBTEXAPPCLASS BIBTEXAPP;

template<class T> inline void miktexbibtexalloc(T*& p, size_t n)
{
    BIBTEXAPP.PascalAllocate(p, n);
}

template<class T> inline void miktexbibtexrealloc(const char* varName, T*& p, size_t n)
{
    if (BIBTEXPROG.logfile != nullptr)
    {
        fprintf(BIBTEXPROG.logfile, "Reallocating '%s' (item size: %d) to %d items.\n",
            varName, static_cast<int>(sizeof(T)), static_cast<int>(n));
    }
    BIBTEXAPP.PascalReallocate(p, n);
}

template<class T> inline void miktexbibtexfree(T*& p)
{
    BIBTEXAPP.Free(p);
}

template<class T> inline bool miktexopenbstfile(T& f)
{
    return BIBTEXAPP.OpenBstFile(f);
}

inline bool miktexhasextension(const char* fileName, const char* extension)
{
    return MiKTeX::Util::PathName(fileName).HasExtension(extension);
}
