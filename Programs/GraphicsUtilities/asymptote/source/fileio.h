/******
 * fileio.h
 * Tom Prince and John Bowman 2004/05/10
 *
 * Handle input/output
 ******/

#ifndef FILEIO_H
#define FILEIO_H

#include <fstream>
#include <iostream>
#include <sstream>

#include "common.h"

#ifdef HAVE_RPC_RPC_H
#include "xstream.h"
#endif

#include "pair.h"
#include "triple.h"
#include "guide.h"
#include "pen.h"

#include "camperror.h"
#include "interact.h"
#include "errormsg.h"
#include "util.h"
#include "process.h"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

namespace vm {
extern bool indebugger;  
}

namespace camp {

extern string tab;
extern string newline;
  
enum Mode {NOMODE,INPUT,OUTPUT,UPDATE,BINPUT,BOUTPUT,BUPDATE,XINPUT,XOUTPUT,
           XUPDATE,OPIPE};

static const string FileModes[]=
{"none","input","output","output(update)",
 "input(binary)","output(binary)","output(binary,update)",
 "input(xdr)","output(xdr)","output(xdr,update)","output(pipe)"};

extern FILE *pipeout;

inline void openpipeout() 
{
  int fd=intcast(settings::getSetting<Int>("outpipe"));
  if(!pipeout && fd >= 0) pipeout=fdopen(fd,"w");
  if(!pipeout) {
    ostringstream buf;
    buf << "Cannot open outpipe " << fd;
    reportError(buf);
  }
}

class file : public gc {
protected:  
  string name;
  bool check;      // Check whether input file exists.
  Mode type;
  
  Int nx,ny,nz;    // Array dimensions
  bool linemode;   // Array reads will stop at eol instead of eof.
  bool csvmode;    // Read comma-separated values.
  bool wordmode;   // Delimit strings by white space instead of eol.
  bool singlereal; // Read/write single-precision XDR/binary reals.
  bool singleint;  // Read/write single-precision XDR/binary ints.
  bool signedint;  // Read/write signed XDR/binary ints.
  
  bool closed;     // File has been closed.
  bool standard;   // Standard input/output
  bool binary;     // Read in binary mode.
  
  bool nullfield;  // Used to detect a final null field in csv+line mode.
  string whitespace;
  size_t index;    // Terminator index.

public: 

  bool Standard() {return standard;}
  
  void standardEOF() {
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
    cout << endl;
#endif  
  }
  
  template<class T>
  void purgeStandard(T&) {
    if(standard) {
      int c;
      if(cin.eof())
        standardEOF();
      else {
        cin.clear();
        while((c=cin.peek()) != EOF) {
          cin.ignore();
          if(c == '\n') break;
        }
      }
    }
  }
  
  void purgeStandard(string&) {
    if(cin.eof())
      standardEOF();
  }
  
  void dimension(Int Nx=-1, Int Ny=-1, Int Nz=-1) {nx=Nx; ny=Ny; nz=Nz;}
  
  file(const string& name, bool check=true, Mode type=NOMODE, bool binary=false,
       bool closed=false) : 
    name(name), check(check), type(type), linemode(false), csvmode(false),
    wordmode(false), singlereal(false), singleint(true), signedint(true),
    closed(closed), standard(name.empty()),
    binary(binary), nullfield(false), whitespace("") {dimension();}
  
  virtual void open() {}
  
  void Check() {
    if(error()) {
      ostringstream buf;
      buf << "Cannot open file \"" << name << "\"";
      reportError(buf);
    }
  }
  
  virtual ~file() {}

  bool isOpen() {
    if(closed) {
      ostringstream buf;
      buf << "I/O operation attempted on ";
      if(name != "") buf << "closed file \'" << name << "\'";
      else buf << "null file";
      reportError(buf);
    }
    return true;
  }
                
  string filename() {return name;}
  virtual bool eol() {return false;}
  virtual bool nexteol() {return false;}
  virtual bool text() {return false;}
  virtual bool eof() {return true;}
  virtual bool error() {return true;}
  virtual void close() {}
  virtual void clear() {}
  virtual Int precision(Int) {return 0;}
  virtual void flush() {}
  virtual size_t tell() {return 0;}
  virtual void seek(Int, bool=true) {}
  
  string FileMode() {return FileModes[type];}
  
  void unsupported(const char *rw, const char *type) {
    ostringstream buf;
    buf << rw << " of type " << type << " not supported in " << FileMode()
        << " mode";
    reportError(buf);
  }
  
