/*****
 * util.cc
 * John Bowman
 *
 * A place for useful utility functions.
 *****/

#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#ifdef __CYGWIN__
#define _POSIX_C_SOURCE 200809L
#endif

#include <cassert>
#include <iostream>
#include <cstdio>
#include <cfloat>
#include <sstream>
#include <cerrno>
#if !defined(MIKTEX_WINDOWS)
#include <sys/wait.h>
#include <sys/param.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <dirent.h>

#include "util.h"
#include "settings.h"
#include "errormsg.h"
#include "camperror.h"
#include "interact.h"
#include "locate.h"
#if defined(MIKTEX)
#include <miktex/Util/PathName>
#include <miktex/Core/Process>
#endif

using namespace settings;
using camp::reportError;

bool False=false;

namespace vm {
void error(const char* message);
}

#if __GNUC__
#include <cxxabi.h>
string demangle(const char *s)
{
  int status;
  char *demangled = abi::__cxa_demangle(s,NULL,NULL,&status);
  if (status == 0 && demangled) {
    string str(demangled);
    free(demangled);
    return str;
  } else if (status == -2) {
    free(demangled);
    return s;
  } else {
    free(demangled);
    return string("Unknown(") + s + ")";
  }
}
#else
string demangle(const char* s)
{
  return s;
}
#endif

char *Strdup(string s)
{
  size_t size=s.size()+1;
  char *dest=new(UseGC) char[size];
  std::memcpy(dest,s.c_str(),size*sizeof(char));
  return dest;
}

char *StrdupNoGC(string s)
{
  size_t size=s.size()+1;
  char *dest=new char[size];
  std::memcpy(dest,s.c_str(),size*sizeof(char));
  return dest;
}

char *StrdupMalloc(string s)
{
  size_t size=s.size()+1;
  char *dest=(char *) std::malloc(size);
  std::memcpy(dest,s.c_str(),size*sizeof(char));
  return dest;
}

string stripDir(string name)
{
#if defined(MIKTEX)
  return MiKTeX::Util::PathName(name.c_str()).GetFileName().GetData();
#else
  size_t p;
#ifdef __MSDOS__
  p=name.rfind('\\');
  if(p < string::npos) name.erase(0,p+1);
#endif
  p=name.rfind('/');
  if(p < string::npos) name.erase(0,p+1);
  return name;
#endif
}

string stripFile(string name)
{
#if defined(MIKTEX)
  return MiKTeX::Util::PathName(name.c_str()).GetDirectoryName().GetData();
#else
  size_t p;
  bool dir=false;
#ifdef __MSDOS__
  p=name.rfind('\\');
  if(p < string::npos) {
    dir=true;
    while(p > 0 && name[p-1] == '\\') --p;
    name.erase(p+1);
  }
#endif
  p=name.rfind('/');
  if(p < string::npos) {
    dir=true;
    while(p > 0 && name[p-1] == '/') --p;
    name.erase(p+1);
  }

  return dir ? name : "";
#endif
}

string stripExt(string name, const string& ext)
{
  string suffix="."+ext;
  size_t p=name.rfind(suffix);
  size_t n=suffix.length();
  if(n == 1 || p == name.length()-n)
    return name.substr(0,p);
  else return name;
}

void backslashToSlash(string& s)
{
  size_t p;
  while((p=s.find('\\')) < string::npos)
    s[p]='/';
}

void spaceToUnderscore(string& s)
{
  size_t p;
  while((p=s.find(' ')) < string::npos)
    s[p]='_';
}

string Getenv(const char *name, bool msdos)
{
  char *s=getenv(name);
  if(!s) return "";
  string S=string(s);
  if(msdos) backslashToSlash(S);
  return S;
}

void readDisabled()
{
  camp::reportError("Read from other directories disabled; override with option -globalread");
}

void writeDisabled()
{
  camp::reportError("Write to other directories disabled; override with option -globalwrite");
}

string cleanpath(string name)
{
  string dir=stripFile(name);
  name=stripDir(name);
  spaceToUnderscore(name);
  return dir+name;
}

string inpath(string name)
{
  bool global=globalread();
  string dir=stripFile(name);
  if(global && !dir.empty()) return name;
  string indir=stripFile(outname());
  if(!(global || dir.empty() || dir == indir)) readDisabled();
  return stripDir(name);
}

