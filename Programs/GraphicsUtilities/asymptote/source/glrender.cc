/*****
 * glrender.cc
 * John Bowman, Orest Shardt, and Supakorn "Jamie" Rassameemasmuang
 * Render 3D Bezier paths and surfaces.
 *****/

#ifdef __CYGWIN__
#define _POSIX_C_SOURCE 200809L
#endif

#if defined(MIKTEX)
#include <miktex/asy-first.h>
#include <miktex/asy.h>
#include <chrono>
#endif
#include <stdlib.h>
#include <fstream>
#include <cstring>
#include <sys/time.h>

#include "common.h"
#include "locate.h"
#include "seconds.h"
#include "statistics.h"
#include "bezierpatch.h"
#include "beziercurve.h"

#include "picture.h"
#include "bbox3.h"
#include "drawimage.h"
#include "interact.h"
#include "fpu.h"

extern uint32_t CLZ(uint32_t a);

bool GPUindexing;
bool GPUcompress;

namespace gl {
#ifdef HAVE_PTHREAD
pthread_t mainthread;
#endif
}

#ifdef HAVE_GL
#include "tr.h"

#ifdef HAVE_LIBGLUT

#ifdef __MSDOS__
#ifndef FGAPI
#define FGAPI GLUTAPI
#endif
#ifndef FGAPIENTRY
#define FGAPIENTRY APIENTRY
#endif
#endif

#define GLUT_BUILDING_LIB

#ifdef FREEGLUT
#include <GL/freeglut_ext.h>
#endif

#endif // HAVE_LIBGLUT

#include "shaders.h"
#include "GLTextures.h"
#include "EXRFiles.h"

#ifdef HAVE_LIBOPENIMAGEIO
#include <OpenImageIO/imageio.h>
#endif

using settings::locateFile;
using utils::seconds;

#endif // HAVE_GL

#ifdef HAVE_LIBGLM

namespace camp {
Billboard BB;

GLint pixelShader;
GLint materialShader[2];
GLint colorShader[2];
GLint generalShader[2];
GLint countShader;
GLint transparentShader;
GLint blendShader;
GLint zeroShader;
GLint compressShader;
GLint sum1Shader;
GLint sum2Shader;
GLint sum3Shader;

GLuint offsetBuffer;
GLuint indexBuffer;
GLuint elementsBuffer;
GLuint countBuffer;
GLuint localSumBuffer;
GLuint globalSumBuffer;
GLuint fragmentBuffer;
GLuint depthBuffer;
GLuint opaqueBuffer;
GLuint opaqueDepthBuffer;

bool ssbo;
bool interlock;
}

#endif

#ifdef HAVE_LIBGLM
using camp::Material;
using camp::Maxmaterials;
using camp::Nmaterials;
using camp::nmaterials;
using camp::MaterialMap;

namespace camp {
bool initSSBO;
GLuint maxFragments;

vertexBuffer material0Data(GL_POINTS);
vertexBuffer material1Data(GL_LINES);
vertexBuffer materialData;
vertexBuffer colorData;
vertexBuffer transparentData;
vertexBuffer triangleData;

const size_t Nbuffer=10000;
const size_t nbuffer=1000;

std::vector<Material> materials;
MaterialMap materialMap;
size_t materialIndex;

size_t Maxmaterials;
size_t Nmaterials=1;
size_t nmaterials=48;
unsigned int Opaque=0;

void clearCenters()
{
  camp::drawElement::centers.clear();
  camp::drawElement::centermap.clear();
}

void clearMaterials()
{
  materials.clear();
  materials.reserve(nmaterials);
  materialMap.clear();

  material0Data.partial=false;
  material1Data.partial=false;
  materialData.partial=false;
  colorData.partial=false;
  triangleData.partial=false;
  transparentData.partial=false;
}

}

extern void exitHandler(int);

namespace gl {

GLint gs2;
GLint gs;
GLint g;
GLuint processors;
GLuint localsize;
GLuint lastlocalsize;
GLint maxgroups;
GLuint maxSize;
GLuint lastSize;

bool outlinemode=false;
bool ibl=false;
bool glthread=false;
bool glupdate=false;
bool glexit=false;
bool initialize=true;

using camp::picture;
using camp::drawRawImage;
using camp::transform;
using camp::pair;
using camp::triple;
using vm::array;
using vm::read;
using camp::bbox3;
using settings::getSetting;
using settings::Setting;

bool Iconify=false;
bool ignorezoom;
int Fitscreen=1;
bool firstFit;

bool queueExport=false;
bool readyAfterExport=false;
bool remesh;
bool copied;

int Mode;

double Aspect;
bool View;
bool ViewExport;
int Oldpid;
string Prefix;
const picture* Picture;
string Format;
int fullWidth,fullHeight;
int Width,Height;
GLuint pixels;
GLuint elements;
GLuint lastpixels;
double oWidth,oHeight;
int screenWidth,screenHeight;
int maxTileWidth;
int maxTileHeight;

double Angle;
bool orthographic;
double H;

double xmin,xmax;
double ymin,ymax;

double Xmin,Xmax;
double Ymin,Ymax;
double Zmin,Zmax;

pair Shift;
pair Margin;
double X,Y;
int x0,y0;
double cx,cy;
double Xfactor,Yfactor;
double ArcballFactor;

static const double pi=acos(-1.0);
static const double degrees=180.0/pi;
static const double radians=1.0/degrees;

double Background[4];

size_t Nlights=1; // Maximum number of lights compiled in shader
size_t nlights; // Actual number of lights
size_t nlights0;
triple *Lights;
double *Diffuse;
double *Specular;
bool antialias;

double Zoom;
double Zoom0;
double lastzoom;

GLint lastshader=-1;

bool format3dWait=false;

using glm::dvec3;
using glm::dmat3;
using glm::mat3;
using glm::mat4;
using glm::dmat4;
using glm::value_ptr;
using glm::translate;

using camp::interlock;
using camp::ssbo;

mat3 normMat;
dmat3 dnormMat;

mat4 projViewMat;
mat4 viewMat;

dmat4 dprojMat;
dmat4 dprojViewMat;
dmat4 dviewMat;
dmat4 drotateMat;

const double *dprojView;
const double *dView;
double BBT[9];

unsigned int framecount;

template<class T>
inline T min(T a, T b)
{
  return (a < b) ? a : b;
}

template<class T>
inline T max(T a, T b)
{
  return (a > b) ? a : b;
}

glm::vec4 vec4(triple v)
{
  return glm::vec4(v.getx(),v.gety(),v.getz(),0);
}

glm::vec4 vec4(double *v)
{
  return glm::vec4(v[0],v[1],v[2],v[3]);
}

void setDimensions(int Width, int Height, double X, double Y)
{
  double Aspect=((double) Width)/Height;
  double xshift=(X/Width+Shift.getx()*Xfactor)*Zoom;
  double yshift=(Y/Height+Shift.gety()*Yfactor)*Zoom;
  double Zoominv=1.0/Zoom;
  if(orthographic) {
    double xsize=Xmax-Xmin;
    double ysize=Ymax-Ymin;
    if(xsize < ysize*Aspect) {
      double r=0.5*ysize*Aspect*Zoominv;
      double X0=2.0*r*xshift;
      double Y0=(Ymax-Ymin)*Zoominv*yshift;
      xmin=-r-X0;
      xmax=r-X0;
      ymin=Ymin*Zoominv-Y0;
      ymax=Ymax*Zoominv-Y0;
    } else {
      double r=0.5*xsize*Zoominv/Aspect;
      double X0=(Xmax-Xmin)*Zoominv*xshift;
      double Y0=2.0*r*yshift;
      xmin=Xmin*Zoominv-X0;
      xmax=Xmax*Zoominv-X0;
      ymin=-r-Y0;
      ymax=r-Y0;
    }
  } else {
    double r=H*Zoominv;
    double rAspect=r*Aspect;
    double X0=2.0*rAspect*xshift;
    double Y0=2.0*r*yshift;
    xmin=-rAspect-X0;
    xmax=rAspect-X0;
    ymin=-r-Y0;
    ymax=r-Y0;
  }
}

void updateProjection()
{
  dprojViewMat=dprojMat*dviewMat;
  projViewMat=mat4(dprojViewMat);
  dprojView=value_ptr(dprojViewMat);
}

void frustum(GLdouble left, GLdouble right, GLdouble bottom,
             GLdouble top, GLdouble nearVal, GLdouble farVal)
{
  dprojMat=glm::frustum(left,right,bottom,top,nearVal,farVal);
  updateProjection();
}

void ortho(GLdouble left, GLdouble right, GLdouble bottom,
           GLdouble top, GLdouble nearVal, GLdouble farVal)
{
  dprojMat=glm::ortho(left,right,bottom,top,nearVal,farVal);
  updateProjection();
}

void setProjection()
{
  setDimensions(Width,Height,X,Y);
  if(orthographic) ortho(xmin,xmax,ymin,ymax,-Zmax,-Zmin);
  else frustum(xmin,xmax,ymin,ymax,-Zmax,-Zmin);
}

void updateModelViewData()
{
  // Like Fortran, OpenGL uses transposed (column-major) format!
  dnormMat=dmat3(glm::inverse(dviewMat));
  double *T=value_ptr(dnormMat);
  for(size_t i=0; i < 9; ++i)
    BBT[i]=T[i];
  normMat=mat3(dnormMat);
}

bool Xspin,Yspin,Zspin;
bool Animate;
bool Step;

#ifdef HAVE_GL

void idle()
{
  glutIdleFunc(NULL);
  Xspin=Yspin=Zspin=Animate=Step=false;
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
  MiKTeX::Aymptote::RequestHandler();
#endif
}
#endif

void home(bool webgl=false)
{
  X=Y=cx=cy=0.0;
#ifdef HAVE_GL
#ifdef HAVE_LIBGLUT
#ifndef HAVE_LIBOSMESA
  if(!webgl)
    idle();
#endif
#endif
#endif
  dviewMat=dmat4(1.0);
  if(!camp::ssbo)
    dView=value_ptr(dviewMat);
  viewMat=mat4(dviewMat);

  drotateMat=dmat4(1.0);

  updateModelViewData();

  remesh=true;
  lastzoom=Zoom=Zoom0;
  setDimensions(Width,Height,0,0);
  framecount=0;
}

double T[16];

#ifdef HAVE_GL

#ifdef HAVE_LIBGLUT
timeval lasttime;
timeval lastframetime;
int oldWidth,oldHeight;

bool queueScreen=false;

string Action;

double lastangle;
int window;
#endif

using utils::statistics;
statistics S;

GLTexture2<float,GL_FLOAT> IBLbrdfTex;
GLTexture2<float,GL_FLOAT> irradiance;
GLTexture3<float,GL_FLOAT> reflTextures;

GLTexture2<float,GL_FLOAT> fromEXR(string const& EXRFile, GLTexturesFmt const& fmt, GLint const& textureNumber)
{
  camp::IEXRFile fil(EXRFile);
  return GLTexture2<float,GL_FLOAT> {fil.getData(),fil.size(),textureNumber,fmt};
}

GLTexture3<float,GL_FLOAT> fromEXR3(
  mem::vector<string> const& EXRFiles, GLTexturesFmt const& fmt, GLint const& textureNumber)
{
  // 3d reflectance textures
  std::vector<float> data;
  size_t count=EXRFiles.size();
  int wi=0, ht=0;

  for(string const& EXRFile : EXRFiles) {
    camp::IEXRFile fil3(EXRFile);
    std::tie(wi,ht)=fil3.size();
    size_t imSize=4*wi*ht;
    std::copy(fil3.getData(),fil3.getData()+imSize,std::back_inserter(data));
  }

  return GLTexture3<float,GL_FLOAT> {data.data(),std::tuple<int,int,int>(wi,ht,count),textureNumber,fmt};
}

void initIBL()
{
  GLTexturesFmt fmt;
  fmt.internalFmt=GL_RGB16F;
  string imageDir=locateFile(getSetting<string>("imageDir"))+"/";
  string imagePath=imageDir+getSetting<string>("image")+"/";
  irradiance=fromEXR(imagePath+"diffuse.exr",fmt,1);

  GLTexturesFmt fmtRefl;
  fmtRefl.internalFmt=GL_RG16F;
  IBLbrdfTex=fromEXR(imageDir+"refl.exr",fmtRefl,2);

  GLTexturesFmt fmt3;
  fmt3.internalFmt=GL_RGB16F;
  fmt3.wrapS=GL_REPEAT;
  fmt3.wrapR=GL_CLAMP_TO_EDGE;
  fmt3.wrapT=GL_CLAMP_TO_EDGE;

  mem::vector<string> files;
  mem::string prefix=imagePath+"refl";
  for(unsigned int i=0; i <= 10; ++i) {
    mem::stringstream mss;
    mss << prefix << i << ".exr";
    files.emplace_back(mss.str());
  }

  reflTextures=fromEXR3(files,fmt3,3);
}

void *glrenderWrapper(void *a);

#ifdef HAVE_LIBOSMESA
OSMesaContext ctx;
unsigned char *osmesa_buffer;
#endif

#ifdef HAVE_PTHREAD

pthread_cond_t initSignal=PTHREAD_COND_INITIALIZER;
pthread_mutex_t initLock=PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t readySignal=PTHREAD_COND_INITIALIZER;
pthread_mutex_t readyLock=PTHREAD_MUTEX_INITIALIZER;

void endwait(pthread_cond_t& signal, pthread_mutex_t& lock)
{
  pthread_mutex_lock(&lock);
  pthread_cond_signal(&signal);
  pthread_mutex_unlock(&lock);
}
void wait(pthread_cond_t& signal, pthread_mutex_t& lock)
{
  pthread_mutex_lock(&lock);
  pthread_cond_signal(&signal);
  pthread_cond_wait(&signal,&lock);
  pthread_mutex_unlock(&lock);
}
#elif defined(MIKTEX)
std::condition_variable initSignal;
std::mutex initLock;
std::condition_variable readySignal;
std::mutex readyLock;
void endwait(std::condition_variable& cond, std::mutex& mutex)
{
  std::unique_lock<std::mutex> lock(mutex);
  cond.notify_one();
}
void wait(std::condition_variable& cond, std::mutex& mutex)
{
  std::unique_lock<std::mutex> lock(mutex);
  cond.notify_one();
  cond.wait(lock);
  if (MiKTeX::Aymptote::exitRequested)
  {
    throw 0;
  }
}
#endif

void noShaders()
{
  cerr << "GLSL shaders not found." << endl;
  exit(-1);
}

void initComputeShaders()
{
  string sum1=locateFile("shaders/sum1.glsl");
  string sum2=locateFile("shaders/sum2.glsl");
  string sum3=locateFile("shaders/sum3.glsl");

  if(sum1.empty() || sum2.empty() || sum3.empty())
    noShaders();

  std::vector<ShaderfileModePair> shaders(1);
  std::vector<std::string> shaderParams;

  shaders[0]=ShaderfileModePair(sum1.c_str(),GL_COMPUTE_SHADER);
  ostringstream s;
  s << "LOCAL_SIZE_X " << localsize << "u" << endl;
  shaderParams.push_back(s.str().c_str());
  GLuint rc=compileAndLinkShader(shaders,shaderParams,true,false,true);
  if(rc == 0) {
    GPUindexing=false; // Compute shaders are unavailable.
    if(settings::verbose > 2)
      cout << "No compute shader support" << endl;
  } else {
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,0,&maxgroups);
    maxgroups=min(1024,maxgroups/(GLint) (localsize*localsize));
    camp::sum1Shader=rc;

    shaders[0]=ShaderfileModePair(sum2.c_str(),GL_COMPUTE_SHADER);
    camp::sum2Shader=compileAndLinkShader(shaders,shaderParams,true,false,true);

    shaders[0]=ShaderfileModePair(sum3.c_str(),GL_COMPUTE_SHADER);
    camp::sum3Shader=compileAndLinkShader(shaders,shaderParams,true,false,
                                          true);
  }
}

