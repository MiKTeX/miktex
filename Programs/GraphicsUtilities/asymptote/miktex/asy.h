/**
 * @file miktex/asy.h
 * @author Christian Schenk
 * @brief MiKTeX Asy
 *
 * @copyright Copyright © 2018-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include "asy-first.h"

#include <atomic>

MIKTEX_BEGIN_NS;

extern std::atomic_bool exitRequested;
extern std::atomic_bool exportRequested;
extern std::atomic_bool* sigusr1;
extern std::atomic_bool updateRequested;

void RequestHandler();

MIKTEX_END_NS;
