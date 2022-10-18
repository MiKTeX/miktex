/**
 * @file miktex/TeXAndFriends/MetafontApp.h
 * @author Christian Schenk
 * @brief MiKTeX METAFONT base implementation
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/TeXAndFriends/config.h>

#include <memory>
#include <string>

#include <miktex/Core/FileType>

#include "TeXMFApp.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

class MIKTEXMFTYPEAPI(MetafontApp) :
  public TeXMFApp
{

public:

    MIKTEXMFEXPORT MIKTEXTHISCALL MetafontApp();
    MetafontApp(const MetafontApp& other) = delete;
    MetafontApp& operator=(const MetafontApp& other) = delete;
    MetafontApp(MetafontApp&& other) = delete;
    MetafontApp& operator=(MetafontApp&& other) = delete;
    virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~MetafontApp() noexcept;

    MIKTEXMFTHISAPI(void) Finalize() override;

    MiKTeX::Core::FileType GetInputFileType() const override
    {
        return MiKTeX::Core::FileType::MF;
    }

    std::string GetMemoryDumpFileExtension() const override
    {
        return ".base";
    }

    MiKTeX::Core::FileType GetMemoryDumpFileType() const override
    {
        return MiKTeX::Core::FileType::BASE;
    }

protected:

    MIKTEXMFTHISAPI(bool) ProcessOption(int opt, const std::string& optArg) override;
    MIKTEXMFTHISAPI(void) AddOptions() override;
    MIKTEXMFTHISAPI(void) Init(std::vector<char*>& args) override;

private:

    class impl;
    std::unique_ptr<impl> pimpl;
};

MIKTEX_TEXMF_END_NAMESPACE;
