/**
 * @file internal.h
 * @author Christian Schenk
 * @brief Internal definitions
 *
 * @copyright Copyright © 2005-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Application Framework.
 *
 * The MiKTeX Application Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#if !defined(UNUSED_ALWAYS)
#   define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#define BEGIN_INTERNAL_NAMESPACE                            \
    namespace MiKTeX {                                      \
        namespace App {                                     \
            namespace AA9DF7E0B05849A3B04FB16D33E23633 {

#define END_INTERNAL_NAMESPACE                              \
            }                                               \
        }                                                   \
    }

#include "AppResources.h"

BEGIN_INTERNAL_NAMESPACE;

template<class VALTYPE> class AutoRestore
{

public:

    AutoRestore(VALTYPE& val) :
        oldVal(val),
        pVal(&val)
    {
    }

    ~AutoRestore()
    {
        *pVal = oldVal;
    }

private:

    VALTYPE oldVal;
    VALTYPE* pVal;
};

END_INTERNAL_NAMESPACE;

using namespace MiKTeX::App::AA9DF7E0B05849A3B04FB16D33E23633;
