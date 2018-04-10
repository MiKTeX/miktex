/*****
 * glrender.cc
 * John Bowman and Orest Shardt
 * Render 3D Bezier paths and surfaces.
 *****/

#ifdef __CYGWIN__
#define _POSIX_C_SOURCE 200809L
#endif

#if defined(MIKTEX)
#include <miktex/asy-first.h>
#include <miktex/asy.h>
#endif
#include <stdlib.h>
#include <fstream>
#include <cstring>
#include <sys/time.h>

#include "common.h"

#ifdef HAVE_GL

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

#include "picture.h"
#include "arcball.h"
#include "bbox3.h"
#include "drawimage.h"
#include "interact.h"
#include "tr.h"

#ifdef HAVE_LIBGLUT
#ifdef FREEGLUT
#include <GL/freeglut_ext.h>
#endif
#endif

namespace camp {
billboard BB;
}

namespace gl {
  
bool outlinemode=false;
bool glthread=false;
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
bool Menu;
bool Motion;
bool ignorezoom;
int Fitscreen;

bool queueExport=false;
bool readyAfterExport=false;

#ifdef HAVE_LIBGLUT
timeval lasttime;
timeval lastframetime;
int oldWidth,oldHeight;

bool Xspin,Yspin,Zspin;
bool Animate;
bool Step;

bool queueScreen=false;

int x0,y0;
string Action;
int MenuButton;

double lastangle;
Arcball arcball;
int window;
#endif

double Aspect;
bool View;
int Oldpid;
string Prefix;
const picture* Picture;
string Format;
int Width,Height;
int fullWidth,fullHeight;
double oWidth,oHeight;
int screenWidth,screenHeight;
int maxWidth;
int maxHeight;
int maxTileWidth;
int maxTileHeight;

double T[16];

int Mode;

double Angle;
bool orthographic;
double H;
double xmin,xmax;
double ymin,ymax;
double zmin,zmax;

double Xmin,Xmax;
double Ymin,Ymax;

pair Shift;
double X,Y;
double cx,cy;
double Xfactor,Yfactor;

static const double pi=acos(-1.0);
static const double degrees=180.0/pi;
static const double radians=1.0/degrees;

double Background[4];
size_t Nlights;
triple *Lights; 
double *Diffuse;
double *Ambient;
double *Specular;
bool ViewportLighting;
bool antialias;

double Zoom;
double Zoom0;
double lastzoom;

GLfloat Rotate[16];  
GLUnurbs *nurb=NULL;

void *glrenderWrapper(void *a);

#ifdef HAVE_LIBOSMESA
OSMesaContext ctx;
unsigned char *osmesa_buffer;
#endif

#ifdef HAVE_PTHREAD
pthread_t mainthread;

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
}
#endif

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

void lighting()
{
  for(size_t i=0; i < Nlights; ++i) {
    GLenum index=GL_LIGHT0+i;
    triple Lighti=Lights[i];
    GLfloat position[]={(GLfloat) Lighti.getx(),(GLfloat) Lighti.gety(),
			(GLfloat) Lighti.getz(),0.0};
    glLightfv(index,GL_POSITION,position);
  }
}

void initlighting() 
{
  glClearColor(Background[0],Background[1],Background[2],Background[3]);
  glEnable(GL_LIGHTING);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,getSetting<bool>("twosided"));
    
  for(size_t i=0; i < Nlights; ++i) {
    GLenum index=GL_LIGHT0+i;
    glEnable(index);
    
    size_t i4=4*i;
    
    GLfloat diffuse[]={(GLfloat) Diffuse[i4],(GLfloat) Diffuse[i4+1],
		       (GLfloat) Diffuse[i4+2],(GLfloat) Diffuse[i4+3]};
    glLightfv(index,GL_DIFFUSE,diffuse);
    
    GLfloat ambient[]={(GLfloat) Ambient[i4],(GLfloat) Ambient[i4+1],
		       (GLfloat) Ambient[i4+2],(GLfloat) Ambient[i4+3]};
    glLightfv(index,GL_AMBIENT,ambient);
    
    GLfloat specular[]={(GLfloat) Specular[i4],(GLfloat) Specular[i4+1],
			(GLfloat) Specular[i4+2],(GLfloat) Specular[i4+3]};
    glLightfv(index,GL_SPECULAR,specular);
  }
  
  static size_t lastNlights=0;
  for(size_t i=Nlights; i < lastNlights; ++i) {
    GLenum index=GL_LIGHT0+i;
    glDisable(index);
  }
  lastNlights=Nlights;
  
  if(ViewportLighting)
    lighting();
}

