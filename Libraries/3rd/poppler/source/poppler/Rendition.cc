//*********************************************************************************
//                               Rendition.cc
//---------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------
// Hugo Mercier <hmercier31[at]gmail.com> (c) 2008
// Pino Toscano <pino@kde.org> (c) 2008
// Carlos Garcia Campos <carlosgc@gnome.org> (c) 2010
// Tobias Koenig <tobias.koenig@kdab.com> (c) 2012
// Albert Astals Cid <aacid@kde.org> (C) 2017
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
#include "Rendition.h"
#include "FileSpec.h"

MediaWindowParameters::MediaWindowParameters() {
  // default values
  type = windowEmbedded;
  width = -1;
  height = -1;
  relativeTo = windowRelativeToDocument;
  XPosition = 0.5;
  YPosition = 0.5;
  hasTitleBar = gTrue;
  hasCloseButton = gTrue;
  isResizeable = gTrue;
}

MediaWindowParameters::~MediaWindowParameters() {
}

void MediaWindowParameters::parseFWParams(Object* obj) {
  Object tmp = obj->dictLookup("D");
  if (tmp.isArray()) {
    Array * dim = tmp.getArray();
    
    if (dim->getLength() >= 2) {
      Object dd = dim->get(0);
      if (dd.isInt()) {
	width = dd.getInt();
      }

      dd = dim->get(1);
      if (dd.isInt()) {
	height = dd.getInt();
      }

    }
  }

  tmp = obj->dictLookup("RT");
  if (tmp.isInt()) {
    int t = tmp.getInt();
    switch(t) {
    case 0: relativeTo = windowRelativeToDocument; break;
    case 1: relativeTo = windowRelativeToApplication; break;
    case 2: relativeTo = windowRelativeToDesktop; break;
    }
  }

  tmp = obj->dictLookup("P");
  if (tmp.isInt()) {
    int t = tmp.getInt();

    switch(t) {
    case 0: // Upper left
      XPosition = 0.0;
      YPosition = 0.0;
      break;
    case 1: // Upper Center
      XPosition = 0.5;
      YPosition = 0.0;
      break;
    case 2: // Upper Right
      XPosition = 1.0;
      YPosition = 0.0;
      break;
    case 3: // Center Left
      XPosition = 0.0;
      YPosition = 0.5;
      break;
    case 4: // Center
      XPosition = 0.5;
      YPosition = 0.5;
      break;
    case 5: // Center Right
      XPosition = 1.0;
      YPosition = 0.5;
      break;
    case 6: // Lower Left
      XPosition = 0.0;
      YPosition = 1.0;
      break;
    case 7: // Lower Center
      XPosition = 0.5;
      YPosition = 1.0;
      break;
    case 8: // Lower Right
      XPosition = 1.0;
      YPosition = 1.0;
      break;
    }
  }

  tmp = obj->dictLookup("T");
  if (tmp.isBool()) {
    hasTitleBar = tmp.getBool();
  }
  tmp = obj->dictLookup("UC");
  if (tmp.isBool()) {
    hasCloseButton = tmp.getBool();
  }
  tmp = obj->dictLookup("R");
  if (tmp.isInt()) {
    isResizeable = (tmp.getInt() != 0);
  }
}

MediaParameters::MediaParameters() {
  // instanciate to default values

  volume = 100;
  fittingPolicy = fittingUndefined;
  autoPlay = gTrue;
  repeatCount = 1.0;
  opacity = 1.0;
  showControls = gFalse;
  duration = 0;
}

MediaParameters::~MediaParameters() {
}

void MediaParameters::parseMediaPlayParameters(Object* obj) {
  Object tmp = obj->dictLookup("V");
  if (tmp.isInt()) {
    volume = tmp.getInt();
  }

  tmp = obj->dictLookup("C");
  if (tmp.isBool()) {
    showControls = tmp.getBool();
  }

  tmp = obj->dictLookup("F");
  if (tmp.isInt()) {
    int t = tmp.getInt();
    
    switch(t) {
    case 0: fittingPolicy = fittingMeet; break;
    case 1: fittingPolicy = fittingSlice; break;
    case 2: fittingPolicy = fittingFill; break;
    case 3: fittingPolicy = fittingScroll; break;
    case 4: fittingPolicy = fittingHidden; break;
    case 5: fittingPolicy = fittingUndefined; break;
    }
  }

  // duration parsing
  // duration's default value is set to 0, which means : intrinsinc media duration
  tmp = obj->dictLookup("D");
  if (tmp.isDict()) {
    Object oname = tmp.dictLookup("S");
    if (oname.isName()) {
      char* name = oname.getName();
      if (!strcmp(name, "F"))
	duration = -1; // infinity
      else if (!strcmp(name, "T")) {
	Object ddict = tmp.dictLookup("T");
	if (ddict.isDict()) {
	  Object tmp2 = ddict.dictLookup("V");
	  if (tmp2.isNum()) {
	    duration = Gulong(tmp2.getNum());
	  }
	}
      }
    }
  }

  tmp = obj->dictLookup("A");
  if (tmp.isBool()) {
    autoPlay = tmp.getBool();
  }

  tmp = obj->dictLookup("RC");
  if (tmp.isNum()) {
    repeatCount = tmp.getNum();
  }
}

