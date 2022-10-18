/**
 * @file internal.cpp
 * @author Christian Schenk
 * @brief Internal definitions
 *
 * @copyright Copyright © 2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/StreamReader>
#include <miktex/Util/PathName>
#include <miktex/Util/PathNameUtil>
#include <miktex/Util/Tokenizer>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;

string OneMiKTeXUtility::Unescape(const string& s)
{
    stringstream out;
    bool wasEsc = false;
    for (auto ch : s)
    {
        if (wasEsc)
        {
            wasEsc = false;
            switch (ch)
            {
            case '\\':
                out << "\\";
                break;
            case 'n':
                out << "\n";
                break;
            default:
                out << "\\" << ch;
                break;
            }
        }
        else if (ch == '\\')
        {
            wasEsc = true;
        }
        else
        {
            out << ch;
        }
    }
    return out.str();
}

void OneMiKTeXUtility::ReadNames(const PathName& path, vector<string>& list)
{
    StreamReader reader(path);
    string line;
    while (reader.ReadLine(line))
    {
        Tokenizer tok(line, " \t\n\r");
        if (tok)
        {
            string name = *tok;
            list.push_back(name);
        }
    }
    reader.Close();
}
