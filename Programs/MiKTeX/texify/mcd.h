/**
 * @file mcd.h
 * @author Christian Schenk
 * @brief Main program
 *
 * @copyright Copyright © 1998-2024 Christian Schenk
 *
 * This file is part of MiKTeX Compiler Driver.
 *
 * MiKTeX Compiler Driver is licensed under GNU General Public License version 2
 * or any later version.
 */

#include <memory>
#include <string>
#include <vector>

#include <cstring>

#include <miktex/App/Application>
#include <miktex/Core/Quoter>
#include <miktex/Core/Process>
#include <miktex/Util/PathName>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <regex.h>

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

#define ALWAYS_UNUSED(x)

#define SUPPORT_OPT_SRC_SPECIALS

/* Patch (#464378) by Thomas Schimming (http://sf.net/users/schimmin/):

   The BibTeX handler is extended to work in a chapterbib compliant mode, that
   is, bibtex is run on all 1st level AUX files referenced in the main AUX file.
   After the special treatment (if use of the chapterbib package is detected),
   the regular bibtex treatment occurs (as before).

   The code uses some heuristics to determine when it's necessary to run this
   extension.  However, this can still be improved as for example changes in
   bibstyle do not correctly trigger a re-bibtex yet.

   Ultimately, the behavior should probably be included into bibtex, however in
   this way, an "orthodox" latex distribution is maintained... */

#define SF464378__CHAPTERBIB

#define PROGRAM_NAME "texify"

#if !defined(THE_NAME_OF_THE_GAME)
#define THE_NAME_OF_THE_GAME T_("MiKTeX Compiler Driver")
#endif

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

inline bool IsPrefixOf(const char* prefix, const std::string& str)
{
    return str.compare(0, strlen(prefix), prefix) == 0;
}

class ProcessOutputTrash : public MiKTeX::Core::IRunProcessCallback
{
public:
    bool MIKTEXTHISCALL OnProcessOutput(const void* chunk, size_t n) override
    {
        ALWAYS_UNUSED(chunk);
        ALWAYS_UNUSED(n);
        return true;
    }
};

class ProcessOutputSaver : public MiKTeX::Core::IRunProcessCallback
{

public:

    bool MIKTEXTHISCALL OnProcessOutput(const void* chunk, size_t n) override
    {
        output.append(reinterpret_cast<const char*>(chunk), n);
        return true;
    }

    const std::string& GetOutput() const
    {
        return output;
    }

private:

    std::string output;
};

enum class MacroLanguage
{
    None,
    LaTeX,
#if defined(WITH_TEXINFO)
    Texinfo
#endif
};

enum class Engine
{
    NotSet,
    TeX,
    pdfTeX,
    pTeX,
    epTeX,
    eupTeX,
    upTeX,
    XeTeX,
    LuaTeX,
    LuaHBTeX
};

enum class OutputType
{
    None,
    DVI,
    PDF,
    PS
};

enum class SyncTeXOption
{
    Uncompressed = -1,
    Disabled = 0,
    Compressed = 1
};

class Options
{

public:

    Options();
    virtual ~Options();
    MiKTeX::Util::PathName startDirectory;
    regex_t regex_bibdata;
    regex_t regex_bibstyle;
    regex_t regex_citation_undefined;
    regex_t regex_no_file_bbl;
    bool batch = false;
    bool clean = false;
    bool expand = false;
    bool quiet = false;
    bool verbose = false;
    OutputType outputType = OutputType::DVI;
    bool runViewer = false;
    SyncTeXOption synctex = SyncTeXOption::Disabled;
    int maxIterations = 5;
    std::vector<std::string> includeDirectories;
    std::string jobName;
    MacroLanguage macroLanguage = MacroLanguage::None;
    std::string sourceSpecialsWhere;
    std::string bibtexProgram;
    std::string latexProgram;
    std::string pdflatexProgram;
    std::string xelatexProgram;
    std::string lualatexProgram;
    std::string luahblatexProgram;
    std::string makeindexProgram;
    std::string makeinfoProgram;
    std::string texProgram;
    std::string pdftexProgram;
    std::string ptexProgram;
    std::string eptexProgram;
    std::string euptexProgram;
    std::string uptexProgram;
    std::string xetexProgram;
    std::string luatexProgram;
    std::string luahbtexProgram;
    std::string texindexProgram;
    std::vector<std::string> makeindexOptions;
    std::vector<std::string> texOptions;
    std::vector<std::string> viewerOptions;
    std::string traceStreams;
    Engine engine = Engine::NotSet;

#if defined(WITH_TEXINFO)
    std::vector<std::string> texinfoCommands;
    regex_t regex_texinfo_version;
#endif

#ifdef SF464378__CHAPTERBIB
    regex_t regex_chapterbib;
    regex_t regex_input_aux;
#endif

#if defined(SUPPORT_OPT_SRC_SPECIALS)
    bool sourceSpecials = false;
#endif

