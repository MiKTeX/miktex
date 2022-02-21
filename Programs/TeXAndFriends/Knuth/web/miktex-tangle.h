/**
 * @file miktex-tangle.h
 * @author Christian Schenk
 * @brief MiKTeX Tangle class
 *
 * @copyright Copyright © 1991-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#if !defined(MIKTEX_INITTANGLE)
#define MIKTEX_TANGLE
#define TANGLEPROG g_TangleProg
#define TANGLEPROGCLASS TangleProgram
#define TANGLEAPP g_TANGLEApp
#define TANGLEAPPCLASS TANGLE
#endif

#include <miktex/TeXAndFriends/WebApp>

#define OPT_OMEGA 1000

extern TANGLEPROGCLASS TANGLEPROG;

class TANGLEAPPCLASS :
    public MiKTeX::TeXAndFriends::WebApp
{
public:
    void Init(std::vector<char*>& args) override
    {
        MiKTeX::TeXAndFriends::WebApp::Init(args);
        TANGLEPROG.maxchar = 255;
    }

public:
    void AddOptions() override
    {
        AddOption(MIKTEXTEXT("omega\0Create Omega compatible output file."), OPT_OMEGA);
        WebApp::AddOptions();
    }

public:
    std::string GetUsage() const override
    {
        return MIKTEXTEXT("[OPTION...] INPUTFILE CHANGEFILE OUTPUTFILE POOLFILE");
    }

public:
    bool ProcessOption(int opt, const std::string& optArg) override
    {
        bool done = true;
        switch (opt)
        {
        case OPT_OMEGA:
            TANGLEPROG.maxchar = 0xffff;
            break;
        default:
            done = WebApp::ProcessOption(opt, optArg);
            break;
        }
        return done;
    }

public:
    void ProcessCommandLineOptions() override
    {
        WebApp::ProcessCommandLineOptions();
        if (GetProgram()->GetArgC() != 5)
        {
            BadUsage();
        }
    }
};
