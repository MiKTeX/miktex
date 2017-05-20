/*
  	#[ License :

    (C) 2016 by authors:
            John Collins (jcc8 at psu dot edu)
            Jos Vermaseren (t68 at nikhef dot nl) 

    axohelp is free software: you can redistribute it and/or modify it under 
    the terms of the GNU General Public License as published by the Free 
    Software Foundation, either version 3 of the License, or (at your option) 
    any later version.

    axohelp is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
    details.

    For the GNU General Public License see <http://www.gnu.org/licenses/>.

  	#] License : 
  	#[ Commentary + Modifications :

    This file contains the source code of the axohelp program that is used
    together with axopdf.sty and pdflatex. It is a conversion of the postscript
    code of axodraw.sty.

    input file contains objects of the type
    [number] axohelp input
    the output contains objects:
      \axo@setObject{label}%
        {input data}%
        {output}
    There may be blank lines and commentary.

 		#[ About folds : (this line starts with one blank and two tabs)

    The internals of the file have been organized in folds.
    These are defined as a range of lines if which the first and last
    lines have a special format. Each starts with any three characters
    (may include tabs), then #[ for the start line and #] for the closing
    line, then both lines need identical name fields, closed by a colon.
    After the colon can be anything. When a fold is closed one should see
    only the first line but with the #[ replaced by ## as in
 		## About folds : (this line starts with one blank and two tabs)
    Folds can be nested.
    This fold concept comes originally from the occam compiler for the
    transputer in the second half of the 1980's although there it was
    implemented differently. It was taken over by the STedi editor in its
    current form. The sources of this editor are available from the form
    home site: http://www.nikhef.nl/~form
    Some people have managed to emulate these folds in editors like emacs
    and vim.

 		#] About folds : 
  	#] Commentary + Modifications :
  	#[ Includes :
*/

#if defined(MIKTEX)
#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#include <miktex/Core/c/api.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/*
  	#] Includes : 
  	#[ Defines :
*/

#define NAME "axohelp"
#define VERSIONDATE "2016 May 23"
#define VERSION 1
#define SUBVERSION 0

#define COMMENTCHAR '%'
#define TERMCHAR ';'
 
#define STROKING 0
#define NONSTROKING 1
 
char **inputs;
long *inputsizes;
double **inargs;
long numinputs = 0;
long inputallocations = 0;
char *axohelp;
FILE *outfile;
char *inname, *outname;
int VerboseFlag = 0;
char outputbuffer[1000000];
char *outpos;
long numobject;
char *nameobject;
int witharrow = 0;
int identification = 0;

static int lastlinefeed = 1;
static double axolinewidth = 0.5;
static struct aRRow {
    double stroke;
    double width;
    double length;
    double inset;
    double where;
    double scale;
    double aspect;
    int    type;     /* 0: old style arrow; 1: Jaxodraw style arrow */
    int    adjust;   /* whether the line length should be adjusted */
} arrow;
double linesep = 0;
int flip = 0;
int clockwise = 0;
 
void OutputString(char *);
void ArrowHead();
void GetArrow(double *);
void BezierArrow(double *);
void ArcSegment(double,double,double);
double *ReadArray(char *,int *,int *);
double *ReadTail(char *,int *);
double LengthBezier(double,double,double,double,double,double,double);

double M_pi;
double torad;
#define COS(x) cos((x)*torad)
#define SIN(x) sin((x)*torad)
#define TAN(x) tan((x)*torad)
 
typedef void (*TFUN)(double *);

typedef struct {
    char *name;
    TFUN func;
    int numargs;
    int colortype;
} KEYWORD;

void GluonHelp(double *,double);
void DoubleGluonHelp(double *,double,double,double,double);
void GluonCircHelp(double *);
void GluonArcHelp(double *,double,double);
void PhotonHelp(double *,double);
void PhotonArcHelp(double *,double,int);
void ZigZagHelp(double *,double);
void ZigZagArcHelp(double *);
double ComputeDash(double *,double,double);
double ComputeDashCirc(double *,double);

void ArrowArc(double *);
void ArrowArcn(double *);
void ArrowDoubleArc(double *);
void ArrowLine(double *);
void ArrowDoubleLine(double *);
void AxoArc(double *);
void AxoBezier(double *);
void AxoGluon(double *);
void AxoGluonArc(double *);
void AxoGluonCirc(double *);
void AxoLine(double *);
void AxoPhoton(double *);
void AxoPhotonArc(double *);
void AxoZigZag(double *);
void AxoZigZagArc(double *);
void BezierCurve(double *);
void Boxc(double *);
void CArc(double *);
void DashArrowArc(double *);
void DashArrowArcn(double *);
void DashArrowLine(double *);
void DashArrowDoubleArc(double *);
void DashArrowDoubleLine(double *);
void DashBezier(double *);
void DashCArc(double *);
void DashDoubleArc(double *);
void DashDoubleBezier(double *);
void DashDoubleLine(double *);
void DashDoublePhoton(double *);
void DashDoublePhotonArc(double *);
void DashDoubleZigZag(double *);
void DashDoubleZigZagArc(double *);
void DashGluon(double *);
void DashGluonArc(double *);
void DashGluonCirc(double *);
void DashPhoton(double *);
void DashPhotonArc(double *);
void DashZigZag(double *);
void DashZigZagArc(double *);
void DashLine(double *);
void DoubleArc(double *);
void DoubleBezier(double *);
void DoubleLine(double *);
void DoublePhoton(double *);
void DoublePhotonArc(double *);
void DoubleZigZag(double *);
void DoubleZigZagArc(double *);
void EBox(double *);
void EBoxc(double *);
void ECirc(double *);
void ETri(double *);
void FBox(double *);
void FBoxc(double *);
void FOval(double *);
void FTri(double *);
void GluonArc(double *);
void GluonCirc(double *);
void Gluon(double *);
void Grid(double *);
void LinAxis(double *);
void Line(double *);
void LogAxis(double *);
void Oval(double *);
void Photon(double *);
void PhotonArc(double *);
void Rotate(double *);
void Vertex(double *);
void ZigZag(double *);
void ZigZagArc(double *);

void Curve(double *,int);
void DashCurve(double *,int);
void Polygon(double *,int,int);

void Inivars(void);

KEYWORD commands[] = {
     { "Line",               Line,               4,    STROKING }
    ,{ "ArrowArc",           ArrowArc,           7,    STROKING }
    ,{ "ArrowArcn",          ArrowArcn,          7,    STROKING }
    ,{ "ArrowDoubleArc",     ArrowDoubleArc,     8,    STROKING }
    ,{ "ArrowLine",          ArrowLine,          6,    STROKING }
    ,{ "ArrowDoubleLine",    ArrowDoubleLine,    7,    STROKING }
    ,{ "AxoArc",             AxoArc,             17,   STROKING }
    ,{ "AxoBezier",          AxoBezier,          19,   STROKING }
    ,{ "AxoGluon",           AxoGluon,           8,    STROKING }
    ,{ "AxoGluonArc",        AxoGluonArc,        10,   STROKING }
    ,{ "AxoGluonCirc",       AxoGluonCirc,       8,    STROKING }
    ,{ "AxoLine",            AxoLine,            15,   STROKING }
    ,{ "AxoPhoton",          AxoPhoton,          8,    STROKING }
    ,{ "AxoPhotonArc",       AxoPhotonArc,       10,   STROKING }
    ,{ "AxoZigZag",          AxoZigZag,          8,    STROKING }
    ,{ "AxoZigZagArc",       AxoZigZagArc,       10,   STROKING }
    ,{ "Bezier",             BezierCurve,        8,    STROKING }
    ,{ "Boxc",               Boxc,               4,    STROKING }
    ,{ "CArc",               CArc,               5,    STROKING }
    ,{ "DashArrowArc",       DashArrowArc,       8,    STROKING }
    ,{ "DashArrowArcn",      DashArrowArcn,      8,    STROKING }
    ,{ "DashArrowDoubleArc", DashArrowDoubleArc, 9,    STROKING }
    ,{ "DashArrowDoubleLine",DashArrowDoubleLine,8,    STROKING }
    ,{ "DashArrowLine",      DashArrowLine,      7,    STROKING }
    ,{ "DashBezier",         DashBezier,         9,    STROKING }
    ,{ "DashCArc",           DashCArc,           6,    STROKING }
    ,{ "DashDoubleArc",      DashDoubleArc,      7,    STROKING }
    ,{ "DashDoubleBezier",   DashDoubleBezier,   10,   STROKING }
    ,{ "DashDoubleLine",     DashDoubleLine,     6,    STROKING }
    ,{ "DashDoublePhoton",   DashDoublePhoton,   8,    STROKING }
    ,{ "DashDoublePhotonArc",DashDoublePhotonArc,10,   STROKING }
    ,{ "DashDoubleZigZag",   DashDoubleZigZag,   8,    STROKING }
    ,{ "DashDoubleZigZagArc",DashDoubleZigZagArc,10,   STROKING }
    ,{ "DashGluon",          DashGluon,          7,    STROKING }
    ,{ "DashGluonArc",       DashGluonArc,       10,   STROKING }
    ,{ "DashGluonCirc",      DashGluonCirc,      7,    STROKING }
    ,{ "DashLine",           DashLine,           5,    STROKING }
    ,{ "DashPhoton",         DashPhoton,         7,    STROKING }
    ,{ "DashPhotonArc",      DashPhotonArc,      10,   STROKING }
    ,{ "DashZigZag",         DashZigZag,         7,    STROKING }
    ,{ "DashZigZagArc",      DashZigZagArc,      10,   STROKING }
    ,{ "DoubleArc",          DoubleArc,          6,    STROKING }
    ,{ "DoubleBezier",       DoubleBezier,       9,    STROKING }
    ,{ "DoubleLine",         DoubleLine,         5,    STROKING }
    ,{ "DoublePhoton",       DoublePhoton,       7,    STROKING }
    ,{ "DoublePhotonArc",    DoublePhotonArc,    8,    STROKING }
    ,{ "DoubleZigZag",       DoubleZigZag,       7,    STROKING }
    ,{ "DoubleZigZagArc",    DoubleZigZagArc,    8,    STROKING }
    ,{ "EBox",               EBox,               4,    STROKING }
    ,{ "FBox",               FBox,               4,    STROKING }
    ,{ "FBoxc",              FBoxc,              4,    STROKING }
    ,{ "ECirc",              ECirc,              3,    STROKING }
    ,{ "ETri",               ETri,               6,    STROKING }
    ,{ "FOval",              FOval,              5,    NONSTROKING }
    ,{ "FTri",               FTri,               6,    NONSTROKING }
    ,{ "GluonArc",           GluonArc,           7,    STROKING }
    ,{ "GluonCirc",          GluonCirc,          6,    STROKING }
    ,{ "Gluon",              Gluon,              6,    STROKING }
    ,{ "Grid",               Grid,               6,    STROKING }
    ,{ "LinAxis",            LinAxis,            8,    STROKING }
    ,{ "LogAxis",            LogAxis,            7,    STROKING }
    ,{ "Oval",               Oval,               5,    STROKING }
    ,{ "Photon",             Photon,             6,    STROKING }
    ,{ "PhotonArc",          PhotonArc,          7,    STROKING }
    ,{ "Rotate",             Rotate,             7,    NONSTROKING }
    ,{ "Vertex",             Vertex,             3,    NONSTROKING }
    ,{ "ZigZag",             ZigZag,             6,    STROKING }
    ,{ "ZigZagArc",          ZigZagArc,          7,    STROKING }
};

/*
  	#] Defines : 
  	#[ SetDefaults :
*/

void SetDefaults()
{
    lastlinefeed = 1;
    axolinewidth = 0.5;
    linesep = 0;
    flip = 0;
    clockwise = 0;
    witharrow = 0;
}

/*
  	#] SetDefaults : 
  	#[ PDF utilities :

    These routines are included to make the program more readable and easier
    to write. It also allows the easy use of the OutputString routine that
    compactifies the output.
*/

#define Stroke outpos += sprintf(outpos," S")
#define CloseAndStroke outpos += sprintf(outpos," h S")
#define Fill outpos += sprintf(outpos," f")
#define CloseAndFill outpos += sprintf(outpos," h f")
#define SaveGraphicsState outpos += sprintf(outpos," q")
#define RestoreGraphicsState outpos += sprintf(outpos," Q")

void Bezier(double x1,double y1,double x2,double y2,double x3,double y3) {
    outpos +=
    sprintf(outpos,"\n %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f c",x1,y1,x2,y2,x3,y3);
}

void LineTo(double x1,double y1) {
    outpos +=
    sprintf(outpos,"\n %12.3f %12.3f l",x1,y1);
}

void MoveTo(double x1,double y1) {
    outpos +=
    sprintf(outpos,"\n %12.3f %12.3f m",x1,y1);
}

void SetLineWidth(double w) {
    outpos +=
    sprintf(outpos," %12.3f w",w);
}

void SetDashSize(double dashsize,double phase) {
    if ( dashsize ) outpos += sprintf(outpos," [%12.3f] %12.3f d",dashsize,phase);
    else outpos += sprintf(outpos," [] 0 d");
}

void SetTransferMatrix(double x11,double x12,double x21,double x22,double x,double y)
{
    if ( ( fabs(x11-1.) > 0.001 ) || ( fabs(x22-1.) > 0.001 )
         || ( fabs(x12) > 0.001 ) || ( fabs(x21) > 0.001 )
         || ( fabs(x) > 0.001 ) || ( fabs(y) > 0.001 ) ) {
        outpos +=
        sprintf(outpos,"%12.3f %12.3f %12.3f %12.3f %12.3f %12.3f cm\n",x11,x12,x21,x22,x,y);
    }
}

static double BzK;

void BezierCircle(double r,char *action)
{
    outpos +=
    sprintf(outpos," %12.3f 0 m %12.3f %12.3f %12.3f %12.3f 0 %12.3f c\n",-r,-r,r*BzK,-r*BzK,r,r);
    outpos +=
    sprintf(outpos," %12.3f %12.3f %12.3f %12.3f %12.3f 0 c\n",r*BzK,r,r,r*BzK,r);
    outpos +=
    sprintf(outpos," %12.3f %12.3f %12.3f %12.3f 0 %12.3f c\n",r,-r*BzK,r*BzK,-r,-r);
    outpos +=
    sprintf(outpos," %12.3f %12.3f %12.3f %12.3f %12.3f 0 c %s\n",-r*BzK,-r,-r,-r*BzK,-r,action);
}

void BezierOval(double w, double h, char *action)
{
    outpos +=
    sprintf(outpos," %12.3f 0 m %12.3f %12.3f %12.3f %12.3f 0 %12.3f c\n",-w,-w,h*BzK,-w*BzK,h,h);
    outpos +=
    sprintf(outpos," %12.3f %12.3f %12.3f %12.3f %12.3f 0 c\n",w*BzK,h,w,h*BzK,w);
    outpos +=
    sprintf(outpos," %12.3f %12.3f %12.3f %12.3f 0 %12.3f c\n",w,-h*BzK,w*BzK,-h,-h);
    outpos +=
    sprintf(outpos," %12.3f %12.3f %12.3f %12.3f %12.3f 0 c %s\n",-w*BzK,-h,-w,-h*BzK,-w,action);
}