  void noread(const char *type) {unsupported("Read",type);}
  void nowrite(const char *type) {unsupported("Write",type);}
  
  virtual void Read(bool&) {noread("bool");}
  virtual void Read(Int&) {noread("int");}
  virtual void Read(double&) {noread("real");}
  virtual void Read(float&) {noread("real");}
  virtual void Read(pair&) {noread("pair");}
  virtual void Read(triple&) {noread("triple");}
  virtual void Read(char&) {noread("char");}
  virtual void Read(string&) {noread("string");}
  virtual void readwhite(string&) {noread("string");}
  
  virtual void write(bool) {nowrite("bool");}
  virtual void write(Int) {nowrite("int");}
  virtual void write(double) {nowrite("real");}
  virtual void write(const pair&) {nowrite("pair");}
  virtual void write(const triple&) {nowrite("triple");}
  virtual void write(const string&) {nowrite("string");}
  virtual void write(const pen&) {nowrite("pen");}
  virtual void write(guide *) {nowrite("guide");}
  virtual void write(const transform&) {nowrite("transform");}
  virtual void writeline() {nowrite("string");}
  
  virtual void ignoreComment() {};
  virtual void csv() {};
  
  template<class T>
  void ignoreComment(T&) {
    ignoreComment();
  }
  
  void ignoreComment(string&) {}
  void ignoreComment(char&) {}
  
  template<class T>
  void read(T& val) {
    if(binary) Read(val);
    else {
      if(standard) clear();
      if(errorstream::interrupt) throw interrupted();
      else {
        ignoreComment(val);
        val=T();
        if(!nullfield)
          Read(val);
        csv();
        whitespace="";
      }
    }
  }
  
  Int Nx() {return nx;}
  Int Ny() {return ny;}
  Int Nz() {return nz;}
  
  void Nx(Int n) {nx=n;}
  void Ny(Int n) {ny=n;}
  void Nz(Int n) {nz=n;}
  
  void LineMode(bool b) {linemode=b;}
  bool LineMode() {return linemode;}
  
  void CSVMode(bool b) {csvmode=b; if(b) wordmode=false;}
  bool CSVMode() {return csvmode;}
  
  void WordMode(bool b) {wordmode=b; if(b) csvmode=false;}
  bool WordMode() {return wordmode;}
  
  void SingleReal(bool b) {singlereal=b;}
  bool SingleReal() {return singlereal;}
  
  void SingleInt(bool b) {singleint=b;}
  bool SingleInt() {return singleint;}
  
  void SignedInt(bool b) {signedint=b;}
  bool SignedInt() {return signedint;}
};

class opipe : public file {
public:
  opipe(const string& name) : file(name,false,OPIPE) {standard=false;}

  void open() {
    openpipeout();
  }
  
  bool text() {return true;}
  bool eof() {return pipeout ? feof(pipeout) : true;}
  bool error() {return pipeout ? ferror(pipeout) : true;}
  void clear() {if(pipeout) clearerr(pipeout);}
  void flush() {if(pipeout) fflush(pipeout);}
  
  void seek(Int pos, bool begin=true) {
    if(!standard && pipeout) {
      clear();
      fseek(pipeout,pos,begin ? SEEK_SET : SEEK_END);
    }
  }
  
  size_t tell() {
    return pipeout ? ftell(pipeout) : 0;
  }
  
  void write(const string& val) {
    fprintf(pipeout,"%s",val.c_str());
  }
  
  void write(bool val) {
    ostringstream s;
    s << val;
    write(s.str());
  }
  
  void write(Int val) {
    ostringstream s;
    s << val;
    write(s.str());
  }
  void write(double val) {
    ostringstream s;
    s << val;
    write(s.str());
  }
  void write(const pair& val) {
    ostringstream s;
    s << val;
    write(s.str());
  }
  void write(const triple& val) {
    ostringstream s;
    s << val;
    write(s.str());
  }

  void write(const pen &val) {
    ostringstream s;
    s << val;
    write(s.str());
  }
  
  void write(guide *val) {
    ostringstream s;
    s << *val;
    write(s.str());
  }
  
  void write(const transform& val) {
    ostringstream s;
    s << val;
    write(s.str());
  }
  
  void writeline() {
    fprintf(pipeout,"\n");
    if(errorstream::interrupt) throw interrupted();
  }
};

class ifile : public file {
protected:  
  istream *stream;
  std::fstream *fstream;
  char comment;
  std::ios::openmode mode;
  bool comma;
  
public:
  ifile(const string& name, char comment, bool check=true, Mode type=INPUT, 
        std::ios::openmode mode=std::ios::in) :
    file(name,check,type), stream(&cin), fstream(NULL), comment(comment),
    mode(mode), comma(false) {}
  
