//========================================================================
//
// XFAScanner.cc
//
// Copyright 2020 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "GString.h"
#include "GHash.h"
#include "Object.h"
#include "Error.h"
#include "Zoox.h"
#include "XFAScanner.h"

//------------------------------------------------------------------------

// fields have two names:
//
// name:
//   - nodes with bind=global set the index to 0 ("foo[0]") regardless
//     of the number of nodes with the same name
//   - nodes with bind=none are dropped from the name
//   - <area> nodes are dropped from the name
//   - used for field value lookup in <xfa:datasets>
//
// fullName:
//   - all named nodes are treated the same, regardless of bind=global
//     or bind=none
//   - <area> nodes are included in the name, but don't reset the
//     numbering (i.e., <area> nodes are "transparent" with respect to
//     node numbering)
//   - used for field value lookup in <form>
//   - used for matching with AcroForm names
//
// Both names use indexes on all nodes, even if there's only one node
// with the name -- this isn't correct for XFA naming, but it matches
// the AcroForm behavior.

//------------------------------------------------------------------------

XFAFieldLayoutInfo::XFAFieldLayoutInfo(XFAFieldLayoutHAlign hAlignA,
				       XFAFieldLayoutVAlign vAlignA) {
  hAlign = hAlignA;
  vAlign = vAlignA;
}

//------------------------------------------------------------------------

XFAFieldPictureInfo::XFAFieldPictureInfo(XFAFieldPictureSubtype subtypeA,
					 GString *formatA) {
  subtype = subtypeA;
  format = formatA;
}

XFAFieldPictureInfo::~XFAFieldPictureInfo() {
  delete format;
}

//------------------------------------------------------------------------

XFAFieldBarcodeInfo::XFAFieldBarcodeInfo(GString *barcodeTypeA,
					 double wideNarrowRatioA,
					 double moduleWidthA,
					 double moduleHeightA,
					 int dataLengthA,
					 int errorCorrectionLevelA,
					 GString *textLocationA) {
  barcodeType = barcodeTypeA;
  wideNarrowRatio = wideNarrowRatioA;
  moduleWidth = moduleWidthA;
  moduleHeight = moduleHeightA;
  dataLength = dataLengthA;
  errorCorrectionLevel = errorCorrectionLevelA;
  textLocation = textLocationA;
}

XFAFieldBarcodeInfo::~XFAFieldBarcodeInfo() {
  delete barcodeType;
  delete textLocation;
}

//------------------------------------------------------------------------

XFAField::XFAField(GString *nameA, GString *fullNameA, GString *valueA,
		   XFAFieldLayoutInfo *layoutInfoA,
		   XFAFieldPictureInfo *pictureInfoA,
		   XFAFieldBarcodeInfo *barcodeInfoA)
  : name(nameA)
  , fullName(fullNameA)
  , value(valueA)
  , layoutInfo(layoutInfoA)
  , pictureInfo(pictureInfoA)
  , barcodeInfo(barcodeInfoA)
{
}

XFAField::~XFAField() {
  delete name;
  delete fullName;
  delete value;
  delete layoutInfo;
  delete pictureInfo;
  delete barcodeInfo;
}

//------------------------------------------------------------------------

XFAScanner *XFAScanner::load(Object *xfaObj) {
  GString *xfaData = readXFAStreams(xfaObj);
  if (!xfaData) {
    return NULL;
  }
  ZxDoc *xml = ZxDoc::loadMem(xfaData->getCString(), xfaData->getLength());
  delete xfaData;
  if (!xml) {
    error(errSyntaxError, -1, "Invalid XML in XFA form");
    return NULL;
  }

  XFAScanner *scanner = new XFAScanner();

  if (xml->getRoot()) {
    GHash *formValues = scanner->scanFormValues(xml->getRoot());
    ZxElement *dataElem = NULL;
    ZxElement *datasets =
        xml->getRoot()->findFirstChildElement("xfa:datasets");
    if (datasets) {
      dataElem = datasets->findFirstChildElement("xfa:data");
    }
    ZxElement *tmpl = xml->getRoot()->findFirstChildElement("template");
    if (tmpl) {
      scanner->scanNode(tmpl, NULL, NULL, NULL, NULL, NULL,
			dataElem, formValues);
    }
    deleteGHash(formValues, GString);
  }

  delete xml;

  return scanner;
}

XFAScanner::XFAScanner() {
  fields = new GHash();
}

XFAScanner::~XFAScanner() {
  deleteGHash(fields, XFAField);
}

