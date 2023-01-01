//========================================================================
//
// ShadingImage.cc
//
// Copyright 2020 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <math.h>
#include "Trace.h"
#include "GfxState.h"
#include "SplashBitmap.h"
#include "SplashPattern.h"
#include "SplashPath.h"
#include "Splash.h"
#include "ShadingImage.h"

// Max recursive depth for a patch mesh shading fill.
#define patchMaxDepth 10

// Max delta allowed in any color component for a patch mesh shading
// fill.
#define patchColorDelta (dblToCol(1 / 256.0))

SplashBitmap *ShadingImage::generateBitmap(GfxState *state,
					   GfxShading *shading,
					   SplashColorMode mode,
					   GBool reverseVideo,
					   Splash *parentSplash,
					   SplashBitmap *parentBitmap,
					   int *xOut, int *yOut) {
  switch (shading->getType()) {
  case 1:
    return generateFunctionBitmap(state, (GfxFunctionShading *)shading,
				  mode, reverseVideo,
				  parentSplash, parentBitmap, xOut, yOut);
    break;
  case 2:
    return generateAxialBitmap(state, (GfxAxialShading *)shading,
			       mode, reverseVideo,
			       parentSplash, parentBitmap, xOut, yOut);
    break;
  case 3:
    return generateRadialBitmap(state, (GfxRadialShading *)shading,
				mode, reverseVideo,
				parentSplash, parentBitmap, xOut, yOut);
    break;
  case 4:
  case 5:
    return generateGouraudTriangleBitmap(state,
					 (GfxGouraudTriangleShading *)shading,
					 mode, reverseVideo,
					 parentSplash, parentBitmap,
					 xOut, yOut);
    break;
  case 6:
  case 7:
    return generatePatchMeshBitmap(state, (GfxPatchMeshShading *)shading,
				   mode, reverseVideo,
				   parentSplash, parentBitmap, xOut, yOut);
    break;
  default:
    return NULL;
  }
}

SplashBitmap *ShadingImage::generateFunctionBitmap(GfxState *state,
						   GfxFunctionShading *shading,
						   SplashColorMode mode,
						   GBool reverseVideo,
						   Splash *parentSplash,
						   SplashBitmap *parentBitmap,
						   int *xOut, int *yOut) {
  // get the shading parameters
  double x0, y0, x1, y1;
  shading->getDomain(&x0, &y0, &x1, &y1);
  double *patternMat = shading->getMatrix();

  // get the clip bbox
  double fxMin, fyMin, fxMax, fyMax;
  state->getClipBBox(&fxMin, &fyMin, &fxMax, &fyMax);
  if (fxMin > fxMax || fyMin > fyMax) {
    return NULL;
  }

  // convert to integer coords
  int xMin = (int)floor(fxMin);
  int yMin = (int)floor(fyMin);
  int xMax = (int)floor(fxMax) + 1;
  int yMax = (int)floor(fyMax) + 1;
  int bitmapWidth = xMax - xMin;
  int bitmapHeight = yMax - yMin;

  // allocate the bitmap
  traceMessage("function shading fill bitmap");
  SplashBitmap *bitmap = new SplashBitmap(bitmapWidth, bitmapHeight, 1, mode,
					  gTrue, gTrue, parentBitmap);
  int nComps = splashColorModeNComps[mode];

  // compute the domain -> device space transform = mat * CTM
  double *ctm = state->getCTM();
  double mat[6];
  mat[0] = patternMat[0] * ctm[0] + patternMat[1] * ctm[2];
  mat[1] = patternMat[0] * ctm[1] + patternMat[1] * ctm[3];
  mat[2] = patternMat[2] * ctm[0] + patternMat[3] * ctm[2];
  mat[3] = patternMat[2] * ctm[1] + patternMat[3] * ctm[3];
  mat[4] = patternMat[4] * ctm[0] + patternMat[5] * ctm[2] + ctm[4];
  mat[5] = patternMat[4] * ctm[1] + patternMat[5] * ctm[3] + ctm[5];

  // compute the device space -> domain transform
  double det = mat[0] * mat[3] - mat[1] * mat[2];
  if (fabs(det) < 0.000001) {
    return NULL;
  }
  det = 1 / det;
  double iMat[6];
  iMat[0] = mat[3] * det;
  iMat[1] = -mat[1] * det;
  iMat[2] = -mat[2] * det;
  iMat[3] = mat[0] * det;
  iMat[4] = (mat[2] * mat[5] - mat[3] * mat[4]) * det;
  iMat[5] = (mat[1] * mat[4] - mat[0] * mat[5]) * det;

  // fill the bitmap
  SplashColorPtr dataPtr = bitmap->getDataPtr();
  Guchar *alphaPtr = bitmap->getAlphaPtr();
  for (int y = 0; y < bitmapHeight; ++y) {
    for (int x = 0; x < bitmapWidth; ++x) {

      // convert coords to the pattern domain
      double tx = xMin + x + 0.5;
      double ty = yMin + y + 0.5;
      double xx = tx * iMat[0] + ty * iMat[2] + iMat[4];
      double yy = tx * iMat[1] + ty * iMat[3] + iMat[5];

      // get the color
      if (xx >= x0 && xx <= x1 && yy >= y0 && yy <= y1) {
	GfxColor color;
	shading->getColor(xx, yy, &color);
	SplashColor sColor;
	computeShadingColor(state, mode, reverseVideo, &color, sColor);
	for (int i = 0; i < nComps; ++i) {
	  *dataPtr++ = sColor[i];
	}
	*alphaPtr++ = 0xff;
      } else {
	dataPtr += nComps;
	*alphaPtr++ = 0;
      }
    }
  }

  *xOut = xMin;
  *yOut = yMin;
  return bitmap;
}