void initBlendShader()
{
  string screen=locateFile("shaders/screen.glsl");
  string blend=locateFile("shaders/blend.glsl");

  if(screen.empty() || blend.empty())
    noShaders();

  std::vector<ShaderfileModePair> shaders(2);
  std::vector<std::string> shaderParams;

  ostringstream s;
  s << "ARRAYSIZE " << maxSize << "u" << endl;
  shaderParams.push_back(s.str().c_str());
  if(GPUindexing)
    shaderParams.push_back("GPUINDEXING");
  if(GPUcompress)
    shaderParams.push_back("GPUCOMPRESS");
  shaders[0]=ShaderfileModePair(screen.c_str(),GL_VERTEX_SHADER);
  shaders[1]=ShaderfileModePair(blend.c_str(),GL_FRAGMENT_SHADER);
  camp::blendShader=compileAndLinkShader(shaders,shaderParams,ssbo);
}

// Return the smallest power of 2 greater than or equal to n.
inline GLuint ceilpow2(GLuint n)
{
  --n;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return ++n;
}

void initShaders()
{
  Nlights=nlights == 0 ? 0 : max(Nlights,nlights);
  Nmaterials=max(Nmaterials,nmaterials);

  string zero=locateFile("shaders/zero.glsl");
  string compress=locateFile("shaders/compress.glsl");
  string vertex=locateFile("shaders/vertex.glsl");
  string count=locateFile("shaders/count.glsl");
  string fragment=locateFile("shaders/fragment.glsl");
  string screen=locateFile("shaders/screen.glsl");

  if(zero.empty() || compress.empty() || vertex.empty() || fragment.empty() ||
     screen.empty() || count.empty())
    noShaders();

  if(GPUindexing)
    initComputeShaders();

  std::vector<ShaderfileModePair> shaders(2);
  std::vector<std::string> shaderParams;

  if(ibl) {
    shaderParams.push_back("USE_IBL");
    initIBL();
  }

  shaders[0]=ShaderfileModePair(vertex.c_str(),GL_VERTEX_SHADER);

#ifdef HAVE_SSBO
  if(GPUindexing)
    shaderParams.push_back("GPUINDEXING");
  if(GPUcompress)
    shaderParams.push_back("GPUCOMPRESS");
  shaders[1]=ShaderfileModePair(count.c_str(),GL_FRAGMENT_SHADER);
  camp::countShader=compileAndLinkShader(shaders,shaderParams,true);
  if(camp::countShader)
    shaderParams.push_back("HAVE_SSBO");
#else
  camp::countShader=0;
#endif

  ssbo=camp::countShader;
#ifdef HAVE_LIBOSMESA
  interlock=false;
#else
  interlock=ssbo && getSetting<bool>("GPUinterlock");
#endif

  if(!ssbo && settings::verbose > 2)
    cout << "No SSBO support; order-independent transparency unavailable"
         << endl;

  shaders[1]=ShaderfileModePair(fragment.c_str(),GL_FRAGMENT_SHADER);
  shaderParams.push_back("MATERIAL");
  if(orthographic)
    shaderParams.push_back("ORTHOGRAPHIC");

  ostringstream lights,materials,opaque;
  lights << "Nlights " << Nlights;
  shaderParams.push_back(lights.str().c_str());
  materials << "Nmaterials " << Nmaterials;
  shaderParams.push_back(materials.str().c_str());

  shaderParams.push_back("WIDTH");
  camp::pixelShader=compileAndLinkShader(shaders,shaderParams,ssbo);
  shaderParams.pop_back();

  shaderParams.push_back("NORMAL");
  if(interlock) shaderParams.push_back("HAVE_INTERLOCK");
  camp::materialShader[0]=compileAndLinkShader(shaders,shaderParams,ssbo,
                                               interlock);
  if(interlock && !camp::materialShader[0]) {
    shaderParams.pop_back();
    interlock=false;
    camp::materialShader[0]=compileAndLinkShader(shaders,shaderParams,ssbo);
    if(settings::verbose > 2)
      cout << "No fragment shader interlock support" << endl;
  }

  shaderParams.push_back("OPAQUE");
  camp::materialShader[1]=compileAndLinkShader(shaders,shaderParams,ssbo);
  shaderParams.pop_back();

  shaderParams.push_back("COLOR");
  camp::colorShader[0]=compileAndLinkShader(shaders,shaderParams,ssbo,
                                            interlock);
  shaderParams.push_back("OPAQUE");
  camp::colorShader[1]=compileAndLinkShader(shaders,shaderParams,ssbo);
  shaderParams.pop_back();

  shaderParams.push_back("GENERAL");
  if(Mode == 2)
    shaderParams.push_back("WIREFRAME");
  camp::generalShader[0]=compileAndLinkShader(shaders,shaderParams,ssbo,
                                              interlock);
  shaderParams.push_back("OPAQUE");
  camp::generalShader[1]=compileAndLinkShader(shaders,shaderParams,ssbo);
  shaderParams.pop_back();

  shaderParams.push_back("TRANSPARENT");
  camp::transparentShader=compileAndLinkShader(shaders,shaderParams,ssbo,
                                               interlock);
  shaderParams.clear();

  if(ssbo) {
    if(GPUindexing)
      shaderParams.push_back("GPUINDEXING");
     shaders[0]=ShaderfileModePair(screen.c_str(),GL_VERTEX_SHADER);
    shaders[1]=ShaderfileModePair(compress.c_str(),GL_FRAGMENT_SHADER);
    camp::compressShader=compileAndLinkShader(shaders,shaderParams,ssbo);
     if(GPUindexing)
      shaderParams.pop_back();
    else {
      shaders[1]=ShaderfileModePair(zero.c_str(),GL_FRAGMENT_SHADER);
      camp::zeroShader=compileAndLinkShader(shaders,shaderParams,ssbo);
    }
    maxSize=1;
    initBlendShader();
  }
  lastshader=-1;
}

