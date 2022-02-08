/*****
 * picture.cc
 * Andy Hammerlindl 2002/06/06
 *
 * Stores a picture as a list of drawElements and handles its output to
 * PostScript.
 *****/

#if defined(MIKTEX)
#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#include <miktex/Core/Directory>
#include <miktex/Core/TemporaryDirectory>

#endif
#include "errormsg.h"
#include "picture.h"
#include "util.h"
#include "settings.h"
#include "interact.h"
#include "drawverbatim.h"
#include "drawlabel.h"
#include "drawlayer.h"
#include "drawsurface.h"
#include "drawpath3.h"

#ifdef __MSDOS__
#include "sys/cygwin.h"
#endif

using std::ifstream;
using std::ofstream;
using vm::array;

using namespace settings;
using namespace gl;

texstream::~texstream() {
  string texengine=getSetting<string>("tex");
  bool context=settings::context(texengine);
  string name;
  if(!context)
    name=stripFile(outname());
  name += "texput.";
  unlink((name+"aux").c_str());
  unlink((name+"log").c_str());
  unlink((name+"out").c_str());
  if(settings::pdf(texengine)) {
    unlink((name+"pdf").c_str());
    unlink((name+"m9").c_str());
  } else
    unlink((name+"pbsdat").c_str());
  if(context) {
    unlink("cont-new.log");
    unlink((name+"tex").c_str());
    unlink((name+"top").c_str());
    unlink((name+"tua").c_str());
    unlink((name+"tui").c_str());
  }
}