XFAField *XFAScanner::findField(GString *acroFormFieldName) {
  return (XFAField *)fields->lookup(acroFormFieldName);
}

GString *XFAScanner::readXFAStreams(Object *xfaObj) {
  GString *data = new GString();
  char buf[4096];
  int n;
  if (xfaObj->isStream()) {
    xfaObj->streamReset();
    while ((n = xfaObj->getStream()->getBlock(buf, sizeof(buf))) > 0) {
      data->append(buf, n);
    }
  } else if (xfaObj->isArray()) {
    for (int i = 1; i < xfaObj->arrayGetLength(); i += 2) {
      Object obj;
      if (!xfaObj->arrayGet(i, &obj)->isStream()) {
	error(errSyntaxError, -1, "XFA array element is wrong type");
	obj.free();
	delete data;
	return NULL;
      }
      obj.streamReset();
      while ((n = obj.getStream()->getBlock(buf, sizeof(buf))) > 0) {
	data->append(buf, n);
      }
      obj.free();
    }
  } else {
    error(errSyntaxError, -1, "XFA object is wrong type");
    return NULL;
  }
  return data;
}

GHash *XFAScanner::scanFormValues(ZxElement *xmlRoot) {
  GHash *formValues = new GHash(gTrue);
  ZxElement *formElem = xmlRoot->findFirstChildElement("form");
  if (formElem) {
    scanFormNode(formElem, NULL, formValues);
  }
  return formValues;
}

void XFAScanner::scanFormNode(ZxElement *elem, GString *fullName,
			      GHash *formValues) {
  GHash *fullNameIdx = new GHash();
  for (ZxNode *node = elem->getFirstChild();
       node;
       node = node->getNextChild()) {
    if (node->isElement("value")) {
      if (fullName) {
	ZxNode *child1Node = ((ZxElement *)node)->getFirstChild();
	if (child1Node && child1Node->isElement()) {
	  ZxNode *child2Node = ((ZxElement *)child1Node)->getFirstChild();
	  if (child2Node && child2Node->isCharData()) {
	    formValues->add(fullName->copy(),
			    ((ZxCharData *)child2Node)->getData()->copy());
	  }
	}
      }
    } else if (node->isElement()) {
      ZxAttr *nameAttr = ((ZxElement *)node)->findAttr("name");
      if (nameAttr && (node->isElement("subform") ||
		       node->isElement("field"))) {
	GString *nodeName = nameAttr->getValue();
	GString *childFullName;
	if (fullName) {
	  childFullName = GString::format("{0:t}.{1:t}", fullName, nodeName);
	} else {
	  childFullName = nodeName->copy();
	}
	int idx = fullNameIdx->lookupInt(nodeName);
	childFullName->appendf("[{0:d}]", idx);
	fullNameIdx->replace(nodeName, idx + 1);
	scanFormNode((ZxElement *)node, childFullName, formValues);
	delete childFullName;
      } else if (node->isElement("subform")) {
	scanFormNode((ZxElement *)node, fullName, formValues);
      }
    }
  }
  delete fullNameIdx;
}

void XFAScanner::scanNode(ZxElement *elem,
			  GString *parentName, GString *parentFullName,
			  GHash *nameIdx, GHash *fullNameIdx,
			  GString *exclGroupName, ZxElement *dataElem,
			  GHash *formValues) {
  GString *nodeName = getNodeName(elem);
  GHash *childNameIdx;
  if (!nameIdx || nodeName) {
    childNameIdx = new GHash();
  } else {
    childNameIdx = nameIdx;
  }
  GString *nodeFullName = getNodeFullName(elem);
  GHash *childFullNameIdx;
  if (!fullNameIdx || (nodeFullName && !elem->isElement("area"))) {
    childFullNameIdx = new GHash();
  } else {
    childFullNameIdx = fullNameIdx;
  }

  GString *childName;
  if (nodeName) {
    if (parentName) {
      childName = GString::format("{0:t}.{1:t}", parentName, nodeName);
    } else {
      childName = nodeName->copy();
    }
    int idx = nameIdx->lookupInt(nodeName);
    nameIdx->replace(nodeName, idx + 1);
    if (nodeIsBindGlobal(elem)) {
      childName->appendf("[0]");
    } else {
      childName->appendf("[{0:d}]", idx);
    }
  } else {
    childName = parentName;
  }
  GString *childFullName;
  if (nodeFullName) {
    if (parentFullName) {
      childFullName = GString::format("{0:t}.{1:t}",
				      parentFullName, nodeFullName);
    } else {
      childFullName = nodeFullName->copy();
    }
    int idx = fullNameIdx->lookupInt(nodeFullName);
    fullNameIdx->replace(nodeFullName, idx + 1);
    childFullName->appendf("[{0:d}]", idx);
  } else {
    childFullName = parentFullName;
  }

  if (elem->isElement("field")) {
    if (childName && childFullName) {
      scanField(elem, childName, childFullName, exclGroupName,
		dataElem, formValues);
    }
  } else {
    GString *childExclGroupName;
    if (elem->isElement("exclGroup")) {
      childExclGroupName = childName;
    } else {
      childExclGroupName = NULL;
    }
    for (ZxNode *child = elem->getFirstChild();
	 child;
	 child = child->getNextChild()) {
      if (child->isElement()) {
	scanNode((ZxElement *)child, childName, childFullName,
		 childNameIdx, childFullNameIdx, childExclGroupName,
		 dataElem, formValues);
      }
    }
  }

  if (childName != parentName) {
    delete childName;
  }
  if (childFullName != parentFullName) {
    delete childFullName;
  }
  if (childNameIdx != nameIdx) {
    delete childNameIdx;
  }
  if (childFullNameIdx != fullNameIdx) {
    delete childFullNameIdx;
  }
}

