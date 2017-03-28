# MiKTeX

MiKTeX is both a C/C++ implemenentation of TeX & Friends and a TeX
distribution.

The [MiKTeX Project Page](https://miktex.org) is the place to go, if
you want to learn more about MiKTeX.

## What you will find here

This readme file is located in the top-level directory of the MiKTeX
source code tree.

Use the source code to build and install a minimal TeX system.  MiKTeX
provides an integrated package manager, i.e., the minimal TeX system
can serve as a starting point for a much larger (complete) TeX system.

Please consult these HOWTOs for platform-specific information:

* [https://miktex.org/howto/build-win](https://miktex.org/howto/build-win "Building MiKTeX (Windows)")
* [https://miktex.org/howto/build-unx](https://miktex.org/howto/build-unx "Building MiKTeX (Unix-line)")
* [https://miktex.org/howto/build-mac](https://miktex.org/howto/build-mac "Building MiKTeX (macOS)")

## Prerequisites

* C/C++ compiler

  You will need a modern C/C++ compiler.  The minimum required
  standard versions are C99 and C++14.

* CMake

  MiKTeX is built by using CMake, which will create Makefiles suited
  for your local system.  The minimum required CMake version is 3.7.0.
  
* Various build utilities

  In addition to the compiler and the CMake build system, you will need these utilities:

  All platforms:
  
  * cat, sed
  * xsltproc
  
  Windows only:
  
  * hhc
  * makehm
  * pandoc
  * mc
  * midl
  * mt
  
* Libraries

  On Unix-like platforms (including macOS), you will need development
  packages for these libraries:

  * apr
  * aprutil
  * bzip2
  * cairo
  * expat
  * fontconfig
  * freetype2
  * fribidi
  * gd
  * gmp
  * graphite2
  * harfbuzz-icu
  * hunspell
  * icu
  * jpeg
  * log4cxx
  * lzma
  * mpfr
  * mspack
  * LibreSSL (or OpenSSL)
  * pixman
  * png
  * poppler
  * popt
  * potrace
  * uriparser
  * zzip

  When building UI components with Qt5:
  
  * poppler-qt5

## Running CMake

It is recommended that you build outside the source code
directory:

    cmake ../source

If you are doing an out-of-source build, then you have to specify the
path to the MiKTeX source code directory (`../source` in the example
above).

CMake for Windows may require a generator specification.  If you want
to generate Makefiles for `nmake`, run this:

    cmake -G "NMake Makefiles" ../source
	
### Useful build variables

When running CMake to generates the Makefiles, it is possible to set
build variables as follows:

    cmake -DVAR1=VAL1 -DVAR2=VAL2 ...

The most useful build variables are:

* CMAKE\_INSTALL\_PREFIX=_PATH_
  The installation directory used by
  `make install` (see below).  This variable defaults to `/usr/local`
  on Unix-like systems.

* WITH\_UI\_QT=ON
  Build UI applications (experimental).
	
## Building MiKTeX

Run the Make utility to build MiKTeX, for example:

    make

or, if you build with NMake on Windows:

    nmake

## Installing

CMake creates a standard `install` target which you can use to install
everything in the standard location (see the `CMAKE\_INSTALL\_PREFIX`
description above).  On Linux and macOS, you would run

    make install

This will install MiKTeX in the `/usr/local`-prefixed directory
tree.

### Relocating the installation

On Unix-like platforms, you can use the `DESTDIR` mechanism in order
to relocate the whole installation:

    make DESTDIR=/home/jane install
   
This will install everything using the installation prefix
(`CMAKE\_INSTALL\_PREFIX`) prepended with the `DESTDIR` value, which
finally gives `/home/jane/usr/local`.

## First steps

### Initialize the package database

MiKTeX has the ability to install missing packages "on-the-fly".  The
package database must have been set up for this to work:

    mpm --admin --update-db
	
The `--admin` option is only required, if you are building a shared
MiKTeX setup.  If you are setting up MiKTeX just for yourself (i.e.,
in your user directory), you have to ommit the `--admin` option.

Consult the man page mpm(1), for more information about the utility.

### Enabling "on-the-fly" package installation

In order to enable the automatic package installer, run the MiKTeX
Configuration Utility as follows:

    initexmf --admin --set-config-value [MPM]AutoInstall=1
	
Again, you have to omit `--admin`, if you intend to use MiKTeX just
for yourself.

Consult the man page and initexmf(1), for more information about the
utility.
   
### Symbolic links

Most of the MiKTeX executables are prefixed with `miktex-`.  For
example, the pdfTeX executable has the file name `miktex-pdftex`.  You
can create symbolic links targetting the `miktex-` prefixed
executables:

    initexmf --admin --mklinks

After running this command, the pdfTeX engine can be invoked as
`pdftex`.

The `--mklinks` option will also create symbolic links for format
files and scripts.  That is, you can run `pdflatex` (pdfTeX with
format `pdflatex` loaded) and `latexmk` (wrapper for the Perl script
`latexmk.pl`).

### Installing basic packages

You use the MiKTeX Package Manager in order to install basic MiKTeX
packages:

    mpm --admin --upgrade=basic

### Making font map files

Run this, to create font map files required by pdfTeX, LuaTeX and XeTeX:
    
	initexmf --admin --mkmaps
   
### Running the pdfTeX engine for the first time

You can now test the MiKTeX setup by running

    miktex-pdflatex sample2e

or, if you want to omit the `miktex-` prefix:

    pdflatex sample2e

This will create the PDF file `sample.pdf`.