string outpath(string name)
{
  bool global=globalwrite();
  string dir=stripFile(name);
  if(global && !dir.empty()) return name;
  string outdir=stripFile(outname());
  if(!(global || dir.empty() || dir == outdir)) writeDisabled();
  return outdir+stripDir(name);
}

string buildname(string name, string suffix, string aux)
{
  name=stripExt(outpath(name),defaultformat())+aux;
  if(!suffix.empty()) name += "."+suffix;
  return name;
}

string auxname(string filename, string suffix)
{
  return buildname(filename,suffix,"_");
}

sighandler_t Signal(int signum, sighandler_t handler)
{
#if defined(MIKTEX_WINDOWS)
  // MIKTEX-TODO
  return signal(signum, handler);
#else
  struct sigaction action,oldaction;
  action.sa_handler=handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags=0;
  return sigaction(signum,&action,&oldaction) == 0 ? oldaction.sa_handler :
    SIG_ERR;
#endif
}

void push_split(mem::vector<string>& a, const string& S)
{
  const char *p=S.c_str();
  string s;
  char c;
  while((c=*(p++))) {
    if(c == ' ') {
      if(s.size() > 0) {
        a.push_back(s);
        s.clear();
      }
    } else s += c;
  }
  if(s.size() > 0)
    a.push_back(s);
}

char **args(const mem::vector<string>& s, bool quiet)
{
  size_t count=s.size();

  char **argv=NULL;
  argv=new char*[count+1];
  for(size_t i=0; i < count; ++i)
    argv[i]=StrdupNoGC(s[i]);

  if(!quiet && settings::verbose > 1) {
    cerr << argv[0];
    for(size_t i=1; i < count; ++i) cerr << " " << argv[i];
    cerr << endl;
  }

  argv[count]=NULL;
  return argv;
}

void execError(const char *command, const char *hint, const char *application)
{
  cerr << "Cannot execute " << command << endl;
  if(*application == 0) application=hint;
  if(hint) {
    string s=string(hint);
    transform(s.begin(), s.end(), s.begin(), toupper);
    cerr << "Please put in a file " << getSetting<string>("config")
         << ": " << endl << endl
         << "import settings;" << endl
         << hint << "=\"LOCATION\";" << endl << endl
         << "where LOCATION specifies the location of "
         << application << "." << endl << endl
         << "Alternatively, set the environment variable ASYMPTOTE_" << s
         << endl << "or use the command line option -" << hint
         << "=\"LOCATION\". For further details, see" << endl
         << "https://asymptote.sourceforge.io/doc/Configuring.html" << endl
         << "https://asymptote.sourceforge.io/doc/Search-paths.html" << endl;
  }
}

// quiet: 0=none; 1=suppress stdout; 2=suppress stdout+stderr.
int System(const mem::vector<string> &command, int quiet, bool wait,
           const char *hint, const char *application, int *ppid)
{
#if defined(MIKTEX_WINDOWS)
  MiKTeX::Core::ProcessStartInfo startInfo;
  startInfo.FileName = command[0];
  startInfo.Arguments = command;
  if (quiet > 0)
  {
    startInfo.RedirectStandardOutput = true;
  }
  if (quiet > 1)
  {
    startInfo.RedirectStandardError = true;
  }
  std::unique_ptr<MiKTeX::Core::Process> process = MiKTeX::Core::Process::Start(startInfo);
  if (ppid != nullptr)
  {
    *ppid = process->GetSystemId();
  }
  if (!wait)
  {
    return 0;
  }
  process->WaitForExit();
  return process->get_ExitCode();
#else
  int status;

  cout.flush(); // Flush stdout to avoid duplicate output.

  char **argv=args(command);

  int pid=fork();
  if(pid == -1)
    camp::reportError("Cannot fork process");

  if(pid == 0) {
    if(interact::interactive) signal(SIGINT,SIG_IGN);
    if(quiet) {
      static int null=creat("/dev/null",O_WRONLY);
      close(STDOUT_FILENO);
      dup2(null,STDOUT_FILENO);
      if(quiet == 2) {
        close(STDERR_FILENO);
        dup2(null,STDERR_FILENO);
      }
    }
    if(argv) {
      execvp(argv[0],argv);
      execError(argv[0],hint,application);
      _exit(-1);
    }
  }

  if(ppid) *ppid=pid;
  for(;;) {
    if(waitpid(pid, &status, wait ? 0 : WNOHANG) == -1) {
      if(errno == ECHILD) return 0;
      if(errno != EINTR) {
        if(quiet < 2) {
          ostringstream msg;
          msg << "Command failed: ";
          for(size_t i=0; i < command.size(); ++i) msg << command[i] << " ";
          camp::reportError(msg);
        }
      }
    } else {
      if(!wait) return 0;
      if(WIFEXITED(status)) {
        if(argv) {
          char **p=argv;
          char *s;
          while((s=*(p++)) != NULL)
            delete [] s;
          delete [] argv;
        }
        return WEXITSTATUS(status);
      } else {
        if(quiet < 2) {
          ostringstream msg;
          msg << "Command exited abnormally: ";
          for(size_t i=0; i < command.size(); ++i) msg << command[i] << " ";
          camp::reportError(msg);
        }
      }
    }
  }
#endif
}

