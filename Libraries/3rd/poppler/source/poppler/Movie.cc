//*********************************************************************************
//                               Movie.cc
//---------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------
// Hugo Mercier <hmercier31[at]gmail.com> (c) 2008
// Pino Toscano <pino@kde.org> (c) 2008
// Carlos Garcia Campos <carlosgc@gnome.org> (c) 2010
// Albert Astals Cid <aacid@kde.org> (c) 2010, 2017
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//*********************************************************************************

#include <math.h>
#include "Movie.h"
#include "FileSpec.h"

MovieActivationParameters::MovieActivationParameters() {
  // default values
  floatingWindow = gFalse;
  xPosition = 0.5;
  yPosition = 0.5;
  rate = 1.0;
  volume = 100;
  showControls = gFalse;
  synchronousPlay = gFalse;
  repeatMode = repeatModeOnce;
  start.units = 0;
  duration.units = 0;
  znum = 1;
  zdenum = 1;
}

MovieActivationParameters::~MovieActivationParameters() {
}

void MovieActivationParameters::parseMovieActivation(Object* aDict) {
  Object obj1 = aDict->dictLookup("Start");
  if (obj1.isNull()) {
    if (obj1.isInt()) {
      // If it is representable as an integer (subject to the implementation limit for
      // integers, as described in Appendix C), it should be specified as such.

      start.units = obj1.getInt();
    } else if (obj1.isString()) {
      // If it is not representable as an integer, it should be specified as an 8-byte
      // string representing a 64-bit twos-complement integer, most significant
      // byte first.

      // UNSUPPORTED
    } else if (obj1.isArray()) {
      Array* a = obj1.getArray();

      Object tmp = a->get(0);
      if (tmp.isInt()) {
        start.units = tmp.getInt();
      }
      if (tmp.isString()) {
        // UNSUPPORTED
      }

      tmp = a->get(1);
      if (tmp.isInt()) {
        start.units_per_second = tmp.getInt();
      }
    }
  }

  obj1 = aDict->dictLookup("Duration");
  if (obj1.isNull()) {
    if (obj1.isInt()) {
      duration.units = obj1.getInt();
    } else if (obj1.isString()) {
      // UNSUPPORTED
    } else if (obj1.isArray()) {
      Array* a = obj1.getArray();

      Object tmp = a->get(0);
      if (tmp.isInt()) {
        duration.units = tmp.getInt();
      }
      if (tmp.isString()) {
        // UNSUPPORTED
      }

      tmp = a->get(1);
      if (tmp.isInt()) {
        duration.units_per_second = tmp.getInt();
      }
    }
  }

  obj1 = aDict->dictLookup("Rate");
  if (obj1.isNum()) {
    rate = obj1.getNum();
  }

  obj1 = aDict->dictLookup("Volume");
  if (obj1.isNum()) {
    // convert volume to [0 100]
    volume = int((obj1.getNum() + 1.0) * 50);
  }

  obj1 = aDict->dictLookup("ShowControls");
  if (obj1.isBool()) {
    showControls = obj1.getBool();
  }

  obj1 = aDict->dictLookup("Synchronous");
  if (obj1.isBool()) {
    synchronousPlay = obj1.getBool();
  }

  obj1 = aDict->dictLookup("Mode");
  if (obj1.isName()) {
    char* name = obj1.getName();
    if (!strcmp(name, "Once")) {
      repeatMode = repeatModeOnce;
    } else if (!strcmp(name, "Open")) {
      repeatMode = repeatModeOpen;
    } else if (!strcmp(name, "Repeat")) {
      repeatMode = repeatModeRepeat;
    } else if (!strcmp(name,"Palindrome")) {
      repeatMode = repeatModePalindrome;
    }
  }

  obj1 = aDict->dictLookup("FWScale");
  if (obj1.isArray()) {
    // the presence of that entry implies that the movie is to be played
    // in a floating window
    floatingWindow = gTrue;

    Array* scale = obj1.getArray();
    if (scale->getLength() >= 2) {
      Object tmp = scale->get(1);
      if (tmp.isInt()) {
        znum = tmp.getInt();
      }
      tmp = scale->get(1);
      if (tmp.isInt()) {
        zdenum = tmp.getInt();
      }
    }
  }

  obj1 = aDict->dictLookup("FWPosition");
  if (obj1.isArray()) {
    Array* pos = obj1.getArray();
    if (pos->getLength() >= 2) {
      Object tmp = pos->get(0);
      if (tmp.isNum()) {
        xPosition = tmp.getNum();
      }
      tmp = pos->get(1);
      if (tmp.isNum()) {
        yPosition = tmp.getNum();
      }
    }
  }
}

void Movie::parseMovie (Object *movieDict) {
  fileName = NULL;
  rotationAngle = 0;
  width = -1;
  height = -1;
  showPoster = gFalse;

  Object obj1 = movieDict->dictLookup("F");
  Object obj2 = getFileSpecNameForPlatform(&obj1);
  if (obj2.isString()) {
    fileName = obj2.getString()->copy();
  } else {
    error (errSyntaxError, -1, "Invalid Movie");
    ok = gFalse;
    return;
  }

  obj1 = movieDict->dictLookup("Aspect");
  if (obj1.isArray()) {
    Array* aspect = obj1.getArray();
    if (aspect->getLength() >= 2) {
      Object tmp = aspect->get(0);
      if( tmp.isNum() ) {
        width = (int)floor( tmp.getNum() + 0.5 );
      }
      tmp = aspect->get(1);
      if( tmp.isNum() ) {
        height = (int)floor( tmp.getNum() + 0.5 );
      }
    }
  }

  obj1 = movieDict->dictLookup("Rotate");
  if (obj1.isInt()) {
    // round up to 90°
    rotationAngle = (((obj1.getInt() + 360) % 360) % 90) * 90;
  }

  //
  // movie poster
  //
  poster = movieDict->dictLookupNF("Poster");
  if (!poster.isNull()) {
    if (poster.isRef() || poster.isStream()) {
      showPoster = gTrue;
    } else if (poster.isBool()) {
      showPoster = poster.getBool();
      poster.setToNull();
    } else {
      poster.setToNull();
    }
  }
}

Movie::~Movie() {
  delete fileName;
}

Movie::Movie(Object *movieDict) {
  ok = gTrue;

  if (movieDict->isDict())
    parseMovie(movieDict);
  else
    ok = gFalse;
}

Movie::Movie(Object *movieDict, Object *aDict) {
  ok = gTrue;

  if (movieDict->isDict()) {
    parseMovie(movieDict);
    if (aDict->isDict())
      MA.parseMovieActivation(aDict);
  } else {
    ok = gFalse;
  }
}

Movie::Movie(const Movie &other)
{
  ok = other.ok;
  rotationAngle = other.rotationAngle;
  width = other.width;
  height = other.height;
  showPoster = other.showPoster;
  MA = other.MA;

  poster = other.poster.copy();

  if (other.fileName)
    fileName = other.fileName->copy();
  else
    fileName = nullptr;
}

void Movie::getFloatingWindowSize(int *widthA, int *heightA)
{
  *widthA = int(width * double(MA.znum) / MA.zdenum);
  *heightA = int(height * double(MA.znum) / MA.zdenum);
}

Movie* Movie::copy() {
  return new Movie(*this);
}