void MediaParameters::parseMediaScreenParameters(Object* obj) {
  Object tmp = obj->dictLookup("W");
  if (tmp.isInt()) {
    int t = tmp.getInt();
    
    switch(t) {
    case 0: windowParams.type = MediaWindowParameters::windowFloating; break;
    case 1: windowParams.type = MediaWindowParameters::windowFullscreen; break;
    case 2: windowParams.type = MediaWindowParameters::windowHidden; break;
    case 3: windowParams.type = MediaWindowParameters::windowEmbedded; break;
    }
  }

  // background color
  tmp = obj->dictLookup("B");
  if (tmp.isArray()) {
    Array* color = tmp.getArray();

    Object component = color->get(0);
    bgColor.r = component.getNum();

    component = color->get(1);
    bgColor.g = component.getNum();

    component = color->get(2);
    bgColor.b = component.getNum();
  }

  // opacity
  tmp = obj->dictLookup("O");
  if (tmp.isNum()) {
    opacity = tmp.getNum();
  }

  if (windowParams.type == MediaWindowParameters::windowFloating) {
    Object winDict = obj->dictLookup("F");
    if (winDict.isDict()) {
      windowParams.parseFWParams(&winDict);
    }
  }
}

MediaRendition::~MediaRendition() {
  delete fileName;
  delete contentType;
}

MediaRendition::MediaRendition(Object* obj) {
  GBool hasClip = gFalse;

  ok = gTrue;
  fileName = NULL;
  contentType = NULL;
  isEmbedded = gFalse;

  //
  // Parse media clip data
  //
  Object tmp2 = obj->dictLookup("C");
  if (tmp2.isDict()) { // media clip
    hasClip = gTrue;
    Object tmp = tmp2.dictLookup("S");
    if (tmp.isName()) {
      if (!strcmp(tmp.getName(), "MCD")) { // media clip data
        Object obj1 = tmp2.dictLookup("D");
	if (obj1.isDict()) {
	  Object obj2 = obj1.dictLookup("F");
	  if (obj2.isString()) {
	    fileName = obj2.getString()->copy();
	  }
	  obj2 = obj1.dictLookup("EF");
	  if (obj2.isDict()) {
	    Object embedded = obj2.dictLookup("F");
	    if (embedded.isStream()) {
	      isEmbedded = gTrue;
	      embeddedStreamObject = embedded.copy();
	    }
	  }

	  // TODO: D might be a form XObject too
	} else {
	  error (errSyntaxError, -1, "Invalid Media Clip Data");
	  ok = gFalse;
	}

	// FIXME: ignore CT if D is a form XObject
	obj1 = tmp2.dictLookup("CT");
	if (obj1.isString()) {
	  contentType = obj1.getString()->copy();
	}
      } else if (!strcmp(tmp.getName(), "MCS")) { // media clip data
        // TODO
      }
    } else {
      error (errSyntaxError, -1, "Invalid Media Clip");
      ok = gFalse;
    }
  }

  if (!ok)
    return;

  //
  // parse Media Play Parameters
  tmp2 = obj->dictLookup("P");
  if (tmp2.isDict()) { // media play parameters
    Object params = tmp2.dictLookup("MH");
    if (params.isDict()) {
      MH.parseMediaPlayParameters(&params);
    }
    params = tmp2.dictLookup("BE");
    if (params.isDict()) {
      BE.parseMediaPlayParameters(&params);
    }
  } else if (!hasClip) {
    error (errSyntaxError, -1, "Invalid Media Rendition");
    ok = gFalse;
  }

  //
  // parse Media Screen Parameters
  tmp2 = obj->dictLookup("SP");
  if (tmp2.isDict()) { // media screen parameters
    Object params = tmp2.dictLookup("MH");
    if (params.isDict()) {
      MH.parseMediaScreenParameters(&params);
    }
    params = tmp2.dictLookup("BE");
    if (params.isDict()) {
      BE.parseMediaScreenParameters(&params);
    }
  }
}

MediaRendition::MediaRendition(const MediaRendition &other) {
  ok = other.ok;
  MH = other.MH;
  BE = other.BE;
  isEmbedded = other.isEmbedded;
  embeddedStreamObject = other.embeddedStreamObject.copy();

  if (other.contentType)
    contentType = other.contentType->copy();
  else
    contentType = nullptr;

  if (other.fileName)
    fileName = other.fileName->copy();
  else
    fileName = nullptr;
}

void MediaRendition::outputToFile(FILE* fp) {
  if (!isEmbedded)
    return;

  embeddedStreamObject.streamReset();

  while (1) {
    int c = embeddedStreamObject.streamGetChar();
    if (c == EOF)
      break;
    
    fwrite(&c, 1, 1, fp);
  }
  
}

MediaRendition* MediaRendition::copy()
{
  return new MediaRendition(*this);
}

// TODO: SelectorRendition
