/*****
 * locate.cc
 * Tom Prince 2005/03/24
 *
 * Locate files in search path.
 *****/

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
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
  return ::access(filename.c_str(), R_OK) == 0;  
}

} // namespace fs


file_list_t searchPath;

// Returns list of possible filenames, accounting for extensions.
file_list_t mungeFileName(string id)
{
  string ext = fs::extension(id);
  file_list_t files;
  if (ext == "."+settings::suffix) {
    files.push_back(id);
    files.push_back(id+"."+settings::suffix);
  } else {
    files.push_back(id+"."+settings::suffix);
    files.push_back(id);
  }
  return files;
}

// Join a directory with the given filename, to give the path to the file.  This
// also avoids unsightly joins such as './file.asy' in favour of 'file.asy' and
// 'dir//file.asy' in favour of 'dir/file.asy'
string join(string dir, string file)
{
  return dir == "." ?           file :
    *dir.rbegin() == '/' ? dir + file :
    dir + "/" + file;
}

// Find the appropriate file, first looking in the local directory, then the
// directory given in settings, and finally the global system directory.
string locateFile(string id)
{
  if(id.empty()) return "";
  file_list_t filenames = mungeFileName(id);
  for (file_list_t::iterator leaf = filenames.begin();
       leaf != filenames.end();
       ++leaf) {
#ifdef __MSDOS__
    size_t p;
    while ((p=leaf->find('\\')) < string::npos)
      (*leaf)[p]='/';
    if ((p=leaf->find(':')) < string::npos && p > 0) {
      (*leaf)[p]='/';
      leaf->insert(0,"/cygdrive/");
    }
#endif    

    if ((*leaf)[0] == '/') {
      string file = *leaf;
      if (fs::exists(file))
        return file;
    } else {
      for (file_list_t::iterator dir = searchPath.begin();
           dir != searchPath.end();
           ++dir) {
        string file = join(*dir,*leaf);
        if (fs::exists(file))
          return file;
      }
    } 
  }
  return string();
}

} // namespace settings

 