SplashBitmap *ShadingImage::generateAxialBitmap(GfxState *state,
						GfxAxialShading *shading,
						SplashColorMode mode,
						GBool reverseVideo,
						Splash *parentSplash,
						SplashBitmap *parentBitmap,
						int *xOut, int *yOut) {
  // get the shading parameters
  double x0, y0, x1, y1;
  shading->getCoords(&x0, &y0, &x1, &y1);
  double t0 = shading->getDomain0();
  double t1 = shading->getDomain1();
  GBool ext0 = shading->getExtend0();
  GBool ext1 = shading->getExtend1();
  double dx = x1 - x0;
  double dy = y1 - y0;
  double d = dx * dx + dy * dy;
  GBool dZero = fabs(d) < 0.0001;
  if (!dZero) {
    d = 1 / d;
  }
  if (dZero && !ext0 && !ext1) {
    return NULL;
  }

  // get the clip bbox
  double fxMin, fyMin, fxMax, fyMax;
  state->getClipBBox(&fxMin, &fyMin, &fxMax, &fyMax);
  if (fxMin > fxMax || fyMin > fyMax) {
    return NULL;
  }

  // convert to integer coords
  int xMin = (int)floor(fxMin);
  int yMin = (int)floor(fyMin);
  int xMax = (int)floor(fxMax) + 1;
  int yMax = (int)floor(fyMax) + 1;
  int bitmapWidth = xMax - xMin;
  int bitmapHeight = yMax - yMin;

  // compute the inverse CTM
  double *ctm = state->getCTM();
  double det = ctm[0] * ctm[3] - ctm[1] * ctm[2];
  if (fabs(det) < 0.000001) {
    return NULL;
  }
  det = 1 / det;
  double ictm[6];
  ictm[0] = ctm[3] * det;
  ictm[1] = -ctm[1] * det;
  ictm[2] = -ctm[2] * det;
  ictm[3] = ctm[0] * det;
  ictm[4] = (ctm[2] * ctm[5] - ctm[3] * ctm[4]) * det;
  ictm[5] = (ctm[1] * ctm[4] - ctm[0] * ctm[5]) * det;

  // convert axis endpoints to device space
  double xx0, yy0, xx1, yy1;
  state->transform(x0, y0, &xx0, &yy0);
  state->transform(x1, y1, &xx1, &yy1);

  // allocate the bitmap
  traceMessage("axial shading fill bitmap");
  SplashBitmap *bitmap = new SplashBitmap(bitmapWidth, bitmapHeight, 1, mode,
					  gTrue, gTrue, parentBitmap);
  int nComps = splashColorModeNComps[mode];

  // special case: zero-length axis
  if (dZero) {
    GfxColor color;
    if (ext0) {
      shading->getColor(t0, &color);
    } else {
      shading->getColor(t1, &color);
    }
    SplashColor sColor;
    computeShadingColor(state, mode, reverseVideo, &color, sColor);
    SplashColorPtr dataPtr = bitmap->getDataPtr();
    for (int y = 0; y < bitmapHeight; ++y) {
      for (int x = 0; x < bitmapWidth; ++x) {
	for (int i = 0; i < nComps; ++i) {
	  *dataPtr++ = sColor[i];
	}
      }
    }
    memset(bitmap->getAlphaPtr(), 0xff, (size_t)bitmapWidth * bitmapHeight);

  // special case: horizontal axis (in device space)
  } else if (fabs(yy0 - yy1) < 0.01) {
    for (int x = 0; x < bitmapWidth; ++x) {
      SplashColorPtr dataPtr = bitmap->getDataPtr() + x * nComps;
      Guchar *alphaPtr = bitmap->getAlphaPtr() + x;
      double tx = xMin + x + 0.5;
      double ty = yMin + 0.5;
      double xx = tx * ictm[0] + ty * ictm[2] + ictm[4];
      double yy = tx * ictm[1] + ty * ictm[3] + ictm[5];
      double s = ((xx - x0) * dx + (yy - y0) * dy) * d;
      GBool go = gFalse;
      if (s < 0) {
	go = ext0;
      } else if (s > 1) {
	go = ext1;
      } else {
	go = gTrue;
      }
      if (go) {
	GfxColor color;
	if (s <= 0) {
	  shading->getColor(t0, &color);
	} else if (s >= 1) {
	  shading->getColor(t1, &color);
	} else {
	  double t = t0 + s * (t1 - t0);
	  shading->getColor(t, &color);
	}
	SplashColor sColor;
	computeShadingColor(state, mode, reverseVideo, &color, sColor);
	for (int y = 0; y < bitmapHeight; ++y) {
	  for (int i = 0; i < nComps; ++i) {
	    dataPtr[i] = sColor[i];
	  }
	  dataPtr += bitmap->getRowSize();
	  *alphaPtr = 0xff;
	  alphaPtr += bitmapWidth;
	}
      } else {
	for (int y = 0; y < bitmapHeight; ++y) {
	  *alphaPtr = 0;
	  alphaPtr += bitmapWidth;
	}
      }
    }

  // special case: vertical axis (in device space)
  } else if (fabs(xx0 - xx1) < 0.01) {
    for (int y = 0; y < bitmapHeight; ++y) {
      SplashColorPtr dataPtr = bitmap->getDataPtr() + y * bitmap->getRowSize();
      Guchar *alphaPtr = bitmap->getAlphaPtr() + y * bitmapWidth;
      double tx = xMin + 0.5;
      double ty = yMin + y + 0.5;
      double xx = tx * ictm[0] + ty * ictm[2] + ictm[4];
      double yy = tx * ictm[1] + ty * ictm[3] + ictm[5];
      double s = ((xx - x0) * dx + (yy - y0) * dy) * d;
      GBool go = gFalse;
      if (s < 0) {
	go = ext0;
      } else if (s > 1) {
	go = ext1;
      } else {
	go = gTrue;
      }
      if (go) {
	GfxColor color;
	if (s <= 0) {
	  shading->getColor(t0, &color);
	} else if (s >= 1) {
	  shading->getColor(t1, &color);
	} else {
	  double t = t0 + s * (t1 - t0);
	  shading->getColor(t, &color);
	}
	SplashColor sColor;
	computeShadingColor(state, mode, reverseVideo, &color, sColor);
	for (int x = 0; x < bitmapWidth; ++x) {
	  for (int i = 0; i < nComps; ++i) {
	    dataPtr[i] = sColor[i];
	  }
	  dataPtr += nComps;
	}
	memset(alphaPtr, 0xff, bitmapWidth);
      } else {
	memset(alphaPtr, 0, bitmapWidth);
      }
    }

  // general case
  } else {
    // pre-compute colors along the axis
    int nColors = (int)(1.5 * sqrt((xx1 - xx0) * (xx1 - xx0)
				   + (yy1 - yy0) * (yy1 - yy0)));
    if (nColors < 16) {
      nColors = 16;
    } else if (nColors > 1024) {
      nColors = 1024;
    }
    SplashColorPtr sColors = (SplashColorPtr)gmallocn(nColors, nComps);
    SplashColorPtr sColor = sColors;
    for (int i = 0; i < nColors; ++i) {
      double s = (double)i / (double)(nColors - 1);
      double t = t0 + s * (t1 - t0);
      GfxColor color;
      shading->getColor(t, &color);
      computeShadingColor(state, mode, reverseVideo, &color, sColor);
      sColor += nComps;
    }

    SplashColorPtr dataPtr = bitmap->getDataPtr();
    Guchar *alphaPtr = bitmap->getAlphaPtr();
    for (int y = 0; y < bitmapHeight; ++y) {
      for (int x = 0; x < bitmapWidth; ++x) {

	// convert coords to user space
	double tx = xMin + x + 0.5;
	double ty = yMin + y + 0.5;
	double xx = tx * ictm[0] + ty * ictm[2] + ictm[4];
	double yy = tx * ictm[1] + ty * ictm[3] + ictm[5];

	// compute the position along the axis
	double s = ((xx - x0) * dx + (yy - y0) * dy) * d;
	GBool go = gFalse;
	if (s < 0) {
	  go = ext0;
	} else if (s > 1) {
	  go = ext1;
	} else {
	  go = gTrue;
	}
	if (go) {
	  if (s <= 0) {
	    sColor = sColors;
	  } else if (s >= 1) {
	    sColor = sColors + (nColors - 1) * nComps;
	  } else {
	    int i = (int)((nColors - 1) * s + 0.5);
	    sColor = sColors + i * nComps;
	  }
	  for (int i = 0; i < nComps; ++i) {
	    *dataPtr++ = sColor[i];
	  }
	  *alphaPtr++ = 0xff;
	} else {
	  dataPtr += nComps;
	  *alphaPtr++ = 0;
	}
      }
    }
    gfree(sColors);
  }

  *xOut = xMin;
  *yOut = yMin;
  return bitmap;
}

