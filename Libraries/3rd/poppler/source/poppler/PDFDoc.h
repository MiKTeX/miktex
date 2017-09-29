//========================================================================
//
// PDFDoc.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2006, 2008 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2005, 2009, 2014, 2015, 2017 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2008 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Eric Toombs <ewtoombs@uwaterloo.ca>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2010, 2014 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Srinivas Adicherla <srinivas.adicherla@geodesic.com>
// Copyright (C) 2011, 2013, 2014, 2016 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Adam Reichold <adamreichold@myopera.com>
// Copyright (C) 2013 Adrian Perez de Castro <aperez@igalia.com>
// Copyright (C) 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright (C) 2015 André Esser <bepandre@hotmail.com>
// Copyright (C) 2016 Jakub Alba <jakubalba@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef PDFDOC_H
#define PDFDOC_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "poppler-config.h"
#include <stdio.h>
#include "goo/GooMutex.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "Annot.h"
#include "Form.h"
#include "OptionalContent.h"
#include "Stream.h"

class GooString;
class GooFile;
class BaseStream;
class OutputDev;
class Links;
class LinkAction;
class LinkDest;
class Outline;
class Linearization;
class SecurityHandler;
class Hints;
class StructTreeRoot;

enum PDFWriteMode {
  writeStandard,
  writeForceRewrite,
  writeForceIncremental
};

//------------------------------------------------------------------------
// PDFDoc
//------------------------------------------------------------------------

class PDFDoc {
public:

  PDFDoc(GooString *fileNameA, GooString *ownerPassword = NULL,
	 GooString *userPassword = NULL, void *guiDataA = NULL);

#ifdef _WIN32
  PDFDoc(wchar_t *fileNameA, int fileNameLen, GooString *ownerPassword = NULL,
	 GooString *userPassword = NULL, void *guiDataA = NULL);
#endif

  PDFDoc(BaseStream *strA, GooString *ownerPassword = NULL,
	 GooString *userPassword = NULL, void *guiDataA = NULL);
  ~PDFDoc();

  static PDFDoc *ErrorPDFDoc(int errorCode, GooString *fileNameA = NULL);

  // Was PDF document successfully opened?
  GBool isOk() { return ok; }

  // Get the error code (if isOk() returns false).
  int getErrorCode() { return errCode; }

  // Get the error code returned by fopen() (if getErrorCode() == 
  // errOpenFile).
  int getFopenErrno() { return fopenErrno; }

  // Get file name.
  GooString *getFileName() { return fileName; }
#if !defined(MIKTEX)
#ifdef _WIN32
  wchar_t *getFileNameU() { return fileNameU; }
#endif
#endif

  // Get the linearization table.
  Linearization *getLinearization();
  GBool checkLinearization();

  // Get the xref table.
  XRef *getXRef() { return xref; }

  // Get catalog.
  Catalog *getCatalog() { return catalog; }

  // Get optional content configuration
  OCGs *getOptContentConfig() { return catalog->getOptContentConfig(); }

  // Get base stream.
  BaseStream *getBaseStream() { return str; }

  // Get page parameters.
  double getPageMediaWidth(int page)
    { return getPage(page) ? getPage(page)->getMediaWidth() : 0.0 ; }
  double getPageMediaHeight(int page)
    { return getPage(page) ? getPage(page)->getMediaHeight() : 0.0 ; }
  double getPageCropWidth(int page)
    { return getPage(page) ? getPage(page)->getCropWidth() : 0.0 ; }
  double getPageCropHeight(int page)
    { return getPage(page) ? getPage(page)->getCropHeight() : 0.0 ; }
  int getPageRotate(int page)
    { return getPage(page) ? getPage(page)->getRotate() : 0 ; }

  // Get number of pages.
  int getNumPages();

  // Return the contents of the metadata stream, or NULL if there is
  // no metadata.
  GooString *readMetadata() { return catalog->readMetadata(); }

  // Return the structure tree root object.
  StructTreeRoot *getStructTreeRoot() { return catalog->getStructTreeRoot(); }

  // Get page.
  Page *getPage(int page);