void deleteComputeShaders()
{
  glDeleteProgram(camp::sum1Shader);
  glDeleteProgram(camp::sum2Shader);
  glDeleteProgram(camp::sum3Shader);
}

void deleteBlendShader()
{
  glDeleteProgram(camp::blendShader);
}

void deleteShaders()
{
  if(camp::ssbo) {
    deleteBlendShader();
    if(GPUindexing)
      deleteComputeShaders();
    else
      glDeleteProgram(camp::zeroShader);
    glDeleteProgram(camp::countShader);
    glDeleteProgram(camp::compressShader);
  }

  glDeleteProgram(camp::transparentShader);
  for(unsigned int opaque=0; opaque < 2; ++opaque) {
    glDeleteProgram(camp::generalShader[opaque]);
    glDeleteProgram(camp::colorShader[opaque]);
    glDeleteProgram(camp::materialShader[opaque]);
  }
  glDeleteProgram(camp::pixelShader);
}

void resizeBlendShader()
{
  gl::maxSize=ceilpow2(gl::maxSize);
  gl::deleteBlendShader();
  gl::initBlendShader();
  gl::lastSize=gl::maxSize;
}

void setBuffers()
{
  GLuint vao;
  glGenVertexArrays(1,&vao);
  glBindVertexArray(vao);

  camp::material0Data.reserve0();
  camp::materialData.reserve();
  camp::colorData.Reserve();
  camp::triangleData.Reserve();
  camp::transparentData.Reserve();

#ifdef HAVE_SSBO
  glGenBuffers(1, &camp::offsetBuffer);
  if(GPUindexing) {
    glGenBuffers(1, &camp::localSumBuffer);
    glGenBuffers(1, &camp::globalSumBuffer);
  } else
    glGenBuffers(1, &camp::countBuffer);
  if(GPUcompress) {
    glGenBuffers(1, &camp::indexBuffer);
    glGenBuffers(1, &camp::elementsBuffer);
  }
  glGenBuffers(1, &camp::fragmentBuffer);
  glGenBuffers(1, &camp::depthBuffer);
  glGenBuffers(1, &camp::opaqueBuffer);
  glGenBuffers(1, &camp::opaqueDepthBuffer);
#endif
}

void drawscene(int Width, int Height)
{
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  static bool first=true;
  if(glthread && first) {
    wait(initSignal,initLock);
    endwait(initSignal,initLock);
    first=false;
  }

  if(format3dWait)
    wait(initSignal,initLock);
#endif

  if((nlights == 0 && Nlights > 0) || nlights > Nlights ||
     nmaterials > Nmaterials) {
    deleteShaders();
    initShaders();
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  triple m(xmin,ymin,Zmin);
  triple M(xmax,ymax,Zmax);
  double perspective=orthographic ? 0.0 : 1.0/Zmax;

  double size2=hypot(Width,Height);

  if(remesh)
    camp::clearCenters();

  Picture->render(size2,m,M,perspective,remesh);

  if(!outlinemode) remesh=false;
}

// Return x divided by y rounded up to the nearest integer.
int ceilquotient(int x, int y)
{
  return (x+y-1)/y;
}

bool exporting=false;

void Export()
{
  size_t ndata=3*fullWidth*fullHeight;
  if(ndata == 0) return;
  glReadBuffer(GL_BACK_LEFT);
  glPixelStorei(GL_PACK_ALIGNMENT,1);
  glFinish();
  exporting=true;

  try {
    unsigned char *data=new unsigned char[ndata];
    if(data) {
      TRcontext *tr=trNew();
      int width=ceilquotient(fullWidth,
                             ceilquotient(fullWidth,min(maxTileWidth,Width)));
      int height=ceilquotient(fullHeight,
                              ceilquotient(fullHeight,
                                           min(maxTileHeight,Height)));
      if(settings::verbose > 1)
        cout << "Exporting " << Prefix << " as " << fullWidth << "x"
             << fullHeight << " image" << " using tiles of size "
             << width << "x" << height << endl;

      unsigned border=min(min(1,width/2),height/2);
      trTileSize(tr,width,height,border);
      trImageSize(tr,fullWidth,fullHeight);
      trImageBuffer(tr,GL_RGB,GL_UNSIGNED_BYTE,data);

      setDimensions(fullWidth,fullHeight,X/Width*fullWidth,Y/Width*fullWidth);
      (orthographic ? trOrtho : trFrustum)(tr,xmin,xmax,ymin,ymax,-Zmax,-Zmin);

      size_t count=0;
      do {
        trBeginTile(tr);
        remesh=true;
        drawscene(fullWidth,fullHeight);
        gl::lastshader=-1;
        ++count;
      } while (trEndTile(tr));
      if(settings::verbose > 1)
        cout << count << " tile" << (count != 1 ? "s" : "") << " drawn" << endl;
      trDelete(tr);

      picture pic;
      double w=oWidth;
      double h=oHeight;
      double Aspect=((double) fullWidth)/fullHeight;
      if(w > h*Aspect) w=(int) (h*Aspect+0.5);
      else h=(int) (w/Aspect+0.5);
      // Render an antialiased image.
      drawRawImage *Image=new drawRawImage(data,fullWidth,fullHeight,
                                           transform(0.0,0.0,w,0.0,0.0,h),
                                           antialias);
      pic.append(Image);
      pic.shipout(NULL,Prefix,Format,false,ViewExport);
      delete Image;
      delete[] data;
    }
  } catch(handled_error const&) {
  } catch(std::bad_alloc&) {
    outOfMemory();
  }
  remesh=true;
  setProjection();

#ifndef HAVE_LIBOSMESA
#ifdef HAVE_LIBGLUT
  glutPostRedisplay();
#endif

#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  if(glthread && readyAfterExport) {
    readyAfterExport=false;
    endwait(readySignal,readyLock);
  }
#endif
#endif
  exporting=false;
  camp::initSSBO=true;
}

void nodisplay()
{
}

void destroywindow()
{
  glutDestroyWindow(glutGetWindow());
}

// Return the greatest power of 2 less than or equal to n.
inline unsigned int floorpow2(unsigned int n)
{
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return n-(n >> 1);
}

void quit()
{
#ifdef HAVE_LIBOSMESA
  if(osmesa_buffer) delete[] osmesa_buffer;
  if(ctx) OSMesaDestroyContext(ctx);
  exit(0);
#endif
#ifdef HAVE_LIBGLUT
  if(glthread) {
    bool animating=getSetting<bool>("animating");
    if(animating)
      Setting("interrupt")=true;
    home();
    Animate=getSetting<bool>("autoplay");
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
    if(!interact::interactive || animating) {
      idle();
      glutDisplayFunc(nodisplay);
      endwait(readySignal,readyLock);
    }
#endif
    if(interact::interactive)
      glutHideWindow();
#if defined(MIKTEX)
    glutLeaveMainLoop();
#endif
  } else {
    glutDestroyWindow(window);
    exit(0);
  }
#endif
}

void mode()
{
  remesh=true;
  if(camp::ssbo)
    camp::initSSBO=true;
  ++Mode;
  if(Mode > 2) Mode=0;

  switch(Mode) {
    case 0: // regular
      outlinemode=false;
      ibl=getSetting<bool>("ibl");
      nlights=nlights0;
      lastshader=-1;
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      break;
    case 1: // outline
      outlinemode=true;
      ibl=false;
      nlights=0; // Force shader recompilation
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      break;
    case 2: // wireframe
      outlinemode=false;
      Nlights=1; // Force shader recompilation
      break;
  }
#ifdef HAVE_LIBGLUT
#ifndef HAVE_LIBOSMESA
  glutPostRedisplay();
#endif
#endif
}

// GUI-related functions
#ifdef HAVE_LIBGLUT
bool capsize(int& width, int& height)
{
  bool resize=false;
  if(width > screenWidth) {
    width=screenWidth;
    resize=true;
  }
  if(height > screenHeight) {
    height=screenHeight;
    resize=true;
  }
  return resize;
}

void reshape0(int width, int height)
{
  X=(X/Width)*width;
  Y=(Y/Height)*height;

  Width=width;
  Height=height;

  static int lastWidth=1;
  static int lastHeight=1;
  if(View && Width*Height > 1 && (Width != lastWidth || Height != lastHeight)
     && settings::verbose > 1) {
    cout << "Rendering " << stripDir(Prefix) << " as "
         << Width << "x" << Height << " image" << endl;
    lastWidth=Width;
    lastHeight=Height;
  }

  setProjection();
  glViewport(0,0,Width,Height);
  if(camp::ssbo)
    camp::initSSBO=true;
}

void windowposition(int& x, int& y, int width=Width, int height=Height)
{
  pair z=getSetting<pair>("position");
  x=(int) z.getx();
  y=(int) z.gety();
  if(x < 0) {
    x += screenWidth-width;
    if(x < 0) x=0;
  }
  if(y < 0) {
    y += screenHeight-height;
    if(y < 0) y=0;
  }
}

