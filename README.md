# MiKTeX

MiKTeX is both a modern C/C++ implemenentation of TeX & Friends and a
TeX distribution.

The [MiKTeX Project Page](https://miktex.org) is the place to go, if
you want to learn more about MiKTeX.

## What you will find here

This readme file is located in the top-level directory of the MiKTeX
source code tree.

The MiKTeX source code allows you to build and install a bare TeX
system (aka "Just enough TeX").  MiKTeX provides an integrated package
manager, which can automatically download and install missing
packages.  Thus:

* you start with MiKTeX executables and man pages
* in the course of authoring your documents: only necessary packages
  will be downloaded and installed
  
MiKTeX can be built on Windows and Unix-like (including macOS)
systems.  Please consult these HOWTOs for platform-specific build
instructions:

* [https://miktex.org/howto/build-win](https://miktex.org/howto/build-win "Building MiKTeX (Windows)")
* [https://miktex.org/howto/build-unx](https://miktex.org/howto/build-unx "Building MiKTeX (Unix-line)")
* [https://miktex.org/howto/build-mac](https://miktex.org/howto/build-mac "Building MiKTeX (macOS)")

In addition, you can try one of the Dockerized build environments to build MiKTeX:

* [Ubuntu 16.04](https://github.com/MiKTeX/docker-miktex-build-xenial)
* [Debian 9](https://github.com/MiKTeX/docker-miktex-build-stretch)
* [Fedora 25](https://github.com/MiKTeX/docker-miktex-build-fedora-25)

## Prerequisites

* C/C++ compiler

  Building MiKTeX requires a modern C/C++ compiler.  The minimum
  required standard versions are C99 and C++14.

* CMake

  The MiKTeX build is driven by CMake, a tool which will create
  Makefiles suited for the local system.  The minimum required CMake
  version is 3.7.0.

* Various build utilities

  In addition to the compiler and the CMake build system, you will need these utilities:

  All platforms:

  * cat
  * sed
  * xsltproc

  Windows only:

  * fop
  * hhc
  * makehm
  * mc
  * midl
  * mt
  * pandoc

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

Please consult the platform-specific HOWTO, for more information.

## Running CMake

It is recommended that you build outside the source code
directory:

    cmake ../source

Here you have to specify the path to the MiKTeX source code directory
(`../source` in the example above).

CMake for Windows may require a generator specification.  If you want
to generate Makefiles for `nmake`, run this:

    cmake -G "NMake Makefiles" ../source

### Useful build variables

When running CMake to generate the Makefiles, it is possible to set
build variables as follows:

    cmake -DVAR1=VAL1 -DVAR2=VAL2 ...

The most useful build variables are:

* `CMAKE_INSTALL_PREFIX=`_PATH_

  The installation directory used by `make install` (see below).  This
  variable defaults to `/usr/local` on Unix-like systems (including
  macOS).

  Suitable prefixes are:

  * `$HOME/miktex`

    Use this prefix, if you want to install MiKTeX just for yourself.
    No administrative privileges are required.

  * `/opt/miktex`

    Use this prefix, if you want to create a self-contained MiKTeX
    setup which does not conflict with other system packages.

* `WITH_UI_QT=ON`

  Build UI applications (experimental).

## Building MiKTeX

Run the Make utility to build MiKTeX, for example:

    make

or, if you build with NMake on Windows:

    nmake

## Installing

CMake has created a standard `install` target which you can use to
install everything in the standard location (see the
`CMAKE_INSTALL_PREFIX` description above).  On Linux and macOS, you
would run

    make install

This will install MiKTeX in the `/usr/local`-prefixed directory
tree.

### Symbolic links

Most of the MiKTeX executables are prefixed with `miktex-`.  For
example, the file name of the pdfTeX executable is `miktex-pdftex`.
You can create symbolic links targetting the `miktex-` prefixed
executables:

    initexmf --admin --mklinks

After running this command, the pdfTeX engine can be invoked as
`pdftex`, provided that `pdftex` did not exist before (use the
`--force` option to overwrite existing files).

The `--mklinks` option will also create symbolic links for format
files and scripts.  For example, you can invoke `pdflatex` (pdfTeX
with format `pdflatex` loaded) and `latexmk` (wrapper for the Perl
script `latexmk.pl`).

### Relocating the installation

On Unix-like platforms, you can use the `DESTDIR` mechanism in order
to relocate the whole installation:

    make DESTDIR=/home/jane install

This will install everything using the installation prefix
(`CMAKE_INSTALL_PREFIX`) prepended with the `DESTDIR` value, which
finally gives `/home/jane/usr/local`.

The `DESTDIR` mechanism is helpful if you want to understand what
`make install` installs where.

## First steps

### Installing packages

You are now at a point where you have the MiKTeX executables, some
configuration files and man pages.

It is recommended that you install some packages now:

    mpm --admin --install=amsfonts --install=ltxbase

Package `amsfonts` contains Type1 fonts for the traditional TeX fonts
(aka "Computer Modern").  Package `ltxbase` contains the LaTeX
document preparation system.

If disk space is not an issue, you can "upgrade" your MiKTeX
installation.  This is usually not recommended when you use MiKTeX
just for yourself.  But it might make sense if you are maintaining a
system-wide MiKTeX installation.

To upgrade to a basic MiKTeX installation, run

    mpm --admin --verbose --package-level=basic --upgrade

In order to make newly installed font map files available for the
various TeX engines, you must run

    initexmf --admin --mkmaps

### Finalizing

These are the final steps: update the file name database again:

    initexmf --admin --update-fndb

If you used the `--admin` option to set up a shared MiKTeX
installation, remove the user `.miktex` directory:

    rm -fr ~/.miktex

## Testing

### Running the pdfTeX engine for the first time

You can now test the MiKTeX setup by running

    miktex-pdflatex sample2e

or, if you want to omit the `miktex-` prefix:

    pdflatex sample2e

This will create the PDF file `sample.pdf`.

### Test suite

In order to validate the MiKTeX setup, you can run the [MiKTeX test
suite](https://github.com/MiKTeX/miktex-testing "MiKTeX test suite on
GitHub").

## Uninstalling

If you want to completely wipe out MiKTeX, you have to

* remove symbolic links
* undo the effect of `make install`
* remove MiKTeX directories

Before you proceed, you should run `initexmf --report` in order to
find the locations of the MiKTeX directories.

### Removing symbolic links

Run

    initexmf --admin --force --remove-links

to remove the symbolic links created by `initexmf --mklinks` (see
above).

### Uninstalling

There is an `uninstall` target which allows you to remove all files
installed by `make install` (see above):

    make uninstall

### Removing MiKTeX directories

There are a couple of MiKTeX directories which have to be removed
manually.  For example:

    rm -fr ~/.miktex
    rm -fr /var/lib/miktex-texmf
    rm -fr /var/cache/miktex-texmf
    rm -fr /usr/local/share/miktex-texmf

Please inspect the output of `initexmf --report` in order to find out
the exact locations.