void setDimensions(int Width, int Height, double X, double Y)
{
  double Aspect=((double) Width)/Height;
  double xshift=X/Width*lastzoom+Shift.getx()*Xfactor;
  double yshift=Y/Height*lastzoom+Shift.gety()*Yfactor;
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

void setProjection()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  setDimensions(Width,Height,X,Y);
  if(orthographic)
    glOrtho(xmin,xmax,ymin,ymax,-zmax,-zmin);
  else
    glFrustum(xmin,xmax,ymin,ymax,-zmax,-zmin);
  glMatrixMode(GL_MODELVIEW);
#ifdef HAVE_LIBGLUT
  double arcballRadius=getSetting<double>("arcballradius");
  arcball.set_params(vec2(0.5*Width,0.5*Height),arcballRadius*Zoom);
#endif
}

void drawscene(double Width, double Height)
{
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  static bool first=true;
  if(glthread && first && !getSetting<bool>("offscreen")) {
    wait(initSignal,initLock);
    endwait(initSignal,initLock);
    first=false;
  }
#endif

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(!ViewportLighting) 
    lighting();
    
  triple m(xmin,ymin,zmin);
  triple M(xmax,ymax,zmax);
  double perspective=orthographic ? 0.0 : 1.0/zmax;
  
  double size2=hypot(Width,Height);
  
  // Render opaque objects
  Picture->render(nurb,size2,m,M,perspective,Nlights,false);
  
  // Enable transparency
  glDepthMask(GL_FALSE);
  
  // Render transparent objects
  Picture->render(nurb,size2,m,M,perspective,Nlights,true);
  glDepthMask(GL_TRUE);
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

      trTileSize(tr,width,height,0);
      trImageSize(tr,fullWidth,fullHeight);
      trImageBuffer(tr,GL_RGB,GL_UNSIGNED_BYTE,data);

      setDimensions(fullWidth,fullHeight,X/Width*fullWidth,Y/Width*fullWidth);
      (orthographic ? trOrtho : trFrustum)(tr,xmin,xmax,ymin,ymax,-zmax,-zmin);
   
      size_t count=0;
      do {
        trBeginTile(tr);
        drawscene(fullWidth,fullHeight);
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
      pic.shipout(NULL,Prefix,Format,0.0,false,View);
      delete Image;
      delete[] data;
    } 
  } catch(handled_error) {
  } catch(std::bad_alloc&) {
    outOfMemory();
  }
  setProjection();
  bool offscreen=getSetting<bool>("offscreen");
#ifdef HAVE_LIBGLUT
  if(!offscreen)
    glutPostRedisplay();
#endif

#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  if(glthread && readyAfterExport && !offscreen) {
    readyAfterExport=false;        
    endwait(readySignal,readyLock);
  }
#endif
}

#ifdef HAVE_LIBGLUT
void idle() 
{
  glutIdleFunc(NULL);
  Xspin=Yspin=Zspin=Animate=Step=false;
}
#endif

