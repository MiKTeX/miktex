/**
 * @file miktex/TeXAndFriends/ETeXMemoryHandlerImpl.h
 * @author Christian Schenk
 * @brief MiKTeX e-TeX memory handler implementation
 *
 * @copyright Copyright © 2017-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/TeXAndFriends/config.h>

#include "TeXMemoryHandlerImpl.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class ETeXMemoryHandlerImpl :
    public TeXMemoryHandlerImpl<PROGRAM_CLASS>
{

public:

    ETeXMemoryHandlerImpl(PROGRAM_CLASS& program, TeXMFApp& texmfapp) :
        TeXMemoryHandlerImpl<PROGRAM_CLASS>(program, texmfapp)
    {
    }

    void Allocate(const std::unordered_map<std::string, int>& userParams) override
    {
        TeXMemoryHandlerImpl<PROGRAM_CLASS>::Allocate(userParams);
        this->AllocateArray("eofseen", this->program.eofseen, this->program.maxinopen);
        this->AllocateArray("grpstack", this->program.grpstack, this->program.maxinopen);
        this->AllocateArray("ifstack", this->program.ifstack, this->program.maxinopen);
    }

    void Free() override
    {
        TeXMemoryHandlerImpl<PROGRAM_CLASS>::Free();
        this->FreeArray("eofseen", this->program.eofseen);
        this->FreeArray("grpstack", this->program.grpstack);
        this->FreeArray("ifstack", this->program.ifstack);
    }

    void Check() override
    {
        TeXMemoryHandlerImpl<PROGRAM_CLASS>::Check();
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.eofseen);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.grpstack);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.ifstack);
    }
};

MIKTEX_TEXMF_END_NAMESPACE;
