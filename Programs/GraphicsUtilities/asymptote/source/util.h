/*****
 * util.h
 * Andy Hammerlindl 2004/05/10
 *
 * A place for useful utility functions.
 *****/

#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include <iostream>
#include <cstdlib>
#include <csignal>

#include "common.h"
#include <unordered_set>

#if !defined(_MSC_VER)
#include <strings.h>
#else
#include <cstring>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

// Demangle a typeid name (if the proper library is installed.
string demangle(const char *s);

// Duplicate a string.
char *Strdup(string s);
char *StrdupNoGC(string s);
char *StrdupMalloc(string s);

// Strip the directory from a filename.
string stripDir(string name);

// Strip the file from a filename, returning the directory.
string stripFile(string name);

// Strip the extension from a filename.
string stripExt(string name, const string& suffix="");

// Escapes characters specified in set
string escapeCharacters(string const& inText, std::unordered_set<char> const& charactersToEscape);

void readDisabled();
void writeDisabled();

// Replace spaces in file part of name with underscores.
string cleanpath(string name);

// Construct the full path name, checking access.
string inpath(string name);
string outpath(string name);

// Construct a filename from the original, adding aux at the end, and
// changing the suffix.
string buildname(string filename, string suffix="", string aux="");

// Construct an alternate filename for a temporary file in the current
// directory.
string auxname(string filename, string suffix="");

// Cast argument to a string.
template<class T>
string String(T x)
{
  ostringstream buf;
  buf << x;
  return buf.str();
}

typedef void (*sighandler_t)(int);

// Portable signal (sigaction wrapper).
sighandler_t Signal(int signum, sighandler_t handler);

// Split string S and push the pieces onto vector a.
void push_split(mem::vector<string>& a, const string& S);

// Wrapper to append /c start "" to MSDOS cmd.
void push_command(mem::vector<string>& a, const string& s);

// Return an argv array corresponding to the fields in command delimited
// by spaces not within matching single quotes.
char **args(const mem::vector<string> &args, bool quiet=false);

// Similar to the standard system call except allows interrupts and does
// not invoke a shell.
int System(const mem::vector<string> &command, int quiet=0, bool wait=true,
           const char *hint=NULL, const char *application="",
           int *pid=NULL);

extern bool False;

// Strip blank lines (which would break the bidirectional TeX pipe)
string stripblanklines(const string& s);

const char *startPath();
const char* setPath(const char *s, bool quiet=false);
const char *changeDirectory(const char *s);
extern char *startpath;
#if !defined(MIKTEX)
extern void recursive_delete(char *name);
#endif

void backslashToSlash(string& s);
void spaceToUnderscore(string& s);
string Getenv(const char *name, bool msdos);
char *getPath(char *p=NULL);

void execError(const char *command, const char *hint, const char *application);

// This invokes a viewer to display the manual.  Subsequent calls will only
// pop-up a new viewer if the old one has been closed.
void popupHelp();

inline Int Abs(Int x) {
#ifdef HAVE_LONG_LONG
  return llabs(x);
#else
#ifdef HAVE_LONG
  return labs(x);
#else
  return abs(x);
#endif
#endif
}

unsigned unsignedcast(Int n);
unsignedInt unsignedIntcast(Int n);
int intcast(Int n);
Int Intcast(unsignedInt n);

bool fileExists(string const& path);

#if defined(_WIN32)
int setenv(const char *name, const char *value, bool overwrite);
int unsetenv(const char *name);
#endif

#endif
