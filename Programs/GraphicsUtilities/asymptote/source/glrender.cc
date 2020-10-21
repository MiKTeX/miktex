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
#endif // HAVE_LIBGLUT

#ifdef HAVE_LIBGLUT
#ifdef FREEGLUT
#include <GL/freeglut_ext.h>
#endif
#endif

#include "shaders.h"

#ifdef HAVE_LIBOPENIMAGEIO
#include <OpenImageIO/imageio.h>
#endif

using settings::locateFile;
using utils::seconds;

namespace camp {
Billboard BB;

GLint pixelShader;
GLint materialShader;
GLint colorShader;
GLint transparentShader;

vertexBuffer material0Data(GL_POINTS);
vertexBuffer material1Data(GL_LINES);
vertexBuffer materialData;
vertexBuffer colorData;
vertexBuffer transparentData;
vertexBuffer triangleData;

const size_t Nbuffer=10000;
const size_t nbuffer=1000;
}

#endif /* HAVE_GL */

#ifdef HAVE_LIBGLM
using camp::Material;
using camp::Maxmaterials;
using camp::Nmaterials;
using camp::nmaterials;
using camp::MaterialMap;

namespace camp {
std::vector<Material> material;
MaterialMap materialMap;
size_t materialIndex;

size_t Maxmaterials;
size_t Nmaterials=1;
size_t nmaterials=48;
}

extern void exitHandler(int);

namespace gl {

bool outlinemode=false;
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

int Mode;

double Aspect;
bool View;
int Oldpid;
string Prefix;
const picture* Picture;
string Format;
int fullWidth,fullHeight;
int Width,Height;
double oWidth,oHeight;
int screenWidth,screenHeight;
int maxTileWidth;
int maxTileHeight;

double Angle;
bool orthographic;
double H;
double xmin,xmax;
double ymin,ymax;
double zmin,zmax;

double Xmin,Xmax;
double Ymin,Ymax;

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

double *Background;
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

using glm::dvec3;
using glm::dmat3;
using glm::mat3;
using glm::mat4;
using glm::dmat4;
using glm::value_ptr;
using glm::translate;

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
  double Zoominv=1.0/lastzoom;
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
      double r=0.5*xsize/(Aspect*Zoom);
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
  if(orthographic) ortho(xmin,xmax,ymin,ymax,-zmax,-zmin);
  else frustum(xmin,xmax,ymin,ymax,-zmax,-zmin);
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
  dView=value_ptr(dviewMat);
  viewMat=mat4(dviewMat);

  drotateMat=dmat4(1.0);

  updateModelViewData();

  remesh=true;
  lastzoom=Zoom=Zoom0;
  setDimensions(Width,Height,0,0);
  framecount=0;
}

#ifdef HAVE_GL

double T[16];

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

#ifdef HAVE_LIBOPENIMAGEIO
GLuint envMapBuf;

GLuint initHDR() {
  GLuint tex;
  glGenTextures(1, &tex);

  auto imagein = OIIO::ImageInput::open(locateFile("res/studio006.hdr").c_str());
  OIIO::ImageSpec const& imspec = imagein->spec();

  // uses GL_TEXTURE1 for now.
  glActiveTexture(GL_TEXTURE1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, tex);
  std::vector<float> pixels(imspec.width*imspec.height*3);
  imagein->read_image(pixels.data());

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imspec.width, imspec.height, 0,
               GL_RGB, GL_FLOAT, pixels.data());

  glGenerateMipmap(GL_TEXTURE_2D);
  imagein->close();

  glActiveTexture(GL_TEXTURE0);
  return tex;
}

#endif
GLint shaderProg,shaderProgColor;

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

