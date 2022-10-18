/**
 * @file miktex/asy.cpp
 * @author Christian Schenk
 * @brief MiKTeX Asy
 *
 * @copyright Copyright © 2018-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include "asy-first.h"
#include "asy.h"

namespace gl
{
    void updateHandler(int);
    void exportHandler(int);
}

MIKTEX_BEGIN_NS;

std::atomic_bool exitRequested = false;
std::atomic_bool exportRequested = false;
std::atomic_bool* sigusr1 = nullptr;
std::atomic_bool updateRequested = false;

void RequestHandler()
{
    if (exitRequested)
    {
        throw 0;
    }
    if (updateRequested)
    {
        gl::updateHandler(0);
        updateRequested = false;
    }
    if (exportRequested)
    {
        gl::exportHandler(0);
        exportRequested = false;
    }
}

MIKTEX_END_NS;