SplashBitmap *ShadingImage::generateRadialBitmap(GfxState *state,
						 GfxRadialShading *shading,
						 SplashColorMode mode,
						 GBool reverseVideo,
						 Splash *parentSplash,
						 SplashBitmap *parentBitmap,
						 int *xOut, int *yOut) {
  // get the shading parameters
  double x0, y0, r0, x1, y1, r1;
  shading->getCoords(&x0, &y0, &r0, &x1, &y1, &r1);
  double t0 = shading->getDomain0();
  double t1 = shading->getDomain1();
  GBool ext0 = shading->getExtend0();
  GBool ext1 = shading->getExtend1();
  double h = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
  GBool enclosed = fabs(r1 - r0) >= h;

  // get the clip bbox
  double fxMin, fyMin, fxMax, fyMax;
  state->getClipBBox(&fxMin, &fyMin, &fxMax, &fyMax);
  if (fxMin > fxMax || fyMin > fyMax) {
    return NULL;
  }

  // intersect with shading region (in user space): if the extend
  // flags are false (or just the larger extend flag is false, in the
  // "enclosed" case), we can use the bbox for the two circles
  if ((!ext0 && !ext1) ||
      (enclosed && !(r0 > r1 ? ext0 : ext1))) {
    double uxMin = (x0 - r0) < (x1 - r1) ? (x0 - r0) : (x1 - r1);
    double uxMax = (x0 + r0) > (x1 + r1) ? (x0 + r0) : (x1 + r1);
    double uyMin = (y0 - r0) < (y1 - r1) ? (y0 - r0) : (y1 - r1);
    double uyMax = (y0 + r0) > (y1 + r1) ? (y0 + r0) : (y1 + r1);
    double dxMin, dyMin, dxMax, dyMax;
    transformBBox(state, uxMin, uyMin, uxMax, uyMax,
		  &dxMin, &dyMin, &dxMax, &dyMax);
    if (dxMin > fxMin) {
      fxMin = dxMin;
    }
    if (dxMax < dxMax) {
      fxMax = dxMax;
    }
    if (dyMin > fyMin) {
      fyMin = dyMin;
    }
    if (dyMax < fyMax) {
      fyMax = dyMax;
    }
    if (fxMin > fxMax || fyMin > fyMax) {
      return NULL;
    }
  }

  // convert to integer coords
  int xMin = (int)floor(fxMin);
  int yMin = (int)floor(fyMin);
  int xMax = (int)floor(fxMax) + 1;
  int yMax = (int)floor(fyMax) + 1;
  int bitmapWidth = xMax - xMin;
  int bitmapHeight = yMax - yMin;

  // compute the inverse CTM
  double *ctm = state->getCTM();
  double det = ctm[0] * ctm[3] - ctm[1] * ctm[2];
  if (fabs(det) < 0.000001) {
    return NULL;
  }
  det = 1 / det;
  double ictm[6];
  ictm[0] = ctm[3] * det;
  ictm[1] = -ctm[1] * det;
  ictm[2] = -ctm[2] * det;
  ictm[3] = ctm[0] * det;
  ictm[4] = (ctm[2] * ctm[5] - ctm[3] * ctm[4]) * det;
  ictm[5] = (ctm[1] * ctm[4] - ctm[0] * ctm[5]) * det;

  // allocate the bitmap
  traceMessage("radial shading fill bitmap");
  SplashBitmap *bitmap = new SplashBitmap(bitmapWidth, bitmapHeight, 1, mode,
					  gTrue, gTrue, parentBitmap);
  int nComps = splashColorModeNComps[mode];

  // pre-compute colors along the axis
  int nColors = (int)sqrt((double)(bitmapWidth * bitmapWidth
				   + bitmapHeight * bitmapHeight));
  if (nColors < 16) {
    nColors = 16;
  } else if (nColors > 1024) {
    nColors = 1024;
  }
  SplashColorPtr sColors = (SplashColorPtr)gmallocn(nColors, nComps);
  SplashColorPtr sColor = sColors;
  for (int i = 0; i < nColors; ++i) {
    double s = (double)i / (double)(nColors - 1);
    double t = t0 + s * (t1 - t0);
    GfxColor color;
    shading->getColor(t, &color);
    computeShadingColor(state, mode, reverseVideo, &color, sColor);
    sColor += nComps;
  }

  // special case: in the "enclosed" + extended case, we can fill the
  // bitmap with the outer color and just render inside the larger
  // circle
  int bxMin, byMin, bxMax, byMax;
  if (enclosed &&
      ((r0 > r1 && ext0) || (r1 > r0 && ext1))) {
    double uxMin, uyMin, uxMax, uyMax;
    if (r0 > r1) {
      sColor = sColors;
      uxMin = x0 - r0;
      uxMax = x0 + r0;
      uyMin = y0 - r0;
      uyMax = y0 + r0;
    } else {
      sColor = sColors + (nColors - 1) * nComps;
      uxMin = x1 - r1;
      uxMax = x1 + r1;
      uyMin = y1 - r1;
      uyMax = y1 + r1;
    }

    // convert bbox of larger circle to device space
    double dxMin, dyMin, dxMax, dyMax;
    transformBBox(state, uxMin, uyMin, uxMax, uyMax,
		  &dxMin, &dyMin, &dxMax, &dyMax);
    bxMin = (int)floor(dxMin - xMin);
    if (bxMin < 0) {
      bxMin = 0;
    }
    byMin = (int)floor(dyMin - yMin);
    if (byMin < 0) {
      byMin = 0;
    }
    bxMax = (int)floor(dxMax - xMin) + 1;
    if (bxMax > bitmapWidth) {
      bxMax = bitmapWidth;
    }
    byMax = (int)floor(dyMax - yMin) + 1;
    if (byMax > bitmapHeight) {
      byMax = bitmapHeight;
    }

    // fill bitmap (except for the rectangle containing the larger circle)
    SplashColorPtr dataPtr = bitmap->getDataPtr();
    Guchar *alphaPtr = bitmap->getAlphaPtr();
    for (int y = 0; y < bitmapHeight; ++y) {
      for (int x = 0; x < bitmapWidth; ++x) {
	if (y >= byMin && y < byMax && x >= bxMin && x < bxMax) {
	  dataPtr += nComps;
	  ++alphaPtr;
	} else {
	  for (int i = 0; i < nComps; ++i) {
	    *dataPtr++ = sColor[i];
	  }
	  *alphaPtr++ = 0xff;
	}
      }
    }

  } else {
    bxMin = 0;
    byMin = 0;
    bxMax = bitmapWidth;
    byMax = bitmapHeight;
  }

  // render the shading into the bitmap
  double dx = x1 - x0;
  double dy = y1 - y0;
  double dr = r1 - r0;
  double r0dr = r0 * dr;
  double r02 = r0 * r0;
  double a = dx * dx + dy * dy - dr * dr;
  GBool aIsZero;
  double a2;
  if (fabs(a) < 0.00001) {
    aIsZero = gTrue;
    a2 = 0;
  } else {
    aIsZero = gFalse;
    a2 = 1 / (2 * a);
  }
  for (int y = byMin; y < byMax; ++y) {
    SplashColorPtr dataPtr = bitmap->getDataPtr()
                             + y * bitmap->getRowSize() + bxMin * nComps;
    Guchar *alphaPtr = bitmap->getAlphaPtr()
                       + y * bitmap->getAlphaRowSize() + bxMin;
    for (int x = bxMin; x < bxMax; ++x) {

      // convert coords to user space
      double tx = xMin + x + 0.5;
      double ty = yMin + y + 0.5;
      double xx = tx * ictm[0] + ty * ictm[2] + ictm[4];
      double yy = tx * ictm[1] + ty * ictm[3] + ictm[5];

      // compute the radius of the circle at x,y
      double b = 2 * ((xx - x0) * dx + (yy - y0) * dy + r0dr);
      double c = (xx - x0) * (xx - x0) + (yy - y0) * (yy - y0) - r02;
      double s = 0;
      GBool go = gFalse;
      if (aIsZero) {
	if (fabs(b) < 0.000001) {
	  if (c <= 0) {
	    if (ext0) {
	      s = 0;
	      go = gTrue;
	    }
	  } else {
	    if (ext1) {
	      s = 1;
	      go = gTrue;
	    }
	  }
	} else {
	  double s0 = c / b;
	  double rs0 = r0 + s0 * (r1 - r0);
	  if ((s0 >= 0 || ext0) && (s0 <= 1 || ext1) && rs0 >= 0) { 
	    s = s0;
	    go = gTrue;
	  }
	}
      } else {
	double e = b*b - 4*a*c;
	if (e >= 0) {
	  double es = sqrt(e);
	  double s0 = (b + es) * a2;
	  double s1 = (b - es) * a2;
	  double rs0 = r0 + s0 * (r1 - r0);
	  double rs1 = r0 + s1 * (r1 - r0);
	  if (s0 > s1) {
	    if ((s0 >= 0 || ext0) && (s0 <= 1 || ext1) && rs0 >= 0) {
	      s = s0;
	      go = gTrue;
	    } else if ((s1 >= 0 || ext0) && (s1 <= 1 || ext1) && rs1 >= 0) {
	      s = s1;
	      go = gTrue;
	    }
	  } else {
	    if ((s1 >= 0 || ext0) && (s1 <= 1 || ext1) && rs1 >= 0) {
	      s = s1;
	      go = gTrue;
	    } else if ((s0 >= 0 || ext0) && (s0 <= 1 || ext1) && rs0 >= 0) {
	      s = s0;
	      go = gTrue;
	    }
	  }
	}
      }
      if (!go) {
	dataPtr += nComps;
	*alphaPtr++ = 0x00;
	continue;
      }
      if (s <= 0) {
	sColor = sColors;
      } else if (s >= 1) {
	sColor = sColors + (nColors - 1) * nComps;
      } else {
	int i = (int)((nColors - 1) * s + 0.5);
	sColor = sColors + i * nComps;
      }
      for (int i = 0; i < nComps; ++i) {
	*dataPtr++ = sColor[i];
      }
      *alphaPtr++ = 0xff;
    }
  }

  gfree(sColors);

  *xOut = xMin;
  *yOut = yMin;
  return bitmap;
}