void SetGray(double grayscale,int par)
{
    if ( par == STROKING ) {
        outpos += sprintf(outpos," %12.3f G",grayscale);
    }
    else {
        outpos += sprintf(outpos," %12.3f g",grayscale);
    }
}

void SetColor(double c, double m, double y, double k,int par)
{
    if ( par == STROKING ) {
        outpos += sprintf(outpos," %12.3f %12.3f %12.3f %12.3f K",c,m,y,k);
    }
    else {
        outpos += sprintf(outpos," %12.3f %12.3f %12.3f %12.3f k",c,m,y,k);
    }
}

void SetBackgroundColor(int par)
{
    if ( par == STROKING ) { outpos += sprintf(outpos," 0 0 0 0 K"); }
    else                   { outpos += sprintf(outpos," 0 0 0 0 k"); }
}

void Rectangle(double x,double y,double w,double h) {
    outpos += sprintf(outpos,"\n %12.3f %12.3f %12.3f %12.3f re",x,y,w,h);
}

void Triangle(double x1,double y1,double x2,double y2,double x3,double y3) {
    outpos +=
    sprintf(outpos,"\n %12.3f %12.3f m %12.3f %12.3f l %12.3f %12.3f l h",x1,y1,x2,y2,x3,y3);
}

/*
  	#] PDF utilities : 
  	#[ Service routines :
 		#[ GetArrow :
*/

void GetArrow(double *args)
{
    witharrow = args[7];
    arrow.stroke = args[0];
    arrow.width  = args[1];
    arrow.length = args[2];
    arrow.inset  = args[3];
    arrow.scale  = args[4];
    arrow.aspect = args[5];
    arrow.where  = args[6];
    arrow.type   = 1;
    arrow.adjust  = 0;
    if ( args[8] ) flip = 1;
    if ( witharrow ) {
        if ( arrow.length == 0 && arrow.width == 0 ) {
                arrow.width = (axolinewidth + 0.7*linesep + 1 ) * 1.2;
            if (arrow.width < 2.5) arrow.width = 2.5;
            arrow.length = 2*arrow.width*arrow.aspect;
        }
        else if ( arrow.width == 0 ) {
            arrow.width = arrow.length/(2*arrow.aspect);
        }
        else if ( arrow.length == 0 ) {
            arrow.length = 2*arrow.width*arrow.aspect;
        }
        arrow.width *= arrow.scale;
        arrow.length *= arrow.scale;
        if ( arrow.where > 1 ) { arrow.where = 1; arrow.adjust = 1; }
        if ( arrow.where < 0 ) { arrow.where = 0; arrow.adjust = 1; }
    }
}

/*
 		#] GetArrow : 
 		#[ ArrowHead :

    Places an arrowhead of a given size at 0 in the +x direction
    The size: Full width is 2*size and full length is also 2*size.
*/

void ArrowHead()
/*
    Jaxodraw style arrows
*/
{
    int k;
    double length;
    SaveGraphicsState;
    if ( flip ) length = -arrow.length;
    else        length =  arrow.length;
    SetDashSize(0,0);
    if ( arrow.stroke ) {
        SetLineWidth(arrow.stroke);
        for (k = 1; k <= 2; k++ ) {
            SaveGraphicsState;
            MoveTo(length*0.5,0);
            LineTo(-length*0.5,arrow.width);
            LineTo(-length*0.5+length*arrow.inset,0);
            LineTo(-length*0.5,-arrow.width);
            if (k == 1) {
                SetBackgroundColor(NONSTROKING);
                outpos += sprintf(outpos," h f");
            }
            else {
                outpos += sprintf(outpos," s");
            }
            RestoreGraphicsState;
        }
    }
    else {
        MoveTo(length*0.5,0);
        LineTo(-length*0.5,arrow.width);
        LineTo(-length*0.5+length*arrow.inset,0);
        LineTo(-length*0.5,-arrow.width);
        outpos += sprintf(outpos," h f");
    }
    RestoreGraphicsState;
}

/*
 		#] ArrowHead : 
 		#[ BezierArrow :

        We compute the length of the curve.
        Then we try to find the t value for which holds:
            Length(t)/Length(1) = arrow.where
*/

void BezierArrow(double *args)
{
    double t,u,x,y,dx,dy,dr,len,tlen,tmin=0,tmax=1.0;
    double x1=args[2]-args[0],x2=args[4]-args[0],x3=args[6]-args[0];
    double y1=args[3]-args[1],y2=args[5]-args[1],y3=args[7]-args[1];
    len = LengthBezier(x1,y1,x2,y2,x3,y3,1.0);
    t = arrow.where;
    tlen = LengthBezier(x1,y1,x2,y2,x3,y3,t);
    while ( fabs(tlen/len-arrow.where) > 0.0001 ) {
        if ( tlen/len > arrow.where ) {
            tmax = t;
            t = 0.5*(tmin+t);
        }
        else {
            tmin = t;
            t = 0.5*(tmax+t);
        }
        tlen = LengthBezier(x1,y1,x2,y2,x3,y3,t);
    }
    u = 1-t;
    x = args[0]*u*u*u+(3*args[2]*u*u+(3*args[4]*u+args[6]*t)*t)*t;
    y = args[1]*u*u*u+(3*args[3]*u*u+(3*args[5]*u+args[7]*t)*t)*t;
    dx = 3*(-args[0]*u*u+args[2]*u*(1-3*t)+args[4]*t*(2-3*t)+args[6]*t*t);
    dy = 3*(-args[1]*u*u+args[3]*u*(1-3*t)+args[5]*t*(2-3*t)+args[7]*t*t);
    dr = sqrt(dx*dx+dy*dy);
    SetTransferMatrix(1,0,0,1,x,y);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);
    ArrowHead();
}

/*
 		#] BezierArrow : 
 		#[ ArcSegment :
*/

void ArcSegment(double r, double phi1, double dphi)
{
    double xphi, d, x, y, phia, phib, x1,y1,x2,y2,x3,y3;
    int num, i;

    num = dphi/90.0001+1.;  /* number of segments we should use. */
    xphi = dphi/num;        /* arc size of each segment */
    d = 4.*TAN(xphi/4.)/3.; /* the magic distance for the control points */

    x = r*COS(phi1); y = r*SIN(phi1);
    MoveTo(x,y);
    for ( i = 0; i < num; i++ ) {
        phia = phi1+i*xphi; /* Start of segment */
        phib = phia+xphi;   /* End of segment */
        x3 = r*COS(phib); y3 = r*SIN(phib);
        x1 = x - y*d;     y1 = y + x*d;
        x2 = x3+y3*d;     y2 = y3-x3*d;

        Bezier(x1,y1,x2,y2,x3,y3);

        x = x3; y = y3;
    }
}

/*
 		#] ArcSegment : 
 		#[ ReadNumber :
*/

char *ReadNumber(char *s,double *num)
{
    double x3,minus = 1;
    int x1,x2;
    while ( *s == '+' || *s == '-' ) {
        if ( *s == '-' ) minus = -minus;
        s++;
    }
    x1 = x2 = 0; x3 = 1;
    while ( *s <= '9' && *s >= '0' ) { x1 = 10*x1 + *s++ - '0'; }
    if ( *s == 0 ) { *num = x1*minus; return(s); }
    if ( *s == '.' ) {
        s++;
        while ( *s >= '0' && *s <= '9' ) { x2 = 10*x2 + *s++ - '0'; x3 *= 10; }
    }
    *num = minus*((double)x1 + x2/x3);
    return(s);
}

/*
 		#] ReadNumber : 
 		#[ ReadArray :

    Reads a tail of floats of the type (x1,y1)(x2,y2),...,(xn,yn) or
    (x1,y1)(x2,y2),...,(xn,yn) f1 ... fm
    The floats may be written as integers.
    Normally they are in the format ####.###
    In num1 we return the number of coordinates encountered.
    In num2 we return the number of extra floats encountered.
    The array of the return value should be freed after it has been used.
*/

double *ReadArray(char *inbuf, int *num1, int *num2)
{
    int argsize = 0, newsize, num = 0, i;
    double *args = 0, *newargs = 0, *extraargs, x, y;
    char *s;
    *num2 = 0;
    s = inbuf;
    while ( *s == ' ' || *s == '\t' || *s == '\n' ) s++;
    if ( *s == '"' ) {
        s++;
        while ( *s == ' ' || *s == '\t' || *s == '\n' ) s++;
    }
    while ( *s == '(' ) { /* We need to read (x,y) */
        s++;
        while ( *s == ' ' || *s == '\n' || *s == '\t' ) s++;
        s = ReadNumber(s,&x);
        while ( *s == ' ' ) s++;
        if ( *s != ',' ) {
            fprintf(stderr,"%s: Illegal format for array of numbers in object %ld of type %s in file %s\n"
                ,axohelp,numobject,nameobject,inname);
            free(args);
            return(0);
        }
        s++;
        while ( *s == ' ' || *s == '\n' || *s == '\t' ) s++;
        s = ReadNumber(s,&y);
        while ( *s == ' ' || *s == '\n' || *s == '\t' ) s++;
        if ( *s != ')' ) {
            fprintf(stderr,"%s: Illegal format for array of numbers in object %ld of type %s in file %s\n"
                ,axohelp,numobject,nameobject,inname);
            free(args);
            return(0);
        }
        s++;
        while ( *s == ' ' ) s++;
        num += 2;
        if ( num >= argsize ) {
            if ( argsize == 0 ) newsize = 20;
            else newsize = 2*argsize;
            newargs = (double *)malloc(sizeof(double)*newsize);
            if ( args == 0 ) { args = newargs; argsize = newsize; }
            else {
                for ( i = 0; i < argsize; i++ ) newargs[i] = args[i];
                free(args);
                args = newargs; argsize = newsize;
            }
        }
        args[num-2] = x; args[num-1] = y;
    }
    if ( *s == '"' ) s++;
    while ( *s == ' ' || *s == '\t' || *s == '\n' ) s++;
    *num1 = num/2;

    if ( *s ) {
        if ( ( extraargs = ReadTail(s,num2) ) == 0 ) {
            return(0);
        }
        if ( num+*num2 > argsize ) {
            newargs = (double *)malloc(sizeof(double)*(num+*num2));
            for ( i = 0; i < num; i++ ) newargs[i] = args[i];
            free(args);
            args = newargs;
            argsize = num+*num2;
        }
        for ( i = 0; i < *num2; i++ ) args[num+i] = extraargs[i];
        free(extraargs);
    }
    return(args);
}

/*
 		#] ReadArray : 
 		#[ ReadTail :

    Reads a command tail that consists of floating point numbers in the
    notation xxx.yyy or just as integers without even the decimal point.
    The output array is allocated and should be returned in a well behaved
    program.
*/

double *ReadTail(char *buff,int *number)
{
    char *s;
    int num = 1, i;
    double *outargs;
    s = buff;
    while ( *s ) {
        if ( *s == ' ' || *s == '\t' || *s == '\n' ) {
            num++; *s++ = 0;
            while ( *s == ' ' || *s == '\t' || *s == '\n' ) *s++ = 0;
        }
        else s++;
    }
    outargs = (double *)malloc(num*sizeof(double));
    s = buff;
    for ( i = 0; i < num; i++ ) {
        while ( *s == 0 ) s++;
        s = ReadNumber(s,outargs+i);
        if ( *s == 'p' && s[1] == 't' ) s += 2;
        if ( *s != 0 ) {
            fprintf(stderr,"%s: Illegal format for number in command %ld (%s) in file %s.\n"
                ,axohelp,numobject,nameobject,inname);
            free(outargs);
            return(0);
        }
    }
    *number = num;
    return(outargs);
}

/*
 		#] ReadTail : 
 		#[ DoCurve :

        Interpolation curve expressed as a Bezier curve.
*/

void DoCurve(double x0, double y0, double x1, double y1,
             double x2, double y2, double x3, double y3)
{
    double xx1, yy1, xx2, yy2;
    xx1 = (2*x1+x2)/3;
    yy1 = ((y1-y0)/(x1-x0)*(x2-x0)+(y2-y0)/(x2-x0)*(x1-x0)+y1+2*y0)/3;
    xx2 = (x1+2*x2)/3;
    yy2 = ((y2-y3)/(x2-x3)*(x1-x3)+(y1-y3)/(x1-x3)*(x2-x3)+y2+2*y3)/3;

    Bezier(xx1,yy1,xx2,yy2,x2,y2);
}

/*
 		#] DoCurve : 
 		#[ LengthBezier :

        Routine computes the length of a Bezier curve.
        Method:
            x = x0*(1-t)^3+3*x1*t*(1-t)^2+3*x2*t^2*(1-t)+x3*t^3
            y = y0*(1-t)^3+3*y1*t*(1-t)^2+3*y2*t^2*(1-t)+y3*t^3
            We assume that x0=y0=0. (Hence call with x1-x0 etc)
        --> dx/dt = 3*x1*(1-t)*(1-3*t)+3*x2*t*(2-3*t)+3*x3*t^2
                  = 3*(x1+2*t*(x2-2*x1)+t^2*(x3-3*x2+3*x1))
        --> L = int_0^1 dt * sqrt(dx^2+dy^2)
        We use ordinary Gaussian quadratures over the domain -1,...,+1

        We have here quadratures for 8, 16 or 32 points.
        For the moment we use the 16 point quadrature. It seems to work well.
        The numbers were taken from Abramowitz and Stegun.
*/

typedef struct quad {
    double x;
    double w;
} QUAD;

QUAD g8[4] = {
     { 0.183434642495650, 0.362683783378362 }
    ,{ 0.525532409916329, 0.313706645877887 }
    ,{ 0.796666477413627, 0.222381034453374 }
    ,{ 0.960289856497536, 0.101228536290376 }
    };
QUAD g16[8] = {
     { 0.095012509837637440185, 0.189450610455068496285 }
    ,{ 0.281603550779258913230, 0.182603415044923588867 }
    ,{ 0.458016777657227386342, 0.169156519395002538189 }
    ,{ 0.617876244402643748447, 0.149595988816576732081 }
    ,{ 0.755404408355003033895, 0.124628971255533872052 }
    ,{ 0.865631202387831743880, 0.095158511682492784810 }
    ,{ 0.944575023073232576078, 0.062253523938647892863 }
    ,{ 0.989400934991649932596, 0.027152459411754094852 }
    };
QUAD g32[16] = {
     { 0.048307665687738316235, 0.096540088514727800567 }
    ,{ 0.144471961582796493485, 0.095638720079274859419 }
    ,{ 0.239287362252137074545, 0.093844399080804565639 }
    ,{ 0.331868602282127649780, 0.091173878695763884713 }
    ,{ 0.421351276130635345364, 0.087652093004403811143 }
    ,{ 0.506899908932229390024, 0.083311924226946755222 }
    ,{ 0.587715757240762329041, 0.078193895787070306472 }
    ,{ 0.663044266930215200975, 0.072345794108848506225 }
    ,{ 0.732182118740289680387, 0.065822222776361846838 }
    ,{ 0.794483795967942406963, 0.058684093478535547145 }
    ,{ 0.849367613732569970134, 0.050998059262376176196 }
    ,{ 0.896321155766052123965, 0.042835898022226680657 }
    ,{ 0.934906075937739689171, 0.034273862913021433103 }
    ,{ 0.964762255587506430774, 0.025392065309262059456 }
    ,{ 0.985611511545268335400, 0.016274394730905670605 }
    ,{ 0.997263861849481563545, 0.007018610009470096600 }
    };

