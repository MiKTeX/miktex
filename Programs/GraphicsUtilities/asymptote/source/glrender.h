/*****
 * glrender.h
 * Render 3D Bezier paths and surfaces.
 *****/

#ifndef GLRENDER_H
#define GLRENDER_H

#if defined(MIKTEX_WINDOWS)
//#include <Windows.h>
#endif
#include "common.h"
#include "triple.h"
#include "pen.h"

#ifdef HAVE_LIBGLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#endif

#ifdef HAVE_GL

#include <csignal>

#define GLEW_NO_GLU

#ifdef __MSDOS__
#define GLEW_STATIC
#define _WIN32
#endif

#include "GL/glew.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>

#ifdef HAVE_LIBGLUT
#include <GLUT/glut.h>
#ifndef GLUT_3_2_CORE_PROFILE
#undef HAVE_GL
#endif

#endif

#ifdef HAVE_LIBOSMESA
#include <GL/osmesa.h>
#endif

#else

#ifdef __MSDOS__
#undef _WIN32
#include <GL/gl.h>
#include <GL/wglew.h>
#include <GL/wglext.h>
#endif

#ifdef HAVE_LIBGLUT
#include <GL/glut.h>
#endif

#ifdef HAVE_LIBOSMESA
#include <GL/osmesa.h>
#endif

#endif

#else
typedef unsigned int GLuint;
typedef int GLint;
typedef float GLfloat;
typedef double GLdouble;
typedef unsigned char GLubyte;
typedef unsigned int GLenum;
#define GL_POINTS				0x0000
#define GL_LINES				0x0001
#define GL_TRIANGLES				0x0004
#endif

#ifdef HAVE_LIBGLM
#include "material.h"
#endif

namespace camp {
class picture;

inline void store(GLfloat *f, double *C)
{
  f[0]=C[0];
  f[1]=C[1];
  f[2]=C[2];
}

inline void store(GLfloat *control, const camp::triple& v)
{
  control[0]=v.getx();
  control[1]=v.gety();
  control[2]=v.getz();
}

inline void store(GLfloat *control, const triple& v, double weight)
{
  control[0]=v.getx()*weight;
  control[1]=v.gety()*weight;
  control[2]=v.getz()*weight;
  control[3]=weight;
}

}

namespace gl {

extern bool outlinemode;
extern bool wireframeMode;

extern bool orthographic;

// 2D bounds
extern double xmin,xmax;
extern double ymin,ymax;

// 3D bounds
extern double Xmin,Xmax;
extern double Ymin,Ymax;
extern double Zmin,Zmax;

extern int fullWidth,fullHeight;
extern double Zoom0;
extern double Angle;
extern camp::pair Shift;
extern camp::pair Margin;

extern camp::triple *Lights;
extern size_t nlights;
extern double *Diffuse;
extern double Background[4];

struct projection
{
public:
  bool orthographic;
  camp::triple camera;
  camp::triple up;
  camp::triple target;
  double zoom;
  double angle;
  camp::pair viewportshift;

  projection(bool orthographic=false, camp::triple camera=0.0,
             camp::triple up=0.0, camp::triple target=0.0,
             double zoom=0.0, double angle=0.0,
             camp::pair viewportshift=0.0) :
    orthographic(orthographic), camera(camera), up(up), target(target),
    zoom(zoom), angle(angle), viewportshift(viewportshift) {}
};

#ifdef HAVE_GL
extern GLuint ubo;
GLuint initHDR();
#endif

projection camera(bool user=true);

void glrender(const string& prefix, const camp::picture* pic,
              const string& format, double width, double height, double angle,
              double zoom, const camp::triple& m, const camp::triple& M,
              const camp::pair& shift, const camp::pair& margin, double *t,
              double *background, size_t nlights, camp::triple *lights,
              double *diffuse, double *specular, bool view, int oldpid=0);

extern const double *dprojView;
extern const double *dView;

extern double BBT[9];
extern double T[16];

extern bool format3dWait;

}

namespace camp {

struct Billboard {
  double cx,cy,cz;

  void init(const triple& center) {
    cx=center.getx();
    cy=center.gety();
    cz=center.getz();
  }

