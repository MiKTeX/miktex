# MiKTeX Change Log

## 2.9.6655 - not yet released

### Upgraded programs

* Asymptote 2.44
* Dvisvgm 2.3.5
* Gregorio 5.1.1

### Bug fixes

* [94](https://github.com/MiKTeX/miktex/issues/94): Silent uninstall in Windows
* [99](https://github.com/MiKTeX/miktex/issues/99): reaching the format and language dialog
* [106](https://github.com/MiKTeX/miktex/issues/106): Texify gives un-informative error message when wrong engine given

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
