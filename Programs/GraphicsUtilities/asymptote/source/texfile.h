/*****
 * texfile.h
 * John Bowman 2003/03/14
 *
 * Encapsulates the writing of commands to a TeX file.
 *****/

#ifndef TEXFILE_H
#define TEXFILE_H

#include <fstream>
#include <iomanip>
#include <iostream>

#include "common.h"
#include "pair.h"
#include "bbox.h"
#include "pen.h"
#include "util.h"
#include "interact.h"
#include "path.h"
#include "array.h"
#include "psfile.h"
#include "settings.h"
#include "asyprocess.h"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

namespace camp {

template<class T>
void texdocumentclass(T& out, bool pipe=false)
{
  if(settings::latex(settings::getSetting<string>("tex")) &&
     (pipe || !settings::getSetting<bool>("inlinetex")))
    out << "\\documentclass[12pt]{article}" << newl;
}

template<class T>
void texuserpreamble(T& out,
                     mem::list<string>& preamble=processData().TeXpreamble,
                     bool pipe=false)
{
  for(mem::list<string>::iterator p=preamble.begin(); p != preamble.end();
      ++p) {
    out << stripblanklines(*p);
    if(pipe) out << newl << newl;
  }
}

template<class T>
void latexfontencoding(T& out)
{
  out << "\\makeatletter%" << newl
      << "\\let\\ASYencoding\\f@encoding%" << newl
      << "\\let\\ASYfamily\\f@family%" << newl
      << "\\let\\ASYseries\\f@series%" << newl
      << "\\let\\ASYshape\\f@shape%" << newl
      << "\\makeatother%" << newl;
}

std::unordered_set const latexCharacters = {'#', '$', '%', '&', '\\', '^', '_', '{', '}', '~'};

template<class T>
void texpreamble(T& out, mem::list<string>& preamble=processData().TeXpreamble,
                 bool pipe=false, bool ASYbox=true)
{
  texuserpreamble(out,preamble,pipe);
  string texengine=settings::getSetting<string>("tex");
  string outPath=stripFile(settings::outname());
  if(settings::context(texengine))
    out << "\\disabledirectives[system.errorcontext]%" << newl;
  if(ASYbox)
    out << "\\newbox\\ASYbox" << newl
        << "\\newdimen\\ASYdimen" << newl;
  out << "\\def\\ASYprefix{" << escapeCharacters(outPath, latexCharacters) << "}" << newl
      << "\\long\\def\\ASYbase#1#2{\\leavevmode\\setbox\\ASYbox=\\hbox{#1}%"
      << "\\ASYdimen=\\ht\\ASYbox%" << newl
      << "\\setbox\\ASYbox=\\hbox{#2}\\lower\\ASYdimen\\box\\ASYbox}" << newl;
  if(!pipe)
    out << "\\long\\def\\ASYaligned(#1,#2)(#3,#4)#5#6#7{\\leavevmode%" << newl
        << "\\setbox\\ASYbox=\\hbox{#7}%" << newl
        << "\\setbox\\ASYbox\\hbox{\\ASYdimen=\\ht\\ASYbox%" << newl
        << "\\advance\\ASYdimen by\\dp\\ASYbox\\kern#3\\wd\\ASYbox"
        << "\\raise#4\\ASYdimen\\box\\ASYbox}%" << newl
        << "\\setbox\\ASYbox=\\hbox{#5\\wd\\ASYbox 0pt\\dp\\ASYbox 0pt\\ht\\ASYbox 0pt\\box\\ASYbox#6}%" << newl
        << "\\hbox to 0pt{\\kern#1pt\\raise#2pt\\box\\ASYbox\\hss}}%" << newl
        << "\\long\\def\\ASYalignT(#1,#2)(#3,#4)#5#6{%" << newl
        << "\\ASYaligned(#1,#2)(#3,#4){%" << newl
        << settings::beginlabel(texengine) << "%" << newl
        << "}{%" << newl
        << settings::endlabel(texengine) << "%" << newl
        << "}{#6}}" << newl
        << "\\long\\def\\ASYalign(#1,#2)(#3,#4)#5{"
        << "\\ASYaligned(#1,#2)(#3,#4){}{}{#5}}" << newl
        << settings::rawpostscript(texengine) << newl;
}

// Work around bug in dvips.def: allow spaces in file names.
template<class T>
void dvipsfix(T &out)
{
  if(!settings::pdf(settings::getSetting<string>("tex"))) {
    out << "\\makeatletter" << newl
        << "\\def\\Ginclude@eps#1{%" << newl
        << " \\message{<#1>}%" << newl
        << "  \\bgroup" << newl
        << "  \\def\\@tempa{!}%" << newl
        << "  \\dimen@\\Gin@req@width" << newl
        << "  \\dimen@ii.1bp%" << newl
        << "  \\divide\\dimen@\\dimen@ii" << newl
        << "  \\@tempdima\\Gin@req@height" << newl
        << "  \\divide\\@tempdima\\dimen@ii" << newl
        << "    \\special{PSfile=#1\\space" << newl
        << "      llx=\\Gin@llx\\space" << newl
        << "      lly=\\Gin@lly\\space" << newl
        << "      urx=\\Gin@urx\\space" << newl
        << "      ury=\\Gin@ury\\space" << newl
        << "      \\ifx\\Gin@scalex\\@tempa\\else rwi=\\number\\dimen@\\space\\fi" << newl
        << "      \\ifx\\Gin@scaley\\@tempa\\else rhi=\\number\\@tempdima\\space\\fi" << newl
        << "      \\ifGin@clip clip\\fi}%" << newl
        << "  \\egroup}" << newl
        << "\\makeatother" << newl;
  }
}

template<class T>
void texdefines(T& out, mem::list<string>& preamble=processData().TeXpreamble,
                bool pipe=false)
{
  string texengine=settings::getSetting<string>("tex");
  bool latex=settings::latex(texengine);
  bool inlinetex=settings::getSetting<bool>("inlinetex");
  if(pipe || !inlinetex) {
    bool lua=settings::lua(texengine);
    if(latex) {
      if(lua) {
        out << "\\edef\\pdfpageattr{\\pdfvariable pageattr}" << newl
            << "\\ifx\\pdfpagewidth\\undefined\\let\\pdfpagewidth\\paperwidth"
            << "\\fi" << newl
            << "\\ifx\\pdfpageheight\\undefined\\let\\pdfpageheight"
            << "\\paperheight"
            << "\\fi" << newl
            << "\\usepackage{graphicx}" << newl;
      } else {
        out << "\\let\\paperwidthsave\\paperwidth\\let\\paperwidth\\undefined"
            << newl
            << "\\usepackage{graphicx}" << newl
            << "\\let\\paperwidth\\paperwidthsave" << newl;
      }
    } else {
      if(lua) {
        out << "\\edef\\pdfpageattr{\\pdfvariable pageattr}" << newl
            << "\\ifx\\pdfpagewidth\\undefined\\let\\pdfpagewidth\\pagewidth"
            << "\\fi" << newl
            << "\\ifx\\pdfpageheight\\undefined\\let\\pdfpageheight"
            << "\\pageheight"
            << "\\fi" << newl;
      }
    }
    texpreamble(out,preamble,pipe);
  }

  if(pipe) {
    // Make tex pipe aware of a previously generated aux file.
    string name=auxname(settings::outname(),"aux");
#if defined(MIKTEX_WINDOWS)
    std::ifstream fin(UW_(name.c_str()));
#else
    std::ifstream fin(name.c_str());
#endif
    if(fin) {
      std::ofstream fout("texput.aux");
      string s;
      while(getline(fin,s))
        fout << s << endl;
    }
  }

  if(latex) {
    if(!inlinetex) {
      dvipsfix(out);
    }
    if(pipe) {
      out << "\\begin{document}" << newl;
      latexfontencoding(out);
    }
  } else if(!settings::context(texengine)) {
    out << "\\input graphicx" << newl // Fix miniltx path parsing bug:
        << "\\makeatletter" << newl
        << "\\def\\filename@parse#1{%" << newl
        << "  \\let\\filename@area\\@empty" << newl
        << "  \\expandafter\\filename@path#1/\\\\}" << newl
        << "\\def\\filename@path#1/#2\\\\{%" << newl
        << "  \\ifx\\\\#2\\\\%" << newl
        << "     \\def\\reserved@a{\\filename@simple#1.\\\\}%" << newl
        << "  \\else" << newl
        << "     \\edef\\filename@area{\\filename@area#1/}%" << newl
        << "     \\def\\reserved@a{\\filename@path#2\\\\}%" << newl
        << "  \\fi" << newl
        << "  \\reserved@a}" << newl
        << "\\makeatother" << newl;
    dvipsfix(out);

    if(!pipe)
      out << "\\input picture" << newl;
  }
}

template<class T>
bool setlatexfont(T& out, const pen& p, const pen& lastpen)
{
  if(p.size() != lastpen.size() || p.Lineskip() != lastpen.Lineskip()) {
    out <<  "\\fontsize{" << p.size()*settings::ps2tex << "}{"
        << p.Lineskip()*settings::ps2tex << "}\\selectfont%" << newl;
    return true;
  }
  return false;
}

template<class T>
bool settexfont(T& out, const pen& p, const pen& lastpen, bool latex)
{
  string font=p.Font();
  if(font != lastpen.Font() || (!latex && p.size() != lastpen.size())) {
    out << font << "%" << newl;
    return true;
  }
  return false;
}

class texfile : public psfile {
protected:
  bbox box;
  bool inlinetex;
  double Hoffset;
  int level;
  bool pdf;

public:
  string texengine;

