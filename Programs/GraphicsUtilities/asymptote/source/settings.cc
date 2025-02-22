/*****
 * settings.cc
 * Andy Hammerlindl 2004/05/10
 *
 * Declares a list of global variables that act as settings in the system.
 *****/

#if defined(MIKTEX)
#include <miktex/ExitThrows>
#endif
#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cerrno>
#include <sys/stat.h>
#include <cfloat>
#include <clocale>
#include <algorithm>

#if defined(_WIN32)
#include <Windows.h>
#include <io.h>
#define isatty _isatty

#include "win32helpers.h"
#else
#include <unistd.h>
#endif

#include "common.h"

#if HAVE_GNU_GETOPT_H
#include <getopt.h>
#else
#include "getopt.h"
#endif

#include "util.h"
#include "settings.h"
#include "interact.h"
#include "locate.h"
#include "lexical.h"
#include "record.h"
#include "env.h"
#include "item.h"
#include "refaccess.h"
#include "pipestream.h"
#include "array.h"

#include "glrender.h"
#if defined(MIKTEX)
#include <miktex/Util/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#endif

#ifdef HAVE_LIBCURSES
extern "C" {

#ifdef HAVE_NCURSES_CURSES_H
#define USE_SETUPTERM
#include <ncurses/curses.h>
#include <ncurses/term.h>
#elif HAVE_NCURSES_H
#define USE_SETUPTERM
#include <ncurses.h>
#include <term.h>
#elif HAVE_CURSES_H
#include <curses.h>

#if defined(HAVE_TERM_H)
#define USE_SETUPTERM
#include <term.h>
#endif

#endif
}
#endif

// Workaround broken curses.h files:
#ifdef clear
#undef clear
#endif
// Workaround broken header file on i386-solaris with g++ 3.4.3.
#ifdef erase
#undef erase
#endif

using vm::item;

using trans::itemRefAccess;
using trans::refAccess;
using trans::varEntry;
using vm::array;

void runFile(const string& filename);