SplashBitmap *ShadingImage::generateGouraudTriangleBitmap(
					GfxState *state,
					GfxGouraudTriangleShading *shading,
					SplashColorMode mode,
					GBool reverseVideo,
					Splash *parentSplash,
					SplashBitmap *parentBitmap,
					int *xOut, int *yOut) {
  // get the clip bbox
  double fxMin, fyMin, fxMax, fyMax;
  state->getClipBBox(&fxMin, &fyMin, &fxMax, &fyMax);
  if (fxMin > fxMax || fyMin > fyMax) {
    return NULL;
  }

  // get the shading bbox
  double tx0, ty0, tx1, ty1, dx, dy, txMin, tyMin, txMax, tyMax;
  shading->getBBox(&tx0, &ty0, &tx1, &ty1);
  state->transform(tx0, ty0, &dx, &dy);
  txMin = txMax = dx;
  tyMin = tyMax = dy;
  state->transform(tx0, ty1, &dx, &dy);
  if (dx < txMin) {
    txMin = dx;
  } else if (dx > txMax) {
    txMax = dx;
  }
  if (dy < tyMin) {
    tyMin = dy;
  } else if (dy > tyMax) {
    tyMax = dy;
  }
  state->transform(tx1, ty0, &dx, &dy);
  if (dx < txMin) {
    txMin = dx;
  } else if (dx > txMax) {
    txMax = dx;
  }
  if (dy < tyMin) {
    tyMin = dy;
  } else if (dy > tyMax) {
    tyMax = dy;
  }
  state->transform(tx1, ty1, &dx, &dy);
  if (dx < txMin) {
    txMin = dx;
  } else if (dx > txMax) {
    txMax = dx;
  }
  if (dy < tyMin) {
    tyMin = dy;
  } else if (dy > tyMax) {
    tyMax = dy;
  }
  if (txMin > fxMin) {
    fxMin = txMin;
  }
  if (txMax < fxMax) {
    fxMax = txMax;
  }
  if (tyMin > fyMin) {
    fyMin = tyMin;
  }
  if (tyMax < fyMax) {
    fyMax = tyMax;
  }
  if (fxMin > fxMax || fyMin > fyMax) {
    return NULL;
  }

  // convert to integer coords
  int xMin = (int)floor(fxMin);
  int yMin = (int)floor(fyMin);
  int xMax = (int)floor(fxMax) + 1;
  int yMax = (int)floor(fyMax) + 1;
  int bitmapWidth = xMax - xMin;
  int bitmapHeight = yMax - yMin;

  // allocate the bitmap
  traceMessage("Gouraud triangle shading fill bitmap");
  SplashBitmap *bitmap = new SplashBitmap(bitmapWidth, bitmapHeight, 1, mode,
					  gTrue, gTrue, parentBitmap);

  // clear the bitmap
  memset(bitmap->getDataPtr(), 0, bitmap->getHeight() * bitmap->getRowSize());
  memset(bitmap->getAlphaPtr(), 0, bitmap->getHeight() * bitmap->getWidth());

  // draw the triangles
  for (int i = 0; i < shading->getNTriangles(); ++i) {
    double x0, y0, x1, y1, x2, y2;
    double color0[gfxColorMaxComps];
    double color1[gfxColorMaxComps];
    double color2[gfxColorMaxComps];
    shading->getTriangle(i, &x0, &y0, color0,
			 &x1, &y1, color1,
			 &x2, &y2, color2);
    gouraudFillTriangle(state, bitmap, mode, reverseVideo,
			xMin, yMin, xMax, yMax,
			x0, y0, color0, x1, y1, color1, x2, y2, color2,
			shading);
  }

  *xOut = xMin;
  *yOut = yMin;
  return bitmap;
}

