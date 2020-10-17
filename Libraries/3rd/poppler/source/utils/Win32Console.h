//========================================================================
//
// Win32Console.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2019 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef WIN32CONSOLE_H
#define WIN32CONSOLE_H

// UTF-8 Support for win32 console
//
// Converts argc/argv to UTF-8. Supports UTF-8 stdout/stderr to win32 console.
// On other platforms this class is a no-op.

#if !defined(MIKTEX) && defined(_WIN32)

// Ensure stdio.h is included before redefining stdio functions. We need to provide
// our own declarations for the redefined functions because win32 stdio.h functions
// have DLL export decorations.
#    include <cstdio>

#    ifndef WIN32_CONSOLE_IMPL // don't redefine in Win32Console.cc so we can call original functions
#        define printf(...) win32_fprintf(stdout, __VA_ARGS__)
#        define fprintf(stream, ...) win32_fprintf(stream, __VA_ARGS__)
#        define puts(s) win32_fprintf(stdout, "%s\n", s)
#        define fputs(s, stream) win32_fprintf(stream, "%s", s)
#        define putc(c) win32_fprintf(stdout, "%c", c)
#        define putchar(c) win32_fprintf(stdout, "%c", c)
#        define fputc(c, stream) win32_fprintf(stream, "%c", c)
#        define fwrite(ptr, size, nmemb, stream) win32_fwrite(ptr, size, nmemb, stream)
#    endif

extern "C" {
int win32_fprintf(FILE *stream, ...);
size_t win32_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
}

class Win32Console
{
public:
    Win32Console(int *argc, char **argv[]);
    ~Win32Console();

private:
    int numArgs;
    char **argList;
    char **privateArgList;
};

#else

// On other platforms this class is a no-op.

class Win32Console
{
public:
    Win32Console(int *argc, char ***argv) { }
};

#endif // _WIN32

#endif