void initShaders()
{
  Nlights=nlights == 0 ? 0 : max(Nlights,nlights);
  Nmaterials=max(Nmaterials,nmaterials);

  shaderProg=glCreateProgram();
  string vs=locateFile("shaders/vertex.glsl");
  string fs=locateFile("shaders/fragment.glsl");
  if(vs.empty() || fs.empty()) {
    cerr << "GLSL shaders not found." << endl;
    exit(-1);
  }

  std::vector<std::string> shaderParams;

#if HAVE_LIBOPENIMAGEIO
  if (getSetting<bool>("envmap")) {
    shaderParams.push_back("ENABLE_TEXTURE");
    envMapBuf=initHDR();
  }
#endif

  std::vector<ShaderfileModePair> shaders;
  shaders.push_back(ShaderfileModePair(vs.c_str(),GL_VERTEX_SHADER));
  shaders.push_back(ShaderfileModePair(fs.c_str(),GL_FRAGMENT_SHADER));
  if(orthographic)
    shaderParams.push_back("ORTHOGRAPHIC");

  shaderParams.push_back("WIDTH");
  camp::pixelShader=compileAndLinkShader(shaders,Nlights,Nmaterials,
                                         shaderParams);
  shaderParams.pop_back();

  shaderParams.push_back("NORMAL");
  camp::materialShader=compileAndLinkShader(shaders,Nlights,Nmaterials,
                                            shaderParams);
  shaderParams.push_back("COLOR");
  camp::colorShader=compileAndLinkShader(shaders,Nlights,Nmaterials,
                                         shaderParams);
  shaderParams.push_back("TRANSPARENT");
  camp::transparentShader=compileAndLinkShader(shaders,Nlights,Nmaterials,
                                               shaderParams);
}

