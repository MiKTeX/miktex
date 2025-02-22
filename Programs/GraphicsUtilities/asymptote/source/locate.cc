/*****
 * locate.cc
 * Tom Prince 2005/03/24
 *
 * Locate files in search path.
 *****/

#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include <unistd.h>

#include "settings.h"
#include "util.h"
#include "locate.h"


namespace settings {

namespace fs {

string extension(string name)
{
  size_t n = name.rfind(".");
  if (n != string::npos)
    return name.substr(n);
  else
    return string();
}

bool exists(string filename)
{
  return fileExists(filename);
}

} // namespace fs


file_list_t searchPath;

// Returns list of possible filenames, accounting for extensions.
file_list_t mungeFileName(string id, string suffix)
{
  string ext = fs::extension(id);
  file_list_t files;
  if (ext == "."+suffix) {
    files.push_back(id);
    files.push_back(id+"."+suffix);
  } else {
    files.push_back(id+"."+suffix);
    files.push_back(id);
  }
  return files;
}

// Join a directory with the given filename, to give the path to the file,
// avoiding unsightly joins such as 'dir//file.asy' in favour of 'dir/file.asy'
string join(string dir, string file, bool full)
{
  return dir == "." ? (full ? string(getPath())+"/"+file : file) :
    *dir.rbegin() == '/' ? dir + file :
    dir + "/" + file;
}

// Find the appropriate file, first looking in the local directory, then the
// directory given in settings, and finally the global system directory.
string locateFile(string id, bool full, string suffix)
{
  if(id.empty()) return "";
  file_list_t filenames = mungeFileName(id,suffix);
  for (auto const& leaf : filenames) {
    if (leaf[0] == '/') { // FIXME: Add windows path check
      string file = leaf;
      if (fs::exists(file))
        return file;
    } else {
      for (auto const& dir : searchPath) {
        string file = join(dir,leaf,full);
        if (fs::exists(file))
          return file;
      }
    }
  }
  return string();
}

} // namespace settings