void setsize(int w, int h, bool reposition=true)
{
  int x,y;

  capsize(w,h);
  if(reposition) {
    windowposition(x,y,w,h);
    glutPositionWindow(x,y);
  } else
    glutPositionWindow(max(glutGet(GLUT_WINDOW_X)-2,0),
                       max(glutGet(GLUT_WINDOW_Y)-2,0));

  glutReshapeWindow(w,h);
  reshape0(w,h);
  glutPostRedisplay();
}

void capzoom()
{
  static double maxzoom=sqrt(DBL_MAX);
  static double minzoom=1.0/maxzoom;
  if(Zoom <= minzoom) Zoom=minzoom;
  if(Zoom >= maxzoom) Zoom=maxzoom;

  if(Zoom != lastzoom) remesh=true;
  lastzoom=Zoom;
}

void fullscreen(bool reposition=true)
{
  Width=screenWidth;
  Height=screenHeight;
  if(firstFit) {
    if(Width < Height*Aspect)
      Zoom *= Width/(Height*Aspect);
    capzoom();
    setProjection();
    firstFit=false;
  }
  Xfactor=((double) screenHeight)/Height;
  Yfactor=((double) screenWidth)/Width;
  reshape0(Width,Height);
  if(reposition)
    glutPositionWindow(0,0);
  glutReshapeWindow(Width,Height);
  glutPostRedisplay();
}

void fitscreen(bool reposition=true)
{
  if(Animate && Fitscreen == 2) Fitscreen=0;
  switch(Fitscreen) {
    case 0: // Original size
    {
      Xfactor=Yfactor=1.0;
      double pixelRatio=getSetting<double>("devicepixelratio");
      setsize(oldWidth*pixelRatio,oldHeight*pixelRatio,reposition);
      break;
    }
    case 1: // Fit to screen in one dimension
    {
      oldWidth=Width;
      oldHeight=Height;
      int w=screenWidth;
      int h=screenHeight;
      if(w > h*Aspect)
        w=min((int) ceil(h*Aspect),w);
      else
        h=min((int) ceil(w/Aspect),h);
      setsize(w,h,reposition);
      break;
    }
    case 2: // Full screen
    {
      fullscreen(reposition);
      break;
    }
  }
}

void togglefitscreen()
{
  ++Fitscreen;
  if(Fitscreen > 2) Fitscreen=0;
  fitscreen();
}

void initTimer()
{
  gettimeofday(&lasttime,NULL);
  lastframetime=lasttime;
}

void idleFunc(void (*f)())
{
  initTimer();
  glutIdleFunc(f);
}

void screen()
{
  if(glthread && !interact::interactive)
    fitscreen(false);
}

void nextframe(int)
{
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  endwait(readySignal,readyLock);
#endif
  double framedelay=getSetting<double>("framedelay");
  if(framedelay > 0)
    usleep((unsigned int) (1000.0*framedelay+0.5));
  if(Step) Animate=false;
}

void display()
{
  if(queueScreen) {
    if(!Animate) screen();
    queueScreen=false;
  }

  bool fps=settings::verbose > 2;
  drawscene(Width,Height);
  if(fps) {
    if(framecount < 20) // Measure steady-state framerate
      seconds();
    else {
      double s=seconds();
      if(s > 0.0) {
        double rate=1.0/s;
        S.add(rate);
        if(framecount % 20 == 0)
          cout << "FPS=" << rate << "\t" << S.mean() << " +/- " << S.stdev()
               << endl;
      }
    }
    ++framecount;
  }
  glutSwapBuffers();

#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  if(glthread && Animate) {
    queueExport=false;
    double delay=1.0/getSetting<double>("framerate");
    timeval tv;
    gettimeofday(&tv,NULL);
    double seconds=tv.tv_sec-lastframetime.tv_sec+
      ((double) tv.tv_usec-lastframetime.tv_usec)/1000000.0;
    lastframetime=tv;
    double milliseconds=1000.0*(delay-seconds);
    double framedelay=getSetting<double>("framedelay");
    if(framedelay > 0) milliseconds -= framedelay;
    if(milliseconds > 0)
      glutTimerFunc((int) (milliseconds+0.5),nextframe,0);
    else nextframe(0);
  }
#endif
  if(queueExport) {
    Export();
    queueExport=false;
  }
  if(!glthread) {
#if defined(MIKTEX_WINDOWS)
    // MIKTEX-UNEXPECTED
#else
    if(Oldpid != 0 && waitpid(Oldpid,NULL,WNOHANG) != Oldpid) {
      kill(Oldpid,SIGHUP);
      Oldpid=0;
    }
#endif
  }
}

void update()
{
  glutDisplayFunc(display);
  Animate=getSetting<bool>("autoplay");
  glutShowWindow();
  if(Zoom != lastzoom) remesh=true;
  lastzoom=Zoom;
  double cz=0.5*(Zmin+Zmax);

  dviewMat=translate(translate(dmat4(1.0),dvec3(cx,cy,cz))*drotateMat,
                     dvec3(0,0,-cz));
  if(!camp::ssbo)
    dView=value_ptr(dviewMat);
  viewMat=mat4(dviewMat);

  setProjection();
  updateModelViewData();

  glutPostRedisplay();
}

void updateHandler(int)
{
  queueScreen=true;
  remesh=true;
  update();
  if(interact::interactive || !Animate) {
    glutShowWindow();
  }
}

void poll(int)
{
  if(glupdate) {
    updateHandler(0);
    glupdate=false;
  }
  if(glexit) {
    exitHandler(0);
    glexit=false;
  }
  glutTimerFunc(100.0,poll,0);
}

void animate()
{
  Animate=!Animate;
  if(Animate) {
    if(Fitscreen == 2) {
      togglefitscreen();
      togglefitscreen();
    }
    update();
  } else idle();
}

void reshape(int width, int height)
{
  if(glthread) {
    static bool initialize=true;
    if(initialize) {
      initialize=false;
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
      MiKTeX::Aymptote::sigusr1 = &MiKTeX::Aymptote::updateRequested;
#else
      Signal(SIGUSR1,updateHandler);
#endif
    }
  }

  if(capsize(width,height))
    glutReshapeWindow(width,height);

  reshape0(width,height);
  remesh=true;
}

void shift(int x, int y)
{
  double Zoominv=1.0/Zoom;
  X += (x-x0)*Zoominv;
  Y += (y0-y)*Zoominv;
  x0=x; y0=y;
  update();
}

void pan(int x, int y)
{
  if(orthographic) {
    double Zoominv=1.0/Zoom;
    X += (x-x0)*Zoominv;
    Y += (y0-y)*Zoominv;
  } else {
    cx += (x-x0)*(xmax-xmin)/Width;
    cy += (y0-y)*(ymax-ymin)/Height;
  }
  x0=x; y0=y;
  update();
}

void zoom(int x, int y)
{
  if(ignorezoom) {ignorezoom=false; y0=y; return;}
  double zoomFactor=getSetting<double>("zoomfactor");
  if(zoomFactor > 0.0) {
    double zoomStep=getSetting<double>("zoomstep");
    const double limit=log(0.1*DBL_MAX)/log(zoomFactor);
    double stepPower=zoomStep*(y0-y);
    if(fabs(stepPower) < limit) {
      Zoom *= pow(zoomFactor,stepPower);
      capzoom();
      y0=y;
      setProjection();
      glutPostRedisplay();
    }
  }
}

void mousewheel(int wheel, int direction, int x, int y)
{
  double zoomFactor=getSetting<double>("zoomfactor");
  if(zoomFactor > 0.0) {
    if(direction > 0)
      Zoom *= zoomFactor;
    else
      Zoom /= zoomFactor;
    capzoom();
    setProjection();
    glutPostRedisplay();
  }
}

struct arcball {
  double angle;
  triple axis;

  arcball(double x0, double y0, double x, double y) {
    triple v0=norm(x0,y0);
    triple v1=norm(x,y);
    double Dot=dot(v0,v1);
    angle=Dot > 1.0 ? 0.0 : Dot < -1.0 ? pi : acos(Dot);
    axis=unit(cross(v0,v1));
  }

  triple norm(double x, double y) {
    double norm=hypot(x,y);
    if(norm > 1.0) {
      double denom=1.0/norm;
      x *= denom;
      y *= denom;
    }
    return triple(x,y,sqrt(max(1.0-x*x-y*y,0.0)));
  }
};

inline double glx(int x) {
  return 2.0*x/Width-1.0;
}

inline double gly(int y) {
  return 1.0-2.0*y/Height;
}

void rotate(int x, int y)
{
  if(x != x0 || y != y0) {
    arcball A(glx(x0),gly(y0),glx(x),gly(y));
    triple v=A.axis;
    drotateMat=glm::rotate<double>(2*A.angle/Zoom*ArcballFactor,
                                   glm::dvec3(v.getx(),v.gety(),v.getz()))*
      drotateMat;
    x0=x; y0=y;
    update();
  }
}

double Degrees(int x, int y)
{
  return atan2(0.5*Height-y-Y,x-0.5*Width-X)*degrees;
}

void rotateX(double step)
{
  dmat4 tmpRot(1.0);
  tmpRot=glm::rotate(tmpRot,glm::radians(step),dvec3(1,0,0));
  drotateMat=tmpRot*drotateMat;
  update();
}

void rotateY(double step)
{
  dmat4 tmpRot(1.0);
  tmpRot=glm::rotate(tmpRot,glm::radians(step),dvec3(0,1,0));
  drotateMat=tmpRot*drotateMat;
  update();
}

void rotateZ(double step)
{
  dmat4 tmpRot(1.0);
  tmpRot=glm::rotate(tmpRot,glm::radians(step),dvec3(0,0,1));
  drotateMat=tmpRot*drotateMat;
  update();
}

void rotateX(int x, int y)
{
  double angle=Degrees(x,y);
  rotateX(angle-lastangle);
  lastangle=angle;
}

void rotateY(int x, int y)
{
  double angle=Degrees(x,y);
  rotateY(angle-lastangle);
  lastangle=angle;
}

void rotateZ(int x, int y)
{
  double angle=Degrees(x,y);
  rotateZ(angle-lastangle);
  lastangle=angle;
}