void ShadingImage::gouraudFillTriangle(GfxState *state, SplashBitmap *bitmap,
				       SplashColorMode mode,
				       GBool reverseVideo,
				       int xMin, int yMin, int xMax, int yMax,
				       double x0, double y0, double *color0,
				       double x1, double y1, double *color1,
				       double x2, double y2, double *color2,
				       GfxGouraudTriangleShading *shading) {
  int nShadingComps = shading->getNComps();
  int nBitmapComps = splashColorModeNComps[mode];

  //--- transform the vertices to device space, sort by y
  double dx0, dy0, dx1, dy1, dx2, dy2;
  state->transform(x0, y0, &dx0, &dy0);
  state->transform(x1, y1, &dx1, &dy1);
  state->transform(x2, y2, &dx2, &dy2);
  if (dy0 > dy1) {
    double t = dx0;  dx0 = dx1;  dx1 = t;
    t = dy0;  dy0 = dy1;  dy1 = t;
    double *tc = color0;  color0 = color1;  color1 = tc;
  }
  if (dy1 > dy2) {
    double t = dx1;  dx1 = dx2;  dx2 = t;
    t = dy1;  dy1 = dy2;  dy2 = t;
    double *tc = color1;  color1 = color2;  color2 = tc;
  }
  if (dy0 > dy1) {
    double t = dx0;  dx0 = dx1;  dx1 = t;
    t = dy0;  dy0 = dy1;  dy1 = t;
    double *tc = color0;  color0 = color1;  color1 = tc;
  }

  //--- y loop
  int syMin = (int)floor(dy0);
  if (syMin < yMin) {
    syMin = yMin;
  }
  int syMax = (int)floor(dy2) + 1;
  if (syMax > yMax) {
    syMax = yMax;
  }
  for (int sy = syMin; sy < syMax; ++sy) {

    //--- vertical interpolation
    double xx0, xx1;
    double cc0[gfxColorMaxComps], cc1[gfxColorMaxComps];
    if (sy <= dy0) {
      xx0 = xx1 = dx0;
      for (int i = 0; i < nShadingComps; ++i) {
	cc0[i] = cc1[i] = color0[i];
      }
    } else if (sy >= dy2) {
      xx0 = xx1 = dx2;
      for (int i = 0; i < nShadingComps; ++i) {
	cc0[i] = cc1[i] = color2[i];
      }
    } else {
      if (sy <= dy1) {
	double interp = (sy - dy0) / (dy1 - dy0);
	xx0 = dx0 + interp * (dx1 - dx0);
	for (int i = 0; i < nShadingComps; ++i) {
	  cc0[i] = color0[i] + interp * (color1[i] - color0[i]);
	}
      } else {
	double interp = (sy - dy1) / (dy2 - dy1);
	xx0 = dx1 + interp * (dx2 - dx1);
	for (int i = 0; i < nShadingComps; ++i) {
	  cc0[i] = color1[i] + interp * (color2[i] - color1[i]);
	}
      }
      double interp = (sy - dy0) / (dy2 - dy0);
      xx1 = dx0 + interp * (dx2 - dx0);
      for (int i = 0; i < nShadingComps; ++i) {
	cc1[i] = color0[i] + interp * (color2[i] - color0[i]);
      }
    }

    //--- x loop
    if (xx0 > xx1) {
      double t = xx0;  xx0 = xx1;  xx1 = t;
      for (int i = 0; i < nShadingComps; ++i) {
	t = cc0[i];  cc0[i] = cc1[i];  cc1[i] = t;
      }
    }
    int sxMin = (int)floor(xx0);
    if (sxMin < xMin) {
      sxMin = xMin;
    }
    int sxMax = (int)floor(xx1) + 1;
    if (sxMax > xMax) {
      sxMax = xMax;
    }
    SplashColorPtr dataPtr = bitmap->getDataPtr()
                             + (sy - yMin) * bitmap->getRowSize()
                             + (sxMin - xMin) * nBitmapComps;
    if (sxMin < sxMax) {
      Guchar *alphaPtr = bitmap->getAlphaPtr()
	                 + (sy - yMin) * bitmap->getWidth()
	                 + (sxMin - xMin);
      memset(alphaPtr, 0xff, sxMax - sxMin);
    }
    for (int sx = sxMin; sx < sxMax; ++sx) {

      //--- horizontal interpolation
      double cc[gfxColorMaxComps];
      if (sx <= xx0) {
	for (int i = 0; i < nShadingComps; ++i) {
	  cc[i] = cc0[i];
	}
      } else if (sx >= xx1) {
	for (int i = 0; i < nShadingComps; ++i) {
	  cc[i] = cc1[i];
	}
      } else {
	for (int i = 0; i < nShadingComps; ++i) {
	  double interp = (sx - xx0) / (xx1 - xx0);
	  cc[i] = cc0[i] + interp * (cc1[i] - cc0[i]);
	}
      }

      //--- compute color and set pixel
      GfxColor gColor;
      shading->getColor(cc, &gColor);
      SplashColor sColor;
      computeShadingColor(state, mode, reverseVideo, &gColor, sColor);
      for (int i = 0; i < nBitmapComps; ++i) {
	dataPtr[i] = sColor[i];
      }
      dataPtr += nBitmapComps;
    }
  }
}

