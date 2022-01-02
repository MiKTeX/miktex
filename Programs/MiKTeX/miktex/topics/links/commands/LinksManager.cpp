/**
 * @file topic/links/commands/LinksManager.h
 * @author Christian Schenk
 * @brief Links manager
 *
 * @copyright Copyright © 2002-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <config.h>

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Configuration/ConfigurationProvider>
#include <miktex/Core/AutoResource>
#include <miktex/Core/Cfg>
#include <miktex/Core/Directory>
#include <miktex/Core/Paths>
#include <miktex/Core/Utils>
#include <miktex/Util/PathName>

#include "internal.h"

#include "LinksManager.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

using namespace OneMiKTeXUtility;

void LinksManager::Init(ApplicationContext& ctx)
{
    this->ctx = &ctx;
}

void LinksManager::Uninstall()
{
    PathName linkTargetDirectory = this->ctx->session->GetSpecialPath(SpecialPath::LinkTargetDirectory);
    this->ctx->ui->Verbose(1, fmt::format(T_("Removing links from target directory {0}..."), Q_(linkTargetDirectory.ToDisplayString())));
    LinkCategoryOptions linkCategories;
    linkCategories.Set();
    this->ManageLinks(linkCategories, true, true);
}

void LinksManager::Install(bool force)
{
    PathName linkTargetDirectory = this->ctx->session->GetSpecialPath(SpecialPath::LinkTargetDirectory);
    this->ctx->ui->Verbose(1, fmt::format(T_("Installing links in target directory {0}..."), Q_(linkTargetDirectory.ToDisplayString())));
    LinkCategoryOptions linkCategories;
    linkCategories.Set();
    this->ManageLinks(linkCategories, false, force);
#if defined(MIKTEX_UNIX)
    this->MakeFilesExecutable();
#endif
}

void LinksManager::ManageLinks(LinkCategoryOptions linkCategories, bool uninstall, bool force)
{
  PathName pathBinDir = this->ctx->session->GetSpecialPath(SpecialPath::BinDirectory);
  PathName internalBinDir = this->ctx->session->GetSpecialPath(SpecialPath::InternalBinDirectory);

  // TODO: MIKTEX_ASSERT(pathBinDir.GetMountPoint() == internalBinDir.GetMountPoint());

  bool supportsHardLinks = Utils::SupportsHardLinks(pathBinDir);

  if (!uninstall && !Directory::Exists(pathBinDir))
  {
    Directory::Create(pathBinDir);
  }

  for (const FileLink& fileLink : CollectLinks(linkCategories))
  {
    ManageLink(fileLink, supportsHardLinks, uninstall, force);
  }
}

#if defined(MIKTEX_UNIX)
void LinksManager::MakeFilesExecutable()
{
    PathName scriptsIni;
    if (!this->ctx->session->FindFile(MIKTEX_PATH_SCRIPTS_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptsIni))
    {
        this->ctx->ui->FatalError(fmt::format(T_("{0}: script configuration file not found"), MIKTEX_PATH_SCRIPTS_INI));
    }
    unique_ptr<Cfg> config(Cfg::Create());
    config->Read(scriptsIni, true);
    this->ctx->installer->EnableInstaller(false);
    MIKTEX_AUTO(this->ctx->installer->EnableInstaller(true));
    for (const shared_ptr<Cfg::Key>& key : *config)
    {
        if (key->GetName() != "sh" && key->GetName() != "exe")
        {
            continue;
        }
        for (const shared_ptr<Cfg::Value>& val : *key)
        {
            if (EndsWith(val->GetName(), "[]"))
            {
                continue;
            }
            PathName scriptPath;
            if (!this->ctx->session->FindFile(this->ctx->session->Expand(val->AsString()), MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptPath))
            {
                continue;
            }
            if (this->ctx->session->GetRootDirectories()[this->ctx->session->DeriveTEXMFRoot(scriptPath)].IsCommon() && !this->ctx->session->IsAdminMode())
            {
                continue;
            }
            File::SetAttributes(scriptPath, File::GetAttributes(scriptPath) + FileAttribute::Executable);
        }
    }
}
#endif

void LinksManager::ManageLink(const FileLink& fileLink, bool supportsHardLinks, bool isRemoveRequested, bool allowOverwrite)
{
    LinkType linkType = fileLink.linkType;
    if (linkType == LinkType::Hard && !supportsHardLinks)
    {
        linkType = LinkType::Copy;
    }
    for (const string& linkName : fileLink.linkNames)
    {
        FileExistsOptionSet fileExistsOptions;
#if defined(MIKTEX_UNIX)
        fileExistsOptions += FileExistsOption::SymbolicLink;
#endif
        if (File::Exists(PathName(linkName), fileExistsOptions))
        {
            if (!isRemoveRequested && (!allowOverwrite || (linkType == LinkType::Copy && File::Equals(PathName(fileLink.target), PathName(linkName)))))
            {
                continue;
            }
#if defined(MIKTEX_UNIX)
            if (File::IsSymbolicLink(PathName(linkName)))
            {
                PathName linkTarget = File::ReadSymbolicLink(PathName(linkName));
                string linkTargetFileName = linkTarget.GetFileName().ToString();
                bool isMiKTeXSymlinked = linkTargetFileName.find(MIKTEX_PREFIX) == 0 || PathName(linkTargetFileName) == PathName(fileLink.target).GetFileName();
                if (!isMiKTeXSymlinked)
                {
                    if (File::Exists(PathName(linkTarget)))
                    {
                        this->ctx->logger->LogWarn(fmt::format("{0} already symlinked to {1}", Q_(linkName), Q_(linkTarget)));
                        continue;
                    }
                    else
                    {
                        this->ctx->logger->LogTrace("{0} is symlinked to non-existing {1}", Q_(linkName), Q_(linkTarget));
                    }
                }
            }
#endif
            this->ctx->ui->Verbose(2, fmt::format(T_("Removing {0}..."), PathName(linkName).ToDisplayString()));
            File::Delete(PathName(linkName), { FileDeleteOption::TryHard, FileDeleteOption::UpdateFndb });
        }
        if (isRemoveRequested)
        {
            continue;
        }
        PathName sourceDirectory(linkName);
        sourceDirectory.RemoveFileSpec();
        if (!Directory::Exists(sourceDirectory))
        {
            Directory::Create(sourceDirectory);
        }
        switch (linkType)
        {
        case LinkType::Symbolic:
        {
            const char* target = Utils::GetRelativizedPath(fileLink.target.c_str(), sourceDirectory.GetData());
            if (target == nullptr)
            {
                target = fileLink.target.c_str();
            }
            this->ctx->ui->Verbose(2, fmt::format(T_("Creating symbolic link: {0} -> {1}..."), Q_(PathName(linkName).ToDisplayString()), Q_(PathName(fileLink.target).ToDisplayString())));
            File::CreateLink(PathName(target), PathName(linkName), { CreateLinkOption::UpdateFndb, CreateLinkOption::Symbolic });
            break;
        }
        case LinkType::Hard:
            this->ctx->ui->Verbose(2, fmt::format(T_("Creating hard link: {0} -> {1}..."), Q_(PathName(linkName).ToDisplayString()), Q_(PathName(fileLink.target).ToDisplayString())));
            File::CreateLink(PathName(fileLink.target), PathName(linkName), { CreateLinkOption::UpdateFndb });
            break;
        case LinkType::Copy:
            this->ctx->ui->Verbose(2, fmt::format(T_("Copying: {0} -> {1}..."), Q_(PathName(linkName).ToDisplayString()), Q_(PathName(fileLink.target).ToDisplayString())));
            File::Copy(PathName(fileLink.target), PathName(linkName), { FileCopyOption::UpdateFndb });
            break;
        default:
            MIKTEX_UNEXPECTED();
        }
    }
}

vector<FileLink> LinksManager::CollectLinks(LinkCategoryOptions linkCategories)
{
  vector<FileLink> result;
  PathName linkTargetDirectory = this->ctx->session->GetSpecialPath(SpecialPath::LinkTargetDirectory);
  PathName pathBinDir = this->ctx->session->GetSpecialPath(SpecialPath::BinDirectory);

  if (linkCategories[LinkCategory::MiKTeX])
  {
    vector<FileLink> links = miktexFileLinks;
    links.insert(links.end(), lua52texLinks.begin(), lua52texLinks.end());
#if defined(MIKTEX_MACOS_BUNDLE)
    PathName console(session->GetSpecialPath(SpecialPath::MacOsDirectory) / PathName(MIKTEX_MACOS_BUNDLE_NAME));
    links.push_back(FileLink(console.ToString(), { MIKTEX_CONSOLE_EXE }, LinkType::Symbolic));
#endif
    for (const FileLink& fileLink : links)
    {
      PathName targetPath;
      if (PathNameUtil::IsAbsolutePath(fileLink.target))
      {
        targetPath = fileLink.target;
      }
      else
      {
        targetPath = pathBinDir / PathName(fileLink.target);
      }
      string extension = targetPath.GetExtension();
      if (File::Exists(targetPath))
      {
        vector<string> linkNames;
        for (const string& linkName : fileLink.linkNames)
        {
          PathName linkPath = linkTargetDirectory / PathName(linkName);
          if (linkPath == targetPath)
          {
            continue;
          }
          if (!extension.empty())
          {
            linkPath.AppendExtension(extension);
          }
          linkNames.push_back(linkPath.ToString());
        }
        result.push_back(FileLink(targetPath.ToString(), linkNames, fileLink.linkType));
      }
      else
      {
        this->ctx->ui->Warning(fmt::format(T_("{0}: the link target does not exist"), Q_(targetPath)));
      }
    }
  }

  if (linkCategories[LinkCategory::Formats])
  {
    for (const FormatInfo& formatInfo : this->ctx->session->GetFormats())
    {
      if (formatInfo.noExecutable)
      {
        continue;
      }
      string engine = formatInfo.compiler;
      PathName enginePath;
      if (!this->ctx->session->FindFile(string(MIKTEX_PREFIX) + engine, FileType::EXE, enginePath))
      {
        this->ctx->ui->Warning(fmt::format(T_("{0}: engine '{1}' not found"), formatInfo.key, engine));
        continue;
      }
      PathName exePath(linkTargetDirectory, PathName(formatInfo.name));
      if (strlen(MIKTEX_EXE_FILE_SUFFIX) > 0)
      {
        exePath.AppendExtension(MIKTEX_EXE_FILE_SUFFIX);
      }
      if (!(enginePath == exePath))
      {
        result.push_back(FileLink(enginePath.ToString(), { exePath.ToString() }));
      }
    }
  }

  if (linkCategories[LinkCategory::Scripts])
  {
    PathName scriptsIni;
    if (!this->ctx->session->FindFile(MIKTEX_PATH_SCRIPTS_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptsIni))
    {
      this->ctx->ui->FatalError(fmt::format(T_("{0}: script configuration file not found"), MIKTEX_PATH_SCRIPTS_INI));
    }
    unique_ptr<Cfg> config(Cfg::Create());
    config->Read(scriptsIni, true);
    for (const shared_ptr<Cfg::Key>& key : *config)
    {
      PathName wrapper = this->ctx->session->GetSpecialPath(SpecialPath::InternalBinDirectory);
      wrapper.AppendDirectoryDelimiter();
      wrapper.Append("run", false);
      wrapper.Append(key->GetName(), false);
      wrapper.Append(MIKTEX_EXE_FILE_SUFFIX, false);
      if (!File::Exists(wrapper))
      {
        continue;
      }
      for (const shared_ptr<Cfg::Value>& v : *key)
      {
        string name = v->GetName();
        if (EndsWith(name, "[]"))
        {
          continue;
        }
        PathName pathExe(linkTargetDirectory, PathName(name));
        if (strlen(MIKTEX_EXE_FILE_SUFFIX) > 0)
        {
          pathExe.AppendExtension(MIKTEX_EXE_FILE_SUFFIX);
        }
        result.push_back(FileLink(wrapper.ToString(), { pathExe.ToString() }));
      }
    }
  }

  return result;
}

const vector<FileLink> LinksManager::miktexFileLinks =
{
#if defined(MIKTEX_WINDOWS)
  { "arctrl" MIKTEX_EXE_FILE_SUFFIX, { "pdfclose", "pdfdde", "pdfopen" } },
#endif
  { "cjklatex" MIKTEX_EXE_FILE_SUFFIX, { "bg5pluslatex", "bg5pluspdflatex", "bg5latex", "bg5pdflatex", "bg5platex", "bg5ppdflatex", "cef5latex", "cef5pdflatex", "ceflatex", "cefpdflatex", "cefslatex", "cefspdflatex", "gbklatex", "gbkpdflatex", "sjislatex", "sjispdflatex" } },

  { MIKTEX_AFM2TFM_EXE, { "afm2tfm" } },
#if defined(MIKTEX_WINDOWS)
  { MIKTEX_ASY_EXE, { "asy" } },
#endif
  { MIKTEX_AUTOSP_EXE, { "autosp" } },
  { MIKTEX_AXOHELP_EXE,{ "axohelp" } },
  { MIKTEX_BG5CONV_EXE, { "bg5conv" } },
  { MIKTEX_BIBSORT_EXE, { "bibsort" } },
  { MIKTEX_BIBTEX8_EXE, { "bibtex8" } },
  { MIKTEX_BIBTEXU_EXE, { "bibtexu" } },
  { MIKTEX_BIBTEX_EXE, { "bibtex" } },
  { MIKTEX_CEF5CONV_EXE, { "cef5conv" } },
  { MIKTEX_CEFCONV_EXE, { "cefconv" } },
  { MIKTEX_CEFSCONV_EXE, { "cefsconv" } },
  { MIKTEX_CHKTEX_EXE, { "chktex" } },
  { MIKTEX_CTANGLE_EXE,{ "ctangle" } },
  { MIKTEX_CWEAVE_EXE,{ "cweave" } },
  { MIKTEX_DEVNAG_EXE, { "devnag" } },
  { MIKTEX_DVICOPY_EXE, { "dvicopy" } },
  { MIKTEX_DVIPDFMX_EXE, { "dvipdfm", "dvipdfmx", "ebb", "extractbb", "xbb", "xdvipdfmx", MIKTEX_XDVIPDFMX_EXE } },
  { MIKTEX_DVIPDFT_EXE,{ "dvipdft" } },
  { MIKTEX_DVIPNG_EXE, { "dvipng" } },
  { MIKTEX_DVIPS_EXE, { "dvips" } },
  { MIKTEX_DVISVGM_EXE, { "dvisvgm" } },
  { MIKTEX_DVITYPE_EXE, { "dvitype" } },
  { MIKTEX_EPSFFIT_EXE, { "epsffit" } },
  { MIKTEX_EPSTOPDF_EXE,{ "epstopdf", "repstopdf", MIKTEX_PREFIX "epstopdf-unrestricted" } },
  { MIKTEX_EPTEX_EXE, { "eptex" } },
  { MIKTEX_EUPTEX_EXE, { "euptex" } },
  { MIKTEX_EXTCONV_EXE, { "extconv" } },
  { MIKTEX_FRIBIDIXETEX_EXE, { "fribidixetex" } },
  { MIKTEX_GFTODVI_EXE, { "gftodvi" } },
  { MIKTEX_GFTOPK_EXE, { "gftopk" } },
  { MIKTEX_GFTYPE_EXE, { "gftype" } },
  { MIKTEX_GREGORIO_EXE, { "gregorio" } },
  { MIKTEX_HBF2GF_EXE, { "hbf2gf" } },
  { MIKTEX_LACHECK_EXE, { "lacheck" } },
  { MIKTEX_MAKEBASE_EXE, { "makebase" } },
  { MIKTEX_MAKEFMT_EXE, { "makefmt" } },
  { MIKTEX_MAKEINDEX_EXE, { "makeindex" } },
  { MIKTEX_MAKEPK_EXE, { "makepk" } },
  { MIKTEX_MAKETFM_EXE, { "maketfm" } },
  { MIKTEX_MIKTEX_EXE, { "mkfntmap" } },
  { MIKTEX_MFT_EXE, { "mft" } },
  { MIKTEX_MF_EXE, { "mf", "inimf", "virmf" } },
  { MIKTEX_MKOCP_EXE, { "mkocp" } },
  { MIKTEX_MPOST_EXE, { "dvitomp", "mpost" } },
  { MIKTEX_ODVICOPY_EXE, { "odvicopy" } },
  { MIKTEX_OFM2OPL_EXE, { "ofm2opl" } },
  { MIKTEX_OPL2OFM_EXE, { "opl2ofm" } },
  { MIKTEX_OTP2OCP_EXE, { "otp2ocp" } },
  { MIKTEX_OUTOCP_EXE, { "outocp" } },
  { MIKTEX_OVF2OVP_EXE, { "ovf2ovp" } },
  { MIKTEX_OVP2OVF_EXE, { "ovp2ovf" } },
  { MIKTEX_PREFIX "patgen" MIKTEX_EXE_FILE_SUFFIX, { "patgen"} },
  { MIKTEX_PBIBTEX_EXE, { "pbibtex" } },
  { MIKTEX_PDFTEX_EXE, { "pdftex", MIKTEX_LATEX_EXE, MIKTEX_PDFLATEX_EXE } },
  { MIKTEX_PDFTOSRC_EXE, { "pdftosrc" } },
  { MIKTEX_PDVITYPE_EXE, { "pdvitype" } },
  { MIKTEX_PK2BM_EXE, { "pk2bm" } },
  { MIKTEX_PLTOTF_EXE, { "pltotf" } },
  { MIKTEX_PMXAB_EXE, { "pmxab" } },
  { MIKTEX_POOLTYPE_EXE, { "pooltype" } },
  { MIKTEX_PPLTOTF_EXE, { "ppltotf" } },
  { MIKTEX_PREPMX_EXE, { "prepmx" } },
  { MIKTEX_PS2PK_EXE, { "ps2pk" } },
  { MIKTEX_PSBOOK_EXE, { "psbook" } },
  { MIKTEX_PSNUP_EXE, { "psnup" } },
  { MIKTEX_PSRESIZE_EXE, { "psresize" } },
  { MIKTEX_PSSELECT_EXE, { "psselect" } },
  { MIKTEX_PSTOPS_EXE, { "pstops" } },
  { MIKTEX_PTEX_EXE, { "ptex" } },
  { MIKTEX_PTFTOPL_EXE, { "ptftopl" } },
  { MIKTEX_REBAR_EXE, { "rebar" } },
  { MIKTEX_SCOR2PRT_EXE, { "scor2prt" } },
  { MIKTEX_SJISCONV_EXE, { "sjisconv" } },
  { MIKTEX_SYNCTEX_EXE, { "synctex" } },
  { MIKTEX_T4HT_EXE, { "t4ht" } },
  { MIKTEX_TANGLE_EXE, { "tangle" } },
  { MIKTEX_PREFIX "tex2aspc" MIKTEX_EXE_FILE_SUFFIX, { "tex2aspc" } },
  { MIKTEX_TEX4HT_EXE, { "tex4ht" } },
#if defined(MIKTEX_QT)
  { MIKTEX_TEXWORKS_EXE, { "texworks" } },
#endif
  { MIKTEX_TEX_EXE, { "tex", "initex", "virtex" } },
  { MIKTEX_TFTOPL_EXE, { "tftopl" } },
  { MIKTEX_TTF2AFM_EXE, { "ttf2afm" } },
  { MIKTEX_TTF2PK_EXE, { "ttf2pk" } },
  { MIKTEX_TTF2TFM_EXE, { "ttf2tfm" } },
  { MIKTEX_UPBIBTEX_EXE, { "upbibtex" } },
  { MIKTEX_UPDVITYPE_EXE, { "updvitype" } },
  { MIKTEX_UPPLTOTF_EXE, { "uppltotf" } },
  { MIKTEX_UPTEX_EXE, { "uptex" } },
  { MIKTEX_UPTFTOPL_EXE, { "uptftopl" } },
  { MIKTEX_PREFIX "upmendex" MIKTEX_EXE_FILE_SUFFIX, { "upmendex"} },
  { MIKTEX_VFTOVP_EXE, { "vftovp" } },
  { MIKTEX_VPTOVF_EXE, { "vptovf" } },
  { MIKTEX_WEAVE_EXE, { "weave" } },
  { MIKTEX_XETEX_EXE, { "xetex", MIKTEX_XELATEX_EXE } },
  { MIKTEX_PREFIX "xml2pmx" MIKTEX_EXE_FILE_SUFFIX, { "xml2pmx" } },
#if defined(WITH_RUNGS)
  { MIKTEX_RUNGS_EXE, { "rungs" } },
#endif
#if defined(WITH_KPSEWHICH)
  { MIKTEX_KPSEWHICH_EXE, { "kpsewhich" } },
#endif
#if defined(MIKTEX_MACOS_BUNDLE)
  { MIKTEX_INITEXMF_EXE, { MIKTEX_INITEXMF_EXE }},
  { MIKTEX_MIKTEX_EXE, { MIKTEX_MIKTEX_EXE }},
  { MIKTEX_MPM_EXE, { MIKTEX_MPM_EXE } },
  { MIKTEX_TEXIFY_EXE, { MIKTEX_TEXIFY_EXE } },
  { "mthelp" MIKTEX_EXE_FILE_SUFFIX, { "mthelp" MIKTEX_EXE_FILE_SUFFIX } },
  { "miktexsetup" MIKTEX_EXE_FILE_SUFFIX, { "miktexsetup" MIKTEX_EXE_FILE_SUFFIX } },
#endif
#if defined(WITH_MKTEXLSR)
  { MIKTEX_MIKTEX_EXE, { "mktexlsr" }, LinkType::Copy },
#endif
#if defined(WITH_TEXHASH)
  { MIKTEX_MIKTEX_EXE, { "texhash" }, LinkType::Copy },
#endif
#if defined(WITH_TEXLINKS)
  { MIKTEX_MIKTEX_EXE, { "texlinks" }, LinkType::Copy },
#endif
#if defined(WITH_UPDMAP)
  { MIKTEX_MIKTEX_EXE, { "updmap" }, LinkType::Copy },
#endif
#if defined(WITH_TEXDOC)
  { "mthelp" MIKTEX_EXE_FILE_SUFFIX, { "texdoc" } },
#endif
#if defined(WITH_POPPLER_UTILS)
  { MIKTEX_PDFATTACH_EXE, { "pdfattach" } },
  { MIKTEX_PDFDETACH_EXE, { "pdfdetach" } },
  { MIKTEX_PDFFONTS_EXE, { "pdffonts" } },
  { MIKTEX_PDFIMAGES_EXE, { "pdfimages" } },
  { MIKTEX_PDFINFO_EXE, { "pdfinfo" } },
  { MIKTEX_PDFSEPARATE_EXE, { "pdfseparate" } },
#if 0
  { MIKTEX_PDFSIG_EXE, { "pdfsig" } },
#endif
  { MIKTEX_PDFTOCAIRO_EXE,{ "pdftocairo" } },
  { MIKTEX_PDFTOHTML_EXE, { "pdftohtml" } },
  { MIKTEX_PDFTOPPM_EXE, { "pdftoppm" } },
  { MIKTEX_PDFTOPS_EXE, { "pdftops" } },
  { MIKTEX_PDFTOTEXT_EXE, { "pdftotext" } },
  { MIKTEX_PDFUNITE_EXE, { "pdfunite" } },
#endif
#if defined(WITH_FONTCONFIG_UTILS)
  { MIKTEX_FC_CACHE_EXE, { "fc-cache" } },
  { MIKTEX_PREFIX "fc-cat" MIKTEX_EXE_FILE_SUFFIX, { "fc-cat" } },
  { MIKTEX_PREFIX "fc-conflist" MIKTEX_EXE_FILE_SUFFIX, { "fc-conflist" } },
  { MIKTEX_PREFIX "fc-list" MIKTEX_EXE_FILE_SUFFIX, { "fc-list" } },
  { MIKTEX_PREFIX "fc-match" MIKTEX_EXE_FILE_SUFFIX, { "fc-match" } },
  { MIKTEX_PREFIX "fc-pattern" MIKTEX_EXE_FILE_SUFFIX, { "fc-pattern" } },
  { MIKTEX_PREFIX "fc-query" MIKTEX_EXE_FILE_SUFFIX, { "fc-query" } },
  { MIKTEX_PREFIX "fc-scan" MIKTEX_EXE_FILE_SUFFIX, { "fc-scan" } },
  { MIKTEX_PREFIX "fc-validate" MIKTEX_EXE_FILE_SUFFIX, { "fc-validate" } },
#endif
#if defined(WITH_LCDF_TYPETOOLS)
  { MIKTEX_PREFIX "cfftot1" MIKTEX_EXE_FILE_SUFFIX, { "cfftot1" } },
  { MIKTEX_PREFIX "mmafm" MIKTEX_EXE_FILE_SUFFIX, { "mmafm" } },
  { MIKTEX_PREFIX "mmpfb" MIKTEX_EXE_FILE_SUFFIX, { "mmpfb" } },
  { MIKTEX_PREFIX "otfinfo" MIKTEX_EXE_FILE_SUFFIX, { "otfinfo" } },
  { MIKTEX_PREFIX "otftotfm" MIKTEX_EXE_FILE_SUFFIX, { "otftotfm" } },
  { MIKTEX_PREFIX "t1dotlessj" MIKTEX_EXE_FILE_SUFFIX, { "t1dotlessj" } },
  { MIKTEX_PREFIX "t1lint" MIKTEX_EXE_FILE_SUFFIX, { "t1lint" } },
  { MIKTEX_PREFIX "t1rawafm" MIKTEX_EXE_FILE_SUFFIX, { "t1rawafm" } },
  { MIKTEX_PREFIX "t1reencode" MIKTEX_EXE_FILE_SUFFIX, { "t1reencode" } },
  { MIKTEX_PREFIX "t1testpage" MIKTEX_EXE_FILE_SUFFIX, { "t1testpage" } },
  { MIKTEX_PREFIX "ttftotype42" MIKTEX_EXE_FILE_SUFFIX, { "ttftotype42" } },
#endif
#if defined(MIKTEX_WINDOWS)
  { MIKTEX_PREFIX "zip" MIKTEX_EXE_FILE_SUFFIX, {"zip"} },
#endif
#if defined(MIKTEX_WINDOWS)
  { MIKTEX_CONSOLE_EXE, { MIKTEX_TASKBAR_ICON_EXE, MIKTEX_UPDATE_EXE } },
  { MIKTEX_CONSOLE_ADMIN_EXE,{ MIKTEX_UPDATE_ADMIN_EXE } },
#endif
};

const vector<FileLink> LinksManager::lua52texLinks =
{
  { MIKTEX_LUAHBTEX_EXE, { "luahbtex", MIKTEX_LUALATEX_EXE } },
  { MIKTEX_LUATEX_EXE, { MIKTEX_PREFIX "texlua", MIKTEX_PREFIX "texluac", "luatex", "texlua", "texluac" } },
};