  // Binary file
  ifile(const string& name, bool check=true, Mode type=BINPUT,
        std::ios::openmode mode=std::ios::in) :
    file(name,check,type,true), mode(mode) {}
  
  ~ifile() {close();}
  
  void open() {
    if(standard) {
      if(mode & std::ios::binary) 
        reportError("Cannot open standard input in binary mode");
      stream=&cin;
    } else {
      if(mode & std::ios::out)
        name=outpath(name);
#if defined(MIKTEX_WINDOWS)
      stream = fstream = new std::fstream(UW_(name.c_str()), mode);
#else
      stream=fstream=new std::fstream(name.c_str(),mode);
#endif
      if(mode & std::ios::out) {
        if(error()) {
          delete fstream;
#if defined(MIKTEX_WINDOWS)
          std::ofstream f(UW_(name.c_str()));
#else
          std::ofstream f(name.c_str());
#endif
          f.close();
#if defined(MIKTEX_WINDOWS)
          stream = fstream = new std::fstream(UW_(name.c_str()), mode);
#else
          stream=fstream=new std::fstream(name.c_str(),mode);
#endif
        }
      }
      index=processData().ifile.add(fstream);
      if(check) Check();
    }
  }
  
  bool eol();
  bool nexteol();
  
  bool text() {return true;}
  bool eof() {return stream->eof();}
  bool error() {return stream->fail();}
  
  void close() {
    if(!standard && fstream) {
      fstream->close();
      closed=true;
      delete fstream;
      fstream=NULL;
      processData().ifile.remove(index);
    }
  }
  
  void clear() {stream->clear();}
  
  void seek(Int pos, bool begin=true) {
    if(!standard && fstream) {
      clear();
      fstream->seekg(pos,begin ? std::ios::beg : std::ios::end);
    }
  }
  
  size_t tell() {
    if(fstream) 
      return fstream->tellg();
    else
      return 0;
  }
  
  void csv();
  
  virtual void ignoreComment();
  
  // Skip over white space
  void readwhite(string& val) {val=string(); *stream >> val;}
  
  void Read(bool &val) {string t; readwhite(t); val=(t == "true");}
  void Read(Int& val) {*stream >> val;}
  void Read(double& val) {*stream >> val;}
  void Read(pair& val) {*stream >> val;}
  void Read(triple& val) {*stream >> val;}
  void Read(char& val) {stream->get(val);}
  void Read(string& val);
};
  
class iofile : public ifile {
public:
  iofile(const string& name, char comment=0) : 
    ifile(name,comment,true,UPDATE,std::ios::in | std::ios::out) {}

  Int precision(Int p) {
    return p == 0 ? stream->precision(settings::getSetting<Int>("digits")) :
      stream->precision(p);
  }
  void flush() {if(fstream) fstream->flush();}
  
  void write(bool val) {*fstream << (val ? "true " : "false ");}
  void write(Int val) {*fstream << val;}
  void write(double val) {*fstream << val;}
  void write(const pair& val) {*fstream << val;}
  void write(const triple& val) {*fstream << val;}
  void write(const string& val) {*fstream << val;}
  void write(const pen& val) {*fstream << val;}
  void write(guide *val) {*fstream << *val;}
  void write(const transform& val) {*fstream << val;}
  
  void writeline() {
    *fstream << newline;
    if(errorstream::interrupt) throw interrupted();
  }
};
  
class ofile : public file {
protected:
  ostream *stream;
  std::ofstream *fstream;
  std::ios::openmode mode;
public:
  ofile(const string& name, Mode type=OUTPUT,
        std::ios::openmode mode=std::ios::trunc) : 
    file(name,true,type), stream(&cout), fstream(NULL), mode(mode) {}
  
  ~ofile() {close();}
  
  void open() {
    if(standard) {
      if(mode & std::ios::binary) 
        reportError("Cannot open standard output in binary mode");
      stream=&cout;
    } else {
      name=outpath(name);
      stream=fstream=new std::ofstream(name.c_str(),mode | std::ios::trunc);
      stream->precision(settings::getSetting<Int>("digits"));
      index=processData().ofile.add(fstream);
      Check();
    }
  }
  
  bool text() {return true;}
  bool eof() {return stream->eof();}
  bool error() {return stream->fail();}
  
