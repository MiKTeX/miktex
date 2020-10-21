/*****
 * parser.h
 * Tom Prince 2004/01/10
 *
 *****/

#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "absyn.h"

namespace parser {

// Opens and parses the file returning the abstract syntax tree.
// If there is an unrecoverable parse error, returns null.
absyntax::file *parseFile(const string& filename,
                          const char *nameOfAction);

// Opens and parses the URL returning the abstract syntax tree.
// If there is an unrecoverable parse error, returns null.
absyntax::file *parseURL(const string& filename,
                         const char *nameOfAction);

// Parses string and returns the abstract syntax tree.  Any error in lexing or
// parsing will be reported and a handled_error thrown.  If the string is
// "extendable", then a parse error simply due to running out of input will not
// throw an exception, but will return null.
absyntax::file *parseString(const string& code,
                            const string& filename,
                            bool extendable=false);

bool isURL(const string& filename);
bool readURL(stringstream& buf, const string& filename);

} // namespace parser

#endif // PARSER_H
