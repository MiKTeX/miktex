/*****
 * bezierpatch.cc
 * Authors: John C. Bowman and Jesse Frohlich
 *
 * Render Bezier patches and triangles.
 *****/

#include "bezierpatch.h"
#include "predicates.h"

namespace camp {

using ::orient2d;
using ::orient3d;

#ifdef HAVE_GL

int MaterialIndex;

//std::vector<GLuint>& I=transparentData.Indices;
//std::vector<VertexData>& V=transparentData.Vertices;
bool colors;

std::vector<GLfloat> zbuffer;
std::vector<GLfloat> xbuffer;
std::vector<GLfloat> ybuffer;

//std::vector<GLfloat> xmin,ymin,zmin;
//std::vector<GLfloat> xmax,ymax,zmax;
std::vector<GLfloat> zsum;

inline double min(double a, double b, double c)
{
  return min(min(a,b),c);
}

inline double max(double a, double b, double c)
{
  return max(max(a,b),c);
}

struct iz {
  unsigned i;
  double z;
  iz() {}
  void minimum(unsigned i, const std::vector<GLuint>& I) {
    this->i=i;
    unsigned i3=3*i;
    z=min(zbuffer[I[i3]],zbuffer[I[i3+1]],zbuffer[I[i3+2]]);
  }
};

std::vector<iz> IZ;

const double FillFactor=0.1;

inline int sgn1(double x) 
{
  return x >= 0.0 ? 1 : -1;
}

inline int sgn(double x) 
{
  return (x > 0.0 ? 1 : (x < 0.0 ? -1 : 0));
}

bool sameside(const double *a, const double *b, int s0, const double *A,
              const double *B, const double *C)
{
  if(sgn(orient2d(a,b,A)) == s0) return true;
  if(sgn(orient2d(a,b,B)) == s0) return true;
  if(sgn(orient2d(a,b,C)) == s0) return true;
  return false;
}

// returns true iff 2D triangles abc and ABC intersect
bool intersect2D(const double *a, const double *b, const double *c,
                 const double *A, const double *B, const double *C)
{
  int s0=sgn(orient2d(a,b,c)); // Optimize away
  int S0=sgn(orient2d(A,B,C)); // Optimize away
  return
    sameside(a,b,s0,A,B,C) &&
    sameside(b,c,s0,A,B,C) &&
    sameside(c,a,s0,A,B,C) &&
    sameside(A,B,S0,a,b,c) &&
    sameside(B,C,S0,a,b,c) &&
    sameside(C,A,S0,a,b,c);
}

// returns true iff triangle abc is pierced by line segment AB.
bool pierce(const double *a, const double *b, const double *c, const double *A, const double *B)
{
  int sa=sgn(orient3d(A,b,c,B));
  int sb=sgn(orient3d(A,c,a,B));
  int sc=sgn(orient3d(A,a,b,B));
  return sa == sb && sb == sc; 
}

// returns true iff triangle abc is pierced by an edge of triangle ABC
bool intersect0(const double *a, const double *b, const double *c,
                const double *A, const double *B, const double *C,
                int sA, int sB, int sC)
{
  if(sA != sB) {
    if(pierce(a,b,c,A,B)) return true;
    if(sC != sA) {
      if(pierce(a,b,c,C,A)) return true;
    } else {
      if(pierce(a,b,c,B,C)) return true;
    }
  } else {
    if(pierce(a,b,c,B,C)) return true;
    if(pierce(a,b,c,C,A)) return true;
  }
  return false;
}  

// returns true iff triangle abc intersects triangle ABC
bool intersect3D(const double *a, const double *b, const double *c,
                 const double *A, const double *B, const double *C)
{
  int sA=sgn(orient3d(a,b,c,A));
  int sB=sgn(orient3d(a,b,c,B));
  int sC=sgn(orient3d(a,b,c,C));
  if(sA == sB && sB == sC) return false;

  int sa=sgn(orient3d(A,B,C,a));
  int sb=sgn(orient3d(A,B,C,b));
  int sc=sgn(orient3d(A,B,C,c));
  if(sa == sb && sb == sc) return false;

  return intersect0(a,b,c,A,B,C,sA,sB,sC) || intersect0(A,B,C,a,b,c,sa,sb,sc);
}

// Return the intersection time of the extension of the line segment PQ
// with the plane perpendicular to n and passing through Z.
inline double intersect(const double *P, const double *Q, const double *n,
                        const double *Z)
{
  double d=n[0]*Z[0]+n[1]*Z[1]+n[2]*Z[2];
  double denom=n[0]*(Q[0]-P[0])+n[1]*(Q[1]-P[1])+n[2]*(Q[2]-P[2]);
  return denom == 0 ? DBL_MAX : (d-n[0]*P[0]-n[1]*P[1]-n[2]*P[2])/denom;
}
                    
inline triple interp(const double *a, const double *b, double t)
{
  return triple(a[0]+t*(b[0]-a[0]),a[1]+t*(b[1]-a[1]),a[2]+t*(b[2]-a[2]));
}

inline void interp(GLfloat *dest,
                   const GLfloat *a, const GLfloat *b, double t)
{
  double onemt=1.0-t;
  for(size_t i=0; i < 4; ++i)
    dest[i]=onemt*a[i]+t*b[i];
}

inline triple interp(const triple& a, const triple& b, double t)
{
  return a+(b-a)*t;
}

unsigned n;
unsigned int count;
  
// Sort nonintersecting triangles by depth.
int compare(const void *p, const void *P)
{
  unsigned Ia=((GLuint *) p)[0];
  unsigned Ib=((GLuint *) p)[1];
  unsigned Ic=((GLuint *) p)[2];

  unsigned IA=((GLuint *) P)[0];
  unsigned IB=((GLuint *) P)[1];
  unsigned IC=((GLuint *) P)[2];
  
  return zbuffer[Ia]+zbuffer[Ib]+zbuffer[Ic] < 
    zbuffer[IA]+zbuffer[IB]+zbuffer[IC] ? -1 : 1;
  
  /*
  double a[]={xbuffer[Ia],ybuffer[Ia],zbuffer[Ia]};
  double b[]={xbuffer[Ib],ybuffer[Ib],zbuffer[Ib]};
  double c[]={xbuffer[Ic],ybuffer[Ic],zbuffer[Ic]};
      
  double A[]={xbuffer[IA],ybuffer[IA],zbuffer[IA]};
  double B[]={xbuffer[IB],ybuffer[IB],zbuffer[IB]};
  double C[]={xbuffer[IC],ybuffer[IC],zbuffer[IC]};

  double viewpoint[]={0,0,100000};
  
  double sa=-orient3d(A,B,C,a);
  double sb=-orient3d(A,B,C,b);
  double sc=-orient3d(A,B,C,c);
  double s=min(sa,sb,sc);
  double S=max(sa,sb,sc);
  double eps=1000;

  if(s < -eps && S > eps) { //swap
    double sA=-orient3d(a,b,c,A);
    double sB=-orient3d(a,b,c,B);
    double sC=-orient3d(a,b,c,C);
    double s=min(sA,sB,sC);
    double S=max(sA,sB,sC);
    if(S < -s) S=s;
    int sz=sgn1(orient3d(a,b,c,viewpoint));
    if(S < -eps) return -sz;
    if(S > eps) return sz;
  }
  if(S < -s) S=s;
  int sz=sgn1(orient3d(A,B,C,viewpoint));
  if(S < -eps) return sz;
  if(S > eps) return -sz;
  return a[2]+b[2]+c[2] < A[2]+B[2]+C[2] ? -1 : 1;
  */
}

#if 0
void split(unsigned i3, GLuint ia, GLuint ib, GLuint ic,
           double *a, double *b, double *c, double *N, double *A) {
  double td=intersect(a,b,N,A);
  double te=intersect(a,c,N,A);
      
  triple d=interp(a,b,td);
  triple e=interp(a,c,te);
      
  GLuint Ia=tstride*ia;
  GLuint Ib=tstride*ib;
  GLuint Ic=tstride*ic;
    
  triple na=triple(V[Ia+3],V[Ia+4],V[Ia+5]);
  triple nb=triple(V[Ib+3],V[Ib+4],V[Ib+5]);
  triple nc=triple(V[Ic+3],V[Ic+4],V[Ic+5]);
  
  triple nd=interp(na,nb,td);
  triple ne=interp(na,nc,te);
      
  GLuint id,ie;
  
  if(colors) {
    GLfloat *ca=&V[Ia+6];
    GLfloat *cb=&V[Ib+6];
    GLfloat *cc=&V[Ic+6];
        
    GLfloat cd[4],ce[4];
    interp(cd,ca,cb,td);
    interp(ce,ca,cc,te);
        
    id=data.Vertex(d,nd,cd);
    ie=data.Vertex(e,ne,ce);
  } else {
    id=data.Vertex(d,nd);
    ie=data.Vertex(e,ne);
  }
      
  I[i3]=ia;
  I[i3+1]=id;
  I[i3+2]=ie;
      
  I.push_back(id);
  I.push_back(ib);
  I.push_back(ie);
      
  I.push_back(ie);
  I.push_back(ib);
  I.push_back(ic);
}
#endif

void BezierPatch::init(double res)
{
  res2=res*res;
  Epsilon=FillFactor*res;

  MaterialIndex=transparent ?
    (color ? -1-materialIndex : 1+materialIndex) : materialIndex;
  
  pvertex=transparent ? &vertexBuffer::tvertex : &vertexBuffer::vertex;
}
    
void BezierPatch::render(const triple *p, bool straight, GLfloat *c0)
{
  triple p0=p[0];
  epsilon=0;
  for(unsigned i=1; i < 16; ++i)
    epsilon=max(epsilon,abs2(p[i]-p0));
  epsilon *= DBL_EPSILON;
    
  triple p3=p[3];
  triple p12=p[12];
  triple p15=p[15];

  triple n0=normal(p3,p[2],p[1],p0,p[4],p[8],p12);
  if(abs2(n0) <= epsilon) {
    n0=normal(p3,p[2],p[1],p0,p[13],p[14],p15);
    if(abs2(n0) <= epsilon) n0=normal(p15,p[11],p[7],p3,p[4],p[8],p12);
  }
  
  triple n1=normal(p0,p[4],p[8],p12,p[13],p[14],p15);
  if(abs2(n1) <= epsilon) {
    n1=normal(p0,p[4],p[8],p12,p[11],p[7],p3);
    if(abs2(n1) <= epsilon) n1=normal(p3,p[2],p[1],p0,p[13],p[14],p15);
  }
    
  triple n2=normal(p12,p[13],p[14],p15,p[11],p[7],p3);
  if(abs2(n2) <= epsilon) {
    n2=normal(p12,p[13],p[14],p15,p[2],p[1],p0);
    if(abs2(n2) <= epsilon) n2=normal(p0,p[4],p[8],p12,p[11],p[7],p3);
  }
    
  triple n3=normal(p15,p[11],p[7],p3,p[2],p[1],p0);
  if(abs2(n3) <= epsilon) {
    n3=normal(p15,p[11],p[7],p3,p[4],p[8],p12);
    if(abs2(n3) <= epsilon) n3=normal(p12,p[13],p[14],p15,p[2],p[1],p0);
  }
    
  GLuint i0,i1,i2,i3;
  if(color) {
    GLfloat *c1=c0+4;
    GLfloat *c2=c0+8;
    GLfloat *c3=c0+12;
    
    i0=data.Vertex(p0,n0,c0);
    i1=data.Vertex(p12,n1,c1);
    i2=data.Vertex(p15,n2,c2);
    i3=data.Vertex(p3,n3,c3);
      
    if(!straight)
      render(p,i0,i1,i2,i3,p0,p12,p15,p3,false,false,false,false,
             c0,c1,c2,c3);
  } else {
    i0=(data.*pvertex)(p0,n0);
    i1=(data.*pvertex)(p12,n1);
    i2=(data.*pvertex)(p15,n2);
    i3=(data.*pvertex)(p3,n3);
    
    if(!straight)
      render(p,i0,i1,i2,i3,p0,p12,p15,p3,false,false,false,false);
  }
    
  if(straight) {
    std::vector<GLuint> &q=data.indices;
    triple Pa[]={p0,p12,p15};
    if(!offscreen(3,Pa)) {
      q.push_back(i0);
      q.push_back(i1);
      q.push_back(i2);
    }
    triple Pb[]={p0,p15,p3};
    if(!offscreen(3,Pb)) {
      q.push_back(i0);
      q.push_back(i2);
      q.push_back(i3);
    }
  }
  append();
}

// Use a uniform partition to draw a Bezier patch.
// p is an array of 16 triples representing the control points.
// Pi are the (possibly) adjusted vertices indexed by Ii.
// The 'flati' are flatness flags for each boundary.
void BezierPatch::render(const triple *p,
                         GLuint I0, GLuint I1, GLuint I2, GLuint I3,
                         triple P0, triple P1, triple P2, triple P3,
                         bool flat0, bool flat1, bool flat2, bool flat3,
                         GLfloat *C0, GLfloat *C1, GLfloat *C2, GLfloat *C3)
{
  pair d=Distance(p);
  if(d.getx() < res2 && d.gety() < res2) { // Bezier patch is flat
    triple Pa[]={P0,P1,P2};
    std::vector<GLuint> &q=data.indices;
    if(!offscreen(3,Pa)) {
      q.push_back(I0);
      q.push_back(I1);
      q.push_back(I2);
    }
    triple Pb[]={P0,P2,P3};
    if(!offscreen(3,Pb)) {
      q.push_back(I0);
      q.push_back(I2);
      q.push_back(I3);
    }
  } else { // Patch is not flat
   if(offscreen(16,p)) return;

   /* Control points are indexed as follows:

       Coordinate
       +-----
        Index
         
        03    13    23    33
       +-----+-----+-----+
       |3    |7    |11   |15
       |     |     |     |
       |02   |12   |22   |32
       +-----+-----+-----+
       |2    |6    |10   |14
       |     |     |     |
       |01   |11   |21   |31
       +-----+-----+-----+
       |1    |5    |9    |13
       |     |     |     |
       |00   |10   |20   |30
       +-----+-----+-----+
        0     4     8     12

   */

    triple p0=p[0];
    triple p3=p[3];
    triple p12=p[12];
    triple p15=p[15];

   if(d.getx() < res2) { // flat in horizontal direction; split vertically
     /*
       P refers to a corner
       m refers to a midpoint
       s refers to a subpatch

       +--------+--------+
       |P3             P2|
       |                 |
       |       s1        |
       |                 |
       |                 |
    m1 +-----------------+ m0
       |                 |
       |                 |
       |       s0        |
       |                 |
       |P0             P1|
       +-----------------+

     */

     Split3 c0(p0,p[1],p[2],p3);
     Split3 c1(p[4],p[5],p[6],p[7]);
     Split3 c2(p[8],p[9],p[10],p[11]);
     Split3 c3(p12,p[13],p[14],p15);

     triple s0[]={p0  ,c0.m0,c0.m3,c0.m5,
                  p[4],c1.m0,c1.m3,c1.m5,
                  p[8],c2.m0,c2.m3,c2.m5,
                  p12 ,c3.m0,c3.m3,c3.m5};

     triple s1[]={c0.m5,c0.m4,c0.m2,p3,
                  c1.m5,c1.m4,c1.m2,p[7],
                  c2.m5,c2.m4,c2.m2,p[11],
                  c3.m5,c3.m4,c3.m2,p15};

     triple n0=normal(s0[12],s0[13],s0[14],s0[15],s0[11],s0[7],s0[3]);
     if(abs2(n0) <= epsilon) {
       n0=normal(s0[12],s0[13],s0[14],s0[15],s0[2],s0[1],s0[0]);
       if(abs2(n0) <= epsilon)
         n0=normal(s0[0],s0[4],s0[8],s0[12],s0[11],s0[7],s0[3]);
     }

     triple n1=normal(s1[3],s1[2],s1[1],s1[0],s1[4],s1[8],s1[12]);
     if(abs2(n1) <= epsilon) {
       n1=normal(s1[3],s1[2],s1[1],s1[0],s1[13],s1[14],s1[15]);
       if(abs2(n1) <= epsilon)
         n1=normal(s1[15],s1[11],s1[7],s1[3],s1[4],s1[8],s1[12]);
     }

     // A kludge to remove subdivision cracks, only applied the first time
     // an edge is found to be flat before the rest of the subpatch is.

     triple m0=0.5*(P1+P2);
     if(!flat1) {
       if((flat1=Straightness(p12,p[13],p[14],p15) < res2))
         m0 -= Epsilon*unit(differential(s1[12],s1[8],s1[4],s1[0]));
       else m0=s0[15];
     }

     triple m1=0.5*(P3+P0);
     if(!flat3) {
       if((flat3=Straightness(p0,p[1],p[2],p3) < res2))
         m1 -= Epsilon*unit(differential(s0[3],s0[7],s0[11],s0[15]));
       else m1=s1[0];
     }

     if(color) {
       GLfloat c0[4],c1[4];
       for(size_t i=0; i < 4; ++i) {
         c0[i]=0.5*(C1[i]+C2[i]);
         c1[i]=0.5*(C3[i]+C0[i]);
       }

       GLuint i0=data.Vertex(m0,n0,c0);
       GLuint i1=data.Vertex(m1,n1,c1);

       render(s0,I0,I1,i0,i1,P0,P1,m0,m1,flat0,flat1,false,flat3,C0,C1,c0,c1);
       render(s1,i1,i0,I2,I3,m1,m0,P2,P3,false,flat1,flat2,flat3,c1,c0,C2,C3);
     } else {
       GLuint i0=(data.*pvertex)(m0,n0);
       GLuint i1=(data.*pvertex)(m1,n1);

       render(s0,I0,I1,i0,i1,P0,P1,m0,m1,flat0,flat1,false,flat3);
       render(s1,i1,i0,I2,I3,m1,m0,P2,P3,false,flat1,flat2,flat3);
     }
     return;
   }
   if(d.gety() < res2) { // flat in vertical direction; split horizontally
     /*
       P refers to a corner
       m refers to a midpoint
       s refers to a subpatch
         
                m1
       +--------+--------+
       |P3      |      P2|
       |        |        |
       |        |        |
       |        |        |
       |        |        |
       |   s0   |   s1   |
       |        |        |
       |        |        |
       |        |        |
       |        |        |
       |P0      |      P1|
       +--------+--------+
                m0

     */

     Split3 c0(p0,p[4],p[8],p12);
     Split3 c1(p[1],p[5],p[9],p[13]);
     Split3 c2(p[2],p[6],p[10],p[14]);
     Split3 c3(p3,p[7],p[11],p15);

     triple s0[]={p0,p[1],p[2],p3,
                  c0.m0,c1.m0,c2.m0,c3.m0,
                  c0.m3,c1.m3,c2.m3,c3.m3,
                  c0.m5,c1.m5,c2.m5,c3.m5};

     triple s1[]={c0.m5,c1.m5,c2.m5,c3.m5,
                  c0.m4,c1.m4,c2.m4,c3.m4,
                  c0.m2,c1.m2,c2.m2,c3.m2,
                  p12,p[13],p[14],p15};

     triple n0=normal(s0[0],s0[4],s0[8],s0[12],s0[13],s0[14],s0[15]);
     if(abs2(n0) <= epsilon) {
       n0=normal(s0[0],s0[4],s0[8],s0[12],s0[11],s0[7],s0[3]);
       if(abs2(n0) <= epsilon)
         n0=normal(s0[3],s0[2],s0[1],s0[0],s0[13],s0[14],s0[15]);
     }

     triple n1=normal(s1[15],s1[11],s1[7],s1[3],s1[2],s1[1],s1[0]);
     if(abs2(n1) <= epsilon) {
       n1=normal(s1[15],s1[11],s1[7],s1[3],s1[4],s1[8],s1[12]);
       if(abs2(n1) <= epsilon)
         n1=normal(s1[12],s1[13],s1[14],s1[15],s1[2],s1[1],s1[0]);
     }

     // A kludge to remove subdivision cracks, only applied the first time
     // an edge is found to be flat before the rest of the subpatch is.

     triple m0=0.5*(P0+P1);
     if(!flat0) {
       if((flat0=Straightness(p0,p[4],p[8],p12) < res2))
         m0 -= Epsilon*unit(differential(s1[0],s1[1],s1[2],s1[3]));
       else m0=s0[12];
     }

     triple m1=0.5*(P2+P3);
     if(!flat2) {
       if((flat2=Straightness(p15,p[11],p[7],p3) < res2))
         m1 -= Epsilon*unit(differential(s0[15],s0[14],s0[13],s0[12]));
       else m1=s1[3];
     }

     if(color) {
       GLfloat c0[4],c1[4];
       for(size_t i=0; i < 4; ++i) {
         c0[i]=0.5*(C0[i]+C1[i]);
         c1[i]=0.5*(C2[i]+C3[i]);
       }

       GLuint i0=data.Vertex(m0,n0,c0);
       GLuint i1=data.Vertex(m1,n1,c1);

       render(s0,I0,i0,i1,I3,P0,m0,m1,P3,flat0,false,flat2,flat3,C0,c0,c1,C3);
       render(s1,i0,I1,I2,i1,m0,P1,P2,m1,flat0,flat1,flat2,false,c0,C1,C2,c1);
     } else {
       GLuint i0=(data.*pvertex)(m0,n0);
       GLuint i1=(data.*pvertex)(m1,n1);

       render(s0,I0,i0,i1,I3,P0,m0,m1,P3,flat0,false,flat2,flat3);
       render(s1,i0,I1,I2,i1,m0,P1,P2,m1,flat0,flat1,flat2,false);
     }
     return;
   }
   /*
       Horizontal and vertical subdivision:
       P refers to a corner
       m refers to a midpoint
       s refers to a subpatch

                m2
       +--------+--------+
       |P3      |      P2|
       |        |        |
       |   s3   |   s2   |
       |        |        |
       |        | m4     |
    m3 +--------+--------+ m1
       |        |        |
       |        |        |
       |   s0   |   s1   |
       |        |        |
       |P0      |      P1|
       +--------+--------+
                m0
    */
    
    // Subdivide patch:
    Split3 c0(p0,p[1],p[2],p3);
    Split3 c1(p[4],p[5],p[6],p[7]);
    Split3 c2(p[8],p[9],p[10],p[11]);
    Split3 c3(p12,p[13],p[14],p15);

    Split3 c4(p0,p[4],p[8],p12);
    Split3 c5(c0.m0,c1.m0,c2.m0,c3.m0);
    Split3 c6(c0.m3,c1.m3,c2.m3,c3.m3);
    Split3 c7(c0.m5,c1.m5,c2.m5,c3.m5);
    Split3 c8(c0.m4,c1.m4,c2.m4,c3.m4);
    Split3 c9(c0.m2,c1.m2,c2.m2,c3.m2);
    Split3 c10(p3,p[7],p[11],p15);

    triple s0[]={p0,c0.m0,c0.m3,c0.m5,c4.m0,c5.m0,c6.m0,c7.m0,
                 c4.m3,c5.m3,c6.m3,c7.m3,c4.m5,c5.m5,c6.m5,c7.m5};
    triple s1[]={c4.m5,c5.m5,c6.m5,c7.m5,c4.m4,c5.m4,c6.m4,c7.m4,
                 c4.m2,c5.m2,c6.m2,c7.m2,p12,c3.m0,c3.m3,c3.m5};
    triple s2[]={c7.m5,c8.m5,c9.m5,c10.m5,c7.m4,c8.m4,c9.m4,c10.m4,
                 c7.m2,c8.m2,c9.m2,c10.m2,c3.m5,c3.m4,c3.m2,p15};
    triple s3[]={c0.m5,c0.m4,c0.m2,p3,c7.m0,c8.m0,c9.m0,c10.m0,
                 c7.m3,c8.m3,c9.m3,c10.m3,c7.m5,c8.m5,c9.m5,c10.m5};
      
    triple m4=s0[15];
      
    triple n0=normal(s0[0],s0[4],s0[8],s0[12],s0[13],s0[14],s0[15]);
    if(abs2(n0) <= epsilon) {
      n0=normal(s0[0],s0[4],s0[8],s0[12],s0[11],s0[7],s0[3]);
      if(abs2(n0) <= epsilon)
        n0=normal(s0[3],s0[2],s0[1],s0[0],s0[13],s0[14],s0[15]);
    }
      
    triple n1=normal(s1[12],s1[13],s1[14],s1[15],s1[11],s1[7],s1[3]);
    if(abs2(n1) <= epsilon) {
      n1=normal(s1[12],s1[13],s1[14],s1[15],s1[2],s1[1],s1[0]);
      if(abs2(n1) <= epsilon)
        n1=normal(s1[0],s1[4],s1[8],s1[12],s1[11],s1[7],s1[3]);
    }
      
    triple n2=normal(s2[15],s2[11],s2[7],s2[3],s2[2],s2[1],s2[0]);
    if(abs2(n2) <= epsilon) {
      n2=normal(s2[15],s2[11],s2[7],s2[3],s2[4],s2[8],s2[12]);
      if(abs2(n2) <= epsilon)
        n2=normal(s2[12],s2[13],s2[14],s2[15],s2[2],s2[1],s2[0]);
    }
      
    triple n3=normal(s3[3],s3[2],s3[1],s3[0],s3[4],s3[8],s3[12]);
    if(abs2(n3) <= epsilon) {
      n3=normal(s3[3],s3[2],s3[1],s3[0],s3[13],s3[14],s3[15]);
      if(abs2(n3) <= epsilon)
        n3=normal(s3[15],s3[11],s3[7],s3[3],s3[4],s3[8],s3[12]);
    }
      
    triple n4=normal(s2[3],s2[2],s2[1],m4,s2[4],s2[8],s2[12]);
      
    // A kludge to remove subdivision cracks, only applied the first time
    // an edge is found to be flat before the rest of the subpatch is.
      
    triple m0=0.5*(P0+P1);
    if(!flat0) {
      if((flat0=Straightness(p0,p[4],p[8],p12) < res2))
        m0 -= Epsilon*unit(differential(s1[0],s1[1],s1[2],s1[3]));
      else m0=s0[12];
    }
      
    triple m1=0.5*(P1+P2);
    if(!flat1) {
      if((flat1=Straightness(p12,p[13],p[14],p15) < res2))
        m1 -= Epsilon*unit(differential(s2[12],s2[8],s2[4],s2[0]));
      else m1=s1[15];
    }
      
    triple m2=0.5*(P2+P3);
    if(!flat2) {
      if((flat2=Straightness(p15,p[11],p[7],p3) < res2))
        m2 -= Epsilon*unit(differential(s3[15],s3[14],s3[13],s3[12]));
      else m2=s2[3];
    }
      
    triple m3=0.5*(P3+P0);
    if(!flat3) {
      if((flat3=Straightness(p0,p[1],p[2],p3) < res2))
        m3 -= Epsilon*unit(differential(s0[3],s0[7],s0[11],s0[15]));
      else m3=s3[0];
    }
      
    if(color) {
      GLfloat c0[4],c1[4],c2[4],c3[4],c4[4];
      for(size_t i=0; i < 4; ++i) {
        c0[i]=0.5*(C0[i]+C1[i]);
        c1[i]=0.5*(C1[i]+C2[i]);
        c2[i]=0.5*(C2[i]+C3[i]);
        c3[i]=0.5*(C3[i]+C0[i]);
        c4[i]=0.5*(c0[i]+c2[i]);
      }
      
      GLuint i0=data.Vertex(m0,n0,c0);
      GLuint i1=data.Vertex(m1,n1,c1);
      GLuint i2=data.Vertex(m2,n2,c2);
      GLuint i3=data.Vertex(m3,n3,c3);
      GLuint i4=data.Vertex(m4,n4,c4);
      
      render(s0,I0,i0,i4,i3,P0,m0,m4,m3,flat0,false,false,flat3,C0,c0,c4,c3);
      render(s1,i0,I1,i1,i4,m0,P1,m1,m4,flat0,flat1,false,false,c0,C1,c1,c4);
      render(s2,i4,i1,I2,i2,m4,m1,P2,m2,false,flat1,flat2,false,c4,c1,C2,c2);
      render(s3,i3,i4,i2,I3,m3,m4,m2,P3,false,false,flat2,flat3,c3,c4,c2,C3);
    } else {
      GLuint i0=(data.*pvertex)(m0,n0);
      GLuint i1=(data.*pvertex)(m1,n1);
      GLuint i2=(data.*pvertex)(m2,n2);
      GLuint i3=(data.*pvertex)(m3,n3);
      GLuint i4=(data.*pvertex)(m4,n4);
      
      render(s0,I0,i0,i4,i3,P0,m0,m4,m3,flat0,false,false,flat3);
      render(s1,i0,I1,i1,i4,m0,P1,m1,m4,flat0,flat1,false,false);
      render(s2,i4,i1,I2,i2,m4,m1,P2,m2,false,flat1,flat2,false);
      render(s3,i3,i4,i2,I3,m3,m4,m2,P3,false,false,flat2,flat3);
    }
  }
}

void BezierTriangle::render(const triple *p, bool straight, GLfloat *c0)
{
  triple p0=p[0];
  epsilon=0;
  for(int i=1; i < 10; ++i)
    epsilon=max(epsilon,abs2(p[i]-p0));
  
  epsilon *= DBL_EPSILON;
    
  triple p6=p[6];
  triple p9=p[9];
    
  triple n0=normal(p9,p[5],p[2],p0,p[1],p[3],p6);
  triple n1=normal(p0,p[1],p[3],p6,p[7],p[8],p9);    
  triple n2=normal(p6,p[7],p[8],p9,p[5],p[2],p0);
    
  GLuint i0,i1,i2;
  if(color) {
    GLfloat *c1=c0+4;
    GLfloat *c2=c0+8;
    
    i0=data.Vertex(p0,n0,c0);
    i1=data.Vertex(p6,n1,c1);
    i2=data.Vertex(p9,n2,c2);
    
    if(!straight)
      render(p,i0,i1,i2,p0,p6,p9,false,false,false,c0,c1,c2);
  } else {
    i0=(data.*pvertex)(p0,n0);
    i1=(data.*pvertex)(p6,n1);
    i2=(data.*pvertex)(p9,n2);
    
    if(!straight)
      render(p,i0,i1,i2,p0,p6,p9,false,false,false);
  }
    
  if(straight) {
    triple P[]={p0,p6,p9};
    if(!offscreen(3,P)) {
      std::vector<GLuint> &q=data.indices;
      q.push_back(i0);
      q.push_back(i1);
      q.push_back(i2);
    }
  }
  append();
}

// Use a uniform partition to draw a Bezier triangle.
// p is an array of 10 triples representing the control points.
// Pi are the (possibly) adjusted vertices indexed by Ii.
// The 'flati' are flatness flags for each boundary.
void BezierTriangle::render(const triple *p,
                            GLuint I0, GLuint I1, GLuint I2,
                            triple P0, triple P1, triple P2,
                            bool flat0, bool flat1, bool flat2,
                            GLfloat *C0, GLfloat *C1, GLfloat *C2)
{
  if(Distance(p) < res2) { // Bezier triangle is flat
    triple P[]={P0,P1,P2};
    if(!offscreen(3,P)) {
      std::vector<GLuint> &q=data.indices;
      q.push_back(I0);
      q.push_back(I1);
      q.push_back(I2);
    }
  } else { // Triangle is not flat
    if(offscreen(10,p)) return;
    /* Control points are indexed as follows:

       Coordinate
        Index

                                  030
                                   9
                                   /\
                                  /  \
                                 /    \
                                /      \
                               /        \
                          021 +          + 120
                           5 /            \ 8
                            /              \
                           /                \
                          /                  \
                         /                    \
                    012 +          +           + 210
                     2 /          111           \ 7
                      /            4             \
                     /                            \
                    /                              \
                   /                                \
                  /__________________________________\
                003         102           201        300
                 0           1             3          6


       Subdivision:
                                   P2
                                   030
                                   /\
                                  /  \
                                 /    \
                                /      \
                               /        \
                              /    up    \
                             /            \
                            /              \
                        p1 /________________\ p0
                          /\               / \
                         /  \             /   \
                        /    \           /     \
                       /      \  center /       \
                      /        \       /         \
                     /          \     /           \
                    /    left    \   /    right    \
                   /              \ /               \
                  /________________V_________________\
                003               p2                300
                P0                                    P1
    */

    // Subdivide triangle:
    triple l003=p[0];
    triple p102=p[1];
    triple p012=p[2];
    triple p201=p[3];
    triple p111=p[4];
    triple p021=p[5];
    triple r300=p[6];
    triple p210=p[7];
    triple p120=p[8];
    triple u030=p[9];

    triple u021=0.5*(u030+p021);
    triple u120=0.5*(u030+p120);

    triple p033=0.5*(p021+p012);
    triple p231=0.5*(p120+p111);
    triple p330=0.5*(p120+p210);

    triple p123=0.5*(p012+p111);

    triple l012=0.5*(p012+l003);
    triple p312=0.5*(p111+p201);
    triple r210=0.5*(p210+r300);

    triple l102=0.5*(l003+p102);
    triple p303=0.5*(p102+p201);
    triple r201=0.5*(p201+r300);

    triple u012=0.5*(u021+p033);
    triple u210=0.5*(u120+p330);
    triple l021=0.5*(p033+l012);
    triple p4xx=0.5*p231+0.25*(p111+p102);
    triple r120=0.5*(p330+r210);
    triple px4x=0.5*p123+0.25*(p111+p210);
    triple pxx4=0.25*(p021+p111)+0.5*p312;
    triple l201=0.5*(l102+p303);
    triple r102=0.5*(p303+r201);

    triple l210=0.5*(px4x+l201); // =c120
    triple r012=0.5*(px4x+r102); // =c021
    triple l300=0.5*(l201+r102); // =r003=c030

    triple r021=0.5*(pxx4+r120); // =c012
    triple u201=0.5*(u210+pxx4); // =c102
    triple r030=0.5*(u210+r120); // =u300=c003

    triple u102=0.5*(u012+p4xx); // =c201
    triple l120=0.5*(l021+p4xx); // =c210
    triple l030=0.5*(u012+l021); // =u003=c300

    triple l111=0.5*(p123+l102);
    triple r111=0.5*(p312+r210);
    triple u111=0.5*(u021+p231);
    triple c111=0.25*(p033+p330+p303+p111);

    triple l[]={l003,l102,l012,l201,l111,l021,l300,l210,l120,l030}; // left
    triple r[]={l300,r102,r012,r201,r111,r021,r300,r210,r120,r030}; // right
    triple u[]={l030,u102,u012,u201,u111,u021,r030,u210,u120,u030}; // up
    triple c[]={r030,u201,r021,u102,c111,r012,l030,l120,l210,l300}; // center

    triple n0=normal(l300,r012,r021,r030,u201,u102,l030);
    triple n1=normal(r030,u201,u102,l030,l120,l210,l300);
    triple n2=normal(l030,l120,l210,l300,r012,r021,r030);
          
    // A kludge to remove subdivision cracks, only applied the first time
    // an edge is found to be flat before the rest of the subpatch is.
    
    triple m0=0.5*(P1+P2);
    if(!flat0) {
      if((flat0=Straightness(r300,p210,p120,u030) < res2))
        m0 -= Epsilon*unit(differential(c[0],c[2],c[5],c[9])+
                           differential(c[0],c[1],c[3],c[6]));
      else m0=r030;
    }

    triple m1=0.5*(P2+P0);
    if(!flat1) {
      if((flat1=Straightness(l003,p012,p021,u030) < res2))
        m1 -= Epsilon*unit(differential(c[6],c[3],c[1],c[0])+
                           differential(c[6],c[7],c[8],c[9]));
      else m1=l030;
    }

    triple m2=0.5*(P0+P1);
    if(!flat2) {
      if((flat2=Straightness(l003,p102,p201,r300) < res2))
        m2 -= Epsilon*unit(differential(c[9],c[8],c[7],c[6])+
                           differential(c[9],c[5],c[2],c[0]));
      else m2=l300;
    }

    if(color) {
      GLfloat c0[4],c1[4],c2[4];
      for(int i=0; i < 4; ++i) {
        c0[i]=0.5*(C1[i]+C2[i]);
        c1[i]=0.5*(C2[i]+C0[i]);
        c2[i]=0.5*(C0[i]+C1[i]);
      }
      
      GLuint i0=data.Vertex(m0,n0,c0);
      GLuint i1=data.Vertex(m1,n1,c1);
      GLuint i2=data.Vertex(m2,n2,c2);
          
      render(l,I0,i2,i1,P0,m2,m1,false,flat1,flat2,C0,c2,c1);
      render(r,i2,I1,i0,m2,P1,m0,flat0,false,flat2,c2,C1,c0);
      render(u,i1,i0,I2,m1,m0,P2,flat0,flat1,false,c1,c0,C2);
      render(c,i0,i1,i2,m0,m1,m2,false,false,false,c0,c1,c2);
    } else {
      GLuint i0=(data.*pvertex)(m0,n0);
      GLuint i1=(data.*pvertex)(m1,n1);
      GLuint i2=(data.*pvertex)(m2,n2);
          
      render(l,I0,i2,i1,P0,m2,m1,false,flat1,flat2);
      render(r,i2,I1,i0,m2,P1,m0,flat0,false,flat2);
      render(u,i1,i0,I2,m1,m0,P2,flat0,flat1,false);
      render(c,i0,i1,i2,m0,m1,m2,false,false,false);
    }
  }
}

void transform(const std::vector<VertexData>& b)
{
  unsigned n=b.size();
//  xbuffer.resize(n);
//  ybuffer.resize(n);
  zbuffer.resize(n);
  
  double Tz0=gl::dView[2];
  double Tz1=gl::dView[6];
  double Tz2=gl::dView[10];
  for(unsigned i=0; i < n; ++i) {
    const GLfloat *v=b[i].position;
    zbuffer[i]=Tz0*v[0]+Tz1*v[1]+Tz2*v[2];
  }
}

#if 0
// precompute min and max bounds of each triangle
void bounds(const std::vector<GLuint>& I)
{
  unsigned n=I.size()/3;
/*  
  xmin.resize(n);
  xmax.resize(n);
  ymin.resize(n);
  ymax.resize(n);
*/
  
  zmin.resize(n);
  zmax.resize(n);
  
  for(unsigned i=0; i < n; ++i) {
    unsigned i3=3*i;
    unsigned Ia=I[i3];
    unsigned Ib=I[i3+1];
    unsigned Ic=I[i3+2];
    
    
    /*
    double xa=xbuffer[Ia];
    double xb=xbuffer[Ib];
    double xc=xbuffer[Ic];
    
    double ya=ybuffer[Ia];
    double yb=ybuffer[Ib];
    double yc=ybuffer[Ic];
    */
    
    double za=zbuffer[Ia];
    double zb=zbuffer[Ib];
    double zc=zbuffer[Ic];
    
    /*
    xmin[i]=min(xa,xb,xc);
    xmax[i]=max(xa,xb,xc);
    
    ymin[i]=min(ya,yb,yc);
    ymax[i]=max(ya,yb,yc);
    */
    
    zmin[i]=min(za,zb,zc);
    zmax[i]=max(za,zb,zc);
  }
}
#endif
  
void sortTriangles()
{
  if(!transparentData.indices.empty()) {
    transform(transparentData.Vertices);
//    bounds(tIndices);
    qsort(&transparentData.indices[0],transparentData.indices.size()/3,
          3*sizeof(GLuint),compare);
  }
}

void Triangles::queue(size_t nP, const triple* P, size_t nN, const triple* N,
                      size_t nC, const prc::RGBAColour* C, size_t nI,
                      const uint32_t (*PP)[3], const uint32_t (*NN)[3],
                      const uint32_t (*CC)[3], bool Transparent)
{
  if(!nN) return;
  
  data.clear();
  Onscreen=true;
  transparent=Transparent;
  
  data.Vertices.resize(nP);
  data.indices.resize(3*nI);
  
  MaterialIndex=nC ? -1-materialIndex : 1+materialIndex; 
        
  for(size_t i=0; i < nI; ++i) {
    const uint32_t *PI=PP[i];
    uint32_t PI0=PI[0];
    uint32_t PI1=PI[1];
    uint32_t PI2=PI[2];
    triple P0=P[PI0];
    triple P1=P[PI1];
    triple P2=P[PI2];
    const uint32_t *NI=NN[i];
    if(nC) {
      const uint32_t *CI=CC[i];
      prc::RGBAColour C0=C[CI[0]];
      prc::RGBAColour C1=C[CI[1]];
      prc::RGBAColour C2=C[CI[2]];
      GLfloat c0[]={(GLfloat) C0.R,(GLfloat) C0.G,(GLfloat) C0.B,
                    (GLfloat) C0.A};
      GLfloat c1[]={(GLfloat) C1.R,(GLfloat) C1.G,(GLfloat) C1.B,
                    (GLfloat) C1.A};
      GLfloat c2[]={(GLfloat) C2.R,(GLfloat) C2.G,(GLfloat) C2.B,
                    (GLfloat) C2.A};
      transparent |= c0[3]+c1[3]+c2[3] < 765;
      data.Vertices[PI0]=VertexData(P0,N[NI[0]],c0);
      data.Vertices[PI1]=VertexData(P1,N[NI[1]],c1);
      data.Vertices[PI2]=VertexData(P2,N[NI[2]],c2);
    } else {
      data.Vertices[PI0]=VertexData(P0,N[NI[0]]);
      data.Vertices[PI1]=VertexData(P1,N[NI[1]]);
      data.Vertices[PI2]=VertexData(P2,N[NI[2]]);
    }
    triple Q[]={P0,P1,P2};
    if(!offscreen(3,Q)) {
      size_t i3=3*i;
      data.indices[i3]=PI0;
      data.indices[i3+1]=PI1;
      data.indices[i3+2]=PI2;
    }
  }
  append();
}
 
#endif

} //namespace camp
