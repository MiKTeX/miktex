/*****
 * texfile.cc
 * John Bowman 2003/03/14
 *
 * Encapsulates the writing of commands to a TeX file.
 *****/

#include <ctime>
#include <cfloat>

#include "texfile.h"
#include "errormsg.h"

using std::ofstream;
using settings::getSetting;
using settings::ps2tex;
using settings::tex2ps;
using vm::array;
using vm::read;

namespace camp {

texfile::texfile(const string& texname, const bbox& box, bool pipe) 
  : box(box)
{
  texengine=getSetting<string>("tex");
  inlinetex=getSetting<bool>("inlinetex");
  Hoffset=inlinetex ? box.right : box.left;
  out=new ofstream(texname.c_str());
  if(!out || !*out)
    reportError("Cannot write to "+texname);
  out->setf(std::ios::fixed);
  out->precision(6);
  texdocumentclass(*out,pipe);
  resetpen();
  level=0;
}

texfile::~texfile()
{
  if(out) {
    delete out;  
    out=NULL;
  }
}
  
void texfile::miniprologue()
{
  texpreamble(*out,processData().TeXpreamble,true);
  if(settings::latex(texengine)) {
    *out << "\\pagestyle{empty}" << newl
         << "\\textheight=2048pt" << newl
         << "\\textwidth=2048pt" << newl
         << "\\begin{document}" << newl;
    latexfontencoding(*out);
  } else if(settings::context(texengine)) {
    *out << "\\setuppagenumbering[location=]" << newl
         << "\\usetypescript[modern]" << newl
         << "\\starttext\\hbox{%" << newl;
  } else *out << "\\nopagenumbers" << newl;
}

void texfile::prologue()
{
  if(inlinetex) {
    string prename=buildname(settings::outname(),"pre");
    std::ofstream *outpreamble=new std::ofstream(prename.c_str());
    texpreamble(*outpreamble,processData().TeXpreamble,false,false);
    outpreamble->close();
  }
  
  texdefines(*out,processData().TeXpreamble,false);
  double width=box.right-box.left;
  double height=box.top-box.bottom;
  if(!inlinetex) {
    if(settings::context(texengine)) {
      *out << "\\definepapersize[asy][width=" << width << "bp,height=" 
           << height << "bp]" << newl
           << "\\setuppapersize[asy][asy]" << newl;
    } else if(settings::pdf(texengine)) {
      double voffset=0.0;
      if(settings::latex(texengine)) {
        if(height < 12.0) voffset=height-12.0;
      } else if(height < 10.0) voffset=height-10.0;

      if(width > 0) 
        *out << "\\pdfpagewidth=" << width << "bp" << newl;
      *out << "\\ifx\\pdfhorigin\\undefined" << newl
           << "\\hoffset=-1in" << newl
           << "\\voffset=" << voffset-72.0 << "bp" << newl;
      if(height > 0)
        *out << "\\pdfpageheight=" << height << "bp" 
             << newl;
      *out << "\\else" << newl
           << "\\pdfhorigin=0bp" << newl
           << "\\pdfvorigin=" << voffset << "bp" << newl;
      if(height > 0)
        *out << "\\pdfpageheight=" << height << "bp" << newl;
      *out << "\\fi" << newl;
    }
  }
  
  if(settings::xe(texengine) && !inlinetex)
    *out << "\\usepackage{everypage}%" << newl;
  
  if(settings::latex(texengine)) {
    *out << "\\setlength{\\unitlength}{1pt}%" << newl;
    if(!inlinetex) {
      *out << "\\pagestyle{empty}" << newl
           << "\\textheight=" << height+18.0 << "bp" << newl
           << "\\textwidth=" << width+18.0 << "bp" << newl;
      if(settings::pdf(texengine))
        *out << "\\parindent=0pt" << newl
             << "\\oddsidemargin=0pt" << newl
             << "\\evensidemargin=\\oddsidemargin" << newl
             << "\\headheight=0pt" << newl
             << "\\headsep=0pt" << newl
             << "\\topmargin=0pt" << newl
             << "\\topskip=0pt" << newl;
      *out << "\\begin{document}" << newl;
    }
    latexfontencoding(*out);
  } else {
    if(!inlinetex) {
      if(settings::context(texengine)) {
        *out << "\\setuplayout["
             << "backspace=0pt,topspace=0pt,"
             << "header=0pt,headerdistance=0pt,footer=0pt]" << newl
             << "\\setuppagenumbering[location=]" << endl
             << "\\usetypescript[modern]" << newl
             << "\\starttext\\hbox{%" << newl;
      } else {
        *out << "\\footline={}" << newl;
        if(settings::pdf(texengine)) {
          *out << "\\hoffset=-20pt" << newl
               << "\\voffset=0pt" << newl;
        } else {
          *out << "\\hoffset=36.6pt" << newl
               << "\\voffset=54.0pt" << newl;
        }
      }
    }
  }
  
  beginpage();
}
    
void texfile::beginlayer(const string& psname, bool postscript)
{
  if(box.right > box.left && box.top > box.bottom) {
    if(postscript) {
      if(settings::context(texengine))
        *out << "\\externalfigure[" << psname << "]%" << newl;
      else {
        *out << "{\\catcode`\"=12%" << newl
             << "\\includegraphics";
        bool pdf=settings::pdf(texengine);
        string name=stripExt(psname);
        if(inlinetex) {
          size_t pos=name.rfind("-");
          if(pos < string::npos) name="\\ASYprefix\\jobname"+name.substr(pos);
        } else {
          if(!pdf) name=psname;
        }
        
        if(!pdf)
          *out << "[bb=" << box.left << " " << box.bottom << " "
               << box.right << " " << box.top << "]";
        *out << "{" << name << "}%" << newl << "}%" << newl;
      }
      if(!inlinetex)
        *out << "\\kern " << (box.left-box.right)*ps2tex << "pt%" << newl;
    } else {
      *out << "\\leavevmode\\vbox to " << (box.top-box.bottom)*ps2tex 
           << "pt{}%" << newl;
      if(inlinetex)
        *out << "\\kern " << (box.right-box.left)*ps2tex << "pt%" << newl;
    }
  }
}

void texfile::endlayer()
{
  if(inlinetex && (box.right > box.left && box.top > box.bottom))
    *out << "\\kern " << (box.left-box.right)*ps2tex << "pt%" << newl;
}

void texfile::writeshifted(path p, bool newPath)
{
  write(p.transformed(shift(pair(-Hoffset,-box.bottom))),newPath);
}

void texfile::setlatexcolor(pen p)
{
  if(p.cmyk() && (!lastpen.cmyk() || 
                  (p.cyan() != lastpen.cyan() || 
                   p.magenta() != lastpen.magenta() || 
                   p.yellow() != lastpen.yellow() ||
                   p.black() != lastpen.black()))) {
    *out << "\\definecolor{ASYcolor}{cmyk}{" 
         << p.cyan() << "," << p.magenta() << "," << p.yellow() << "," 
         << p.black() << "}\\color{ASYcolor}%" << newl;
  } else if(p.rgb() && (!lastpen.rgb() ||
                        (p.red() != lastpen.red() ||
                         p.green() != lastpen.green() || 
                         p.blue() != lastpen.blue()))) {
    *out << "\\definecolor{ASYcolor}{rgb}{" 
         << p.red() << "," << p.green() << "," << p.blue()
         << "}\\color{ASYcolor}%" << newl;
  } else if(p.grayscale() && (!lastpen.grayscale() || 
                              p.gray() != lastpen.gray())) {
    *out << "\\definecolor{ASYcolor}{gray}{" 
         << p.gray()
         << "}\\color{ASYcolor}%" << newl;
  }
}
  
void texfile::setfont(pen p)
{
  bool latex=settings::latex(texengine);
  
  if(latex) setlatexfont(*out,p,lastpen);
  settexfont(*out,p,lastpen,latex);
  
  lastpen.setfont(p);
}
  
void texfile::setpen(pen p)
{
  bool latex=settings::latex(texengine);
  
  p.convert();
  if(p == lastpen) return;

  if(latex) setlatexcolor(p);
  else setcolor(p,settings::beginspecial(texengine),settings::endspecial());
  
  setfont(p);
}
   
void texfile::beginpicture(const bbox& b)
{
  verbatim(settings::beginpicture(texengine));
  if(!settings::context(texengine)) {
    verbatim("(");
    double width=b.right-b.left;
    double height=b.top-b.bottom;
    write(width*ps2tex);
    verbatim(",");
    write(height*ps2tex);
    verbatim(")");
  }
  verbatimline("%");
}
  
void texfile::endpicture(const bbox& b)
{
  verbatimline(settings::endpicture(texengine));
  verbatim("\\kern");
  double width=b.right-b.left;
  write(-width*ps2tex);
  verbatimline("pt%");
}
  
void texfile::gsave(bool)
{
  *out << settings::beginspecial(texengine);
  psfile::gsave(true);
  *out << settings::endspecial() << newl;
}

void texfile::grestore(bool)
{
  *out << settings::beginspecial(texengine);
  psfile::grestore(true);
  *out << settings::endspecial() << newl;
}

void texfile::beginspecial() 
{
  *out << settings::beginspecial(texengine);
}
  
void texfile::endspecial() 
{
  *out << settings::endspecial() << newl;
}
  
void texfile::beginraw() 
{
  *out << "\\ASYraw{" << newl;
}
  
void texfile::endraw() 
{
  *out << "}%" << newl;
}
  
void texfile::put(const string& label, const transform& T, const pair& z,
                  const pair& align)
{
  double sign=settings::pdf(texengine) ? 1.0 : -1.0;

  if(label.empty()) return;
  
  bool trans=!T.isIdentity();
  
  *out << "\\ASYalign";
  if(trans) *out << "T";
  *out << "(" << (z.getx()-Hoffset)*ps2tex
       << "," << (z.gety()-box.bottom)*ps2tex
       << ")(" << align.getx()
       << "," << align.gety() 
       << ")";
  if(trans)
    *out << "{" << T.getxx() << " " << sign*T.getyx()
         << " " << sign*T.getxy() << " " << T.getyy() << "}";
  *out << "{" << label << "}%" << newl;
}

void texfile::epilogue(bool pipe)
{
  endpage();
  if(settings::latex(texengine))
    *out << "\\end{document}" << newl;
  else if(settings::context(texengine))
    *out << "}\\stoptext" << newl;
  else
    *out << "\\bye" << newl;
  out->flush();
}

string svgtexfile::nl="{?nl}%\n";

void svgtexfile::beginspecial()
{
  inspecial=true;
  out->unsetf(std::ios::fixed);
  *out << "\\catcode`\\#=11%" << newl
       << "\\special{dvisvgm:raw" << nl;
}
    
void svgtexfile::endspecial()
{
  inspecial=false;
  *out << "}\\catcode`\\#=6%" << newl;
  out->setf(std::ios::fixed);
}
  
void svgtexfile::begintransform()
{
  bbox b=box;
  b.left=-Hoffset;
  b=svgbbox(b);
  *out << "<g transform='matrix(" << tex2ps << " 0 0 " << tex2ps <<" "
       << b.left << " " << b.top << ")'>" << nl;
}
    
void svgtexfile::endtransform()
{
  *out << "</g>";
}
  
void svgtexfile::gsave(bool)
{
  if(clipstack.size() < 1)
    clipstack.push(0);
  else
    clipstack.push(clipcount);
  *out << "\\special{dvisvgm:raw <g>}%" << newl;
  pens.push(lastpen);
}
  
void svgtexfile::grestore(bool)
{
  if(pens.size() < 1 || clipstack.size() < 1)
    reportError("grestore without matching gsave");
  lastpen=pens.top();
  pens.pop();
  clipstack.pop();
  *out << "\\special{dvisvgm:raw </g>}%" << newl;
}

void svgtexfile::clippath()
{
  if(clipstack.size() > 0) {
    size_t count=clipstack.top();
    if(count > 0)
      *out << "clip-path='url(#clip" << count << ")' ";
  }
}
  
void svgtexfile::beginpath()
{
  *out << "<path ";
  clippath();
  *out << "d='";
}
  
void svgtexfile::endpath()
{
  *out << "/>" << nl;
}
  
void svgtexfile::dot(path p, pen q, bool newPath)
{
  beginspecial();
  begintransform();
  *out << "<circle ";
  clippath();
  pair z=p.point((Int) 0);
  *out << "cx='" << z.getx()*ps2tex
       << "' cy='" << -z.gety()*ps2tex
       << "' r='" << 0.5*q.width()*ps2tex;
}

void svgtexfile::beginclip()
{
  beginspecial();
  begintransform();
  *out << "<clipPath ";
  clippath();
  ++clipcount;
  *out << "id='clip" << clipcount << "'>" << nl;
  beginpath();
  if(clipstack.size() > 0)
    clipstack.pop();
  clipstack.push(clipcount);
}
  
void svgtexfile::endclip0(const pen &p) 
{
  *out << "'";
  fillrule(p,"clip");
  endpath();
  *out << "</clipPath>" << nl;
}

void svgtexfile::endclip(const pen &p) 
{
  endclip0(p);
  endtransform();
  endspecial();
}

void svgtexfile::fillrule(const pen& p, const string& type)
{
  if(p.Fillrule() != lastpen.Fillrule())
    *out << " " << type << "-rule='" << 
      (p.evenodd() ? "evenodd" : "nonzero") << "'";
  lastpen.setfillrule(p);
}
   
void svgtexfile::color(const pen &p, const string& type)
{
  *out << "' " << type << "='#" << rgbhex(p) << "'";
  double opacity=p.opacity();
  if(opacity != 1.0)
    *out << " opacity='" << opacity << "'";
}

void svgtexfile::fill(const pen &p)
{
  color(p,"fill");
  fillrule(p);
  endpath();
  endtransform();
  endspecial();
}

void svgtexfile::properties(const pen& p)
{
  if(p.cap() != lastpen.cap())
    *out << " stroke-linecap='" << PSCap[p.cap()] << "'";
    
  if(p.join() != lastpen.join())
    *out << " stroke-linejoin='" << Join[p.join()] << "'";
  
  if(p.miter() != lastpen.miter())
    *out << " stroke-miterlimit='" << p.miter()*ps2tex << "'";
    
  if(p.width() != lastpen.width())
    *out << " stroke-width='" << p.width()*ps2tex << "'";
  
  const LineType *linetype=p.linetype();
  const LineType *lastlinetype=lastpen.linetype();
  
  if(!(linetype->pattern == lastlinetype->pattern)) {
    size_t n=linetype->pattern.size();
    if(n > 0) {
      *out << " stroke-dasharray='";
      *out << vm::read<double>(linetype->pattern,0)*ps2tex;
      for(size_t i=1; i < n; ++i)
        *out << "," << vm::read<double>(linetype->pattern,i)*ps2tex;
      *out << "'";
    }
  }
  
  if(linetype->offset != lastlinetype->offset)
    *out << " stroke-dashoffset='" << linetype->offset*ps2tex << "'";
  
  lastpen=p;
}
  
void svgtexfile::stroke(const pen &p, bool dot)
{
  if(dot) 
    color(p,"fill");
  else {
    color(p,"fill='none' stroke");
    properties(p);  
  }
  endpath();
  endtransform();
  endspecial();
}
  
void svgtexfile::strokepath()
{
  reportWarning("SVG does not support strokepath");
}

void svgtexfile::begingradientshade(bool axial, ColorSpace colorspace,
                                    const pen& pena, const pair& a, double ra,
                                    const pen& penb, const pair& b, double rb)
{
  string type=axial ? "linear" : "radial";
  beginspecial();
  *out << "<" << type << "Gradient id='grad" << gradientcount;
  if(axial) {
    *out << "' x1='" << a.getx()*ps2tex << "' y1='" << -a.gety()*ps2tex
         << "' x2='" << b.getx()*ps2tex << "' y2='" << -b.gety()*ps2tex;
  } else {
    *out << "' cx='" << b.getx()*ps2tex << "' cy='" << -b.gety()*ps2tex
         << "' r='" << rb*ps2tex;
  }
  *out <<"' gradientUnits='userSpaceOnUse'>" << nl
       << "<stop offset='0' stop-color='#" << rgbhex(pena) << "'/>" << nl
       << "<stop offset='1' stop-color='#" << rgbhex(penb) << "'/>" << nl
       << "</" << type << "Gradient>" << nl;
  begintransform();
  beginpath();
}

void svgtexfile::gradientshade(bool axial, ColorSpace colorspace,
                               const pen& pena, const pair& a, double ra,
                               bool, const pen& penb, const pair& b,
                               double rb, bool)

{
  *out << "' fill='url(#grad" << gradientcount << ")'";
  fillrule(pena);
  endpath();
  ++gradientcount;
  endtransform();
  endspecial();
}
  
// Return the point on the line through p and q that is closest to z.
pair closest(pair p, pair q, pair z)
{
  pair u=q-p;
  double denom=dot(u,u);
  return denom == 0.0 ? p : p+dot(z-p,u)/denom*u;
}

void svgtexfile::gouraudshade(const pen& p0, const pair& z0,
                              const pen& p1, const pair& z1, 
                              const pen& p2, const pair& z2)
{
  string hex[]={rgbhex(p0),rgbhex(p1),rgbhex(p2)};
  pair Z[]={z0,z1,z2};
    
  *out << "<defs>" << nl
       << "<filter id='colorAdd'>" << nl
       << "<feBlend in='SourceGraphic' in2='BackgroundImage'/>" << nl
       << "</filter>";

  for(size_t k=0; k < 3; ++k) {
    pair z=Z[k];
    pair opp=closest(Z[(k+1) % 3],Z[(k+2) % 3],z);
    *out << "<linearGradient id='grad-" << gouraudcount << "-" << k 
         << "' gradientUnits='userSpaceOnUse'" << nl
         << " x1='" << z.getx()*ps2tex << "' y1='" << -z.gety()*ps2tex
         << "' x2='" << opp.getx()*ps2tex << "' y2='" << -opp.gety()*ps2tex
         << "'>" << nl
         << "<stop offset='0' stop-color='#" << hex[k] 
         << "' stop-opacity='1'/>" << nl
         << "<stop offset='1' stop-color='#" << hex[k] 
         << "' stop-opacity='0'/>" << nl
         << "</linearGradient>" << nl;
  }
  *out << "<polygon ";
  clippath();
  *out << "id='triangle" << gouraudcount << "' points='"
       << z0.getx()*ps2tex << "," << -z0.gety()*ps2tex << " "
       << z1.getx()*ps2tex << "," << -z1.gety()*ps2tex << " "
       << z2.getx()*ps2tex << "," << -z2.gety()*ps2tex << "'/>" << nl
       << "</defs>" << nl;
  *out << "<use xlink:href='#triangle" << gouraudcount
       << "' fill='url(#grad-" << gouraudcount << "-" 
       << "0)'/>" << nl
       << "<use xlink:href='#triangle" << gouraudcount
       << "' fill='url(#grad-" << gouraudcount << "-" 
       << "1)' filter='url(#colorAdd)'/>" << nl
       << "<use xlink:href='#triangle" << gouraudcount
       << "' fill='url(#grad-" << gouraudcount << "-" 
       << "2)' filter='url(#colorAdd)'/>" << nl;
  ++gouraudcount;
}

void svgtexfile::begingouraudshade(const vm::array& pens,
                                   const vm::array& vertices,
                                   const vm::array& edges)
{
  size_t size=pens.size();
  if(size == 0) return;
  beginclip();
}

void svgtexfile::gouraudshade(const pen& pentype,
                              const array& pens, const array& vertices,
                              const array& edges)
{
  size_t size=pens.size();
  if(size == 0) return;
  
  endclip0(pentype);
  
  pen *p0=NULL,*p1=NULL,*p2=NULL;
  pair z0,z1,z2;
  
  for(size_t i=0; i < size; i++) {
    Int edge=read<Int>(edges,i);
    
    switch(edge) {
      case 0:
        p0=read<pen *>(pens,i);
        z0=read<pair>(vertices,i);
        ++i;
	if(i < size) {
	  p1=read<pen *>(pens,i);
	  z1=read<pair>(vertices,i);
	  ++i;
	  if(i < size) {
	    p2=read<pen *>(pens,i);
	    z2=read<pair>(vertices,i);
	  }
	}
        break;
      case 1:
        p0=read<pen *>(pens,i);
        z0=read<pair>(vertices,i);
        break;
      case 2:
        p1=read<pen *>(pens,i);
        z1=read<pair>(vertices,i);
        break;
      default:
	break;
    }
    if(p0 == NULL || p1 == NULL || p2 == NULL)
      reportError("invalid edge flag");
    gouraudshade(*p0,z0,*p1,z1,*p2,z2);
  }
  endtransform();
  endspecial();
}

void svgtexfile::begintensorshade(const vm::array& pens,
                                  const vm::array& boundaries,
                                  const vm::array& z) 
{
  beginspecial();
  *out << "<defs>" << nl;

  path g=read<path>(boundaries,0);
  pair Z[]={g.point((Int) 0),g.point((Int) 3),g.point((Int) 2),
            g.point((Int) 1)};
      
  array *pi=read<array *>(pens,0);
  if(checkArray(pi) != 4)
    reportError("specify 4 pens for each path");
  string hex[]={rgbhex(read<pen>(pi,0)),rgbhex(read<pen>(pi,3)),
                rgbhex(read<pen>(pi,2)),rgbhex(read<pen>(pi,1))};
    
  *out << "<filter id='colorAdd'>" << nl
       << "<feBlend in='SourceGraphic' in2='BackgroundImage'/>" << nl
       << "</filter>";

  pair mean=0.25*(Z[0]+Z[1]+Z[2]+Z[3]);
  for(size_t k=0; k < 4; ++k) {
    pair opp=(k % 2 == 0) ? Z[(k+2) % 4] : mean;
    *out << "<linearGradient id='grad" << tensorcount << "-" << k 
         << "' gradientUnits='userSpaceOnUse'" << nl
         << " x1='" << Z[k].getx()*ps2tex << "' y1='" << -Z[k].gety()*ps2tex
         << "' x2='" << opp.getx()*ps2tex << "' y2='" << -opp.gety()*ps2tex
         << "'>" << nl
         << "<stop offset='0' stop-color='#" << hex[k] 
         << "' stop-opacity='1'/>" << nl
         << "<stop offset='1' stop-color='#" << hex[k] 
         << "' stop-opacity='0'/>" << nl
         << "</linearGradient>" << nl;
  }
  beginpath();
}

void svgtexfile::tensorshade(const pen& pentype, const vm::array& pens,
                             const vm::array& boundaries, const vm::array& z)
{
  *out << "' id='path" << tensorcount << "'";
  fillrule(pentype);
  endpath();
  *out << "</defs></g>" << nl;
  begintransform();
  *out << "<use xlink:href='#path" << tensorcount
       << "' fill='url(#grad" << tensorcount << "-" 
       << "0)'/>" << nl
       << "<use xlink:href='#path" << tensorcount
       << "' fill='url(#grad" << tensorcount << "-" 
       << "2)' filter='url(#colorAdd)'/>" << nl
       << "<use xlink:href='#path" << tensorcount
       << "' fill='url(#grad" << tensorcount << "-" 
       << "1)' filter='url(#colorAdd)'/>" << nl
       << "<use xlink:href='#path" << tensorcount 
       << "' fill='url(#grad" << tensorcount << "-"
       << "3)' filter='url(#colorAdd)'/>" << nl;

  ++tensorcount;
  endspecial();
}

} //namespace camp
