/**
 * @file miktex/Wrappers/PoptWrapper.h
 * @author Christian Schenk
 * @brief popt library wrapper
 *
 * @copyright Copyright © 2001-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include <popt.h>

#define MIKTEX_POPT_BEGIN_NAMESPACE             \
    namespace MiKTeX {                          \
        namespace Wrappers {

#define MIKTEX_POPT_END_NAMESPACE               \
        }                                       \
    }

#include <string>
#include <vector>

MIKTEX_POPT_BEGIN_NAMESPACE;

class PoptWrapper
{

public:

    PoptWrapper() = default;
    PoptWrapper(const PoptWrapper& other) = delete;
    PoptWrapper& operator=(const PoptWrapper& other) = delete;
    PoptWrapper(PoptWrapper&& other) = delete;
    PoptWrapper& operator=(PoptWrapper&& other) = delete;

    virtual ~PoptWrapper() noexcept
    {
        Dispose();
    }

    PoptWrapper(const char* name, int argc, const char** argv, const struct poptOption* options, int flags = 0)
    {
        Construct(name, argc, argv, options, flags);
    }

    PoptWrapper(int argc, const char** argv, const struct poptOption* options, int flags = 0)
    {
        Construct(argc, argv, options, flags);
    }

    void Construct(const char* name, int argc, const char** argv, const struct poptOption* options, int flags = 0)
    {
        ctx = poptGetContext(name, argc, argv, options, flags);
    }

    void Construct(int argc, const char** argv, const struct poptOption* options, int flags = 0)
    {
        Construct(nullptr, argc, argv, options, flags);
    }

    void Reset()
    {
        poptResetContext(ctx);
    }

    int AddAlias(const char* longName, char shortName, int argc, const char** argvFreeable)
    {
        return poptAddAlias(ctx, { longName, shortName, argc, argvFreeable }, 0);
    }

    void Dispose()
    {
        if (ctx != nullptr)
        {
            poptFreeContext(ctx);
            ctx = nullptr;
        }
    }

    operator poptContext() const
    {
        return ctx;
    }

    operator poptContext()
    {
        return ctx;
    }

    void SetOtherOptionHelp(const std::string& usage)
    {
        poptSetOtherOptionHelp(ctx, usage.c_str());
    }

    void PrintUsage(FILE* stream = stdout, int flags = 0) const
    {
        poptPrintUsage(ctx, stream, flags);
    }

    void PrintHelp(FILE* stream = stdout, int flags = 0) const
    {
        poptPrintHelp(ctx, stream, flags);
    }

    int GetNextOpt()
    {
        return poptGetNextOpt(ctx);
    }

    std::string GetOptArg()
    {
        std::string result;
        char* lpsz = poptGetOptArg(ctx);
        if (lpsz != nullptr)
        {
            result = lpsz;
            free(lpsz);
        }
        return result;
    }

    std::vector<std::string> GetLeftovers()
    {
        std::vector<std::string> result;
        const char** args = poptGetArgs(ctx);
        if (args != nullptr)
        {
            for (const char** a = args; *a != nullptr; ++a)
            {
                result.push_back(*a);
            }
        }
        return result;
    }

    std::string BadOption(int flags)
    {
        return poptBadOption(ctx, flags);
    }

    static std::string Strerror(int error)
    {
        return poptStrerror(error);
    }

private:

    poptContext ctx = nullptr;
};

MIKTEX_POPT_END_NAMESPACE;
