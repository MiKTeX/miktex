/*****
 * pen.h
 * John Bowman 2003/03/23
 *
 *****/

#ifndef PEN_H
#define PEN_H

#include <iomanip>

#include "transform.h"
#include "settings.h"
#include "bbox.h"
#include "common.h"
#include "path.h"
#include "array.h"

namespace camp {

class LineType
{
public:  
  vm::array pattern;    // Array of PostScript style line pattern entries.
  double offset;        // The offset in the pattern at which to start drawing.
  bool scale;           // Scale the line type values by the pen width?
  bool adjust;          // Adjust the line type values to fit the arclength?
  bool isdefault;   
  
  LineType() : offset(0.0), scale(true), adjust(true), isdefault(true) {}
  
  LineType(vm::array pattern, double offset, bool scale, bool adjust) : 
    pattern(pattern), offset(offset), scale(scale), adjust(adjust),
    isdefault(false) {}
  
  void Scale(double factor) {
    size_t n=pattern.size();
    for(size_t i=0; i < n; i++)
      pattern[i]=vm::read<double>(pattern,i)*factor;
    offset *= factor;
  }
};
  
extern const char* DEFPAT;
extern const char* DEFLATEXFONT;
extern const char* DEFCONTEXTFONT;
extern const char* DEFTEXFONT;
extern const double DEFWIDTH;
extern const Int DEFCAP;
extern const Int DEFJOIN;
extern const double DEFMITER;
extern const transform nullTransform;

static const struct invisiblepen_t {} invisiblepen={};
static const struct setlinewidth_t {} setlinewidth={};
static const struct setfont_t {} setfont={};
static const struct setfontsize_t {} setfontsize={};
static const struct setpattern_t {} setpattern={};
static const struct setlinecap_t {} setlinecap={};
static const struct setlinejoin_t {} setlinejoin={};
static const struct setmiterlimit_t {} setmiterlimit={};
static const struct setoverwrite_t {} setoverwrite={};
static const struct initialpen_t {} initialpen={};
static const struct resolvepen_t {} resolvepen={};
  
extern const char* PSCap[];
extern const char* Cap[];
extern const Int nCap;
extern const char* Join[];
extern const Int nJoin;
  
enum overwrite_t {DEFWRITE=-1,ALLOW,SUPPRESS,SUPPRESSQUIET,MOVE,MOVEQUIET};
extern const char* OverwriteTag[];
extern const Int nOverwrite;
  
enum FillRule {DEFFILL=-1,ZEROWINDING,EVENODD};
extern const char* FillRuleTag[];
extern const Int nFill;
  
enum BaseLine {DEFBASE=-1,NOBASEALIGN,BASEALIGN};
extern const char* BaseLineTag[];
extern const Int nBaseLine;
  
enum ColorSpace {DEFCOLOR=0,INVISIBLE,GRAYSCALE,RGB,CMYK,PATTERN};
extern const size_t ColorComponents[];
extern const char* ColorDeviceSuffix[];
extern const unsigned nColorSpace;
  
inline bool operator == (const vm::array& a, const vm::array& b)
{
  size_t asize=a.size();
  size_t bsize=b.size();
  if(asize != bsize) return false;
  for(size_t i=0; i < asize; ++i)
    if(vm::read<double>(a,i) != vm::read<double>(b,i)) return false;
  return true;
}
  
inline bool operator == (const LineType& a, const LineType& b)
{
  return a.pattern == b.pattern && a.offset == b.offset && a.scale == b.scale
    && a.adjust == b.adjust;
}
  
inline ostream& operator << (ostream& out, const vm::array& a)
{
  out << "[";
  size_t n=a.size();
  if(n > 0) {
    out << vm::read<double>(a,0);
      for(size_t i=1; i < n; ++i)
        out << " " << vm::read<double>(a,i);
  }
  out << "]";
  return out;
}

class Transparency
{
public:  
  string blend;
  double opacity;
  bool isdefault;   
  
  Transparency() :
    blend("Compatible"), opacity(1.0), isdefault(true) {}
  
  Transparency(const string& blend, double opacity) :
    blend(blend), opacity(opacity), isdefault(false) {}
};
  
inline bool operator == (const Transparency& a, const Transparency& b) {
  return a.blend == b.blend && a.opacity == b.opacity;
}
  
extern const char* BlendMode[];
extern const Int nBlendMode;

const double bytescale=256.0*(1.0-DBL_EPSILON);

// Map [0,1] to [0,255]
inline unsigned int byte(double r) 
{
  if(r < 0.0) r=0.0;
  else if(r > 1.0) r=1.0;
  return (int)(bytescale*r);
}

class pen;
pen& defaultpen();

class pen : public gc { 
  LineType line;

