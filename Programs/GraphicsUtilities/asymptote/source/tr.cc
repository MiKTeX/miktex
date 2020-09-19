/* This file is released under version 2 of the GNU Library General Public
 * License (see the files LICENSE.LIBRARY and LICENSE).
 */

/* $Id: tr.c,v 1.9 1998/01/29 16:56:54 brianp Exp $ */

/*
 * $Log: tr.c,v $
 * Revision 1.9  1998/01/29  16:56:54  brianp
 * allow trOrtho() and trFrustum() to be called at any time, minor clean-up
 *
 * Revision 1.8  1998/01/28  19:47:39  brianp
 * minor clean-up for C++
 *
 * Revision 1.7  1997/07/21  17:34:38  brianp
 * added tile borders
 *
 * Revision 1.6  1997/07/21  15:47:35  brianp
 * renamed all "near" and "far" variables
 *
 * Revision 1.5  1997/04/26  21:23:25  brianp
 * added trRasterPos3f function
 *
 * Revision 1.4  1997/04/26  19:59:36  brianp
 * set CurrentTile to -1 before first tile and after last tile
 *
 * Revision 1.3  1997/04/22  23:51:15  brianp
 * added WIN32 header stuff, removed tabs
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
 */

#include "common.h"

#ifdef HAVE_GL

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "tr.h"
#ifdef WIN32
#include <windows.h>
#endif

#define DEFAULT_TILE_WIDTH  256
#define DEFAULT_TILE_HEIGHT 256
#define DEFAULT_TILE_BORDER 0

namespace gl {
void frustum(GLdouble left, GLdouble right, GLdouble bottom,
             GLdouble top, GLdouble nearVal, GLdouble farVal);
void ortho(GLdouble left, GLdouble right, GLdouble bottom,
           GLdouble top, GLdouble nearVal, GLdouble farVal);
}

using gl::frustum;
using gl::ortho;

struct _TRctx {
   /* Final image parameters */
   GLint ImageWidth, ImageHeight;
   GLenum ImageFormat, ImageType;
   GLvoid *ImageBuffer;

   /* Tile parameters */
   GLint TileWidth, TileHeight;
   GLint TileWidthNB, TileHeightNB;
   GLint TileBorder;
   GLenum TileFormat, TileType;
   GLvoid *TileBuffer;

   /* Projection parameters */
   GLboolean Perspective;
   GLdouble Left;
   GLdouble Right;
   GLdouble Bottom;
   GLdouble Top;
   GLdouble Near;
   GLdouble Far;

   /* Misc */
   TRenum RowOrder;
   GLint Rows, Columns;
   GLint CurrentTile;
   GLint CurrentTileWidth, CurrentTileHeight;
   GLint CurrentRow, CurrentColumn;

   GLint ViewportSave[4];
};



/*
 * Misc setup including computing number of tiles (rows and columns).
 */
static void Setup(TRcontext *tr)
{
   if (!tr)
      return;

   tr->Columns = (tr->ImageWidth + tr->TileWidthNB - 1) / tr->TileWidthNB;
   tr->Rows = (tr->ImageHeight + tr->TileHeightNB - 1) / tr->TileHeightNB;
   tr->CurrentTile = 0;

   assert(tr->Columns >= 0);
   assert(tr->Rows >= 0);
}



TRcontext *trNew(void)
{
   TRcontext *tr = (TRcontext *) calloc(1, sizeof(TRcontext));
   if (tr) {
      tr->TileWidth = DEFAULT_TILE_WIDTH;
      tr->TileHeight = DEFAULT_TILE_HEIGHT;
      tr->TileBorder = DEFAULT_TILE_BORDER;
      tr->RowOrder = TR_BOTTOM_TO_TOP;
      tr->CurrentTile = -1;
   }
   return (TRcontext *) tr;
}


void trDelete(TRcontext *tr)
{
   if (tr)
      free(tr);
}



void trTileSize(TRcontext *tr, GLint width, GLint height, GLint border)
{
   if (!tr)
      return;

   assert(border >= 0);
   assert(width >= 1);
   assert(height >= 1);
   assert(width >= 2*border);
   assert(height >= 2*border);

   tr->TileBorder = border;
   tr->TileWidth = width;
   tr->TileHeight = height;
   tr->TileWidthNB = width - 2 * border;
   tr->TileHeightNB = height - 2 * border;
   Setup(tr);
}



