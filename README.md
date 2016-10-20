# MiKTeX

MiKTeX is:

* a C/C++ implemenentation of TeX & Friends
* a TeX distribution

More information: please visit the [MiKTeX project page](http://miktex.org)

## What you will find here

This README file is located in the top-level directory of the the
MiKTeX source code tree. You use this source code to build and install
a minimal TeX system. MiKTeX provides an integrated package manager,
i.e., this mininal TeX system can serve as a starting point for a much
larger (complete) TeX system.

## Prerequisites

* C++ compiler

  You will need a modern C/C++ compiler.  The minimum required
  standard version is C++14.

* CMake

  MiKTeX is built by using CMake. CMake will create Makefiles suited
  for your local system.
  
* Various build utilities

  You will need utilities like Bison, Flex, etc. Please see the
  top-level CMakeLists.txt for more information.

* Libraries

  On Unix-alike platforms, you will need development packages (header
  files) for a handful of popular libraries. Please see the top-level
  CMakeLists.txt for more information.

## Running CMake

It is recommended that you build outside the source code
directory. For a server (non-UI) build, run this:

    cmake -DWITH_UI_QT=OFF ../source

If you are doing an out-of-source build, then you have to specify the
path to the MiKTeX source code directory (`../source` in the example
above).

CMake for Windows may require a generator specification. If you want
to generate Makefiles for nmake, run this:

    cmake -G "NMake Makefiles" ../source

## Building MiKTeX

Run the make utility to build MiKTeX, for example:

    make

or

    nmake
	
## Installing

CMake creates a standard `install` target which you can use to install
everything in the standard location. On Linux, you run

    sudo make install
	
This will install MiKTeX in the `/usr/local`-prefixed directory
tree.
