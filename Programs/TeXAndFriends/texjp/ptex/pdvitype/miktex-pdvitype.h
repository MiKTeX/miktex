/**
 * @file miktex-pdvitype.h
 * @author Christian Schenk
 * @brief MiKTeX pDVItype
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include "miktex-pdvitype-config.h"

#include <iostream>

#include <miktex/TeXAndFriends/WebApp>

#include <miktex/TeXjp/common.h>

#define OPT_KANJI 1000
#define OPT_MAX_PAGES 1001
#define OPT_NEW_MAG 1002
#define OPT_OUT_MODE 1003
#define OPT_RESOLUTION 1004
#define OPT_START_THERE 1005

extern PDVITYPEPROGCLASS PDVITYPEPROG;

class PDVITYPEAPPCLASS :
    public MiKTeX::TeXAndFriends::WebApp
{

public:

    void AddOptions() override
    {
        AddOption("kanji", MIKTEXTEXT("Set Japanese encoding (ENC=euc|jis|sjis|utf8)."), OPT_KANJI, POPT_ARG_STRING, "ENC");
        AddOption("max-pages", MIKTEXTEXT("Set maximum number of pages."), OPT_MAX_PAGES, POPT_ARG_STRING, "N");
        AddOption("new-mag", MIKTEXTEXT("Set new magnification."), OPT_NEW_MAG, POPT_ARG_STRING, "MAG");
        AddOption("out-mode", MIKTEXTEXT("Set output mode."), OPT_OUT_MODE, POPT_ARG_STRING, "MODE");
        AddOption("resolution", MIKTEXTEXT("Set desired resolution."), OPT_RESOLUTION, POPT_ARG_STRING, MIKTEXTEXT("NUM/DEN"));
        AddOption("start-there", MIKTEXTEXT("Set starting page."), OPT_START_THERE, POPT_ARG_STRING, "PAGESPEC");
        WebApp::AddOptions();
    }

    std::string GetUsage() const override
    {
        return MIKTEXTEXT("[OPTION...] INPUTFILE");
    }

    void Init(std::vector<char*>& args) override
    {
        WebApp::Init(args);
        PDVITYPEPROG.maxpages = 1000000;
        PDVITYPEPROG.newmag = 0;
        PDVITYPEPROG.outmode = 4;
        PDVITYPEPROG.resolution = 300.0;
        PDVITYPEPROG.startthere[0] = false;
        PDVITYPEPROG.startvals = 0;
    }

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
        case OPT_MAX_PAGES:
            if (optArg.empty() || !isdigit(optArg[0]))
            {
                BadUsage();
            }
            PDVITYPEPROG.maxpages = std::stoi(optArg);
            break;
        case OPT_NEW_MAG:
            if (optArg.empty() || !isdigit(optArg[0]))
            {
                BadUsage();
            }
            PDVITYPEPROG.newmag = std::stoi(optArg);
            break;
        case OPT_OUT_MODE:
            if (!(optArg == "0" || optArg == "1" || optArg == "2" || optArg == "3" || optArg == "4"))
            {
                BadUsage();
            }
            PDVITYPEPROG.outmode = std::stoi(optArg);
            break;
        case OPT_RESOLUTION:
        {
            int num, den;
#if defined(MIKTEX_WINDOWS)
            if (sscanf_s(optArg.c_str(), "%d/%d", &num, &den) != 2 || (num < 0))
            {
                BadUsage();
            }
#else
            if (sscanf(optArg.c_str(), "%d/%d", &num, &den) != 2 || (num < 0))
            {
                BadUsage();
            }
#endif
            PDVITYPEPROG.resolution = static_cast<float>(num) / den;
            break;
        }
        case OPT_START_THERE:
        {
            const char* lpsz = optArg.c_str();
            size_t k = 0;
            do
            {
                if (*lpsz == MIKTEXTEXT('*'))
                {
                    PDVITYPEPROG.startthere[k] = false;
                    ++lpsz;
                }
                else if (!(isdigit(*lpsz) || (*lpsz == MIKTEXTEXT('-') && isdigit(lpsz[1]))))
                {
                    BadUsage();
                }
                else
                {
                    PDVITYPEPROG.startthere[k] = true;
                    char* lpsz2 = 0;
                    PDVITYPEPROG.startcount[k] = strtol(lpsz, &lpsz2, 10);
                    lpsz = const_cast<const char*>(lpsz2);
                }
                if (k < 9 && *lpsz == '.')
                {
                    ++k;
                    ++lpsz;
                }
                else if (*lpsz == 0)
                {
                    PDVITYPEPROG.startvals = static_cast<C4P::C4P_signed8>(k);
                }
                else
                {
                    BadUsage();
                }
            } while (PDVITYPEPROG.startvals != static_cast<C4P::C4P_signed8>(k));
            break;
        }
        default:
            done = WebApp::ProcessOption(opt, optArg);
            break;
        }
        return done;
    }

    void ProcessCommandLineOptions() override
    {
        WebApp::ProcessCommandLineOptions();
        if (GetProgram()->GetArgC() != 2)
        {
            BadUsage();
        }
    }
};