void trTileBuffer(TRcontext *tr, GLenum format, GLenum type, GLvoid *image)
{
   if (!tr)
      return;

   tr->TileFormat = format;
   tr->TileType = type;
   tr->TileBuffer = image;
}



void trImageSize(TRcontext *tr, GLint width, GLint height)
{
   if (!tr)
      return;

   tr->ImageWidth = width;
   tr->ImageHeight = height;
   Setup(tr);
}


void trImageBuffer(TRcontext *tr, GLenum format, GLenum type, GLvoid *image)
{
   if (!tr)
      return;

   tr->ImageFormat = format;
   tr->ImageType = type;
   tr->ImageBuffer = image;
}


GLint trGet(TRcontext *tr, TRenum param)
{
   if (!tr)
      return 0;

   switch (param) {
      case TR_TILE_WIDTH:
         return tr->TileWidth;
      case TR_TILE_HEIGHT:
         return tr->TileHeight;
      case TR_TILE_BORDER:
         return tr->TileBorder;
      case TR_IMAGE_WIDTH:
         return tr->ImageWidth;
      case TR_IMAGE_HEIGHT:
         return tr->ImageHeight;
      case TR_ROWS:
         return tr->Rows;
      case TR_COLUMNS:
         return tr->Columns;
      case TR_CURRENT_ROW:
         if (tr->CurrentTile<0)
            return -1;
         else
            return tr->CurrentRow;
      case TR_CURRENT_COLUMN:
         if (tr->CurrentTile<0)
            return -1;
         else
            return tr->CurrentColumn;
      case TR_CURRENT_TILE_WIDTH:
         return tr->CurrentTileWidth;
      case TR_CURRENT_TILE_HEIGHT:
         return tr->CurrentTileHeight;
      case TR_ROW_ORDER:
         return (GLint) tr->RowOrder;
      default:
         return 0;
   }
}


void trRowOrder(TRcontext *tr, TRenum order)
{
   if (!tr)
      return;

   if (order==TR_TOP_TO_BOTTOM || order==TR_BOTTOM_TO_TOP)
      tr->RowOrder = order;
}


void trOrtho(TRcontext *tr,
             GLdouble left, GLdouble right,
             GLdouble bottom, GLdouble top,
             GLdouble zNear, GLdouble zFar)
{
   if (!tr)
      return;

   tr->Perspective = GL_FALSE;
   tr->Left = left;
   tr->Right = right;
   tr->Bottom = bottom;
   tr->Top = top;
   tr->Near = zNear;
   tr->Far = zFar;
}


void trFrustum(TRcontext *tr,
               GLdouble left, GLdouble right,
               GLdouble bottom, GLdouble top,
               GLdouble zNear, GLdouble zFar)
{
   if (!tr)
      return;

   tr->Perspective = GL_TRUE;
   tr->Left = left;
   tr->Right = right;
   tr->Bottom = bottom;
   tr->Top = top;
   tr->Near = zNear;
   tr->Far = zFar;
}


void trPerspective(TRcontext *tr,
                   GLdouble fovy, GLdouble aspect,
                   GLdouble zNear, GLdouble zFar )
{
   GLdouble xmin, xmax, ymin, ymax;
   static const double halfradians=acos(-1.0)/360.0;
   ymax = zNear * tan(fovy * halfradians);
   ymin = -ymax;
   xmin = ymin * aspect;
   xmax = ymax * aspect;
   trFrustum(tr, xmin, xmax, ymin, ymax, zNear, zFar);
}


