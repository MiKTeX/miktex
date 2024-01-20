/**
 * @file miktex/Util/Tokenizer.h
 * @author Christian Schenk
 * @brief Tokenizer class
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#include <miktex/Util/config.h>

#include <memory>
#include <string>

MIKTEX_UTIL_BEGIN_NAMESPACE;

class Tokenizer
{

public:

    Tokenizer() = delete;
    Tokenizer(const Tokenizer& other) = delete;
    Tokenizer& operator=(const Tokenizer& other) = delete;
    Tokenizer(Tokenizer&& other) = delete;
    Tokenizer& operator=(Tokenizer&& other) = delete;
    virtual MIKTEXUTILEXPORT MIKTEXTHISCALL ~Tokenizer() noexcept;

    MIKTEXUTILEXPORT MIKTEXTHISCALL Tokenizer(const std::string& s, const std::string& delims);
    MIKTEXUTILTHISAPI(void) SetDelimiters(const std::string& delims);
    explicit MIKTEXUTILEXPORT MIKTEXTHISCALL operator bool() const;
    MIKTEXUTILTHISAPI(std::string) operator*() const;
    MIKTEXUTILTHISAPI(Tokenizer&) operator++();

private:

    class impl;
    std::unique_ptr<impl> pimpl;
};

MIKTEX_UTIL_END_NAMESPACE;