double LengthBezier(double x1,double y1,double x2,double y2,double x3,double y3,double tmax)
{
    double xa = 3*x1, xb = 6*(x2-2*x1), xc = 3*(x3-3*x2+3*x1);
    double ya = 3*y1, yb = 6*(y2-2*y1), yc = 3*(y3-3*y2+3*y1);
    double t, sum = 0, dx, dy;
    int j;

    for ( j = 0; j < 8; j++ ) {
        t = 0.5*(1+g16[j].x)*tmax;
        dx = xa+t*(xb+t*xc);
        dy = ya+t*(yb+t*yc);
        sum += 0.5*g16[j].w*sqrt(dx*dx+dy*dy);
        t = 0.5*(1-g16[j].x)*tmax;
        dx = xa+t*(xb+t*xc);
        dy = ya+t*(yb+t*yc);
        sum += 0.5*g16[j].w*sqrt(dx*dx+dy*dy);
    }
    return(sum*tmax);
}

/*
 		#] LengthBezier : 
  	#] Service routines : 
  	#[ ScanForObjects :

    Routine reads the input buffer and sets up pointers to the commands.
    Basically it is responsible for checking the input syntax and making
    sure all commands will have the proper number of arguments.
*/

long ScanForObjects(char *buffer)
{
    char *s = buffer, *t;
    if ( *s == COMMENTCHAR ) { while ( *s && *s != '\n' ) s++; }
    while ( *s ) { /* here we are to look for a new command */
        if ( *s != '[' ) { /* for the case of the very first character */
            while ( *s && ( *s != '[' || ( *s == '[' && s[-1] == '\\' ) ) ) {
                if ( *s == ' ' || *s == '\n' ) {
                    while ( *s == ' ' || *s == '\n' ) s++; /* Skip blank lines */
                }
                else if ( *s == COMMENTCHAR && s[-1] != '\\' ) {
                    while ( *s && *s != '\n' ) s++;
                }
                else s++;
            }
        }
        if ( *s == 0 ) break;
/*
        if everything is correct we are now on a [
        a: look for matching ] in the same line. This defines the label.
*/
        t = s+1;
        while ( *t && *t != ']' && *t != '\n' && ( *t != COMMENTCHAR ||
            ( *t == COMMENTCHAR && t[-1] != '\\' ) ) ) t++;
        if ( *t == 0 ) {
            fprintf(stderr,"%s: irregular end of file %s.\n",axohelp,inname);
            return(-1);
        }
        else if ( *t == '\n' ) {
            fprintf(stderr,"%s: a label in file %s should inside a single line.\n",axohelp,inname);
            return(-1);
        }
        else if ( *t == COMMENTCHAR ) {
            fprintf(stderr,"%s: illegal comment character inside a label in file %s.\n",axohelp,inname);
            return(-1);
        }
        else if ( *t != ']' ) {
            fprintf(stderr,"%s: internal error reading a label in file %s.\n",axohelp,inname);
            return(-1);
        }
/*
        Store the address of this command
*/
        if ( numinputs >= inputallocations ) {  /* we need more space */
            long newnum, i;
            char **newadd;
            if ( inputallocations == 0 ) { newnum = 100; }
            else { newnum = 2*inputallocations; }
            if ( ( newadd = (char **)malloc(newnum*sizeof(char *)) ) == 0 ) {
                fprintf(stderr,"%s: memory error reading file %s\n",axohelp,inname);
                return(-1);
            }
            for ( i = 0; i < inputallocations; i++ ) { newadd[i] = inputs[i]; }
            if ( inputs != 0 ) free(inputs);
            inputs = newadd;
            inputallocations = newnum;
        }
        inputs[numinputs++] = s;
/*
        Now scan for the first comment character. That is the end of the object.
*/
        while ( *t && ( *t != TERMCHAR || ( *t == TERMCHAR && t[-1] == '\\' ) )
        && *t != '[' ) t++;
        if ( *t != '[' ) {
            while ( t[-1] == ' ' || t[-1] == '\n' ) t--;
            *t++ = 0;
        }
        s = t;
    }
    return(numinputs);
}

/*
  	#] ScanForObjects : 
  	#[ ReadInput :
*/

char *ReadInput(char *filename)
{
    FILE *finput;
    long filesize, num;
    char *buffer;
    if ( ( finput = fopen(filename,"r") ) == 0 ) {
        fprintf(stderr,"%s: Cannot open file %s\n",axohelp,filename);
        exit(-1);
    }
    if ( ( fseek(finput,0,SEEK_END) != 0 )
      || ( ( filesize = ftell(finput) ) < 0 )
      || ( fseek(finput,0,SEEK_SET) != 0 ) ) {
        fprintf(stderr,"%s: File error in file %s\n",axohelp,filename);
        exit(-1);
    }
    if ( ( buffer = malloc((filesize+1)*sizeof(char)) ) == 0 ) {
        fprintf(stderr,"%s: Error allocating %ld bytes of memory",axohelp,filesize+1);
        exit(-1);
    }
/*
        Assume character in file is 1 byte, which is true for all cases
        we currently encounter.
*/
    num = fread( buffer, 1, filesize, finput );
    if ( ferror(finput) ) {
        fprintf(stderr,"%s: Error reading file %s\n",axohelp,filename);
        exit(-1);
    }
/*
        By definition, fread reads ALL the items specified, or it gets to
        end-of-file, or there is an error.
        It returns the actual number of items successfully read, which
        is less than the number given in the 3rd argument ONLY if a
        read error or end-of-file is encountered.
        We have already tested for an error.
        But num could legitimately be less than filesize, because of
        translation of CRLF to LF (on MSWindows with MSWindows text file).
*/
    buffer[num] = 0;
    fclose(finput);
    return(buffer);
}

/*
  	#] ReadInput : 
  	#[ CleanupOutput :
*/

void CleanupOutput(char *str)
{
    char *s, *t;
    int period = 0;
    s = t = str;
    while ( *s && *s != '}' ) {
        if ( *s == '\n' ) *s = ' ';
        if ( ( *s == ' ' || *s == '\n' ) && ( s[1] == ' ' || s[1] == '\n' ) ) s++;
        else *t++ = *s++;
    }
    while ( *s ) *t++ = *s++;
    *t = 0;
    s = t = str;
    while ( *s ) {
        if ( *s == '.' ) { period = 1; *t++ = *s++; }
        else if ( *s == '-' && s[1] == '0' && s[2] == ' ' ) { s++; }
        else if ( *s <= '9' && *s >= '0' ) { *t++ = *s++; }
        else if ( *s == '\n' && ( t > str && t[-1] == '\n' ) ) { s++; }
        else if ( period ) {
            while ( t > str && t[-1] == '0' ) t--;
            if ( t > str && t[-1] == '.' ) t--;
            while ( *s == ' ' && s[1] == ' ' ) s++;
            period = 0; *t++ = *s++;
        }
        else if ( *s == ' ' && s[1] == ' ' ) s++;
        else {
            period = 0; *t++ = *s++;
        }
    }
    *t = 0;
    s = t = str;
    while ( *s ) {
        if ( *s == '-' && s[1] == '0' && s[2] == ' ' ) { s++; }
        else *t++ = *s++;
    }
    *t = 0;
}

/*
  	#] CleanupOutput : 
  	#[ DoOneObject :
*/

int DoOneObject(char *cinput)
{
    int num, i, num1, num2;
    char *s, *t, *StartClean;
    double *argbuf = 0;
    SetDefaults();
    s = cinput; while ( *s != '[' ) s++;
    s++; t = s; while ( *t != ']' ) t++;
    *t++ = 0; while ( *t == ' ' || *t == '\t' || *t == '\n' ) t++;
    outpos = outputbuffer;
    outpos += sprintf(outpos,"\\axo@setObject{%s}%%\n{%s%c}%%\n{",s,t,TERMCHAR);
    if ( *s == '0' && s[1] == ']' ) {
/*
        The identification line.
        In due time we might add more options here.
*/
        if ( strcmp(nameobject,"AxodrawWantsPDF") == 0 ) {
            identification = 1;
            outpos += sprintf(outpos,"Axohelp version %d.%d. PDF output.}",VERSION,SUBVERSION);
            fprintf(outfile,"%s",outputbuffer);
            return(0);
        }
        else {
            fprintf(stderr,"%s: Illegal request in identification string [0]: %s\n"
                        ,axohelp,nameobject);
            if ( argbuf ) free(argbuf);
            return(-1); 
        }
    }
/*
    if ( identification == 0 ) {
        fprintf(stderr,"%s: No identification string. Check versions.\n",axohelp);
        if ( argbuf ) free(argbuf);
        return(-1); 
    }
*/
    StartClean = outpos;

    nameobject = t; while ( *t != ' ' && *t != '\t' && *t != '\n' && *t ) t++;
    *t++ = 0; while ( *t == ' ' || *t == '\t' || *t == '\n' ) t++;
/*
        Now nameobject is the name of the command and t points at the first parameter.
*/
    if ( ( strcmp(nameobject,"Curve") == 0 )
      || ( strcmp(nameobject,"Polygon") == 0 )
      || ( strcmp(nameobject,"FilledPolygon") == 0 ) ) {
/*
 		#[ Curve,Polygons :
*/
        if ( ( argbuf = ReadArray(t,&num1,&num2) ) == 0 ) return(-1);
        if ( num2-1 != 0 ) {
            fprintf(stderr,"%s: Command %s should have no extra numbers in %s.\n",
                    axohelp,nameobject,inname);
            free(argbuf);
            return(-1);
        }
        else {
/*
            First some 'fixed' operations to set the state right
            1: scale      2: linewidth      3: color
            Then the function.
            Finally the trailer and cleanup.
*/
            axolinewidth = argbuf[2*num1+num2-1];
            SetLineWidth(axolinewidth);
            if ( strcmp(nameobject,"Curve") == 0 ) {
                Curve(argbuf,num1);
            }
            else if ( strcmp(nameobject,"Polygon") == 0 ) {
                Polygon(argbuf,num1,0);
            }
            else if ( strcmp(nameobject,"FilledPolygon") == 0 ) {
                Polygon(argbuf,num1,1);
            }
            free(argbuf);
        }
/*
 		#] Curve,Polygons : 
*/
    }
    else if ( strcmp(nameobject,"DashCurve") == 0 ) {
/*
 		#[ DashCurve :
*/
        if ( ( argbuf = ReadArray(t,&num1,&num2) ) == 0 ) return(-1);
        if ( num2 != 2 ) {
            fprintf(stderr,"%s: Command %s does not have two numbers after the coordinates\n  in file %s.\n",
                    axohelp,nameobject,inname);
            free(argbuf);
            return(-1);
        }
        else {
/*
            First some 'fixed' operations to set the state right
            1: scale      2: linewidth      3: color
            Then the function.
            Finally the trailer and cleanup.
*/
            axolinewidth = argbuf[2*num1+num2-1];
            SetLineWidth(axolinewidth);
            DashCurve(argbuf,num1);
            free(argbuf);
        }
/*
 		#] DashCurve : 
*/
    }
    else {
/*
 		#[ Regular command :
*/
        if ( ( argbuf = ReadTail(t,&num) ) == 0 ) return(-1);
        for ( i = 0; i < sizeof(commands)/sizeof(KEYWORD); i++ ) {
            if ( strcmp(nameobject,commands[i].name) == 0 ) {
                if ( num == commands[i].numargs+1 ) {
/*
                    First some 'fixed' operations to set the state right
                    1: scale      2: linewidth      3: color
                    Then the function.
                    Finally the trailer and cleanup.
*/
                    axolinewidth = argbuf[num-1];
                    SetLineWidth(axolinewidth);
                    (*(commands[i].func))(argbuf);
                    free(argbuf);
                    break;
                }
                else {
                    fprintf(stderr,"%s: Command %s should have %d(+1) arguments in %s.\n"
                        ,axohelp,nameobject,commands[i].numargs,inname);
                    free(argbuf);
                    return(-1); 
                }
            }
        }
/*
 		#] Regular command : 
*/
        if ( i >= sizeof(commands)/sizeof(KEYWORD) ) {
            fprintf(stderr,"%s: Command %s not recognized in file %s.\n",
                    axohelp,nameobject,inname);
            free(argbuf);
            return(-1); 
        }
    }
    outpos += sprintf(outpos,"}\n");
    CleanupOutput(StartClean);
    fprintf(outfile,"%s",outputbuffer);
    return(0);
}

/*
  	#] DoOneObject : 
  	#[ PrintHelp :
*/

void PrintHelp(char *name)
{
    fprintf(stderr,"This is %s v. %d.%d of %s\n", NAME, VERSION, SUBVERSION, VERSIONDATE);
    fprintf(stderr,"Proper use is: %s [-h] [-v] filename\n",name);
    fprintf(stderr,"Input will then be from filename.ax1, output to filename.ax2\n");
    fprintf(stderr,"-h : prints this help information and terminates.\n");
    fprintf(stderr,"-v : prints information about each function treated in stdout.\n");
    exit(-1);
}

/*
  	#] PrintHelp : 
  	#[ Inivars :
*/

void Inivars()
{
    M_pi = acos(-1.);
    torad = M_pi/180.;
    BzK = 4.*(sqrt(2.)-1.)/3.;
}

/*
  	#] Inivars : 
  	#[ main :
*/

int main(int argc,char **argv)
{
    char *s, *inbuffer;
    int length, error = 0;
    long num,i;

    Inivars();
    argc--;
    axohelp = *argv++;
    if ( argc <= 0 ) PrintHelp(axohelp);
    s = *argv;
    while ( *s == '-' ) {   /* we have arguments */
        if ( s[1] == 'h' && s[2] == 0 ) PrintHelp(axohelp);
        else if ( s[1] == 'v' && s[2] == 0 ) {
            VerboseFlag = 1;
        }
        else {
            fprintf(stderr,"Illegal option %s in call to %s\n",s,axohelp);
            PrintHelp(axohelp);
        }
        argc--; argv++;
        if ( argc <= 0 ) {
            fprintf(stderr,"Not enough arguments in call to %s\n",axohelp);
            PrintHelp(axohelp);
        }
        s = *argv;
    }
    if ( argc != 1 ) {
        fprintf(stderr,"Too many arguments in call to %s\n",axohelp);
        PrintHelp(axohelp);
    }
/*
    The filename is now in s. We should copy it to a separate string and
    paste on the extension .ax1 (if needed). We should also construct the
    name of the output file.
*/
    length = strlen(s);
    inname  = strcpy(malloc((length+5)*sizeof(char)),s);
    outname = strcpy(malloc((length+5)*sizeof(char)),s);
    s = inname + length;
    if ( length > 4 && s[-4] == '.' && s[-3] == 'a' && s[-2] == 'x' && s[-1] == '1' ) {
        outname[length-1] = '2';
    }
    else {
        inname[length] = '.'; inname[length+1] = 'a';
        inname[length+2] = 'x'; inname[length+3] = '1'; inname[length+4] = 0;
        outname[length] = '.'; outname[length+1] = 'a';
        outname[length+2] = 'x'; outname[length+3] = '2'; outname[length+4] = 0;
    }
    if ( ( inbuffer = ReadInput(inname) ) == 0 ) return(-1);
    if ( ( outfile = fopen(outname,"w") ) == 0 ) {
        fprintf(stderr,"%s: Cannot create file %s\n",axohelp,outname);
        exit(-1);
    }
    num = ScanForObjects(inbuffer);
    for ( i = 0; i < num; i++ ) {
        numobject = i+1;
        if ( DoOneObject(inputs[i]) < 0 ) { error++; }
    }
    fclose(outfile);
    if ( error > 0 ) {
        fprintf(stderr,"%s: %d objects in %s were not translated correctly.\n",
            axohelp,error,inname);
        return(-1);
    }
    return(0);
}

