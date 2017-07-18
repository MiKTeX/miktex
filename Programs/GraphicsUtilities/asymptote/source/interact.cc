/*****
 * interact.cc
 *
 * The glue between the lexical analyzer and the readline library.
 *****/

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstring>

#include "interact.h"
#include "runhistory.h"

#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "util.h"
#include "errormsg.h"

using namespace settings;

namespace run {
void init_readline(bool);
}

namespace interact {

bool interactive=false;
bool uptodate=true;
int lines=0;  
bool query=false;

bool tty=isatty(STDIN_FILENO);  
completer *currentCompleter=0;

void setCompleter(completer *c) {
  currentCompleter=c;
}

char *call_completer(const char *text, int state) {
  return currentCompleter ? (*currentCompleter)(text, state) : 0;
}

#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
void init_completion() {
  rl_completion_entry_function=call_completer;

  rl_completion_append_character='\0'; // Don't add a space after a match.

  /*
  // Build a string containing all characters that separate words to be
  // completed.  All characters that can't form part of an identifier are
  // treated as break characters.
  static char break_characters[128];
  Int j=0;
  for (unsigned char c=9; c < 128; ++c)
    if (!isalnum(c) && c != '_') {
      break_characters[j]=c;
      ++j;
    }
  break_characters[j]='\0';
  rl_completer_word_break_characters=break_characters;
  */
}
#endif  

char *(*Readline)(const char *prompt);

char *readverbatimline(const char *prompt)
{
  if(!cin.good()) {cin.clear(); return NULL;}
  cout << prompt;
  string s;
  getline(cin,s);
  return StrdupMalloc(s);
}
  
FILE *fin=NULL;

char *readpipeline(const char *prompt)
{
#if _POSIX_VERSION >= 200809L
  char *line=NULL;
  size_t n=0;
  return getline(&line,&n,fin) >= 0 ? line : NULL;
#else
  const int max_size=256;
  static char buf[max_size];
  ostringstream s;
  do {
    if(fgets(buf,max_size-1,fin) == NULL) break;
    s << buf;
  } while(buf[std::strlen(buf)-1] != '\n');
  return StrdupMalloc(s.str());
#endif
}
  
void pre_readline()
{
  int fd=intcast(settings::getSetting<Int>("inpipe"));
  if(fd >= 0) {
    if(!fin) fin=fdopen(fd,"r");
    Readline=readpipeline;
  } else {
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
    if(tty) {
      Readline=readline;
    } else
#endif
      Readline=readverbatimline;
  }
}

void init_interactive()
{
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  if(tty) {
    init_completion();
    run::init_readline(getSetting<bool>("tabcompletion"));
    read_history(historyname.c_str());
  }
#endif
}
  
string simpleline(string prompt) {
  // Rebind tab key, as the setting tabcompletion may be changed at runtime.
  pre_readline();
  
  Signal(SIGINT,SIG_IGN);
  // Get a line from the user.
  char *line=Readline(prompt.c_str());
  Signal(SIGINT,interruptHandler);

  // Reset scroll count.
  interact::lines=0;
  interact::query=tty;
  
  // Ignore keyboard interrupts while taking input.
  errorstream::interrupt=false;

  if(line) {
    string s=line;
    free(line);
    return s;
  } else {
    cout << endl;
    if(!tty || getSetting<bool>("exitonEOF"))
      throw eof();
    return "\n";
  }
}

void addToHistory(string line) {
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  // Only add it if it has something other than newlines.
  if(tty && line.find_first_not_of('\n') != string::npos) {
    add_history(line.c_str());
  }
#endif    
}

string getLastHistoryLine() {
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  if(tty && history_length > 0) {
    HIST_ENTRY *entry=history_list()[history_length-1];
    if(!entry) {
      em.compiler();
      em << "cannot access last history line";
      return "";
    } else 
      return entry->line;
  } else
#endif
    return "";
}

void setLastHistoryLine(string line) {
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  if(tty) {
    if (history_length > 0) {
      HIST_ENTRY *entry=remove_history(history_length-1);

      if(!entry) {
        em.compiler();
        em << "cannot modify last history line";
      } else {
        free(entry->line);
        free(entry);
      }
    }
    addToHistory(line);
  }
#endif
}

void deleteLastLine() {
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  if(tty) {
    HIST_ENTRY *entry=remove_history(history_length-1);
    if(!entry) {
      em.compiler();
      em << "cannot delete last history line";
    } else {
      free(entry->line);
      free(entry);
    }
  }
#endif
}

void cleanup_interactive() {
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  // Write the history file.
  if(tty) {
    stifle_history(intcast(getSetting<Int>("historylines")));
    write_history(historyname.c_str());
  }
#endif
}

} // namespace interact
