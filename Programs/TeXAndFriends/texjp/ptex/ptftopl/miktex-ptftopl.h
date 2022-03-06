/**
 * @file miktex-ptftopl.h
 * @author Christian Schenk
 * @brief MiKTeX pTFtoPL
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include "miktex-ptftopl-config.h"

#include <iostream>

#include <miktex/TeXAndFriends/WebApp>

#include <miktex/TeXjp/common.h>

#define OPT_KANJI 1000

extern PTFTOPLPROGCLASS PTFTOPLPROG;

class PTFTOPLAPPCLASS :
    public MiKTeX::TeXAndFriends::WebApp
{

public:

    void AddOptions() override
    {
        AddOption("kanji", MIKTEXTEXT("Set Japanese encoding (ENC=euc|jis|sjis|utf8)."), OPT_KANJI, POPT_ARG_STRING, "ENC");
        WebApp::AddOptions();
    }

    std::string GetUsage() const override
    {
        return MIKTEXTEXT("[OPTION...] INPUTFILE OUTPUTFILE");
    }

    void Init(std::vector<char*>& args) override
    {
        WebApp::Init(args);
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