  triple transform(const triple& v) const {
    double x=v.getx()-cx;
    double y=v.gety()-cy;
    double z=v.getz()-cz;

    return triple(x*gl::BBT[0]+y*gl::BBT[3]+z*gl::BBT[6]+cx,
                  x*gl::BBT[1]+y*gl::BBT[4]+z*gl::BBT[7]+cy,
                  x*gl::BBT[2]+y*gl::BBT[5]+z*gl::BBT[8]+cz);
  }
};

extern Billboard BB;

#ifdef HAVE_LIBGLM
typedef mem::map<CONST Material,size_t> MaterialMap;

extern std::vector<Material> materials;
extern MaterialMap materialMap;
extern size_t materialIndex;
extern int MaterialIndex;

extern const size_t Nbuffer; // Initial size of 2D dynamic buffers
extern const size_t nbuffer; // Initial size of 0D & 1D dynamic buffers

class vertexData
{
public:
  GLfloat position[3];
  GLfloat normal[3];
  GLint material;
  vertexData() {};
  vertexData(const triple& v, const triple& n) {
    position[0]=v.getx();
    position[1]=v.gety();
    position[2]=v.getz();
    normal[0]=n.getx();
    normal[1]=n.gety();
    normal[2]=n.getz();
    material=MaterialIndex;
  }
};

class VertexData
{
public:
  GLfloat position[3];
  GLfloat normal[3];
  GLint material;
  GLfloat color[4];
  VertexData() {};
  VertexData(const triple& v, const triple& n) {
    position[0]=v.getx();
    position[1]=v.gety();
    position[2]=v.getz();
    normal[0]=n.getx();
    normal[1]=n.gety();
    normal[2]=n.getz();
    material=MaterialIndex;
  }
  VertexData(const triple& v, const triple& n, GLfloat *c) {
    position[0]=v.getx();
    position[1]=v.gety();
    position[2]=v.getz();
    normal[0]=n.getx();
    normal[1]=n.gety();
    normal[2]=n.getz();
    material=MaterialIndex;
    color[0]=c[0];
    color[1]=c[1];
    color[2]=c[2];
    color[3]=c[3];
  }
};

class vertexData0 {
public:
  GLfloat position[3];
  GLfloat width;
  GLint material;
  vertexData0() {};
  vertexData0(const triple& v, double width) : width(width) {
    position[0]=v.getx();
    position[1]=v.gety();
    position[2]=v.getz();
    material=MaterialIndex;
  }
};

class vertexBuffer {
public:
  GLenum type;

  GLuint verticesBuffer;
  GLuint VerticesBuffer;
  GLuint vertices0Buffer;
  GLuint indicesBuffer;
  GLuint materialsBuffer;

  std::vector<vertexData> vertices;
  std::vector<VertexData> Vertices;
  std::vector<vertexData0> vertices0;
  std::vector<GLuint> indices;

  std::vector<Material> materials;
  std::vector<GLint> materialTable;

  bool rendered; // Are all patches in this buffer fully rendered?
  bool partial;  // Does buffer contain incomplete data?

  vertexBuffer(GLint type=GL_TRIANGLES) : type(type),
                                          verticesBuffer(0),
                                          VerticesBuffer(0),
                                          vertices0Buffer(0),
                                          indicesBuffer(0),
                                          materialsBuffer(0),
                                          rendered(false),
                                          partial(false)
  {}

  void clear() {
    vertices.clear();
    Vertices.clear();
    vertices0.clear();
    indices.clear();
    materials.clear();
    materialTable.clear();
  }

  void reserve0() {
    vertices0.reserve(nbuffer);
  }

  void reserve() {
    vertices.reserve(Nbuffer);
    indices.reserve(Nbuffer);
  }

  void Reserve() {
    Vertices.reserve(Nbuffer);
    indices.reserve(Nbuffer);
  }

// Store the vertex v and its normal vector n.
  GLuint vertex(const triple &v, const triple& n) {
    size_t nvertices=vertices.size();
    vertices.push_back(vertexData(v,n));
    return nvertices;
  }

// Store the vertex v and its normal vector n, without an explicit color.
  GLuint tvertex(const triple &v, const triple& n) {
    size_t nvertices=Vertices.size();
    Vertices.push_back(VertexData(v,n));
    return nvertices;
  }

// Store the vertex v, its normal vector n, and colors c.
  GLuint Vertex(const triple &v, const triple& n, GLfloat *c) {
    size_t nvertices=Vertices.size();
    Vertices.push_back(VertexData(v,n,c));
    return nvertices;
  }

// Store the pixel v and its width.
  GLuint vertex0(const triple &v, double width) {
    size_t nvertices=vertices0.size();
    vertices0.push_back(vertexData0(v,width));
    return nvertices;
  }

  // append array b onto array a with offset
  void appendOffset(std::vector<GLuint>& a,
                    const std::vector<GLuint>& b, size_t offset) {
    size_t n=a.size();
    size_t m=b.size();
    a.resize(n+m);
    for(size_t i=0; i < m; ++i)
      a[n+i]=b[i]+offset;
  }

  void append(const vertexBuffer& b) {
    appendOffset(indices,b.indices,vertices.size());
    vertices.insert(vertices.end(),b.vertices.begin(),b.vertices.end());
  }

  void Append(const vertexBuffer& b) {
    appendOffset(indices,b.indices,Vertices.size());
    Vertices.insert(Vertices.end(),b.Vertices.begin(),b.Vertices.end());
  }

  void append0(const vertexBuffer& b) {
    appendOffset(indices,b.indices,vertices0.size());
    vertices0.insert(vertices0.end(),b.vertices0.begin(),b.vertices0.end());
  }
};

extern vertexBuffer material0Data;   // pixels
extern vertexBuffer material1Data;   // material Bezier curves
extern vertexBuffer materialData;    // material Bezier patches & triangles
extern vertexBuffer colorData;       // colored Bezier patches & triangles
extern vertexBuffer triangleData;    // opaque indexed triangles
extern vertexBuffer transparentData; // transparent patches & triangles

void drawBuffer(vertexBuffer& data, GLint shader, bool color=false);
void drawBuffers();

void clearMaterials();
void clearCenters();

typedef void draw_t();
void setMaterial(vertexBuffer& data, draw_t *draw);

void drawMaterial0();
void drawMaterial1();
void drawMaterial();
void drawColor();
void drawTriangle();
void drawTransparent();

#endif

}

#endif
