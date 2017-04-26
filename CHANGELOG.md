# MiKTeX Change Log

## 2.9.6320 - not yet released

### Upgraded programs

* dvipdfmx 20170318
  - TL sync: support additional length units

### Upgraded libraries

* libcurl 7.54.0
* [HarBuzz 1.4.6](https://github.com/behdad/harfbuzz/releases/tag/1.4.6)

## 2.9.6300 - 2017-05-25

### Additions

* [Gregorio](http://gregorio-project.github.io) - convert from gabc to GregorioTeX
* [autosp](http://ctan.org/pkg/autosp) - a preprocessor that generates note-spacing commands for MusiXTeX scores

### Upgraded programs

* Dvisvgm 2.1.3
* LuaTeX 1.0.4
* pdfTeX 1.40.18
* XeTeX 0.99998

### New features

* LuaTeX now understands the [--include-directory](https://docs.miktex.org/manual/luatex.html#luatex-include-directory) option.

### New platforms

It is possible to build this release on [Linux](https://miktex.org/howto/build-unx) and [macOS](https://miktex.org/howto/build-mac).

### Bug fixes

* [2593](https://sourceforge.net/p/miktex/bugs/2593/): luatex can't include colors of a plot
* [2591](https://sourceforge.net/p/miktex/bugs/2591/): Noto fonts not listed in MO font list
* [2586](https://sourceforge.net/p/miktex/bugs/2586/): Bis: lualatex.exe fails if the profil name has non ASCII characters
* [2581](https://sourceforge.net/p/miktex/bugs/2581/): epstopdf creates .eps.pdf file instead of .pdf as in TeXLive
* [2580](https://sourceforge.net/p/miktex/bugs/2580/): wrong output to stdin when using lualatex
* [2579](https://sourceforge.net/p/miktex/bugs/2579/): xelatex adds unwanted stuff to stdin at the end of the compilation
* [2570](https://sourceforge.net/p/miktex/bugs/2570/): pdflatex doesn't ignore BOM (byte order mark)
