/**
 * @file miktex/TeXjp/common.h
 * @author Christian Schenk
 * @brief MiKTeX TeXjp base classes
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include <unordered_set>

#include <miktex/TeXAndFriends/TeXMFApp>

#include <kanji.h>

#define byte1 BYTE1
#define byte2 BYTE2
#define byte3 BYTE3
#define byte4 BYTE4
#define frombuff fromBUFF
#define frombuffshort fromBUFFshort
#define fromdvi fromDVI
#define fromeuc fromEUC
#define fromjis fromJIS
#define fromkuten fromKUTEN
#define fromsjis fromSJIS
#define fromucs fromUCS
#define hi Hi
#define isinternaleuc is_internalEUC
#define isinternaluptex is_internalUPTEX
#define isterminalutf8 is_terminalUTF8
#define lo Lo
#define tobuff toBUFF
#define todvi toDVI
#define toucs toUCS
#define ucstoutf8 UCStoUTF8

template<class FileType> inline void miktexprintencstring(FileType& f)
{
    fprintf(f, " (%s)", get_enc_string());
}

template<class FileType> inline void miktexwritechar(FileType& f, C4P::C4P_unsigned16 ch)
{
    putc2(static_cast<int>(ch), f);
}

namespace MiKTeX
{
    namespace TeXjp
    {
        template<class BASE> class WebApp :
            public BASE
        {

        public:

            void AddOptions() override
            {
                BASE::AddOptions();
                BASE::AddOption("kanji", T_("set Japanese encoding (ENC=euc|jis|sjis|utf8)."), OPT_KANJI, POPT_ARG_STRING, "ENC");
                BASE::AddOption("kanji-internal", T_("set Japanese internal encoding (ENC=euc|sjis)."), OPT_KANJI_INTERNAL, POPT_ARG_STRING, "ENC");
            }

            bool ProcessOption(int opt, const std::string& optArg) override
            {
                bool done = true;
                switch (opt)
                {
                case OPT_KANJI:
                    if (!set_enc_string (optArg.c_str(), nullptr))
                    {
                        std::cerr << T_("Unknown encoding: ") << optArg << std::endl;
                        throw 1;
                    }
                    break;
                case OPT_KANJI_INTERNAL:
                    if (!set_enc_string (nullptr, optArg.c_str()))
                    {
                        std::cerr << T_("Unknown encoding: ") << optArg << std::endl;
                        throw 1;
                    }
                    break;
                default:
                    done = BASE::ProcessOption(opt, optArg);
                    break;
                }
                return done;
            }

        private:

            enum
            {
                OPT_KANJI = 10000,
                OPT_KANJI_INTERNAL,
            };

            std::string T_(const char* msgId)
            {
                return msgId;
            }

        };

        template<class BASE> class TeXEngineBase :
            public WebApp<BASE>
        {

        public:

            int GetJobName(int fallbackJobName) const override
            {
                auto s = BASE::GetJobName(fallbackJobName);
                auto stringHandler = BASE::GetStringHandler();
                auto strstart = stringHandler->strstart();
                auto strpool = stringHandler->strpool16();
                auto begin = strstart[s];
                auto end = strstart[s + 1];
                auto pos = begin;
                while (pos < end)
                {
                    auto charLen = multistrlenshort(reinterpret_cast<unsigned short*>(strpool), end, pos);
                    if (charLen > 1)
                    {
                        for (int charEnd = pos + charLen; pos < charEnd; ++pos)
                        {
                            strpool[pos] = (0xff & strpool[pos]) + 0x100;
                        }
                    }
                    else
                    {
                        ++pos;
                    }
                }
                return s;
            }

            size_t InputLineInternal(FILE* f, char* buffer, char* buffer2, size_t bufferSize, size_t bufferPosition, int& lastChar) const override
            {
                return static_cast<size_t>(input_line2(f, reinterpret_cast<unsigned char*>(buffer), reinterpret_cast<unsigned char*>(buffer2), static_cast<long>(bufferPosition), static_cast<long>(bufferSize), &lastChar));
            }

            FILE* OpenFileInternal(const MiKTeX::Util::PathName& path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access) override
            {
                if (mode == MiKTeX::Core::FileMode::Command || access != MiKTeX::Core::FileAccess::Read)
                {
                    return BASE::OpenFileInternal(path, mode, access);
                }
                FILE* f = nkf_open(path.GetData(), "rb");
                if (f != nullptr)
                {
                    inputFiles.insert(f);
                }
                return f;
            }

            FILE* TryOpenFileInternal(const MiKTeX::Util::PathName& path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access) override
            {
                if (mode == MiKTeX::Core::FileMode::Command || access != MiKTeX::Core::FileAccess::Read)
                {
                    return BASE::TryOpenFileInternal(path, mode, access);
                }
                FILE* f = nkf_open(path.GetData(), "rb");
                if (f != nullptr)
                {
                    inputFiles.insert(f);
                }
                return f;
            }

            void CloseFileInternal(FILE* f) override
            {
                std::unordered_set<const FILE*>::iterator it = inputFiles.find(f);
                if (it == inputFiles.end())
                {
                    BASE::CloseFileInternal(f);
                    return;
                }
                inputFiles.erase(it);
                nkf_close(f);
            }

            MiKTeX::Util::PathName DecodeFileName(const MiKTeX::Util::PathName& fileNameInternalEncoding) override
            {
                auto decoded = ptenc_from_internal_enc_string_to_utf8(reinterpret_cast<const unsigned char*>(fileNameInternalEncoding.GetData()));
                if (decoded == nullptr)
                {
                    return fileNameInternalEncoding;
                }
                MiKTeX::Util::PathName result(reinterpret_cast<char*>(decoded));
                // FIXME: should be xfree
                free(decoded);
                return result;
            }

        private:

            std::unordered_set<const FILE*> inputFiles;
        };

        template<class BASE, class PROGRAM_CLASS> class PTeXInputOutputImpl :
            public BASE
        {

        public:

            PTeXInputOutputImpl(PROGRAM_CLASS& program) :
                BASE(program)
            {                
            }

            char* buffer2() override
            {
                return reinterpret_cast<char*>(BASE::program.buffer2);
            }
        };

        template<class BASE, class PROGRAM_CLASS> class PTeXMemoryHandlerImpl :
            public BASE
        {

        public:
        
            PTeXMemoryHandlerImpl(PROGRAM_CLASS& program, MiKTeX::TeXAndFriends::TeXMFApp& texmfapp) :
                BASE(program, texmfapp)
            {
            }

            void Allocate(const std::unordered_map<std::string, int>& userParams) override
            {
                BASE::Allocate(userParams);
                BASE::AllocateArray("buffer2", BASE::program.buffer2, BASE::program.bufsize);
                MIKTEX_ASSERT(BASE::program.constfontbase == 0);
                size_t nFonts = BASE::program.fontmax - BASE::program.constfontbase;
                BASE::AllocateArray("fontdir", BASE::program.fontdir, nFonts);
                BASE::AllocateArray("fontnumext", BASE::program.fontnumext, nFonts);
                BASE::AllocateArray("ctypebase", BASE::program.ctypebase, nFonts);
            }

            void Free() override
            {
                BASE::Free();
                BASE::FreeArray("buffer2", BASE::program.buffer2);
                BASE::FreeArray("fontdir", BASE::program.fontdir);
                BASE::FreeArray("fontnumext", BASE::program.fontnumext);
                BASE::FreeArray("ctypebase", BASE::program.ctypebase);
            }

            void Check() override
            {
                BASE::Check();
                MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(BASE::program.fontdir);
                MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(BASE::program.fontnumext);
                MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(BASE::program.ctypebase);
            }
        };
    }
}