#ifndef GLUT_WHEEL_UP
#define GLUT_WHEEL_UP 3
#endif

#ifndef GLUT_WHEEL_DOWN
#define GLUT_WHEEL_DOWN 4
#endif

string action(int button, int mod)
{
  size_t Button;
  size_t nButtons=5;
  switch(button) {
    case GLUT_LEFT_BUTTON:
      Button=0;
      break;
    case GLUT_MIDDLE_BUTTON:
      Button=1;
      break;
    case GLUT_RIGHT_BUTTON:
      Button=2;
      break;
    case GLUT_WHEEL_UP:
      Button=3;
      break;
    case GLUT_WHEEL_DOWN:
      Button=4;
      break;
    default:
      Button=nButtons;
  }

  size_t Mod;
  size_t nMods=4;
  switch(mod) {
    case 0:
      Mod=0;
      break;
    case GLUT_ACTIVE_SHIFT:
      Mod=1;
      break;
    case GLUT_ACTIVE_CTRL:
      Mod=2;
      break;
    case GLUT_ACTIVE_ALT:
      Mod=3;
      break;
    default:
      Mod=nMods;
  }

  if(Button < nButtons) {
    array *left=getSetting<array *>("leftbutton");
    array *middle=getSetting<array *>("middlebutton");
    array *right=getSetting<array *>("rightbutton");
    array *wheelup=getSetting<array *>("wheelup");
    array *wheeldown=getSetting<array *>("wheeldown");
    array *Buttons[]={left,middle,right,wheelup,wheeldown};
    array *a=Buttons[button];
    size_t size=checkArray(a);
    if(Mod < size)
      return read<string>(a,Mod);
  }
  return "";
}

void timeout(int)
{
}

void mouse(int button, int state, int x, int y)
{
  int mod=glutGetModifiers();
  string Action=action(button,mod);

  if(Action == "zoomin") {
    glutMotionFunc(NULL);
    mousewheel(0,1,x,y);
    return;
  }
  if(Action == "zoomout") {
    glutMotionFunc(NULL);
    mousewheel(0,-1,x,y);
    return;
  }

  if(state == GLUT_DOWN) {
    if(Action == "rotate") {
      x0=x; y0=y;
      glutMotionFunc(rotate);
    } else if(Action == "shift") {
      x0=x; y0=y;
      glutMotionFunc(shift);
    } else if(Action == "pan") {
      x0=x; y0=y;
      glutMotionFunc(pan);
    } else if(Action == "zoom" || Action == "zoom/menu") {
      y0=y;
      glutMotionFunc(zoom);
    } else if(Action == "rotateX") {
      lastangle=Degrees(x,y);
      glutMotionFunc(rotateX);
    } else if(Action == "rotateY") {
      lastangle=Degrees(x,y);
      glutMotionFunc(rotateY);
    } else if(Action == "rotateZ") {
      lastangle=Degrees(x,y);
      glutMotionFunc(rotateZ);
    }
  } else {
    glutMotionFunc(NULL);
  }
}

double spinstep()
{
  timeval tv;
  gettimeofday(&tv,NULL);
  double step=getSetting<double>("spinstep")*
    (tv.tv_sec-lasttime.tv_sec+
     ((double) tv.tv_usec-lasttime.tv_usec)/1000000.0);
  lasttime=tv;
  return step;
}

void xspin()
{
  rotateX(spinstep());
}

void yspin()
{
  rotateY(spinstep());
}

void zspin()
{
  rotateZ(spinstep());
}

void expand()
{
  double resizeStep=getSetting<double>("resizestep");
  if(resizeStep > 0.0)
    setsize((int) (Width*resizeStep+0.5),(int) (Height*resizeStep+0.5));
}

void shrink()
{
  double resizeStep=getSetting<double>("resizestep");
  if(resizeStep > 0.0)
    setsize(max((int) (Width/resizeStep+0.5),1),
            max((int) (Height/resizeStep+0.5),1));
}

void spinx()
{
  if(Xspin)
    idle();
  else {
    idleFunc(xspin);
    Xspin=true;
    Yspin=Zspin=false;
  }
}

void spiny()
{
  if(Yspin)
    idle();
  else {
    idleFunc(yspin);
    Yspin=true;
    Xspin=Zspin=false;
  }
}

void spinz()
{
  if(Zspin)
    idle();
  else {
    idleFunc(zspin);
    Zspin=true;
    Xspin=Yspin=false;
  }
}

void showCamera()
{
  projection P=camera();
  string projection=P.orthographic ? "orthographic(" : "perspective(";
  string indent(2+projection.length(),' ');
  cout << endl
       << "currentprojection=" << endl << "  "
       << projection << "camera=" << P.camera << "," << endl
       << indent << "up=" << P.up << "," << endl
       << indent << "target=" << P.target << "," << endl
       << indent << "zoom=" << P.zoom;
  if(!orthographic)
    cout << "," << endl << indent << "angle=" << P.angle;
  if(P.viewportshift != pair(0.0,0.0))
    cout << "," << endl << indent << "viewportshift=" << P.viewportshift*Zoom;
  if(!orthographic)
    cout << "," << endl << indent << "autoadjust=false";
  cout << ");" << endl;
}

void keyboard(unsigned char key, int x, int y)
{
  switch(key) {
    case 'h':
      home();
      update();
      break;
    case 'f':
      togglefitscreen();
      break;
    case 'x':
      spinx();
      break;
    case 'y':
      spiny();
      break;
    case 'z':
      spinz();
      break;
    case 's':
      idle();
      break;
    case 'm':
      mode();
      break;
    case 'e':
      Export();
      break;
    case 'c':
      showCamera();
      break;
    case '+':
    case '=':
    case '>':
      expand();
      break;
    case '-':
    case '_':
    case '<':
      shrink();
      break;
    case 'p':
      if(getSetting<bool>("reverse")) Animate=false;
      Setting("reverse")=Step=false;
      animate();
      break;
    case 'r':
      if(!getSetting<bool>("reverse")) Animate=false;
      Setting("reverse")=true;
      Step=false;
      animate();
      break;
    case ' ':
      Step=true;
      animate();
      break;
    case 17: // Ctrl-q
    case 'q':
      if(!Format.empty()) Export();
      quit();
      break;
  }
}

void setosize()
{
  oldWidth=(int) ceil(oWidth);
  oldHeight=(int) ceil(oHeight);
}
#endif
// end of GUI-related functions

void exportHandler(int=0)
{
#ifdef HAVE_LIBGLUT
#ifndef HAVE_LIBOSMESA
  if(!Iconify)
    glutShowWindow();
#endif
#endif
  readyAfterExport=true;
  Export();

#ifdef HAVE_LIBGLUT
#ifndef HAVE_LIBOSMESA
  if(!Iconify)
    glutHideWindow();
  glutDisplayFunc(nodisplay);
#endif
#endif
}

static bool glinitialize=true;

projection camera(bool user)
{
  if(glinitialize) return projection();

  camp::Triple vCamera,vUp,vTarget;

  double cz=0.5*(Zmin+Zmax);

  double *Rotate=value_ptr(drotateMat);

  if(user) {
    for(int i=0; i < 3; ++i) {
      double sumCamera=0.0, sumTarget=0.0, sumUp=0.0;
      int i4=4*i;
      for(int j=0; j < 4; ++j) {
        int j4=4*j;
        double R0=Rotate[j4];
        double R1=Rotate[j4+1];
        double R2=Rotate[j4+2];
        double R3=Rotate[j4+3];
        double T4ij=T[i4+j];
        sumCamera += T4ij*(R3-cx*R0-cy*R1-cz*R2);
        sumUp += T4ij*R1;
        sumTarget += T4ij*(R3-cx*R0-cy*R1);
      }
      vCamera[i]=sumCamera;
      vUp[i]=sumUp;
      vTarget[i]=sumTarget;
    }
  } else {
    for(int i=0; i < 3; ++i) {
      int i4=4*i;
      double R0=Rotate[i4];
      double R1=Rotate[i4+1];
      double R2=Rotate[i4+2];
      double R3=Rotate[i4+3];
      vCamera[i]=R3-cx*R0-cy*R1-cz*R2;
      vUp[i]=R1;
      vTarget[i]=R3-cx*R0-cy*R1;
    }
  }

  return projection(orthographic,vCamera,vUp,vTarget,Zoom,
                    2.0*atan(tan(0.5*Angle)/Zoom)/radians,
                    pair(X/Width+Shift.getx(),
                         Y/Height+Shift.gety()));
}

void init()
{
#ifdef HAVE_LIBGLUT
  mem::vector<string> cmd;
  cmd.push_back(settings::argv0);
  if(!interact::interactive && Iconify)
    cmd.push_back("-iconic");
  push_split(cmd,getSetting<string>("glOptions"));
  char **argv=args(cmd,true);
  int argc=cmd.size();

#ifndef __APPLE__
  glutInitContextVersion(4,0);
  glutInitContextProfile(GLUT_CORE_PROFILE);
#endif

  fpu_trap(false); // Work around FE_INVALID
  glutInit(&argc,argv);
  fpu_trap(settings::trap());

  screenWidth=glutGet(GLUT_SCREEN_WIDTH);
  screenHeight=glutGet(GLUT_SCREEN_HEIGHT);
#endif
}