void XFAScanner::scanField(ZxElement *elem, GString *name, GString *fullName,
			   GString *exclGroupName, ZxElement *dataElem,
			   GHash *formValues) {
  GString *value = getFieldValue(elem, name, fullName, exclGroupName,
				 dataElem, formValues);
  XFAFieldLayoutInfo *layoutInfo = getFieldLayoutInfo(elem);
  XFAFieldPictureInfo *pictureInfo = getFieldPictureInfo(elem);
  XFAFieldBarcodeInfo *barcodeInfo = getFieldBarcodeInfo(elem);
  XFAField *field = new XFAField(name->copy(), fullName->copy(), value,
				 layoutInfo, pictureInfo, barcodeInfo);
  fields->add(field->fullName, field);
}

GString *XFAScanner::getFieldValue(ZxElement *elem, GString *name,
				   GString *fullName, GString *exclGroupName,
				   ZxElement *dataElem, GHash *formValues) {
  GString *val = NULL;

  //--- check the <xfa:datasets> packet
  val = getDatasetsValue(name->getCString(), dataElem);
  if (!val && exclGroupName) {
    val = (GString *)getDatasetsValue(exclGroupName->getCString(), dataElem);
  }

  //--- check the <form> element
  if (!val) {
    val = (GString *)formValues->lookup(fullName);
  }

  //--- check the <value> element within the field
  if (!val) {
    ZxElement *valueElem = elem->findFirstChildElement("value");
    if (valueElem) {
      ZxNode *child1Node = valueElem->getFirstChild();
      if (child1Node && child1Node->isElement()) {
	ZxNode *child2Node = ((ZxElement *)child1Node)->getFirstChild();
	if (child2Node && child2Node->isCharData()) {
	  val = ((ZxCharData *)child2Node)->getData();
	}
      }
    }
  }

  //--- get the checkbutton item value
  GString *checkbuttonItem = NULL;
  ZxElement *uiElem = elem->findFirstChildElement("ui");
  if (uiElem) {
    ZxNode *uiChild = uiElem->getFirstChild();
    if (uiChild && uiChild->isElement("checkButton")) {
      ZxElement *itemsElem = elem->findFirstChildElement("items");
      if (itemsElem) {
	ZxNode *node1 = itemsElem->getFirstChild();
	if (node1 && node1->isElement()) {
	  ZxNode *node2 = ((ZxElement *)node1)->getFirstChild();
	  if (node2 && node2->isCharData()) {
	    checkbuttonItem = ((ZxCharData *)node2)->getData();
	  }
	}
      }
    }
  }
  // convert XFA checkbutton value to AcroForm-style On/Off value
  if (checkbuttonItem && val) {
    if (val->cmp(checkbuttonItem)) {
      val = new GString("Off");
    } else {
      val = new GString("On");
    }
  } else if (val) {
    val = val->copy();
  }

  return val;
}

