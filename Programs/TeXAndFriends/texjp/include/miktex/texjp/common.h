#pragma once

#include <unordered_set>

#include <miktex/TeXAndFriends/TeXMFApp>

#include <kanji.h>

#define byte1 BYTE1
#define byte2 BYTE2
#define byte3 BYTE3
#define byte4 BYTE4
#define frombuff fromBUFF
#define fromdvi fromDVI
#define fromeuc fromEUC
#define fromjis fromJIS
#define fromkuten fromKUTEN
#define fromsjis fromSJIS
#define fromucs fromUCS
#define hi Hi
#define isinternaleuc is_internalEUC
#define isinternaluptex is_internalUPTEX
#define lo Lo
#define tobuff toBUFF
#define todvi toDVI
#define toucs toUCS
#define ucstoutf8 UCStoUTF8

template<class FileType> inline void miktexprintencstring(FileType& f)
{
  fprintf(f, " (%s)", get_enc_string());
}

namespace MiKTeX
{
    namespace TeXjp
    {
        template<class BASE> class WebApp :
            public BASE
        {
        private:
            enum {
                OPT_KANJI = 10000,
                OPT_KANJI_INTERNAL,
            };

        private:
            std::string T_(const char* msgId)
            {
                return msgId;
            }

        public:
            void AddOptions() override
            {
                BASE::AddOptions();
                AddOption("kanji", T_("set Japanese encoding (ENC=euc|jis|sjis|utf8)."), OPT_KANJI, POPT_ARG_STRING, "ENC");
                AddOption("kanji-internal", T_("set Japanese internal encoding (ENC=euc|sjis)."), OPT_KANJI_INTERNAL, POPT_ARG_STRING, "ENC");
            }

        public:
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
        };

        template<class BASE> class WebAppInputLine :
            public WebApp<BASE>
        {
        public:
            size_t InputLineInternal(FILE* f, char* buffer, size_t bufferSize, size_t bufferPosition, int& lastChar) const override
            {
                return static_cast<size_t>(input_line2(f, reinterpret_cast<unsigned char*>(buffer), static_cast<long>(bufferPosition), static_cast<long>(bufferSize), &lastChar));
            }

        private:
            std::unordered_set<const FILE*> inputFiles;

        public:
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

        public:
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

        public:
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

        public:
            MiKTeX::Util::PathName DecodeFileName(const MiKTeX::Util::PathName& fileNameInternalEncoding) override
            {
                return MiKTeX::Util::PathName(reinterpret_cast<char*>(ptenc_from_internal_enc_string_to_utf8(reinterpret_cast<const unsigned char*>(fileNameInternalEncoding.GetData()))));
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

        public:
            void Allocate(const std::unordered_map<std::string, int>& userParams) override
            {
                BASE::Allocate(userParams);
                MIKTEX_ASSERT(BASE::program.constfontbase == 0);
                size_t nFonts = BASE::program.fontmax - BASE::program.constfontbase;
                AllocateArray("fontdir", BASE::program.fontdir, nFonts);
                AllocateArray("fontnumext", BASE::program.fontnumext, nFonts);
                AllocateArray("ctypebase", BASE::program.ctypebase, nFonts);
            }

        public:
            void Free() override
            {
                BASE::Free();
                FreeArray("fontdir", BASE::program.fontdir);
                FreeArray("fontnumext", BASE::program.fontnumext);
                FreeArray("ctypebase", BASE::program.ctypebase);
            }

        public:
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