/*
  	#] main : 
  	#[ routines :
 		#[ Line routines :
			#[ Line : *

            Line(x1,y1)(x2,y2)
*/

void Line(double *args)
{
    MoveTo(args[0],args[1]);
    LineTo(args[2],args[3]);
    Stroke;
}

/*
			#] Line : 
			#[ DoubleLine : *

            DoubleLine(x1,y1)(x2,y2){sep}
*/

void DoubleLine(double *args)
{
    SaveGraphicsState;
    if ( args[4] > 0 ) {
        SetLineWidth(args[4]+axolinewidth);
        Line(args);
        SetLineWidth(args[4]-axolinewidth);
        SetBackgroundColor(STROKING);
    }
    Line(args);
    RestoreGraphicsState;
}

/*
			#] DoubleLine : 
			#[ DashLine : *

            DashLine(x1,y1)(x2,y2){dashsize}
*/

void DashLine(double *args)
{
    double dx = args[2]-args[0], dy = args[3]-args[1], dr = sqrt(dx*dx+dy*dy);
    double dashsize = args[4];
    int num;
    num = dr/dashsize;
    if ( ( num%2 ) == 1 ) num++;
    dashsize = dr/num;
    SetDashSize(dashsize,dashsize/2);
    Line(args);
}

/*
			#] DashLine : 
			#[ DashDoubleLine : *

            DashDoubleLine(x1,y1)(x2,y2){sep}{dashsize}
*/

void DashDoubleLine(double *args)
{
    double dx = args[2]-args[0], dy = args[3]-args[1], dr = sqrt(dx*dx+dy*dy);
    double dashsize = args[5];
    int num;
    num = dr/dashsize;
    if ( ( num%2 ) == 1 ) num++;
    dashsize = dr/num;
    SetDashSize(dashsize,dashsize/2);
    DoubleLine(args);
}

/*
			#] DashDoubleLine : 
			#[ ArrowLine : *

            ArrowLine(x1,y1)(x2,y2){size}{where}

            where: x of arrowhead is x1+where*(x2-x1). Same for y.
            serves both ArrowLine and LongArrowLine
*/

void ArrowLine(double *args)
{
    double dx, dy, dr, where;
    Line(args);
    if ( arrow.type == 0 ) where = args[5];
    else where = arrow.where;
    dx = args[2]-args[0];
    dy = args[3]-args[1];
    dr = sqrt(dx*dx+dy*dy);
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);
    SetTransferMatrix(1,0,0,1,dr*where,0);
    if ( arrow.type == 0 ) arrow.width = args[4];
    ArrowHead();
}

/*
			#] ArrowLine : 
			#[ ArrowDoubleLine : *

            ArrowDoubleLine(x1,y1)(x2,y2){sep}{size}{where}

            where: x of arrowhead is x1+where*(x2-x1). Same for y.
            serves both ArrowLine and LongArrowLine
*/

void ArrowDoubleLine(double *args)
{
    double dx, dy, dr, where;
    DoubleLine(args);
    if ( arrow.type == 0 ) where = args[6];
    else where = arrow.where;
    dx = args[2]-args[0];
    dy = args[3]-args[1];
    dr = sqrt(dx*dx+dy*dy);
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);
    SetTransferMatrix(1,0,0,1,dr*where,0);
    if ( arrow.type == 0 ) arrow.width = args[5]+args[4];
    ArrowHead();
}

/*
			#] ArrowDoubleLine : 
			#[ DashArrowLine : *

        DashArrowLine(x1,y1)(x2,y2){dashsize}{amplitude}{where}

        where: x of arrowhead is x1+where*(x2-x1). Same for y.
        we re-adjust the position of the arrow to place it on a dash.
*/

void DashArrowLine(double *args)
{
    double dx = args[2]-args[0], dy = args[3]-args[1], dr = sqrt(dx*dx+dy*dy);
    double dashsize = args[4], where;
    int num, nw;
    if ( arrow.type == 0 ) where = args[6];
    else where = arrow.where;
    num = dr/dashsize;
    if ( ( num%2 ) == 1 ) num++;
    if ( num%4 != 0 && where > 0.499 && where < 0.501 ) num += 2;
    dashsize = dr/num;
    SetDashSize(dashsize,dashsize/2);
    Line(args);
    nw = where*(num/2)+0.5;
    where = (2.0*nw)/num;
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);
    SetTransferMatrix(1,0,0,1,dr*where,0);
    if ( arrow.type == 0 ) arrow.width = args[5];
    ArrowHead();
}

/*
			#] DashArrowLine : 
			#[ DashArrowDoubleLine : *

        DashArrowDoubleLine(x1,y1)(x2,y2){sep}{dashsize}{amplitude}{where}

        where: x of arrowhead is x1+where*(x2-x1). Same for y.
        we re-adjust the position of the arrow to place it on a dash.
*/

void DashArrowDoubleLine(double *args)
{
    double dx = args[2]-args[0], dy = args[3]-args[1], dr = sqrt(dx*dx+dy*dy);
    double dashsize = args[5], where;
    int num, nw;
    if ( arrow.type == 0 ) where = args[7];
    else where = arrow.where;
    num = dr/dashsize;
    if ( ( num%2 ) == 1 ) num++;
    if ( num%4 != 0 && where > 0.499 && where < 0.501 ) num += 2;
    dashsize = dr/num;
    SetDashSize(dashsize,dashsize/2);
    DoubleLine(args);
    nw = where*(num/2)+0.5;
    where = (2.0*nw)/num;
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);
    SetTransferMatrix(1,0,0,1,dr*where,0);
    if ( arrow.type == 0 ) arrow.width = args[6]+args[4];
    ArrowHead();
}

/*
			#] DashArrowDoubleLine : 
 		#] Line routines : 
 		#[ Arc routines :
			#[ CArc : *

        CArc(x1,y1)(r,phi1,phi2)

        The arc segment runs anticlockwise

        We divide the segment into a number of equal segments, each less
        than 90 degrees. Then the control points are at distance
        4*tan(90/n)/3 from the endpoints, in which n=360/(phi2-phi1)
        (note that if n=4 we get tan(22.5)=sqrt(2)-1).
*/

void CArc(double *args)
{
    double phi1 = args[3], phi2 = args[4], r = args[2];
    double dphi;
    while ( phi2 < phi1 ) phi2 += 360;
    dphi = phi2-phi1;
    if ( dphi <= 0 ) { return; }
    if ( dphi >= 360 ) { ECirc(args); return; }
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    ArcSegment(r,phi1,dphi);
    Stroke;
}

/*
			#] CArc : 
			#[ DoubleArc : *

        DoubleArc(x1,y1)(r,phi1,phi2){sep}

        The arc segment runs anticlockwise

        We divide the segment into a number of equal segments, each less
        than 90 degrees. Then the control points are at distance
        4*tan(90/n)/3 from the endpoints, in which n=360/(phi2-phi1)
        (note that if n=4 we get tan(22.5)=sqrt(2)-1).
*/

void DoubleArc(double *args)
{
    double phi1 = args[3], phi2 = args[4], r = args[2];
    double dphi, sep = args[5];
    while ( phi2 < phi1 ) phi2 += 360;
    dphi = phi2-phi1;
    if ( dphi <= 0 ) { return; }
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    ArcSegment(r+sep/2,phi1,dphi);
    Stroke;
    ArcSegment(r-sep/2,phi1,dphi);
    Stroke;
}

/*
			#] DoubleArc : 
			#[ DashCArc : *

        DashCArc(x1,y1)(r,phi1,phi2){dashsize}

        The arc segment runs anticlockwise
*/

void DashCArc(double *args)
{
    double arcsize = args[4]-args[3];
    double r = args[2], dr, dashsize = args[5];
    int num;
    if ( arcsize <= 0 ) arcsize += 360;
    if ( arcsize > 360 ) arcsize = 360;
    dr = 2*M_pi*r*(arcsize/360);
    num = dr/dashsize;
    if ( (num%2) == 1 ) num++;
    dashsize = dr/num;
    SetDashSize(dashsize,dashsize/2);
    CArc(args);
}

/*
			#] DashCArc : 
			#[ DashDoubleArc : *

        DashDoubleArc(x1,y1)(r,phi1,phi2){sep}{dashsize}

        The arc segment runs anticlockwise
        The trouble here is to synchronize the two dash patterns.
        This is done by a rescaling. We assume that the rescaling is
        sufficiently small that the linewidth does not suffer from it.
*/

void DashDoubleArc(double *args)
{
    double phi1 = args[3], r = args[2];
    double arcsize = args[4]-args[3];
    double dr, dashsize = args[6], sep = args[5];
    int num;
    linesep = sep;
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    if ( arcsize <= 0 ) arcsize += 360;
    if ( arcsize > 360 ) arcsize = 360;
    dr = 2*M_pi*r*(arcsize/360);
    num = dr/dashsize;
    if ( (num%2) == 1 ) num++;
    dashsize = dr/num;
    SetDashSize(dashsize,dashsize/2);
    SaveGraphicsState;
    SetTransferMatrix(1+0.5*sep/r,0,0,1+0.5*sep/r,0,0);
    ArcSegment(r,phi1,arcsize);
    Stroke;
    RestoreGraphicsState;
    SaveGraphicsState;
    SetTransferMatrix(1-0.5*sep/r,0,0,1-0.5*sep/r,0,0);
    ArcSegment(r,phi1,arcsize);
    Stroke;
    RestoreGraphicsState;
}

/*
			#] DashDoubleArc : 
			#[ ArrowArc : *

        ArrowArc(x1,y1)(r,phi1,phi2){amplitude}{where}

        where: phi of arrowhead is phi1+where*(phi2-phi1)
        The arc segment runs anticlockwise
        serves both ArrowArc and LongArrowArc and ... (Jaxodraw addition)
*/

void ArrowArc(double *args)
{
    double phi1 = args[3], phi2 = args[4], r = args[2];
    double dphi, x, y, phi;
    if ( arrow.type == 0 ) {
        arrow.width = args[5];
        arrow.where = args[6];
    }

    while ( phi2 < phi1 ) phi2 += 360;
    dphi = phi2-phi1;
    SetTransferMatrix(1,0,0,1,args[0],args[1]);

    if ( dphi <= 0 ) { return; }
    ArcSegment(r,phi1,dphi);
    Stroke;
/*
    Now compute the position and angle of the arrowhead
*/
    phi = phi1 + arrow.where*dphi;
    x = r*COS(phi); y = r*SIN(phi);
    SetTransferMatrix(1,0,0,1,x,y);
    SetTransferMatrix(COS(phi+90),SIN(phi+90),-SIN(phi+90),COS(phi+90),0,0);
    ArrowHead();
}

/*
			#] ArrowArc : 
			#[ ArrowDoubleArc : *

        ArrowDoubleArc(x1,y1)(r,phi1,phi2){sep}{amplitude}{where}

        where: phi of arrowhead is phi1+where*(phi2-phi1)
        The arc segment runs anticlockwise
        serves both ArrowArc and LongArrowArc and ... (Jaxodraw addition)
*/

void ArrowDoubleArc(double *args)
{
    double phi1 = args[3], phi2 = args[4], r = args[2];
    double dphi, x, y, phi;
    linesep = args[5];
    while ( phi2 < phi1 ) phi2 += 360;
    dphi = phi2-phi1;
    if ( dphi <= 0 ) { return; }
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    ArcSegment(r+linesep/2,phi1,dphi);
    Stroke;
    ArcSegment(r-linesep/2,phi1,dphi);
    Stroke;
/*
    Now compute the position and angle of the arrowhead
*/
    if ( arrow.type == 0 ) {
        arrow.width = args[6];
        arrow.where = args[7];
    }
    phi = phi1 + arrow.where*dphi;
    x = r*COS(phi); y = r*SIN(phi);
    SetTransferMatrix(1,0,0,1,x,y);
    SetTransferMatrix(COS(phi+90),SIN(phi+90),-SIN(phi+90),COS(phi+90),0,0);
    ArrowHead();
}

/*
			#] ArrowDoubleArc : 
			#[ DashArrowArc : +

        DashArrowArc(x1,y1)(r,phi1,phi2){dashsize}{amplitude}{where}

        where: phi of arrowhead is phi1+where*(phi2-phi1)
        The arc segment runs anticlockwise
*/

void DashArrowArc(double *args)
{
    double dphi, x, y, phi, phi1 = args[3], phi2 = args[4];
    double r = args[2], dr, dashsize = args[5];
    int num;
    if ( arrow.type == 0 ) {
        arrow.width = args[6];
        arrow.where = args[7];
    }

    while ( phi2 < phi1 ) phi2 += 360;
    dphi = phi2-phi1;
    if ( dphi > 360 ) dphi = 360;
    dr = 2*M_pi*r*(dphi/360);
    num = dr/dashsize;
    if ( (num%2) == 1 ) num++;
    if ( num%4 != 0 && arrow.where > 0.499 && arrow.where < 0.501 ) num += 2;
    dashsize = dr/num;
    SetDashSize(dashsize,dashsize/2);
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    if ( dphi <= 0 ) { return; }
    ArcSegment(r,phi1,dphi);
    Stroke;
/*
    Now compute the position and angle of the arrowhead
*/
    phi = phi1 + arrow.where*dphi;
    x = r*COS(phi); y = r*SIN(phi);
    SetTransferMatrix(1,0,0,1,x,y);
    SetTransferMatrix(COS(phi+90),SIN(phi+90),-SIN(phi+90),COS(phi+90),0,0);
    ArrowHead();
}

/*
			#] DashArrowArc : 
			#[ DashArrowDoubleArc : +

        DashArrowDoubleArc(x1,y1)(r,phi1,phi2){sep}{dashsize}{amplitude}{where}

        where: phi of arrowhead is phi1+where*(phi2-phi1)
        The arc segment runs anticlockwise
*/

void DashArrowDoubleArc(double *args)
{
    double dphi, x, y, phi, phi1 = args[3], phi2 = args[4];
    double r = args[2], dr, dashsize = args[6];
    int num;
    linesep = args[5];
    if ( arrow.type == 0 ) {
        arrow.width = args[7];
        arrow.where = args[8];
    }

    while ( phi2 < phi1 ) phi2 += 360;
    dphi = phi2-phi1;
    if ( dphi > 360 ) dphi = 360;
    dr = 2*M_pi*r*(dphi/360);
    num = dr/dashsize;
    if ( (num%2) == 1 ) num++;
    if ( num%4 != 0 && arrow.where > 0.499 && arrow.where < 0.501 ) num += 2;
    dashsize = dr/num;
    SetDashSize(dashsize,dashsize/2);
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    if ( dphi <= 0 ) { return; }
    SaveGraphicsState;
    SetTransferMatrix(1+0.5*linesep/r,0,0,1+0.5*linesep/r,0,0);
    ArcSegment(r,phi1,dphi);
    Stroke;
    RestoreGraphicsState;
    SaveGraphicsState;
    SetTransferMatrix(1-0.5*linesep/r,0,0,1-0.5*linesep/r,0,0);
    ArcSegment(r,phi1,dphi);
    Stroke;
    RestoreGraphicsState;
/*
    Now compute the position and angle of the arrowhead
*/
    phi = phi1 + arrow.where*dphi;
    x = r*COS(phi); y = r*SIN(phi);
    SetTransferMatrix(1,0,0,1,x,y);
    SetTransferMatrix(COS(phi+90),SIN(phi+90),-SIN(phi+90),COS(phi+90),0,0);
    ArrowHead();
}

