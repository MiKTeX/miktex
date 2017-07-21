/*****
 * settings.cc
 * Andy Hammerlindl 2004/05/10
 *
 * Declares a list of global variables that act as settings in the system.
 *****/

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cerrno>
#include <sys/stat.h>
#include <cfloat>
#include <locale.h>
#include <unistd.h>
#include <algorithm>

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
#if defined(MIKTEX)
#  include <miktex/Core/PathName>
#  include <miktex/Core/Session>
#endif

#ifdef HAVE_LIBCURSES
extern "C" {

#ifdef HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#include <ncurses/term.h>
#elif HAVE_NCURSES_H
#include <ncurses.h>
#include <term.h>
#elif HAVE_CURSES_H
#include <curses.h>
#include <term.h>
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
  
#ifdef HAVE_GL
const bool havegl=true;  
#else
const bool havegl=false;
#endif
  
mode_t mask;

#if defined(MIKTEX)
string systemDir;
#else
string systemDir=ASYMPTOTE_SYSDIR;
#endif
string defaultEPSdriver="eps2write";

#ifndef __MSDOS__
  
bool msdos=false;
string HOME="HOME";
#if defined(MIKTEX)
string docdir;
const char pathSeparator = MiKTeX::Core::PathName::PathNameDelimiter;
#else
string docdir=ASYMPTOTE_DOCDIR;
const char pathSeparator = ':';
#endif
string defaultPSViewer="gv";
#ifdef __APPLE__
string defaultPDFViewer="open";
#else  
string defaultPDFViewer="acroread";
#endif  
string defaultGhostscript="gs";
string defaultGhostscriptLibrary="/usr/lib/libgs.so";
string defaultDisplay="display";
string defaultAnimate="animate";
void queryRegistry() {}
const string dirsep="/";
  
#else  
  
bool msdos=true;
string HOME="USERPROFILE";
string docdir="c:\\Program Files\\Asymptote";
const char pathSeparator=';';
//string defaultPSViewer="gsview32.exe";
string defaultPSViewer="cmd";
//string defaultPDFViewer="AcroRd32.exe";
string defaultPDFViewer="cmd";
string defaultGhostscript;
string defaultGhostscriptLibrary;
//string defaultDisplay="imdisplay";
string defaultDisplay="cmd";
//string defaultAnimate="animate";
string defaultAnimate="cmd";
const string dirsep="\\";
  
#include <dirent.h>
  
// Use key to look up an entry in the MSWindows registry, respecting wild cards
string getEntry(const string& location, const string& key)
{
  string path="/proc/registry"+location+key;
  size_t star;
  string head;
  while((star=path.find("*")) < string::npos) {
    string prefix=path.substr(0,star);
    string suffix=path.substr(star+1);
    size_t slash=suffix.find("/");
    if(slash < string::npos) {
      path=suffix.substr(slash);
      suffix=suffix.substr(0,slash);
    } else {
      path=suffix;
      suffix="";
    }
    string directory=head+stripFile(prefix);
    string file=stripDir(prefix);
    DIR *dir=opendir(directory.c_str());
    if(dir == NULL) return "";
    dirent *p;
    string rsuffix=suffix;
    reverse(rsuffix.begin(),rsuffix.end());
    while((p=readdir(dir)) != NULL) {
      string dname=p->d_name;
      string rdname=dname;
      reverse(rdname.begin(),rdname.end());
      if(dname != "." && dname != ".." && 
         dname.substr(0,file.size()) == file &&
         rdname.substr(0,suffix.size()) == rsuffix) {
        head=directory+p->d_name;
        break;
      }
    }
    if(p == NULL) return "";
  }
  std::ifstream fin((head+path).c_str());
  if(fin) {
    string s;
    getline(fin,s);
    size_t end=s.find('\0');
    if(end < string::npos)
      return s.substr(0,end);
  }
  return "";
}
  
// Use key to look up an entry in the MSWindows registry, respecting wild cards
string getEntry(const string& key)
{
  string entry=getEntry("64/HKEY_CURRENT_USER/Software/",key);
  if(entry.empty()) entry=getEntry("64/HKEY_LOCAL_MACHINE/SOFTWARE/",key);
  if(entry.empty()) entry=getEntry("/HKEY_CURRENT_USER/Software/",key);
  if(entry.empty()) entry=getEntry("/HKEY_LOCAL_MACHINE/SOFTWARE/",key);
  return entry;
}

void queryRegistry()
{
  defaultGhostscriptLibrary=getEntry("GPL Ghostscript/*/GS_DLL");
  if(defaultGhostscriptLibrary.empty())
    defaultGhostscriptLibrary=getEntry("AFPL Ghostscript/*/GS_DLL");
  
  string gslib=stripDir(defaultGhostscriptLibrary);
  defaultGhostscript=stripFile(defaultGhostscriptLibrary)+
    ((gslib.empty() || gslib.substr(5,2) == "32") ? "gswin32c.exe" : "gswin64c.exe");
  if(defaultPDFViewer != "cmd")
    defaultPDFViewer=getEntry("Adobe/Acrobat Reader/*/InstallPath/@")+"\\"+
      defaultPDFViewer;
  if(defaultPSViewer != "cmd")
    defaultPSViewer=getEntry("Ghostgum/GSview/*")+"\\gsview\\"+defaultPSViewer;
  string s;
  s=getEntry("Microsoft/Windows/CurrentVersion/App Paths/Asymptote/Path");
  if(!s.empty()) docdir=s;
  // An empty systemDir indicates a TeXLive build
  if(!systemDir.empty() && !docdir.empty())
    systemDir=docdir;
}
  
#endif  
  
const char PROGRAM[]=PACKAGE_NAME;
const char VERSION[]=PACKAGE_VERSION;
const char BUGREPORT[]=PACKAGE_BUGREPORT;

// The name of the program (as called).  Used when displaying help info.
char *argv0;

// The verbosity setting, a global variable.
Int verbose;

// Conserve memory at the expense of speed.
bool compact;
  
// Colorspace conversion flags (stored in global variables for efficiency). 
bool gray;
bool bw;  
bool rgb;
bool cmyk;
  
// Disable system calls.
bool safe=true;
// Enable writing to (or changing to) other directories
bool globaloption=false;
  
bool globalwrite() {return globaloption || !safe;}
  
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
  if(getSetting<bool>("debug")) return true;
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
  virtual void describe() {
    // Don't show the option if it has no desciption.
    if (!desc.empty()) {
      const unsigned WIDTH=22;
      string start=describeStart();
      cerr << std::left << std::setw(WIDTH) << start;
      if (start.size() >= WIDTH) {
        cerr << endl;
        cerr << std::left << std::setw(WIDTH) << "";
      }
      cerr << " " << desc;
      if(cmdlineonly) cerr << "; command-line only";
      if(Default != "")
        cerr << " [" << Default << "]";
      cerr << endl;
    }
  }
  
  virtual void reset() {
  }
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
                  defaultValue ? "true" : "false") {}

  bool getOption() {
    value=(item)true;
    return true;
  }

  option *negation(string name) {
    struct negOption : public option {
      boolSetting &base;

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
    : argumentSetting(name, code, argname, desc.empty() ? "" :
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
  cerr << PROGRAM << " version " << REVISION
       << " [(C) 2004 Andy Hammerlindl, John C. Bowman, Tom Prince]" 
       << endl;
}

void usage(const char *program)
{
  version();
  cerr << "\t\t\t" << "http://asymptote.sourceforge.net/"
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

void displayOptions()
{
  cerr << endl;
  cerr << "Options (negate by replacing - with -no): " 
       << endl << endl;
  for (optionsMap_t::iterator opt=optionsMap.begin();
       opt!=optionsMap.end();
       ++opt)
    opt->second->describe();
}

struct helpOption : public option {
  helpOption(string name, char code, string desc)
    : option(name, code, noarg, desc, true) {}

  bool getOption() {
    usage(argv0);
    displayOptions();
    cerr << endl;
    exit(0);

    // Unreachable code.
    return true;
  }
};

struct versionOption : public option {
  versionOption(string name, char code, string desc)
    : option(name, code, noarg, desc, true) {}

  bool getOption() {
    version();
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
    queryRegistry();
    initialize=false;
  }

  settingsModule=new types::dummyRecord(symbol::trans("settings"));
  
// Default mouse bindings
  
// LEFT: rotate
// SHIFT LEFT: zoom
// CTRL LEFT: shift
// ALT LEFT: pan
  const char *leftbutton[]={"rotate","zoom","shift","pan",NULL};
  
// MIDDLE: menu (must be unmodified; ignores Shift, Ctrl, and Alt)
  const char *middlebutton[]={"menu",NULL};
  
// RIGHT: zoom/menu (must be unmodified)
// SHIFT RIGHT: rotateX
// CTRL RIGHT: rotateY
// ALT RIGHT: rotateZ
  const char *rightbutton[]={"zoom/menu","rotateX","rotateY","rotateZ",NULL};
  
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

  addOption(new warnSetting("warn", 0, "string", "Enable warning"));
  
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
                            "Emulate unimplemented SVG shading", false));
  addOption(new boolSetting("prc", 0,
                            "Embed 3D PRC graphics in PDF output", true));
  addOption(new boolSetting("toolbar", 0,
                            "Show 3D toolbar in PDF output", true));
  addOption(new boolSetting("axes3", 0,
                            "Show 3D axes in PDF output", true));
  addOption(new realSetting("render", 0, "n",
                            "Render 3D graphics using n pixels per bp (-1=auto)",
                            havegl ? -1.0 : 0.0));
  addOption(new IntSetting("antialias", 0, "n",
                           "Antialiasing width for rasterized output", 2));
  addOption(new IntSetting("multisample", 0, "n",
                           "Multisampling width for screen images", 4));
  addOption(new boolSetting("offscreen", 0,
                            "Use offscreen rendering",false));
  addOption(new boolSetting("twosided", 0,
                            "Use two-sided 3D lighting model for rendering",
                            true));
  addOption(new pairSetting("position", 0, "pair", 
                            "Initial 3D rendering screen position"));
  addOption(new pairSetting("maxviewport", 0, "pair",
                            "Maximum viewport size",pair(2048,2048)));
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
                            "Use POSIX threads for 3D rendering", !msdos));
  addOption(new boolSetting("fitscreen", 0,
                            "Fit rendered image to screen", true));
  addOption(new boolSetting("interactiveWrite", 0,
                            "Write expressions entered at the prompt to stdout",
                            true));
  addOption(new helpOption("help", 'h', "Show summary of options"));
  addOption(new versionOption("version", 0, "Show version"));

  addOption(new pairSetting("offset", 'O', "pair", "PostScript offset"));
  addOption(new pairSetting("aligndir", 0, "pair",
                             "Directional page alignment (overrides align)"));
  addOption(new alignSetting("align", 'a', "C|B|T|Z",
                             "Center, Bottom, Top, or Zero page alignment",
                             "C"));
  
  addOption(new boolSetting("debug", 'd', "Enable debugging messages"));
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
  addOption(new boolSetting("parseonly", 'p', "Parse file"));
  addOption(new boolSetting("translate", 's',
                            "Show translated virtual machine code"));
  addOption(new boolSetting("tabcompletion", 0,
                            "Interactive prompt auto-completion", true));
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
                                      &globaloption, false));
  addSecureSetting(new stringSetting("outname", 'o', "name",
                                     "Alternative output directory/filename"));
  addOption(new stringOption("cd", 0, "directory", "Set current directory",
                             &startpath));
  
