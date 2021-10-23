/* miktex-uppltotf.h:

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

#include "miktex-uppltotf-config.h"

#include <iostream>

#include <miktex/TeXAndFriends/WebApp>

#include <miktex/uptex.h>

#define OPT_KANJI 1000

extern UPPLTOTFPROGCLASS UPPLTOTFPROG;

class UPPLTOTFAPPCLASS :
    public MiKTeX::TeXAndFriends::WebApp
{
public:
    void AddOptions() override
    {
        AddOption("kanji", MIKTEXTEXT("Set Japanese encoding (ENC=euc|jis|sjis|utf8)."), OPT_KANJI, POPT_ARG_STRING, "ENC");
        WebApp::AddOptions();
    }

public:
    std::string GetUsage() const override
    {
        return MIKTEXTEXT("[OPTION...] INPUTFILE OUTPUTFILE");
    }

public:
    void Init(std::vector<char*>& args) override
    {
        WebApp::Init(args);
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
        if (GetProgram()->GetArgC() != 2)
        {
            BadUsage();
        }
    }
};