  // Display a page.
  void displayPage(OutputDev *out, int page,
		   double hDPI, double vDPI, int rotate,
		   GBool useMediaBox, GBool crop, GBool printing,
		   GBool (*abortCheckCbk)(void *data) = NULL,
		   void *abortCheckCbkData = NULL,
                   GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
                   void *annotDisplayDecideCbkData = NULL, GBool copyXRef = gFalse);

  // Display a range of pages.
  void displayPages(OutputDev *out, int firstPage, int lastPage,
		    double hDPI, double vDPI, int rotate,
		    GBool useMediaBox, GBool crop, GBool printing,
		    GBool (*abortCheckCbk)(void *data) = NULL,
		    void *abortCheckCbkData = NULL,
                    GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
                    void *annotDisplayDecideCbkData = NULL);

  // Display part of a page.
  void displayPageSlice(OutputDev *out, int page,
			double hDPI, double vDPI, int rotate, 
			GBool useMediaBox, GBool crop, GBool printing,
			int sliceX, int sliceY, int sliceW, int sliceH,
			GBool (*abortCheckCbk)(void *data) = NULL,
			void *abortCheckCbkData = NULL,
                        GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
                        void *annotDisplayDecideCbkData = NULL, GBool copyXRef = gFalse);

  // Find a page, given its object ID.  Returns page number, or 0 if
  // not found.
  int findPage(int num, int gen) { return catalog->findPage(num, gen); }

  // Returns the links for the current page, transferring ownership to
  // the caller.
  Links *getLinks(int page);

  // Find a named destination.  Returns the link destination, or
  // NULL if <name> is not a destination.
  LinkDest *findDest(GooString *name)
    { return catalog->findDest(name); }

  // Process the links for a page.
  void processLinks(OutputDev *out, int page);


#ifndef DISABLE_OUTLINE
  // Return the outline object.
  Outline *getOutline();
#endif

  // Is the file encrypted?
  GBool isEncrypted() { return xref->isEncrypted(); }

  std::vector<FormWidgetSignature*> getSignatureWidgets();

  // Check various permissions.
  GBool okToPrint(GBool ignoreOwnerPW = gFalse)
    { return xref->okToPrint(ignoreOwnerPW); }
  GBool okToPrintHighRes(GBool ignoreOwnerPW = gFalse)
    { return xref->okToPrintHighRes(ignoreOwnerPW); }
  GBool okToChange(GBool ignoreOwnerPW = gFalse)
    { return xref->okToChange(ignoreOwnerPW); }
  GBool okToCopy(GBool ignoreOwnerPW = gFalse)
    { return xref->okToCopy(ignoreOwnerPW); }
  GBool okToAddNotes(GBool ignoreOwnerPW = gFalse)
    { return xref->okToAddNotes(ignoreOwnerPW); }
  GBool okToFillForm(GBool ignoreOwnerPW = gFalse)
    { return xref->okToFillForm(ignoreOwnerPW); }
  GBool okToAccessibility(GBool ignoreOwnerPW = gFalse)
    { return xref->okToAccessibility(ignoreOwnerPW); }
  GBool okToAssemble(GBool ignoreOwnerPW = gFalse)
    { return xref->okToAssemble(ignoreOwnerPW); }


  // Is this document linearized?
  GBool isLinearized(GBool tryingToReconstruct = gFalse);

  // Return the document's Info dictionary (if any).
  Object getDocInfo() { return xref->getDocInfo(); }
  Object getDocInfoNF() { return xref->getDocInfoNF(); }

  // Create and return the document's Info dictionary if none exists.
  // Otherwise return the existing one.
  Object createDocInfoIfNoneExists() { return xref->createDocInfoIfNoneExists(); }

  // Remove the document's Info dictionary and update the trailer dictionary.
  void removeDocInfo() { xref->removeDocInfo(); }

  // Set doc info string entry. NULL or empty value will cause a removal.
  // Takes ownership of value.
  void setDocInfoStringEntry(const char *key, GooString *value);