/*
			#] DashArrowDoubleArc : 
			#[ ArrowArcn : +

        ArrowArcn(x1,y1)(r,phi1,phi2){amplitude}{where}

        where: phi of arrowhead is phi1+where*(phi2-phi1)
        The arc segment runs clockwise
*/

void ArrowArcn(double *args)
{
    double newargs[7], a;
    int i;
    for ( i = 0; i < 7; i++ ) newargs[i] = args[i];
    newargs[6] = 1-newargs[6];
    a = newargs[3]; newargs[3] = newargs[4]; newargs[4] = a;
    ArrowArc(newargs);
}

/*
			#] ArrowArcn : 
			#[ DashArrowArcn : +

        DashArrowArc(x1,y1)(r,phi1,phi2){amplitude}{where}{dashsize}

        where: phi of arrowhead is phi1+where*(phi2-phi1)
        The arc segment runs clockwise
*/

void DashArrowArcn(double *args)
{
    double newargs[8], a;
    int i;
    for ( i = 0; i < 8; i++ ) newargs[i] = args[i];
    newargs[6] = 1-newargs[6];
    a = newargs[3]; newargs[3] = newargs[4]; newargs[4] = a;
    DashArrowArc(newargs);
}

/*
			#] DashArrowArcn : 
 		#] Arc routines : 
 		#[ Circle routines :
			#[ ECirc : +

        ECirc(x,y){radius}
        Draws a circle
*/

void ECirc(double *args)
{
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    BezierCircle(args[2],"S");
}

/*
			#] ECirc : 
			#[ FOval : +

        FOval(x1,y1)(h,w) filled oval in default color

*/

void FOval(double *args)
{
    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of oval */
    SetTransferMatrix(COS(args[4]),SIN(args[4]),-SIN(args[4]),COS(args[4]),0,0);
    BezierOval(args[3],args[2],"f");
    BezierOval(args[3],args[2],"S");
}

/*
			#] COval : 
			#[ Oval : +

        Oval(x1,y1)(h,w)(phi)

        One way would be with different scales in the x and y direction,
        but that messes up the linewidth.
        We need to describe quarter ovals with Bezier curves. The proper
        parameters for the curves we can obtain from the circle with scaling.
*/

void Oval(double *args)
{
    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of oval */
    SetTransferMatrix(COS(args[4]),SIN(args[4]),-SIN(args[4]),COS(args[4]),0,0);
    BezierOval(args[3],args[2],"S");
}

/*
			#] Oval : 
 		#] Circle routines : 
 		#[ Box routines :
			#[ EBox : *

        \EBox(#1,#2)(#3,#4)

        Draws a box with the left bottom at (x1,y1) and the right top
        at (x2,y2).
                Transparent interior. Current color for edge.
*/

void EBox(double *args)
{
    Rectangle(args[0],args[1],args[2]-args[0],args[3]-args[1]);
    Stroke;
}

/*
			#] EBox : 
			#[ FBox : *

        \FBox(#1,#2)(#3,#4)

        Draws a filled box with the left bottom at (x1,y1) and 
                the right top at (x2,y2).
                Current color.
*/

void FBox(double *args)
{
    Rectangle(args[0],args[1],args[2]-args[0],args[3]-args[1]);
    Fill;
}

/*
			#] FBox : 
			#[ Boxc : *

        \Boxc(#1,#2)(#3,#4)

    Draws a transparent box with the center at (x1,y1).
    The width and height are (3,4).  Uses current color.
*/

void Boxc(double *args)
{
    Rectangle(args[0]-args[2]/2,args[1]-args[3]/2,args[2],args[3]);
    Stroke;
}

/*
			#] Boxc : 
			#[ FBoxc : *

        \FBoxc(#1,#2)(#3,#4)

    Draws a filled box with the center at (x1,y1).
    The width and height are (3,4). Uses current color.
*/

void FBoxc(double *args)
{
    Rectangle(args[0]-args[2]/2,args[1]-args[3]/2,args[2],args[3]);
    Fill;
}

/*
			#] FBoxc : 
 		#] Box routines : 
 		#[ Triangle routines :
			#[ ETri : *

        \ETri(#1,#2)(#3,#4)(#5,#6)

        Draws a triangle with the three corners.
*/

void ETri(double *args)
{
    Triangle(args[0],args[1],args[2],args[3],args[4],args[5]);
    Stroke;
}

/*
			#] ETri : 
			#[ FTri : *

        \FTri(#1,#2)(#3,#4)(#5,#6)

        Draws a triangle with the three corners.
*/

void FTri(double *args)
{
    Triangle(args[0],args[1],args[2],args[3],args[4],args[5]);
    Fill;
}

/*
			#] FTri : 
 		#] Triangle routines : 
 		#[ Particle routines :
			#[ Vertex : +

        Vertex(x,y){radius}
        Draws a filled circle
*/

void Vertex(double *args)
{
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    BezierCircle(args[2],"f");
}

/*
			#] Vertex : 
			#[ ComputeDash :
*/

double ComputeDash(double *args,double dr,double indash)
{
    int numwindings = args[5]+0.5, numdashes;
    int numhalfwindings = 2*numwindings+2.1;
    double onehalfwinding = dr/numhalfwindings; 
    double amp8 = fabs(args[4])*0.9;
    double size = LengthBezier(-amp8,0, -amp8,2*args[4], onehalfwinding,2*args[4],1.0);

    numdashes = size/(2*indash);
    if ( numdashes == 0 ) numdashes = 1;
    if ( fabs(size-2*indash*numdashes) > fabs(size-2*indash*(numdashes+1)) )
        numdashes++;
    return(size/(2*numdashes));
}

/*
			#] ComputeDash : 
			#[ ComputeDashCirc :
*/

double ComputeDashCirc(double *args,double indash)
{
    int num = args[5]+0.5, numdashes;
    double ampi = args[4], radius = args[2];
    double darc;
    double dr,conv,inc;
    double amp1,amp2,amp4,amp5,amp8;
    double x0,x1,x2,x3,y0,y1,y2,y3,xx,size;
    darc = 360.;
    dr = darc*torad*radius;
    conv = 1.0/radius;
    inc = dr/(2*num);      /* increment per half winding */
    amp8 = ampi*0.9;
    amp1 = radius+ampi;
    amp2 = radius-ampi;
    amp4 = amp1/cos((inc+amp8)*conv);
    amp5 = amp2/cos(amp8*conv);
    if ( amp8 < 0 ) amp8 = -amp8;
    xx = 2*inc;
    x0 = amp1*cos(inc*conv);
    y0 = amp1*sin(inc*conv);
    x1 = amp4*cos((xx+amp8)*conv)-x0;
    y1 = amp4*sin((xx+amp8)*conv)-y0;
    x2 = amp5*cos((xx+amp8)*conv)-x0;
    y2 = amp5*sin((xx+amp8)*conv)-y0;
    x3 = amp2*cos(xx*conv)-x0;
    y3 = amp2*sin(xx*conv)-y0;
    size = LengthBezier(x1,y1,x2,y2,x3,y3,1.0);
    numdashes = size/(2*indash);
    if ( numdashes == 0 ) numdashes = 1;
    if ( fabs(size-2*indash*numdashes) > fabs(size-2*indash*(numdashes+1)) )
        numdashes++;
    return(size/(2*numdashes));
}

/*
			#] ComputeDashCirc : 
			#[ ComputeDashPhotonArc :
*/

double ComputeDashPhotonArc(double *args,double darc,double dashsize,double *dashstart)
{
    int numdashes, numd;
    double len1, len2, size, size2, ampli = args[5], radius = args[2];
    double cp = cos(darc);
    double sp = sin(darc);
    double cp2 = cos(darc/2.);
    double sp2 = sin(darc/2.);
    double beta = radius*darc/(M_pi*ampli);
    double tt = (sp-cp*beta)/(cp+sp*beta);
    double x2 = ((radius+ampli)*8*(beta*cp2-sp2)-(beta*(4+cp)
        +(tt*cp*3.-sp*4.))*radius)/((beta-tt)*3.);
    double x1 = ((radius+ampli)*8.*cp2-(1+cp)*radius)/3.-x2;
    double y1 = (x1-radius)*beta;
    double y2 = (x2-radius*cp)*tt+radius*sp;
    double x3 = radius*cp;
    double y3 = radius*sp;
    len1 = LengthBezier(x1-radius,y1,x2-radius,y2,x3-radius,y3,1.0);
    ampli = -ampli;
    beta = radius*darc/(M_pi*ampli);
    tt = (sp-cp*beta)/(cp+sp*beta);
    x2 = ((radius+ampli)*8*(beta*cp2-sp2)-(beta*(4+cp)
        +(tt*cp*3.-sp*4.))*radius)/((beta-tt)*3.);
    x1 = ((radius+ampli)*8.*cp2-(1+cp)*radius)/3.-x2;
    y1 = (x1-radius)*beta;
    y2 = (x2-radius*cp)*tt+radius*sp;
    x3 = radius*cp;
    y3 = radius*sp;
    len2 = LengthBezier(x1-radius,y1,x2-radius,y2,x3-radius,y3,1.0);
    size = (len1+len2)/2;
    size2 = len1/2;
    numdashes = size/(2*dashsize);
    if ( numdashes == 0 ) numdashes = 1;
    if ( fabs(size-2*dashsize*numdashes) > fabs(size-2*dashsize*(numdashes+1)) )
        numdashes++;
    dashsize = size/(2*numdashes);
    numd = size2/(2*dashsize);
    *dashstart = -(size2 - 2*numd*dashsize) + dashsize/2;
    if ( *dashstart < 0 ) *dashstart += 2*dashsize;
    return(dashsize);
}

/*
			#] ComputeDashPhotonArc : 
			#[ ComputeDashGluonArc :
*/

double ComputeDashGluonArc(double *args,double darc,double dashsize)
{
    int numdashes, num = args[6];
    double radius = args[2], ampi = args[5];
    double dr,conv,inc, size;
    double amp1,amp2,amp4,amp5,amp8;
    double x1,x2,x3,y1,y2,y3,xx,x3p,y3p;
    dr = darc*torad*radius;
    conv = 1.0/radius;
    inc = dr/(2*num+2);      /* increment per half winding */
    amp8 = ampi*0.9;
    amp1 = radius+ampi;
    amp2 = radius-ampi;
    amp4 = amp1/cos((inc+amp8)*conv);
    amp5 = amp2/cos(amp8*conv);
    if ( amp8 < 0 ) amp8 = -amp8;

    xx = 2*inc;
    x3p = amp2*cos(xx*conv);
    y3p = amp2*sin(xx*conv);
    x1 = amp5*cos((xx-amp8)*conv)-x3p;
    y1 = amp5*sin((xx-amp8)*conv)-y3p;
    x2 = amp4*cos((xx-amp8)*conv)-x3p;
    y2 = amp4*sin((xx-amp8)*conv)-y3p;
    x3 = amp1*cos((xx+inc)*conv)-x3p;
    y3 = amp1*sin((xx+inc)*conv)-y3p;
    size = LengthBezier(x1,y1,x2,y2,x3,y3,1.0);

    numdashes = size/(2*dashsize);
    if ( numdashes == 0 ) numdashes = 1;
    if ( fabs(size-2*dashsize*numdashes) > fabs(size-2*dashsize*(numdashes+1)) )
        numdashes++;
    dashsize = size/(2*numdashes);
    return(dashsize);
}

/*
			#] ComputeDashGluonArc : 
			#[ GluonHelp :

        We draw the gluon in two strokes. This is due to the possibility
        of a dash pattern. We want the dashes to be nicely symmetric on
        the central windings. That means that either the start and end need
        a different size dashes, or we draw the start 'backwards'.
        We have chosen for the last solution.
*/

void GluonHelp(double *args,double dr)
{
    int numwindings = args[5]+0.5;
    int numhalfwindings = 2*numwindings+2.1;
    double onehalfwinding = dr/numhalfwindings; 
    double amp8 = fabs(args[4])*0.9;
    double xx = 2*onehalfwinding;
    int i;
    MoveTo(xx,-args[4]);
    Bezier(xx+amp8,-args[4],xx+amp8,args[4],1.4*onehalfwinding,args[4]);
    Bezier(0.5*onehalfwinding,args[4],
           0.1*onehalfwinding,args[4]*0.5,0,0);
    Stroke;
    MoveTo(xx,-args[4]);
    for ( i = 0; i < numwindings-1; i++ ) {
        Bezier(xx-amp8,-args[4], xx-amp8,args[4], xx+onehalfwinding,args[4]);
        xx += 2*onehalfwinding;
        Bezier(xx+amp8,args[4], xx+amp8,-args[4], xx,-args[4]);
    }
    Bezier(xx-amp8,-args[4], xx-amp8,args[4], xx+onehalfwinding*0.6,args[4]);
    Bezier(dr-onehalfwinding*0.5,args[4],
           dr-onehalfwinding*0.1,args[4]*0.5,
           dr,0);
    Stroke;
}

/*
			#] GluonHelp : 
			#[ GluonCircHelp :
*/

void GluonCircHelp(double *args)
{
    int num = args[5], i;
    double ampi = args[4], radius = args[2];
    double darc;
    double dr,conv,inc;
    double amp1,amp2,amp4,amp5,amp8;
    double x1,x2,x3,y1,y2,y3,xx;
    darc = 360.;
    dr = darc*torad*radius;
    conv = 1.0/radius;
    inc = dr/(2*num);      /* increment per half winding */
    amp8 = ampi*0.9;
    amp1 = radius+ampi;
    amp2 = radius-ampi;
    amp4 = amp1/cos((inc+amp8)*conv);
    amp5 = amp2/cos(amp8*conv);
    if ( amp8 < 0 ) amp8 = -amp8;
    xx = 2*inc;
    x3 = amp1*cos(inc*conv);
    y3 = amp1*sin(inc*conv);
    MoveTo(x3,y3);
/*
    Now the loop
*/
    for ( i = 0; i < num; i++ ) {
        x1 = amp4*cos((xx+amp8)*conv);
        y1 = amp4*sin((xx+amp8)*conv);
        x2 = amp5*cos((xx+amp8)*conv);
        y2 = amp5*sin((xx+amp8)*conv);
        x3 = amp2*cos(xx*conv);
        y3 = amp2*sin(xx*conv);
            Bezier(x1,y1,x2,y2,x3,y3);
        x1 = amp5*cos((xx-amp8)*conv);
        y1 = amp5*sin((xx-amp8)*conv);
        x2 = amp4*cos((xx-amp8)*conv);
        y2 = amp4*sin((xx-amp8)*conv);
        x3 = amp1*cos((xx+inc)*conv);
        y3 = amp1*sin((xx+inc)*conv);
            Bezier(x1,y1,x2,y2,x3,y3);
        xx += 2*inc;
    }
    Stroke;
}

