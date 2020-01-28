/*****
 * pen.cc
 * John Bowman
 *
 *****/

#include "pen.h"
#include "drawelement.h"

namespace camp {

const char* DEFPAT="<default>";
const char* DEFLATEXFONT="\\usefont{\\ASYencoding}{\\ASYfamily}{\\ASYseries}{\\ASYshape}";
const char* DEFCONTEXTFONT="modern";
const char* DEFTEXFONT="cmr12";
const double DEFWIDTH=-1;
const Int DEFCAP=-1;
const Int DEFJOIN=-1;
const double DEFMITER=0;
const transform nullTransform=transform(0.0,0.0,0.0,0.0,0.0,0.0);

const char* PSCap[]={"butt","round","square"};
const char* Cap[]={"square","round","extended"};
const Int nCap=sizeof(Cap)/sizeof(char*);
const char* Join[]={"miter","round","bevel"};
const Int nJoin=sizeof(Join)/sizeof(char*);
const char* OverwriteTag[]={"Allow","Suppress","SupressQuiet",
                            "Move","MoveQuiet"};
const Int nOverwrite=sizeof(OverwriteTag)/sizeof(char*);
const char* FillRuleTag[]={"ZeroWinding","EvenOdd"};
const Int nFill=sizeof(FillRuleTag)/sizeof(char*);
const char* BaseLineTag[]={"NoAlign","Align"};
const Int nBaseLine=sizeof(BaseLineTag)/sizeof(char*);

const char* ColorDeviceSuffix[]={"","","Gray","RGB","CMYK",""};
const unsigned nColorSpace=sizeof(ColorDeviceSuffix)/sizeof(char*);
const char* BlendMode[]={"Compatible","Normal","Multiply","Screen",
                         "Overlay","SoftLight","HardLight",
                         "ColorDodge","ColorBurn","Darken",
                         "Lighten","Difference","Exclusion",
                         "Hue","Saturation","Color","Luminosity"};
const Int nBlendMode=sizeof(BlendMode)/sizeof(char*);

pen drawElement::lastpen;

}