  // Set document's properties in document's Info dictionary.
  // NULL or empty value will cause a removal.
  // Takes ownership of value.
  void setDocInfoTitle(GooString *title) { setDocInfoStringEntry("Title", title); }
  void setDocInfoAuthor(GooString *author) { setDocInfoStringEntry("Author", author); }
  void setDocInfoSubject(GooString *subject) { setDocInfoStringEntry("Subject", subject); }
  void setDocInfoKeywords(GooString *keywords) { setDocInfoStringEntry("Keywords", keywords); }
  void setDocInfoCreator(GooString *creator) { setDocInfoStringEntry("Creator", creator); }
  void setDocInfoProducer(GooString *producer) { setDocInfoStringEntry("Producer", producer); }
  void setDocInfoCreatDate(GooString *creatDate) { setDocInfoStringEntry("CreationDate", creatDate); }
  void setDocInfoModDate(GooString *modDate) { setDocInfoStringEntry("ModDate", modDate); }

  // Get document's properties from document's Info dictionary.
  // Returns NULL on fail.
  // Returned GooStrings should be freed by the caller.
  GooString *getDocInfoStringEntry(const char *key);

  GooString *getDocInfoTitle() { return getDocInfoStringEntry("Title"); }
  GooString *getDocInfoAuthor() { return getDocInfoStringEntry("Author"); }
  GooString *getDocInfoSubject() { return getDocInfoStringEntry("Subject"); }
  GooString *getDocInfoKeywords() { return getDocInfoStringEntry("Keywords"); }
  GooString *getDocInfoCreator() { return getDocInfoStringEntry("Creator"); }
  GooString *getDocInfoProducer() { return getDocInfoStringEntry("Producer"); }
  GooString *getDocInfoCreatDate() { return getDocInfoStringEntry("CreationDate"); }
  GooString *getDocInfoModDate() { return getDocInfoStringEntry("ModDate"); }

  // Return the PDF version specified by the file.
  int getPDFMajorVersion() { return pdfMajorVersion; }
  int getPDFMinorVersion() { return pdfMinorVersion; }

  //Return the PDF ID in the trailer dictionary (if any).
  GBool getID(GooString *permanent_id, GooString *update_id);

  // Save one page with another name.
  int savePageAs(GooString *name, int pageNo);
  // Save this file with another name.
  int saveAs(GooString *name, PDFWriteMode mode=writeStandard);
  // Save this file in the given output stream.
  int saveAs(OutStream *outStr, PDFWriteMode mode=writeStandard);
  // Save this file with another name without saving changes
  int saveWithoutChangesAs(GooString *name);
  // Save this file in the given output stream without saving changes
  int saveWithoutChangesAs(OutStream *outStr);

  // Return a pointer to the GUI (XPDFCore or WinPDFCore object).
  void *getGUIData() { return guiData; }

  // rewrite pageDict with MediaBox, CropBox and new page CTM
  void replacePageDict(int pageNo, int rotate, PDFRectangle *mediaBox, PDFRectangle *cropBox);
  void markPageObjects(Dict *pageDict, XRef *xRef, XRef *countRef, Guint numOffset, int oldRefNum, int newRefNum, std::set<Dict*> *alreadyMarkedDicts = nullptr);
  GBool markAnnotations(Object *annots, XRef *xRef, XRef *countRef, Guint numOffset, int oldPageNum, int newPageNum, std::set<Dict*> *alreadyMarkedDicts = nullptr);
  void markAcroForm(Object *acrpForm, XRef *xRef, XRef *countRef, Guint numOffset, int oldPageNum, int newPageNum);
  // write all objects used by pageDict to outStr
  Guint writePageObjects(OutStream *outStr, XRef *xRef, Guint numOffset, GBool combine = gFalse);
  static void writeObject (Object *obj, OutStream* outStr, XRef *xref, Guint numOffset, Guchar *fileKey,
                           CryptAlgorithm encAlgorithm, int keyLength, int objNum, int objGen, std::set<Dict*> *alreadyWrittenDicts = nullptr);
  static void writeHeader(OutStream *outStr, int major, int minor);

