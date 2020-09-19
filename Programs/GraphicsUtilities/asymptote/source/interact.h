/*****
 * interact.h
 *
 * The glue between the lexical analyzer and the readline library.
 *****/

#ifndef INTERACT_H
#define INTERACT_H

#include "common.h"

void interruptHandler(int);

namespace interact {

extern bool interactive;
extern bool uptodate;
extern int lines; // Interactive scroll count
extern bool query; // Enable interactive scrolling;

void init_interactive();
  
// Read a line from the input, without any processing.
string simpleline(string prompt);

// Add a line of input to the readline history.
void addToHistory(string line);

// Functions to work with the most recently entered line in the history.
string getLastHistoryLine();
void setLastHistoryLine(string line);

// Remove the line last added to the history.
void deleteLastLine();

// Write out the history of input lines to the history file.
void cleanup_interactive();

// This class is used to set a text completion function for readline.  A class
// is used instead the usual function pointer so that information such as the
// current environment can be coded into the function (mimicking a closure).
class completer {
public:
  virtual ~completer() {};
  virtual char *operator () (const char *text, int state) = 0;
};

void setCompleter(completer *c);

#define YY_READ_BUF_SIZE YY_BUF_SIZE
  
void init_readline(bool tabcompletion);
}

#endif // INTERACT_H
