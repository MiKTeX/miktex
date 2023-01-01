//========================================================================
//
// XFAScanner.h
//
// Copyright 2020 Glyph & Cog, LLC
//
//========================================================================

#ifndef XFASCANNER_H
#define XFASCANNER_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

class GHash;
class ZxElement;

//------------------------------------------------------------------------

enum XFAFieldLayoutHAlign {
  xfaFieldLayoutHAlignLeft,
  xfaFieldLayoutHAlignCenter,
  xfaFieldLayoutHAlignRight
};

enum XFAFieldLayoutVAlign {
  xfaFieldLayoutVAlignTop,
  xfaFieldLayoutVAlignMiddle,
  xfaFieldLayoutVAlignBottom
};

class XFAFieldLayoutInfo {
public:

  XFAFieldLayoutInfo(XFAFieldLayoutHAlign hAlignA,
		     XFAFieldLayoutVAlign vAlignA);

  XFAFieldLayoutHAlign hAlign;
  XFAFieldLayoutVAlign vAlign;
};

//------------------------------------------------------------------------

enum XFAFieldPictureSubtype {
  xfaFieldPictureDateTime,
  xfaFieldPictureNumeric,
  xfaFieldPictureText
};

class XFAFieldPictureInfo {
public:

  XFAFieldPictureInfo(XFAFieldPictureSubtype subtypeA, GString *formatA);
  ~XFAFieldPictureInfo();

  XFAFieldPictureSubtype subtype;
  GString *format;		// picture format string
};

//------------------------------------------------------------------------

class XFAFieldBarcodeInfo {
public:

  XFAFieldBarcodeInfo(GString *barcodeTypeA, double wideNarrowRatioA,
		      double moduleWidthA, double moduleHeightA,
		      int dataLengthA, int errorCorrectionLevelA,
		      GString *textLocationA);
  ~XFAFieldBarcodeInfo();

  GString *barcodeType;
  double wideNarrowRatio;
  double moduleWidth;
  double moduleHeight;
  int dataLength;
  int errorCorrectionLevel;
  GString *textLocation;
};

//------------------------------------------------------------------------

class XFAField {
public:

  XFAField(GString *nameA, GString *fullNameA, GString *valueA,
	   XFAFieldLayoutInfo *layoutInfoA,
	   XFAFieldPictureInfo *pictureInfoA,
	   XFAFieldBarcodeInfo *barcodeInfoA);
  ~XFAField();

  // Get the field's value, or NULL if it doesn't have a value.  Sets
  // *[length] to the length of the Unicode string.
  GString *getValue() { return value; }

  // Return a pointer to the field's picture formatting info object,
  // or NULL if the field doesn't have picture formatting.
  XFAFieldPictureInfo *getPictureInfo() { return pictureInfo; }

  // Return a pointer to the field's layout info object, or NULL if
  // the field doesn't have layout info.
  XFAFieldLayoutInfo *getLayoutInfo() { return layoutInfo; }

  // Return a pointer to the field's barcode info object, or NULL if
  // the field isn't a barcode.
  XFAFieldBarcodeInfo *getBarcodeInfo() { return barcodeInfo; }

private:

  friend class XFAScanner;

  GString *name;		// UTF-8
  GString *fullName;		// UTF-8
  GString *value;		// UTF-8
  XFAFieldLayoutInfo *layoutInfo;
  XFAFieldPictureInfo *pictureInfo;
  XFAFieldBarcodeInfo *barcodeInfo;
};

//------------------------------------------------------------------------

class XFAScanner {
public:

  static XFAScanner *load(Object *xfaObj);

  virtual ~XFAScanner();

  // Find an XFA field matchined the specified AcroForm field name.
  // Returns NULL if there is no matching field.
  XFAField *findField(GString *acroFormFieldName);

private:

  XFAScanner();
  static GString *readXFAStreams(Object *xfaObj);
  GHash *scanFormValues(ZxElement *xmlRoot);
  void scanFormNode(ZxElement *elem, GString *fullName,
		    GHash *formValues);
  void scanNode(ZxElement *elem,
		GString *parentName, GString *parentFullName,
		GHash *nameIdx, GHash *fullNameIdx,
		GString *exclGroupName, ZxElement *xmlRoot,
		GHash *formValues);
  void scanField(ZxElement *elem, GString *name, GString *fullName,
		 GString *exclGroupName, ZxElement *xmlRoot,
		 GHash *formValues);
  GString *getFieldValue(ZxElement *elem, GString *name,
			 GString *fullName, GString *exclGroupName,
			 ZxElement *xmlRoot, GHash *formValues);
  GString *getDatasetsValue(char *partName, ZxElement *elem);
  XFAFieldLayoutInfo *getFieldLayoutInfo(ZxElement *elem);
  XFAFieldPictureInfo *getFieldPictureInfo(ZxElement *elem);
  XFAFieldBarcodeInfo *getFieldBarcodeInfo(ZxElement *elem);
  double getMeasurement(GString *s);
  GString *getNodeName(ZxElement *elem);
  GString *getNodeFullName(ZxElement *elem);
  GBool nodeIsBindGlobal(ZxElement *elem);
  GBool nodeIsBindNone(ZxElement *elem);

  GHash *fields;		// [XFAField]
};

#endif