  texfile(const string& texname, const bbox& box, bool pipe=false);
  virtual ~texfile();

  void prologue(bool deconstruct=false);
  virtual void beginpage() {}

  void epilogue(bool pipe=false);
  virtual void endpage() {}

  void setpen(pen p);

  void setfont(pen p);

  void gsave(bool tex=true);

  void grestore(bool tex=true);

  void beginspecial();

  void endspecial();

  void special(const string &s);

  void beginraw();

  void endraw();

  void begingroup() {++level;}

  void endgroup() {--level;}

  bool toplevel() {return level == 0;}

  virtual void beginpicture(const bbox& b);
  void endpicture(const bbox& b, bool newPage=false);

  virtual void newpage(const bbox&) {
    verbatimline(settings::newpage(texengine));
  }

  void BBox(const bbox& b) {
    bbox B=b.shift(pair(-hoffset(),-voffset()));
    if(pdf) {
      if(settings::xe(texengine))
        *out << "\\special{pdf: put @thispage <</MediaBox [" << B << "]>>}%"
             << newl;
      else
        if(settings::context(texengine)) {
          double width=B.right-B.left;
          double height=B.top-B.bottom;
          *out << "\\definepapersize[asy]["
               << "width=" << width << "bp,"
               << "height=" << height << "bp]%" << newl
               << "\\setuppapersize[asy][asy]%" << newl
               << "\\setuplayout["
               << "backspace=" << -B.left << "bp,"
               << "topspace=" << B.top-(box.top-box.bottom) << "bp]%" << newl;
        } else
          *out << "\\pdfpageattr{/MediaBox [" << B << "]}%" << newl;
    }
  }

