/**
 * @file miktex/tex4ht.h
 * @author Christian Schenk
 * @brief MiKTeX specialties
 *
 * @copyright Copyright © 2020-2023 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include <miktex/Core/Process>

int miktex_system(const char* commandLine)
{
    if (commandLine == nullptr)
    {
        return 1;
    }
    try
    {
        int exitCode;
        if (MiKTeX::Core::Process::ExecuteSystemCommand(commandLine, &exitCode))
        {
            return exitCode;
        }
        else
        {
            return -1;
        }
    }
    catch (const MiKTeX::Core::MiKTeXException &)
    {
        return -1;
    }
}