GString *XFAScanner::getDatasetsValue(char *partName, ZxElement *elem) {
  if (!elem) {
    return NULL;
  }

  // partName = xxxx[nn].yyyy----
  char *p = strchr(partName, '[');
  if (!p) {
    return NULL;
  }
  int partLen = (int)(p - partName);
  int idx = atoi(p + 1);
  p = strchr(p + 1, '.');
  if (p) {
    ++p;
  }

  int curIdx = 0;
  for (ZxNode *node = elem->getFirstChild();
       node;
       node = node->getNextChild()) {
    if (!node->isElement()) {
      continue;
    }
    GString *nodeName = ((ZxElement *)node)->getType();
    if (nodeName->getLength() != partLen ||
	strncmp(nodeName->getCString(), partName, partLen)) {
      continue;
    }
    if (curIdx != idx) {
      ++curIdx;
      continue;
    }
    if (p) {
      GString *val = getDatasetsValue(p, (ZxElement *)node);
      if (val) {
	return val;
      }
      break;
    } else {
      ZxNode *child = ((ZxElement *)node)->getFirstChild();
      if (!child || !child->isCharData()) {
	return NULL;
      }
      return ((ZxCharData *)child)->getData();
    }
  }

  // search for an 'ancestor match'
  if (p) {
    return getDatasetsValue(p, elem);
  }

  return NULL;
}

XFAFieldLayoutInfo *XFAScanner::getFieldLayoutInfo(ZxElement *elem) {
  ZxElement *paraElem = elem->findFirstChildElement("para");
  if (!paraElem) {
    return NULL;
  }
  XFAFieldLayoutHAlign hAlign = xfaFieldLayoutHAlignLeft;
  ZxAttr *hAlignAttr = paraElem->findAttr("hAlign");
  if (hAlignAttr) {
    if (!hAlignAttr->getValue()->cmp("left")) {
      hAlign = xfaFieldLayoutHAlignLeft;
    } else if (!hAlignAttr->getValue()->cmp("center")) {
      hAlign = xfaFieldLayoutHAlignCenter;
    } else if (!hAlignAttr->getValue()->cmp("right")) {
      hAlign = xfaFieldLayoutHAlignRight;
    }
  }
  XFAFieldLayoutVAlign vAlign = xfaFieldLayoutVAlignTop;
  ZxAttr *vAlignAttr = paraElem->findAttr("vAlign");
  if (vAlignAttr) {
    if (!vAlignAttr->getValue()->cmp("top")) {
      vAlign = xfaFieldLayoutVAlignTop;
    } else if (!vAlignAttr->getValue()->cmp("middle")) {
      vAlign = xfaFieldLayoutVAlignMiddle;
    } else if (!vAlignAttr->getValue()->cmp("bottom")) {
      vAlign = xfaFieldLayoutVAlignBottom;
    }
  }
  return new XFAFieldLayoutInfo(hAlign, vAlign);
}

XFAFieldPictureInfo *XFAScanner::getFieldPictureInfo(ZxElement *elem) {
  ZxElement *uiElem = elem->findFirstChildElement("ui");
  if (!uiElem) {
    return NULL;
  }
  XFAFieldPictureSubtype subtype;
  if (uiElem->findFirstChildElement("dateTimeEdit")) {
    subtype = xfaFieldPictureDateTime;
  } else if (uiElem->findFirstChildElement("numericEdit")) {
    subtype = xfaFieldPictureNumeric;
  } else if (uiElem->findFirstChildElement("textEdit")) {
    subtype = xfaFieldPictureText;
  } else {
    return NULL;
  }

  ZxElement *formatElem, *pictureElem;
  ZxNode *pictureChildNode;
  if (!(formatElem = elem->findFirstChildElement("format")) ||
      !(pictureElem = formatElem->findFirstChildElement("picture")) ||
      !(pictureChildNode = pictureElem->getFirstChild()) ||
      !pictureChildNode->isCharData()) {
    return NULL;
  }
  GString *format = ((ZxCharData *)pictureChildNode)->getData()->copy();

  return new XFAFieldPictureInfo(subtype, format);
}

