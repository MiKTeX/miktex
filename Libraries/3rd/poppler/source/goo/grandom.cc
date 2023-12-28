/*
 * grandom.cc
 *
 * This file is licensed under the GPLv2 or later
 *
 * Pseudo-random number generation
 *
 * Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
 * Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
 * Copyright (C) 2022 Albert Astals Cid <aacid@kde.org>
 */

#include "grandom.h"

#include <random>

namespace {

auto &grandom_engine()
{
    static thread_local std::default_random_engine engine { std::random_device {}() };
    return engine;
}

}

void grandom_fill(unsigned char *buff, int size)
{
    auto &engine = grandom_engine();
    std::uniform_int_distribution<unsigned short> distribution { std::numeric_limits<unsigned char>::min(), std::numeric_limits<unsigned char>::max() };
    for (int index = 0; index < size; ++index) {
        buff[index] = static_cast<unsigned char>(distribution(engine));
    }
}

double grandom_double()
{
    auto &engine = grandom_engine();
    return std::generate_canonical<double, std::numeric_limits<double>::digits>(engine);
}