SplashBitmap *ShadingImage::generatePatchMeshBitmap(
					GfxState *state,
					GfxPatchMeshShading *shading,
					SplashColorMode mode,
					GBool reverseVideo,
					Splash *parentSplash,
					SplashBitmap *parentBitmap,
					int *xOut, int *yOut) {
  // get the clip bbox
  double fxMin, fyMin, fxMax, fyMax;
  state->getClipBBox(&fxMin, &fyMin, &fxMax, &fyMax);
  if (fxMin > fxMax || fyMin > fyMax) {
    return NULL;
  }

  // get the shading bbox
  double tx0, ty0, tx1, ty1, dx, dy, txMin, tyMin, txMax, tyMax;
  shading->getBBox(&tx0, &ty0, &tx1, &ty1);
  state->transform(tx0, ty0, &dx, &dy);
  txMin = txMax = dx;
  tyMin = tyMax = dy;
  state->transform(tx0, ty1, &dx, &dy);
  if (dx < txMin) {
    txMin = dx;
  } else if (dx > txMax) {
    txMax = dx;
  }
  if (dy < tyMin) {
    tyMin = dy;
  } else if (dy > tyMax) {
    tyMax = dy;
  }
  state->transform(tx1, ty0, &dx, &dy);
  if (dx < txMin) {
    txMin = dx;
  } else if (dx > txMax) {
    txMax = dx;
  }
  if (dy < tyMin) {
    tyMin = dy;
  } else if (dy > tyMax) {
    tyMax = dy;
  }
  state->transform(tx1, ty1, &dx, &dy);
  if (dx < txMin) {
    txMin = dx;
  } else if (dx > txMax) {
    txMax = dx;
  }
  if (dy < tyMin) {
    tyMin = dy;
  } else if (dy > tyMax) {
    tyMax = dy;
  }
  if (txMin > fxMin) {
    fxMin = txMin;
  }
  if (txMax < fxMax) {
    fxMax = txMax;
  }
  if (tyMin > fyMin) {
    fyMin = tyMin;
  }
  if (tyMax < fyMax) {
    fyMax = tyMax;
  }
  if (fxMin > fxMax || fyMin > fyMax) {
    return NULL;
  }

  // convert to integer coords
  int xMin = (int)floor(fxMin);
  int yMin = (int)floor(fyMin);
  int xMax = (int)floor(fxMax) + 1;
  int yMax = (int)floor(fyMax) + 1;
  int bitmapWidth = xMax - xMin;
  int bitmapHeight = yMax - yMin;

  // allocate the bitmap
  traceMessage("Gouraud triangle shading fill bitmap");
  SplashBitmap *bitmap = new SplashBitmap(bitmapWidth, bitmapHeight, 1, mode,
					  gTrue, gTrue, parentBitmap);

  // allocate a Splash object
  // vector antialiasing is disabled to avoid artifacts along triangle edges
  Splash *splash = new Splash(bitmap, gFalse,
			      parentSplash->getImageCache(),
			      parentSplash->getScreen());
  SplashColor zero;
  for (int i = 0; i < splashColorModeNComps[mode]; ++i) {
    zero[i] = 0;
  }
  splash->clear(zero, 0x00);

  // draw the patches
  int start;
  if (shading->getNPatches() > 128) {
    start = 3;
  } else if (shading->getNPatches() > 64) {
    start = 2;
  } else if (shading->getNPatches() > 16) {
    start = 1;
  } else {
    start = 0;
  }
  for (int i = 0; i < shading->getNPatches(); ++i) {
    fillPatch(state, splash, mode, reverseVideo,
	      xMin, yMin, shading->getPatch(i), shading, start);
  }

  delete splash;

  *xOut = xMin;
  *yOut = yMin;
  return bitmap;
}