void home() 
{
  X=Y=cx=cy=0.0;
#ifdef HAVE_LIBGLUT
  if(!getSetting<bool>("offscreen")) {
    idle();
    arcball.init();
  }
#endif
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX,Rotate);
  lastzoom=Zoom=Zoom0;
  setDimensions(Width,Height,0,0);
  initlighting();
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
  if(getSetting<bool>("offscreen")) {
    if(osmesa_buffer) delete[] osmesa_buffer;
    if(ctx) OSMesaDestroyContext(ctx);
    exit(0);
  }
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
  } else {
    glutDestroyWindow(window);
    exit(0);
  }
#endif
}
  
void mode() 
{
  switch(Mode) {
    case 0:
      for(size_t i=0; i < Nlights; ++i) 
        glEnable(GL_LIGHT0+i);
      outlinemode=false;
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      gluNurbsProperty(nurb,GLU_DISPLAY_MODE,GLU_FILL);
      ++Mode;
      break;
    case 1:
      for(size_t i=0; i < Nlights; ++i) 
        glDisable(GL_LIGHT0+i);
      outlinemode=true;
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      gluNurbsProperty(nurb,GLU_DISPLAY_MODE,GLU_OUTLINE_PATCH);
      ++Mode;
      break;
    case 2:
      outlinemode=false;
      gluNurbsProperty(nurb,GLU_DISPLAY_MODE,GLU_OUTLINE_POLYGON);
      Mode=0;
      break;
  }
#ifdef HAVE_LIBGLUT
  if(!getSetting<bool>("offscreen"))
    glutPostRedisplay();
#endif
}

// GUI-related functions
#ifdef HAVE_LIBGLUT
bool capsize(int& width, int& height) 
{
  bool resize=false;
  if(width > maxWidth) {
    width=maxWidth;
    resize=true;
  }
  if(height > maxHeight) {
    height=maxHeight;
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
  }
  glutReshapeWindow(w,h);
  reshape0(w,h);
  glutPostRedisplay();
}

void fullscreen(bool reposition=true) 
{
  Width=screenWidth;
  Height=screenHeight;
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
      setsize(oldWidth,oldHeight,reposition);
      break;
    }
    case 1: // Fit to screen in one dimension
    {       
      oldWidth=Width;
      oldHeight=Height;
      int w=screenWidth;
      int h=screenHeight;
      if(w >= h*Aspect) w=(int) (h*Aspect+0.5);
      else h=(int) (w/Aspect+0.5);
      setsize(w,h,reposition);
      break;
    }
    case 2: // Full screen
    {
      Xfactor=((double) screenHeight)/Height;
      Yfactor=((double) screenWidth)/Width;
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
  drawscene(Width,Height);
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
  lastzoom=Zoom;
  glLoadIdentity();
  double cz=0.5*(zmin+zmax);
  glTranslatef(cx,cy,cz);
  glMultMatrixf(Rotate);
  glTranslatef(0,0,-cz);
  setProjection();
  glutPostRedisplay();
}

void updateHandler(int)
{
  queueScreen=true;
  update();
  if(interact::interactive || !Animate) {
    glutShowWindow();
  }
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
  }
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
}
  
void shift(int x, int y)
{
  if(x > 0 && y > 0) {
    double Zoominv=1.0/Zoom;
    X += (x-x0)*Zoominv;
    Y += (y0-y)*Zoominv;
    x0=x; y0=y;
    update();
  }
}
  