  void writepair(pair z) {
    *out << z;
  }

  void miniprologue();

  void writeshifted(path p, bool newPath=true);
  virtual double hoffset() {return Hoffset;}
  virtual double voffset() {return box.bottom;}

  // Draws label transformed by T at position z.
  void put(const string& label, const transform& T, const pair& z,
           const pair& Align);

  void beginlayer(const string& psname, bool postscript);
  void endlayer();

  virtual void Offset(const bbox& box, bool special=false) {};
};

class svgtexfile : public texfile {
  mem::stack<size_t> clipstack;
  size_t clipcount;
  size_t gradientcount;
  size_t gouraudcount;
  size_t tensorcount;
  bool inspecial;
  static string nl;
  pair offset;
  bool first;
  bool deconstruct;
public:
  svgtexfile(const string& texname, const bbox& box, bool pipe=false,
             bool deconstruct=false) :
    texfile(texname,box,pipe), deconstruct(deconstruct) {
    inspecial=false;

    *out << "\\catcode`\\%=12" << newl
         << "\\def\\percent{%}" << newl
         << "\\catcode`\\%=14" << newl;

    first=true;
    Offset(box);
  }

  void Offset(const bbox& b, bool special=false) {
    box=b;
    if(special) {
      texfile::beginpicture(b);
      pair bboxshift=pair(-2*b.left,b.top-b.bottom);
      bbox b0=svgbbox(b,bboxshift);
      *out << "\\special{dvisvgm:bbox f "
           << b0.left << "bp "
           << b0.bottom << "bp "
           << b0.right << "bp "
           << b0.top << "bp}%" << newl;
    }

    Hoffset=inlinetex ? box.right : box.left;
    offset=pair(box.left,box.top);
    clipstack=mem::stack<size_t>();
    clipcount=0;
    gradientcount=0;
    gouraudcount=0;
    tensorcount=0;
  }