namespace settings {

using camp::pair;

#ifdef HAVE_LIBGLM
const bool havegl=true;
#else
const bool havegl=false;
#endif

#if !defined(_WIN32)
mode_t mask;
#endif

#if defined(MIKTEX)
string systemDir;
#else
string systemDir=ASYMPTOTE_SYSDIR;
#endif
string defaultPSdriver="ps2write";
string defaultEPSdriver="eps2write";
string defaultPNGdriver="png16m"; // pngalpha has issues at high resolutions
string defaultAsyGL="https://vectorgraphics.github.io/asymptote/base/webgl/asygl-"+
  string(AsyGLVersion)+".js";

#if !defined(_WIN32)

bool msdos=false;
string HOME="HOME";
#if defined(MIKTEX)
string docdir;
#else
string docdir=ASYMPTOTE_DOCDIR;
#endif
const char pathSeparator=':';
#ifdef __APPLE__
string defaultPSViewer="open";
string defaultPDFViewer="open";
string defaultHTMLViewer="open";
#else
string defaultPSViewer="evince";
string defaultPDFViewer="evince";
string defaultHTMLViewer="google-chrome";
#endif
string defaultGhostscript="gs";
string defaultGhostscriptLibrary="";
string defaultDisplay="display";
string defaultAnimate="magick";
void queryRegistry() {}
const string dirsep="/";

#else

bool msdos=true;
string HOME="USERPROFILE";
string docdir="c:\\Program Files\\Asymptote";
const char pathSeparator=';';
string defaultPSViewer;
//string defaultPDFViewer="AcroRd32.exe";
string defaultPDFViewer;
string defaultHTMLViewer;
string defaultGhostscript;
string defaultGhostscriptLibrary;
string defaultDisplay;
//string defaultAnimate="magick";
string defaultAnimate="";
const string dirsep="\\";

/**
 * Use key to look up an entry in the MSWindows registry,
 * @param baseRegLocation base location for a key
 * @param key Key to look up, respecting wild cards
 * @remark Wildcards can only be in keys, not in the final value
 * @return Entry value, or an empty string if not found
 */
string getEntry(const HKEY& baseRegLocation, const string& key)
{
  string path=key;
  size_t star;
  string head;
  while((star=path.find('*')) < string::npos)
  {
    // has asterisk in the path

    string prefix=path.substr(0,star);
    string pathSuffix=path.substr(star+1);
    // path = prefix [*] pathSuffix
    size_t slash=pathSuffix.find('\\');
    if(slash < string::npos) {
      // pathsuffix is not leaf yet
      // pathSuffix = <new path suffix>[/<new path>]
      path=pathSuffix.substr(slash);
      pathSuffix=pathSuffix.substr(0,slash);
    } else {
      // pathSuffix is entirely part of registry value name
      // pathSuffix = <new path>, new path suffix is empty
      path=pathSuffix;
      pathSuffix="";
    }
    string directory=head+stripFile(prefix);
    string file=stripDir(prefix);
    // prefix = directory/file
    // [old path] = directory/file [*] pathSuffix [/path]
    // or, if asterisk is in the leaf
    // [old path] = directory/file[*]path, pathSuffix is empty

    camp::w32::RegKeyWrapper currRegDirectory;
    if (RegOpenKeyExA(baseRegLocation, directory.c_str(), 0, KEY_READ, currRegDirectory.put()) != ERROR_SUCCESS)
    {
      currRegDirectory.release();
      return "";
    }

    DWORD numSubKeys;
    DWORD longestSubkeySize;

    if (RegQueryInfoKeyA(
                currRegDirectory.getKey(),
                nullptr, nullptr, nullptr,
                &numSubKeys, &longestSubkeySize,
                nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
    {
      numSubKeys = 0;
      longestSubkeySize = 0;
    }
    mem::vector<char> subkeyBuffer(longestSubkeySize + 1);
    bool found=false;

    string rsuffix= pathSuffix;
    reverse(rsuffix.begin(), rsuffix.end());
    for (DWORD i=0;i<numSubKeys;++i)
    {
      DWORD cchValue=longestSubkeySize + 1;

      if (auto const regQueryResult= RegEnumKeyExA(
                  currRegDirectory.getKey(),
                  i,
                  subkeyBuffer.data(),
                  &cchValue,
                  nullptr,
                  nullptr,
                  nullptr,
                  nullptr
          );
          regQueryResult != ERROR_SUCCESS)
      {
        continue;
      }

      string const dname(subkeyBuffer.data());
      string rdname=dname;
      reverse(rdname.begin(),rdname.end());
      if(dname.substr(0,file.size()) == file &&
         rdname.substr(0, pathSuffix.size()) == rsuffix) {
        // dname matches file [*} pathSuffix
        head=directory+dname;
        found=true;
        break;
      }
    }

    if (!found)
    {
      return "";
    }
  }

  if (path.find('\\') == 0)
  {
    path = path.substr(1); // strip the prefix separator
  }

  if (path == "@")
  {
    path= "";// default registry key
  }

  DWORD requiredStrSize=0;
  // FIXME: Add handling of additional types
  if (RegGetValueA(baseRegLocation, head.c_str(), path.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &requiredStrSize) !=
    ERROR_SUCCESS)
  {
    return "";
  }

  mem::vector<BYTE> outputBuffer(requiredStrSize);

  if (auto const retCheck = RegGetValueA(baseRegLocation, head.c_str(), path.c_str(),
              RRF_RT_REG_SZ,
              nullptr,
              outputBuffer.data(),
              &requiredStrSize
      );
      retCheck != ERROR_SUCCESS)
  {
    return "";
  }

  return reinterpret_cast<char const*>(outputBuffer.data());
}

// Use key to look up an entry in the MSWindows registry, respecting wild cards
string getEntry(const string& key)
{
  for (HKEY const keyToSearch : { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER })
  {
    camp::w32::RegKeyWrapper baseRegKey;
    if (RegOpenKeyExA(keyToSearch, "Software", 0, KEY_READ, baseRegKey.put()) != ERROR_SUCCESS)
    {
      baseRegKey.release();
      continue;
    }

    if (string entry=getEntry(baseRegKey.getKey(),key); !entry.empty())
    {
      return entry;
    }
  }

  return "";
}

void queryRegistry()
{
  defaultGhostscriptLibrary= getEntry(R"(GPL Ghostscript\*\GS_DLL)");
  if(defaultGhostscriptLibrary.empty())
    defaultGhostscriptLibrary=getEntry(R"(AFPL Ghostscript\*\GS_DLL)");

  string gslib=stripDir(defaultGhostscriptLibrary);
  defaultGhostscript=stripFile(defaultGhostscriptLibrary)+
    ((gslib.empty() || gslib.substr(5,2) == "32") ? "gswin32c.exe" : "gswin64c.exe");

  if (string const s= getEntry(R"(Microsoft\Windows\CurrentVersion\App Paths\Asymptote\Path)"); !s.empty())
  {
    docdir= s;
  }
  // An empty systemDir indicates a TeXLive build
  if(!systemDir.empty() && !docdir.empty())
    systemDir=docdir;
}

#endif

// The name of the program (as called).  Used when displaying help info.
char *argv0;

Int verbose;
bool debug;
bool xasy;

bool quiet=false;

// Conserve memory at the expense of speed.
bool compact;

// Colorspace conversion flags (stored in global variables for efficiency).
bool gray;
bool bw;
bool rgb;
bool cmyk;

// Disable system calls.
bool safe=true;
// Enable reading from other directories
bool globalRead=true;
// Enable writing to (or changing to) other directories
bool globalWrite=false;

bool globalwrite() {return globalWrite || !safe;}
bool globalread() {return globalRead || !safe;}

const string suffix="asy";
const string guisuffix="gui";
const string standardprefix="out";

string initdir;
string historyname;

// Local versions of the argument list.
int argCount = 0;
char **argList = 0;

typedef ::option c_option;

types::dummyRecord *settingsModule;

types::record *getSettingsModule() {
  return settingsModule;
}

void noWarn(const string& s)
{
  array *Warn=getSetting<array *>("suppress");
  size_t size=checkArray(Warn);
  if(s.empty()) return;
  for(size_t i=0; i < size; i++)
    if(vm::read<string>(Warn,i) == s) return;
  Warn->push(s);
}

void Warn(const string& s)
{
  array *Warn=getSetting<array *>("suppress");
  size_t size=checkArray(Warn);
  for(size_t i=0; i < size; i++)
    if(vm::read<string>(Warn,i) == s)
      (*Warn).erase((*Warn).begin()+i,(*Warn).begin()+i+1);
}

bool warn(const string& s)
{
  if(debug) return true;
  array *Warn=getSetting<array *>("suppress");
  size_t size=checkArray(Warn);
  for(size_t i=0; i < size; i++)
    if(vm::read<string>(Warn,i) == s) return false;
  return true;
}

// The dictionaries of long options and short options.
struct option;
typedef mem::map<CONST string, option *> optionsMap_t;
optionsMap_t optionsMap;

typedef mem::map<CONST char, option *> codeMap_t;
codeMap_t codeMap;

struct option : public gc {
  string name;
  char code;      // Command line option, i.e. 'V' for -V.
  bool argument;  // If it takes an argument on the command line.  This is set
                  // based on whether argname is empty.
  string argname; // The argument name for printing the description.
  string desc; // One line description of what the option does.
  bool cmdlineonly; // If it is only available on the command line.
  string Default; // A string containing an optional default value.

  option(string name, char code, string argname, string desc,
         bool cmdlineonly=false, string Default="")
    : name(name), code(code), argument(!argname.empty()), argname(argname),
      desc(desc), cmdlineonly(cmdlineonly), Default(Default) {}

  virtual ~option() {}

  // Builds this option's contribution to the optstring argument of get_opt().
  virtual string optstring() {
    if (code) {
      string base;
      base.push_back(code);
      if(argument) base.push_back(':');
      return base;
    }
    else return "";
  }

  // Sets the contribution to the longopt array.
  virtual void longopt(c_option &o) {
    o.name=name.c_str();
    o.has_arg=argument ? 1 : 0;
    o.flag=0;
    o.val=0;
  }

  // Add to the dictionaries of options.
  virtual void add() {
    optionsMap[name]=this;
    if (code)
      codeMap[code]=this;
  }

  // Set the option from the command-line argument.  Return true if the option
  // was correctly parsed.
  virtual bool getOption() = 0;

  void error(string msg) {
    cerr << endl << argv0 << ": ";
    if (code)
      cerr << "-" << code << " ";
    cerr << "(-" << name << ") " << msg << endl;
  }

  // The "-f,-outformat format" part of the option.
  virtual string describeStart() {
    ostringstream ss;
    if (code)
      ss << "-" << code << ",";
    ss << "-" << name;
    if (argument)
      ss << " " << argname;
    return ss.str();
  }

  // Outputs description of the command for the -help option.
  virtual void describe(char option) {
    // Don't show the option if it has no desciption.
    if(!hide() && ((option == 'h') ^ env())) {
      const unsigned WIDTH=22;
      string start=describeStart();
      cerr << std::left << std::setw(WIDTH) << start;
      if (start.size() >= WIDTH) {
        cerr << endl;
        cerr << std::left << std::setw(WIDTH) << "";
      }
      cerr << " " << desc;
      if(cmdlineonly) cerr << "; command-line only";
      if(Default != "") {
        if(!desc.empty()) cerr << " ";
        cerr << Default;
      }
      cerr << endl;
    }
  }

  virtual void reset() {
  }

  virtual bool env() {return false;}
  virtual bool hide() {return false;}
};

const string noarg;

struct setting : public option {
  types::ty *t;

private:
  trans::permission perm;
  bool added;

  // Flag the setting as secure, so that it can only be set on the command-line,
  // though it can still be read in Asymptote code.
  void secure() {
    assert(!added);
    perm = trans::RESTRICTED;
  }

public:
  setting(string name, char code, string argname, string desc,
          types::ty *t, string Default)
    : option(name, code, argname, desc, false,Default),
      t(t), perm(trans::PUBLIC), added(false) {}

  void reset() = 0;

  virtual trans::access *buildAccess() = 0;

  // Add to the dictionaries of options and to the settings module.
  virtual void add() {
    assert(!added);

    option::add();
    settingsModule->add(name, t, buildAccess(), perm);

    added=true;
  }

  friend void addSecureSetting(setting *s) {
    s->secure();
    s->add();
  }
};

struct itemSetting : public setting {
  item defaultValue;
  item value;

  itemSetting(string name, char code,
              string argname, string desc,
              types::ty *t, item defaultValue, string Default="")
    : setting(name, code, argname, desc, t, Default),
      defaultValue(defaultValue) {reset();}

  void reset() {
    value=defaultValue;
  }

  trans::access *buildAccess() {
    return new itemRefAccess(&(value));
  }
};

item& Setting(string name) {
  itemSetting *s=dynamic_cast<itemSetting *>(optionsMap[name]);
  if(!s) {
    cerr << "Cannot find setting named '" << name << "'" << endl;
    exit(-1);
  }
  return s->value;
}

struct boolSetting : public itemSetting {
  boolSetting(string name, char code, string desc,
              bool defaultValue=false)
    : itemSetting(name, code, noarg, desc,
                  types::primBoolean(), (item)defaultValue,
                  defaultValue ? "[true]" : "[false]") {}

  bool getOption() {
    value=(item)true;
    return true;
  }

  option *negation(string name) {
    struct negOption : public option {
      boolSetting &base;

      bool hide() {return true;}

      negOption(boolSetting &base, string name)
        : option(name, 0, noarg, ""), base(base) {}

      bool getOption() {
        base.value=(item)false;
        return true;
      }
    };
    return new negOption(*this, name);
  }

  void add() {
    setting::add();
    negation("no"+name)->add();
    if (code) {
      string nocode="no"; nocode.push_back(code);
      negation(nocode)->add();
    }
  }

  // Set several related boolean options at once.  Used for view and trap which
  // have batch and interactive settings.
  struct multiOption : public option {
    typedef mem::list<boolSetting *> setlist;
    setlist set;
    multiOption(string name, char code, string desc)
      : option(name, code, noarg, desc, true) {}

    void add(boolSetting *s) {
      set.push_back(s);
    }

    void setValue(bool value) {
      for (setlist::iterator s=set.begin(); s!=set.end(); ++s)
        (*s)->value=(item)value;
    }

    bool getOption() {
      setValue(true);
      return true;
    }

    option *negation(string name) {
      struct negOption : public option {
        multiOption &base;

        bool hide() {return true;}

        negOption(multiOption &base, string name)
          : option(name, 0, noarg, ""), base(base) {}

        bool getOption() {
          base.setValue(false);
          return true;
        }
      };
      return new negOption(*this, name);
    }

    void add() {
      option::add();
      negation("no"+name)->add();
      if (code) {
        string nocode="no"; nocode.push_back(code);
        negation(nocode)->add();
      }

      for (multiOption::setlist::iterator s=set.begin(); s!=set.end(); ++s)
        (*s)->add();
    }
  };
};

typedef boolSetting::multiOption multiOption;

struct argumentSetting : public itemSetting {
  argumentSetting(string name, char code,
                  string argname, string desc,
                  types::ty *t, item defaultValue)
    : itemSetting(name, code, argname, desc, t, defaultValue)
  {
    assert(!argname.empty());
  }
};

struct stringSetting : public argumentSetting {
  stringSetting(string name, char code,
                string argname, string desc,
                string defaultValue="")
    : argumentSetting(name, code, argname, desc == "" ? "["+defaultValue+"]" :
                      desc+(defaultValue.empty() ? "" : " ["+defaultValue+"]"),
                      types::primString(), (item)defaultValue) {}

  bool getOption() {
    value=(item)(string)optarg;
    return true;
  }
};

struct userSetting : public argumentSetting {
  userSetting(string name, char code,
              string argname, string desc,
              string defaultValue="")
    : argumentSetting(name, code, argname, desc,
                      types::primString(), (item)defaultValue) {}

  bool getOption() {
    string s=vm::get<string>(value)+string(optarg);
    s.push_back(';');
    value=(item) s;
    return true;
  }
};

struct warnSetting : public option {
  warnSetting(string name, char code,
              string argname, string desc)
    : option(name, code, argname, desc, true) {}

  bool getOption() {
    Warn(string(optarg));
    return true;
  }

  option *negation(string name) {
    struct negOption : public option {
      warnSetting &base;

      bool hide() {return true;}

      negOption(warnSetting &base, string name, string argname)
        : option(name, 0, argname, ""), base(base) {}

      bool getOption() {
        noWarn(string(optarg));
        return true;
      }
    };
    return new negOption(*this, name, argname);
  }

  void add() {
    option::add();
    negation("no"+name)->add();
    if (code) {
      string nocode="no"; nocode.push_back(code);
      negation(nocode)->add();
    }
  }
};

string GetEnv(string s, string Default) {
  transform(s.begin(), s.end(), s.begin(), toupper);
  string t=Getenv(("ASYMPTOTE_"+s).c_str(),msdos);
  return t.empty() ? Default : t;
}

struct envSetting : public stringSetting {
  envSetting(string name, string Default)
    : stringSetting(name, 0, " ", "", GetEnv(name,Default)) {}
  bool env() {return true;}
};

template<class T>
struct dataSetting : public argumentSetting {
  string text;
  dataSetting(const char *text, string name, char code,
              string argname, string desc, types::ty *type,
              T defaultValue)
    : argumentSetting(name, code, argname, desc,
                      type, (item)defaultValue), text(text) {}

  bool getOption() {
    try {
      value=(item)lexical::cast<T>(optarg);
    } catch (lexical::bad_cast&) {
      error("option requires " + text + " as an argument");
      return false;
    }
    return true;
  }
};

template<class T>
string description(string desc, T defaultValue)
{
  return desc.empty() ? "" : desc+" ["+String(defaultValue)+"]";
}

struct IntSetting : public dataSetting<Int> {
  IntSetting(string name, char code,
             string argname, string desc, Int defaultValue=0)
    : dataSetting<Int>("an int", name, code, argname,
                       description(desc,defaultValue),
                       types::primInt(), defaultValue) {}
};

struct realSetting : public dataSetting<double> {
  realSetting(string name, char code,
              string argname, string desc, double defaultValue=0.0)
    : dataSetting<double>("a real", name, code, argname,
                          description(desc,defaultValue),
                          types::primReal(), defaultValue) {}
};

struct pairSetting : public dataSetting<pair> {
  pairSetting(string name, char code,
              string argname, string desc, pair defaultValue=0.0)
    : dataSetting<pair>("a pair", name, code, argname,
                        description(desc,defaultValue),
                        types::primPair(), defaultValue) {}
};

// For setting the alignment of a figure on the page.
struct alignSetting : public argumentSetting {
  alignSetting(string name, char code,
               string argname, string desc,
               string defaultValue)
    : argumentSetting(name, code, argname, description(desc,defaultValue),
                      types::primString(), (item)defaultValue) {}

  bool getOption() {
    string str=optarg;
    if(str == "C" || str == "T" || str == "B" || str == "Z") {
      value=str;
      return true;
    }
    error("invalid argument for option");
    return false;
  }
};

struct stringArraySetting : public itemSetting {
  stringArraySetting(string name, array *defaultValue)
    : itemSetting(name, 0, "", "",
                  types::stringArray(), (item) defaultValue) {}
  bool hide() {return true;}

  bool getOption() {return true;}
};

struct engineSetting : public argumentSetting {
  engineSetting(string name, char code,
                string argname, string desc,
                string defaultValue)
    : argumentSetting(name, code, argname, description(desc,defaultValue),
                      types::primString(), (item)defaultValue) {}

  bool getOption() {
    string str=optarg;

    if(str == "latex" || str == "pdflatex" || str == "xelatex" ||
       str == "tex" || str == "pdftex" || str == "luatex" ||
       str == "lualatex" || str == "context" || str == "none") {
      value=str;
      return true;
    }
    error("invalid argument for option");
    return false;
  }
};

template<class T>
string stringCast(T x)
{
  ostringstream buf;
  buf.precision(DBL_DIG);
  buf.setf(std::ios::boolalpha);
  buf << x;
  return string(buf.str());
}

template <class T>
struct refSetting : public setting {
  T *ref;
  T defaultValue;
  string text;

  refSetting(string name, char code, string argname,
             string desc, types::ty *t, T *ref, T defaultValue,
             const char *text="")
    : setting(name, code, argname, desc, t, stringCast(defaultValue)),
      ref(ref), defaultValue(defaultValue), text(text) {
    reset();
  }

  virtual bool getOption() {
    try {
      *ref=lexical::cast<T>(optarg);
    } catch (lexical::bad_cast&) {
      error("option requires " + text + " as an argument");
      return false;
    }
    return true;
  }

  virtual void reset() {
    *ref=defaultValue;
  }

  trans::access *buildAccess() {
    return new refAccess<T>(ref);
  }
};

struct boolrefSetting : public refSetting<bool> {
  boolrefSetting(string name, char code, string desc, bool *ref,
                 bool Default=false)
    : refSetting<bool>(name, code, noarg, desc,
                       types::primBoolean(), ref, Default) {}
  virtual bool getOption() {
    *ref=true;
    return true;
  }

  virtual option *negation(string name) {
    struct negOption : public option {
      boolrefSetting &base;

      bool hide() {return true;}

      negOption(boolrefSetting &base, string name)
        : option(name, 0, noarg, ""), base(base) {}

      bool getOption() {
        *(base.ref)=false;
        return true;
      }
    };
    return new negOption(*this, name);
  }

  void add() {
    setting::add();
    negation("no"+name)->add();
    if (code) {
      string nocode="no"; nocode.push_back(code);
      negation(nocode)->add();
    }
  }
};

struct compactSetting : public boolrefSetting {
  compactSetting(string name, char code, string desc, bool *ref,
                 bool Default=false)
    : boolrefSetting(name,code,desc,ref,Default) {}
  bool getOption() {
    mem::compact(1);
    return boolrefSetting::getOption();
  }

  option *negation(string name) {
    mem::compact(0);
    return boolrefSetting::negation(name);
  }
};

struct incrementSetting : public refSetting<Int> {
  incrementSetting(string name, char code, string desc, Int *ref)
    : refSetting<Int>(name, code, noarg, desc,
                      types::primInt(), ref, 0) {}

  bool getOption() {
    // Increment the value.
    ++(*ref);
    return true;
  }

  option *negation(string name) {
    struct negOption : public option {
      incrementSetting &base;

      bool hide() {return true;}

      negOption(incrementSetting &base, string name)
        : option(name, 0, noarg, ""), base(base) {}

      bool getOption() {
        if(*base.ref) --(*base.ref);
        return true;
      }
    };
    return new negOption(*this, name);
  }

  void add() {
    setting::add();
    negation("no"+name)->add();
    if (code) {
      string nocode="no"; nocode.push_back(code);
      negation(nocode)->add();
    }
  }
};

struct incrementOption : public option {
  Int *ref;
  Int level;

  incrementOption(string name, char code, string desc, Int *ref,
                  Int level=1)
    : option(name, code, noarg, desc, true), ref(ref), level(level) {}

  bool hide() {return true;}

  bool getOption() {
    // Increment the value.
    (*ref) += level;
    return true;
  }
};

void addOption(option *o) {
  o->add();
}

void version()
{
  cerr << PACKAGE_NAME << " version " << REVISION
       << " [(C) 2004 Andy Hammerlindl, John C. Bowman, Tom Prince]"
       << endl;
}

void usage(const char *program)
{
  version();
  cerr << "\t\t\t" << "https://asymptote.sourceforge.io/"
       << endl
       << "Usage: " << program << " [options] [file ...]"
       << endl;
}

void reportSyntax() {
  cerr << endl;
  usage(argv0);
  cerr << endl << "Type '" << argv0
       << " -h' for a description of options." << endl;
  exit(1);
}

void displayOptions(char code)
{
  cerr << endl;
  if(code == 'h')
    cerr << "Options (negate boolean options by replacing - with -no): "
         << endl << endl;
  else
    cerr << "Environment settings: "
         << endl << endl;
  for (optionsMap_t::iterator opt=optionsMap.begin();
       opt!=optionsMap.end();
       ++opt)
    opt->second->describe(code);
}

struct helpOption : public option {
  helpOption(string name, char code, string desc)
    : option(name, code, noarg, desc, true) {}

  bool getOption() {
    usage(argv0);
    displayOptions(code);
    cerr << endl;
    exit(0);

    // Unreachable code.
    return true;
  }
};

struct versionOption : public option {
  versionOption(string name, char code, string desc)
    : option(name, code, noarg, desc, true) {}

  bool disabled;

  const void feature(const char *s, bool enabled) {
    if(enabled ^ disabled)
      cerr << s << endl;
  }

  void features(bool enabled) {
    disabled=!enabled;
    cerr << endl << (disabled ? "DIS" : "EN") << "ABLED OPTIONS:" << endl;

    bool glm=false;
    bool gl=false;
    bool ssbo=false;
    bool gsl=false;
    bool fftw3=false;
    bool eigen=false;
    bool xdr=false;
    bool curl=false;
    bool lsp=false;
    bool readline=false;
    bool editline=false;
    bool sigsegv=false;
    bool usegc=false;
    bool usethreads=false;

#if HAVE_LIBGLM
    glm=true;
#endif

#ifdef HAVE_GL
    gl=true;
#endif

#ifdef HAVE_SSBO
    ssbo=true;
#endif

#ifdef HAVE_LIBGSL
    gsl=true;
#endif

#ifdef HAVE_LIBFFTW3
    fftw3=true;
#endif

#ifdef HAVE_EIGEN_DENSE
    eigen=true;
#endif

#ifdef HAVE_LIBTIRPC
    xdr=true;
#endif

#ifdef HAVE_LIBCURL
    curl=true;
#endif

#ifdef HAVE_LSP
    lsp=true;
#endif

#ifdef HAVE_LIBCURSES
#ifdef HAVE_LIBREADLINE
    readline=true;
#else
#ifdef HAVE_LIBEDIT
    editline=true;
#endif
#endif
#endif

#ifdef HAVE_LIBSIGSEGV
    sigsegv=true;
#endif

#ifdef USEGC
    usegc=true;
#endif

#ifdef HAVE_PTHREAD
    usethreads=true;
#endif

    feature("V3D      3D vector graphics output",glm && xdr);
    feature("WebGL    3D HTML rendering",glm);
#ifdef HAVE_LIBOSMESA
    feature("OpenGL   3D OSMesa offscreen rendering",gl);
#else
    feature("OpenGL   3D OpenGL rendering",gl);
#endif
    feature("SSBO     GLSL shader storage buffer objects",ssbo);
    feature("GSL      GNU Scientific Library (special functions)",gsl);
    feature("FFTW3    Fast Fourier transforms",fftw3);
    feature("Eigen    Eigenvalue library",eigen);
    feature("XDR      External Data Representation (portable binary file format for V3D)",xdr);
    feature("CURL     URL support",curl);
    feature("LSP      Language Server Protocol",lsp);
    feature("Readline Interactive history and editing",readline);
    if(!readline)
      feature("Editline interactive editing (Readline is unavailable)",editline);
    feature("Sigsegv  Distinguish stack overflows from segmentation faults",
            sigsegv);
    feature("GC       Boehm garbage collector",usegc);
    feature("threads  Render OpenGL in separate thread",usethreads);
  }

  bool getOption() {
    version();
    features(1);
    features(0);
    exit(0);

    // Unreachable code.
    return true;
  }
};

struct divisorOption : public option {
  divisorOption(string name, char code, string argname, string desc)
    : option(name, code, argname, desc) {}

  bool getOption() {
    try {
#ifdef USEGC
      Int n=lexical::cast<Int>(optarg);
      if(n > 0) GC_set_free_space_divisor((GC_word) n);
#endif
    } catch (lexical::bad_cast&) {
      error("option requires an int as an argument");
      return false;
    }
    return true;
  }
};

// For security reasons, these options aren't fields of the settings module.
struct stringOption : public option {
  char **variable;
  stringOption(string name, char code, string argname,
               string desc, char **variable)
    : option(name, code, argname, desc, true), variable(variable) {}

  bool getOption() {
    *variable=optarg;
    return true;
  }
};

string build_optstring() {
  string s;
  for (codeMap_t::iterator p=codeMap.begin(); p !=codeMap.end(); ++p)
    s +=p->second->optstring();

  return s;
}

c_option *build_longopts() {
  size_t n=optionsMap.size();

  c_option *longopts=new(UseGC) c_option[n+1];
  Int i=0;
  for (optionsMap_t::iterator p=optionsMap.begin();
       p !=optionsMap.end();
       ++p, ++i)
    p->second->longopt(longopts[i]);

  longopts[n].name=NULL;
  longopts[n].has_arg=0;
  longopts[n].flag=NULL;
  longopts[n].val=0;

  return longopts;
}

void resetOptions()
{
  for(optionsMap_t::iterator opt=optionsMap.begin(); opt != optionsMap.end();
      ++opt)
    if(opt->first != "config" && opt->first != "dir" && opt->first != "sysdir")
      opt->second->reset();
}

void getOptions(int argc, char *argv[])
{
  bool syntax=false;
  optind=0;

  string optstring=build_optstring();
  //cerr << "optstring: " << optstring << endl;
  c_option *longopts=build_longopts();
  int long_index = 0;

  errno=0;
  for(;;) {
    int c = getopt_long_only(argc,argv,
                             optstring.c_str(), longopts, &long_index);
    if (c == -1)
      break;

    if (c == 0) {
      const char *name=longopts[long_index].name;
      //cerr << "long option: " << name << endl;
      if (!optionsMap[name]->getOption())
        syntax=true;
    }
    else if (codeMap.find((char)c) != codeMap.end()) {
      //cerr << "char option: " << (char)c << endl;
      if (!codeMap[(char)c]->getOption())
        syntax=true;
    }
    else {
      syntax=true;
    }

    errno=0;
  }

  if (syntax)
    reportSyntax();
}

#ifdef USEGC
void no_GCwarn(char *, GC_word)
{
}
#endif

array* stringArray(const char **s)
{
  size_t count=0;
  while(s[count])
    ++count;
  array *a=new array(count);
  for(size_t i=0; i < count; ++i)
    (*a)[i]=string(s[i]);
  return a;
}

void initSettings() {
  static bool initialize=true;
  if(initialize) {
#if defined(_WIN32)
    queryRegistry();
#endif
    initialize=false;
  }

  settingsModule=new types::dummyRecord(symbol::literalTrans("settings"));

// Default mouse bindings

// LEFT: rotate
// SHIFT LEFT: zoom
// CTRL LEFT: shift
// ALT LEFT: pan
  const char *leftbutton[]={"rotate","zoom","shift","pan",NULL};

// MIDDLE:
  const char *middlebutton[]={NULL};

// RIGHT: zoom
// SHIFT RIGHT: rotateX
// CTRL RIGHT: rotateY
// ALT RIGHT: rotateZ
  const char *rightbutton[]={"zoom","rotateX","rotateY","rotateZ",NULL};

// WHEEL_UP: zoomin
  const char *wheelup[]={"zoomin",NULL};

// WHEEL_DOWN: zoomout
  const char *wheeldown[]={"zoomout",NULL};

  addOption(new stringArraySetting("leftbutton", stringArray(leftbutton)));
  addOption(new stringArraySetting("middlebutton", stringArray(middlebutton)));
  addOption(new stringArraySetting("rightbutton", stringArray(rightbutton)));
  addOption(new stringArraySetting("wheelup", stringArray(wheelup)));
  addOption(new stringArraySetting("wheeldown", stringArray(wheeldown)));
  addOption(new stringArraySetting("suppress", new array));

  addOption(new warnSetting("warn", 0, "str", "Enable warning"));

  multiOption *view=new multiOption("View", 'V', "View output");
  view->add(new boolSetting("batchView", 0, "View output in batch mode",
                            msdos));
  view->add(new boolSetting("multipleView", 0,
                            "View output from multiple batch-mode files",
                            false));
  view->add(new boolSetting("interactiveView", 0,
                            "View output in interactive mode", true));
  addOption(view);
  addOption(new stringSetting("outformat", 'f', "format",
                              "Convert each output file to specified format",
                              ""));
  addOption(new boolSetting("svgemulation", 0,
                            "Emulate unimplemented SVG shading", true));
  addOption(new boolSetting("prc", 0,
                            "Embed 3D PRC graphics in PDF output", false));
  addOption(new boolSetting("v3d", 0,
                            "Embed 3D V3D graphics in PDF output", false));
  addOption(new boolSetting("toolbar", 0,
                            "Show 3D toolbar in PDF output", true));
  addOption(new boolSetting("axes3", 0,
                            "Show 3D axes in PDF output", true));
  addOption(new boolSetting("ibl", 0,
                            "Enable environment map image-based lighting", false));
  addOption(new stringSetting("image", 0,"str","Environment image name","snowyField"));
  addOption(new stringSetting("imageDir", 0,"str","Environment image library directory","ibl"));
  addOption(new stringSetting("imageURL", 0,"str","Environment image library URL","https://vectorgraphics.gitlab.io/asymptote/ibl"));
  addOption(new realSetting("render", 0, "n",
                            "Render 3D graphics using n pixels per bp (-1=auto)",
                            havegl ? -1.0 : 0.0));
  addOption(new realSetting("devicepixelratio", 0, "n", "Ratio of physical to logical pixels", 1.0));
  addOption(new IntSetting("antialias", 0, "n",
                           "Antialiasing width for rasterized output", 2));
  addOption(new IntSetting("multisample", 0, "n",
                           "Multisampling width for screen images", 4));
  addOption(new boolSetting("twosided", 0,
                            "Use two-sided 3D lighting model for rendering",
                            true));
  addOption(new boolSetting("GPUindexing", 0,
                            "Compute indexing partial sums on GPU", true));
  addOption(new boolSetting("GPUinterlock", 0,
                            "Use fragment shader interlock", true));
  addOption(new boolSetting("GPUcompress", 0,
                            "Compress GPU transparent fragment counts",
                            false));
  addOption(new IntSetting("GPUlocalSize", 0, "n",
                           "Compute shader local size", 256));
  addOption(new IntSetting("GPUblockSize", 0, "n",
                           "Compute shader block size", 8));

  addOption(new pairSetting("position", 0, "pair",
                            "Initial 3D rendering screen position"));
  addOption(new pairSetting("maxviewport", 0, "pair",
                            "Maximum viewport size",pair(0,0)));
  addOption(new pairSetting("viewportmargin", 0, "pair",
                            "Horizontal and vertical 3D viewport margin",
                            pair(0.5,0.5)));
  addOption(new boolSetting("webgl2", 0,
                            "Use webgl2 if available", false));
  addOption(new boolSetting("absolute", 0,
                            "Use absolute WebGL dimensions", false));
  addOption(new pairSetting("maxtile", 0, "pair",
                            "Maximum rendering tile size",pair(1024,768)));
  addOption(new boolSetting("iconify", 0,
                            "Iconify rendering window", false));
  addOption(new boolSetting("thick", 0,
                            "Render thick 3D lines", true));
  addOption(new boolSetting("thin", 0,
                            "Render thin 3D lines", true));
  addOption(new boolSetting("autobillboard", 0,
                            "3D labels always face viewer by default", true));
  addOption(new boolSetting("threads", 0,
                            "Use POSIX threads for 3D rendering", true));
  addOption(new boolSetting("fitscreen", 0,
                            "Fit rendered image to screen", true));
  addOption(new boolSetting("interactiveWrite", 0,
                            "Write expressions entered at the prompt to stdout",
                            true));
  addOption(new helpOption("help", 'h', "Show summary of options"));
  addOption(new helpOption("environment", 'e', "Show summary of environment settings"));
  addOption(new versionOption("version", 0, "Show version"));

  addOption(new pairSetting("offset", 'O', "pair", "PostScript offset"));
  addOption(new pairSetting("aligndir", 0, "pair",
                            "Directional page alignment (overrides align)"));
  addOption(new alignSetting("align", 'a', "C|B|T|Z",
                             "Center, Bottom, Top, or Zero page alignment",
                             "C"));

  addOption(new boolrefSetting("debug", 'd', "Enable debugging messages and traceback",&debug));
  addOption(new incrementSetting("verbose", 'v',
                                 "Increase verbosity level (can specify multiple times)", &verbose));
  // Resolve ambiguity with --version
  addOption(new incrementOption("vv", 0,"", &verbose,2));
  addOption(new incrementOption("novv", 0,"", &verbose,-2));

  addOption(new boolSetting("keep", 'k', "Keep intermediate files"));
  addOption(new boolSetting("keepaux", 0,
                            "Keep intermediate LaTeX .aux files"));
  addOption(new engineSetting("tex", 0, "engine",
                              "latex|pdflatex|xelatex|lualatex|tex|pdftex|luatex|context|none",
                              "latex"));

  addOption(new boolSetting("twice", 0,
                            "Run LaTeX twice (to resolve references)"));
  addOption(new boolSetting("inlinetex", 0, "Generate inline TeX code"));
  addOption(new boolSetting("embed", 0, "Embed rendered preview image", true));
  addOption(new boolSetting("auto3D", 0, "Automatically activate 3D scene",
                            true));
  addOption(new boolSetting("autoplay", 0, "Autoplay 3D animations", false));
  addOption(new boolSetting("loop", 0, "Loop 3D animations", false));
  addOption(new boolSetting("interrupt", 0, "", false));
  addOption(new boolSetting("animating", 0, "", false));
  addOption(new boolSetting("reverse", 0, "reverse 3D animations", false));

  addOption(new boolSetting("inlineimage", 0,
                            "Generate inline embedded image"));
  addOption(new boolSetting("compress", 0,
                            "Compress images in PDF output", true));
  addOption(new boolSetting("parseonly", 'p', "Parse file"));
  addOption(new boolSetting("translate", 's',
                            "Show translated virtual machine code"));
  addOption(new boolSetting("tabcompletion", 0,
                            "Interactive prompt auto-completion", true));
  addOption(new realSetting("prerender", 0, "resolution",
                            "Prerender V3D objects (0 implies vector output)", 0));
  addOption(new boolSetting("lossy", 0,
                            "Use single precision for V3D reals", false));
  addOption(new boolSetting("listvariables", 'l',
                            "List available global functions and variables"));
  addOption(new boolSetting("where", 0,
                            "Show where listed variables are declared"));

  multiOption *mask=new multiOption("mask", 'm',
                                    "Mask fpu exceptions");
  mask->add(new boolSetting("batchMask", 0,
                            "Mask fpu exceptions in batch mode", false));
  mask->add(new boolSetting("interactiveMask", 0,
                            "Mask fpu exceptions in interactive mode", true));
  addOption(mask);

  addOption(new boolrefSetting("bw", 0,
                               "Convert all colors to black and white",&bw));
  addOption(new boolrefSetting("gray", 0, "Convert all colors to grayscale",
                               &gray));
  addOption(new boolrefSetting("rgb", 0, "Convert cmyk colors to rgb",&rgb));
  addOption(new boolrefSetting("cmyk", 0, "Convert rgb colors to cmyk",&cmyk));

  addSecureSetting(new boolrefSetting("safe", 0, "Disable system call",
                                      &safe, true));
  addSecureSetting(new boolrefSetting("globalwrite", 0,
                                      "Allow write to other directory",
                                      &globalWrite, false));
  addSecureSetting(new boolrefSetting("globalread", 0,
                                      "Allow read from other directory",
                                      &globalRead, true));
  addSecureSetting(new stringSetting("outname", 'o', "name",
                                     "Alternative output directory/file prefix"));
  addOption(new stringOption("cd", 0, "directory", "Set current directory",
                             &startpath));

  addOption(new compactSetting("compact", 0,
                               "Conserve memory at the expense of speed",
                               &compact));
  addOption(new divisorOption("divisor", 0, "n",
                              "Garbage collect using purge(divisor=n) [2]"));

  addOption(new stringSetting("prompt", 0,"str","Prompt","> "));
  addOption(new stringSetting("prompt2", 0,"str",
                              "Continuation prompt for multiline input ",
                              ".."));
  addOption(new boolSetting("multiline", 0,
                            "Input code over multiple lines at the prompt"));
  addOption(new boolrefSetting("xasy", 0,
                            "Interactive mode for xasy",&xasy));

  addOption(new boolSetting("lsp", 0, "Interactive mode for the Language Server Protocol"));
  addOption(new envSetting("lspport", ""));
  addOption(new envSetting("lsphost", "127.0.0.1"));

  addOption(new boolSetting("wsl", 0, "Run asy under the Windows Subsystem for Linux"));

  addOption(new boolSetting("wait", 0,
                            "Wait for child processes to finish before exiting"));
  addOption(new IntSetting("inpipe", 0, "n","Input pipe",-1));
  addOption(new IntSetting("outpipe", 0, "n","Output pipe",-1));
  addOption(new boolSetting("exitonEOF", 0, "Exit interactive mode on EOF",
                            true));

  addOption(new boolSetting("quiet", 'q',
                            "Suppress welcome text and noninteractive stdout"));
  addOption(new boolSetting("localhistory", 0,
                            "Use a local interactive history file"));
  addOption(new IntSetting("historylines", 0, "n",
                           "Retain n lines of history",1000));
  addOption(new IntSetting("scroll", 0, "n",
                           "Scroll standard output n lines at a time",0));
  addOption(new IntSetting("level", 0, "n", "Postscript level",3));
  addOption(new boolSetting("autoplain", 0,
                            "Enable automatic importing of plain",
                            true));
  addOption(new boolSetting("autorotate", 0,
                            "Enable automatic PDF page rotation",
                            false));
  addOption(new boolSetting("offline", 0,
                            "Produce offline html files",false));
  addOption(new boolSetting("pdfreload", 0,
                            "Automatically reload document in pdfviewer",
                            false));
  addOption(new IntSetting("pdfreloaddelay", 0, "usec",
                           "Delay before attempting initial pdf reload"
                           ,750000));
  addOption(new stringSetting("autoimport", 0, "str",
                              "Module to automatically import"));
  addOption(new userSetting("command", 'c', "str",
                            "Command to autoexecute"));
  addOption(new userSetting("user", 'u', "str",
                            "General purpose user string"));

  addOption(new realSetting("zoomfactor", 0, "factor", "Zoom step factor",
                            1.05));
  addOption(new realSetting("zoomPinchFactor", 0, "n",
                            "WebGL zoom pinch sensitivity", 10));
  addOption(new realSetting("zoomPinchCap", 0, "limit",
                            "WebGL maximum zoom pinch", 100));
  addOption(new realSetting("zoomstep", 0, "step", "Mouse motion zoom step",
                            0.1));
  addOption(new realSetting("shiftHoldDistance", 0, "n",
                            "WebGL touch screen distance limit for shift mode",
                            20));
  addOption(new realSetting("shiftWaitTime", 0, "ms",
                            "WebGL touch screen shift mode delay",
                            200));
  addOption(new realSetting("vibrateTime", 0, "ms",
                            "WebGL shift mode vibrate duration",
                            25));
  addOption(new realSetting("spinstep", 0, "deg/s", "Spin speed",
                            60.0));
  addOption(new realSetting("framerate", 0, "frames/s", "Animation speed",
                            30.0));
  addOption(new realSetting("resizestep", 0, "step", "Resize step", 1.2));
  addOption(new IntSetting("digits", 0, "n",
                           "Default output file precision", 7));

  addOption(new realSetting("paperwidth", 0, "bp", "Default page width"));
  addOption(new realSetting("paperheight", 0, "bp", "Default page height"));

  addOption(new stringSetting("dvipsOptions", 0, "str", ""));
  addOption(new stringSetting("dvisvgmOptions", 0, "str", "", "--optimize"));
  addOption(new boolSetting("dvisvgmMultipleFiles", 0,
                            "dvisvgm supports multiple files", true));
  addOption(new stringSetting("convertOptions", 0, "str", ""));
  addOption(new stringSetting("gsOptions", 0, "str", ""));
  addOption(new stringSetting("htmlviewerOptions", 0, "str", ""));
  addOption(new stringSetting("psviewerOptions", 0, "str", ""));
  addOption(new stringSetting("pdfviewerOptions", 0, "str", ""));
  addOption(new stringSetting("pdfreloadOptions", 0, "str", ""));
  addOption(new stringSetting("glOptions", 0, "str", ""));
  addOption(new stringSetting("hyperrefOptions", 0, "str",
                              "","setpagesize=false,unicode,pdfborder=0 0 0"));

  addOption(new envSetting("config","config."+suffix));
  addOption(new envSetting("htmlviewer", defaultHTMLViewer));
  addOption(new envSetting("pdfviewer", defaultPDFViewer));
  addOption(new envSetting("psviewer", defaultPSViewer));
  addOption(new envSetting("gs", defaultGhostscript));
  addOption(new envSetting("libgs", defaultGhostscriptLibrary));
  addOption(new envSetting("epsdriver", defaultEPSdriver));
  addOption(new envSetting("psdriver", defaultPSdriver));
  addOption(new envSetting("pngdriver", defaultPNGdriver));
  addOption(new envSetting("asygl", defaultAsyGL));
  addOption(new envSetting("texpath", ""));
  addOption(new envSetting("texcommand", ""));
  addOption(new envSetting("dvips", "dvips"));
  addOption(new envSetting("dvisvgm", "dvisvgm"));
  addOption(new envSetting("convert", "magick"));
  addOption(new envSetting("display", defaultDisplay));
  addOption(new envSetting("animate", defaultAnimate));
  addOption(new envSetting("papertype", "letter"));
  addOption(new envSetting("dir", ""));
  addOption(new envSetting("sysdir", systemDir));
  addOption(new envSetting("textcommand","groff"));
  addOption(new envSetting("textcommandOptions","-e -P -b16"));
  addOption(new envSetting("textextension", "roff"));
  addOption(new envSetting("textoutformat", "ps"));
  addOption(new envSetting("textprologue", ".EQ\ndelim $$\n.EN"));
  addOption(new envSetting("textinitialfont", ".fam T\n.ps 12"));
  addOption(new envSetting("textepilogue", ".bp"));
}

// Access the arguments once options have been parsed.
int numArgs() { return argCount; }
char *getArg(int n) { return argList[n]; }

void setInteractive()
{
  if(xasy && getSetting<Int>("outpipe") < 0) {
    cerr << "Missing outpipe." << endl;
    exit(-1);
  }

  bool lspmode=getSetting<bool>("lsp");

  if(numArgs() == 0 && !getSetting<bool>("listvariables") &&
     getSetting<string>("command").empty() &&
     (isatty(STDIN_FILENO) || xasy || lspmode))
    interact::interactive=true;

  if(getSetting<bool>("localhistory"))
    historyname=string(getPath())+dirsep+"."+suffix+"_history";
  else {
#if defined(MIKTEX_WINDOWS)
    if (mkdir(initdir.c_str()) != 0 && errno != EEXIST)
      cerr << "failed to create directory " + initdir + "." << endl;
#else
#if defined(_WIN32)
    bool mkdirResult = CreateDirectoryA(initdir.c_str(), nullptr);
    bool mkdirSuccess = mkdirResult || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    int mkdirResult = mkdir(initdir.c_str(),0777);
    bool mkdirSuccess = mkdirResult == 0 || errno == EEXIST;
#endif
    if(!mkdirSuccess)
      cerr << "failed to create directory "+initdir+"." << endl;
#endif
    historyname=initdir+"/history";
  }
  if(!quiet && verbose > 1)
    cerr << "Using history " << historyname << endl;
}

bool view()
{
  if (interact::interactive)
    return getSetting<bool>("interactiveView");
  else
    return getSetting<bool>("batchView") &&
      (numArgs() == 1 || getSetting<bool>("multipleView"));
}

bool trap()
{
  if (interact::interactive)
    return !getSetting<bool>("interactiveMask");
  else
    return !getSetting<bool>("batchMask");
}

string outname()
{
  string name=getSetting<string>("outname");
  if(name.empty() && interact::interactive) return standardprefix;
  if(msdos) backslashToSlash(name);
  return name;
}

string lookup(const string& symbol)
{
#if defined(MIKTEX)
  if (symbol == "TEXMFMAIN")
  {
    return MIKTEX_SESSION()->GetSpecialPath(MiKTeX::Configuration::SpecialPath::DistRoot).GetData();
  }
  else
  {
    return "";
  }
#else
  string s;
  mem::vector<string> cmd;
  string kpsewhich="kpsewhich";
  string fullname=stripFile(argv0)+kpsewhich;
  std::ifstream exists(fullname.c_str());
  if(!exists) fullname=kpsewhich;
  cmd.push_back(fullname);
  cmd.push_back("--var-value="+symbol);
  iopipestream pipe(cmd);
  pipe >> s;
  size_t n=s.find('\r');
  if(n != string::npos)
    s.erase(n,1);
  n=s.find('\n');
  if(n != string::npos)
    s.erase(n,1);
  return s;
#endif
}

void initDir() {
  if(getSetting<string>("sysdir").empty()) {
    string s=lookup("TEXMFMAIN");
    if(s.size() > 1) {
      string texmf=s+dirsep;
      docdir=texmf+"doc"+dirsep+"asymptote";
      Setting("sysdir")=texmf+"asymptote";
      s=lookup("ASYMPTOTE_HOME");
      if(s.size() > 1)
        initdir=s;
    }
  }

  if(initdir.empty())
    initdir=Getenv("ASYMPTOTE_HOME",msdos);

  if(initdir.empty())
    initdir=Getenv(HOME.c_str(),msdos)+dirsep+"."+suffix;

#if defined(_WIN32) && !defined(MIKTEX)
  DWORD dirAttrib = GetFileAttributesA(initdir.c_str());
  bool dirExists = dirAttrib != INVALID_FILE_ATTRIBUTES && ((dirAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
  bool dirExists = access(initdir.c_str(),F_OK) == 0;
#endif

  if(dirExists) {
    if(!quiet && verbose > 1)
      cerr << "Using configuration directory " << initdir << endl;
  }
}

void setPath() {
  searchPath.clear();
  searchPath.push_back(".");
  string asydir=getSetting<string>("dir");
  if(asydir != "") {
    size_t p,i=0;
    while((p=asydir.find(pathSeparator,i)) < string::npos) {
      if(p > i) searchPath.push_back(asydir.substr(i,p-i));
      i=p+1;
    }
    if(i < asydir.length()) searchPath.push_back(asydir.substr(i));
  }
#if defined(_WIN32) && !defined(MIKTEX)
  DWORD dirAttrib = GetFileAttributesA(initdir.c_str());
  bool dirExists = dirAttrib != INVALID_FILE_ATTRIBUTES && ((dirAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
  bool dirExists = access(initdir.c_str(),F_OK) == 0;
#endif

  if(dirExists)
    searchPath.push_back(initdir);
  string sysdir=getSetting<string>("sysdir");
  if(sysdir != "")
    searchPath.push_back(sysdir);
  searchPath.push_back(docdir+"/examples");
}

void SetPageDimensions() {
  string paperType=getSetting<string>("papertype");

  if(paperType.empty() &&
     getSetting<double>("paperwidth") != 0.0 &&
     getSetting<double>("paperheight") != 0.0) return;

  if(paperType == "letter") {
    Setting("paperwidth")=8.5*inches;
    Setting("paperheight")=11.0*inches;
  } else {
    Setting("paperwidth")=21.0*cm;
    Setting("paperheight")=29.7*cm;

    if(paperType != "a4") {
      cerr << "Unknown paper size \'" << paperType << "\'; assuming a4."
           << endl;
      Setting("papertype")=string("a4");
    }
  }
}

bool xe(const string& texengine)
{
  return texengine == "xelatex";
}

bool lua(const string& texengine)
{
  return texengine == "luatex" || texengine == "lualatex";
}

bool context(const string& texengine)
{
  return texengine == "context";
}

bool pdf(const string& texengine)
{
  return texengine == "pdflatex" || texengine == "pdftex" || xe(texengine) ||
    lua(texengine) || context(texengine);
}

bool latex(const string& texengine)
{
  return texengine == "latex" || texengine == "pdflatex" ||
    texengine == "xelatex" || texengine == "lualatex";
}

string nativeformat()
{
  return pdf(getSetting<string>("tex")) ? "pdf" : "eps";
}

string defaultformat()
{
  string format=getSetting<string>("outformat");
  return (format.empty()) ? nativeformat() : format;
}

// TeX special command to set up currentmatrix for typesetting labels.
const char *beginlabel(const string& texengine)
{
  if(pdf(texengine))
    return xe(texengine) ? "\\special{pdf:literal q #5 0 0 cm}" :
      "\\special{pdf:q #5 0 0 cm}";
  else
    return "\\special{ps:gsave currentpoint currentpoint translate [#5 0 0] "
      "concat neg exch neg exch translate}";
}

// TeX special command to restore currentmatrix after typesetting labels.
const char *endlabel(const string& texengine)
{
  if(pdf(texengine))
    return xe(texengine) ? "\\special{pdf:literal Q}" : "\\special{pdf:Q}";
  else
    return "\\special{ps:currentpoint grestore moveto}";
}

// TeX macro to typeset raw postscript code
const char *rawpostscript(const string& texengine)
{
  if(pdf(texengine))
    return "\\def\\ASYraw#1{#1}";
  else
    return "\\def\\ASYraw#1{\n"
      "currentpoint currentpoint translate matrix currentmatrix\n"
      "100 12 div -100 12 div scale\n"
      "#1\n"
      "setmatrix neg exch neg exch translate}";
}

// TeX macro to begin picture
const char *beginpicture(const string& texengine)
{
  if(latex(texengine))
    return "\\begin{picture}";
  if(context(texengine))
    return "";
  else
    return "\\picture";
}

// TeX macro to end picture
const char *endpicture(const string& texengine)
{
  if(latex(texengine))
    return "\\end{picture}%";
  else if(context(texengine))
    return "%";
  else
    return "\\endpicture%";
}

// TeX macro to begin new page.
const char *newpage(const string& texengine)
{
  if(latex(texengine))
    return "\\newpage";
  else if(context(texengine))
    return "}\\page\\hbox{%";
  else
    return "\\eject";
}

// Begin TeX special command.
const char *beginspecial(const string& texengine)
{
  if(pdf(texengine))
    return xe(texengine) ? "\\special{pdf:literal " : "\\special{pdf:";
  else
    return "\\special{ps:";
}

// End TeX special command.
const char *endspecial()
{
  return "}%";
}

string texcommand()
{
  string command=getSetting<string>("texcommand");
  return command.empty() ? getSetting<string>("tex") : command;
}

string texprogram()
{
  string path=getSetting<string>("texpath");
  string engine=texcommand();
  return path.empty() ? engine : (string) (path+"/"+engine);
}

Int getScroll()
{
  Int scroll=settings::getSetting<Int>("scroll");
  if(scroll < 0) {
#ifdef HAVE_LIBCURSES
    static char *terminal=NULL;
    if(!terminal)
      terminal=getenv("TERM");
    if(terminal) {
#if defined(USE_SETUPTERM)
      int error=setupterm(terminal,1,&error);
      if(error == 0) scroll=lines > 2 ? lines-1 : 1;
      else
#endif
        scroll=0;
    } else scroll=0;
#else
    scroll=0;
#endif
  }
  return scroll;
}

void doConfig(string file)
{
  bool autoplain=getSetting<bool>("autoplain");
  bool listvariables=getSetting<bool>("listvariables");
  if(autoplain) Setting("autoplain")=false; // Turn off for speed.
  if(listvariables) Setting("listvariables")=false;

  runFile(file);

  if(autoplain) Setting("autoplain")=true;
  if(listvariables) Setting("listvariables")=true;
}

void setOptions(int argc, char *argv[])
{
  argv0=argv[0];

  cout.precision(DBL_DIG);

  // Build settings module.
  initSettings();

  // Read command-line options initially to obtain config, dir, sysdir,
  // verbose, and quiet.
  getOptions(argc,argv);

  quiet=getSetting<bool>("quiet");

  // Make configuration and history directory
  initDir();

  Int Verbose=verbose;
  string sysdir=getSetting<string>("sysdir");

  resetOptions();

  // Read user configuration file.
  setPath();
  string filename=getSetting<string>("config");
  if(!filename.empty()) {
    string file=locateFile(filename);
    if(!file.empty()) {
      if(!quiet && Verbose > 1)
        cerr << "Loading " << filename << " from " << file << endl;
      doConfig(file);
    }
  }

  // Read command-line options again to override configuration file defaults.
  getOptions(argc,argv);

  if(getSetting<Int>("outpipe") == 2) // Redirect cerr to cout
    std::cerr.rdbuf(std::cout.rdbuf());

  Setting("sysdir")=sysdir;

  if(docdir.empty())
    docdir=getSetting<string>("dir");

#ifdef USEGC
  if(verbose == 0 && !debug) GC_set_warn_proc(no_GCwarn);
#endif

  if(setlocale (LC_ALL, "") == NULL && debug)
    perror("setlocale");

  // Set variables for the file arguments.
  argCount = argc - optind;
  argList = argv + optind;

  // Recompute search path.
  setPath();

  if(getSetting<double>("paperwidth") != 0.0 &&
     getSetting<double>("paperheight") != 0.0)
    Setting("papertype")=string("");

  SetPageDimensions();

  setInteractive();
}

}