void pan(int x, int y)
{
  if(x > 0 && y > 0) {
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
}
  
void capzoom() 
{
  static double maxzoom=sqrt(DBL_MAX);
  static double minzoom=1/maxzoom;
  if(Zoom <= minzoom) Zoom=minzoom;
  if(Zoom >= maxzoom) Zoom=maxzoom;
  
}

int menustatus=GLUT_MENU_NOT_IN_USE;

void menuStatus(int status, int x, int y) 
{
  menustatus=status;
}
  
void disableMenu() 
{
  Menu=false;
  if(menustatus == GLUT_MENU_NOT_IN_USE)
    glutDetachMenu(MenuButton);
}

void zoom(int x, int y)
{
  if(ignorezoom) {ignorezoom=false; y0=y; return;}
  if(x > 0 && y > 0) {
    if(Menu) {
      disableMenu();
      y0=y;
      return;
    }
    Motion=true;
    double zoomFactor=getSetting<double>("zoomfactor");
    if(zoomFactor > 0.0) {
      double zoomStep=getSetting<double>("zoomstep");
      const double limit=log(0.1*DBL_MAX)/log(zoomFactor);
      double s=zoomStep*(y0-y);
      if(fabs(s) < limit) {
        Zoom *= pow(zoomFactor,s);
        capzoom();
        lastzoom=Zoom;
        y0=y;
        setProjection();
        glutPostRedisplay();
      }
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
    lastzoom=Zoom;
    setProjection();
    glutPostRedisplay();
  }
}

void rotate(int x, int y)
{
  if(x > 0 && y > 0) {
    if(Menu) {
      disableMenu();
      arcball.mouse_down(x,Height-y);
      return;
    }
    Motion=true;
    arcball.mouse_motion(x,Height-y,0,
                         Action == "rotateX", // X rotation only
                         Action == "rotateY");  // Y rotation only

    for(int i=0; i < 4; ++i) {
      const vec4& roti=arcball.rot[i];
      int i4=4*i;
      for(int j=0; j < 4; ++j)
        Rotate[i4+j]=roti[j];
    }
    
    update();
  }
}
  
double Degrees(int x, int y) 
{
  return atan2(0.5*Height-y-Y,x-0.5*Width-X)*degrees;
}

void updateArcball() 
{
  for(int i=0; i < 4; ++i) {
    int i4=4*i;
    vec4& roti=arcball.rot[i];
    for(int j=0; j < 4; ++j)
      roti[j]=Rotate[i4+j];
  }
  update();
}

void rotateX(double step) 
{
  glLoadIdentity();
  glRotatef(step,1,0,0);
  glMultMatrixf(Rotate);
  glGetFloatv(GL_MODELVIEW_MATRIX,Rotate);
  updateArcball();
}

void rotateY(double step) 
{
  glLoadIdentity();
  glRotatef(step,0,1,0);
  glMultMatrixf(Rotate);
  glGetFloatv(GL_MODELVIEW_MATRIX,Rotate);
  updateArcball();
}

void rotateZ(double step) 
{
  glLoadIdentity();
  glRotatef(step,0,0,1);
  glMultMatrixf(Rotate);
  glGetFloatv(GL_MODELVIEW_MATRIX,Rotate);
  updateArcball();
}

void rotateZ(int x, int y)
{
  if(x > 0 && y > 0) {
    if(Menu) {
      disableMenu();
      return;
    }
    Motion=true;
    double angle=Degrees(x,y);
    rotateZ(angle-lastangle);
    lastangle=angle;
  }
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
  if(Menu) disableMenu();
}

void mouse(int button, int state, int x, int y)
{
  int mod=glutGetModifiers();
  string Action=action(button,mod);

  if(!Menu) {
    if(mod == 0 && state == GLUT_UP && !Motion && Action == "zoom/menu") {
      MenuButton=button;
      glutMotionFunc(NULL);
      glutTimerFunc(getSetting<Int>("doubleclick"),timeout,0);
      glutAttachMenu(button);
      Menu=true;
      return;
    } else Motion=false;
  }
  
  disableMenu();
  
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
    if(Action == "rotate" || Action == "rotateX" || Action == "rotateY") {
      arcball.mouse_down(x,Height-y);
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
    } else if(Action == "rotateZ") {
      lastangle=Degrees(x,y);
      glutMotionFunc(rotateZ);
    }
  } else {
    arcball.mouse_up();
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

void write(const char *text, const double *v)
{
  cout << text << "=(" << v[0] << "," << v[1] << "," << v[2] << ")";
}

void showCamera()
{
  projection P=camera();
  cout << endl
       << "currentprojection=" 
       << (P.orthographic ? "orthographic(" : "perspective(")  << endl
       << "camera=" << P.camera << "," << endl
       << "up=" << P.up << "," << endl
       << "target=" << P.target << "," << endl
       << "zoom=" << P.zoom;
  if(!orthographic)
    cout << "," << endl << "angle=" << P.angle;
  if(P.viewportshift != pair(0.0,0.0))
    cout << "," << endl << "viewportshift=" << P.viewportshift;
  if(!orthographic)
    cout << "," << endl << "autoadjust=false";
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
 
enum Menu {HOME,FITSCREEN,XSPIN,YSPIN,ZSPIN,STOP,MODE,EXPORT,CAMERA,
           PLAY,STEP,REVERSE,QUIT};

void menu(int choice)
{
  if(Menu) disableMenu();
  ignorezoom=true;
  Motion=true;
  switch (choice) {
    case HOME: // Home
      home();
      update();
      break;
    case FITSCREEN:
      togglefitscreen();
      break;
    case XSPIN:
      spinx();
      break;
    case YSPIN:
      spiny();
      break;
    case ZSPIN:
      spinz();
      break;
    case STOP:
      idle();
      break;
    case MODE:
      mode();
      break;
    case EXPORT:
      queueExport=true;
      break;
    case CAMERA:
      showCamera();
      break;
    case PLAY:
      if(getSetting<bool>("reverse")) Animate=false;
      Setting("reverse")=Step=false;
      animate();
      break;
    case REVERSE:
      if(!getSetting<bool>("reverse")) Animate=false;
      Setting("reverse")=true;
      Step=false;
      animate();
      break;
    case STEP:
      Step=true;
      animate();
      break;
    case QUIT:
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
  bool offscreen=getSetting<bool>("offscreen");
  if(!Iconify && !offscreen)
    glutShowWindow();
#endif  
  readyAfterExport=true;
  Export();
#ifdef HAVE_LIBGLUT  
  if(!Iconify && !offscreen)
    glutHideWindow();
#endif  
  glutDisplayFunc(nodisplay);
}

static bool glinitialize=true;

projection camera(bool user)
{
  if(glinitialize) return projection();
                   
  camp::Triple vCamera,vUp,vTarget;
  
  double cz=0.5*(zmin+zmax);

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
                    pair(X/Width*lastzoom+Shift.getx(),
                         Y/Height*lastzoom+Shift.gety()));
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

// angle=0 means orthographic.
void glrender(const string& prefix, const picture *pic, const string& format,
              double width, double height, double angle, double zoom,
              const triple& m, const triple& M, const pair& shift, double *t,
              double *background, size_t nlights, triple *lights,
              double *diffuse, double *ambient, double *specular,
              bool Viewportlighting, bool view, int oldpid)
{
  bool offscreen=getSetting<bool>("offscreen");
  Iconify=getSetting<bool>("iconify");
  
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  static bool initializedView=false;
#endif  

  width=max(width,1.0);
  height=max(height,1.0);
  
  if(zoom == 0.0) zoom=1.0;
  
  Prefix=prefix;
  Picture=pic;
  Format=format;
  for(int i=0; i < 16; ++i)
    T[i]=t[i];
  for(int i=0; i < 4; ++i)
    Background[i]=background[i];
  
  Nlights=min(nlights,(size_t) GL_MAX_LIGHTS);
  Lights=lights;
  Diffuse=diffuse;
  Ambient=ambient;
  Specular=specular;
  ViewportLighting=Viewportlighting;
  View=view;
  Angle=angle*radians;
  Zoom0=zoom;
  Oldpid=oldpid;
  Shift=shift;
  
  Xmin=m.getx();
  Xmax=M.getx();
  Ymin=m.gety();
  Ymax=M.gety();
  zmin=m.getz();
  zmax=M.getz();
  
  orthographic=Angle == 0.0;
  H=orthographic ? 0.0 : -tan(0.5*Angle)*zmax;
    
  Menu=false;
  Motion=true;
  ignorezoom=false;
  Mode=0;
  Xfactor=Yfactor=1.0;
  
  pair maxtile=getSetting<pair>("maxtile");
  maxTileWidth=(int) maxtile.getx();
  maxTileHeight=(int) maxtile.gety();
  if(maxTileWidth <= 0) maxTileWidth=1024;
  if(maxTileHeight <= 0) maxTileHeight=768;

  if(offscreen) {
    screenWidth=maxTileWidth;
    screenHeight=maxTileHeight;

    static bool osmesa_initialized=false;
    if(!osmesa_initialized) {
      osmesa_initialized=true;
      init_osmesa();
    }
  }
  
  if(glinitialize) {
    glinitialize=false;
    init();
    Fitscreen=1;
  }

  static bool initialized=false;
  if(!initialized || !interact::interactive) {
    antialias=getSetting<Int>("antialias") > 1;
    double expand=getSetting<double>("render");
    if(expand < 0)
      expand *= (Format.empty() || Format == "eps" || Format == "pdf") 
        ? -2.0 : -1.0;
    if(antialias) expand *= 2.0;
  
    // Force a hard viewport limit to work around direct rendering bugs.
    // Alternatively, one can use -glOptions=-indirect (with a performance
    // penalty).
    pair maxViewport=getSetting<pair>("maxviewport");
    maxWidth=(int) ceil(maxViewport.getx());
    maxHeight=(int) ceil(maxViewport.gety());
    if(maxWidth <= 0) maxWidth=max(maxHeight,2);
    if(maxHeight <= 0) maxHeight=max(maxWidth,2);
    if(screenWidth <= 0) screenWidth=maxWidth;
    else screenWidth=min(screenWidth,maxWidth);
    if(screenHeight <= 0) screenHeight=maxHeight;
    else screenHeight=min(screenHeight,maxHeight);
  
    oWidth=width;
    oHeight=height;
    Aspect=width/height;
  
    fullWidth=(int) ceil(expand*width);
    fullHeight=(int) ceil(expand*height);
  
    Width=min(fullWidth,screenWidth);
    Height=min(fullHeight,screenHeight);
  
    if(Width > Height*Aspect) 
      Width=min((int) (ceil(Height*Aspect)),screenWidth);
    else 
      Height=min((int) (ceil(Width/Aspect)),screenHeight);
  
    Aspect=((double) Width)/Height;

    if(maxTileWidth <= 0) maxTileWidth=screenWidth;
    if(maxTileHeight <= 0) maxTileHeight=screenHeight;
#ifdef HAVE_LIBGLUT    
    setosize();
#endif
    
    if(View && settings::verbose > 1) 
      cout << "Rendering " << stripDir(prefix) << " as "
           << Width << "x" << Height << " image" << endl;
  }

  bool havewindow=initialized && glthread && !offscreen;
  
#ifdef HAVE_LIBGLUT    
  unsigned int displaymode=GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH;
  
  int buttons[]={GLUT_LEFT_BUTTON,GLUT_MIDDLE_BUTTON,GLUT_RIGHT_BUTTON};
  string buttonnames[]={"left","middle","right"};
  size_t nbuttons=sizeof(buttons)/sizeof(int);
#endif  
  
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
  if(glthread && initializedView && !offscreen) {
    if(!View)
      readyAfterExport=queueExport=true;
#if defined(MIKTEX) && !defined(HAVE_PTHREAD)
    *MiKTeX::Aymptote::sigusr1 = true;
#else
    pthread_kill(mainthread,SIGUSR1);
#endif
    return;
  }
#endif    
  
#ifdef HAVE_LIBGLUT
  if(!offscreen) {
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

      ostringstream buf;
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
        string suffix;
        for(size_t i=0; i < nbuttons; ++i) {
          int button=buttons[i];
          if(action(button,0) == "zoom/menu") {
            suffix="Double click "+buttonnames[i]+" button for menu";
            break;
          }
        }
        if(suffix.empty()) {
          for(size_t i=0; i < nbuttons; ++i) {
            int button=buttons[i];
            if(action(button,0) == "menu") {
              suffix="Click "+buttonnames[i]+" button for menu";
              break;
            }
          }
        }
      
        title += " ["+suffix+"]";
    
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
      window=glutCreateWindow("");
      glutHideWindow();
    }
  }
#endif // HAVE_LIBGLUT
  initialized=true;
  
  glMatrixMode(GL_MODELVIEW);
    
  home();
    
#ifdef HAVE_LIBGLUT
  if(!offscreen) {
    Animate=getSetting<bool>("autoplay") && glthread;
  
    if(View) {
      if(!getSetting<bool>("fitscreen"))
        Fitscreen=0;
      fitscreen();
      setosize();
    }
  }
#endif
  
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MAP1_VERTEX_3);
  glEnable(GL_MAP1_VERTEX_4);
  glEnable(GL_MAP2_VERTEX_3);
  glEnable(GL_MAP2_VERTEX_4);
  glEnable(GL_MAP2_COLOR_4);
  
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  
  if(nurb == NULL) {
    nurb=gluNewNurbsRenderer();
    if(nurb == NULL) 
      outOfMemory();
    gluNurbsProperty(nurb,GLU_SAMPLING_METHOD,GLU_PARAMETRIC_ERROR);
    gluNurbsProperty(nurb,GLU_SAMPLING_TOLERANCE,0.5);
    gluNurbsProperty(nurb,GLU_PARAMETRIC_TOLERANCE,1.0);
    gluNurbsProperty(nurb,GLU_CULLING,GLU_TRUE);
  
    // The callback tessellation algorithm avoids artifacts at degenerate
    // control points.
    gluNurbsProperty(nurb,GLU_NURBS_MODE,GLU_NURBS_TESSELLATOR);
    gluNurbsCallback(nurb,GLU_NURBS_BEGIN,(_GLUfuncptr) glBegin);
    gluNurbsCallback(nurb,GLU_NURBS_VERTEX,(_GLUfuncptr) glVertex3fv);
    gluNurbsCallback(nurb,GLU_NURBS_END,(_GLUfuncptr) glEnd);
    gluNurbsCallback(nurb,GLU_NURBS_COLOR,(_GLUfuncptr) glColor4fv);
  }
  
  mode();
  
  if(View && !offscreen) {
#ifdef HAVE_LIBGLUT
#if defined(MIKTEX) || defined(HAVE_PTHREAD)
    initializedView=true;
#endif    
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
  
    glutCreateMenu(menu);
    glutAddMenuEntry("(h) Home",HOME);
    glutAddMenuEntry("(f) Fitscreen",FITSCREEN);
    glutAddMenuEntry("(x) X spin",XSPIN);
    glutAddMenuEntry("(y) Y spin",YSPIN);
    glutAddMenuEntry("(z) Z spin",ZSPIN);
    glutAddMenuEntry("(s) Stop",STOP);
    glutAddMenuEntry("(m) Mode",MODE);
    glutAddMenuEntry("(e) Export",EXPORT);
    glutAddMenuEntry("(c) Camera",CAMERA);
    glutAddMenuEntry("(p) Play",PLAY);
    glutAddMenuEntry("(r) Reverse",REVERSE);
    glutAddMenuEntry("( ) Step",STEP);
    glutAddMenuEntry("(q) Quit" ,QUIT);
    glutMenuStatusFunc(menuStatus);
  
    for(size_t i=0; i < nbuttons; ++i) {
      int button=buttons[i];
      if(action(button,0) == "menu")
        glutAttachMenu(button);
    }
    
    glutMainLoop();
#endif // HAVE_LIBGLUT
  } else {
    if(glthread && !offscreen) {
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
}

} // namespace gl

#endif