/*
			#] GluonCircHelp : 
			#[ GluonArcHelp :
*/

void GluonArcHelp(double *args, double darc, double ampi)
{
    int num = args[6], i;
    double radius = args[2];
    double dr,conv,inc;
    double amp1,amp2,amp3,amp4,amp5,amp6,amp7,amp8;
    double x1,x2,x3,y1,y2,y3,xx,x1p,y1p,x2p,y2p,x3p,y3p;
    dr = darc*torad*radius;
    conv = 1.0/radius;
    inc = dr/(2*num+2);      /* increment per half winding */
    amp8 = ampi*0.9;
    amp1 = radius+ampi;
    amp2 = radius-ampi;
    amp3 = radius+ampi/2;
    amp4 = amp1/cos((inc+amp8)*conv);
    amp5 = amp2/cos(amp8*conv);
    amp6 = amp1/cos((inc*0.6+amp8)*conv);
    amp7 = amp1/cos(inc*0.9*conv);
    if ( amp8 < 0 ) amp8 = -amp8;
    xx = 2*inc;
/*
    First the starting part. We draw it separately because there could
    be a dashing pattern. This way the windings come out best.
*/
    x1 = amp3*cos(inc*0.1*conv);
    y1 = amp3*sin(inc*0.1*conv);
    x2 = amp7*cos(inc*0.5*conv);
    y2 = amp7*sin(inc*0.5*conv);
    x3 = amp1*cos(inc*1.4*conv);
    y3 = amp1*sin(inc*1.4*conv);
    x1p = amp6*cos((xx+amp8)*conv);
    y1p = amp6*sin((xx+amp8)*conv);
    x2p = amp5*cos((xx+amp8)*conv);
    y2p = amp5*sin((xx+amp8)*conv);
    x3p = amp2*cos(xx*conv);
    y3p = amp2*sin(xx*conv);
    MoveTo(x3p,y3p);
    Bezier(x2p,y2p,x1p,y1p,x3,y3);
    Bezier(x2,y2,x1,y1,radius,0);
    Stroke;
/*
    Now the loop
*/
    MoveTo(x3p,y3p);
    for ( i = 1; i < num; i++ ) {
        x1 = amp5*cos((xx-amp8)*conv);
        y1 = amp5*sin((xx-amp8)*conv);
        x2 = amp4*cos((xx-amp8)*conv);
        y2 = amp4*sin((xx-amp8)*conv);
        x3 = amp1*cos((xx+inc)*conv);
        y3 = amp1*sin((xx+inc)*conv);
            Bezier(x1,y1,x2,y2,x3,y3);
        xx += 2*inc;
        x1 = amp4*cos((xx+amp8)*conv);
        y1 = amp4*sin((xx+amp8)*conv);
        x2 = amp5*cos((xx+amp8)*conv);
        y2 = amp5*sin((xx+amp8)*conv);
        x3 = amp2*cos(xx*conv);
        y3 = amp2*sin(xx*conv);
            Bezier(x1,y1,x2,y2,x3,y3);
    }
/*
    And now the end point
*/
    x1 = amp5*cos((xx-amp8)*conv);
    y1 = amp5*sin((xx-amp8)*conv);
    x2 = amp6*cos((xx-amp8)*conv);
    y2 = amp6*sin((xx-amp8)*conv);
    x3 = amp1*cos((xx+inc*0.6)*conv);
    y3 = amp1*sin((xx+inc*0.6)*conv);
        Bezier(x1,y1,x2,y2,x3,y3);
    x1 = amp7*cos((xx+inc*1.5)*conv);
    y1 = amp7*sin((xx+inc*1.5)*conv);
    x2 = amp3*cos((dr-inc*0.1)*conv);
    y2 = amp3*sin((dr-inc*0.1)*conv);
    x3 = radius*cos(dr*conv);
    y3 = radius*sin(dr*conv);

    Bezier(x1,y1,x2,y2,x3,y3);

    Stroke;
}

/*
			#] GluonArcHelp : 
			#[ PhotonHelp :
*/

void PhotonHelp(double *args, double dr)
{
    int numhalfwindings = args[5]*2+0.5;
    double onehalfwinding = dr/numhalfwindings; 
    double y = 4.*args[4]/3;
    double x, xx;
    int i;
    MoveTo(0,0);
/*
    Now loop over the half windings, alternating the sign of the y's
*/
    x = (4*onehalfwinding)/3/M_pi; xx = 0;
    for ( i = 0; i < numhalfwindings; i++, y = -y ) {
        Bezier(xx+x,y, xx+onehalfwinding-x,y, xx+onehalfwinding,0);
        xx += onehalfwinding;
    }
    Stroke;
}
/*
			#] PhotonHelp : 
			#[ PhotonArcHelp :
*/

void PhotonArcHelp(double *args,double arcend,int num)
{
    int i;
    double ampli = args[5], radius = args[2];
    double cp,sp,cp2,sp2,cpi,spi;
    double x1,x2,x3,y1,y2,y3,beta,tt;

    cp = cos(arcend);
    sp = sin(arcend);
    cp2 = cos(arcend/2.);
    sp2 = sin(arcend/2.);

    MoveTo(radius,0);
    for ( i = 0; i < num; i++, ampli = -ampli ) {
        cpi = cos(i*arcend);
        spi = sin(i*arcend);
        beta = radius*arcend/(M_pi*ampli);
        tt = (sp-cp*beta)/(cp+sp*beta);
        x2 = ((radius+ampli)*8*(beta*cp2-sp2)-(beta*(4+cp)
            +(tt*cp*3.-sp*4.))*radius)/((beta-tt)*3.);
        x1 = ((radius+ampli)*8.*cp2-(1+cp)*radius)/3.-x2;
        y1 = (x1-radius)*beta;
        y2 = (x2-radius*cp)*tt+radius*sp;
        x3 = radius*cp;
        y3 = radius*sp;
        Bezier(cpi*x1-spi*y1,cpi*y1+spi*x1,
               cpi*x2-spi*y2,cpi*y2+spi*x2,
               cpi*x3-spi*y3,cpi*y3+spi*x3);
    }
    Stroke;
}

/*
			#] PhotonArcHelp : 
			#[ ZigZagHelp :
*/

void ZigZagHelp(double *args, double dr)
{
    int numhalfwindings = args[5]*2+0.5;
    double onehalfwinding = dr/numhalfwindings; 
    double x = onehalfwinding, y = args[4];
    int i;
    MoveTo(0,0);
/*
    Now loop over the half windings, alternating the sign of the y's
*/
    for ( i = 0; i < numhalfwindings; i++, y = -y ) {
        LineTo(x-onehalfwinding/2.,y); LineTo(x,0);
        x += onehalfwinding;
    }
    Stroke;
}

/*
			#] ZigZagHelp : 
			#[ ZigZagArcHelp :
*/

void ZigZagArcHelp(double *args)
{
    int num = 2*args[6]-0.5, i;
    double amp = args[5], r = args[2];
    double arcstart = args[3], arcend = args[4], darc;
    if ( arcend < arcstart ) arcend += 360.;
    darc = (arcend-arcstart)/(num+1);
    MoveTo(r*COS(arcstart),r*SIN(arcstart));
    arcstart += darc/2;
    for ( i = 0; i <= num; i++, amp = -amp ) {
        LineTo((r+amp)*COS(arcstart+darc*i),(r+amp)*SIN(arcstart+darc*i));
    }
    LineTo(r*COS(arcend),r*SIN(arcend));
    Stroke;
}

/*
			#] ZigZagArcHelp : 
			#[ Gluon : *

        Gluon(x1,y1)(x2,y2){amplitude}{windings}

        Each half winding is one cubic Bezier curve.
        In addition the end points are different Bezier curves.
*/

void Gluon(double *args)
{
    double dx = args[2] - args[0];
    double dy = args[3] - args[1];
    double dr = sqrt(dx*dx+dy*dy);
 
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);
    GluonHelp(args,dr);
}

/*
			#] Gluon : 
			#[ DashGluon : *

        DashGluon(x1,y1)(x2,y2){amplitude}{windings}{dashsize}

        Each half winding is one cubic Bezier curve.
        In addition the end points are different Bezier curves.
*/

void DashGluon(double *args)
{
    double dx = args[2] - args[0];
    double dy = args[3] - args[1];
    double dr = sqrt(dx*dx+dy*dy);
    double dashsize;
 
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);

    dashsize = ComputeDash(args,dr,args[6]);
    SetDashSize(dashsize,dashsize/2);
    GluonHelp(args,dr);
}

/*
			#] DashGluon : 
			#[ GluonCirc : *

        GluonCirc(x1,y1)(r,phi){amplitude}{windings}

        Draws a gluon on a circle
        x_center,y_center,radius,phase_angle,gluon_radius,num
        in which num is the number of windings of the gluon.
        Method: Same as GluonArc, but without special start and end
*/

void GluonCirc(double *args)
{
    int num = args[5];
    double arcstart = args[3];

    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */

    arcstart += 360./(2*num);  /* extra phase to make 0 angle more accessible */

    SetTransferMatrix(COS(arcstart),SIN(arcstart)
                    ,-SIN(arcstart),COS(arcstart),0,0);

    GluonCircHelp(args);
}

/*
			#] GluonCirc : 
			#[ DashGluonCirc : *

        DashGluonCirc(x1,y1)(r,phi){amplitude}{windings}{dashsize}

        Draws a gluon on a circle
        x_center,y_center,radius,phase_angle,gluon_radius,num
        in which num is the number of windings of the gluon.
        Method: Same as GluonArc, but without special start and end
*/

void DashGluonCirc(double *args)
{
    int num = args[5];
    double arcstart = args[3], dashsize;

    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */

    arcstart += 360./(2*num);  /* extra phase to make 0 angle more accessible */

    SetTransferMatrix(COS(arcstart),SIN(arcstart)
                    ,-SIN(arcstart),COS(arcstart),0,0);

    dashsize = ComputeDashCirc(args,args[6]);

    SetDashSize(dashsize,dashsize/2);
    GluonCircHelp(args);
}

/*
			#] DashGluonCirc : 
			#[ GluonArc : *

        GluonArc(x1,y1)(r,phi1,phi2){amplitude}{windings}

        Draws a gluon on an arcsegment
        x_center,y_center,radius,stat_angle,end_angle,gluon_radius,num
        in which num is the number of windings of the gluon.
        Method:
        1:  compute length of arc.
        2:  generate gluon in x and y as if the arc is a straight line
        3:  x' = (radius+y)*cos(x*const)
            y' = (radius+y)*sin(x*const)
*/

void GluonArc(double *args)
{
    double darc, arcstart = args[3],arcend = args[4], ampi = args[5];
/*  
        When arcend comes before arcstart we have a problem. The solution is
        to flip the order and change the sign on ampi
*/
    if ( arcend < arcstart ) {
        darc = arcstart; arcstart = arcend; arcend = darc; ampi = -ampi;
    }

    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */

    SetTransferMatrix(COS(arcstart),SIN(arcstart)
                    ,-SIN(arcstart),COS(arcstart),0,0);
    darc = arcend-arcstart;
    GluonArcHelp(args,darc,ampi);
}

/*
			#] GluonArc : 
			#[ DashGluonArc : *

        DashGluonArc(x1,y1)(r,phi1,phi2){amplitude}{windings}{dashsize}

        Draws a gluon on an arcsegment
        x_center,y_center,radius,stat_angle,end_angle,gluon_radius,num
        in which num is the number of windings of the gluon.
        Method:
        1:  compute length of arc.
        2:  generate gluon in x and y as if the arc is a straight line
        3:  x' = (radius+y)*cos(x*const)
            y' = (radius+y)*sin(x*const)
*/

void DashGluonArc(double *args)
{
    double darc, arcstart = args[3],arcend = args[4], ampi = args[5];
    double dashsize = args[7];
/*  
        When arcend comes before arcstart we have a problem. The solution is
        to flip the order and change the sign on ampi
*/
    if ( arcend < arcstart ) {
        darc = arcstart; arcstart = arcend; arcend = darc; ampi = -ampi;
    }

    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */

    SetTransferMatrix(COS(arcstart),SIN(arcstart)
                    ,-SIN(arcstart),COS(arcstart),0,0);
    darc = arcend-arcstart;
    dashsize = ComputeDashGluonArc(args,darc,dashsize);
    SetDashSize(dashsize,dashsize/2);
    GluonArcHelp(args,darc,ampi);
}

/*
			#] DashGluonArc : 
			#[ Photon : *

        Photon(x1,y1)(x2,y2){amplitude}{windings}

        Each half winding is one cubic Bezier curve.
*/

void Photon(double *args)
{
    double dx = args[2] - args[0];
    double dy = args[3] - args[1];
    double dr = sqrt(dx*dx+dy*dy);

    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);

    PhotonHelp(args,dr);
}

/*
			#] Photon : 
			#[ DoublePhoton : *

        DoublePhoton(x1,y1)(x2,y2){amplitude}{windings}{sep}

        Each half winding is one cubic Bezier curve.
*/

void DoublePhoton(double *args)
{
    double dx = args[2] - args[0];
    double dy = args[3] - args[1];
    double dr = sqrt(dx*dx+dy*dy);
    linesep = args[6];

    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);

    SaveGraphicsState;
    SetLineWidth(linesep+axolinewidth);
    PhotonHelp(args,dr);
    RestoreGraphicsState;

    SaveGraphicsState;
    SetLineWidth(linesep-axolinewidth);
    SetBackgroundColor(STROKING);
    PhotonHelp(args,dr);
    RestoreGraphicsState;
}

/*
			#] DoublePhoton : 
			#[ DashPhoton : *

        DashPhoton(x1,y1)(x2,y2){amplitude}{windings}{dashsize}

        Each half winding is one cubic Bezier curve.
*/

void DashPhoton(double *args)
{
    double dx = args[2] - args[0];
    double dy = args[3] - args[1];
    double dr = sqrt(dx*dx+dy*dy);
    int numdashes, numhalfwindings = args[5]*2+0.5;
    double x, y, size;
    double dashsize = args[6], onehalfwinding = dr/numhalfwindings; 
    x = (4*onehalfwinding)/3/M_pi; y = 4.*args[4]/3;
    size = 0.5*LengthBezier(x,y, onehalfwinding-x,y, onehalfwinding,0,1.0);
    numdashes = size/(2*args[6]);
    if ( numdashes == 0 ) numdashes = 1;
    if ( fabs(size-2*dashsize*numdashes) > fabs(size-2*dashsize*(numdashes+1)) )
        numdashes++;
    dashsize = size/(2*numdashes);

    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);

    SetDashSize(dashsize,dashsize/2);
    PhotonHelp(args,dr);
}

/*
			#] DashPhoton : 
			#[ DashDoublePhoton : *

        DashDoublePhoton(x1,y1)(x2,y2){amplitude}{windings}{sep}{dashsize}

        Each half winding is one cubic Bezier curve.
*/

