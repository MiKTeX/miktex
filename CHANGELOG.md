# MiKTeX Change Log

## next

### Upgraded programs

* asymptote 2.81
* luatex 1.15.1
* metapost 2.02

### Fixed issues

- [1138](https://github.com/MiKTeX/miktex/issues/1138): updmap crash

## 22.7.30 - 2022-07-30

### Windows 7 deprecation

Windows 7 is now deprecated.

### Upgraded programs

* dvipdfmx 20220710

### Fixed issues

- [1136](https://github.com/MiKTeX/miktex/issues/1136): platex/uplatex can't determine size of graphic automatically

## 22.7.26 - 2022-07-26

### Windows 32-bit deprecation

The 32-bit version of MiKTeX for Windows is now deprecated.

## 22.7.25 - 2022-07-25

### Fixed issues

- [1129](https://github.com/MiKTeX/miktex/issues/1129): platex build fail with tarticle class

## 22.7 - 2022-07-17

### Fixed issues

- [1071](https://github.com/MiKTeX/miktex/issues/1071): Cannot get TeXworks version: QString::arg: 1 argument(s) missing in TeXworks %1 (%2) [r.%3, %4]
- [1079](https://github.com/MiKTeX/miktex/issues/1079): spell check is gone
- [1090](https://github.com/MiKTeX/miktex/issues/1090): "Too many open files" errors with xelatex
- [1112](https://github.com/MiKTeX/miktex/issues/1112): [BUG]: wrong version number for miktex-dvisvgm-bin-x64-2.9
- [1120](https://github.com/MiKTeX/miktex/issues/1120): can't build format of platex/platex-dev
- [1121](https://github.com/MiKTeX/miktex/issues/1121): \pdffilesize doesn't work correctly in eptex and perhaps other engines

## 22.3 - 2022-03-14

### HiTeX

MiKTeX now includes the HiTeX engine.

### Upgraded programs

* asymptote 2.79
* cweb 4.7
* dvips 2022.1
* dvisvgm 2.13.3
* luatex 1.15.0
* pmxab 2.9.8a
* ptex 4.0.0
* texworks 0.6.7
* xetex 0.999994

### Fixed issues

- [1054](https://github.com/MiKTeX/miktex/issues/1054): Open Terminal window does nothing on elementary OS
- [1055](https://github.com/MiKTeX/miktex/issues/1055): GFtoPK crashes in supre mode for fonts in certain optical sizes
- [1062](https://github.com/MiKTeX/miktex/issues/1062): mpm --admin --require=@listfile with non-existing listfile
- 
## 22.1 - 2022-01-31

### Fixed issues

- [995](https://github.com/MiKTeX/miktex/issues/995): Support for Fedora 35
- [1022](https://github.com/MiKTeX/miktex/issues/1022): expand variables when locating files

## 21.12.10 - 2021-12-10

### Upgraded programs

* bibsort 2.3

### Fixed issues

- [984](https://github.com/MiKTeX/miktex/issues/984): xelatex crashes if fonts are called by name and the open type renderer is used

## 21.12 - 2021-12-07

### (e-)(u)pTeX

MiKTeX now includes the four TeX engines of the "Japanese TeX processing tools" suite.

### Upgraded programs

* dvipdfmx 20211117
* makeindex 2.16
* pdftex 1.40.24
* upmendex 1.00

### Fixed issues

- [858](https://github.com/MiKTeX/miktex/issues/858): Hope MiKTeX can support upTeX series (upTeX/upLaTeX/eupTex...) engine
- [983](https://github.com/MiKTeX/miktex/issues/983): dvisvgm: unexpected error in conjunction with raw SVG elements

## 21.10 - 2021-10-05

### Fixed issues

- [899](https://github.com/MiKTeX/miktex/issues/899): Texify: Getting version leads to internal error
- [921](https://github.com/MiKTeX/miktex/issues/921): Epstopdf Crashes
- [923](https://github.com/MiKTeX/miktex/issues/923): Handle competing fontconfig installation
- [933](https://github.com/MiKTeX/miktex/issues/933): "texify.exe" crashes on Windows in 21.6 when invoked with incorrect usage
- [940](https://github.com/MiKTeX/miktex/issues/940): remove type1 font directories from localfonts.conf

## 21.8 - 2021-08-06

### Documentation browser

MiKTeX Console now provides a user interface which makes it easier to find and view package documentation.

### Fixed issues

- [338](https://github.com/MiKTeX/miktex/issues/338): MikTeX Console : direkt link to open package documentation

## 21.7 - 2021-07-19

### Upgraded programs

* dvipdfmx 20210609
* pdftex 1.40.23

### Fixed issues

- [847](https://github.com/MiKTeX/miktex/issues/847): Shared vs not-shared?
- [863](https://github.com/MiKTeX/miktex/issues/863): Auto-Install package__doc by file name
- [867](https://github.com/MiKTeX/miktex/issues/867): MiKTeX console crash while changing remote package repository
- [869](https://github.com/MiKTeX/miktex/issues/869): MiKTeX setup utility encounters internal error during installation
- [871](https://github.com/MiKTeX/miktex/issues/871): TeXstudio stopped working - could not find perl.exe

## 21.6.28 - 2021-06-28

### Breaking changes

#### Common TEXMF root directories in a private (current user) installation

In the past, common TEXMF root directories (e.g., `C:\Program Files\MiKTeX`)
were in the input search path, even if MiKTeX was installed for the current user only.
As of 21.6.28, common TEXMF root directories are excluded from the input search path.

This should have no impact for most users, because these directories usually
do not exist when MiKTeX is installed for the current user.

### Fixed issues

- [841](https://github.com/MiKTeX/miktex/issues/841): When a package have been uninstalled, its directories are not removed
- [844](https://github.com/MiKTeX/miktex/issues/844): Documentation not found
- [846](https://github.com/MiKTeX/miktex/issues/846): miktex update causes issues “go to source/pdf” in TeXstudio
  
## 21.6 - 2021-06-07

### Breaking changes

Non-secure package repository URLs (schemes `ftp` and `http`) are deprecated. The package repository
picker (`<Random package repository on the Internet>`) always chooses a secure URL (scheme `https`).

### Upgraded programs

* luatex 1.14.0
* metapost 2.01

### Fixed issues

- [754](https://github.com/MiKTeX/miktex/issues/754): search path of automatic format creation
- [762](https://github.com/MiKTeX/miktex/issues/762): output of lfs.dir in luatex
- [765](https://github.com/MiKTeX/miktex/issues/765): MiKTeX 21.3 / pdflatex crashes sometimes after compiling LaTeX documents
- [768](https://github.com/MiKTeX/miktex/issues/768): kpsewhich with option --all
- [780](https://github.com/MiKTeX/miktex/issues/780): [MikTeX Portable] Can't remove local texmf directory
- [785](https://github.com/MiKTeX/miktex/issues/785): Create all auxiliary files in the directory specified by -aux-directory without ever writing in the source directory
- [825](https://github.com/MiKTeX/miktex/issues/825): wrong search path for lua files with lualatex-dev
- [833](https://github.com/MiKTeX/miktex/issues/833): Package downloader chooses insecure random mirrors

## 21.3 - 2021-03-16

### Upgraded programs

* axohelp 1.4
* dvipdfmx 20210318
* gregorio 6.0.0-beta2
* texworks 0.6.6

### Fixed issues

- [698](https://github.com/MiKTeX/miktex/issues/698): Unable to update to MikTeX 21.1 on Fedora 33
- [732](https://github.com/MiKTeX/miktex/issues/732): texify --clean has trouble with .aux files using luatex engine
- [740](https://github.com/MiKTeX/miktex/issues/740): Asymptote fails to startup

## 21.2 - 2021-02-22

### MiKTeX Console Simplified Chinese translation

The MiKTeX Console user interface has been translated to Simplified Chinese.

### New programs

- xml2pmx 2021-02-07

### Upgraded programs

* 2021 tune-up of TeX and METAFONT
* asymptote 2.69
* dvipdfmx 20210212
* dvisvgm 2.11.1
* pdftex 1.40.22
* pmxab 2.9.4a
* xetex 0.999993

### Fixed issues

- [697](https://github.com/MiKTeX/miktex/issues/697): Add zh_CN translations

## 21.1 - 2021-01-18

### MiKTeX Console German translation

MiKTeX Console now supports localization. To begin with, the user interface has been translated to German.

### Upgraded programs

* asymptote 2.68
* autosp 2020-07-01
* dvipdfmx 20210116
* dvisvgm 2.11
* luatex 1.13.0

### Fixed issues

- [676](https://github.com/MiKTeX/miktex/issues/676): MiKTeX Console localization: de_DE

## 20.12 - 2020-12-14

### Breaking changes

Options `--disable-pipes` and `--enable-pipes` (TeX engines) are deprecated and will be removed in a future release. If specified, they now don't have any effect.

### Fixed issues

- [597](https://github.com/MiKTeX/miktex/issues/597): xetex rounds all image sizes to integer pt values
- [652](https://github.com/MiKTeX/miktex/issues/652): miktexsetup.exe: MiKTeX encountered an internal error.
- [659](https://github.com/MiKTeX/miktex/issues/659): Check for directory that executable doesn't have permissions for causes crash
- [666](https://github.com/MiKTeX/miktex/issues/666): miktex (lualatex.exe) fails to run from flask subprocess
- [668](https://github.com/MiKTeX/miktex/issues/668): expand tilde prefixes
- [669](https://github.com/MiKTeX/miktex/issues/669): testing if pipes are enabled.
- [672](https://github.com/MiKTeX/miktex/issues/672): miktex-mf (or maketfm) doesn't find .mf files in the current directory

## 20.11 - 2020-11-02

### Upgraded programs

* asymptote 2.67
* axohelp 1.3
* dvipdfmx 20201014
* dvisvgm 2.10.1

### Fixed issues

- [648](https://github.com/MiKTeX/miktex/issues/648): lualatex/xelatex: handling of `stderr` when using pipes
- [650](https://github.com/MiKTeX/miktex/issues/650): Compilation get's stuck when using input pipes with inputs larger than 4KB

## 20.10 - 2020-10-12

### Fixed issues

- [206](https://github.com/MiKTeX/miktex/issues/206): store log-files of "build format" runs
- [598](https://github.com/MiKTeX/miktex/issues/598): Installing anyway when hitting "back" on windows
- [607](https://github.com/MiKTeX/miktex/issues/607): MiKTeX 20.6.29 64 bit installation bug
- [634](https://github.com/MiKTeX/miktex/issues/634): .fls files are put in output directory, should be placed in aux directory instead
- [635](https://github.com/MiKTeX/miktex/issues/635): make4ht odt compilations on miktex systems fail

## 20.7 - 2020-08-03

### Fixed issues

- [576](https://github.com/MiKTeX/miktex/issues/576): Net Installer Crashing
- [581](https://github.com/MiKTeX/miktex/issues/581): pdflatex fails with "invalid argument in putc"
- [584](https://github.com/MiKTeX/miktex/issues/584): upon rebuilding all formats: invalid value definition in mltex.ini

## 20.6.29 - 2020-06-29

This followup release to fix some regressions:

- [568](https://github.com/MiKTeX/miktex/issues/568): pdflatex fails after recent update
- [569](https://github.com/MiKTeX/miktex/issues/569): \show crashes MiKTeX

## 20.6 - 2020-06-28

### New release versioning scheme

The MiKTeX project has switched to a date-based versioning scheme for releases.

See this issue for more info: [https://github.com/MiKTeX/miktex/issues/554](https://github.com/MiKTeX/miktex/issues/554)

### New programs

- patgen 1.0.6
- upmendex 0.54

### Fixed issues

- [41](https://github.com/MiKTeX/miktex/issues/41): Error running miktex on Win 10 ver. 1709
- [534](https://github.com/MiKTeX/miktex/issues/534): include patgen in repo
- [538](https://github.com/MiKTeX/miktex/issues/538): MACOS, mthelp calls firefox instead of open
- [552](https://github.com/MiKTeX/miktex/issues/552): LuaLaTex fails on special characters in the windows user account path
- [554](https://github.com/MiKTeX/miktex/issues/554): MiKTeX release versioning
- [556](https://github.com/MiKTeX/miktex/issues/556): -jobname cli option broken on latex, pdflatex, xelatex, still working on lualatex
- [561](https://github.com/MiKTeX/miktex/issues/561): include upmendex in repository

## 2.9.7440 - 2020-05-17

### Breaking changes

#### Removal of Omega 1.15

Omega 1.15 is obsolete and has now been removed from the distribution.

#### Windows 8.3 file names

In the past, it was possible to specify Windows 8.3 file names (aka
short file names) when invoking a TeX engine. As of 2.9.7440, this is
not supported anymore.

Impact: The following use case is broken:

```
pdflatex abraka~1.tex
```

where `abraka~1.tex` is the 8.3 variant of the long file name `abrakadabra.tex`.

#### Package database signing

In the past, package database signing was optional. As of MiKTeX
2.9.7440, it is required that MiKTeX package database files are
provided with a valid signature.

Reason: In order to support custom remote package repositories (see
[Allow local http
mirrors](https://github.com/MiKTeX/miktex/issues/519)), it must be
ensured that the contents of the custom package repository is
unmodified.

Impact: Users will receive an error message if they visit an outdated
(no signature) or modified (tampered database) package repository.

### Unrestricted shell escape commands and elevated privileges

As of MiKTeX 2.9.7440, it is possible to prevent the execution of
unrestricted shell escape commands when a program is running with
elevated privileges:

```
initexmf --verbose --set-config-value [Core]AllowUnrestrictedSuperUser=f
```

This prevents the following use case:

```
sudo pdflatex --shell-escape file.tex
```

where `file.tex` contains:

```
\documentclass{minimal}
\usepackage{shellesc}
\begin{document}
\ShellEscape{echo hello, world! > hello.txt}
\input{hello.txt}
\end{document}
```

For more information and discussion, visit the issue page at GitHub:

* [533](https://github.com/MiKTeX/miktex/issues/533): Unrestricted shell escape commands and elevated privileges

### Ubuntu 20.04 & Fedora 32

MiKTeX is now available for Ubuntu 20.04 and Fedora 32.  See [this
howto](https://miktex.org/howto/install-miktex-unx) for more info.

### Fixed issues

* [486](https://github.com/MiKTeX/miktex/issues/486): Luatex cannot find input file when using texify on windows
* [491](https://github.com/MiKTeX/miktex/issues/491): Empty file error when trying to complete the setup on a Mac
* [510](https://github.com/MiKTeX/miktex/issues/510): Long file name support
* [514](https://github.com/MiKTeX/miktex/issues/514): Kerberos Proxy support
* [520](https://github.com/MiKTeX/miktex/issues/520): Make MiKTeX Console usable for keyboard only users
* [521](https://github.com/MiKTeX/miktex/issues/521): Cannot install on Fedora 32
* [528](https://github.com/MiKTeX/miktex/issues/528): Ubuntu 20.04 Repositories
* [532](https://github.com/MiKTeX/miktex/issues/532): Calling external commands with quotes in path via shell on Windows
* [541](https://github.com/MiKTeX/miktex/issues/541): Change BibTeX "I couldn't open database file" message to include double extension

## 2.9.7400 - 2020-04-26

### Fixed issues

* [312](https://github.com/MiKTeX/miktex/issues/312): MiKTeX Setup Wizard ignores --user-install in Windows 10
* [429](https://github.com/MiKTeX/miktex/issues/429): Suggested feature: epstopdf - alert on missing font packages

### Upgraded programs

* asymptote 2.65
* texworks 0.6.5

## 2.9.7350 - 2020-03-22

### lualatex: switching engine from LuaTeX to LuaHBTeX

`lualatex` now uses LuaHBTeX as the underlying engine.

### Upgraded programs

* asymptote 2.64
* autosp 2020-03-11
* dvipdfmx 20200315
* dvisvgm 2.9.1
* texworks 0.6.4

### Fixed issues

* [445](https://github.com/MiKTeX/miktex/issues/445): Autoinstall packages on the fly (without asking): give option to install for all users
* [475](https://github.com/MiKTeX/miktex/issues/475): BibTeX single_fn_space = 100 is too small; could we have 1000, please?
* [486](https://github.com/MiKTeX/miktex/issues/486): Luatex cannot find input file when using texify on windows

## 2.9.7300 - 2020-02-17

### Group-delimited \input argument

MiKTeX adopted the forthcoming TL 2020 \input enhancement which allows
you to specify file with spaces in the name:

```
\input{my stuff}
\input{"my stuff"}
```

To quote Karl Berry:

> For TL 2020, I think we're going to be able to extend
> \input so that it can take a grouped argument (\input{...}), and thus
> allow any character in the name -- except that " characters will still
> be eliminated, at LaTeX's request.  This won't affect the standard
> space/token-delimited use of \input in any way.

### CentOS 8

MiKTeX is now available for CentOS 8. Quick installation instructions:

```
sudo rpm --import "https://keyserver.ubuntu.com/pks/lookup?op=get&search=0xD6BC243565B2087BC3F897C9277A7293F59E4889"
sudo curl -L -o /etc/yum.repos.d/miktex.repo https://miktex.org/download/centos/8/miktex.repo
sudo dnf update
sudo dnf install miktex
```

Please read this [HOWTO](https://miktex.org/howto/install-miktex-unx),
if this is the first time you install MiKTeX for Linux.

### LCDF Typetools

The MiKTeX distribution now includes [LCDF Typetools](https://ctan.org/pkg/lcdf-typetools),
a bundle of outline font manipulation tools.

### Upgraded programs

* asymptote 2.62
* autosp 2020-02-06
* dvipdfmx 20200116
* dvipng 1.17
* dvips 2020.1
* dvisvgm 2.8.2
* LuaTeX 1.12.0
* pdftex 1.40.21
* pmxab 2.9.4

### Upgraded libraries

* libpng 1.6.37

### Fixed issues

* [419](https://github.com/MiKTeX/miktex/issues/419): Admin mode required but not accessible
* [420](https://github.com/MiKTeX/miktex/issues/420): miktexsetup fails -- how to advance?
* [422](https://github.com/MiKTeX/miktex/issues/422): Increase font_mem_size upper limit
* [428](https://github.com/MiKTeX/miktex/issues/428): GSF2PK failed on postscript font
* [435](https://github.com/MiKTeX/miktex/issues/435): language patterns in the format can not be changed manually
* [436](https://github.com/MiKTeX/miktex/issues/436): Problems with the oberdiek package
* [438](https://github.com/MiKTeX/miktex/issues/438): Create "fc-cache.exe" as alias
* [441](https://github.com/MiKTeX/miktex/issues/441): Cannot build new format
* [448](https://github.com/MiKTeX/miktex/issues/448): miktex-makepk fails if the transcript file does not exist
* [451](https://github.com/MiKTeX/miktex/issues/451): Clicking Close button vs pressing Esc key upon finished package installation
* [459](https://github.com/MiKTeX/miktex/issues/459): dvipng 1.16 with pk fonts error on Win32
* [471](https://github.com/MiKTeX/miktex/issues/471): BibTeX max_glob_strs = 10 or 20 much too small; could we have 100, please?

## 2.9.7250 - 2019-11-12

### New programs

* LuaHBTeX 1.11.2

### Upgraded programs

* LuaTeX 1.11.2

### Fixed issues

* [25](https://github.com/MiKTeX/miktex/issues/25): tex4ht support
* [328](https://github.com/MiKTeX/miktex/issues/328): initexmf: access denied error
* [411](https://github.com/MiKTeX/miktex/issues/411): texify does not run bibtex

## 2.9.7230 - 2019-10-28

Fixed macOS build issues.

## 2.9.7200 - 2019-10-22

### Breaking changes

#### Windows: text files with UNIX line endings

All TeX engines now write text output files with UNIX line endings.

#### EPS-to-PDF converter embeds all fonts

The `miktex-epstopdf` utility now creates a PDF with embedded fonts.

### New programs

* synctex 1.5

### Downgraded programs

* luatex 1.10.0

### Fixed issues

* [353](https://github.com/MiKTeX/miktex/issues/353): luatex 1.10.1 has a bug affecting font usage
* [363](https://github.com/MiKTeX/miktex/issues/363): SyncTeX binaries?
* [369](https://github.com/MiKTeX/miktex/issues/369): luatex doesn't handle commands in the name of the main file correctly
* [370](https://github.com/MiKTeX/miktex/issues/370): line endings
* [373](https://github.com/MiKTeX/miktex/issues/373): tftopl failed with large .tfm file
* [376](https://github.com/MiKTeX/miktex/issues/376): Miktex Update removes essential packages
* [377](https://github.com/MiKTeX/miktex/issues/377): pdfcrop: The script engine could not be found.
* [391](https://github.com/MiKTeX/miktex/issues/391): miktex-console_admin building format fails

## 2.9.7140 - 2019-07-31

### New programs

* [harftex](https://github.com/khaledhosny/harftex): new TeX enginge with embedded HarfBuzz and Lua

### Upgraded programs

* dvipdfmx 20190522
* luatex 1.10.1

### Upgraded libraries

* harfbuzz 2.5.3

## 2.9.7050 - 2019-05-30

### Breaking changes

#### `--c-style-errors` does not imply `\scrollmode` anymore

You can use the `--interaction=scrollmode` option, if you depend on the old behaviour:

```
pdflatex --c-style-errors --interaction=scrollmode document.tex
```

### Upgraded programs

* dvipdfmx 20190305

### Upgraded libraries

* uriparser 0.9.2

### Fixed issues

* [181](https://github.com/MiKTeX/miktex/issues/181): Make file-line-error more talkative
* [293](https://github.com/MiKTeX/miktex/issues/293): Bundles old vulnerable copy of uriparser 0.8.6, please update
* [294](https://github.com/MiKTeX/miktex/issues/294): kpsewhich not in PATH in MacOS
* [312](https://github.com/MiKTeX/miktex/issues/312): MiKTeX Setup Wizard ignores --user-install in Windows 10

## 2.9.7000 - 2019-04-10

### Upgraded programs

* dvipdfmx 20190225
* dvipng 1.16
* dvips 5.999
* dvisvgm 2.6.3
* Gregorio 5.2.0
* luatex 1.10.0
* tex4ht 2018-07-03-10:36
* TeXworks 0.6.3

### Fixed issues

* [195](https://github.com/MiKTeX/miktex/issues/195): Add option to change directory for binaries (Linux)
* [262](https://github.com/MiKTeX/miktex/issues/262): keeping user/admin updates in sync
* [263](https://github.com/MiKTeX/miktex/issues/263): MPM crashes with --require=@
* [264](https://github.com/MiKTeX/miktex/issues/264): MiKTeX failed to test file existence for /dev/null, on macOS and Linux
* [272](https://github.com/MiKTeX/miktex/issues/272): TEXMFHOME is empty on Windows
* [279](https://github.com/MiKTeX/miktex/issues/279): Latest GregorioTeX update did not include updated binary

## 2.9.6980 - 2019-02-11

### Fixed issues

* [254](https://github.com/MiKTeX/miktex/issues/254): biber: missing on Ubuntu and Mac

## 2.9.6960 - 2019-02-04

### Upgraded programs

* dvipdfmx 20190127
* dvisvgm 2.6.2
* luatex 1.09.2

### Fixed issues

* [249](https://github.com/MiKTeX/miktex/issues/249): TeXworks blocks FNDB

## 2.9.6930 - 2019-01-06

### Resetting personal MiKTeX configuration

In order to avoid leftovers, MiKTeX Console learned to reset the
personal MiKTeX configuration if MiKTeX was installed system-wide:

![](images/console-cleanup-reset-personal.png)

This allows users to remove their MiKTeX related installation files
before a system administrator completely uninstalls MiKTeX.

### Admin mode: blinding out of personal MiKTeX configuration

Prior to MiKTeX 2.9.6930, personal TEXMF root directories were visible
(e.g., in MiKTeX Console) when running in administrator mode. This
behaviour has been changed: in MiKTeX 2.9.6930 and higher, personal
TEXMF root directories are blinded out when running in administrator
mode.

### Update improvements

The `Check for updates` operation is much faster now.

### Upgraded programs

* dvipdfmx 20181221
* XeTeX 0.999991

### Fixed issues

* [43](https://github.com/MiKTeX/miktex/issues/43): MPM (admin) cannot be run if TeXworks is running
* [220](https://github.com/MiKTeX/miktex/issues/220): MikTex TexWorks on Mac not displaying any text.
* [222](https://github.com/MiKTeX/miktex/issues/222): Running MiKTeX commands in parallel fails
* [226](https://github.com/MiKTeX/miktex/issues/226): xetex does not look for local user's font folder
* [235](https://github.com/MiKTeX/miktex/issues/235): Multithreading poppler utilities on windows yields errors

## 2.9.6880 (intermediate) - 2018-11-12

### Upgraded programs

* dvipdfmx 20181012
* Dvisvgm 2.6
* LuaTeX 1.09.0

### Upgraded libraries

* cairo 1.16.0
* expat 2.2.6
* fontconfig 2.13.1
* FreeType 2.9.1
* gmp 6.1.2
* graphite2 1.3.12
* HarfBuzz 2.0.2
* hunspell 1.6.2
* jpeg 9c
* libcurl 7.61.1
* liblzma 5.2.4
* libpng 1.6.35
* LibreSSL 2.8.2
* m-tx 0.63a
* mpfr 4.0.1
* mspack 0.8alpha
* teckit 2.5.8
* uriparser 0.8.6
* zzip 0.13.68

## 2.9.6840 - 2018-09-30

### Fixed security issues

#### CVE-2018-17407

This applies to MiKTeX (TeX Live is an upstream source):

> An issue was discovered in t1_check_unusual_charstring functions in writet1.c files in TeX Live before 2018-09-21.
> A buffer overflow in the handling of Type 1 fonts allows arbitrary code execution when a malicious font is loaded by one of
> the vulnerable tools: pdflatex, pdftex, dvips, or luatex.

More info: [https://nvd.nist.gov/vuln/detail/CVE-2018-17407](https://nvd.nist.gov/vuln/detail/CVE-2018-17407)

### Fixed issues

* [199](https://github.com/MiKTeX/miktex/issues/199): miktex searches the whole file system if grffile is used

## 2.9.6800 - 2018-08-27

### Fedora 28 & openSUSE Leap 15

MiKTeX is now available for Fedora 28 and openSUSE Leap 15.

### Fixed issues

* [36](https://github.com/MiKTeX/miktex/issues/36): enhance support to install packages from a list file
* [50](https://github.com/MiKTeX/miktex/issues/50): pdftex (started via the compiler driver) can't find input files if the main file is on a network drive
* [66](https://github.com/MiKTeX/miktex/issues/66): [MikTeX Portable] Can't add local texmf directory
* [149](https://github.com/MiKTeX/miktex/issues/149): Texify / macOS: "The TeX engine could not be found."
* [156](https://github.com/MiKTeX/miktex/issues/156): latexmk does not work on a fresh mac install
* [161](https://github.com/MiKTeX/miktex/issues/161): xelatex: -disable-installer is not propagated to miktex-makemf
* [166](https://github.com/MiKTeX/miktex/issues/166): Alternative for sudo
* [168](https://github.com/MiKTeX/miktex/issues/168): Installing MiKTex via Command Line
* [174](https://github.com/MiKTeX/miktex/issues/174): initexmf --force --mklinks --admin fails if TeXworks is open

## 2.9.6730 - 2018-06-29

### OS X El Capitan & macOS Sierra

MiKTeX is now available for these older Macintosh platforms:

* OS X El Capitan
* macOS Sierra

### Upgraded libraries

* HarfBuzz 1.8.0

### Fixed issues

* [42](https://github.com/MiKTeX/miktex/issues/42): download of cm-super is speed-limited
* [116](https://github.com/MiKTeX/miktex/issues/116): Cannot run on El Capitan
* [138](https://github.com/MiKTeX/miktex/issues/138): U+0361 COMBINING DOUBLE INVERTED BREVE is misaligned when using Linguistics Pro font
* [143](https://github.com/MiKTeX/miktex/issues/143): MiKTeX portable creates start menu entries
* [148](https://github.com/MiKTeX/miktex/issues/148): MiKTeX Console: uninstall MiKTeX

## 2.9.6700 - 2018-05-21

### Ubuntu 18.04 & Debian 9

MiKTeX is now available for Ubuntu 18.04 and Debian 9.  See [this
howto](https://miktex.org/howto/install-miktex-unx) for more info.

### Upgraded programs

* dvipdfmx 20180506
* Dvisvgm 2.4

### Fixed issues

* [94](https://github.com/MiKTeX/miktex/issues/94): Silent uninstall in Windows
* [120](https://github.com/MiKTeX/miktex/issues/120): Installation error in Ubuntu 18.04
* [122](https://github.com/MiKTeX/miktex/issues/122): Get back "a random package repository on the Internet"
* [128](https://github.com/MiKTeX/miktex/issues/128): Starting miktex console as administrator is not detected

## 2.9.6670 - 2018-04-21

### Upgraded programs

* Asymptote 2.44
* Dvisvgm 2.3.5
* Gregorio 5.1.1

### Fixed issues

* [26](https://github.com/MiKTeX/miktex/issues/26): native Asymptote adaptive OpenGL-based 3D-renderer doesn't work
* [94](https://github.com/MiKTeX/miktex/issues/94): Silent uninstall in Windows
* [99](https://github.com/MiKTeX/miktex/issues/99): reaching the format and language dialog
* [100](https://github.com/MiKTeX/miktex/issues/100): Running miktexsetup --portable with elevated permissions fails
* [106](https://github.com/MiKTeX/miktex/issues/106): Texify gives un-informative error message when wrong engine given
* [112](https://github.com/MiKTeX/miktex/issues/112): Faulty prepmx binary

## 2.9.6650 - 2018-03-25

Fixed a MiKTeX Console regression bug on macOS/Linux.

## 2.9.6630 - 2018-03-17

### Upgraded programs

* axohelp 1.2
* autosp 2018-02-14
* dvipdfmx 20180217
* dvips 5.998
* Dvisvgm 2.3.3
* pdfTeX 1.40.19
* pmxab 2.8.4

### Bug fixes

* [70](https://github.com/MiKTeX/miktex/issues/70): MPM search filter doesn't show all packages
* [71](https://github.com/MiKTeX/miktex/issues/71): mpm --require=@listfile issue
* [73](https://github.com/MiKTeX/miktex/issues/73): pdflatex incorrectly modifies path if double semicolon is present in $env:PATH
* [74](https://github.com/MiKTeX/miktex/issues/74): texdoc and mthelp command not found
* [77](https://github.com/MiKTeX/miktex/issues/77): xelatex segfault (Linux Mint)
* [81](https://github.com/MiKTeX/miktex/issues/81): libfreetype.6.dylib cannot be called rightly using command-line on Mac

## 2.9.6600 - 2018-02-11

### MiKTeX Console

[MiKTeX Console](https://miktex.org/announcement/miktex-console) is
the new MiKTeX management tool which combines three UI applications
into one:

* MiKTeX Package Manager
* MiKTeX Settings
* MiKTeX Update Wizard

### `miktex-lua53tex`

LuaTeX now comes in two variants:

* `miktex-luatex` is the default engine with Lua 5.2 inside
* `miktex-lua53tex` is the experimental engine with Lua 5.3 inside

#### Switching engines

By default, the LuaTeX executables (`luatex`, `lualatex`, ...) are
linked to the 5.2 engine.

You can switch the engine by setting a configuration value and
recreating links to the executables.

To use the new 5.3 based engine:

    initexmf --set-config-value [luatex]luaver=5.3
    initexmf --force --mklinks

To switch back to the 5.2 based engine:

    initexmf --set-config-value [luatex]luaver=5.2
    initexmf --force --mklinks

As always: run `initexmf` with administrator privileges and add the
`--admin` option, if you manage a shared MiKTeX installation.

### Upgraded programs

* axohelp 1.1
* Dvisvgm 2.3.2
* LuaTeX 1.07.0
* m-tx 0.63
* XeTeX 0.99999

### Upgraded libraries

* cairo 1.14.12

## 2.9.6530 - 2017-11-20

### Bug fixes

* luatex does not work properly on Ubuntu

## 2.9.6500 - 2017-11-04

### Upgraded programs

* dvipdfmx 20170918, XeTeX
  Support /Rotate in PDF image inclusion
* Dvisvgm 2.2
* LuaTeX 1.06.2

### Upgraded libraries

* fontconfig 2.12.6
* FreeType 2.8.1
* HarfBuzz 1.6.3
* ICU 60.1
* libcurl 7.56.1
* LibGD 2.2.5
* libpng 1.6.34
* Poppler 0.60.1
* Qt 5.9.2

### Bug fixes

* [4](https://github.com/MiKTeX/miktex/issues/4): `xdvipdfmx` with options supplied as `output-driver` to XeLaTeX results in `can't write on file` error
* [5](https://github.com/MiKTeX/miktex/issues/5): Package manager "Miktex encountered an internal error" *[windows]*
* [19](https://github.com/MiKTeX/miktex/issues/19): option --user-install of MiKTeX setup utility is ignored *[windows]*
* [22](https://github.com/MiKTeX/miktex/issues/22): miktex is noticably slower than texlive
* [2618](https://sourceforge.net/p/miktex/bugs/2618/): Unknown filter "JPXDecode" Syntax Error
* [2620](https://sourceforge.net/p/miktex/bugs/2620/): xetex can't input files with BOM anymore
* [2622](https://sourceforge.net/p/miktex/bugs/2622/): shell commands are not executed correctly
* [2624](https://sourceforge.net/p/miktex/bugs/2624/): Problem with \pdfresettimer & \pdfelapsedtime
* [2625](https://sourceforge.net/p/miktex/bugs/2625/): Package installation fails as normal user account

## 2.9.6400 - 2017-07-31

### New programs

* asy: asymptote - 2D and 3D TeX-Aware Vector Graphics Language
* lacheck: a consistency checker checker for LaTeX documents

### Upgraded programs

* autosp 2017-07-14

### Upgraded libraries

* cairo 1.4.10
* fontconfig 2.12.3
* FreeType 2.8
* graphite2 1.3.10
* HarfBuzz 1.4.7
* libcurl 7.54.1
* libpng 1.6.30
* teckit 2.5.7
* zzip 0.13.66

### Ubuntu 16.04 / Linux Mint 18.x

It is now possible to install MiKTeX on Ubuntu 16.04 64-bit and Linux Mint 18.x 64-bit:

    sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys D6BC243565B2087BC3F897C9277A7293F59E4889
    echo "deb http://miktex.org/download/ubuntu xenial universe" | sudo tee /etc/apt/sources.list.d/miktex.list
    sudo apt-get update
    sudo apt-get install miktex

See [this howto](https://miktex.org/howto/install-miktex-unx) for more info.

### Dockerized build environments

The MiKTeX build environment has been dockerized for the following
Linux distros:

* [Ubuntu 16.04](https://github.com/MiKTeX/docker-miktex-build-xenial)
* [Debian 9](https://github.com/MiKTeX/docker-miktex-build-stretch)
* [Fedora 25](https://github.com/MiKTeX/docker-miktex-build-fedora-25)

This makes it possible to build MiKTeX using Docker.  No build tools
required.

For example, to build MiKTeX for Debian 9:

    mkdir -p /tmp/miktex
    curl -fsSL https://miktex.org/download/ctan/systems/win32/miktex/source/miktex-2.9.tar.xz | \
      tar -xJ --strip-components=1 -C /tmp/miktex
    docker pull miktex/miktex-build-stretch
    docker run --rm -t \
      -v /tmp/miktex:/miktex/source:ro \
      -v `pwd`:/miktex/build:rw \
      miktex/miktex-build-stretch

### Closed feature requets

* [345](https://sourceforge.net/p/miktex/feature-requests/345/): Build pdftocairo with libpng support 
* [346](https://sourceforge.net/p/miktex/feature-requests/346/): Include Asymptote 

### Bug fixes

* dvipdfmx: warning: Ignore invalid attribute dictionary

## 2.9.6350 - 2017-06-03

### New programs

* axohelp: a helper utility for the [axodraw2 package](https://miktex.org/packages/axodraw2).

### Upgraded programs

* TeXworks 0.6.2

### Changed programs

* dvipdfmx
  - Support additional length units
* pdfTeX
  - Adding SyncTeX support to `\pdfxform`
  
### Upgraded libraries

* HarfBuzz 1.4.6
* libcurl 7.54.0
* LibreSSL 2.5.3
* Poppler 0.55.0

### macOS support

It is now possible to install MiKTeX via the Homebrew package manager:

    /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
    brew tap miktex/miktex
    brew install miktex

See [this howto](https://miktex.org/howto/install-miktex-mac) for more info.

### Bug fixes

* [2604](https://sourceforge.net/p/miktex/bugs/2604/): Report clearly when repository servers are intentionally offline

## 2.9.6300 - 2017-04-25

### New programs

* [autosp](https://miktex.org/packages/autosp) - a preprocessor that generates note-spacing commands for MusiXTeX scores
* [Gregorio](http://gregorio-project.github.io) - convert from gabc to GregorioTeX

### Upgraded programs

* Dvisvgm 2.1.3
* LuaTeX 1.0.4
* pdfTeX 1.40.18
* XeTeX 0.99998

### Changed programs

* LuaTeX
  - New option: [--include-directory=DIR](https://docs.miktex.org/manual/luatex.html#luatex-include-directory)

### New platforms

It is possible to build this release on [Linux](https://miktex.org/howto/build-unx) and [macOS](https://miktex.org/howto/build-mac).

### Bug fixes

* [2570](https://sourceforge.net/p/miktex/bugs/2570/): pdflatex doesn't ignore BOM (byte order mark)
* [2579](https://sourceforge.net/p/miktex/bugs/2579/): xelatex adds unwanted stuff to stdin at the end of the compilation
* [2580](https://sourceforge.net/p/miktex/bugs/2580/): wrong output to stdin when using lualatex
* [2581](https://sourceforge.net/p/miktex/bugs/2581/): epstopdf creates .eps.pdf file instead of .pdf as in TeXLive
* [2586](https://sourceforge.net/p/miktex/bugs/2586/): Bis: lualatex.exe fails if the profil name has non ASCII characters
* [2591](https://sourceforge.net/p/miktex/bugs/2591/): Noto fonts not listed in MO font list
* [2593](https://sourceforge.net/p/miktex/bugs/2593/): luatex can't include colors of a plot
