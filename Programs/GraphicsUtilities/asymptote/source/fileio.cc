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

#if !defined(_WIN32)
#define _fdopen fdopen
#endif

namespace camp {

FILE *pipeout=NULL;

string tab="\t";
string newline="\n";

ofile Stdout("");
file nullfile("",false,NOMODE,false,true);

void openpipeout()
{
  int fd=intcast(settings::getSetting<Int>("outpipe"));
  if(!pipeout && fd >= 0) pipeout=_fdopen(fd,"w");
  if(!pipeout) {
    cerr << "Cannot open outpipe " << fd << endl;
    exit(-1);
  }
}

string locatefile(string name)
{
  string s=settings::locateFile(name,false,"");
  return s.empty() ? name : s;
}

bool file::Standard()
{
  return standard;
}

void file::standardEOF()
{
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  cout << endl;
#endif
}

void file::purgeStandard(string&)
{
  if(cin.eof())
    standardEOF();
}

void file::dimension(Int Nx, Int Ny, Int Nz)
{
  if(Nx < -2 || Ny < -2 || Nz < -2) {
    ostringstream buf;
    buf << "Invalid array dimensions: " << Nx << ", " << Ny << ", " << Nz;
    reportError(buf);
  }

  nx=Nx; ny=Ny; nz=Nz;
}

file::file(string const& name, bool check, Mode type, bool binary, bool closed) :
    name(name), check(check), type(type), linemode(false), csvmode(false),
    wordmode(false), singlereal(false), singleint(true), signedint(true),
    closed(closed), standard(name.empty()), binary(binary), nullfield(false)
{
  whitespace="";
  dimension();
}

void file::Check()
{
  if(error()) {
    ostringstream buf;
    buf << "Cannot open file \"" << name << "\"";
    reportError(buf);
  }
}

file::~file()
{

}

bool file::isOpen()
{
  if(closed) {
    ostringstream buf;
    buf << "I/O operation attempted on ";
    if(name != "") buf << "closed file \'" << name << "\'";
    else buf << "null file";
    reportError(buf);
  }
  return true;
}
void file::unsupported(char const* rw, char const* type)
{
  ostringstream buf;
  buf << rw << " of type " << type << " not supported in " << FileMode()
      << " mode";
  reportError(buf);
}

void ifile::open()
{
  if(standard) {
    if(mode & std::ios::binary)
      reportError("Cannot open standard input in binary mode");
    stream=&cin;
  } else {
    if(mode & std::ios::out)
      name=outpath(name);
    if(mode & std::ios::in) {
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

void ifile::Read(double& val) {
  char c;
  std::string str;
  bool neg;

  while(isspace(c=stream->peek()))
    stream->ignore();
  neg=stream->peek() == '-';
  // Try parsing the input as a number.
  if(*stream >> val)
    return;

  clear();

  switch(stream->peek()) {
    case 'I': case 'i': // inf
    case 'N': case 'n': // NaN
      for(Int i=0; i < 3 && stream->good(); i++)
        str += stream->get();
      break;
    default:
      stream->setstate(std::ios_base::failbit);
      return;
  }

  if(strcasecmp(str.c_str(),"inf") == 0)
    val=std::numeric_limits < double > ::infinity();
  else if(strcasecmp(str.c_str(),"nan") == 0)
    val=std::numeric_limits < double > ::quiet_NaN();
  else {
    for(auto it=str.rbegin(); it != str.rend(); ++it)
      stream->putback(*it);
    stream->setstate(std::ios_base::failbit);
    return;
  }

  if(neg)
    val=-val;
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

void ofile::open()
{
  if(standard) {
    if(mode & std::ios::binary)
      reportError("Cannot open standard output in binary mode");
    stream=&cout;
  } else {
    name=outpath(name);
#if definede(MIKTEX_WINDOWS)
    stream = fstream = new std::ofstream(MiKTeX::Util::CharBuffer<wchar_t>(name).GetData(), mode | std::ios::trunc);
#else
    stream=fstream=new std::ofstream(name.c_str(),mode | std::ios::trunc);
#endif
    stream->precision(settings::getSetting<Int>("digits"));
    index=processData().ofile.add(fstream);
    Check();
  }
}

void ofile::close()
{
  if(!standard && fstream) {
    fstream->close();
    closed=true;
    delete fstream;
    fstream=NULL;
    processData().ofile.remove(index);
  }
}

Int ofile::precision(Int p)
{
  return p == 0 ? stream->precision(settings::getSetting<Int>("digits")) :
         stream->precision(p);
}

void ofile::seek(Int pos, bool begin)
{
  if(!standard && fstream) {
    clear();
    fstream->seekp(pos,begin ? std::ios::beg : std::ios::end);
  }
}
size_t ofile::tell()
{
  if(fstream)
    return fstream->tellp();
  else
    return 0;
}

bool ofile::enabled()
{
  return !standard || settings::verbose > 1 ||
        interact::interactive || !settings::getSetting<bool>("quiet");
}

void opipe::write(const string& val) {
  if (fprintf(pipeout,"%s",val.c_str()) < 0)
  {
    reportError("Write failed to pipe");
  }
}

void opipe::flush()
{
  if(pipeout)
  {
    if (fflush(pipeout) == EOF)
    {
      reportError("Flushing pipe failed");
    }
  }
}

void iofile::writeline()
{
  *fstream << newline;
  if(errorstream::interrupt) throw interrupted();
}

#ifdef HAVE_LIBTIRPC

void igzxfile::open()
{
  name=locatefile(inpath(name));
  gzfile=gzopen(name.c_str(),"rb");
  Check();

  while(!gzeof(gzfile)) {
    std::vector<char> tmpBuf(readSize);
    auto filSz = gzread(gzfile,tmpBuf.data(),readSize);
    std::copy(tmpBuf.begin(),tmpBuf.begin()+filSz,std::back_inserter(readData));
  }
  gzclose(gzfile);

  fstream=new xdr::memixstream(readData);
  index=processData().ixfile.add(fstream);
}

void igzxfile::closeFile()
{
  if(fstream) {
    fstream->close();
    closed=true;
    delete fstream;
    processData().ixfile.remove(index);
  }
}
#endif

} // namespace camp