void DashDoublePhoton(double *args)
{
    double dx = args[2] - args[0];
    double dy = args[3] - args[1];
    double dr = sqrt(dx*dx+dy*dy);
    int numdashes, numhalfwindings = args[5]*2+0.5;
    double x, y, size;
    double dashsize = args[7], onehalfwinding = dr/numhalfwindings; 
    x = (4*onehalfwinding)/3/M_pi; y = 4.*args[4]/3;
    size = 0.5*LengthBezier(x,y, onehalfwinding-x,y, onehalfwinding,0,1.0);
    numdashes = size/(2*args[6]);
    if ( numdashes == 0 ) numdashes = 1;
    if ( fabs(size-2*dashsize*numdashes) > fabs(size-2*dashsize*(numdashes+1)) )
        numdashes++;
    dashsize = size/(2*numdashes);

    linesep = args[6];

    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);
    SetDashSize(dashsize,dashsize/2);

    SaveGraphicsState;
    SetLineWidth(linesep+axolinewidth);
    PhotonHelp(args,dr);
    RestoreGraphicsState;

/*  SetDashSize(0,0); */
    SaveGraphicsState;
    SetLineWidth(linesep-axolinewidth);
    SetBackgroundColor(STROKING);
    PhotonHelp(args,dr);
    RestoreGraphicsState;
}

/*
			#] DashDoublePhoton : 
			#[ PhotonArc : *

        PhotonArc(x1,y1)(r,phi1,phi2){amplitude}{windings}

        This routine follows the Postscript routine closely, except for that
        we do not put a transfer matrix inside the loop. The corresponding
        moveto messes up the path. One would have to put stroking operations
        in there each time.
*/

void PhotonArc(double *args)
{
    double arcstart = args[3],arcend = args[4];
    int num = 2*args[6]+0.5;

    if ( arcend < arcstart ) arcend += 360.;

    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */

    arcend = torad*(arcend-arcstart)/num;

    SetTransferMatrix(COS(arcstart),SIN(arcstart)
                    ,-SIN(arcstart),COS(arcstart),0,0);

    PhotonArcHelp(args,arcend,num);
}

/*
			#] PhotonArc : 
			#[ DoublePhotonArc : *

        DoublePhotonArc(x1,y1)(r,phi1,phi2){amplitude}{windings}{sep}
*/

void DoublePhotonArc(double *args)
{
    double arcstart = args[3],arcend = args[4];
    int num = 2*args[6]+0.5;
    linesep = args[7];

    if ( arcend < arcstart ) arcend += 360.;

    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */

    arcend = torad*(arcend-arcstart)/num;

    SetTransferMatrix(COS(arcstart),SIN(arcstart)
                    ,-SIN(arcstart),COS(arcstart),0,0);

    SaveGraphicsState;
    SetLineWidth(linesep+axolinewidth);
    PhotonArcHelp(args,arcend,num);
    RestoreGraphicsState;

    SaveGraphicsState;
    SetLineWidth(linesep-axolinewidth);
    SetBackgroundColor(STROKING);
    PhotonArcHelp(args,arcend,num);
    RestoreGraphicsState;
}

/*
			#] DoublePhotonArc : 
			#[ DashPhotonArc : *

        DashPhotonArc(x1,y1)(r,phi1,phi2){amplitude}{windings}{dashsize}
*/

void DashPhotonArc(double *args)
{
    double arcstart = args[3],arcend = args[4];
    double dashsize = args[7], dashstart;
    int num = 2*args[6]+0.5;

    if ( arcend < arcstart ) arcend += 360.;

    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */

    arcend = torad*(arcend-arcstart)/num;

    SetTransferMatrix(COS(arcstart),SIN(arcstart)
                    ,-SIN(arcstart),COS(arcstart),0,0);

    dashsize = ComputeDashPhotonArc(args,arcend,dashsize,&dashstart);
    SetDashSize(dashsize,dashstart);
    PhotonArcHelp(args,arcend,num);
}

/*
			#] DashPhotonArc : 
			#[ DashDoublePhotonArc : *

        DashDoublePhotonArc(x1,y1)(r,phi1,phi2){amplitude}{windings}{sep}{dashsize}
*/

void DashDoublePhotonArc(double *args)
{
    double arcstart = args[3],arcend = args[4];
    double dashsize = args[8], dashstart;
    int num = 2*args[6]+0.5;
    linesep = args[7];

    if ( arcend < arcstart ) arcend += 360.;

    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */

    arcend = torad*(arcend-arcstart)/num;

    SetTransferMatrix(COS(arcstart),SIN(arcstart)
                    ,-SIN(arcstart),COS(arcstart),0,0);
    dashsize = ComputeDashPhotonArc(args,arcend,dashsize,&dashstart);

    SaveGraphicsState;
    SetDashSize(dashsize,dashstart);
    SetLineWidth(linesep+axolinewidth);
    PhotonArcHelp(args,arcend,num);
    RestoreGraphicsState;

    SaveGraphicsState;
    SetDashSize(0,0);
    SetLineWidth(linesep-axolinewidth);
    SetBackgroundColor(STROKING);
    PhotonArcHelp(args,arcend,num);
    RestoreGraphicsState;
}

/*
			#] DashDoublePhotonArc : 
			#[ ZigZag : *

        ZigZag(x1,y1)(x2,y2){amplitude}{windings}

        We draw each half winding as two straight lines.
        This can be done better!
*/

void ZigZag(double *args)
{
    double dx = args[2] - args[0];
    double dy = args[3] - args[1];
    double dr = sqrt(dx*dx+dy*dy);

    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);
    ZigZagHelp(args,dr);
}

/*
			#] ZigZag : 
			#[ DoubleZigZag : *

        DoubleZigZag(x1,y1)(x2,y2){amplitude}{windings}{sep}

        We draw each half winding as two straight lines.
        This can be done better!
*/

void DoubleZigZag(double *args)
{
    double dx = args[2] - args[0];
    double dy = args[3] - args[1];
    double dr = sqrt(dx*dx+dy*dy);
    linesep = args[6];

    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);

    SaveGraphicsState;
    SetLineWidth(linesep+axolinewidth);
    ZigZagHelp(args,dr);
    RestoreGraphicsState;

    SaveGraphicsState;
    SetLineWidth(linesep-axolinewidth);
    SetBackgroundColor(STROKING);
    ZigZagHelp(args,dr);
    RestoreGraphicsState;
}

/*
			#] DoubleZigZag : 
			#[ DashZigZag : *

        DashZigZag(x1,y1)(x2,y2){amplitude}{windings}{dashsize}

        We should recalculate the size of the dashes. Otherwise the points
        of the teeth can become messy.
*/

void DashZigZag(double *args)
{
    double dx = args[2] - args[0];
    double dy = args[3] - args[1];
    double dr = sqrt(dx*dx+dy*dy);
    double dashsize = args[6];
    int n = args[5]*2+0.5;
    double size = dr/(n*2);

    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);

    size = sqrt(size*size+args[4]*args[4]);
    n = size/(2*dashsize);   /* number of complete dash patterns rounded down */
/*
    Now test what is closer to dash: size/n or size/(n+1)
*/
    if ( n == 0 ) n = 1;
    if ( fabs(size-2*dashsize*n) > fabs(size-2*dashsize*(n+1)) ) n++;
    dashsize = size/(2*n);

    SetDashSize(dashsize,dashsize/2);
    ZigZagHelp(args,dr);
}

/*
			#] DashZigZag : 
			#[ DashDoubleZigZag : *

        DashDoubleZigZag(x1,y1)(x2,y2){amplitude}{windings}{sep}{dashsize}

        We draw each half winding as two straight lines.
        This can be done better!
*/

void DashDoubleZigZag(double *args)
{
    double dx = args[2] - args[0];
    double dy = args[3] - args[1];
    double dr = sqrt(dx*dx+dy*dy);
    double dashsize = args[7];
    int n = args[5]*2+0.5;
    double size = dr/(n*2);
    linesep = args[6];

    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);

    size = sqrt(size*size+args[4]*args[4]);
    n = size/(2*dashsize);   /* number of complete dash patterns rounded down */
/*
    Now test what is closer to dash: size/n or size/(n+1)
*/
    if ( n == 0 ) n = 1;
    if ( fabs(size-2*dashsize*n) > fabs(size-2*dashsize*(n+1)) ) n++;
    dashsize = size/(2*n);

    SetDashSize(dashsize,dashsize/2);

    SaveGraphicsState;
    SetLineWidth(linesep+axolinewidth);
    ZigZagHelp(args,dr);
    RestoreGraphicsState;

    SetDashSize(0,0);

    SaveGraphicsState;
    SetLineWidth(linesep-axolinewidth);
    SetBackgroundColor(STROKING);
    ZigZagHelp(args,dr);
    RestoreGraphicsState;
}

/*
			#] DashDoubleZigZag : 
			#[ ZigZagArc : *

        ZigZagArc(x1,y1)(r,phi1,phi2){amplitude}{windings}
*/

void ZigZagArc(double *args)
{
    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */
    ZigZagArcHelp(args);
}

/*
			#] ZigZagArc : 
			#[ DoubleZigZagArc : *

        DoubleZigZagArc(x1,y1)(r,phi1,phi2){amplitude}{windings}{sep}
*/

void DoubleZigZagArc(double *args)
{
    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */
    linesep = args[7];

    SaveGraphicsState;
    SetLineWidth(linesep+axolinewidth);
    ZigZagArcHelp(args);
    RestoreGraphicsState;

    SaveGraphicsState;
    SetLineWidth(linesep-axolinewidth);
    SetBackgroundColor(STROKING);
    ZigZagArcHelp(args);
    RestoreGraphicsState;
}

/*
			#] DoubleZigZagArc : 
			#[ DashZigZagArc : *

        DashZigZagArc(x1,y1)(r,phi1,phi2){amplitude}{windings}{dashsize}
*/

void DashZigZagArc(double *args)
{
    double dashsize = args[7], dashstart;
    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */
    {
        int num = 2*args[6]-0.5, numdashes;
        double amp = args[5], r = args[2], size, size2;
        double arcstart = args[3], arcend = args[4], darc;
        if ( arcend < arcstart ) arcend += 360.;
        darc = (arcend-arcstart)/(num+1);
        size = sqrt(0.5*(amp*amp+r*r-(r*r-amp*amp)*COS(darc)));
        size2 = sqrt(amp*amp+2*(amp+r)*r*(1-COS(darc/2)));
        numdashes = size/(2*dashsize);
        if ( numdashes == 0 ) numdashes = 1;
        if ( fabs(size-2*dashsize*numdashes) > fabs(size-2*dashsize*(numdashes+1)) )
                numdashes++;
        dashsize = size/(2*numdashes);
        num = size2/(2*dashsize);
        dashstart = -(size2 - 2*num*dashsize) + dashsize/2;
        if ( dashstart < 0 ) dashstart += 2*dashsize;
    }
    SetDashSize(dashsize,dashstart);
    ZigZagArcHelp(args);
}

/*
			#] DashZigZagArc : 
			#[ DashDoubleZigZagArc : *

        DashDoubleZigZagArc(x1,y1)(r,phi1,phi2){amplitude}{windings}{sep}{dashsize}
*/

void DashDoubleZigZagArc(double *args)
{
    double dashsize = args[8], dashstart;
    SetTransferMatrix(1,0,0,1,args[0],args[1]);  /* Move to center of circle */
    linesep = args[7];
    {
        int num = 2*args[6]-0.5, numdashes;
        double amp = args[5], r = args[2], size, size2;
        double arcstart = args[3], arcend = args[4], darc;
        if ( arcend < arcstart ) arcend += 360.;
        darc = (arcend-arcstart)/(num+1);
        size = sqrt(0.5*(amp*amp+r*r-(r*r-amp*amp)*COS(darc)));
        size2 = sqrt(amp*amp+2*(amp+r)*r*(1-COS(darc/2)));
        numdashes = size/(2*dashsize);
        if ( numdashes == 0 ) numdashes = 1;
        if ( fabs(size-2*dashsize*numdashes) > fabs(size-2*dashsize*(numdashes+1)) )
                numdashes++;
        dashsize = size/(2*numdashes);
        num = size2/(2*dashsize);
        dashstart = -(size2 - 2*num*dashsize) + dashsize/2;
        if ( dashstart < 0 ) dashstart += 2*dashsize;
    }

    SaveGraphicsState;
    SetDashSize(dashsize,dashstart);
    SetLineWidth(linesep+axolinewidth);
    ZigZagArcHelp(args);
    RestoreGraphicsState;

    SaveGraphicsState;
    SetDashSize(0,0);
    SetLineWidth(linesep-axolinewidth);
    SetBackgroundColor(STROKING);
    ZigZagArcHelp(args);
    RestoreGraphicsState;
}

/*
			#] DashDoubleZigZagArc : 
 		#] Particle routines : 
 		#[ Drawing routines :
			#[ Polygon :
*/

void Polygon(double *args,int num,int type)
{
    int i;
    MoveTo(args[0],args[1]);
    args += 2;
    for ( i = 1; i < num; i++, args += 2 ) {
        LineTo(args[0],args[1]);
    }
    if ( type == 0 ) { CloseAndStroke; }
    else if ( type ==  1 ) { CloseAndFill; }
}

/*
			#] Polygon : 
			#[ Curve : +

        Curve{(x1,y1),...,(xn,yn)}

        num is the number of pairs in points.
*/

void Curve(double *points,int num)
{
    int i, ss;
    double x0,y0,x1,y1,x2,y2,x3,y3;

    if ( num < 2 ) return;
    if ( num == 2 ) { Line(points); return; }

    ss = 2*num;

    x1 = points[0]; y1 = points[1];
    x2 = points[2]; y2 = points[3];
    x3 = points[4]; y3 = points[5];
    x0 = 2*x1-x2;
    y0 = 2*((y3-y2)/(x3-x2)-(y2-y1)/(x2-x1))*((x2-x1)*(x2-x1)/(x3-x1))+2*y1-y2;

    MoveTo(x1,y1);
    DoCurve(x0,y0,x1,y1,x2,y2,x3,y3);

    for ( i = 0; i < ss-6; i += 2 ) {
        DoCurve(points[i  ],points[i+1],points[i+2],points[i+3],
                points[i+4],points[i+5],points[i+6],points[i+7]);
    }
    if ( ss > 6 ) {
        x0 = points[ss-6]; y0 = points[ss-5];
        x1 = points[ss-4]; y1 = points[ss-3];
        x2 = points[ss-2]; y2 = points[ss-1];
        x3 = 2*x2-x1;
        y3 = 2*((y2-y1)/(x2-x1)-(y1-y0)/(x1-x0))*((x2-x1)*(x2-x1)/(x2-x0))+2*y2-y1;

        DoCurve(x0,y0,x1,y1,x2,y2,x3,y3);
    }
    Stroke;
}

/*
			#] Curve : 
			#[ DashCurve : +

        DashCurve{(x1,y1),...,(xn,yn)}{dashsize}
*/

void DashCurve(double *args,int num1)
{
    double dashsize = args[2*num1];
    if ( num1 == 2 ) {
        DashLine(args);
    }
    else if ( num1 > 2 ) {
        SetDashSize(dashsize,dashsize/2);
        Curve(args,num1);
    }
}

/*
			#] DashCurve : 
			#[ LogAxis :

        Draws a line with logarithmic hash marks along it.
        LogAxis(x1,y1)(x2,y2)(num_logs,hashsize,offset,width)
        The line is from (x1,y1) to (x2,y2) and the marks are on the left side
        when hashsize is positive, and right when it is negative.
        num_logs is the number of orders of magnitude and offset is the number
        at which one starts at (x1,y1) (like if offset=2 we start at 2)
        When offset is 0 we start at 1. Width is the linewidth.
*/