  // Width of line, in PS units.
  double linewidth;
  path P;               // A polygonal path defining a custom pen nib;
                        // nullpath means the default (typically circular) nib.
  string font;
  double fontsize;  
  double lineskip;  
  
  ColorSpace color;
  double r,g,b;         // RGB or CMY value
  double grey;          // grayscale or K value
  
  string pattern;       // The name of the user-defined fill/draw pattern
  FillRule fillrule;    // Zero winding-number (default) or even-odd rule
  BaseLine baseline;    // Align to TeX baseline?
  Transparency transparency;
  Int linecap;
  Int linejoin;
  double miterlimit;
  overwrite_t overwrite;
  
  // The transformation applied to the pen nib for calligraphic effects.
  // nullTransform means the default (typically identity) transformation.
  transform t;
  
public:
  static double pos0(double x) {return x >= 0 ? x : 0;}
  
  void greyrange() {if(grey > 1.0) grey=1.0;}
  
  void rgbrange() {
    double sat=rgbsaturation();
    if(sat > 1.0) {
      double scale=1.0/sat;
      r *= scale;
      g *= scale;
      b *= scale;
    }
  }
  
  void cmykrange() {
    double sat=cmyksaturation();
    if(sat > 1.0) {
      double scale=1.0/sat;
      r *= scale;
      g *= scale;
      b *= scale;
      grey *= scale;
    }
  }
  
  void colorless() {
    r=g=b=grey=0.0;
    color=DEFCOLOR;
  }
  
  pen() :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}

  pen(const LineType& line, double linewidth, const path& P,
      const string& font, double fontsize, double lineskip,
      ColorSpace color, double r, double g, double b,  double grey,
      const string& pattern, FillRule fillrule, BaseLine baseline,
      const Transparency& transparency,
      Int linecap, Int linejoin, double miterlimit,
      overwrite_t overwrite, const transform& t) :
    line(line), linewidth(linewidth), P(P),
    font(font), fontsize(fontsize), lineskip(lineskip), color(color),
    r(r), g(g), b(b), grey(grey),
    pattern(pattern), fillrule(fillrule), baseline(baseline),
    transparency(transparency),
    linecap(linecap), linejoin(linejoin), miterlimit(miterlimit),
    overwrite(overwrite), t(t) {}
      
  pen(invisiblepen_t) : 
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(INVISIBLE),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(setlinewidth_t, double linewidth) : 
    line(), linewidth(linewidth), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(path P) : 
    line(), linewidth(DEFWIDTH), P(P),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(const LineType& line) :
    line(line), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(setfont_t, string font) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(font), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(setfontsize_t, double fontsize, double lineskip) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(fontsize), lineskip(lineskip), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(setpattern_t, const string& pattern) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(PATTERN),
    r(0), g(0), b(0), grey(0),
    pattern(pattern), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(FillRule fillrule) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(fillrule), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(BaseLine baseline) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(baseline),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(const Transparency& transparency) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(transparency),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(setlinecap_t, Int linecap) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(linecap), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(setlinejoin_t, Int linejoin) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(linejoin), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(setmiterlimit_t, double miterlimit) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(miterlimit),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  pen(setoverwrite_t, overwrite_t overwrite) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(DEFCOLOR),
    r(0), g(0), b(0), grey(0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(overwrite), t(nullTransform) {}
  
  explicit pen(double grey) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(GRAYSCALE),
    r(0.0), g(0.0), b(0.0), grey(pos0(grey)),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform)
  {greyrange();}
  