XFAFieldBarcodeInfo *XFAScanner::getFieldBarcodeInfo(ZxElement *elem) {
  ZxElement *uiElem, *barcodeElem;
  if (!(uiElem = elem->findFirstChildElement("ui")) ||
      !(barcodeElem = uiElem->findFirstChildElement("barcode"))) {
    return NULL;
  }

  ZxAttr *attr;
  if (!(attr = barcodeElem->findAttr("type"))) {
    return NULL;
  }
  GString *barcodeType = attr->getValue()->copy();

  double wideNarrowRatio = 3;
  if ((attr = barcodeElem->findAttr("wideNarrowRatio"))) {
    char *s = attr->getValue()->getCString();
    char *colon = strchr(s, ':');
    if (colon) {
      GString *numStr = new GString(s, (int)(colon - s));
      double num = atof(numStr->getCString());
      delete numStr;
      double den = atof(colon + 1);
      if (den == 0) {
	wideNarrowRatio = num;
      } else {
	wideNarrowRatio = num / den;
      }
    } else {
      wideNarrowRatio = atof(s);
    }
  }

  double moduleWidth = (0.25 / 25.4) * 72.0; // 0.25mm
  if ((attr = barcodeElem->findAttr("moduleWidth"))) {
    moduleWidth = getMeasurement(attr->getValue());
  }

  double moduleHeight = (5.0 / 25.4) * 72.0; // 5mm
  if ((attr = barcodeElem->findAttr("moduleHeight"))) {
    moduleHeight = getMeasurement(attr->getValue());
  }

  int dataLength = 0;
  if ((attr = barcodeElem->findAttr("dataLength"))) {
    dataLength = atoi(attr->getValue()->getCString());
  }

  int errorCorrectionLevel = 0;
  if ((attr = barcodeElem->findAttr("errorCorrectionLevel"))) {
    errorCorrectionLevel = atoi(attr->getValue()->getCString());
  }

  GString *textLocation;
  if ((attr = barcodeElem->findAttr("textLocation"))) {
    textLocation = attr->getValue()->copy();
  } else {
    textLocation = new GString("below");
  }

  return new XFAFieldBarcodeInfo(barcodeType, wideNarrowRatio,
				 moduleWidth, moduleHeight, dataLength,
				 errorCorrectionLevel, textLocation);
}

double XFAScanner::getMeasurement(GString *s) {
  int i = 0;
  GBool neg = gFalse;
  if (i < s->getLength() && s->getChar(i) == '+') {
    ++i;
  } else if (i < s->getLength() && s->getChar(i) == '-') {
    neg = gTrue;
    ++i;
  }
  double val = 0;
  while (i < s->getLength() && s->getChar(i) >= '0' && s->getChar(i) <= '9') {
    val = val * 10 + s->getChar(i) - '0';
    ++i;
  }
  if (i < s->getLength() && s->getChar(i) == '.') {
    ++i;
    double mul = 0.1;
    while (i < s->getLength() &&
	   s->getChar(i) >= '0' && s->getChar(i) <= '9') {
      val += mul * (s->getChar(i) - '0');
      mul *= 0.1;
      ++i;
    }
  }
  if (neg) {
    val = -val;
  }
  if (i+1 < s->getLength()) {
    if (s->getChar(i) == 'i' && s->getChar(i+1) == 'n') {
      val *= 72;
    } else if (s->getChar(i) == 'p' && s->getChar(i+1) == 't') {
      // no change
    } else if (s->getChar(i) == 'c' && s->getChar(i+1) == 'm') {
      val *= 72 / 2.54;
    } else if (s->getChar(i) == 'm' && s->getChar(i+1) == 'm') {
      val *= 72 / 25.4;
    } else {
      // default to inches
      val *= 72;
    }
  } else {
    // default to inches
    val *= 72;
  }
  return val;
}

GString *XFAScanner::getNodeName(ZxElement *elem) {
  if (elem->isElement("template") ||
      elem->isElement("area") ||
      elem->isElement("draw")) {
    return NULL;
  }
  if (!elem->isElement("field") && nodeIsBindNone(elem)) {
    return NULL;
  }
  ZxAttr *nameAttr = elem->findAttr("name");
  if (!nameAttr) {
    return NULL;
  }
  return nameAttr->getValue();
}

GString *XFAScanner::getNodeFullName(ZxElement *elem) {
  if (elem->isElement("template") ||
      elem->isElement("draw")) {
    return NULL;
  }
  ZxAttr *nameAttr = elem->findAttr("name");
  if (!nameAttr) {
    return NULL;
  }
  return nameAttr->getValue();
}

GBool XFAScanner::nodeIsBindGlobal(ZxElement *elem) {
  ZxElement *bindElem = elem->findFirstChildElement("bind");
  if (!bindElem) {
    return gFalse;
  }
  ZxAttr *attr = bindElem->findAttr("match");
  return attr && !attr->getValue()->cmp("global");
}

GBool XFAScanner::nodeIsBindNone(ZxElement *elem) {
  ZxElement *bindElem = elem->findFirstChildElement("bind");
  if (!bindElem) {
    return gFalse;
  }
  ZxAttr *attr = bindElem->findAttr("match");
  return attr && !attr->getValue()->cmp("none");
}