  static Object createTrailerDict (int uxrefSize, GBool incrUpdate, Goffset startxRef,
                                  Ref *root, XRef *xRef, const char *fileName, Goffset fileSize);
  static void writeXRefTableTrailer (Object &&trailerDict, XRef *uxref, GBool writeAllEntries,
                                     Goffset uxrefOffset, OutStream* outStr, XRef *xRef);
  static void writeXRefStreamTrailer (Object &&trailerDict, XRef *uxref, Ref *uxrefStreamRef,
                                      Goffset uxrefOffset, OutStream* outStr, XRef *xRef);

private:
  // insert referenced objects in XRef
  void markDictionnary (Dict* dict, XRef *xRef, XRef *countRef, Guint numOffset, int oldRefNum, int newRefNum, std::set<Dict*> *alreadyMarkedDicts);
  void markObject (Object *obj, XRef *xRef, XRef *countRef, Guint numOffset, int oldRefNum, int newRefNum, std::set<Dict*> *alreadyMarkedDicts = nullptr);
  static void writeDictionnary (Dict* dict, OutStream* outStr, XRef *xRef, Guint numOffset, Guchar *fileKey,
                                CryptAlgorithm encAlgorithm, int keyLength, int objNum, int objGen, std::set<Dict*> *alreadyWrittenDicts);

  // Write object header to current file stream and return its offset
  static Goffset writeObjectHeader (Ref *ref, OutStream* outStr);
  static void writeObjectFooter (OutStream* outStr);

  void writeObject (Object *obj, OutStream* outStr, Guchar *fileKey, CryptAlgorithm encAlgorithm,
                    int keyLength, int objNum, int objGen, std::set<Dict*> *alreadyWrittenDicts = nullptr)
  { writeObject(obj, outStr, getXRef(), 0, fileKey, encAlgorithm, keyLength, objNum, objGen, alreadyWrittenDicts); }
  static void writeStream (Stream* str, OutStream* outStr);
  static void writeRawStream (Stream* str, OutStream* outStr);
  void writeXRefTableTrailer (Goffset uxrefOffset, XRef *uxref, GBool writeAllEntries,
                              int uxrefSize, OutStream* outStr, GBool incrUpdate);
  static void writeString (GooString* s, OutStream* outStr, Guchar *fileKey,
                           CryptAlgorithm encAlgorithm, int keyLength, int objNum, int objGen);
  void saveIncrementalUpdate (OutStream* outStr);
  void saveCompleteRewrite (OutStream* outStr);

  Page *parsePage(int page);

  // Get hints.
  Hints *getHints();

  PDFDoc();
  void init();
  GBool setup(GooString *ownerPassword, GooString *userPassword);
  GBool checkFooter();
  void checkHeader();
  GBool checkEncryption(GooString *ownerPassword, GooString *userPassword);
  // Get the offset of the start xref table.
  Goffset getStartXRef(GBool tryingToReconstruct = gFalse);
  // Get the offset of the entries in the main XRef table of a
  // linearized document (0 for non linearized documents).
  Goffset getMainXRefEntriesOffset(GBool tryingToReconstruct = gFalse);
  long long strToLongLong(char *s);

  // Mark the document's Info dictionary as modified.
  void setDocInfoModified(Object *infoObj);

  GooString *fileName;
#if !defined(MIKTEX)
#ifdef _WIN32
  wchar_t *fileNameU;
#endif
#endif
  GooFile *file;
  BaseStream *str;
  void *guiData;
  int pdfMajorVersion;
  int pdfMinorVersion;
  Linearization *linearization;
  // linearizationState = 0: unchecked
  // linearizationState = 1: checked and valid
  // linearizationState = 2: checked and invalid
  int linearizationState;
  XRef *xref;
  SecurityHandler *secHdlr;
  Catalog *catalog;
  Hints *hints;
#ifndef DISABLE_OUTLINE
  Outline *outline;
#endif
  Page **pageCache;

  GBool ok;
  int errCode;
  //If there is an error opening the PDF file with fopen() in the constructor, 
  //then the POSIX errno will be here.
  int fopenErrno;

  Goffset startXRefPos;		// offset of last xref table
#if MULTITHREADED
  GooMutex mutex;
#endif
};

#endif