  pen(double r, double g, double b) : 
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(RGB),
    r(pos0(r)), g(pos0(g)), b(pos0(b)),  grey(0.0), 
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform)
  {rgbrange();}
  
  pen(double c, double m, double y, double k) :
    line(), linewidth(DEFWIDTH), P(nullpath),
    font(""), fontsize(0.0), lineskip(0.0), color(CMYK),
    r(pos0(c)), g(pos0(m)), b(pos0(y)), grey(pos0(k)),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(DEFBASE),
    transparency(),
    linecap(DEFCAP), linejoin(DEFJOIN), miterlimit(DEFMITER),
    overwrite(DEFWRITE), t(nullTransform)
  {cmykrange();}
  
  // Construct one pen from another, resolving defaults
  pen(resolvepen_t, const pen& p) : 
    line(LineType(p.line.pattern,p.line.offset,p.line.scale,p.line.adjust)),
    linewidth(p.width()), P(p.Path()),
    font(p.Font()), fontsize(p.size()), lineskip(p.Lineskip()),
    color(p.colorspace()),
    r(p.red()), g(p.green()), b(p.blue()), grey(p.gray()),
    pattern(""), fillrule(p.Fillrule()), baseline(p.Baseline()),
    transparency(Transparency(p.blend(), p.opacity())),
    linecap(p.cap()), linejoin(p.join()), miterlimit(p.miter()),
    overwrite(p.Overwrite()), t(p.getTransform()) {}
  
  static pen initialpen() {
    return pen(LineType(vm::array(0),0.0,true,true),0.5,nullpath,"",
               12.0*settings::tex2ps,12.0*1.2*settings::tex2ps,GRAYSCALE,
               0.0,0.0,0.0,0.0,"",ZEROWINDING,NOBASEALIGN,
               Transparency(),1,1,10.0,ALLOW,identity);
  }
  
  pen(initialpen_t) : 
    line(), linewidth(-2.0), P(nullpath),
    font("<invalid>"), fontsize(-1.0), lineskip(-1.0), color(INVISIBLE),
    r(0.0), g(0.0), b(0.0), grey(0.0),
    pattern(DEFPAT), fillrule(DEFFILL), baseline(NOBASEALIGN),
    transparency(),linecap(-2), linejoin(-2), miterlimit(-1.0),
    overwrite(DEFWRITE), t(nullTransform) {}
  
  double width() const {
    return linewidth == DEFWIDTH ? defaultpen().linewidth : linewidth;
  }
  
  path Path() const {
    return P.empty() ? defaultpen().P : P;
  }
  
  double size() const {
    return fontsize == 0.0 ? defaultpen().fontsize : fontsize;
  }
  
  string Font() const {
    if(font.empty()) {
      if(defaultpen().font.empty()) {
        string texengine=settings::getSetting<string>("tex");
        if(settings::latex(texengine))
          return DEFLATEXFONT;
        else if(texengine == "none")
          return settings::getSetting<string>("textinitialfont");
        else {
          ostringstream buf;
  // Work around misalignment in ConTeXt switchtobodyfont if font is not found.
          if(texengine == "context")
            buf << "\\switchtobodyfont[" 
                << DEFCONTEXTFONT << "," << size()*settings::ps2tex 
                << "pt]\\removeunwantedspaces%" << newl;
          else
            buf << "\\font\\ASYfont=" << DEFTEXFONT
                << " at " << size()*settings::ps2tex << "pt\\ASYfont";
          return buf.str();
        }
      }
      else return defaultpen().font;
    }
    return font;
  }
  
  double Lineskip() const {
    return lineskip == 0.0 ? defaultpen().lineskip : lineskip;
  }
  
  const LineType *linetype() const {
    return line.isdefault ? &defaultpen().line : &line;
  }
  
  void adjust(double factor) {
    if(line.isdefault) 
      line=defaultpen().line;
    line.Scale(factor);
  }
  
  void setstroke(const vm::array& s) {line.pattern=s;}
  void setoffset(const double& offset) {line.offset=offset;}
  
  string fillpattern() const {
    return pattern == DEFPAT ? (string)"" : pattern;
  }
  
  FillRule Fillrule() const {
    return fillrule == DEFFILL ? defaultpen().fillrule : fillrule;
  }
  
  bool evenodd() const {
    return Fillrule() == EVENODD;
  }
  
  bool inside(Int count) const {
    return evenodd() ? count % 2 : count != 0;
  }
  
  BaseLine Baseline() const {
    return baseline == DEFBASE ? defaultpen().baseline : baseline;
  }
  
  Transparency transp() const {
    return transparency.isdefault ? defaultpen().transparency : transparency;
  }
  
  string blend() const {
    return transparency.isdefault ? defaultpen().transparency.blend :
      transparency.blend;
  }
  
  double opacity() const {
    return transparency.isdefault ? defaultpen().transparency.opacity :
      transparency.opacity;
  }
  
  Int cap() const {
    return linecap == DEFCAP ? defaultpen().linecap : linecap;
  }
  
  Int join() const {
    return linejoin == DEFJOIN ? defaultpen().linejoin : linejoin;
  }
  
  double miter() const {
    return miterlimit == DEFMITER ? defaultpen().miterlimit : miterlimit;
  }
  
  overwrite_t Overwrite() const {
    return overwrite == DEFWRITE ? defaultpen().overwrite : overwrite;
  }
  
  ColorSpace colorspace() const {
    return color == DEFCOLOR ? defaultpen().color : color;
  }
  
  string hex() const {
    int n=ColorComponents[colorspace()];
    ostringstream buf;
    buf.setf(std::ios::hex,std::ios::basefield);
    buf.fill('0');
    
    switch(n) {
      case 0:
        break;
      case 1: 
        buf << std::setw(2) << byte(gray());
        break;
      case 3:
        buf << std::setw(2) << byte(red())
            << std::setw(2) << byte(green())
            << std::setw(2) << byte(blue());
        break;
      case 4:
        buf << std::setw(2) << byte(cyan())
            << std::setw(2) << byte(magenta())
            << std::setw(2) << byte(yellow())
            << std::setw(2) << byte(black());
        break;
      default:
        break;
    }
    return buf.str();
  }
  
  bool invisible() const {return colorspace() == INVISIBLE;}
  
  bool grayscale() const {return colorspace() == GRAYSCALE;}
  
  bool rgb() const {return colorspace() == RGB;}
  
  bool cmyk() const {return colorspace() == CMYK;}
  
  double gray() const {return color == DEFCOLOR ? defaultpen().grey : grey;}
  
  double red() const {return color == DEFCOLOR ? defaultpen().r : r;}
  
  double green() const {return color == DEFCOLOR ? defaultpen().g : g;}
  
  double blue() const {return color == DEFCOLOR ? defaultpen().b : b;}
  
  double cyan() const {return red();}
  
  double magenta() const {return green();}
  
  double yellow() const {return blue();}
  
  double black() const {return gray();}
  
  double rgbsaturation() const {return max(max(r,g),b);}
  
  double cmyksaturation() const {return max(rgbsaturation(),black());}
  
  void greytorgb() {
    r=g=b=grey; grey=0.0;
    color=RGB;
  }
  
  void rgbtogrey() {
    grey=0.299*r+0.587*g+0.114*b; // Standard YUV luminosity coefficients
    r=g=b=0.0;
    color=GRAYSCALE;
  }
  
  void greytocmyk() {
    grey=1.0-grey;
    r=g=b=0.0;
    color=CMYK;
  }
  
  void rgbtocmyk() {
    double sat=rgbsaturation();
    grey=1.0-sat;
    if(sat) {
      double scale=1.0/sat;
      r=1.0-r*scale;
      g=1.0-g*scale;
      b=1.0-b*scale;
    }
    color=CMYK;
  }

  void cmyktorgb() {
    double sat=1.0-grey;
    r=(1.0-r)*sat;
    g=(1.0-g)*sat;
    b=(1.0-b)*sat;
    grey=0.0;
    color=RGB;
  }

  void cmyktogrey() {
    cmyktorgb();
    rgbtogrey();
  }
  
  void togrey() {
    if(rgb()) rgbtogrey();
    else if(cmyk()) cmyktogrey();
  }
  
  void torgb() {
    if(cmyk()) cmyktorgb();
    else if(grayscale()) greytorgb();
  }
  
  void tocmyk() {
    if(rgb()) rgbtocmyk();
    else if(grayscale()) greytocmyk();
  }
  
  void settransparency(const pen& p) {
    transparency=p.transparency;
  }
                                                               
  void setfont(const pen& p) {
    font=p.font;
  }
  
  void setfillrule(const pen& p) {
    fillrule=p.fillrule;
  }
  
  void convert() {
    if(settings::gray || settings::bw) {
      if(rgb()) rgbtogrey();
      else if(cmyk()) cmyktogrey();
      if(settings::bw) {grey=(grey == 1.0) ? 1.0 : 0.0;}
    } 
    else if(settings::rgb && cmyk()) cmyktorgb();
    else if(settings::cmyk && rgb()) rgbtocmyk();
  }   
  
  // Try to upgrade to the specified colorspace.
  bool promote(ColorSpace c) {
    if(color == c) return true;
    
    switch(color) {
      case PATTERN:
      case INVISIBLE:
        break;
      case DEFCOLOR:
      {
        return true;
        break;
      }
      break;
      case GRAYSCALE:
      {
        if(c == RGB) {greytorgb(); return true;}
        else if(c == CMYK) {greytocmyk(); return true;}
        break;
      }
      case RGB:
      {
        if(c == CMYK) {rgbtocmyk(); return true;}
        break;
      }
      case CMYK:
      {
        break;
      }
    }
    return false;
  }
  
  friend pen operator * (double x, const pen& q) {
    pen p=q;
    if(x < 0.0) x = 0.0;
    switch(p.color) {
      case PATTERN:
      case INVISIBLE:
      case DEFCOLOR:
        break;
      case GRAYSCALE:
      {
        p.grey *= x;
        p.greyrange();
        break;
      }
      case RGB:
      {
        p.r *= x;
        p.g *= x;
        p.b *= x;
        p.rgbrange();
        break;
      }
      case CMYK:
      {
        p.r *= x;
        p.g *= x;
        p.b *= x;
        p.grey *= x;
        p.cmykrange();
        break;
      }
    }
    return p;
  }
  
  friend pen operator + (const pen& p, const pen& q) {
    pen P=p;
    pen Q=q;
    
    if(P.color == PATTERN && P.pattern.empty()) P.color=DEFCOLOR;
    ColorSpace colorspace=(ColorSpace) max((Int) P.color,(Int) Q.color);
    
    if(!(p.transparency.isdefault && q.transparency.isdefault))
      P.transparency.opacity=max(p.opacity(),q.opacity());
    
    switch(colorspace) {
      case PATTERN:
      case INVISIBLE:
      case DEFCOLOR:
        break;
      case GRAYSCALE:
      {
        P.grey += Q.grey;
        P.greyrange();
        break;
      }
      
      case RGB:
      {
        if(P.color == GRAYSCALE) P.greytorgb();
        else if(Q.color == GRAYSCALE) Q.greytorgb();
        
        P.r += Q.r;
        P.g += Q.g;
        P.b += Q.b;
        P.rgbrange();
        break;
      }
      
      case CMYK:
      {
        if(P.color == GRAYSCALE) P.greytocmyk();
        else if(Q.color == GRAYSCALE) Q.greytocmyk();
        
        if(P.color == RGB) P.rgbtocmyk();
        else if(Q.color == RGB) Q.rgbtocmyk();
        
        P.r += Q.r;
        P.g += Q.g;
        P.b += Q.b;
        P.grey += Q.grey;
        P.cmykrange();
        break;
      }
    }
    
    return pen(q.line.isdefault ? p.line : q.line,
               q.linewidth == DEFWIDTH ? p.linewidth : q.linewidth,
               q.P.empty() ? p.P : q.P,
               q.font.empty() ? p.font : q.font,
               q.fontsize == 0.0 ? p.fontsize : q.fontsize,
               q.lineskip == 0.0 ? p.lineskip : q.lineskip,
               colorspace,P.r,P.g,P.b,P.grey,
               q.pattern == DEFPAT ? p.pattern : q.pattern,
               q.fillrule == DEFFILL ? p.fillrule : q.fillrule,
               q.baseline == DEFBASE ? p.baseline : q.baseline,
               q.transparency.isdefault ? p.transparency : q.transparency,
               q.linecap == DEFCAP ? p.linecap : q.linecap,
               q.linejoin == DEFJOIN ? p.linejoin : q.linejoin,
               q.miterlimit == DEFMITER ? p.miterlimit : q.miterlimit,
               q.overwrite == DEFWRITE ? p.overwrite : q.overwrite,
               q.t.isNull() ? p.t : q.t);
  }

  friend pen interpolate(const pen& p, const pen& q, double t) {
    pen P=p;
    pen Q=q;
  
    if(P.color == PATTERN && P.pattern.empty()) P.color=DEFCOLOR;
    ColorSpace colorspace=(ColorSpace) max((Int) P.color,(Int) Q.color);
  
    switch(colorspace) {
      case PATTERN:
      case INVISIBLE:
      case DEFCOLOR:
      case GRAYSCALE:
        break;
      case RGB:
      {
        if(P.color == GRAYSCALE) P.greytorgb();
        else if(Q.color == GRAYSCALE) Q.greytorgb();
        break;
      }
      
      case CMYK:
      {
        if(P.color == GRAYSCALE) P.greytocmyk();
        else if(Q.color == GRAYSCALE) Q.greytocmyk();
        
        if(P.color == RGB) P.rgbtocmyk();
        else if(Q.color == RGB) Q.rgbtocmyk();
        break;
      }
    }
    
    return (1-t)*P+t*Q;
  }

  friend bool operator == (const pen& p, const pen& q) {
    return  *(p.linetype()) == *(q.linetype()) 
      && p.width() == q.width() 
      && p.Path() == q.Path()
      && p.Font() == q.Font()
      && p.Lineskip() == q.Lineskip()
      && p.size() == q.size()
      && p.colorspace() == q.colorspace()
      && (!(p.grayscale() || p.cmyk()) || p.gray() == q.gray())
      && (!(p.rgb() || p.cmyk()) || (p.red() == q.red() &&
                                     p.green() == q.green() &&
                                     p.blue() == q.blue()))
      && p.pattern == q.pattern
      && p.Fillrule() == q.Fillrule()
      && p.Baseline() == q.Baseline()
      && p.transp() == q.transp()
      && p.cap() == q.cap()
      && p.join() == q.join()
      && p.miter() == q.miter()
      && p.Overwrite() == q.Overwrite()
      && p.t == q.t;
  }
  
  friend bool operator != (const pen& p, const pen& q) {
    return !(p == q);
  }
  
  friend ostream& operator << (ostream& out, const pen& p) {
    out << "(";
    if(p.line.isdefault)
      out << "default";
    else
      out << p.line.pattern;
    if(p.line.offset)
      out << p.line.offset;
    if(!p.line.scale)
      out << " bp";
    if(!p.line.adjust)
      out << " fixed";
    if(p.linewidth != DEFWIDTH)
      out << ", linewidth=" << p.linewidth;
    if(!p.P.empty())
      out << ", path=" << p.P;
    if(p.linecap != DEFCAP)
      out << ", linecap=" << Cap[p.linecap];
    if(p.linejoin != DEFJOIN)
      out << ", linejoin=" << Join[p.linejoin];
    if(p.miterlimit != DEFMITER)
      out << ", miterlimit=" << p.miterlimit;
    if(!p.font.empty())
      out << ", font=\"" << p.font << "\"";
    if(p.fontsize)
      out << ", fontsize=" << p.fontsize;
    if(p.lineskip)
      out << ", lineskip=" << p.lineskip;
    if(p.color == INVISIBLE)
      out << ", invisible";
    else if(p.color == GRAYSCALE)
      out << ", gray=" << p.grey;
    else if(p.color == RGB)
      out << ", red=" << p.red() << ", green=" << p.green() 
          << ", blue=" << p.blue();
    else if(p.color == CMYK)
      out << ", cyan=" << p.cyan() << ", magenta=" << p.magenta() 
          << ", yellow=" << p.yellow() << ", black=" << p.black();
    if(p.pattern != DEFPAT)
      out << ", pattern=" << "\"" << p.pattern << "\"";
    if(p.fillrule != DEFFILL)
      out << ", fillrule=" << FillRuleTag[p.fillrule];
    if(p.baseline != DEFBASE)
      out << ", baseline=" << BaseLineTag[p.baseline];
    if(!p.transparency.isdefault) {
      out << ", opacity=" << p.transparency.opacity;
      out << ", blend=" << p.transparency.blend;
    }
    if(p.overwrite != DEFWRITE)
      out << ", overwrite=" << OverwriteTag[p.overwrite];
    if(!p.t.isNull())
      out << ", transform=" << p.t;
    out << ")";
    
    return out;
  }

  const transform getTransform() const {
    return t.isNull() ? defaultpen().t : t;
  }
  
  // The bounds of the circle or ellipse the pen describes.
  bbox bounds() const
  {
    double maxx, maxy;
    pair shift;

    if(!P.empty()) return P.bounds();
    
    transform t=getTransform();
    
    if(t.isIdentity()) {
      maxx = 1;
      maxy = 1;
      shift = pair(0,0);
    } else {
      double xx = t.getxx(), xy = t.getxy(),
        yx = t.getyx(), yy = t.getyy();

      // These are the maximum x and y values that a linear transform can map
      // a point in the unit circle.  This can be proven by the Lagrange
      // Multiplier theorem or by properties of the dot product.
      maxx = length(pair(xx,xy));
      maxy = length(pair(yx,yy));

      shift = t*pair(0,0);
    }

    bbox b;
    pair z=0.5*width()*pair(maxx,maxy);
    b += z + shift;
    b += -z + shift;

    return b;
  }

  friend pen transformed(const transform& t, const pen& p) {
    pen ret = p;
    if(!p.P.empty()) ret.P = p.P.transformed(t);
    ret.t = p.t.isNull() ? t : t*p.t;
    return ret;
  }

};
  
pen transformed(const transform& t, const pen &p);
}

#endif