  void close() {
    if(!standard && fstream) {
      fstream->close();
      closed=true;
      delete fstream;
      fstream=NULL;
      processData().ofile.remove(index);
    }
  }
  void clear() {stream->clear();}
  Int precision(Int p) {
    return p == 0 ? stream->precision(settings::getSetting<Int>("digits")) :
      stream->precision(p);
  }
  void flush() {stream->flush();}
  
  void seek(Int pos, bool begin=true) {
    if(!standard && fstream) {
      clear();
      fstream->seekp(pos,begin ? std::ios::beg : std::ios::end);
    }
  }
  
  size_t tell() {
    if(fstream) 
      return fstream->tellp();
    else
      return 0;
  }
  
  bool enabled() {return !standard || settings::verbose > 1 || 
      interact::interactive || !settings::getSetting<bool>("quiet");}
  
  void write(bool val) {*stream << (val ? "true " : "false ");}
  void write(Int val) {*stream << val;}
  void write(double val) {*stream << val;}
  void write(const pair& val) {*stream << val;}
  void write(const triple& val) {*stream << val;}
  void write(const string& val) {*stream << val;}
  void write(const pen& val) {*stream << val;}
  void write(guide *val) {*stream << *val;}
  void write(const transform& val) {*stream << val;}
  
  void writeline();
};

class ibfile : public ifile {
public:
  ibfile(const string& name, bool check=true, Mode type=BINPUT,
         std::ios::openmode mode=std::ios::in) : 
    ifile(name,check,type,mode | std::ios::binary) {}
  template<class T>
  void iread(T& val) {
    val=T();
    if(fstream) fstream->read((char *) &val,sizeof(T));
  }
  
  void Read(bool& val) {iread(val);}
  void Read(Int& val) {
    if(signedint) {
      if(singleint) {int ival; iread(ival); val=ival;}
      else iread(val);
    } else {
      if(singleint) {unsigned ival; iread(ival); val=Intcast(ival);}
      else {unsignedInt ival; iread(ival); val=Intcast(ival);}
    }
  }
  void Read(char& val) {iread(val);}
  void Read(string& val) {char c; iread(c); val=c;}
  
  void Read(double& val) {
    if(singlereal) {float fval; iread(fval); val=fval;}
    else iread(val);
  }
};
  
class iobfile : public ibfile {
public:
  iobfile(const string& name) : 
    ibfile(name,true,BUPDATE,std::ios::in | std::ios::out) {}

  void flush() {if(fstream) fstream->flush();}
  
  template<class T>
  void iwrite(T val) {
    if(fstream) fstream->write((char *) &val,sizeof(T));
  }
  
  void write(bool val) {iwrite(val);}
  void write(Int val) {
    if(signedint) {
      if(singleint) iwrite(intcast(val));
      else iwrite(val);
    } else {
      if(singleint) iwrite(unsignedcast(val));
      else iwrite(unsignedIntcast(val));
    }
  }
  void write(const string& val) {iwrite(val);}
  void write(const pen& val) {iwrite(val);}
  void write(guide *val) {iwrite(val);}
  void write(const transform& val) {iwrite(val);}
  void write(double val) {
    if(singlereal) iwrite((float) val);
    else iwrite(val);
  }
  void write(const pair& val) {
    write(val.getx());
    write(val.gety());
  }
  void write(const triple& val) {
    write(val.getx());
    write(val.gety());
    write(val.getz());
  }
  void writeline() {}
};
  
class obfile : public ofile {
public:
  obfile(const string& name) : ofile(name,BOUTPUT,std::ios::binary) {}

  template<class T>
  void iwrite(T val) {
    if(fstream) fstream->write((char *) &val,sizeof(T));
  }
  
  void write(bool val) {iwrite(val);}
  void write(Int val) {
    if(signedint) {
      if(singleint) iwrite(intcast(val));
      else iwrite(val);
    } else {
      if(singleint) iwrite(unsignedcast(val));
      else iwrite(unsignedIntcast(val));
    }
  }
  void write(const string& val) {iwrite(val);}
  void write(const pen& val) {iwrite(val);}
  void write(guide *val) {iwrite(val);}
  void write(const transform& val) {iwrite(val);}
  void write(double val) {
    if(singlereal) iwrite((float) val);
    else iwrite(val);
  }
  void write(const pair& val) {
    write(val.getx());
    write(val.gety());
  }
  void write(const triple& val) {
    write(val.getx());
    write(val.gety());
    write(val.getz());
  }
  
