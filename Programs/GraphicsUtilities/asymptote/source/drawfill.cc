/*****
 * drawfill.cc
 * Andy Hammerlindl 2002/06/06
 *
 * Stores a cyclic path that will outline a filled shape in a picture.
 *****/

#include "drawfill.h"

namespace camp {

using settings::ps2tex;
using settings::tex2ps;

void drawAxialShade::palette(psfile *out)
{
  pentype.convert();
  penb.convert();
  
  colorspace=(ColorSpace) max(pentype.colorspace(),penb.colorspace());
  
  switch(colorspace) {
    case RGB:
    {
      if (pentype.grayscale()) pentype.greytorgb();
      else if (penb.grayscale()) penb.greytorgb();
      break;
    }
      
    case CMYK:
    {
      if (pentype.grayscale()) pentype.greytocmyk();
      else if (penb.grayscale()) penb.greytocmyk();
        
      if (pentype.rgb()) pentype.rgbtocmyk();
      else if (penb.rgb()) penb.rgbtocmyk();
      break;
    }
    default:
      break;
  }
  
  out->gsave();
}  
  
bool drawFill::draw(psfile *out)
{
  if(pentype.invisible() || empty()) return true;
  
  palette(out);
  writepath(out);
  fill(out);
  return true;
}
  
drawElement *drawFill::transformed(const transform& t)
{
  return new drawFill(transpath(t),stroke,transpen(t),KEY);
}
  
drawElement *drawLatticeShade::transformed(const transform& t)
{
  return new drawLatticeShade(transpath(t),stroke,pentype,pens,t*T,KEY);
}

drawElement *drawAxialShade::transformed(const transform& t)
{
  pair A=t*a, B=t*b;
  return new drawAxialShade(transpath(t),stroke,pentype,A,extenda,penb,B,
                            extendb,KEY);
}
  
drawElement *drawRadialShade::transformed(const transform& t)
{
  pair A=t*a, B=t*b;
  double RA=length(t*(a+ra)-A);
  double RB=length(t*(b+rb)-B);
  return new drawRadialShade(transpath(t),stroke,pentype,A,RA,extenda,penb,B,
                             RB,extendb,KEY);
}

drawElement *drawGouraudShade::transformed(const transform& t)
{
  size_t size=vertices.size();
  vm::array *Vertices=new vm::array(size);
  for(size_t i=0; i < size; i++)
    (*Vertices)[i]=t*vm::read<pair>(vertices,i);

  return new drawGouraudShade(transpath(t),stroke,pentype,pens,*Vertices,
                              edges,KEY);
}

drawElement *drawTensorShade::transformed(const transform& t)
{
  size_t size=boundaries.size();
  size_t zsize=z.size();
  vm::array *Boundaries=new vm::array(size);
  vm::array *Z=new vm::array(zsize);
  
  for(size_t i=0; i < size; i++)
    (*Boundaries)[i]=vm::read<path>(boundaries,i).transformed(t);
  
  for(size_t i=0; i < zsize; i++) {
    vm::array *zi=vm::read<vm::array *>(z,i);
    size_t zisize=checkArray(zi);
    vm::array *Zi=new vm::array(zisize);
    (*Z)[i]=Zi;
    for(size_t j=0; j < zisize; j++)
      (*Zi)[j]=t*vm::read<pair>(zi,j);
  }

  return new drawTensorShade(transpath(t),stroke,pentype,pens,*Boundaries,*Z,
    KEY);
}

bool drawFunctionShade::write(texfile *out, const bbox& box)
{
  if(empty()) return true;
  
  ColorSpace colorspace=pentype.colorspace();
  size_t ncomponents=ColorComponents[colorspace];
  
  out->verbatim("\\pdfobj stream attr {/FunctionType 4");
  out->verbatim("/Domain [0 1 0 1]");
  out->verbatim("/Range [");
  for(size_t i=0; i < ncomponents; ++i)
    out->verbatim("0 1 ");
  out->verbatim("]}{{");
  out->verbatimline(shader);
  out->verbatimline("}}%");
  out->verbatimline("\\edef\\lastobj{\\the\\pdflastobj}\\pdfrefobj\\lastobj");
   
  out->verbatim("\\setbox\\ASYbox=\\hbox to ");
  double Hoffset=out->hoffset();
  double hoffset=(bpath.Max().getx()-Hoffset)*ps2tex;
  out->write(hoffset);
  out->verbatim("pt {");
  out->verbatim("\\vbox to ");
  out->write((box.top-box.bottom)*ps2tex);
  out->verbatimline("pt {\\vfil%");
  out->gsave();
  out->beginspecial();
  out->beginraw();
  writeshiftedpath(out);
  if(stroke) strokepath(out);
  out->endclip(pentype);
  out->verbatimline("/Sh sh");
  out->endraw();
  out->endspecial();
  out->grestore();
  out->verbatimline("}\\hfil}%");
  
  out->verbatimline("\\pdfxform resources {");
  out->verbatimline("/Shading << /Sh << /ShadingType 1");
  out->verbatim("/Matrix [");

  out->write(shift(pair(-Hoffset,-box.bottom))*matrix(bpath.Min(),bpath.Max()));
  out->verbatimline("]");
  out->verbatim("/Domain [0 1 0 1]");
  out->verbatim("/ColorSpace /Device");
  out->verbatimline(ColorDeviceSuffix[colorspace]);
  out->verbatimline("/Function \\lastobj\\space 0 R >> >>}\\ASYbox");
  
  out->verbatimline("\\pdfrefxform\\the\\pdflastxform");
  out->verbatim("\\kern");
  out->write(-hoffset);
  out->verbatimline("pt%");
  return true;
}

drawElement *drawFunctionShade::transformed(const transform& t)
{
  return new drawFunctionShade(transpath(t),stroke,pentype,shader,KEY);
}

} // namespace camp
