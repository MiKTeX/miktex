/* This file is released under version 2 of the GNU Library General Public
 * License (see the files LICENSE.LIBRARY and LICENSE).
 */

/* $Id: tr.h,v 1.5 1997/07/21 17:34:07 brianp Exp $ */

/*
 * $Log: tr.h,v $
 * Revision 1.5  1997/07/21  17:34:07  brianp
 * added tile borders, incremented version to 1.1
 *
 * Revision 1.4  1997/07/21  15:47:35  brianp
 * renamed all "near" and "far" variables
 *
 * Revision 1.3  1997/04/26  21:23:25  brianp
 * added trRasterPos3f function
 *
 * Revision 1.2  1997/04/19  23:26:10  brianp
 * many API changes
 *
 * Revision 1.1  1997/04/18  21:53:05  brianp
 * Initial revision
 *
 */


/*
 * Tiled Rendering library
 * Version 1.1
 * Copyright (C) Brian Paul
 *
 *
 * This library allows one to render arbitrarily large images with OpenGL.
 * The basic idea is to break the image into tiles which are rendered one
 * at a time.  The tiles are assembled together to form the final, large
 * image.  Tiles and images can be of any size.
 *
 * Basic usage:
 *
 * 1. Allocate a tile rendering context:
 *       TRcontext t = trNew();
 *
 * 2. Specify the final image buffer and tile size:
 *       GLubyte image[W][H][4]
 *       trImageSize(t, W, H);
 *       trImageBuffer(t, GL_RGBA, GL_UNSIGNED_BYTE, (GLubyte *) image);
 *
 * 3. Setup your projection:
 *       trFrustum(t, left, right, bottom top, near, far);
 *    or
 *       trOrtho(t, left, right, bottom top, near, far);
 *    or
 *       trPerspective(t, fovy, aspect, near, far);
 *
 * 4. Render the tiles:
 *       do {
 *           trBeginTile(t);
 *           DrawMyScene();
 *       } while (trEndTile(t));
 *
 *    You provide the DrawMyScene() function which calls glClear() and
 *    draws all your stuff.
 *
 * 5. The image array is now complete.  Display it, write it to a file, etc.
 *
 * 6. Delete the tile rendering context when finished:
 *       trDelete(t);
 *
 */


#ifndef TR_H
#define TR_H


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else

#ifdef __MSDOS__
#include </usr/include/w32api/GL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif


#define TR_VERSION "1.1"
#define TR_MAJOR_VERSION 1
#define TR_MINOR_VERSION 1


typedef struct _TRctx TRcontext;


typedef enum {
   TR_TILE_WIDTH = 100,
   TR_TILE_HEIGHT,
   TR_TILE_BORDER,
   TR_IMAGE_WIDTH,
   TR_IMAGE_HEIGHT,
   TR_ROWS,
   TR_COLUMNS,
   TR_CURRENT_ROW,
   TR_CURRENT_COLUMN,
   TR_CURRENT_TILE_WIDTH,
   TR_CURRENT_TILE_HEIGHT,
   TR_ROW_ORDER,
   TR_TOP_TO_BOTTOM,
   TR_BOTTOM_TO_TOP
} TRenum;



extern TRcontext *trNew(void);

extern void trDelete(TRcontext *tr);


extern void trTileSize(TRcontext *tr, GLint width, GLint height, GLint border);

extern void trTileBuffer(TRcontext *tr, GLenum format, GLenum type,
			 GLvoid *image);


extern void trImageSize(TRcontext *tr, GLint width, GLint height);

extern void trImageBuffer(TRcontext *tr, GLenum format, GLenum type,
			  GLvoid *image);


extern void trRowOrder(TRcontext *tr, TRenum order);


extern GLint trGet(TRcontext *tr, TRenum param);


extern void trOrtho(TRcontext *tr,
		    GLdouble left, GLdouble right,
		    GLdouble bottom, GLdouble top,
		    GLdouble zNear, GLdouble zFar);

extern void trFrustum(TRcontext *tr,
		      GLdouble left, GLdouble right,
		      GLdouble bottom, GLdouble top,
		      GLdouble zNear, GLdouble zFar);

extern void trPerspective(TRcontext *tr,
			  GLdouble fovy, GLdouble aspect,
			  GLdouble zNear, GLdouble zFar );


extern void trBeginTile(TRcontext *tr);

extern int trEndTile(TRcontext *tr);


extern void trRasterPos3f(TRcontext *tr, GLfloat x, GLfloat y, GLfloat z);



#ifdef __cplusplus
}
#endif


#endif
