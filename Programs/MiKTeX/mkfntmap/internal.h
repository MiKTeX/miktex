/**
 * @file internal.h
 * @author Christian Schenk
 * @brief Internal definitions.
 * 
 * @copyright Copyright © 2002-2021 Christian Schenk
 * 
 * This file is part of MiKTeX Fontmap Maintenance Utility.
 *
 * MiKTeX Fontmap Maintenance Utility is licensed under GNU General Public
 * License version 2 or any later version.
 *
 * MiKTeX Fontmap Maintenance Utility is based on the updmap Perl script
 * (updmap.pl):
 * 
 * @code {.unparsed}
 * # Copyright 2011-2021 Norbert Preining
 * # This file is licensed under the GNU General Public License version 2
 * # or any later version.
 * @endcode
 */

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "mkfntmap-version.h"

#include <miktex/App/Application>

#include <miktex/Core/Cfg>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/StreamReader>
#include <miktex/Core/StreamWriter>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/ConsoleCodePageSwitcher>
#endif

#include <miktex/Util/Tokenizer>

#include <miktex/Wrappers/PoptWrapper>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/rollingfileappender.h>

#define UNUSED_ALWAYS(x)

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#define BOOLSTR(b) ((b) ? "true" : "false")