    std::string SetProgramName(const std::string& envName, const std::string& defaultProgram)
    {
        std::string programName;
        if (!MiKTeX::Core::Utils::GetEnvironmentString(envName, programName))
        {
            programName = defaultProgram;
        }
        return programName;
    }

    bool SetEngine(const std::string& engineName)
    {
        if (MiKTeX::Core::Utils::EqualsIgnoreCase(engineName, "tex"))
        {
            engine = Engine::TeX;
        }
        else if (MiKTeX::Core::Utils::EqualsIgnoreCase(engineName, "pdftex"))
        {
            engine = Engine::pdfTeX;
        }
        else if (MiKTeX::Core::Utils::EqualsIgnoreCase(engineName, "ptex"))
        {
            engine = Engine::pTeX;
        }
        else if (MiKTeX::Core::Utils::EqualsIgnoreCase(engineName, "eptex"))
        {
            engine = Engine::epTeX;
        }
        else if (MiKTeX::Core::Utils::EqualsIgnoreCase(engineName, "euptex"))
        {
            engine = Engine::eupTeX;
        }
        else if (MiKTeX::Core::Utils::EqualsIgnoreCase(engineName, "uptex"))
        {
            engine = Engine::upTeX;
        }
        else if (MiKTeX::Core::Utils::EqualsIgnoreCase(engineName, "xetex"))
        {
            engine = Engine::XeTeX;
        }
        else if (MiKTeX::Core::Utils::EqualsIgnoreCase(engineName, "luatex"))
        {
            engine = Engine::LuaTeX;
        }
        else if (MiKTeX::Core::Utils::EqualsIgnoreCase(engineName, "luahbtex"))
        {
            engine = Engine::LuaHBTeX;
        }
        else
        {
            return false;
        }
        return true;
    }
};

class McdApp : public MiKTeX::App::Application
{

public:

    McdApp() : traceStream(MiKTeX::Trace::TraceStream::Open(PROGRAM_NAME))
    {
        forbiddenTexOptions.push_back("aux-directory");
        forbiddenTexOptions.push_back("job-name");
        forbiddenTexOptions.push_back("jobname");
        forbiddenTexOptions.push_back("output-directory");
    };

    void Run(int argc, const char** argv);
    void MyTrace(const std::string& s);
    void Verbose(const std::string& s);
    void Version();

    Options options;

private:

    std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;
    std::vector<std::string> forbiddenTexOptions;
};

class Driver
{

public:

    virtual ~Driver();
    void Initialize(McdApp* app, Options* options, const char* fileName);
    void Run();

protected:
    std::shared_ptr<MiKTeX::Core::Session> session = MIKTEX_SESSION();

private:

    void FatalUtilityError(const std::string& name)
    {
        app->FatalError(fmt::format("A required utility could not be found. Utility name: {}.", name));
    }

    MacroLanguage GuessMacroLanguage(const MiKTeX::Util::PathName& fileName);
    void SetIncludeDirectories();
    void ExpandMacros();
    void InsertCommands();
    bool RunMakeinfo(const MiKTeX::Util::PathName& pathFrom, const MiKTeX::Util::PathName& pathTo);
    void RunBibTeX();
    MiKTeX::Util::PathName GetTeXEnginePath(std::string& exeName);
    void RunTeX();
    void RunIndexGenerator(const std::vector<std::string>& idxFiles);
    void RunViewer();
    bool Ready();
    void InstallOutputFile();
    void GetAuxFiles(std::vector<std::string>& auxFiles, std::vector<std::string>* idxFiles = nullptr);
    void GetAuxFiles(const MiKTeX::Util::PathName& baseName, const char* extension, std::vector<std::string>& auxFiles);
    void InstallProgram(const char* program);

    MacroLanguage macroLanguage = MacroLanguage::None;
    MiKTeX::Util::PathName givenFileName;
    MiKTeX::Util::PathName originalInputFile;
    std::unique_ptr<MiKTeX::Core::TemporaryDirectory> tempDirectory;
    MiKTeX::Util::PathName originalInputDirectory;
    MiKTeX::Util::PathName inputName;
    MiKTeX::Util::PathName jobName;
    MiKTeX::Util::PathName workingDirectory;
    MiKTeX::Util::PathName auxDirectory;
    MiKTeX::Util::PathName pathInputFile;
    std::vector<std::string> previousAuxFiles;
    McdApp* app = nullptr;
    Options* options = nullptr;
#if defined(WITH_TEXINFO)
    void TexinfoPreprocess(const MiKTeX::Util::PathName& pathFrom, const MiKTeX::Util::PathName& pathTo);
    void TexinfoUncomment(const MiKTeX::Util::PathName& pathFrom, const MiKTeX::Util::PathName& pathTo);
    bool Check_texinfo_tex();
    MiKTeX::Util::PathName extraDirectory;
#endif
};