namespace camp {

extern void draw();

bool isIdTransform3(const double* t)
{
  return (t == NULL || (t[0]==1 && t[1]==0 && t[2]==0 && t[3]==0 &&
                        t[4]==0 && t[5]==1 && t[6]==0 && t[7]==0 &&
                        t[8]==0 && t[9]==0 && t[10]==1 && t[11]==0 &&
                        t[12]==0 && t[13]==0 && t[14]==0 && t[15]==1));
}

// copy array to 4x4 transform matrix with range checks
void copyArray4x4C(double*& dest, const vm::array *a)
{
  double tt[16];
  const size_t n=checkArray(a);
  const string fourbyfour="4x4 array of doubles expected";
  if(n != 4) reportError(fourbyfour);

  for(size_t i=0; i < 4; i++) {
    const vm::array *ai=vm::read<vm::array*>(a,i);
    const size_t aisize=checkArray(ai);
    double *tti=tt+4*i;
    if(aisize == 4) {
      for(size_t j=0; j < 4; j++)
        tti[j]=vm::read<double>(ai,j);
    } else reportError(fourbyfour);
  }

  copyTransform3(dest,tt);
}

void copyTransform3(double*& d, const double* s, GCPlacement placement)

{
  if(s != NULL) {
    if(d == NULL)
      d=placement == NoGC ? new double[16] : new(placement) double[16];
    memcpy(d,s,sizeof(double)*16);
  }
}

// t = s*r
void multiplyTransform3(double*& t, const double* s, const double* r)
{
  if(isIdTransform3(s)) {
    copyTransform3(t,r);
  } else if(isIdTransform3(r)) {
    copyTransform3(t,s);
  } else {
    t=new(UseGC) double[16];
    for(size_t i=0; i < 4; i++) {
      size_t i4=4*i;
      const double *si=s+i4;
      const double& s0=si[0];
      const double& s1=si[1];
      const double& s2=si[2];
      const double& s3=si[3];
      double *ti=t+i4;
      ti[0]=s0*r[0]+s1*r[4]+s2*r[8]+s3*r[12];
      ti[1]=s0*r[1]+s1*r[5]+s2*r[9]+s3*r[13];
      ti[2]=s0*r[2]+s1*r[6]+s2*r[10]+s3*r[14];
      ti[3]=s0*r[3]+s1*r[7]+s2*r[11]+s3*r[15];
    }
  }
}

double xratio(const triple& v) {return v.getx()/v.getz();}
double yratio(const triple& v) {return v.gety()/v.getz();}

class matrixstack {
  mem::stack<const double*> mstack;

public:
  // return current transform
  const double* T() const
  {
    if(mstack.empty())
      return NULL;
    else
      return mstack.top();
  }
  // we store the accumulated transform of all pushed transforms
  void push(const double *r)
  {
    double* T3 = NULL;
    multiplyTransform3(T3,T(),r);
    mstack.push(T3);
  }
  void pop()
  {
    if(!mstack.empty())
      mstack.pop();
  }

};

const char *texpathmessage() {
  ostringstream buf;
  buf << "the directory containing your " << getSetting<string>("tex")
      << " engine (" << texcommand() << ")";
  return Strdup(buf.str());
}

picture::~picture()
{
}

void picture::enclose(drawElement *begin, drawElement *end)
{
  assert(begin);
  assert(end);
  nodes.push_front(begin);
  lastnumber=0;
  lastnumber3=0;

  for(nodelist::iterator p=nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    if((*p)->islayer()) {
      nodes.insert(p,end);
      ++p;
      while(p != nodes.end() && (*p)->islayer()) ++p;
      if(p == nodes.end()) return;
      nodes.insert(p,begin);
    }
  }
  nodes.push_back(end);
}

// Insert at beginning of picture.
void picture::prepend(drawElement *p)
{
  assert(p);
  nodes.push_front(p);
  lastnumber=0;
  lastnumber3=0;
}

void picture::append(drawElement *p)
{
  assert(p);
  nodes.push_back(p);
}

void picture::add(picture &pic)
{
  if (&pic == this) return;

  // STL's funny way of copying one list into another.
  copy(pic.nodes.begin(), pic.nodes.end(), back_inserter(nodes));
}

// Insert picture pic at beginning of picture.
void picture::prepend(picture &pic)
{
  if (&pic == this) return;

  copy(pic.nodes.begin(), pic.nodes.end(), inserter(nodes, nodes.begin()));
  lastnumber=0;
  lastnumber3=0;
}

bool picture::havelabels()
{
  size_t n=nodes.size();
  if(n > lastnumber && !labels && getSetting<string>("tex") != "none") {
    // Check to see if there are any labels yet
    nodelist::iterator p=nodes.begin();
    for(size_t i=0; i < lastnumber; ++i) ++p;
    for(; p != nodes.end(); ++p) {
      assert(*p);
      if((*p)->islabel()) {
        labels=true;
        break;
      }
    }
  }
  return labels;
}

bool picture::have3D()
{
  for(nodelist::iterator p=nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    if((*p)->is3D())
      return true;
  }
  return false;
}

bool picture::havepng()
{
  for(nodelist::iterator p=nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    if((*p)->svgpng())
      return true;
  }
  return false;
}

unsigned int picture::pagecount()
{
  unsigned int c=1;
  for(nodelist::iterator p=nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    if((*p)->isnewpage())
      ++c;
  }
  return c;
}

bbox picture::bounds()
{
  size_t n=nodes.size();
  if(n == lastnumber) return b_cached;

  if(lastnumber == 0) { // Maybe these should be put into a structure.
    b_cached=bbox();
    labelbounds.clear();
    bboxstack.clear();
  }

  if(havelabels()) texinit();

  nodelist::iterator p=nodes.begin();
  processDataStruct& pd=processData();

  for(size_t i=0; i < lastnumber; ++i) ++p;
  for(; p != nodes.end(); ++p) {
    assert(*p);
    (*p)->bounds(b_cached,pd.tex,labelbounds,bboxstack);

    // Optimization for interpreters with fixed stack limits.
    if((*p)->endclip()) {
      nodelist::iterator q=p;
      if(q != nodes.begin()) {
        --q;
        assert(*q);
        if((*q)->endclip())
          (*q)->save(false);
      }
    }
  }

  lastnumber=n;
  return b_cached;
}

bbox3 picture::bounds3()
{
  size_t n=nodes.size();
  if(n == lastnumber3) return b3;

  if(lastnumber3 == 0)
    b3=bbox3();

  matrixstack ms;
  size_t i=0;
  for(nodelist::const_iterator p=nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    if((*p)->begingroup3())
      ms.push((*p)->transf3());
    else if((*p)->endgroup3())
      ms.pop();
    else
      (*p)->bounds(ms.T(),b3);
    i++;
  }

  lastnumber3=n;
  return b3;
}

pair picture::ratio(double (*m)(double, double))
{
  bool first=true;
  pair b;
  bounds3();
  double fuzz=Fuzz*(b3.Max()-b3.Min()).length();
  matrixstack ms;
  for(nodelist::const_iterator p=nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    if((*p)->begingroup3())
      ms.push((*p)->transf3());
    else if((*p)->endgroup3())
      ms.pop();
    else
      (*p)->ratio(ms.T(),b,m,fuzz,first);
  }
  return b;
}

void texinit()
{
  drawElement::lastpen=pen(initialpen);
  processDataStruct &pd=processData();
  // Output any new texpreamble commands
  if(pd.tex.isopen()) {
    if(pd.TeXpipepreamble.empty()) return;
    texpreamble(pd.tex,pd.TeXpipepreamble,true);
    pd.TeXpipepreamble.clear();
    return;
  }

  bool context=settings::context(getSetting<string>("tex"));
  string dir=stripFile(outname());
  dir=dir.substr(0,dir.length()-1);
  string logname;
  if(!context) logname=dir;
  logname += "texput.log";
  const char *cname=logname.c_str();
  ofstream writeable(cname);
  if(!writeable)
    reportError("Cannot write to "+logname);
  else
    writeable.close();
  unlink(cname);

  mem::vector<string> cmd;
  cmd.push_back(texprogram());
  string oldPath;
  if(context) {
    if(!dir.empty()) {
      oldPath=getPath();
      setPath(dir.c_str());
    }
    cmd.push_back("--pipe");
  } else {
    if(!dir.empty())
      cmd.push_back("-output-directory="+dir);
    string jobname="texput";
    if(getSetting<bool>("inlineimage") || getSetting<bool>("inlinetex")) {
      string name=stripDir(stripExt((outname())));
      size_t pos=name.rfind("-");
      if(pos < string::npos) {
        name=stripExt(name).substr(0,pos);
        unlink((name+".aux").c_str());
        jobname=name.substr(0,pos);
        cmd.push_back("-jobname="+jobname);
#if defined(MIKTEX_WINDOWS) && !defined(__MSDOS__)
        cmd.push_back("nul");
#endif
#ifdef __MSDOS__
        cmd.push_back("NUL"); // For MikTeX
#endif
      }
    }
    cmd.push_back("\\scrollmode");
  }

  pd.tex.open(cmd,"texpath");
  pd.tex.wait("\n*");
  pd.tex << "\n";
  texdocumentclass(pd.tex,true);

  texdefines(pd.tex,pd.TeXpreamble,true);
  pd.TeXpipepreamble.clear();
}

int opentex(const string& texname, const string& prefix, bool dvi)
{
  string aux=auxname(prefix,"aux");
  unlink(aux.c_str());
  bool context=settings::context(getSetting<string>("tex"));
  mem::vector<string> cmd;
  cmd.push_back(texprogram());
  if(dvi)
    cmd.push_back("-output-format=dvi");
  string dir=stripFile(texname);
  dir=dir.substr(0,dir.length()-1);
  string oldPath;
  if(context) {
    if(!dir.empty()) {
      oldPath=getPath();
      setPath(dir.c_str());
    }
    cmd.push_back("--nonstopmode");
    cmd.push_back(texname);
  } else {
    if(!dir.empty())
      cmd.push_back("-output-directory="+dir);
    cmd.push_back("\\nonstopmode\\input");
    cmd.push_back(stripDir(texname));
  }

  bool quiet=verbose <= 1;
  int status=System(cmd,quiet ? 1 : 0,true,"texpath",texpathmessage());
  if(!status && getSetting<bool>("twice"))
    status=System(cmd,quiet ? 1 : 0,true,"texpath",texpathmessage());
  if(status) {
    if(quiet) {
      cmd[1]=context ? "--scrollmode" : "\\scrollmode\\input";
      System(cmd,0);
    }
  }
  if(context && !oldPath.empty())
    setPath(oldPath.c_str());
  return status;
}

string dvisvgmCommand(mem::vector<string>& cmd, const string& outname)
{
  string dir=stripFile(outname);
  string oldPath;
  if(!dir.empty()) {
    oldPath=getPath();
    setPath(dir.c_str());
  }
  cmd.push_back(getSetting<string>("dvisvgm"));
  cmd.push_back("-n");
  cmd.push_back("-v3");
  string libgs=getSetting<string>("libgs");
  if(!libgs.empty())
    cmd.push_back("--libgs="+libgs);
//  cmd.push_back("--optimize"); // Requires dvisvgm > 2.9.1
  push_split(cmd,getSetting<string>("dvisvgmOptions"));
  string outfile=stripDir(outname);
  if(!outfile.empty())
    cmd.push_back("-o"+outfile);
  return oldPath;
}

bool picture::texprocess(const string& texname, const string& outname,
                         const string& prefix, const pair& bboxshift,
                         bool svg)
{
  int status=1;
  ifstream outfile;

  outfile.open(texname.c_str());
  bool keep=getSetting<bool>("keep");

  if(outfile) {
    outfile.close();

    status=opentex(texname,prefix);
    string texengine=getSetting<string>("tex");

    if(status == 0) {
      string dviname=auxname(prefix,"dvi");
      mem::vector<string> cmd;

      if(svg) {
        string name=deconstruct ? buildname(prefix+"_%1p","svg") : outname;
        string oldPath=dvisvgmCommand(cmd,name);
        cmd.push_back(stripDir(dviname));
        if(deconstruct)
          cmd.push_back("-p1-");
        status=System(cmd,0,true,"dvisvgm");
        if(!oldPath.empty())
          setPath(oldPath.c_str());
        if(!keep)
          unlink(dviname.c_str());
      } else {
        if(!settings::pdf(texengine)) {
          string psname=auxname(prefix,"ps");
          double height=b.top-b.bottom+1.0;

          // Magic dvips offsets:
          double hoffset=-128.4;
          double vertical=height;
          if(!latex(texengine)) vertical += 2.0;
          double voffset=(vertical < 13.0) ? -137.8+vertical : -124.8;
          double paperHeight=getSetting<double>("paperheight");

          hoffset += b.left+bboxshift.getx();
          voffset += paperHeight-height-b.bottom-bboxshift.gety();

          string dvipsrc=getSetting<string>("dir");
          if(dvipsrc.empty()) dvipsrc=systemDir;
          dvipsrc += dirsep+"nopapersize.ps";
#if defined(MIKTEX_WINDOWS)
          putenv(("DVIPSRC=" + dvipsrc).c_str());
#else
          setenv("DVIPSRC",dvipsrc.c_str(),1);
#endif
          string papertype=getSetting<string>("papertype") == "letter" ?
            "letterSize" : "a4size";
          cmd.push_back(getSetting<string>("dvips"));
          cmd.push_back("-R");
          cmd.push_back("-Pdownload35");
          cmd.push_back("-D600");
          cmd.push_back("-O"+String(hoffset)+"bp,"+String(voffset)+"bp");
          bool ps=pagecount() > 1;
          cmd.push_back("-T"+String(getSetting<double>("paperwidth"))+"bp,"+
                        String(paperHeight)+"bp");
          push_split(cmd,getSetting<string>("dvipsOptions"));
          if(ps && getSetting<string>("papertype") != "")
            cmd.push_back("-t"+papertype);
          if(verbose <= 1) cmd.push_back("-q");
          cmd.push_back("-o"+psname);
          cmd.push_back(dviname);
          status=System(cmd,0,true,"dvips");
          if(status == 0) {
            ifstream fin(psname.c_str());
            psfile fout(outname,false);

            string s;
            bool first=true;
            transform t=shift(bboxshift)*T;
            bool shift=!t.isIdentity();

            const string beginspecial="TeXDict begin @defspecial";
            const size_t beginlength=beginspecial.size();
            const string endspecial="@fedspecial end";
            const size_t endlength=endspecial.size();

            bool inpapersize=false;
            while(getline(fin,s)) {

              if (inpapersize) {
                if(s.find("%%EndPaperSize") == 0)
                  inpapersize=false;
                continue;
              } else {
                if (s.find("%%BeginPaperSize:") == 0) {
                  inpapersize=true;
                  continue;
                }
              }

              if (s[0] == '%') {
                if (s.find("%%DocumentPaperSizes:") == 0)
                  continue;

                if(s.find("%!PS-Adobe-") == 0) {
                  fout.header(!ps);
                  continue;
                }

                if (first && s.find("%%BoundingBox:") == 0) {
                  bbox box=b.shift(bboxshift);
                  if(verbose > 2) BoundingBox(cout,box);
                  fout.BoundingBox(box);
                  first=false;
                  continue;
                }
              }

              if (shift) {
                if (s.compare(0, beginlength, beginspecial) == 0) {
                  fout.verbatimline(s);
                  fout.gsave();
                  fout.concat(t);
                  continue;
                }
                if (s.compare(0, endlength, endspecial) == 0) {
                  fout.grestore();
                  fout.verbatimline(s);
                  continue;
                }
              }

              // For the default line, output it unchanged.
              fout.verbatimline(s);
            }
          }
          if(!keep) {
            unlink(dviname.c_str());
            unlink(psname.c_str());
          }
        }
      }
    }

    if(!keep) {
      unlink(texname.c_str());
      if(!getSetting<bool>("keepaux"))
        unlink(auxname(prefix,"aux").c_str());
      unlink(auxname(prefix,"log").c_str());
      unlink(auxname(prefix,"out").c_str());
      string dir=stripFile(prefix);
      unlink((dir+"texput.log").c_str());
      unlink((dir+"texput.aux").c_str());
      if(settings::context(texengine)) {
        unlink(auxname(prefix,"top").c_str());
        unlink(auxname(prefix,"tua").c_str());
        unlink(auxname(prefix,"tuc").c_str());
        unlink(auxname(prefix,"tui").c_str());
        unlink(auxname(prefix,"tuo").c_str());
      }
    }
    if(status == 0) return true;
  }
  return false;
}

int picture::epstopdf(const string& epsname, const string& pdfname)
{
  string compress=getSetting<bool>("compress") ? "true" : "false";
  mem::vector<string> cmd;
  cmd.push_back(getSetting<string>("gs"));
  cmd.push_back("-q");
  cmd.push_back("-dNOPAUSE");
  cmd.push_back("-dBATCH");
  cmd.push_back("-P");
  if(safe)
    cmd.push_back("-dSAFER");
  cmd.push_back("-dALLOWPSTRANSPARENCY"); // Support transparency extensions.
  cmd.push_back("-sDEVICE=pdfwrite");
  cmd.push_back("-dEPSCrop");
  cmd.push_back("-dSubsetFonts=true");
  cmd.push_back("-dEmbedAllFonts=true");
  cmd.push_back("-dMaxSubsetPct=100");
  cmd.push_back("-dEncodeColorImages="+compress);
  cmd.push_back("-dEncodeGrayImages="+compress);
  cmd.push_back("-dCompatibilityLevel=1.4");
  if(!getSetting<bool>("autorotate"))
    cmd.push_back("-dAutoRotatePages=/None");
  cmd.push_back("-g"+String(max(ceil(getSetting<double>("paperwidth")),1.0))
                +"x"+String(max(ceil(getSetting<double>("paperheight")),1.0)));
  cmd.push_back("-dDEVICEWIDTHPOINTS="+String(max(b.right-b.left,3.0)));
  cmd.push_back("-dDEVICEHEIGHTPOINTS="+String(max(b.top-b.bottom,3.0)));
  push_split(cmd,getSetting<string>("gsOptions"));
  cmd.push_back("-sOutputFile="+stripDir(pdfname));
  if(safe) {
    cmd.push_back("-c");
    cmd.push_back(".setsafe");
    cmd.push_back("-f");
  }
  cmd.push_back(stripDir(epsname));

  char *oldPath=NULL;
  string dir=stripFile(pdfname);
  if(!dir.empty()) {
    oldPath=getPath();
    setPath(dir.c_str());
  }
  int status=System(cmd,0,true,"gs","Ghostscript");
  if(oldPath != NULL)
    setPath(oldPath);
  return status;
}

int picture::pdftoeps(const string& pdfname, const string& epsname, bool eps)
{
  mem::vector<string> cmd;
  cmd.push_back(getSetting<string>("gs"));
  cmd.push_back("-q");
  cmd.push_back("-dNOCACHE");
  cmd.push_back("-dNOPAUSE");
  cmd.push_back("-dBATCH");
  cmd.push_back("-P");
  if(safe)
    cmd.push_back("-dSAFER");
  string texengine=getSetting<string>("tex");
  cmd.push_back("-sDEVICE="+getSetting<string>(eps ? "epsdriver": "psdriver"));
  cmd.push_back("-sOutputFile="+stripDir(epsname));
  cmd.push_back(stripDir(pdfname));

  char *oldPath=NULL;
  string dir=stripFile(epsname);
  if(!dir.empty()) {
    oldPath=getPath();
    setPath(dir.c_str());
  }
  int status=System(cmd,0,true,"gs","Ghostscript");
  if(oldPath != NULL)
    setPath(oldPath);
  return status;
}

bool picture::reloadPDF(const string& Viewer, const string& outname) const
{
  static bool needReload=true;
  static bool haveReload=false;

  string reloadprefix="reload";
  if(needReload) {
    needReload=false;
    string name=getPath()+string("/")+outname;
    // Write javascript code to redraw picture.
    runString("settings.tex='pdflatex'; tex('\\ \\pdfannot width 0pt height 0pt { /AA << /PO << /S /JavaScript /JS (try{reload(\""+name+"\");} catch(e) {} closeDoc(this);) >> >> }'); shipout('"+reloadprefix+"',wait=false,view=false);erase();exit();",false);
    haveReload=true;
  }

  if(haveReload) {
    mem::vector<string> cmd;
    push_command(cmd,Viewer);
    string pdfreloadOptions=getSetting<string>("pdfreloadOptions");
    if(!pdfreloadOptions.empty())
      cmd.push_back(pdfreloadOptions);
    cmd.push_back(reloadprefix+".pdf");
    System(cmd,0,false);
  }
  return true;
}

int picture::epstosvg(const string& epsname, const string& outname,
                      unsigned int pages)
{
  string oldPath;
  int status=0;
  if(deconstruct && getSetting<bool>("dvisvgmMultipleFiles")) {
    mem::vector<string> cmd;
    oldPath=dvisvgmCommand(cmd,stripFile(outname));
    for(unsigned i=1; i <= pages; ++i) {
      ostringstream buf;
      buf << epsname << i << ".ps";
      cmd.push_back(buf.str());
    }
    cmd.push_back("-E");
    status=System(cmd,0,true,"dvisvgm");
    for(unsigned i=1; i <= pages; ++i) {
      ostringstream buf;
      buf << epsname << i << ".ps";
      if(!getSetting<bool>("keep"))
        unlink(buf.str().c_str());
    }
    if(!oldPath.empty())
      setPath(oldPath.c_str());
  } else {
    string outprefix=stripExt(outname);
    for(unsigned i=1; i <= pages; ++i) {
      mem::vector<string> cmd;
      ostringstream out;
      out << outprefix;
      if(deconstruct)
        out << "_" << i;
      out << ".svg";
      oldPath=dvisvgmCommand(cmd,out.str());
      ostringstream buf;
      buf << epsname << i << ".ps";
      cmd.push_back(buf.str());
      cmd.push_back("-E");
      status=System(cmd,0,true,"dvisvgm");
      if(!getSetting<bool>("keep"))
        unlink(buf.str().c_str());
      if(!oldPath.empty())
        setPath(oldPath.c_str());
      if(status != 0) break;
    }
  }
  return status;
}

void htmlView(string name)
{
  mem::vector<string> cmd;
  push_command(cmd,getSetting<string>("htmlviewer"));
#ifdef __MSDOS__
  ssize_t size=cygwin_conv_path(CCP_POSIX_TO_WIN_A,
                                locateFile(name,true).c_str(),NULL,0);
  if(size <= 0) return;
  char filename[size];
  size=cygwin_conv_path(CCP_POSIX_TO_WIN_A,locateFile(name,true).c_str(),
                        filename,size);
  cmd.push_back("file://"+string(filename));
#else
  cmd.push_back(locateFile(name,true));
#endif
  push_split(cmd,getSetting<string>("htmlviewerOptions"));
  System(cmd,2,false);
}

bool picture::postprocess(const string& prename, const string& outname,
                          const string& outputformat,
                          bool wait, bool view, bool pdftex,
                          bool epsformat, bool svg)
{
  int status=0;
  bool pdf=settings::pdf(getSetting<string>("tex"));
  bool pdfformat=(pdf && outputformat == "") || outputformat == "pdf";

  mem::vector<string> cmd;
  if(pdftex || !epsformat) {
    if(pdfformat) {
      if(pdftex) {
        status=rename(prename.c_str(),outname.c_str());
        if(status != 0)
          reportError("Cannot rename "+prename+" to "+outname);
      } else status=epstopdf(prename,outname);
    } else if(epsformat) {
      if(svg) {
        string psname=stripExt(prename);
        status=pdftoeps(prename,psname+"%d.ps",false);
        if(status != 0) return false;
        status=epstosvg(stripDir(psname),outname,pagecount());
        if(status != 0) return false;
        epsformat=false;
      } else
        status=pdftoeps(prename,outname);
    } else {
      double render=fabs(getSetting<double>("render"));
      if(render == 0) render=1.0;
      double res=render*72.0;
      Int antialias=getSetting<Int>("antialias");
      if(outputformat == "png" && antialias == 2) {
        cmd.push_back(getSetting<string>("gs"));
        cmd.push_back("-q");
        cmd.push_back("-dNOPAUSE");
        cmd.push_back("-dBATCH");
        cmd.push_back("-P");
        cmd.push_back("-sDEVICE="+getSetting<string>("pngdriver"));
        if(safe)
          cmd.push_back("-dSAFER");
        cmd.push_back("-r"+String(res)+"x"+String(res));
        push_split(cmd,getSetting<string>("gsOptions"));
        cmd.push_back("-sOutputFile="+outname);
        cmd.push_back(prename);
        status=System(cmd,0,true,"gs","Ghostscript");
      } else if(!svg && !getSetting<bool>("xasy")) {
        double expand=antialias;
        if(expand < 2.0) expand=1.0;
        res *= expand;
        cmd.push_back(getSetting<string>("convert"));
        cmd.push_back("-density");
        cmd.push_back(String(res)+"x"+String(res));
        if(expand == 1.0)
          cmd.push_back("+antialias");
        push_split(cmd,getSetting<string>("convertOptions"));
        cmd.push_back("-resize");
        cmd.push_back(String(100.0/expand)+"%x");
        if(outputformat == "jpg") cmd.push_back("-flatten");
        cmd.push_back(prename);
        cmd.push_back(outputformat+":"+outname);
        status=System(cmd,0,true,"convert");
      }
    }
    if(!getSetting<bool>("keep"))
      unlink(prename.c_str());
  }
  if(status != 0) return false;

  if(verbose > 0 && !deconstruct)
    cout << "Wrote " << outname << endl;

  return display(outname,outputformat,wait,view,epsformat);
}

bool picture::display(const string& outname, const string& outputformat,
                      bool wait, bool view, bool epsformat)
{
  int status=0;
  static mem::map<CONST string,int> pids;
  bool View=settings::view() && view;

  if(View) {
    bool pdf=settings::pdf(getSetting<string>("tex"));
    bool pdfformat=(pdf && outputformat == "") || outputformat == "pdf";

    if(epsformat || pdfformat) {
      // Check to see if there is an existing viewer for this outname.
      mem::map<CONST string,int>::iterator p=pids.find(outname);
      bool running=(p != pids.end());
      string Viewer=pdfformat ? getSetting<string>("pdfviewer") :
        getSetting<string>("psviewer");
      int pid;
      if(running) {
        pid=p->second;
#if defined(MIKTEX)
        // MIKTEX-TODO
#else
        if(pid)
          running=(waitpid(pid, &status, WNOHANG) != pid);
#endif
      }

      bool pdfreload=pdfformat && getSetting<bool>("pdfreload");
      if(running) {
        // Tell gv/acroread to reread file.
#if defined(MIKTEX)
        // MIKTEX-TODO
        if (Viewer == "gv");
#else
        if(Viewer == "gv") kill(pid,SIGHUP);
#endif
        else if(pdfreload)
          reloadPDF(Viewer,outname);
      } else {
        mem::vector<string> cmd;
        push_command(cmd,Viewer);
        string viewerOptions=getSetting<string>(pdfformat ?
                                                "pdfviewerOptions" :
                                                "psviewerOptions");
        if(!viewerOptions.empty())
          push_split(cmd,viewerOptions);
        cmd.push_back(outname);
        status=System(cmd,0,wait,
                      pdfformat ? "pdfviewer" : "psviewer",
                      pdfformat ? "your PDF viewer" : "your PostScript viewer",
                      &pid);
        if(status != 0) return false;

        if(!wait) pids[outname]=pid;

        if(pdfreload) {
          // Work around race conditions in acroread initialization script
          usleep(getSetting<Int>("pdfreloaddelay"));
          // Only reload if pdf viewer process is already running.
#if defined(MIKTEX_WINDOWS)
          // MIKTEX-TODO
#else
          if(waitpid(pid, &status, WNOHANG) == pid)
            reloadPDF(Viewer,outname);
#endif
        }
      }
    } else {
      if(outputformat == "svg" || outputformat == "html")
        htmlView(outname);
      else {
        mem::vector<string> cmd;
        push_command(cmd,getSetting<string>("display"));
        cmd.push_back(outname);
        string application="your "+outputformat+" viewer";
        status=System(cmd,0,wait,"display",application.c_str());
        if(status != 0) return false;
      }
    }
  }

  return true;
}

string Outname(const string& prefix, const string& outputformat,
               bool standardout, string aux="")
{
  return standardout ? "-" : buildname(prefix,outputformat,aux);
}

bool picture::shipout(picture *preamble, const string& Prefix,
                      const string& format, bool wait, bool view)
{
  bool keep=getSetting<bool>("keep");

  string aux="";
  b=bounds();
  bool empty=b.empty;

  string outputformat=format.empty() ? defaultformat() : format;

  bool htmlformat=outputformat == "html";
  if(htmlformat) {
    outputformat="svg";
    aux="_";
    if(view) view=false;
    else htmlformat=false;
  }

#ifndef HAVE_LIBGLM
  if(outputformat == "v3d")
    camp::reportError("to support V3D rendering, please install glm header files, then ./configure; make");
#endif

  bool svgformat=outputformat == "svg";
  bool png=outputformat == "png";

  string texengine=getSetting<string>("tex");

  string texengineSave;

  if(!empty && !deconstruct && (png || (svgformat && havepng())) &&
     texengine == "latex") {
    texengineSave=texengine;
    Setting("tex")=texengine="pdflatex";
  }

  bool usetex=texengine != "none";
  bool TeXmode=getSetting<bool>("inlinetex") && usetex;
  bool pdf=settings::pdf(texengine);

  bool standardout=Prefix == "-";
  string prefix=standardout ? standardprefix : stripExt(Prefix);

  string preformat=nativeformat();
  bool epsformat=outputformat == "eps";
  bool pdfformat=pdf || png || outputformat == "pdf";
  bool dvi=false;
  bool svg=svgformat && usetex &&
    (!have3D() || getSetting<double>("render") == 0.0);
  if(svg) {
    if(pdf) epsformat=true;
    else dvi=true;
  }

  string outname=Outname(prefix,outputformat,standardout,aux);
  string epsname=epsformat ? (standardout ? "" : outname) :
    auxname(prefix,"eps");

  bool Labels=labels || TeXmode;

  bool Empty=b.right-b.left < 1.0 || b.top-b.bottom < 1.0;
  if(png && Empty)
    empty=true;

  if(Empty && (svgformat || htmlformat)) return true;

  if(empty && !Labels) { // Output a null file
    bbox b;
    b.left=b.bottom=0;
    b.right=b.top=1;
    psfile out(epsname,false);
    out.prologue(b);
    out.epilogue();
    out.close();
    return postprocess(epsname,outname,outputformat,wait,view,false,
                       epsformat,false);
  }

  if(svg || png)
    Labels=true;

  if(Labels)
    prefix=cleanpath(prefix);

  string prename=((epsformat && !pdf) || !Labels) ? epsname :
    auxname(prefix,preformat);

  SetPageDimensions();

  pair aligndir=getSetting<pair>("aligndir");
  string origin=getSetting<string>("align");

  pair bboxshift=(origin == "Z" && epsformat) ?
    pair(0.0,0.0) : pair(-b.left,-b.bottom);

  if(epsformat) {
    bboxshift += getSetting<pair>("offset");
    double yexcess=max(getSetting<double>("paperheight")-
                       (b.top-b.bottom+1.0),0.0);
    double xexcess=max(getSetting<double>("paperwidth")-
                       (b.right-b.left+1.0),0.0);
    if(aligndir == pair(0,0)) {
      if(origin != "Z" && origin != "B") {
        if(origin == "T") bboxshift += pair(0.0,yexcess);
        else bboxshift += pair(0.5*xexcess,0.5*yexcess);
      }
    } else {
      double scale=max(fabs(aligndir.getx()),fabs(aligndir.gety()));
      if(scale != 0) aligndir *= 0.5/scale;
      bboxshift +=
        pair((aligndir.getx()+0.5)*xexcess,(aligndir.gety()+0.5)*yexcess);
    }
  }

  bool status=true;

  string texname;
  texfile *tex=NULL;

  if(Labels) {
    texname=TeXmode ? buildname(prefix,"tex") : auxname(prefix,"tex");
    tex=dvi ?
      new svgtexfile(texname,b,false,deconstruct) :
      new texfile(texname,b);
    tex->prologue(deconstruct);
  }

  nodelist::iterator layerp=nodes.begin();
  nodelist::iterator p=layerp;
  unsigned layer=0;
  mem::list<string> files;

  bbox bshift=b;

  int svgcount=0;

  typedef mem::list<drawElement *> clipstack;
  clipstack begin;

  while(p != nodes.end()) {
    string psname,pdfname;
    if(Labels) {
      ostringstream buf;
      buf << prefix << "_" << layer;
      psname=buildname(buf.str(),"eps");
      if(pdf) pdfname=buildname(buf.str(),"pdf");
    } else {
      psname=epsname;
      bshift=bshift.shift(bboxshift);
    }
    files.push_back(psname);
    if(pdf) files.push_back(pdfname);
    psfile out(psname,pdfformat);
    out.prologue(bshift);

    if(!Labels) {
      out.gsave();
      out.translate(bboxshift);
    }

    if(preamble) {
      // Postscript preamble.
      nodelist Nodes=preamble->nodes;
      nodelist::iterator P=Nodes.begin();
      if(P != Nodes.end()) {
        out.resetpen();
        for(; P != Nodes.end(); ++P) {
          assert(*P);
          (*P)->draw(&out);
        }
      }
    }
    out.resetpen();

    bool postscript=false;
    drawLabel *L=NULL;

    if(dvi)
      for(nodelist::const_iterator r=begin.begin(); r != begin.end(); ++r)
        (*r)->draw(&out);

    processDataStruct &pd=processData();

    for(; p != nodes.end(); ++p) {
      assert(*p);
      if(Labels && (*p)->islayer()) break;

      if(dvi && (*p)->svg()) {
        picture *f=(*p)->svgpng() ? new picture : NULL;
        nodelist::const_iterator q=layerp;
        for(;;) {
          if((*q)->beginclip())
            begin.push_back(*q);
          else if((*q)->endclip()) {
            if(begin.size() < 1)
              reportError("endclip without matching beginclip");
            begin.pop_back();
          }
          if(q == p) break;
          ++q;
        }

        if(f) {
          for(nodelist::const_iterator r=begin.begin(); r != begin.end(); ++r)
            f->append(*r);

          f->append(*(q++));
        }

        while(q != nodes.end() && !(*q)->islayer()) ++q;

        clipstack end;

        for(nodelist::const_iterator r=--q;; --r) {
          if((*r)->beginclip() && end.size() >= 1)
            end.pop_back();
          else if((*r)->endclip())
            end.push_back(*r);
          if(r == p) break;
        }

        for(nodelist::reverse_iterator r=end.rbegin(); r != end.rend();
            ++r) {
          (*r)->draw(&out);
          if(f)
            f->append(*r);
        }

        if(f) {
          ostringstream buf;
          buf << prefix << "_" << svgcount;
          ++svgcount;
          string pngname=buildname(buf.str(),"png");
          f->shipout(preamble,buf.str(),"png",false,false);
          pair m=f->bounds().Min();
          pair M=f->bounds().Max();
          delete f;

          pair size=M-m;
          ostringstream cmd;
          cmd << "\\special{dvisvgm:img " << size.getx()*ps2tex << " "
              << size.gety()*ps2tex << " " << pngname << "}";
          static pen P;
          static pair zero;
          L=new drawLabel(cmd.str(),"",identity,pair(m.getx(),M.gety()),zero,P);
          texinit();
          L->bounds(b_cached,pd.tex,labelbounds,bboxstack);
          postscript=true;
        }
        break;
      } else postscript |= (*p)->draw(&out);
    }

    if(Labels) {
      if(!svg || pdf)
        tex->beginlayer(pdf ? pdfname : psname,postscript);
    } else out.grestore();

    out.epilogue();
    out.close();

    if(Labels) {
      tex->resetpen();
      if(pdf && !b.empty) {
        status=(epstopdf(psname,pdfname) == 0);
        if(!keep) unlink(psname.c_str());
      }

      if(status) {
        for (p=layerp; p != nodes.end(); ++p) {
          assert(*p);
          bool islayer=(*p)->islayer();
          if(dvi && (*p)->svg()) {
            islayer=true;
            if((*p)->svgpng())
              L->write(tex,b);
            else
              (*p)->draw(tex);
          } else
            (*p)->write(tex,b);
          if(islayer) {
            tex->endlayer();
            layerp=++p;
            layer++;
            break;
          }
        }
      }
    }
  }

  bool context=settings::context(texengine);
  if(status) {
    if(TeXmode) {
      if(Labels && verbose > 0) cout << "Wrote " << texname << endl;
      delete tex;
    } else {
      if(Labels) {
        tex->epilogue();
        if(context) prefix=stripDir(prefix);
        status=texprocess(texname,dvi ? outname : prename,prefix,
                          bboxshift,dvi);
        delete tex;
        if(!keep) {
          for(mem::list<string>::iterator p=files.begin(); p != files.end();
              ++p)
            unlink(p->c_str());
        }
      }
      if(status) {
        if(context) prename=stripDir(prename);
        status=postprocess(prename,outname,outputformat,wait,
                           view,pdf && Labels,epsformat,svg);
        if(pdfformat && !keep) {
          unlink(auxname(prefix,"m9").c_str());
          unlink(auxname(prefix,"pbsdat").c_str());
        }
      }
    }
  }

  if(!status) reportError("shipout failed");

  if(!texengineSave.empty()) Setting("tex")=texengineSave;

  if(htmlformat) {
    jsfile out;
    out.svgtohtml(prefix);
    string name=buildname(prefix,"html");
    display(name,"html",wait,true,false);
    if(!keep)
      unlink(outname.c_str());
  }

  return true;
}

// render viewport with width x height pixels.
void picture::render(double size2, const triple& Min, const triple& Max,
                     double perspective, bool remesh) const
{
  for(nodelist::const_iterator p=nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    if(remesh) (*p)->meshinit();
    (*p)->render(size2,Min,Max,perspective,remesh);
  }

#ifdef HAVE_GL
  drawBuffers();
#endif
}

struct Communicate : public gc {
  string prefix;
  picture* pic;
  string format;
  double width;
  double height;
  double angle;
  double zoom;
  triple m;
  triple M;
  pair shift;
  pair margin;
  double *t;
  double *background;
  size_t nlights;
  triple *lights;
  double *diffuse;
  double *specular;
  bool view;
};

Communicate com;

extern bool allowRender;

void glrenderWrapper()
{
#ifdef HAVE_GL
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  wait(initSignal,initLock);
  endwait(initSignal,initLock);
#endif
  if(allowRender)
    glrender(com.prefix,com.pic,com.format,com.width,com.height,com.angle,
             com.zoom,com.m,com.M,com.shift,com.margin,com.t,com.background,
             com.nlights,com.lights,com.diffuse,com.specular,com.view);
#endif
}

bool picture::shipout3(const string& prefix, const string& format,
                       double width, double height, double angle, double zoom,
                       const triple& m, const triple& M, const pair& shift,
                       const pair& margin, double *t, double *background,
                       size_t nlights, triple *lights, double *diffuse,
                       double *specular, bool view)
{
  if(getSetting<bool>("interrupt"))
    return true;

  if(width <= 0 || height <= 0) return false;

  bool webgl=format == "html";

#ifndef HAVE_LIBGLM
  if(webgl)
    camp::reportError("to support WebGL rendering, please install glm header files, then ./configure; make");
#endif

#ifndef HAVE_LIBOSMESA
#ifndef HAVE_GL
  if(!webgl)
    camp::reportError("to support onscreen OpenGL rendering; please install the glut library, then ./configure; make");
#endif
#endif


  picture *pic = new picture;

  matrixstack ms;
  for(nodelist::const_iterator p=nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    if((*p)->begingroup3())
      ms.push((*p)->transf3());
    else if((*p)->endgroup3())
      ms.pop();
    else
      pic->append((*p)->transformed(ms.T()));
  }

  pic->b3=bbox3();
  for(nodelist::iterator p=pic->nodes.begin(); p != pic->nodes.end(); ++p) {
    assert(*p);
    (*p)->bounds(pic->b3);
  }
  pic->lastnumber3=pic->nodes.size();

  for(nodelist::iterator p=pic->nodes.begin(); p != pic->nodes.end(); ++p) {
    assert(*p);
    (*p)->displacement();
  }

  const string outputformat=format.empty() ?
    getSetting<string>("outformat") : format;

#ifdef HAVE_LIBGLM
  static int oldpid=0;
  bool View=settings::view() && view;
#endif

#ifdef HAVE_GL
  bool offscreen=false;
#ifdef HAVE_LIBOSMESA
  offscreen=true;
#endif
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  bool animating=getSetting<bool>("animating");
  bool Wait=!interact::interactive || !View || animating;
#endif
#endif

  bool v3d=format == "v3d";
  bool format3d=webgl || v3d;

  if(!format3d) {
#ifdef HAVE_GL
    if(glthread && !offscreen) {
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
      if(gl::initialize) {
        gl::initialize=false;
        com.prefix=prefix;
        com.pic=pic;
        com.format=outputformat;
        com.width=width;
        com.height=height;
        com.angle=angle;
        com.zoom=zoom;
        com.m=m;
        com.M=M;
        com.shift=shift;
        com.margin=margin;
        com.t=t;
        com.background=background;
        com.nlights=nlights;
        com.lights=lights;
        com.diffuse=diffuse;
        com.specular=specular;
        com.view=View;
        if(Wait)
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
          readyLock.lock();
#else
          pthread_mutex_lock(&readyLock);
#endif
        wait(initSignal,initLock);
        endwait(initSignal,initLock);
        static bool initialize=true;
        if(initialize) {
          wait(initSignal,initLock);
          endwait(initSignal,initLock);
          initialize=false;
        }
        if(Wait) {
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
          std::unique_lock<std::mutex> lck(readyLock, std::adopt_lock);
          readySignal.wait(lck);
#else
          pthread_cond_wait(&readySignal,&readyLock);
          pthread_mutex_unlock(&readyLock);
#endif
        }
        return true;
      }
      if(Wait)
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
        std::unique_lock<std::mutex> lck(readyLock, std::adopt_lock);
#else
        pthread_mutex_lock(&readyLock);
#endif
#endif
    } else {
#if defined(MIKTEX_WINDOWS)
      // MIKTEX-TODO
      int pid = -1;
#else
      int pid=fork();
#endif
#if defined(MIKTEX_WINDOWS)
      // MIKTEX-TODO
#else
      if(pid == -1)
        camp::reportError("Cannot fork process");
      if(pid != 0)  {
        oldpid=pid;
        waitpid(pid,NULL,interact::interactive && View ? WNOHANG : 0);
        return true;
      }
#endif
    }
#endif
  }

#if HAVE_LIBGLM
  glrender(prefix,pic,outputformat,width,height,angle,zoom,m,M,shift,margin,t,
           background,nlights,lights,diffuse,specular,View,oldpid);

  if(format3d) {
    string name=buildname(prefix,format);
    abs3Doutfile *fileObj=nullptr;

    if(webgl)
      fileObj=new jsfile(name);
    else if(v3d)
#ifdef HAVE_RPC_RPC_H
      fileObj=new gzv3dfile(name,getSetting<bool>("lossy") ||
                            getSetting<double>("prerender") > 0.0);
#else
    {
    ostringstream buf;
    buf << name << ": XDR write support not enabled";
    reportError(buf);
    }
#endif

    if(fileObj) {
      for (auto& p : pic->nodes) {
        assert(p);
        p->write(fileObj);
      }

      fileObj->close();
      delete fileObj;
    }

    if(webgl && View)
      htmlView(name);

#ifdef HAVE_GL
    if(format3dWait) {
      gl::format3dWait=false;
#ifdef HAVE_PTHREAD
      endwait(initSignal,initLock);
#endif
    }
#endif

    return true;
  }
#endif

#ifdef HAVE_GL
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  if(glthread && !offscreen && Wait) {
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
    std::unique_lock<std::mutex> lck(readyLock, std::adopt_lock);
    readySignal.wait(lck);
#else
    pthread_cond_wait(&readySignal,&readyLock);
    pthread_mutex_unlock(&readyLock);
#endif
  }
  return true;
#endif
#endif

  return false;
}

bool picture::shipout3(const string& prefix, const string format)
{
  bounds3();
  bool status;

  string name=buildname(prefix,"prc");
  prcfile prc(name);

  static const double limit=2.5*10.0/INT_MAX;
  double compressionlimit=max(length(b3.Max()),length(b3.Min()))*limit;

  groups.push_back(groupmap());
  for(nodelist::iterator p=nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    (*p)->write(&prc,&billboard,compressionlimit,groups);
  }
  groups.pop_back();
  status=prc.finish();

  if(!status) reportError("shipout3 failed");

  if(verbose > 0) cout << "Wrote " << name << endl;

  return true;
}

picture *picture::transformed(const transform& t)
{
  picture *pic = new picture;

  nodelist::iterator p;
  for (p = nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    pic->append((*p)->transformed(t));
  }
  pic->T=transform(t*T);

  return pic;
}

picture *picture::transformed(const array& t)
{
  picture *pic = new picture;
  double* T=NULL;
  copyArray4x4C(T,&t);
  size_t level = 0;
  for (nodelist::iterator p = nodes.begin(); p != nodes.end(); ++p) {
    assert(*p);
    if(level==0)
      pic->append((*p)->transformed(T));
    else
      pic->append(*p);
    if((*p)->begingroup3())
      level++;
    if((*p)->endgroup3()) {
      if(level==0)
        reportError("endgroup3 without matching begingroup3");
      else
        level--;
    }
  }

  return pic;
}


} // namespace camp