void ShadingImage::fillPatch(GfxState *state, Splash *splash,
			     SplashColorMode mode, GBool reverseVideo,
			     int xMin, int yMin,
			     GfxPatch *patch,
			     GfxPatchMeshShading *shading,
			     int depth) {
  GfxColor c00;
  shading->getColor(patch->color[0][0], &c00);
  GBool stop = gFalse;

  // stop subdivision at max depth
  if (depth == patchMaxDepth) {
    stop = gTrue;
  }

  // stop subdivision if colors are close enough
  if (!stop) {
    int nComps = shading->getColorSpace()->getNComps();
    GfxColor c01, c10, c11;
    shading->getColor(patch->color[0][1], &c01);
    shading->getColor(patch->color[1][0], &c10);
    shading->getColor(patch->color[1][1], &c11);
    int i;
    for (i = 0; i < nComps; ++i) {
      if (abs(c00.c[i] - c01.c[i]) > patchColorDelta ||
	  abs(c01.c[i] - c11.c[i]) > patchColorDelta ||
	  abs(c11.c[i] - c10.c[i]) > patchColorDelta ||
	  abs(c10.c[i] - c00.c[i]) > patchColorDelta) {
	break;
      }
    }
    if (i == nComps) {
      stop = gTrue;
    }
  }

  // stop subdivision if patch is small enough
  if (!stop) {
    double xxMin = 0;
    double yyMin = 0;
    double xxMax = 0;
    double yyMax = 0;
    for (int j = 0; j < 4; ++j) {
      for (int i = 0; i < 4; ++i) {
	double xx, yy;
	state->transformDelta(patch->x[i][j], patch->y[i][j], &xx, &yy);
	if (i == 0 && j == 0) {
	  xxMin = xxMax = xx;
	  yyMin = yyMax = yy;
	} else {
	  if (xx < xxMin) {
	    xxMin = xx;
	  } else if (xx > xxMax) {
	    xxMax = xx;
	  }
	  if (yy < yyMin) {
	    yyMin = yy;
	  } else if (yy > yyMax) {
	    yyMax = yy;
	  }
	}
      }
    }
    if (xxMax - xxMin < 1 && yyMax - yyMin < 1) {
      stop = gTrue;
    }
  }

  // draw the patch
  if (stop) {
    SplashColor sColor;
    computeShadingColor(state, mode, reverseVideo, &c00, sColor);
    splash->setFillPattern(new SplashSolidColor(sColor));
    SplashPath *path = new SplashPath();
    double xx0, yy0, xx1, yy1, xx2, yy2, xx3, yy3;
    state->transform(patch->x[0][0], patch->y[0][0], &xx0, &yy0);
    path->moveTo(xx0 - xMin, yy0 - yMin);
    state->transform(patch->x[0][1], patch->y[0][1], &xx1, &yy1);
    state->transform(patch->x[0][2], patch->y[0][2], &xx2, &yy2);
    state->transform(patch->x[0][3], patch->y[0][3], &xx3, &yy3);
    path->curveTo(xx1 - xMin, yy1 - yMin, xx2 - xMin, yy2 - yMin,
		  xx3 - xMin, yy3 - yMin);
    state->transform(patch->x[1][3], patch->y[1][3], &xx1, &yy1);
    state->transform(patch->x[2][3], patch->y[2][3], &xx2, &yy2);
    state->transform(patch->x[3][3], patch->y[3][3], &xx3, &yy3);
    path->curveTo(xx1 - xMin, yy1 - yMin, xx2 - xMin, yy2 - yMin,
		  xx3 - xMin, yy3 - yMin);
    state->transform(patch->x[3][2], patch->y[3][2], &xx1, &yy1);
    state->transform(patch->x[3][1], patch->y[3][1], &xx2, &yy2);
    state->transform(patch->x[3][0], patch->y[3][0], &xx3, &yy3);
    path->curveTo(xx1 - xMin, yy1 - yMin, xx2 - xMin, yy2 - yMin,
		  xx3 - xMin, yy3 - yMin);
    state->transform(patch->x[2][0], patch->y[2][0], &xx1, &yy1);
    state->transform(patch->x[1][0], patch->y[1][0], &xx2, &yy2);
    path->curveTo(xx1 - xMin, yy1 - yMin, xx2 - xMin, yy2 - yMin,
		  xx0 - xMin, yy0 - yMin);
    path->close();
    splash->fill(path, gFalse);
    delete path;

  // subdivide the patch
  } else {
    double xx[4][8], yy[4][8];
    for (int i = 0; i < 4; ++i) {
      xx[i][0] = patch->x[i][0];
      yy[i][0] = patch->y[i][0];
      xx[i][1] = 0.5 * (patch->x[i][0] + patch->x[i][1]);
      yy[i][1] = 0.5 * (patch->y[i][0] + patch->y[i][1]);
      double xxm = 0.5 * (patch->x[i][1] + patch->x[i][2]);
      double yym = 0.5 * (patch->y[i][1] + patch->y[i][2]);
      xx[i][6] = 0.5 * (patch->x[i][2] + patch->x[i][3]);
      yy[i][6] = 0.5 * (patch->y[i][2] + patch->y[i][3]);
      xx[i][2] = 0.5 * (xx[i][1] + xxm);
      yy[i][2] = 0.5 * (yy[i][1] + yym);
      xx[i][5] = 0.5 * (xxm + xx[i][6]);
      yy[i][5] = 0.5 * (yym + yy[i][6]);
      xx[i][3] = xx[i][4] = 0.5 * (xx[i][2] + xx[i][5]);
      yy[i][3] = yy[i][4] = 0.5 * (yy[i][2] + yy[i][5]);
      xx[i][7] = patch->x[i][3];
      yy[i][7] = patch->y[i][3];
    }
    GfxPatch patch00, patch01, patch10, patch11;
    for (int i = 0; i < 4; ++i) {
      patch00.x[0][i] = xx[0][i];
      patch00.y[0][i] = yy[0][i];
      patch00.x[1][i] = 0.5 * (xx[0][i] + xx[1][i]);
      patch00.y[1][i] = 0.5 * (yy[0][i] + yy[1][i]);
      double xxm = 0.5 * (xx[1][i] + xx[2][i]);
      double yym = 0.5 * (yy[1][i] + yy[2][i]);
      patch10.x[2][i] = 0.5 * (xx[2][i] + xx[3][i]);
      patch10.y[2][i] = 0.5 * (yy[2][i] + yy[3][i]);
      patch00.x[2][i] = 0.5 * (patch00.x[1][i] + xxm);
      patch00.y[2][i] = 0.5 * (patch00.y[1][i] + yym);
      patch10.x[1][i] = 0.5 * (xxm + patch10.x[2][i]);
      patch10.y[1][i] = 0.5 * (yym + patch10.y[2][i]);
      patch00.x[3][i] = 0.5 * (patch00.x[2][i] + patch10.x[1][i]);
      patch00.y[3][i] = 0.5 * (patch00.y[2][i] + patch10.y[1][i]);
      patch10.x[0][i] = patch00.x[3][i];
      patch10.y[0][i] = patch00.y[3][i];
      patch10.x[3][i] = xx[3][i];
      patch10.y[3][i] = yy[3][i];
    }
    for (int i = 4; i < 8; ++i) {
      patch01.x[0][i-4] = xx[0][i];
      patch01.y[0][i-4] = yy[0][i];
      patch01.x[1][i-4] = 0.5 * (xx[0][i] + xx[1][i]);
      patch01.y[1][i-4] = 0.5 * (yy[0][i] + yy[1][i]);
      double xxm = 0.5 * (xx[1][i] + xx[2][i]);
      double yym = 0.5 * (yy[1][i] + yy[2][i]);
      patch11.x[2][i-4] = 0.5 * (xx[2][i] + xx[3][i]);
      patch11.y[2][i-4] = 0.5 * (yy[2][i] + yy[3][i]);
      patch01.x[2][i-4] = 0.5 * (patch01.x[1][i-4] + xxm);
      patch01.y[2][i-4] = 0.5 * (patch01.y[1][i-4] + yym);
      patch11.x[1][i-4] = 0.5 * (xxm + patch11.x[2][i-4]);
      patch11.y[1][i-4] = 0.5 * (yym + patch11.y[2][i-4]);
      patch01.x[3][i-4] = 0.5 * (patch01.x[2][i-4] + patch11.x[1][i-4]);
      patch01.y[3][i-4] = 0.5 * (patch01.y[2][i-4] + patch11.y[1][i-4]);
      patch11.x[0][i-4] = patch01.x[3][i-4];
      patch11.y[0][i-4] = patch01.y[3][i-4];
      patch11.x[3][i-4] = xx[3][i];
      patch11.y[3][i-4] = yy[3][i];
    }
    for (int i = 0; i < shading->getNComps(); ++i) {
      patch00.color[0][0][i] = patch->color[0][0][i];
      patch00.color[0][1][i] = 0.5 * (patch->color[0][0][i] +
				      patch->color[0][1][i]);
      patch01.color[0][0][i] = patch00.color[0][1][i];
      patch01.color[0][1][i] = patch->color[0][1][i];
      patch01.color[1][1][i] = 0.5 * (patch->color[0][1][i] +
				      patch->color[1][1][i]);
      patch11.color[0][1][i] = patch01.color[1][1][i];
      patch11.color[1][1][i] = patch->color[1][1][i];
      patch11.color[1][0][i] = 0.5 * (patch->color[1][1][i] +
				      patch->color[1][0][i]);
      patch10.color[1][1][i] = patch11.color[1][0][i];
      patch10.color[1][0][i] = patch->color[1][0][i];
      patch10.color[0][0][i] = 0.5 * (patch->color[1][0][i] +
				      patch->color[0][0][i]);
      patch00.color[1][0][i] = patch10.color[0][0][i];
      patch00.color[1][1][i] = 0.5 * (patch00.color[1][0][i] +
				      patch01.color[1][1][i]);
      patch01.color[1][0][i] = patch00.color[1][1][i];
      patch11.color[0][0][i] = patch00.color[1][1][i];
      patch10.color[0][1][i] = patch00.color[1][1][i];
    }
    fillPatch(state, splash, mode, reverseVideo, xMin, yMin, &patch00,
	      shading, depth + 1);
    fillPatch(state, splash, mode, reverseVideo, xMin, yMin, &patch10,
	      shading, depth + 1);
    fillPatch(state, splash, mode, reverseVideo, xMin, yMin, &patch01,
	      shading, depth + 1);
    fillPatch(state, splash, mode, reverseVideo, xMin, yMin, &patch11,
	      shading, depth + 1);
  }
}