void init_osmesa()
{
#ifdef HAVE_LIBOSMESA
  // create context and buffer
  if(settings::verbose > 1)
    cout << "Allocating osmesa_buffer of size " << screenWidth << "x"
         << screenHeight << "x4x" << sizeof(GLubyte) << endl;
  osmesa_buffer=new unsigned char[screenWidth*screenHeight*4*sizeof(GLubyte)];
  if(!osmesa_buffer) {
    cerr << "Cannot allocate image buffer." << endl;
    exit(-1);
  }

  const int attribs[]={
    OSMESA_FORMAT,OSMESA_RGBA,
    OSMESA_DEPTH_BITS,16,
    OSMESA_STENCIL_BITS,0,
    OSMESA_ACCUM_BITS,0,
    OSMESA_PROFILE,OSMESA_CORE_PROFILE,
    OSMESA_CONTEXT_MAJOR_VERSION,4,
    OSMESA_CONTEXT_MINOR_VERSION,3,
    0,0
  };

  ctx=OSMesaCreateContextAttribs(attribs,NULL);
  if(!ctx) {
    ctx=OSMesaCreateContextExt(OSMESA_RGBA,16,0,0,NULL);
    if(!ctx) {
      cerr << "OSMesaCreateContextExt failed." << endl;
      exit(-1);
    }
  }

  if(!OSMesaMakeCurrent(ctx,osmesa_buffer,GL_UNSIGNED_BYTE,
                        screenWidth,screenHeight )) {
    cerr << "OSMesaMakeCurrent failed." << endl;
    exit(-1);
  }

  int z=0, s=0, a=0;
  glGetIntegerv(GL_DEPTH_BITS,&z);
  glGetIntegerv(GL_STENCIL_BITS,&s);
  glGetIntegerv(GL_ACCUM_RED_BITS,&a);
  if(settings::verbose > 1)
    cout << "Offscreen context settings: Depth=" << z << " Stencil=" << s
         << " Accum=" << a << endl;

  if(z <= 0) {
    cerr << "Error initializing offscreen context: Depth=" << z << endl;
    exit(-1);
  }
#endif // HAVE_LIBOSMESA
}

#endif /* HAVE_GL */

bool NVIDIA()
{
  char *GLSL_VERSION=(char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
  return string(GLSL_VERSION).find("NVIDIA") != string::npos;
}

// angle=0 means orthographic.
void glrender(const string& prefix, const picture *pic, const string& format,
              double width, double height, double angle, double zoom,
              const triple& m, const triple& M, const pair& shift,
              const pair& margin, double *t,
              double *background, size_t nlightsin, triple *lights,
              double *diffuse, double *specular, bool view, int oldpid)
{
  Iconify=getSetting<bool>("iconify");

  if(zoom == 0.0) zoom=1.0;

  Prefix=prefix;
  Picture=pic;
  Format=format;

  nlights0=nlights=nlightsin;

  Lights=lights;
  Diffuse=diffuse;
  Specular=specular;
  View=view;
  Angle=angle*radians;
  Zoom0=zoom;
  Oldpid=oldpid;
  Shift=shift/zoom;
  Margin=margin;
  for(size_t i=0; i < 4; ++i)
    Background[i]=background[i];

  Xmin=m.getx();
  Xmax=M.getx();
  Ymin=m.gety();
  Ymax=M.gety();
  Zmin=m.getz();
  Zmax=M.getz();

  orthographic=Angle == 0.0;
  H=orthographic ? 0.0 : -tan(0.5*Angle)*Zmax;

  ignorezoom=false;
  Xfactor=Yfactor=1.0;

  pair maxtile=getSetting<pair>("maxtile");
  maxTileWidth=(int) maxtile.getx();
  maxTileHeight=(int) maxtile.gety();
  if(maxTileWidth <= 0) maxTileWidth=1024;
  if(maxTileHeight <= 0) maxTileHeight=768;

  bool v3d=format == "v3d";
  bool webgl=format == "html";
  bool format3d=webgl || v3d;

#ifdef HAVE_GL
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
#ifndef HAVE_LIBOSMESA
  static bool initializedView=false;
#endif
#endif

#ifdef HAVE_LIBOSMESA
  if(!webgl) {
    screenWidth=maxTileWidth;
    screenHeight=maxTileHeight;

    static bool osmesa_initialized=false;
    if(!osmesa_initialized) {
      osmesa_initialized=true;
      fpu_trap(false); // Work around FE_INVALID.
      init_osmesa();
      fpu_trap(settings::trap());
    }
  }
#else
  if(glinitialize) {
    if(!format3d) init();
    Fitscreen=1;
  }
#endif
#endif

  for(int i=0; i < 16; ++i)
    T[i]=t[i];

  static bool initialized=false;

  if(!(initialized && (interact::interactive ||
                       getSetting<bool>("animating")))) {
    antialias=getSetting<Int>("antialias") > 1;
    double expand;
    if(format3d)
      expand=1.0;
    else {
      expand=getSetting<double>("render");
      if(expand < 0)
        expand *= (Format.empty() || Format == "eps" || Format == "pdf")                 ? -2.0 : -1.0;
      if(antialias) expand *= 2.0;
    }

    oWidth=width;
    oHeight=height;
    Aspect=width/height;

    // Force a hard viewport limit to work around direct rendering bugs.
    // Alternatively, one can use -glOptions=-indirect (with a performance
    // penalty).
    pair maxViewport=getSetting<pair>("maxviewport");
    int maxWidth=maxViewport.getx() > 0 ? (int) ceil(maxViewport.getx()) :
      screenWidth;
    int maxHeight=maxViewport.gety() > 0 ? (int) ceil(maxViewport.gety()) :
      screenHeight;
    if(maxWidth <= 0) maxWidth=max(maxHeight,2);
    if(maxHeight <= 0) maxHeight=max(maxWidth,2);

    if(screenWidth <= 0) screenWidth=maxWidth;
    else screenWidth=min(screenWidth,maxWidth);
    if(screenHeight <= 0) screenHeight=maxHeight;
    else screenHeight=min(screenHeight,maxHeight);

    fullWidth=(int) ceil(expand*width);
    fullHeight=(int) ceil(expand*height);

    if(format3d) {
      Width=fullWidth;
      Height=fullHeight;
    } else {
      Width=min(fullWidth,screenWidth);
      Height=min(fullHeight,screenHeight);

      if(Width > Height*Aspect)
        Width=min((int) (ceil(Height*Aspect)),screenWidth);
      else
        Height=min((int) (ceil(Width/Aspect)),screenHeight);
    }

    home(format3d);
    setProjection();
    if(format3d) {
      remesh=true;
      return;
    }

    camp::maxFragments=0;

    ArcballFactor=1+8.0*hypot(Margin.getx(),Margin.gety())/hypot(Width,Height);

#ifdef HAVE_GL
    Aspect=((double) Width)/Height;

    if(maxTileWidth <= 0) maxTileWidth=screenWidth;
    if(maxTileHeight <= 0) maxTileHeight=screenHeight;
#ifdef HAVE_LIBGLUT
    setosize();
#endif
#endif
  }

#ifdef HAVE_GL
  bool havewindow=initialized && glthread;

#ifndef HAVE_LIBOSMESA
#ifdef HAVE_LIBGLUT
  unsigned int displaymode=GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH;
#endif

#ifdef __APPLE__
  displaymode |= GLUT_3_2_CORE_PROFILE;
#endif
#endif

  if(glthread && format3d)
    format3dWait=true;

  camp::clearMaterials();

#ifndef HAVE_LIBOSMESA

#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  if(glthread && initializedView) {
    if(View) {
#ifdef __MSDOS__ // Signals are unreliable in MSWindows
      glupdate=true;
#else
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
      *MiKTeX::Aymptote::sigusr1 = true;
#else
      pthread_kill(mainthread,SIGUSR1);
#endif
#endif
    } else readyAfterExport=queueExport=true;
    return;
  }
#endif

#ifdef HAVE_LIBGLUT
  if(View) {
    int x,y;
    if(havewindow)
      glutDestroyWindow(window);

    windowposition(x,y);
    glutInitWindowPosition(x,y);
    glutInitWindowSize(1,1);
    Int multisample=getSetting<Int>("multisample");
    if(multisample <= 1) multisample=0;
    if(multisample)
      displaymode |= GLUT_MULTISAMPLE;
    glutInitDisplayMode(displaymode);

    int samples;

#ifdef FREEGLUT
#ifdef GLUT_INIT_MAJOR_VERSION
    while(true) {
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
        MiKTeX::Aymptote::RequestHandler();
#endif
      if(multisample > 0)
        glutSetOption(GLUT_MULTISAMPLE,multisample);
#endif
#endif
      string title=string(settings::PROGRAM)+": "+prefix;
      fpu_trap(false); // Work around FE_INVALID
      window=glutCreateWindow(title.c_str());
      fpu_trap(settings::trap());

      GLint samplebuf[1];
      glGetIntegerv(GL_SAMPLES,samplebuf);
      samples=samplebuf[0];
#ifdef FREEGLUT
#ifdef GLUT_INIT_MAJOR_VERSION
      if(samples < multisample) {
        multisample=floorpow2(multisample-1);
        if(multisample > 1) {
          glutReshapeWindow(1,1);
          glutDisplayFunc(destroywindow);
          glutShowWindow();
          glutMainLoopEvent();
          continue;
        }
      }
      break;
    }
#endif
#endif
    if(settings::verbose > 1 && samples > 1)
      cout << "Multisampling enabled with sample width " << samples
           << endl;
    glutDisplayFunc(display);
    glutShowWindow();
  } else if(!havewindow) {
    glutInitWindowSize(maxTileWidth,maxTileHeight);
    glutInitDisplayMode(displaymode);
    fpu_trap(false); // Work around FE_INVALID
    window=glutCreateWindow("");
    fpu_trap(settings::trap());
    glutHideWindow();
  }
#endif // HAVE_LIBGLUT
#endif // HAVE_LIBOSMESA

  initialized=true;

#if defined(HAVE_COMPUTE_SHADER) && !defined(HAVE_LIBOSMESA)
  GPUindexing=getSetting<bool>("GPUindexing");
  GPUcompress=NVIDIA() || getSetting<bool>("GPUcompress");
#else
  GPUindexing=false;
  GPUcompress=false;
#endif

  GLint val;
  glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE,&val);

  if(GPUindexing)
    lastlocalsize=localsize=getSetting<Int>("GPUlocalSize");

  Maxmaterials=val/sizeof(Material);
  if(nmaterials > Maxmaterials) nmaterials=Maxmaterials;

  if(glinitialize) {
    glinitialize=false;

    char *GLSL_VERSION=(char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
    GLSLversion=(int) (100*atof(GLSL_VERSION)+0.5);

    if(GLSLversion < 130) {
      cerr << "Unsupported GLSL version: " << GLSL_VERSION << "." << endl;
      exit(-1);
    }

    if(settings::verbose > 2)
      cout << "GLSL version " << GLSL_VERSION << endl;

    int result = glewInit();

    if(result != GLEW_OK) {
      cerr << "GLEW initialization error." << endl;
      exit(-1);
    }

    ibl=getSetting<bool>("ibl");
    initShaders();
    setBuffers();
  }

  glClearColor(background[0],background[1],background[2],background[3]);

#ifdef HAVE_LIBGLUT
#ifndef HAVE_LIBOSMESA
  Animate=getSetting<bool>("autoplay") && glthread;

  if(View) {
    if(!getSetting<bool>("fitscreen"))
      Fitscreen=0;
    firstFit=true;
    fitscreen();
    setosize();
  }
#endif
#endif

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glEnable(GL_TEXTURE_3D);

  if(!camp::ssbo) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  }

  Mode=2;
  mode();

  ViewExport=View;