void deleteShaders()
{
  glDeleteProgram(camp::transparentShader);
  glDeleteProgram(camp::colorShader);
  glDeleteProgram(camp::materialShader);
  glDeleteProgram(camp::pixelShader);
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
#endif

  if((nlights == 0 && Nlights > 0) || nlights > Nlights ||
     nmaterials > Nmaterials) {
    deleteShaders();
    initShaders();
    lastshader=-1;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  triple m(xmin,ymin,zmin);
  triple M(xmax,ymax,zmax);
  double perspective=orthographic ? 0.0 : 1.0/zmax;

  double size2=hypot(Width,Height);

  if(remesh)
    camp::drawElement::center.clear();

  Picture->render(size2,m,M,perspective,remesh);

  if(!outlinemode) remesh=false;
}

// Return x divided by y rounded up to the nearest integer.
int Quotient(int x, int y)
{
  return (x+y-1)/y;
}

void Export()
{
  glReadBuffer(GL_BACK_LEFT);
  glPixelStorei(GL_PACK_ALIGNMENT,1);
  glFinish();
  try {
    size_t ndata=3*fullWidth*fullHeight;
    unsigned char *data=new unsigned char[ndata];
    if(data) {
      TRcontext *tr=trNew();
      int width=Quotient(fullWidth,Quotient(fullWidth,min(maxTileWidth,Width)));
      int height=Quotient(fullHeight,Quotient(fullHeight,
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
      (orthographic ? trOrtho : trFrustum)(tr,xmin,xmax,ymin,ymax,-zmax,-zmin);

      size_t count=0;
      do {
        trBeginTile(tr);
        fpu_trap(false); // Work around FE_INVALID in OSMesa.
        remesh=true;
        drawscene(fullWidth,fullHeight);
        fpu_trap(settings::trap());
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
      pic.shipout(NULL,Prefix,Format,false,View);
      delete Image;
      delete[] data;
    }
  } catch(handled_error) {
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
  switch(Mode) {
    case 0: // regular
      outlinemode=false;
      nlights=nlights0;
      lastshader=-1;
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      ++Mode;
      break;
    case 1: // outline
      outlinemode=true;
      nlights=0;
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      ++Mode;
      break;
    case 2: // wireframe
      outlinemode=false;
      Mode=0;
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

  setProjection();
  glViewport(0,0,Width,Height);
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
  gettimeofday(&lastframetime,NULL);
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
    if(framecount < 10) // Measure steady-state framerate
      seconds();
    else {
      double s=seconds();
      if(s > 0.0) {
        double rate=1.0/s;
        S.add(rate);
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
  double cz=0.5*(zmin+zmax);

  dviewMat=translate(translate(dmat4(1.0),dvec3(cx,cy,cz))*drotateMat,
                     dvec3(0,0,-cz));
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
    drotateMat=glm::rotate<double>(2*A.angle/lastzoom*ArcballFactor,
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

  double cz=0.5*(zmin+zmax);

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
  glutInitContextProfile(GLUT_CORE_PROFILE);
#endif

  glutInit(&argc,argv);
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

  ctx = OSMesaCreateContextExt(OSMESA_RGBA,16,0,0,NULL);
  if(!ctx) {
    cerr << "OSMesaCreateContext failed." << endl;
    exit(-1);
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
  Background=background;

  Xmin=m.getx();
  Xmax=M.getx();
  Ymin=m.gety();
  Ymax=M.gety();
  zmin=m.getz();
  zmax=M.getz();

  orthographic=Angle == 0.0;
  H=orthographic ? 0.0 : -tan(0.5*Angle)*zmax;

  ignorezoom=false;
  Mode=0;
  Xfactor=Yfactor=1.0;

  pair maxtile=getSetting<pair>("maxtile");
  maxTileWidth=(int) maxtile.getx();
  maxTileHeight=(int) maxtile.gety();
  if(maxTileWidth <= 0) maxTileWidth=1024;
  if(maxTileHeight <= 0) maxTileHeight=768;

  bool webgl=Format == "html";

#ifdef HAVE_GL
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  static bool initializedView=false;
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
    if(!webgl) init();
    Fitscreen=1;
  }
#endif
#endif

  static bool initialized=false;

  if(!(initialized && (interact::interactive ||
                       getSetting<bool>("animating")))) {
    antialias=getSetting<Int>("antialias") > 1;
    double expand;
    if(webgl)
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

    if(webgl) {
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

    home(webgl);
    setProjection();
    if(webgl) return;

    ArcballFactor=1+8.0*hypot(Margin.getx(),Margin.gety())/hypot(Width,Height);

#ifdef HAVE_GL
    for(int i=0; i < 16; ++i)
      T[i]=t[i];

    Aspect=((double) Width)/Height;

    if(maxTileWidth <= 0) maxTileWidth=screenWidth;
    if(maxTileHeight <= 0) maxTileHeight=screenHeight;
#ifdef HAVE_LIBGLUT
    setosize();
#endif

    if(View && settings::verbose > 1)
      cout << "Rendering " << stripDir(prefix) << " as "
           << Width << "x" << Height << " image" << endl;
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

  camp::clearMaterialBuffer();

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
      window=glutCreateWindow(title.c_str());

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
    fpu_trap(false); // Work around FE_INVALID in Gallium
    window=glutCreateWindow("");
    fpu_trap(settings::trap());
    glutHideWindow();
  }
#endif // HAVE_LIBGLUT
#endif // HAVE_LIBOSMESA

  initialized=true;

  GLint val;
  glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE,&val);
  Maxmaterials=val/sizeof(Material);
  if(nmaterials > Maxmaterials) nmaterials=Maxmaterials;

  if(glinitialize) {
    glinitialize=false;
    int result = glewInit();

    if (result != GLEW_OK) {
      cerr << "GLEW initialization error." << endl;
      exit(-1);
    }

    initShaders();
    setBuffers();
  }

  glClearColor(Background[0],Background[1],Background[2],Background[3]);

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

  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  mode();

#ifdef HAVE_LIBOSMESA
  View=false;
#endif

  if(View) {
#ifdef HAVE_LIBGLUT
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
    initializedView=true;
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

void setUniforms(vertexBuffer& data, GLint shader)
{
  bool normal=shader != pixelShader;

  if(shader != gl::lastshader) {
    glUseProgram(shader);
    gl::lastshader=shader;

    glUniform1i(glGetUniformLocation(shader,"nlights"),gl::nlights);

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

#if HAVE_LIBOPENIMAGEIO
    // textures
    if (settings::getSetting<bool>("envmap")) {
      glActiveTexture(GL_TEXTURE1);
      glBindBuffer(GL_TEXTURE_2D, gl::envMapBuf);
      glUniform1i(glGetUniformLocation(shader, "environmentMap"), 1);
      glActiveTexture(GL_TEXTURE0);
    }
#endif
  }

  GLuint binding=0;
  GLint blockindex=glGetUniformBlockIndex(shader,"MaterialBuffer");
  glUniformBlockBinding(shader,blockindex,binding);
  bool copy=gl::remesh || data.partial || !data.rendered;
  registerBuffer(data.materials,data.materialsBuffer,copy,GL_UNIFORM_BUFFER);
  glBindBufferBase(GL_UNIFORM_BUFFER,binding,data.materialsBuffer);

  glUniformMatrix4fv(glGetUniformLocation(shader,"projViewMat"),1,GL_FALSE,
                     value_ptr(gl::projViewMat));

  glUniformMatrix4fv(glGetUniformLocation(shader,"viewMat"),1,GL_FALSE,
                     value_ptr(gl::viewMat));
  if(normal)
    glUniformMatrix3fv(glGetUniformLocation(shader,"normMat"),1,GL_FALSE,
                       value_ptr(gl::normMat));
}

void drawBuffer(vertexBuffer& data, GLint shader)
{
  if(data.indices.empty()) return;

  bool normal=shader != pixelShader;
  bool color=shader == colorShader || shader == transparentShader;

  const size_t size=sizeof(GLfloat);
  const size_t intsize=sizeof(GLint);
  const size_t bytestride=color ? sizeof(VertexData) :
    (normal ? sizeof(vertexData) : sizeof(vertexData0));

  bool copy=gl::remesh || data.partial || !data.rendered;
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
    glVertexAttribPointer(colorAttrib,4,GL_UNSIGNED_BYTE,GL_TRUE,bytestride,
                          (void *) (6*size+intsize));
    glEnableVertexAttribArray(colorAttrib);
  }

  glDrawElements(data.type,data.indices.size(),GL_UNSIGNED_INT,(void *) 0);

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
  drawBuffer(material1Data,materialShader);
  material1Data.clear();
}

void drawMaterial()
{
  drawBuffer(materialData,materialShader);
  materialData.clear();
}

void drawColor()
{
  drawBuffer(colorData,colorShader);
  colorData.clear();
}

void drawTriangle()
{
  drawBuffer(triangleData,transparentShader);
  triangleData.rendered=false; // Force copying of sorted triangles to GPU.
  triangleData.clear();
}

void drawTransparent()
{
  sortTriangles();
  glDepthMask(GL_FALSE); // Enable transparency
  drawBuffer(transparentData,transparentShader);
  transparentData.rendered=false; // Force copying of sorted triangles to GPU.
  glDepthMask(GL_TRUE); // Disable transparency
  transparentData.clear();
}

void drawBuffers()
{
  drawMaterial0();
  drawMaterial1();
  drawMaterial();
  drawColor();
  drawTriangle();
  drawTransparent();
}

void clearMaterialBuffer()
{
  material.clear();
  material.reserve(nmaterials);
  materialMap.clear();
  materialIndex=0;

  material0Data.partial=false;
  material1Data.partial=false;
  materialData.partial=false;
  colorData.partial=false;
  triangleData.partial=false;
  transparentData.partial=false;
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
    data.materials.push_back(material[materialIndex]);
  }
  materialIndex=data.materialTable[materialIndex];
}

}
#endif /* HAVE_GL */
