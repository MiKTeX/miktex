//========================================================================
//
// AcroForm.h
//
// Copyright 2012 Glyph & Cog, LLC
//
//========================================================================

#ifndef ACROFORM_H
#define ACROFORM_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

class TextString;
class Gfx;
class GfxFont;
class GfxFontDict;
class AcroFormField;
class XFAScanner;
class XFAField;
class XFAFieldBarcodeInfo;

//------------------------------------------------------------------------

class AcroForm {
public:

  static AcroForm *load(PDFDoc *docA, Catalog *catalog, Object *acroFormObjA);

  ~AcroForm();

  const char *getType();

  void draw(int pageNum, Gfx *gfx, GBool printing);

  int getNumFields();
  AcroFormField *getField(int idx);
  AcroFormField *findField(int pg, double x, double y);
  int findFieldIdx(int pg, double x, double y);

private:

  AcroForm(PDFDoc *docA, Object *acroFormObjA);
  void buildAnnotPageList(Catalog *catalog);
  int lookupAnnotPage(Object *annotRef);
  void scanField(Object *fieldRef);

  PDFDoc *doc;
  Object acroFormObj;
  GBool needAppearances;
  GList *annotPages;		// [AcroFormAnnotPage]
  GList *fields;		// [AcroFormField]
  XFAScanner *xfaScanner;
  GBool isStaticXFA;

  friend class AcroFormField;
};

//------------------------------------------------------------------------

enum AcroFormFieldType {
  acroFormFieldPushbutton,
  acroFormFieldRadioButton,
  acroFormFieldCheckbox,
  acroFormFieldFileSelect,
  acroFormFieldMultilineText,
  acroFormFieldText,
  acroFormFieldBarcode,
  acroFormFieldComboBox,
  acroFormFieldListBox,
  acroFormFieldSignature
};

class AcroFormField {
public:

  static AcroFormField *load(AcroForm *acroFormA, Object *fieldRefA);

  ~AcroFormField();

  int getPageNum();
  const char *getType();
  Unicode *getName(int *length);
  Unicode *getValue(int *length);
  void getBBox(double *llx, double *lly, double *urx, double *ury);
  void getFont(Ref *fontID, double *fontSize);
  void getColor(double *red, double *green, double *blue);
  int getMaxLen();

  Object *getResources(Object *res);

  AcroFormFieldType getAcroFormFieldType() { return type; }
  Object *getFieldRef(Object *ref);
  Object *getValueObj(Object *val);
  Object *getParentRef(Object *parent);
  GBool getTypeFromParent() { return typeFromParent; }

private:

  AcroFormField(AcroForm *acroFormA, Object *fieldRefA, Object *fieldObjA,
		AcroFormFieldType typeA, TextString *nameA,
		Guint flagsA, GBool typeFromParentA, XFAField *xfaFieldA);
  Ref findFontName(char *fontTag);
  void draw(int pageNum, Gfx *gfx, GBool printing);
  void drawAnnot(int pageNum, Gfx *gfx, GBool printing,
		 Object *annotRef, Object *annotObj);
  void drawExistingAppearance(Gfx *gfx, Dict *annot,
			      double xMin, double yMin,
			      double xMax, double yMax);
  void drawNewAppearance(Gfx *gfx, Dict *annot,
			 double xMin, double yMin,
			 double xMax, double yMax);
  void setColor(Array *a, GBool fill, int adjust, GString *appearBuf);
  void drawText(GString *text, GString *da, GfxFontDict *fontDict,
		GBool multiline, int comb, int quadding, int vAlign,
		GBool txField, GBool forceZapfDingbats, int rot,
		double x, double y, double width, double height,
		double border, GBool whiteBackground, GString *appearBuf);
  void drawListBox(GString **text, GBool *selection,
		   int nOptions, int topIdx,
		   GString *da, GfxFontDict *fontDict,
		   GBool quadding, double xMin, double yMin,
		   double xMax, double yMax, double border,
		   GString *appearBuf);
  void getNextLine(GString *text, int start,
		   GfxFont *font, double fontSize, double wMax,
		   int *end, double *width, int *next);
  void drawCircle(double cx, double cy, double r, const char *cmd,
		  GString *appearBuf);
  void drawCircleTopLeft(double cx, double cy, double r,
			 GString *appearBuf);
  void drawCircleBottomRight(double cx, double cy, double r,
			     GString *appearBuf);
  void drawBarcode(GString *value, GString *da, GfxFontDict *fontDict, int rot,
		   double xMin, double yMin, double xMax, double yMax,
		   XFAFieldBarcodeInfo *barcodeInfo, GString *appearBuf);
  GList *tokenize(GString *s);
  Object *getAnnotObj(Object *annotObj);
  Object *getAnnotResources(Dict *annot, Object *res);
  void buildDefaultResourceDict(Object *dr);
  Object *fieldLookup(const char *key, Object *obj);
  Object *fieldLookup(Dict *dict, const char *key, Object *obj);
  Unicode *utf8ToUnicode(GString *s, int *unicodeLength);
  GString *unicodeToLatin1(Unicode *u, int unicodeLength);
  GBool unicodeStringEqual(Unicode *u, int unicodeLength, GString *s);
  GBool unicodeStringEqual(Unicode *u, int unicodeLength, const char *s);
  GString *pictureFormatDateTime(GString *value, GString *picture);
  GString *pictureFormatNumber(GString *value, GString *picture);
  GString *pictureFormatText(GString *value, GString *picture);
  GBool isValidInt(GString *s, int start, int len);
  int convertInt(GString *s, int start, int len);

  AcroForm *acroForm;
  Object fieldRef;
  Object fieldObj;
  AcroFormFieldType type;
  TextString *name;
  Guint flags;
  GBool typeFromParent;
  XFAField *xfaField;

  friend class AcroForm;
};

#endif