  void beginpicture(const bbox& b) {
    Offset(b,true);
  }

  void newpage(const bbox& b) {
    if(deconstruct) {
      if(first)
        first=false;
      else
        endpicture(b,true);
      beginpicture(b);
    }
  }

  void writeclip(path p, bool newPath=true) {
    write(p,false);
  }

  void dot(path p, pen, bool newPath=true);

  void writeshifted(pair z) {
    write(conj(shift(-offset)*z)*settings::ps2tex);
  }

  double hoffset() {return Hoffset+offset.getx();}
  double voffset() {return box.bottom+offset.gety();}

  void translate(pair z) {}
  void concat(transform t) {}

  void beginspecial(bool def=false);
  void endspecial();

  void beginpage() {
    beginpicture(box);
  }

  void endpage() {
    endpicture(box);
  }

  void transform();
  void begintransform();
  void endtransform();

  void clippath();

  void beginpath();
  void endpath();

  void newpath() {
    beginspecial();
    begintransform();
    beginpath();
  }

// Workaround libc++ parsing bug under MacOS.
#ifdef __APPLE__
  const string sep=" ";
#else
  const string sep="";
#endif
  void moveto(pair z) {
    *out << sep << "M";
    writeshifted(z);
  }

  void lineto(pair z) {
    *out << sep << "L";
    writeshifted(z);
  }

  void curveto(pair zp, pair zm, pair z1) {
    *out << sep << "C";
    writeshifted(zp); writeshifted(zm); writeshifted(z1);
  }

  void closepath() {
    *out << sep << "Z";
  }

  string rgbhex(pen p) {
    p.torgb();
    return p.hex();
  }

  void properties(const pen& p);
  void color(const pen &p, const string& type);

  void stroke(const pen &p, bool dot=false);
  void strokepath();

  void fillrule(const pen& p, const string& type="fill");
  void fill(const pen &p);

  void begingradientshade(bool axial, ColorSpace colorspace,
                          const pen& pena, const pair& a, double ra,
                          const pen& penb, const pair& b, double rb);
  void gradientshade(bool axial, ColorSpace colorspace,
                     const pen& pena, const pair& a, double ra,
                     bool extenda, const pen& penb, const pair& b,
                     double rb, bool extendb);

  void gouraudshade(const pen& p0, const pair& z0,
                    const pen& p1, const pair& z1,
                    const pen& p2, const pair& z2);
  void begingouraudshade(const vm::array& pens, const vm::array& vertices,
                         const vm::array& edges);
  void gouraudshade(const pen& pentype, const vm::array& pens,
                    const vm::array& vertices, const vm::array& edges);

  void beginclip();

  void endclip(const pen &p);
  void endpsclip(const pen &p) {}

  void setpen(pen p) {if(!inspecial) texfile::setpen(p);}

  void gsave(bool tex=false);

  void grestore(bool tex=false);
};

} //namespace camp

#endif
