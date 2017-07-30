# MiKTeX Change Log

## 2.9.6400 - 2017-07-31

### New programs

* asy: asymptote - 2D and 3D TeX-Aware Vector Graphics Language
* lacheck: a consistency checker checker for LaTeX documents

### Upgraded programs

* autosp 2017-07-14

### Upgraded libraries

* cairo 1.4.10
* fontconfig 2.12.3
* freetype 2.8
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
