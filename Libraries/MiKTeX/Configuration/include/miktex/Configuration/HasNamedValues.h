/**
 * @file miktex/Configuration/HasNamedValues.h
 * @author Christian Schenk
 * @brief HasNamedValues interface
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Configuration Library.
 *
 * The MiKTeX Configuration Library is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/Configuration/config.h>

#include <string>

MIKTEX_CONFIG_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE HasNamedValues
{

public:

    virtual bool MIKTEXTHISCALL TryGetValue(const std::string& valueName, std::string& value) = 0;
    virtual std::string MIKTEXTHISCALL GetValue(const std::string& valueName) = 0;
};

MIKTEX_CONFIG_END_NAMESPACE;