  void writeline() {}
};
  
#ifdef HAVE_RPC_RPC_H

class ixfile : public file {
protected:  
  xdr::ioxstream *fstream;
  xdr::xios::open_mode mode;
public:
  ixfile(const string& name, bool check=true, Mode type=XINPUT,
         xdr::xios::open_mode mode=xdr::xios::in) :
    file(name,check,type,true), fstream(NULL), mode(mode) {}

  void open() {
    fstream=new xdr::ioxstream(name.c_str(),mode);
    index=processData().ixfile.add(fstream);
    if(check) Check();
  }
    
  void close() {
    if(fstream) {
      fstream->close();
      closed=true;
      delete fstream;
      fstream=NULL;
      processData().ixfile.remove(index);
    }
  }
  
  ~ixfile() {close();}
  
  bool eof() {return fstream ? fstream->eof() : true;}
  bool error() {return fstream ? fstream->fail() : true;}

  void clear() {if(fstream) fstream->clear();}
  
  void seek(Int pos, bool begin=true) {
    if(!standard && fstream) {
      clear();
      fstream->seek(pos,begin ? xdr::xios::beg : xdr::xios::end);
    }
  }
  
  size_t tell() {
    if(fstream) 
      return fstream->tell();
    else
      return 0;
  }
  
  void Read(Int& val) {
    if(signedint) {
      if(singleint) {int ival=0; *fstream >> ival; val=ival;}
      else {val=0; *fstream >> val;}
    } else {
      if(singleint) {unsigned ival=0; *fstream >> ival; val=Intcast(ival);}
      else {unsignedInt ival=0; *fstream >> ival; val=Intcast(ival);}
    }
  }
  void Read(double& val) {
    if(singlereal) {float fval=0.0; *fstream >> fval; val=fval;}
    else {
      val=0.0;
      *fstream >> val;
    }
  }
  void Read(pair& val) {
    double x,y;
    Read(x);
    Read(y);
    val=pair(x,y);
  }
  void Read(triple& val) {
    double x,y,z;
    Read(x);
    Read(y);
    Read(z);
    val=triple(x,y,z);
  }
};

class ioxfile : public ixfile {
public:
  ioxfile(const string& name) : ixfile(name,true,XUPDATE,xdr::xios::out) {}

  void flush() {if(fstream) fstream->flush();}
  
  void write(Int val) {
    if(signedint) {
      if(singleint) *fstream << intcast(val);
      else *fstream << val;
    } else {
      if(singleint) *fstream << unsignedcast(val);
      else *fstream << unsignedIntcast(val);
    }
  }
  void write(double val) {
    if(singlereal) *fstream << (float) val;
    else *fstream << val;
  }
  void write(const pair& val) {
    write(val.getx());
    write(val.gety());
  }
  void write(const triple& val) {
    write(val.getx());
    write(val.gety());
    write(val.getz());
  }
};
  
class oxfile : public file {
  xdr::oxstream *fstream;
public:
  oxfile(const string& name) : file(name,true,XOUTPUT), fstream(NULL) {}

  void open() {
    fstream=new xdr::oxstream(outpath(name).c_str(),xdr::xios::trunc);
    index=processData().oxfile.add(fstream);
    Check();
  }
  
  void close() {
    if(fstream) {
      fstream->close();
      closed=true;
      delete fstream;
      fstream=NULL;
      processData().oxfile.remove(index);
    }
  }
  
  ~oxfile() {close();}
  
  bool eof() {return fstream ? fstream->eof() : true;}
  bool error() {return fstream ? fstream->fail() : true;}
  void clear() {if(fstream) fstream->clear();}
  void flush() {if(fstream) fstream->flush();}
  
  void seek(Int pos, bool begin=true) {
    if(!standard && fstream) {
      clear();
      fstream->seek(pos,begin ? xdr::xios::beg : xdr::xios::end);
    }
  }
  
  size_t tell() {
    if(fstream) 
      return fstream->tell();
    else
      return 0;
  }
  
  void write(Int val) {
    if(signedint) {
      if(singleint) *fstream << intcast(val);
      else *fstream << val;
    } else {
      if(singleint) *fstream << unsignedcast(val);
      else *fstream << unsignedIntcast(val);
    }
  }
  void write(double val) {
    if(singlereal) *fstream << (float) val;
    else *fstream << val;
  }
  void write(const pair& val) {
    write(val.getx());
    write(val.gety());
  }
  void write(const triple& val) {
    write(val.getx());
    write(val.gety());
    write(val.getz());
  }
};

#endif

extern ofile Stdout;
extern file nullfile;

} // namespace camp

#endif // FILEIO_H