#ifdef USEGC  
  addOption(new compactSetting("compact", 0,
                               "Conserve memory at the expense of speed",
                               &compact));
  addOption(new divisorOption("divisor", 0, "n",
                              "Garbage collect using purge(divisor=n) [2]"));
#endif  
  
  addOption(new stringSetting("prompt", 0,"string","Prompt","> "));
  addOption(new stringSetting("prompt2", 0,"string",
                              "Continuation prompt for multiline input ",
                              ".."));
  addOption(new boolSetting("multiline", 0,
                            "Input code over multiple lines at the prompt"));

  addOption(new boolSetting("wait", 0,
                            "Wait for child processes to finish before exiting"));
  addOption(new IntSetting("inpipe", 0, "n","",-1));
  addOption(new IntSetting("outpipe", 0, "n","",-1));
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
  addOption(new boolSetting("pdfreload", 0,
                            "Automatically reload document in pdfviewer",
                            false));
  addOption(new IntSetting("pdfreloaddelay", 0, "usec",
                           "Delay before attempting initial pdf reload"
                           ,750000));
  addOption(new stringSetting("autoimport", 0, "string",
                              "Module to automatically import"));
  addOption(new userSetting("command", 'c', "string",
                            "Command to autoexecute"));
  addOption(new userSetting("user", 'u', "string",
                            "General purpose user string"));
  
  addOption(new realSetting("zoomfactor", 0, "factor", "Zoom step factor",
                            1.05));
  addOption(new realSetting("zoomstep", 0, "step", "Mouse motion zoom step",
                            0.1));
  addOption(new realSetting("spinstep", 0, "deg/s", "Spin speed",
                            60.0));
  addOption(new realSetting("framerate", 0, "frames/s", "Animation speed",
                            30.0));
  addOption(new realSetting("framedelay", 0, "ms",
                            "Additional frame delay", 0.0));
  addOption(new realSetting("arcballradius", 0, "pixels",
                            "Arcball radius", 750.0));
  addOption(new realSetting("resizestep", 0, "step", "Resize step", 1.2));
  addOption(new IntSetting("doubleclick", 0, "ms",
                           "Emulated double-click timeout", 200));
  
  addOption(new realSetting("paperwidth", 0, "bp", ""));
  addOption(new realSetting("paperheight", 0, "bp", ""));
  
  addOption(new stringSetting("dvipsOptions", 0, "string", ""));
  addOption(new stringSetting("dvisvgmOptions", 0, "string", ""));
  addOption(new stringSetting("convertOptions", 0, "string", ""));
  addOption(new stringSetting("gsOptions", 0, "string", ""));
  addOption(new stringSetting("psviewerOptions", 0, "string", ""));
  addOption(new stringSetting("pdfviewerOptions", 0, "string", ""));
  addOption(new stringSetting("pdfreloadOptions", 0, "string", ""));
  addOption(new stringSetting("glOptions", 0, "string", ""));
  addOption(new stringSetting("hyperrefOptions", 0, "str",
                              "","setpagesize=false,unicode,pdfborder=0 0 0"));
  
  addOption(new envSetting("config","config."+suffix));
  addOption(new envSetting("pdfviewer", defaultPDFViewer));
  addOption(new envSetting("psviewer", defaultPSViewer));
  addOption(new envSetting("gs", defaultGhostscript));
  addOption(new envSetting("libgs", defaultGhostscriptLibrary));
  addOption(new envSetting("epsdriver", defaultEPSdriver));
  addOption(new envSetting("texpath", ""));
  addOption(new envSetting("texcommand", ""));
  addOption(new envSetting("dvips", "dvips"));
  addOption(new envSetting("dvisvgm", "dvisvgm"));
  addOption(new envSetting("convert", "convert"));
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
  if(numArgs() == 0 && !getSetting<bool>("listvariables") && 
     getSetting<string>("command").empty() &&
     (isatty(STDIN_FILENO) || getSetting<Int>("inpipe") >= 0))
    interact::interactive=true;
  
  if(getSetting<bool>("localhistory"))
    historyname=string(getPath())+dirsep+"."+suffix+"_history";
  else {
#if defined(MIKTEX_WINDOWS)
    if (mkdir(initdir.c_str()) != 0 && errno != EEXIST)
      cerr << "failed to create directory " + initdir + "." << endl;
#else
    if(mkdir(initdir.c_str(),0777) != 0 && errno != EEXIST)
      cerr << "failed to create directory "+initdir+"." << endl;
#endif
    historyname=initdir+"/history";
  }
  if(verbose > 1)
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
    return MiKTeX::Core::Session::Get()->GetSpecialPath(MiKTeX::Core::SpecialPath::DistRoot).GetData();
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
  
#ifdef __MSDOS__  
  mask=umask(0);
  if(mask == 0) mask=0027;
  umask(mask);
#endif  
  if(access(initdir.c_str(),F_OK) == 0) {
    if(verbose > 1)
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
  if(access(initdir.c_str(),F_OK) == 0)
    searchPath.push_back(initdir);
  string sysdir=getSetting<string>("sysdir");
  if(sysdir != "")
    searchPath.push_back(sysdir);
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
#ifndef __MSDOS__      
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
  
  // Read command-line options initially to obtain config, dir, sysdir, verbose.
  getOptions(argc,argv);
  
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
      if(Verbose > 1)
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
  if(verbose == 0 && !getSetting<bool>("debug")) GC_set_warn_proc(no_GCwarn);
#endif  

  if(setlocale (LC_ALL, "") == NULL && getSetting<bool>("debug"))
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
