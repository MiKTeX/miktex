/**
 * @file miktex/TeXAndFriends/ETeXApp.h
 * @author Christian Schenk
 * @brief MiKTeX e-TeX base implementation
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

#include "TeXApp.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

class MIKTEXMFTYPEAPI(ETeXApp) :
    public TeXApp
{

public:

    MIKTEXMFEXPORT MIKTEXTHISCALL ETeXApp();
    ETeXApp(const ETeXApp& other) = delete;
    ETeXApp& operator=(const ETeXApp& other) = delete;
    ETeXApp(ETeXApp&& other) = delete;
    ETeXApp& operator=(ETeXApp&& other) = delete;
    virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~ETeXApp() noexcept;

    MIKTEXMFTHISAPI(bool) ETeXP() const;
    MIKTEXMFTHISAPI(void) Finalize() override;
    MIKTEXMFTHISAPI(void) OnTeXMFStartJob() override;

    static ETeXApp* GetETeXApp()
    {
        MIKTEX_ASSERT(dynamic_cast<ETeXApp*>(Application::GetApplication()) != nullptr);
        return (ETeXApp*)Application::GetApplication();
    }

protected:

    MIKTEXMFTHISAPI(bool) ProcessOption(int c, const std::string& optArg) override;
    MIKTEXMFTHISAPI(void) AddOptions() override;
    MIKTEXMFTHISAPI(void) Init(std::vector<char*>& args) override;

private:

    class impl;
    std::unique_ptr<impl> pimpl;
};

inline bool miktexetexp()
{
    return ETeXApp::GetETeXApp()->ETeXP();
}

MIKTEX_TEXMF_END_NAMESPACE;