string stripblanklines(const string& s)
{
  string S=string(s);
  bool blank=true;
  const char *t=S.c_str();
  size_t len=S.length();

  for(size_t i=0; i < len; i++) {
    if(t[i] == '\n') {
      if(blank) S[i]=' ';
      else blank=true;
    } else if(t[i] != '\t' && t[i] != ' ') blank=false;
  }
  return S;
}

char *startpath=NULL;

void noPath()
{
  camp::reportError("Cannot get current path");
}

char *getPath(char *p)
{
#ifdef MAXPATHLEN
  static size_t size = MAXPATHLEN;
#else
  static size_t size = 1024;
#endif
  if(!p) p=new(UseGC) char[size];
  if(!p) noPath();
  else while(getcwd(p,size) == NULL) {
      if(errno == ERANGE) {
        size *= 2;
        p=new(UseGC) char[size];
      } else {noPath(); p=NULL;}
    }
  return p;
}

const char *setPath(const char *s, bool quiet)
{
  if(startpath == NULL) startpath=getPath(startpath);
  if(s == NULL || *s == 0) s=startpath;
  int rc=chdir(s);
  if(rc != 0) {
    ostringstream buf;
    buf << "Cannot change to directory '" << s << "'";
    camp::reportError(buf);
  }
  char *p=getPath();
  if(p && (!interact::interactive || quiet) && verbose > 1)
    cout << "cd " << p << endl;
  return p;
}

void push_command(mem::vector<string>& a, const string& s)
{
  a.push_back(s);
#if defined(MIKTEX_WINDOWS) && !defined(__MSDOS__)
  if (s == "cmd")
  {
    a.push_back("/c");
    a.push_back("start");
    a.push_back("\"\"");
  }
#endif
#ifdef __MSDOS__
  if(s == "cmd") {
    a.push_back("/c");
    a.push_back("start");
    a.push_back("\"\"");
  }
#endif
}

void popupHelp() {
  // If the popped-up help is already running, pid stores the pid of the viewer.
  static int pid=0;

  // Status is ignored.
  static int status=0;

  // If the help viewer isn't running (or its last run has termined), launch the
  // viewer again.
#if defined(MIKTEX_WINDOWS)
  // MIKTEX-TODO
  if (pid == 0) {
#else
  if (pid==0 || (waitpid(pid, &status, WNOHANG) == pid)) {
#endif
    mem::vector<string> cmd;
    push_command(cmd,getSetting<string>("pdfviewer"));
    string viewerOptions=getSetting<string>("pdfviewerOptions");
    if(!viewerOptions.empty())
      cmd.push_back(viewerOptions);
    cmd.push_back(docdir+dirsep+"asymptote.pdf");
    status=System(cmd,0,false,"pdfviewer","your PDF viewer",&pid);
  }
}

const char *intrange="integer argument is outside valid range";
const char *uintrange="integer argument is outside valid unsigned range";

unsigned unsignedcast(Int n)
{
  if(n < 0 || n/2 > INT_MAX)
    vm::error(uintrange);
  return (unsigned) n;
}

unsignedInt unsignedIntcast(Int n)
{
  if(n < 0)
    vm::error(uintrange);
  return (unsignedInt) n;
}

int intcast(Int n)
{
  if(Abs(n) > INT_MAX)
    vm::error(intrange);
  return (int) n;
}

Int Intcast(unsignedInt n)
{
  if(n > (unsignedInt) Int_MAX)
    vm::error(intrange);
  return (Int) n;
}
