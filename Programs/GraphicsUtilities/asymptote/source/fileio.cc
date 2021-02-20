/******
 * fileio.cc
 * Tom Prince and John Bowman 2004/08/10
 *
 * Handle input/output
 ******/

#include "fileio.h"
#include "settings.h"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

namespace camp {

FILE *pipeout=NULL;

string tab="\t";
string newline="\n";

ofile Stdout("");
file nullfile("",false,NOMODE,false,true);

void ifile::open()
{
  if(standard) {
    if(mode & std::ios::binary)
      reportError("Cannot open standard input in binary mode");
    stream=&cin;
  } else {
    if(mode & std::ios::out)
      name=outpath(name);
    else {
#ifdef HAVE_LIBCURL
      if(parser::isURL(name)) {
        parser::readURL(buf,name);
        stream=&buf;
      } else
#endif
      {
        name=locatefile(inpath(name));
#if defined(MIKTEX_WINDOWS)
        stream = fstream = new std::fstream(UW_(name), mode);
#else
        stream=fstream=new std::fstream(name.c_str(),mode);
#endif
      }
    }

    if(mode & std::ios::out) {
      if(error()) {
        delete fstream;
#if defined(MIKTEX_WINDOWS)
        std::ofstream f(UW_(name));
#else
        std::ofstream f(name.c_str());
#endif
        f.close();
#if defined(MIKTEX_WINDOWS)
        stream = fstream = new std::fstream(UW_(name), mode);
#else
        stream=fstream=new std::fstream(name.c_str(),mode);
#endif
      }
    }
    index=processData().ifile.add(fstream);
    if(check) Check();
  }
}

void ifile::ignoreComment()
{
  if(comment == 0) return;
  int c=stream->peek();
  bool eol=c == '\n';
  if(csvmode && eol) {nullfield=true; return;}
  if(csvmode && c == ',') nullfield=true;
  for(;;) {
    while(isspace(c=stream->peek())) {
      stream->ignore();
      whitespace += (char) c;
    }
    if(c == comment) {
      whitespace="";
      while((c=stream->peek()) != '\n' && c != EOF)
        stream->ignore();
      if(c == '\n')
        stream->ignore();
    } else {if(c != EOF && eol) stream->unget(); return;}
  }
}

bool ifile::eol()
{
  int c;
  while(isspace(c=stream->peek())) {
    if(c == '\n') return true;
    else {
      stream->ignore();
      whitespace += (char) c;
    }
  }
  return false;
}

bool ifile::nexteol()
{
  int c;
  if(nullfield) {
    nullfield=false;
    return true;
  }

  while(isspace(c=stream->peek())) {
    if(c == '\n' && comma) {
      nullfield=true;
      return false;
    }
    stream->ignore();
    if(c == '\n') {
      while(isspace(c=stream->peek())) {
        if(c == '\n') {nullfield=true; return true;}
        else {
          stream->ignore();
          whitespace += (char) c;
        }
      }
      return true;
    }
    else whitespace += (char) c;
  }
  return false;
}

void ifile::csv()
{
  comma=false;
  nullfield=false;
  if(!csvmode || stream->eof()) return;
  std::ios::iostate rdstate=stream->rdstate();
  if(stream->fail()) stream->clear();
  int c=stream->peek();
  if(c == ',') stream->ignore();
  else if(c == '\n') {
    stream->ignore();
    if(linemode && stream->peek() != EOF) stream->unget();
  } else stream->clear(rdstate);
  if(c == ',') comma=true;
}

void ifile::Read(string& val)
{
  string s;
  if(wordmode) {
    whitespace="";
    while(isspace(stream->peek())) stream->ignore();
  }
  if(csvmode || wordmode) {
    bool quote=false;
    while(stream->good()) {
      int c=stream->peek();
      if(c == '"') {quote=!quote; stream->ignore(); continue;}
      if(!quote) {
        if(comment && c == comment) {
          while((c=stream->peek()) != '\n' && c != EOF)
            stream->ignore();
          if(wordmode && !linemode)
            while(isspace(stream->peek())) stream->ignore();
          if(stream->peek() == '"') {quote=!quote; stream->ignore(); continue;}
          if(s.empty() && c == '\n') {
            stream->ignore();
            continue;
          }
        }
        if(csvmode && (c == ',' || c == '\n'))
          break;
        if(wordmode && isspace(c)) {
          if(!linemode) while(isspace(stream->peek())) stream->ignore();
          break;
        }
      }
      s += (char) stream->get();
    }
  } else
    getline(*stream,s);

  if(comment) {
    size_t p=0;
    while((p=s.find(comment,p)) < string::npos) {
      if(p+1 < s.length() && s[p+1] == comment) {
        s.erase(p,1);
        ++p;
      } else {
        s.erase(p);
        break;
      }
    }
  }
  size_t n=s.length();
  if(n > 0) {
    size_t pos=n-1;
    if(s[pos] == '\r') s.erase(pos,1);
  }
  val=whitespace+s;
}

void ofile::writeline()
{
  if(standard && interact::query && !vm::indebugger) {
    Int scroll=settings::getScroll();
    if(scroll && interact::lines > 0 && interact::lines % scroll == 0) {
      for(;;) {
        if(!cin.good()) {
          *stream << newline;
          cin.clear();
          break;
        }
        int c=cin.get();
        if(c == '\n') break;
        // Discard any additional characters
        while(cin.good() && cin.get() != '\n');
        if(c == 's') {interact::query=false; break;}
        if(c == 'q') {interact::query=false; interact::lines=0; throw quit();}
      }
    } else *stream << newline;
    ++interact::lines;
  } else *stream << newline;
  if(errorstream::interrupt) {interact::lines=0; throw interrupted();}
}

} // namespace camp
