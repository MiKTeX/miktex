//========================================================================
//
// Win32Console.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifdef _WIN32

#    include "goo/gmem.h"
#    include "UTF.h"

#    define WIN32_CONSOLE_IMPL
#    include "Win32Console.h"

#    include <windows.h>
#    include <shellapi.h>

static const int BUF_SIZE = 4096;
static int bufLen = 0;
static char buf[BUF_SIZE];
static wchar_t wbuf[BUF_SIZE];
static bool stdoutIsConsole = true;
static bool stderrIsConsole = true;
static HANDLE consoleHandle = nullptr;

// If all = true, flush all characters to console.
// If all = false, flush up to and including last newline.
// Also flush all if buffer > half full to ensure space for future
// writes.
static void flush(bool all = false)
{
    int nchars = 0;

    if (all || bufLen > BUF_SIZE / 2) {
        nchars = bufLen;
    } else if (bufLen > 0) {
        // find num chars up to and including last '\n'
        for (nchars = bufLen; nchars > 0; --nchars) {
            if (buf[nchars - 1] == '\n')
                break;
        }
    }

    if (nchars > 0) {
        DWORD wlen = utf8ToUtf16(buf, (uint16_t *)wbuf, BUF_SIZE, nchars);
        WriteConsoleW(consoleHandle, wbuf, wlen, &wlen, nullptr);
        if (nchars < bufLen) {
            memmove(buf, buf + nchars, bufLen - nchars);
            bufLen -= nchars;
        } else {
            bufLen = 0;
        }
    }
}

static inline bool streamIsConsole(FILE *stream)
{
    return ((stream == stdout && stdoutIsConsole) || (stream == stderr && stderrIsConsole));
}

int win32_fprintf(FILE *stream, ...)
{
    va_list args;
    int ret = 0;

    va_start(args, stream);
    const char *format = va_arg(args, const char *);
    if (streamIsConsole(stream)) {
        ret = vsnprintf(buf + bufLen, BUF_SIZE - bufLen, format, args);
        bufLen += ret;
        if (ret >= BUF_SIZE - bufLen) {
            // output was truncated
            buf[BUF_SIZE - 1] = 0;
            bufLen = BUF_SIZE - 1;
        }
        flush();
    } else {
        vfprintf(stream, format, args);
    }
    va_end(args);

    return ret;
}

size_t win32_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t ret = 0;

    if (streamIsConsole(stream)) {
        int n = size * nmemb;
        if (n > BUF_SIZE - bufLen - 1)
            n = BUF_SIZE - bufLen - 1;
        memcpy(buf + bufLen, ptr, n);
        bufLen += n;
        buf[bufLen] = 0;
        flush();
    } else {
        ret = fwrite(ptr, size, nmemb, stream);
    }

    return ret;
}

Win32Console::Win32Console(int *argc, char **argv[])
{
    LPWSTR *wargv;
    fpos_t pos;

    argList = nullptr;
    privateArgList = nullptr;
    wargv = CommandLineToArgvW(GetCommandLineW(), &numArgs);
    if (wargv) {
        argList = new char *[numArgs];
        privateArgList = new char *[numArgs];
        for (int i = 0; i < numArgs; i++) {
            argList[i] = utf16ToUtf8((uint16_t *)(wargv[i]));
            // parseArgs will rearrange the argv list so we keep our own copy
            // to use for freeing all the strings
            privateArgList[i] = argList[i];
        }
        LocalFree(wargv);
        *argc = numArgs;
        *argv = argList;
    }

    bufLen = 0;
    buf[0] = 0;
    wbuf[0] = 0;

    // check if stdout or stderr redirected
    // GetFileType() returns CHAR for console and special devices COMx, PRN, CON, NUL etc
    // fgetpos() succeeds on all CHAR devices except console and CON.

    stdoutIsConsole = (GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_CHAR) && (fgetpos(stdout, &pos) != 0);

    stderrIsConsole = (GetFileType(GetStdHandle(STD_ERROR_HANDLE)) == FILE_TYPE_CHAR) && (fgetpos(stderr, &pos) != 0);

    // Need a handle to the console. Doesn't matter if we use stdout or stderr as
    // long as the handle output is to the console.
    if (stdoutIsConsole)
        consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    else if (stderrIsConsole)
        consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
}

Win32Console::~Win32Console()
{
    flush(true);
    if (argList) {
        for (int i = 0; i < numArgs; i++)
            gfree(privateArgList[i]);
        delete[] argList;
        delete[] privateArgList;
    }
}

#endif // _WIN32
