/**
 * @file texinfo.cpp
 * @author Christian Schenk
 * @brief Texinfo specialties
 *
 * @copyright Copyright © 1998-2024 Christian Schenk
 *
 * This file is part of MiKTeX Compiler Driver.
 *
 * MiKTeX Compiler Driver is licensed under GNU General Public License version 2
 * or any later version.
 */

#if defined(WITH_TEXINFO)

#include <miktex/Core/StreamReader>
#include <miktex/Core/StreamWriter>
#include <miktex/Core/TemporaryDirectory>
#include <miktex/Util/PathName>

#include "mcd.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

/* _________________________________________________________________________

   Driver::TexinfoPreprocess

   A function that preprocesses Texinfo sources in order to keep the iftex
   sections only.  We want to remove non TeX sections, and comment (with `@c
   texi2dvi') TeX sections so that makeinfo does not try to parse them.
   Nevertheless, while commenting TeX sections, don't comment @macro/@end macro
   so that makeinfo does propagate them.  Unfortunately makeinfo --iftex
   --no-ifhtml --no-ifinfo doesn't work well enough (yet) to use that, so work
   around with sed.
   _________________________________________________________________________ */

void Driver::TexinfoPreprocess(const PathName& pathFrom, const PathName& pathTo)
{
    StreamReader reader(pathFrom);
    StreamWriter writer(pathTo);
    bool at_tex = false;
    bool at_iftex = false;
    bool at_macro = false;
    bool at_html = false;
    bool at_ifhtml = false;
    bool at_ifnottex = false;
    bool at_ifinfo = false;
    bool at_menu = false;
    string line;
    while (reader.ReadLine(line))
    {
        if (IsPrefixOf("@tex", line))
        {
            at_tex = true;
        }
        else if (IsPrefixOf("@iftex", line))
        {
            at_iftex = true;
        }
        else if (IsPrefixOf("@macro", line))
        {
            at_macro = true;
        }
        else if (IsPrefixOf("@html", line))
        {
            at_html = true;
        }
        else if (IsPrefixOf("@ifnottex", line))
        {
            at_ifnottex = true;
        }
        else if (IsPrefixOf("@ifinfo", line))
        {
            at_ifinfo = true;
        }
        else if (IsPrefixOf("@menu", line))
        {
            at_menu = true;
        }

        bool commentingOut = (at_tex || (at_iftex && !at_macro));
        bool deleting = (at_html || at_ifhtml || at_ifnottex || (at_ifinfo && !at_menu && !IsPrefixOf("@node", line)));

        if (commentingOut)
        {
            writer.WriteLine(string("@c texi2dvi") + line);
        }
        else if (!deleting)
        {
            writer.WriteLine(line);
        }

        if (IsPrefixOf("@end tex", line))
        {
            at_tex = false;
        }
        else if (IsPrefixOf("@end iftex", line))
        {
            at_iftex = false;
        }
        else if (IsPrefixOf("@end macro", line))
        {
            at_macro = false;
        }
        else if (IsPrefixOf("@end html", line))
        {
            at_html = false;
        }
        else if (IsPrefixOf("@end ifnottex", line))
        {
            at_ifnottex = false;
        }
        else if (IsPrefixOf("@end ifinfo", line))
        {
            at_ifinfo = false;
        }
        else if (IsPrefixOf("@end menu", line))
        {
            at_menu = false;
        }
    }
}

/* _________________________________________________________________________

   Driver::TexinfoUncomment

   Uncommenting is simple: Remove any leading `@c texi2dvi'.
   _________________________________________________________________________ */

void Driver::TexinfoUncomment(const PathName& pathFrom, const PathName& pathTo)
{
    StreamReader reader(pathFrom);
    StreamWriter writer(pathTo);
    string line;
    while (reader.ReadLine(line))
    {
        if (IsPrefixOf("@c texi2dvi", line))
        {
            writer.WriteLine(line.substr(11));
        }
        else
        {
            writer.WriteLine(line);
        }
    }
    writer.Close();
    reader.Close();
}

/* _________________________________________________________________________

   Driver::Check_texinfo_tex

   Check if texinfo.tex performs macro expansion by looking for its version.
   The version is a date of the form YEAR-MO-DA.
   _________________________________________________________________________ */

   // minimum texinfo.tex version to have macro expansion
const string txiprereq = "19990129";

bool Driver::Check_texinfo_tex()
{
    PathName pathExe;

    if (!session->FindFile(options->texProgram, FileType::EXE, pathExe))
    {
        FatalUtilityError(options->texProgram);
    }

    bool newer = false;

    unique_ptr<TemporaryDirectory> tmpdir = TemporaryDirectory::Create();

    PathName fileName = tmpdir->GetPathName() / "txiversion.tex";
    StreamWriter writer(fileName);
    writer.WriteLine("\\input texinfo.tex @bye");
    writer.Close();

    int exitCode = 0;
    ProcessOutputSaver processOutput;
    if (!Process::Run(pathExe, vector<string>{options->texProgram, fileName.ToString()}, & processOutput, & exitCode, tmpdir->GetPathName().GetData()))
    {
        MIKTEX_UNEXPECTED();
    }

    if (exitCode == 0)
    {
        regmatch_t regMatch[5];
        if (regexec(&options->regex_texinfo_version, processOutput.GetOutput().c_str(), 5, regMatch, 0) == 0)
        {
            string txiformat;
            for (int i = regMatch[1].rm_so; i < regMatch[1].rm_eo; ++i)
            {
                txiformat += processOutput.GetOutput()[i];
            }
            string version;
            version += processOutput.GetOutput()[regMatch[2].rm_so + 0];
            version += processOutput.GetOutput()[regMatch[2].rm_so + 1];
            version += processOutput.GetOutput()[regMatch[2].rm_so + 2];
            version += processOutput.GetOutput()[regMatch[2].rm_so + 3];
            version += processOutput.GetOutput()[regMatch[3].rm_so + 0];
            version += processOutput.GetOutput()[regMatch[3].rm_so + 1];
            version += processOutput.GetOutput()[regMatch[4].rm_so + 0];
            version += processOutput.GetOutput()[regMatch[4].rm_so + 1];
            app->Verbose(fmt::format(T_("texinfo.tex preloaded as {}, version is {}..."), txiformat, version));
            newer = std::stoi(txiprereq) <= std::stoi(version);
        }
    }

    return !newer;
}

#endif