void LogAxis(double *args)
{
    double dx = args[2]-args[0], dy = args[3]-args[1], dr = sqrt(dx*dx+dy*dy);
    double width = args[7], size, nlogs = args[4], hashsize = args[5];
    double offset = args[6], x;
    int i, j;
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);
    MoveTo(0,0); LineTo(dr,0); Stroke;
/*
    Now compute the hash marks.
*/
    size = dr/nlogs;
    if ( offset <= 0 ) { offset = 0; }
    else { offset = log10(offset); }
/*
    Big hash marks
*/
    for ( i = 1; i <= nlogs; i++ ) {
        MoveTo((i-offset)*size,0);
        LineTo((i-offset)*size,hashsize*1.2);
        Stroke;
    }
/*
    Little hash marks
*/
    SetLineWidth(0.6*width);
    for ( i = 0; i <= nlogs; i++ ) {
        for ( j = 2; j < 10; j++ ) {
            x = (i-offset+log10(j))*size;
            if ( x >= 0 && x <= dr ) {
                MoveTo(x,0); LineTo(x,hashsize*0.8); Stroke;
            }
        }
    }
}

/*
			#] LogAxis : 
			#[ LinAxis :

        Draws a line with linear hash marks along it.
        LinAxis(x1,y1)(x2,y2)(num_decs,per_dec,hashsize,offset,width)
        The line is from (x1,y1) to (x2,y2) and the marks are on the left side
        when hashsize is positive, and right when it is negative.
        num_decs is the number of accented marks, per_dec the number of
        divisions between them and offset is the number
        at which one starts at (x1,y1) (like if offset=2 we start at the second
        small mark) Width is the linewidth.
*/

void LinAxis(double *args)
{
    double width = args[8], hashsize = args[6], x;
    double dx = args[2]-args[0], dy = args[3]-args[1], dr = sqrt(dx*dx+dy*dy);
    double num_decs = args[4], per_dec = args[5], size, size2;
    int i, j, numperdec = per_dec+0.5, offset = args[7];
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(dx/dr,dy/dr,-dy/dr,dx/dr,0,0);
    MoveTo(0,0); LineTo(dr,0); Stroke;
    size = dr/num_decs;
    if ( numperdec > 1 ) size2 = size / numperdec;
    else { size2 = size; numperdec = 1; }
    if ( offset > numperdec ) offset = numperdec;
    else if ( offset <= 0 ) offset = 0;
/*
    Big hashes
*/
    for ( i = 0; i <= num_decs; i++ ) {
        x = i*size-offset*size2;
        if ( x >= 0 && x <= dr ) {
            MoveTo(x,0); LineTo(x,hashsize*1.2); Stroke;
        }
    }
/*
    Little hash marks.
*/
    j = num_decs*numperdec+0.5;
    SetLineWidth(0.6*width);
    for ( i = 0; i <= j; i++ ) {
        if ( (i+offset)%numperdec != 0 ) {
            x = i*size2;
            if ( x >= 0 && x <= dr ) {
                MoveTo(x,0); LineTo(x,hashsize*0.8); Stroke;
            }
        }
    }
}

/*
			#] LinAxis : 
			#[ BezierCurve :

            Draws a Bezier curve. Starts at (x1,y1).
            The control points are (x2,y2),(x3,y3),(x4,y4)
*/

void BezierCurve(double *args)
{
    MoveTo(args[0],args[1]);
    Bezier(args[2],args[3],args[4],args[5],args[6],args[7]);
    Stroke;
    if ( witharrow ) BezierArrow(args);
}

/*
			#] BezierCurve : 
			#[ DoubleBezier :

            Draws a Bezier curve. Starts at (x1,y1).
            The control points are (x2,y2),(x3,y3),(x4,y4)
*/

void DoubleBezier(double *args)
{
    linesep = args[8];
    SaveGraphicsState;
    SetLineWidth(linesep+axolinewidth);
    MoveTo(args[0],args[1]);
    Bezier(args[2],args[3],args[4],args[5],args[6],args[7]);
    Stroke;
    RestoreGraphicsState;
    SaveGraphicsState;
    SetLineWidth(linesep-axolinewidth);
    SetBackgroundColor(STROKING);
    MoveTo(args[0],args[1]);
    Bezier(args[2],args[3],args[4],args[5],args[6],args[7]);
    Stroke;
    RestoreGraphicsState;
    if ( witharrow ) BezierArrow(args);
}

/*
			#] DoubleBezier : 
			#[ DashBezier :

            Draws a Bezier curve. Starts at (x1,y1).
            The control points are (x2,y2),(x3,y3),(x4,y4)
*/

void DashBezier(double *args)
{
    int numdashes;
    double size, dashsize = args[8];
    size = LengthBezier(args[2]-args[0],args[3]-args[1]
        ,args[4]-args[0],args[5]-args[1],args[6]-args[0],args[7]-args[1],1.0);

    numdashes = size/(2*dashsize);
    if ( numdashes == 0 ) numdashes = 1;
    if ( fabs(size-2*dashsize*numdashes) > fabs(size-2*dashsize*(numdashes+1)) )
        numdashes++;
    dashsize = (size/(2*numdashes));

    SetDashSize(dashsize,dashsize/2);
    MoveTo(args[0],args[1]);
    Bezier(args[2],args[3],args[4],args[5],args[6],args[7]);
    Stroke;
    if ( witharrow ) BezierArrow(args);
}

/*
			#] DashBezier : 
			#[ DashDoubleBezier :

            Draws a Bezier curve. Starts at (x1,y1).
            The control points are (x2,y2),(x3,y3),(x4,y4)
*/

void DashDoubleBezier(double *args)
{
    int numdashes;
    double size, dashsize = args[9];
    size = LengthBezier(args[2]-args[0],args[3]-args[1]
        ,args[4]-args[0],args[5]-args[1],args[6]-args[0],args[7]-args[1],1.0);

    numdashes = size/(2*dashsize);
    if ( numdashes == 0 ) numdashes = 1;
    if ( fabs(size-2*dashsize*numdashes) > fabs(size-2*dashsize*(numdashes+1)) )
        numdashes++;
    dashsize = (size/(2*numdashes));

    SetDashSize(dashsize,dashsize/2);
    linesep = args[8];
    SaveGraphicsState;
    SetLineWidth(linesep+axolinewidth);
    MoveTo(args[0],args[1]);
    Bezier(args[2],args[3],args[4],args[5],args[6],args[7]);
    Stroke;
    RestoreGraphicsState;
    SaveGraphicsState;
    SetLineWidth(linesep-axolinewidth);
    SetBackgroundColor(STROKING);
    MoveTo(args[0],args[1]);
    Bezier(args[2],args[3],args[4],args[5],args[6],args[7]);
    Stroke;
    RestoreGraphicsState;
    if ( witharrow ) BezierArrow(args);
}

/*
			#] DashDoubleBezier : 
 		#] Drawing routines : 
 		#[ Wrapper routines :
			#[ AxoArc :

            Draws arc centered at (#1,#2), radius #3, starting and ending
            angles #4, #5.
            Double, dashing, arrow, flip, clockwise
*/

void AxoArc(double *args)
{
    double dashsize = args[6];
    linesep = args[5];
    GetArrow(args+7);
    if ( args[16] ) {   /* If clockwise: reverse the angles and the arrow */
        double e;
        clockwise = 1;  /* In principle not needed */
        flip = 1-flip;
        arrow.where = 1-arrow.where;
        e = args[3]; args[3] = args[4]; args[4] = e;
    }

    if ( witharrow ) {
        if ( arrow.where > 1 ) arrow.where = 1;
        if ( arrow.where < 0 ) arrow.where = 0;
        if ( dashsize > 0 ) {
            if ( linesep > 0 ) {
                DashArrowDoubleArc(args);
            }
            else {
                args[5] = args[6];
                DashArrowArc(args);
            }
        }
        else {
            if ( linesep > 0 ) {
                ArrowDoubleArc(args);
            }
            else {
                ArrowArc(args);
            }
        }
    }
    else {
        if ( dashsize > 0 ) {
            if ( linesep > 0 ) {
                DashDoubleArc(args);
            }
            else {
                args[5] = args[6];
                DashCArc(args);
            }
        }
        else {
            if ( linesep > 0 ) {
                DoubleArc(args);
            }
            else {
                CArc(args);
            }
        }
    }
}

/*
			#] AxoArc : 
			#[ AxoBezier :
*/

void AxoBezier(double *args)
{
    linesep = args[8];
    GetArrow(args+10);
    if ( witharrow ) {
        if ( arrow.where > 1 ) arrow.where = 1;
        if ( arrow.where < 0 ) arrow.where = 0;
    }
    if ( args[9] ) {    /* dashes */
        if ( args[8] ) {    /* double */
            DashDoubleBezier(args);
        }
        else {
            args[8] = args[9];
            DashBezier(args);
        }
    }
    else {
        if ( args[8] ) {    /* double */
            DoubleBezier(args);
        }
        else {
            BezierCurve(args);  /* The name Bezier was already taken */
        }
    }
}

/*
			#] AxoBezier : 
			#[ AxoGluon :
*/

void AxoGluon(double *args)
{
        SetLineWidth(axolinewidth + args[6]);
    if ( args[7] ) {    /* dashes */
        args[6] = args[7];
        DashGluon(args);
    }
    else {
        Gluon(args);
    }
}

/*
			#] AxoGluon : 
			#[ AxoGluonArc :
*/

void AxoGluonArc(double *args)
{
        SetLineWidth(axolinewidth + args[7]);
    if ( args[9] ) {    /* Clockwise */
        double a = args[3]; args[3] = args[4]; args[4] = a;
    }
    if ( args[8] ) {  /* Dashes */
        args[7] = args[8];
        DashGluonArc(args);
    }
    else {
            GluonArc(args);
    }
}

/*
			#] AxoGluonArc : 
			#[ AxoGluonCirc :
*/

void AxoGluonCirc(double *args)
{
        SetLineWidth(axolinewidth + args[6]);
    if ( args[7] ) {    /* dashes */
        args[6] = args[7];
        DashGluonCirc(args);
    }
    else {
        GluonCirc(args);
    }
}

/*
			#] AxoGluonCirc : 
			#[ AxoLine :

        AxoLine(x1,y1)(x2,y2){sep}{dashsize}{stroke width length inset}{where}

        Generic switchyard to the various routines for compatibility
        with Jaxodraw and axodraw4j

        Note: because the specific routines can be called either in the
        direct way or by means of the generic routine, they have to know
        what arrow to use. This is regulated by arrow.type. 0=old arrows.
*/

void AxoLine(double *args)
{
    linesep = args[4];
    GetArrow(args+6);
    if ( witharrow ) {
        if ( arrow.where > 1 ) arrow.where = 1;
        if ( arrow.where < 0 ) arrow.where = 0;
        if ( args[5] == 0 ) {
            if ( linesep == 0 ) ArrowLine(args);
            else { ArrowDoubleLine(args); }
        }
        else {
            if ( linesep == 0 ) {
                args[4] = args[5];
                DashArrowLine(args);
            }
            else { DashArrowDoubleLine(args); }
        }
    }
    else {
        if ( args[5] == 0 ) {  /* No dashing */
            if ( linesep == 0 ) Line(args);
            else                DoubleLine(args);
        }
        else {
            if ( linesep == 0 ) {
                args[4] = args[5]; DashLine(args);
            }
            else DashDoubleLine(args);
        }
    }
}

/*
			#] AxoLine : 
			#[ AxoPhoton :
*/

void AxoPhoton(double *args)
{
    if ( args[7] ) {    /* dashes */
        if ( args[6] ) {    /* double */
            DashDoublePhoton(args);
        }
        else {
            args[6] = args[7];
            DashPhoton(args);
        }
    }
    else {
        if ( args[6] ) {    /* double */
            DoublePhoton(args);
        }
        else {
            Photon(args);
        }
    }
}

/*
			#] AxoPhoton : 
			#[ AxoPhotonArc :
*/

void AxoPhotonArc(double *args)
{
    if ( args[9] ) {    /* Clockwise */
        int num = 2*args[6]+0.5;
        double a = args[3]; args[3] = args[4]; args[4] = a;
        if ( ( num & 1 ) == 0 ) args[5] = -args[5];
    }
    if ( args[8] ) {  /* dash */
        if ( args[7] ) {    /* double */
            DashDoublePhotonArc(args);
        }
        else {
            args[7] = args[8];
            DashPhotonArc(args);
        }
    }
    else {
        if ( args[7] ) {    /* double */
            DoublePhotonArc(args);
        }
        else {
            PhotonArc(args);
        }
    }
}

/*
			#] AxoPhotonArc : 
			#[ AxoZigZag :
*/

void AxoZigZag(double *args)
{
    if ( args[7] ) {    /* dashes */
        if ( args[6] ) {    /* double */
            DashDoubleZigZag(args);
        }
        else {
            args[6] = args[7];
            DashZigZag(args);
        }
    }
    else {
        if ( args[6] ) {    /* double */
            DoubleZigZag(args);
        }
        else {
            ZigZag(args);
        }
    }
}

/*
			#] AxoZigZag : 
			#[ AxoZigZagArc :
*/

void AxoZigZagArc(double *args)
{
    if ( args[9] ) {    /* Clockwise */
        int num = 2*args[6]+0.5;
        double a = args[3]; args[3] = args[4]; args[4] = a;
        if ( ( num & 1 ) == 0 ) args[5] = -args[5];
    }
    if ( args[8] ) {  /* dash */
        if ( args[7] ) {    /* double */
            DashDoubleZigZagArc(args);
        }
        else {
            args[7] = args[8];
            DashZigZagArc(args);
        }
    }
    else {
        if ( args[7] ) {    /* double */
            DoubleZigZagArc(args);
        }
        else {
            ZigZagArc(args);
        }
    }
}

/*
			#] AxoZigZagArc : 
 		#] Wrapper routines : 
 		#[ Various routines :
			#[ Rotate : +

        Rotate: x y angle hmode vmode textwidth textheight
        Note, the textwidth/textheight have been scaled already;
*/

void Rotate(double *args)
{
    double textheight = args[6]*args[4]/2/65536.;
    double textwidth  = args[5]*args[3]/2/65536.;
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    SetTransferMatrix(COS(args[2]),SIN(args[2]),-SIN(args[2]),COS(args[2]),0,0);
    SetTransferMatrix(1,0,0,1,-textwidth,textheight);
}

/*
			#] Rotate : 
			#[ Grid :

        Makes a coordinate grid in the indicated color.
        (x0,y0)(incx,incy)(nx,ny){color}{linewidth}
*/

void Grid(double *args)
{
    int i, nx = args[4]+0.01, ny = args[5]+0.01;
    double maxx = args[2]*args[4];
    double maxy = args[3]*args[5];
    SetTransferMatrix(1,0,0,1,args[0],args[1]);
    for ( i = 0; i <= nx; i++ ) {
        MoveTo(i*args[2],0);
        LineTo(i*args[2],maxy);
        Stroke;
    }
    for ( i = 0; i <= ny; i++ ) {
        MoveTo(0,i*args[3]);
        LineTo(maxx,i*args[3]);
        Stroke;
    }
}

/*
			#] Grid : 
 		#] Various routines : 
  	#] routines : 
*/