void trBeginTile(TRcontext *tr)
{
   GLint tileWidth, tileHeight, border;
   GLdouble left, right, bottom, top;

   if (!tr)
      return;

   if (tr->CurrentTile <= 0) {
      Setup(tr);
      /* Save user's viewport, will be restored after last tile rendered */
      glGetIntegerv(GL_VIEWPORT, tr->ViewportSave);
   }

   /* which tile (by row and column) we're about to render */
   if (tr->RowOrder==TR_BOTTOM_TO_TOP) {
      tr->CurrentRow = tr->CurrentTile / tr->Columns;
      tr->CurrentColumn = tr->CurrentTile % tr->Columns;
   }
   else if (tr->RowOrder==TR_TOP_TO_BOTTOM) {
      tr->CurrentRow = tr->Rows - (tr->CurrentTile / tr->Columns) - 1;
      tr->CurrentColumn = tr->CurrentTile % tr->Columns;
   }
   else {
      /* This should never happen */
      abort();
   }
   assert(tr->CurrentRow < tr->Rows);
   assert(tr->CurrentColumn < tr->Columns);

   border = tr->TileBorder;

   /* Compute actual size of this tile with border */
   if (tr->CurrentRow < tr->Rows-1)
      tileHeight = tr->TileHeight;
   else
      tileHeight = tr->ImageHeight - (tr->Rows-1) * (tr->TileHeightNB) + 2 * border;

   if (tr->CurrentColumn < tr->Columns-1)
      tileWidth = tr->TileWidth;
   else
      tileWidth = tr->ImageWidth - (tr->Columns-1) * (tr->TileWidthNB) + 2 * border;

   /* Save tile size, with border */
   tr->CurrentTileWidth = tileWidth;
   tr->CurrentTileHeight = tileHeight;

   glViewport(0, 0, tileWidth, tileHeight);  /* tile size including border */

   /* compute projection parameters */
   left = tr->Left + (tr->Right - tr->Left)
        * (tr->CurrentColumn * tr->TileWidthNB - border) / tr->ImageWidth;
   right = left + (tr->Right - tr->Left) * tileWidth / tr->ImageWidth;
   bottom = tr->Bottom + (tr->Top - tr->Bottom)
          * (tr->CurrentRow * tr->TileHeightNB - border) / tr->ImageHeight;
   top = bottom + (tr->Top - tr->Bottom) * tileHeight / tr->ImageHeight;

   if (tr->Perspective)
      frustum(left, right, bottom, top, tr->Near, tr->Far);
   else
      ortho(left, right, bottom, top, tr->Near, tr->Far);
}



int trEndTile(TRcontext *tr)
{
   GLint prevRowLength, prevSkipRows, prevSkipPixels;

   if (!tr)
      return 0;

   assert(tr->CurrentTile>=0);

   /* be sure OpenGL rendering is finished */
   glFlush();

   /* save current glPixelStore values */
   glGetIntegerv(GL_PACK_ROW_LENGTH, &prevRowLength);
   glGetIntegerv(GL_PACK_SKIP_ROWS, &prevSkipRows);
   glGetIntegerv(GL_PACK_SKIP_PIXELS, &prevSkipPixels);
   /*glGetIntegerv(GL_PACK_ALIGNMENT, &prevAlignment);*/

   if (tr->TileBuffer) {
      GLint srcX = tr->TileBorder;
      GLint srcY = tr->TileBorder;
      GLint srcWidth = tr->TileWidthNB;
      GLint srcHeight = tr->TileHeightNB;
      glReadPixels(srcX, srcY, srcWidth, srcHeight,
                   tr->TileFormat, tr->TileType, tr->TileBuffer);
   }

   if (tr->ImageBuffer) {
      GLint srcX = tr->TileBorder;
      GLint srcY = tr->TileBorder;
      GLint srcWidth = tr->CurrentTileWidth - 2 * tr->TileBorder;
      GLint srcHeight = tr->CurrentTileHeight - 2 * tr->TileBorder;
      GLint destX = tr->TileWidthNB * tr->CurrentColumn;
      GLint destY = tr->TileHeightNB * tr->CurrentRow;

      /* setup pixel store for glReadPixels */
      glPixelStorei(GL_PACK_ROW_LENGTH, tr->ImageWidth);
      glPixelStorei(GL_PACK_SKIP_ROWS, destY);
      glPixelStorei(GL_PACK_SKIP_PIXELS, destX);
      /*glPixelStorei(GL_PACK_ALIGNMENT, 1);*/

      /* read the tile into the final image */
      glReadPixels(srcX, srcY, srcWidth, srcHeight,
                   tr->ImageFormat, tr->ImageType, tr->ImageBuffer);
   }

   /* restore previous glPixelStore values */
   glPixelStorei(GL_PACK_ROW_LENGTH, prevRowLength);
   glPixelStorei(GL_PACK_SKIP_ROWS, prevSkipRows);
   glPixelStorei(GL_PACK_SKIP_PIXELS, prevSkipPixels);
   /*glPixelStorei(GL_PACK_ALIGNMENT, prevAlignment);*/

   /* increment tile counter, return 1 if more tiles left to render */
   tr->CurrentTile++;
   if (tr->CurrentTile >= tr->Rows * tr->Columns) {
      /* restore user's viewport */
      glViewport(tr->ViewportSave[0], tr->ViewportSave[1],
                 tr->ViewportSave[2], tr->ViewportSave[3]);
      tr->CurrentTile = -1;  /* all done */
      return 0;
   }
   else
      return 1;
}

#endif
