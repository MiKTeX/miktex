# MiKTeX Change Log

## 2.9.6350 - 2017-05-28

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

* [HarfBuzz 1.4.6](https://github.com/behdad/harfbuzz/releases/tag/1.4.6)
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