void ShadingImage::computeShadingColor(GfxState *state,
				       SplashColorMode mode,
				       GBool reverseVideo,
				       GfxColor *color,
				       SplashColorPtr sColor) {
  GfxGray gray;
  GfxRGB rgb;
#if SPLASH_CMYK
  GfxCMYK cmyk;
#endif

  state->setFillColor(color);
  switch (mode) {
  case splashModeMono8:
    state->getFillGray(&gray);
    if (reverseVideo) {
      gray = gfxColorComp1 - gray;
    }
    sColor[0] = colToByte(gray);
    break;
  case splashModeRGB8:
    state->getFillRGB(&rgb);
    if (reverseVideo) {
      rgb.r = gfxColorComp1 - rgb.r;
      rgb.g = gfxColorComp1 - rgb.g;
      rgb.b = gfxColorComp1 - rgb.b;
    }
    sColor[0] = colToByte(rgb.r);
    sColor[1] = colToByte(rgb.g);
    sColor[2] = colToByte(rgb.b);
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    state->getFillCMYK(&cmyk);
    sColor[0] = colToByte(cmyk.c);
    sColor[1] = colToByte(cmyk.m);
    sColor[2] = colToByte(cmyk.y);
    sColor[3] = colToByte(cmyk.k);
    break;
#endif
  case splashModeMono1:
  case splashModeBGR8:
    // mode cannot be Mono1 or BGR8
    break;
  }
}

// Transform a user space bbox to a device space bbox.
void ShadingImage::transformBBox(GfxState *state,
				 double uxMin, double uyMin,
				 double uxMax, double uyMax,
				 double *dxMin, double *dyMin,
				 double *dxMax, double *dyMax) {
  double tx, ty;
  state->transform(uxMin, uyMin, &tx, &ty);
  *dxMin = *dxMax = tx;
  *dyMin = *dyMax = ty;
  state->transform(uxMin, uyMax, &tx, &ty);
  if (tx < *dxMin) {
    *dxMin = tx;
  } else if (tx > *dxMax) {
    *dxMax = tx;
  }
  if (ty < *dyMin) {
    *dyMin = ty;
  } else if (ty > *dyMax) {
    *dyMax = ty;
  }
  state->transform(uxMax, uyMin, &tx, &ty);
  if (tx < *dxMin) {
    *dxMin = tx;
  } else if (tx > *dxMax) {
    *dxMax = tx;
  }
  if (ty < *dyMin) {
    *dyMin = ty;
  } else if (ty > *dyMax) {
    *dyMax = ty;
  }
  state->transform(uxMax, uyMax, &tx, &ty);
  if (tx < *dxMin) {
    *dxMin = tx;
  } else if (tx > *dxMax) {
    *dxMax = tx;
  }
  if (ty < *dyMin) {
    *dyMin = ty;
  } else if (ty > *dyMax) {
    *dyMax = ty;
  }
}