#ifdef HAVE_LIBOSMESA
  View=false;
#endif

  if(View) {
#ifdef HAVE_LIBGLUT
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
#ifndef HAVE_LIBOSMESA
    initializedView=true;
#endif
#endif
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutDisplayFunc(display);

#ifdef __MSDOS__
    if(glthread && interact::interactive)
      poll(0);
#endif

    glutMainLoop();
#endif // HAVE_LIBGLUT
  } else {
    if(glthread) {
      if(havewindow) {
        readyAfterExport=true;
#ifdef HAVE_PTHREAD
        pthread_kill(mainthread,SIGUSR1);
#elif defined(MIKTEX)
        *MiKTeX::Aymptote::sigusr1 = true;
#endif
      } else {
        initialized=true;
        readyAfterExport=true;
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
        MiKTeX::Aymptote::sigusr1 = &MiKTeX::Aymptote::exportRequested;
#else
        Signal(SIGUSR1,exportHandler);
#endif
        exportHandler();
      }
    } else {
      exportHandler();
      quit();
    }
  }

#endif /* HAVE_GL */
}

} // namespace gl

#endif

#ifdef HAVE_GL

namespace camp {

string getLightIndex(size_t const& index, string const& fieldName) {
  ostringstream buf;
  buf << "lights[" << index << "]." << fieldName;
  return Strdup(buf.str());
}

string getCenterIndex(size_t const& index) {
  ostringstream buf;
  buf << "Centers[" << index << "]";
  return Strdup(buf.str());
}

template<class T>
void registerBuffer(const std::vector<T>& buffervector, GLuint& bufferIndex,
                    bool copy, GLenum type=GL_ARRAY_BUFFER) {
  if(!buffervector.empty()) {
    if(bufferIndex == 0) {
      glGenBuffers(1,&bufferIndex);
      copy=true;
    }
    glBindBuffer(type,bufferIndex);
    if(copy)
      glBufferData(type,buffervector.size()*sizeof(T),
                   buffervector.data(),GL_STATIC_DRAW);
  }
}

void clearCount()
{
  glUseProgram(zeroShader);
  gl::lastshader=zeroShader;
  glUniform1ui(glGetUniformLocation(zeroShader,"width"),gl::Width);
  fpu_trap(false); // Work around FE_INVALID
  glDrawArrays(GL_TRIANGLES, 0, 3);
  fpu_trap(settings::trap());
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void compressCount()
{
  glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
  glUseProgram(compressShader);
  gl::lastshader=compressShader;
  glUniform1ui(glGetUniformLocation(compressShader,"width"),gl::Width);
  fpu_trap(false); // Work around FE_INVALID
  glDrawArrays(GL_TRIANGLES, 0, 3);
  fpu_trap(settings::trap());
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void initPartialSums()
{
  gl::gs=gl::localsize*gl::g;
  gl::gs2=gl::localsize*gl::gs;
  gl::processors=gl::localsize*gl::gs2;
  GLuint zero=0;
  glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::localSumBuffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               (gl::processors+gl::gs2+2)*sizeof(GLuint),NULL,
               GL_DYNAMIC_DRAW);
  glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R32UI,GL_RED_INTEGER,
                    GL_UNSIGNED_INT,&zero);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2,camp::localSumBuffer);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::globalSumBuffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER,(gl::gs+1)*sizeof(GLuint),NULL,
               GL_DYNAMIC_DRAW);
  glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R32UI,GL_RED_INTEGER,
                    GL_UNSIGNED_INT,&zero);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER,3,camp::globalSumBuffer);
}

GLuint partialSums(bool readSize=false)
{
  GLuint fragments;
  // Compute local partial sums on the GPU
  glUseProgram(sum1Shader);
  glUniform1ui(glGetUniformLocation(sum1Shader,"elements"),gl::elements);

  glDispatchCompute(gl::gs2,1,1);

  glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

  glUseProgram(sum2Shader);
  GLuint offset2=gl::processors+1;
  glUniform1ui(glGetUniformLocation(sum2Shader,"offset2"),offset2);
  glDispatchCompute(gl::gs,1,1);

  glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

  glUseProgram(sum3Shader);
  glUniform1ui(glGetUniformLocation(sum3Shader,"offset2"),offset2);
  glUniform1ui(glGetUniformLocation(sum3Shader,"final"),offset2+gl::gs2);
  glDispatchCompute(gl::g,1,1);

  glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
  // Compute global partial sums, including number of fragments, on the CPU
  GLuint *sum=(GLuint *) glMapBuffer(GL_SHADER_STORAGE_BUFFER,GL_READ_WRITE);

  if(readSize) {
    gl::maxSize=sum[0];
    sum[0]=0;
    if(gl::maxSize > gl::lastSize)
      gl::resizeBlendShader();
  }

  fragments=sum[1];
  for(GLint i=2; i < gl::gs; ++i)
    sum[i]=fragments += sum[i];
  fragments += sum[gl::gs];

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  return fragments;
}

void refreshBuffers()
{
  GLuint zero=0;
  GLuint fragments;
  gl::pixels=gl::Width*gl::Height;

  if(initSSBO) {
    gl::processors=1;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::offsetBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 (GPUindexing ? 2 : 1)*(gl::pixels+1)*sizeof(GLuint),
                 NULL,GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,camp::offsetBuffer);
    if(GPUcompress) {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::indexBuffer);
      glBufferData(GL_SHADER_STORAGE_BUFFER,gl::pixels*sizeof(GLuint),
                   NULL,GL_DYNAMIC_DRAW);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER,1,camp::indexBuffer);
      glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R32UI,GL_RED_INTEGER,
                        GL_UNSIGNED_INT,&zero);

      GLuint one=1;
      glBindBuffer(GL_ATOMIC_COUNTER_BUFFER,camp::elementsBuffer);
      glBufferData(GL_ATOMIC_COUNTER_BUFFER,sizeof(GLuint),&one,
                   GL_DYNAMIC_DRAW);
      glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER,0,camp::elementsBuffer);
    } else {
      if(GPUindexing)
        glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R8UI,GL_RED_INTEGER,
                          GL_UNSIGNED_BYTE,&zero);
    }

    if(!GPUindexing) {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::countBuffer);
      glBufferData(GL_SHADER_STORAGE_BUFFER,(gl::pixels+1)*sizeof(GLuint),
                   NULL,GL_DYNAMIC_DRAW);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2,camp::countBuffer);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::opaqueBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,gl::pixels*sizeof(glm::vec4),NULL,
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,6,camp::opaqueBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::opaqueDepthBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(GLuint)+gl::pixels*sizeof(GLfloat),NULL,
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,7,camp::opaqueDepthBuffer);
    const GLfloat zerof=0.0;
    glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R32F,GL_RED,GL_FLOAT,&zerof);
  }

  // Determine the fragment offsets

  if(gl::exporting && GPUindexing && !GPUcompress) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::offsetBuffer);
    glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R8UI,GL_RED_INTEGER,
                      GL_UNSIGNED_BYTE,&zero);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::globalSumBuffer);
  }

  if(!interlock) {
    drawBuffer(material1Data,countShader);
    drawBuffer(materialData,countShader);
    drawBuffer(colorData,countShader,true);
    drawBuffer(triangleData,countShader,true);
  }

  glDepthMask(GL_FALSE); // Don't write to depth buffer
  glDisable(GL_MULTISAMPLE);
  drawBuffer(transparentData,countShader,true);
  glEnable(GL_MULTISAMPLE);
  glDepthMask(GL_TRUE); // Write to depth buffer

  if(GPUcompress) {
    compressCount();

    GLuint *p=(GLuint *) glMapBuffer(GL_ATOMIC_COUNTER_BUFFER,GL_READ_WRITE);
    gl::elements=GPUindexing ? p[0] : p[0]-1;
    p[0]=1;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    if(gl::elements == 0) return;
  } else
    gl::elements=gl::pixels;

  if(initSSBO) {
    if(GPUindexing) {
      double Tmin=HUGE_VAL;
      GLuint G=1;
      GLuint twos3=2*gl::localsize*gl::localsize*gl::localsize;

      if(twos3 > gl::elements) {
        gl::localsize=1;
        twos3=2;
      } else gl::localsize=settings::getSetting<Int>("GPUlocalSize");

      if(gl::localsize != gl::lastlocalsize) {
        gl::deleteComputeShaders();
        gl::initComputeShaders();
        gl::lastlocalsize=gl::localsize;
      }

      GLint stop=min(gl::maxgroups,(GLint) (gl::elements/twos3));
      for(gl::g=2; gl::g <= stop; gl::g *= 2) {
        initPartialSums();
        partialSums();
        seconds();
        partialSums();
        double T=seconds();
        if(T < Tmin) {
          Tmin=T;
          G=gl::g;
        }
      }
      gl::g=G;
      initPartialSums();
    }
    initSSBO=false;
  }

  if(GPUindexing)
    fragments=partialSums(true);
  else {
    size_t size=gl::elements*sizeof(GLuint);

    // Compute partial sums on the CPU
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::countBuffer);
    GLuint *p=(GLuint *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
                                              0,size+sizeof(GLuint),
                                              GL_MAP_READ_BIT);
    gl::maxSize=p[0];
    GLuint *count=p+1;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::offsetBuffer);
    GLuint *offset=(GLuint *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
                                               sizeof(GLuint),size,
                                               GL_MAP_WRITE_BIT);

    size_t Offset=offset[0]=count[0];
    for(size_t i=1; i < gl::elements; ++i)
      offset[i]=Offset += count[i];
    fragments=Offset;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::offsetBuffer);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::countBuffer);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    if(gl::exporting) {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::countBuffer);
      glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R32UI,GL_RED_INTEGER,
                        GL_UNSIGNED_INT,&zero);
    } else
      clearCount();

    if(gl::maxSize > gl::lastSize)
      gl::resizeBlendShader();
  }

  if(fragments > maxFragments) {
    // Initialize the alpha buffer
    maxFragments=11*fragments/10;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::fragmentBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,maxFragments*sizeof(glm::vec4),
                 NULL,GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,4,camp::fragmentBuffer);


    glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::depthBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,maxFragments*sizeof(GLfloat),
                 NULL,GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,5,camp::depthBuffer);

    if(GPUindexing)
      glBindBuffer(GL_SHADER_STORAGE_BUFFER,camp::globalSumBuffer);
  }

  gl::lastshader=-1;
}

