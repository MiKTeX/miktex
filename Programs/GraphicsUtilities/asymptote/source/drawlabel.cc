/*****
 * drawlabel.cc
 * John Bowman 2003/04/07
 *
 * Add a label to a picture.
 *****/

#include <sstream>

#include "drawlabel.h"
#include "settings.h"
#include "util.h"
#include "lexical.h"

using namespace settings;

namespace camp {
  
string texready=string("(Please type a command or say `\\end')\n*");

void drawLabel::labelwarning(const char *action) 
{
  cerr << "warning: label \"" << label 
       << "\" " << action << " to avoid overwriting" << endl;
}
 
// Reads one of the dimensions from the pipe.
void texdim(iopipestream& tex, double& dest, const string command,
            const string name)
{
  string start(">dim(");
  string stop(")dim");
  string expect("pt"+stop+"\n\n*");

  // ask the tex engine for dimension
  tex << "\\immediate\\write16{" << start << "\\the\\" << command << "\\ASYbox"
      << stop << "}\n";
  // keep reading output until ')dim\n\n*' is read
  tex.wait(expect.c_str());
  string buffer = tex.getbuffer();

  size_t dim1=buffer.find(start);
  size_t dim2=buffer.find("pt" + stop);
  string cannotread="Cannot read label "+name;
  if (dim1 != string::npos && dim2 != string::npos) {
    string n=buffer.substr(dim1+start.size(),dim2-dim1-start.size());
    try {
      dest=lexical::cast<double>(n,true)*camp::tex2ps;
    } catch(lexical::bad_cast&) {
      camp::reportError(cannotread);
    }
  } else {
    camp::reportError(cannotread);
  }
}

void texbounds(double& width, double& height, double& depth,
               iopipestream& tex, string& s)
{
  tex << "\\setbox\\ASYbox=\\hbox{" << stripblanklines(s) << "}\n\n";
  tex.wait(texready.c_str());
  texdim(tex,width,"wd","width");
  texdim(tex,height,"ht","height");
  texdim(tex,depth,"dp","depth");
}   

inline double urand()
{                         
  static const double factor=2.0/RANDOM_MAX;
#if defined(MIKTEX)
  return rand()*factor - 1.0;
#else
  return random()*factor-1.0;
#endif
}

void setpen(iopipestream& tex, const string& texengine, const pen& pentype) 
{
  bool Latex=latex(texengine);
  
  if(Latex && setlatexfont(tex,pentype,drawElement::lastpen)) {
    tex << "\n";
    tex.wait(texready.c_str());
  }
  if(settexfont(tex,pentype,drawElement::lastpen,Latex)) {
    tex << "\n";
    tex.wait(texready.c_str());
  }
  
  drawElement::lastpen=pentype;
}

void drawLabel::getbounds(iopipestream& tex, const string& texengine)
{
  if(havebounds) return;
  havebounds=true;
  
  setpen(tex,texengine,pentype);
  texbounds(width,height,depth,tex,label);
  
  if(width == 0.0 && height == 0.0 && depth == 0.0 && !size.empty())
    texbounds(width,height,depth,tex,size);

  enabled=true;
    
  Align=inverse(T)*align;
  double scale0=max(fabs(Align.getx()),fabs(Align.gety()));
  if(scale0) Align *= 0.5/scale0;
  Align -= pair(0.5,0.5);
  double Depth=(pentype.Baseline() == NOBASEALIGN) ? depth : 
    -depth*Align.gety();
  texAlign=Align;
  const double vertical=height+depth;
  if(Depth > 0) texAlign += pair(0.0,Depth/vertical);
  Align.scale(width,vertical);
  Align += pair(0.0,Depth-depth);
  Align=T*Align;
}

void drawLabel::bounds(bbox& b, iopipestream& tex, boxvector& labelbounds,
                       bboxlist&)
{
  string texengine=getSetting<string>("tex");
  if(texengine == "none") {b += position; return;}
  
  getbounds(tex,texengine);
  
  // alignment point
  pair p=position+Align;
  const double vertical=height+depth;
  const double fuzz=pentype.size()*0.1+0.3;
  pair A=p+T*pair(-fuzz,-fuzz);
  pair B=p+T*pair(-fuzz,vertical+fuzz);
  pair C=p+T*pair(width+fuzz,vertical+fuzz);
  pair D=p+T*pair(width+fuzz,-fuzz);
  
  if(pentype.Overwrite() != ALLOW && label != "") {
    size_t n=labelbounds.size();
    box Box=box(A,B,C,D);
    for(size_t i=0; i < n; i++) {
      if(labelbounds[i].intersect(Box)) {
        switch(pentype.Overwrite()) {
          case SUPPRESS:
            labelwarning("suppressed");
          case SUPPRESSQUIET:
            suppress=true; 
            return;
          case MOVE:
            labelwarning("moved");
          default:
            break;
        }

        pair Align=(align == pair(0,0)) ? unit(pair(urand(),urand())) :
          unit(align);
        double s=0.1*pentype.size();
        double dx=0, dy=0;
        if(Align.getx() > 0.1) dx=labelbounds[i].xmax()-Box.xmin()+s;
        if(Align.getx() < -0.1) dx=labelbounds[i].xmin()-Box.xmax()-s;
        if(Align.gety() > 0.1) dy=labelbounds[i].ymax()-Box.ymin()+s;
        if(Align.gety() < -0.1) dy=labelbounds[i].ymin()-Box.ymax()-s;
        pair offset=pair(dx,dy);
        position += offset;
        A += offset;
        B += offset;
        C += offset;
        D += offset;
        Box=box(A,B,C,D);
        i=0;
      }
    }
    labelbounds.resize(n+1);
    labelbounds[n]=Box;
  }
  
  Box=bbox();
  Box += A;
  Box += B;
  Box += C;
  Box += D;
  
  b += Box;
}

void drawLabel::checkbounds()
{
  if(!havebounds)
    reportError("drawLabel::write called before bounds");
}

bool drawLabel::write(texfile *out, const bbox&)
{
  checkbounds();
  if(suppress || pentype.invisible() || !enabled) return true;
  out->setpen(pentype);
  out->put(label,T,position,texAlign);
  return true;
}

drawElement *drawLabel::transformed(const transform& t)
{
  return new drawLabel(label,size,t*T,t*position,
                       length(align)*unit(shiftless(t)*align),pentype);
}

void drawLabelPath::bounds(bbox& b, iopipestream& tex, boxvector&, bboxlist&)
{
  string texengine=getSetting<string>("tex");
  if(texengine == "none") {b += position; return;}
    
  getbounds(tex,texengine);
  double L=p.arclength();
  
  double s1,s2;
  if(justify == "l") {
    s1=0.0;
    s2=width;
  } else if(justify == "r") {
    s1=L-width;
    s2=L;
  } else {
    double s=0.5*L;
    double h=0.5*width;
    s1=s-h;
    s2=s+h;
  }
  
  double Sx=shift.getx();
  double Sy=shift.gety();
  s1 += Sx;
  s2 += Sx;
  
  if(width > L || (!p.cyclic() && (s1 < 0 || s2 > L))) {
    ostringstream buf;
    buf << "Cannot fit label \"" << label << "\" to path";
    reportError(buf);
  }
  
  path q=p.subpath(p.arctime(s1),p.arctime(s2));
  
  b += q.bounds(Sy,Sy+height);
  Box=b;
}
  
bool drawLabelPath::write(texfile *out, const bbox&)
{
  bbox b=Box;
  double Hoffset=getSetting<bool>("inlinetex") ? b.right : b.left;
  b.shift(pair(-Hoffset,-b.bottom));
  
  checkbounds();
  if(drawLabel::pentype.invisible()) return true;
  out->setpen(drawLabel::pentype);
  out->verbatimline("\\psset{unit=1pt}%");
  out->verbatim("\\pstextpath[");
  out->verbatim(justify);
  out->verbatim("]");
  out->writepair(shift);
  out->verbatim("{\\pstVerb{");
  out->beginraw();
  writeshiftedpath(out);
  out->endraw();
  out->verbatim("}}{");
  out->verbatim(label);
  out->verbatimline("}");
  return true;
}

drawElement *drawLabelPath::transformed(const transform& t)
{
  return new drawLabelPath(label,size,transpath(t),justify,shift,
                           transpen(t));
}

} //namespace camp
