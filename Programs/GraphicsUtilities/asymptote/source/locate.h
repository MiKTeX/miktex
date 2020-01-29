/*****
 * locate.h
 * Tom Prince 2005/03/24
 *
 * Locate files in search path.
 *****/

#ifndef LOCATE_H
#define LOCATE_H

#include "common.h"

namespace settings {

typedef mem::list<string> file_list_t;
extern file_list_t searchPath;

// Find the appropriate file, first looking in the local directory, then the
// directory given in settings, and finally the global system directory.
string locateFile(string id, bool full=false);

namespace fs {

// Check to see if a file of given name exists.
bool exists(string filename);

}

} // namespace settings

#endif // LOCATE_H