void setUniforms(vertexBuffer& data, GLint shader)
{
  bool normal=shader != pixelShader;

  if(shader != gl::lastshader) {
    glUseProgram(shader);

    if(normal)
      glUniform1ui(glGetUniformLocation(shader,"width"),gl::Width);

    if(camp::ssbo && GPUindexing &&
       (shader == transparentShader || (!Opaque && !interlock))) {
      GLuint offset2=1+gl::processors;
      glUniform1ui(glGetUniformLocation(shader,"elements"),gl::elements);
      glUniform1ui(glGetUniformLocation(shader,"offset2"),offset2);
      GLuint m=gl::elements/gl::processors;
      GLuint r=gl::elements-m*gl::processors;
      glUniform1ui(glGetUniformLocation(shader,"m1"),m);
      glUniform1ui(glGetUniformLocation(shader,"m2"),gl::localsize);
      glUniform1ui(glGetUniformLocation(shader,"r"),r);
    }
  }

  glUniformMatrix4fv(glGetUniformLocation(shader,"projViewMat"),1,GL_FALSE,
                     value_ptr(gl::projViewMat));

  glUniformMatrix4fv(glGetUniformLocation(shader,"viewMat"),1,GL_FALSE,
                     value_ptr(gl::viewMat));
  if(normal)
    glUniformMatrix3fv(glGetUniformLocation(shader,"normMat"),1,GL_FALSE,
                       value_ptr(gl::normMat));

  if(shader == countShader) {
    gl::lastshader=shader;
    return;
  }

  if(shader != gl::lastshader) {
    gl::lastshader=shader;
    glUniform1ui(glGetUniformLocation(shader,"nlights"),gl::nlights);

    for(size_t i=0; i < gl::nlights; ++i) {
      triple Lighti=gl::Lights[i];
      size_t i4=4*i;
      glUniform3f(glGetUniformLocation(shader,
                                       getLightIndex(i,"direction").c_str()),
                  (GLfloat) Lighti.getx(),(GLfloat) Lighti.gety(),
                  (GLfloat) Lighti.getz());

      glUniform3f(glGetUniformLocation(shader,
                                       getLightIndex(i,"color").c_str()),
                  (GLfloat) gl::Diffuse[i4],(GLfloat) gl::Diffuse[i4+1],
                  (GLfloat) gl::Diffuse[i4+2]);
    }

    if(settings::getSetting<bool>("ibl")) {
      gl::IBLbrdfTex.setUniform(glGetUniformLocation(shader,
                                                     "reflBRDFSampler"));
      gl::irradiance.setUniform(glGetUniformLocation(shader,
                                                     "diffuseSampler"));
      gl::reflTextures.setUniform(glGetUniformLocation(shader,
                                                       "reflImgSampler"));
    }
  }

  GLuint binding=0;
  GLint blockindex=glGetUniformBlockIndex(shader,"MaterialBuffer");
  glUniformBlockBinding(shader,blockindex,binding);
  bool copy=(gl::remesh || data.partial || !data.rendered) && !gl::copied;
  registerBuffer(data.materials,data.materialsBuffer,copy,GL_UNIFORM_BUFFER);
  glBindBufferBase(GL_UNIFORM_BUFFER,binding,data.materialsBuffer);
}

void drawBuffer(vertexBuffer& data, GLint shader, bool color)
{
  if(data.indices.empty()) return;

  bool normal=shader != pixelShader;

  const size_t size=sizeof(GLfloat);
  const size_t intsize=sizeof(GLint);
  const size_t bytestride=color ? sizeof(VertexData) :
    (normal ? sizeof(vertexData) : sizeof(vertexData0));

  bool copy=(gl::remesh || data.partial || !data.rendered) && !gl::copied;
  if(color) registerBuffer(data.Vertices,data.VerticesBuffer,copy);
  else if(normal) registerBuffer(data.vertices,data.verticesBuffer,copy);
  else registerBuffer(data.vertices0,data.vertices0Buffer,copy);

  registerBuffer(data.indices,data.indicesBuffer,copy,GL_ELEMENT_ARRAY_BUFFER);

  camp::setUniforms(data,shader);

  data.rendered=true;

  glVertexAttribPointer(positionAttrib,3,GL_FLOAT,GL_FALSE,bytestride,
                        (void *) 0);
  glEnableVertexAttribArray(positionAttrib);

  if(normal && gl::Nlights > 0) {
    glVertexAttribPointer(normalAttrib,3,GL_FLOAT,GL_FALSE,bytestride,
                          (void *) (3*size));
    glEnableVertexAttribArray(normalAttrib);
  } else if(!normal) {
    glVertexAttribPointer(widthAttrib,1,GL_FLOAT,GL_FALSE,bytestride,
                          (void *) (3*size));
    glEnableVertexAttribArray(widthAttrib);
  }

  glVertexAttribIPointer(materialAttrib,1,GL_INT,bytestride,
                         (void *) ((normal ? 6 : 4)*size));
  glEnableVertexAttribArray(materialAttrib);

  if(color) {
    glVertexAttribPointer(colorAttrib,4,GL_FLOAT,GL_FALSE,bytestride,
                          (void *) (6*size+intsize));
    glEnableVertexAttribArray(colorAttrib);
  }

  fpu_trap(false); // Work around FE_INVALID
  glDrawElements(data.type,data.indices.size(),GL_UNSIGNED_INT,(void *) 0);
  fpu_trap(settings::trap());

  glDisableVertexAttribArray(positionAttrib);
  if(normal && gl::Nlights > 0)
    glDisableVertexAttribArray(normalAttrib);
  if(!normal)
    glDisableVertexAttribArray(widthAttrib);
  glDisableVertexAttribArray(materialAttrib);
  if(color)
    glDisableVertexAttribArray(colorAttrib);

  glBindBuffer(GL_UNIFORM_BUFFER,0);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

void drawMaterial0()
{
  drawBuffer(material0Data,pixelShader);
  material0Data.clear();
}

void drawMaterial1()
{
  drawBuffer(material1Data,materialShader[Opaque]);
  material1Data.clear();
}

void drawMaterial()
{
  drawBuffer(materialData,materialShader[Opaque]);
  materialData.clear();
}

void drawColor()
{
  drawBuffer(colorData,colorShader[Opaque],true);
  colorData.clear();
}

void drawTriangle()
{
  drawBuffer(triangleData,generalShader[Opaque],true);
  triangleData.clear();
}

void aBufferTransparency()
{
  // Collect transparent fragments
  glDepthMask(GL_FALSE); // Disregard depth
  drawBuffer(transparentData,transparentShader,true);
  glDepthMask(GL_TRUE); // Respect depth

  // Blend transparent fragments
  glDisable(GL_DEPTH_TEST);
  glUseProgram(blendShader);
  gl::lastshader=blendShader;
  glUniform1ui(glGetUniformLocation(blendShader,"width"),gl::Width);
  if(GPUindexing) {
    GLuint offset2=gl::processors+1;
    glUniform1ui(glGetUniformLocation(blendShader,"elements"),gl::elements);
    glUniform1ui(glGetUniformLocation(blendShader,"offset2"),offset2);
    GLuint m=gl::elements/gl::processors;
    GLuint r=gl::elements-m*gl::processors;
    glUniform1ui(glGetUniformLocation(blendShader,"m1"),m);
    glUniform1ui(glGetUniformLocation(blendShader,"m2"),gl::localsize);
    glUniform1ui(glGetUniformLocation(blendShader,"r"),r);
  }
  glUniform4f(glGetUniformLocation(blendShader,"background"),
              gl::Background[0],gl::Background[1],gl::Background[2],
              gl::Background[3]);
  fpu_trap(false); // Work around FE_INVALID
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glDrawArrays(GL_TRIANGLES,0,3);
  fpu_trap(settings::trap());
  transparentData.clear();
  glEnable(GL_DEPTH_TEST);
}

void drawTransparent()
{
  if(camp::ssbo) {
    glDisable(GL_MULTISAMPLE);
    aBufferTransparency();
    glEnable(GL_MULTISAMPLE);
  } else {
    sortTriangles();
    transparentData.rendered=false; // Force copying of sorted triangles to GPU
    glDepthMask(GL_FALSE); // Don't write to depth buffer
    drawBuffer(transparentData,transparentShader,true);
    glDepthMask(GL_TRUE); // Write to depth buffer
    transparentData.clear();
  }
}

void drawBuffers()
{
  gl::copied=false;
  Opaque=transparentData.indices.empty();
  bool transparent=!Opaque;
  if(camp::ssbo) {
    if(transparent) {
      refreshBuffers();
      if(!interlock) gl::copied=true;
    }
  }

  drawMaterial0();
  drawMaterial1();
  drawMaterial();
  drawColor();
  drawTriangle();

  if(transparent) {
    gl::copied=true;
    drawTransparent();
  }
  Opaque=0;
}

void setMaterial(vertexBuffer& data, draw_t *draw)
{
  if(materialIndex >= data.materialTable.size() ||
     data.materialTable[materialIndex] == -1) {
    if(data.materials.size() >= Maxmaterials) {
      data.partial=true;
      (*draw)();
    }
    size_t size0=data.materialTable.size();
    data.materialTable.resize(materialIndex+1);
    for(size_t i=size0; i < materialIndex; ++i)
      data.materialTable[i]=-1;
    data.materialTable[materialIndex]=data.materials.size();
    data.materials.push_back(materials[materialIndex]);
  }
  materialIndex=data.materialTable[materialIndex];
}

}
#endif /* HAVE_GL */
