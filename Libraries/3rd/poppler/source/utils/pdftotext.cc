//========================================================================
//
// pdftotext.cc
//
// Copyright 1997-2003 Glyph & Cog, LLC
//
// Modified for Debian by Hamish Moffatt, 22 May 2002.
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Dominic Lachowicz <cinamod@hotmail.com>
// Copyright (C) 2007-2008, 2010, 2011, 2017 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Jan Jockusch <jan@jockusch.de>
// Copyright (C) 2010, 2013 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Kenneth Berland <ken@hero.com>
// Copyright (C) 2011 Tom Gleason <tom@buildadam.com>
// Copyright (C) 2011 Steven Murdoch <Steven.Murdoch@cl.cam.ac.uk>
// Copyright (C) 2013 Yury G. Kudryashov <urkud.urkud@gmail.com>
// Copyright (C) 2013 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
// Copyright (C) 2015 Jeremy Echols <jechols@uoregon.edu>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "parseargs.h"
#include "printencodings.h"
#include "goo/GooString.h"
#include "goo/gmem.h"
#include "GlobalParams.h"
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "TextOutputDev.h"
#include "CharTypes.h"
#include "UnicodeMap.h"
#include "PDFDocEncoding.h"
#include "Error.h"
#include <string>
#include <sstream>
#include <iomanip>

static void printInfoString(FILE *f, Dict *infoDict, const char *key,
			    const char *text1, const char *text2, UnicodeMap *uMap);
static void printInfoDate(FILE *f, Dict *infoDict, const char *key, const char *fmt);
void printDocBBox(FILE *f, PDFDoc *doc, TextOutputDev *textOut, int first, int last);
void printWordBBox(FILE *f, PDFDoc *doc, TextOutputDev *textOut, int first, int last);

static int firstPage = 1;
static int lastPage = 0;
static double resolution = 72.0;
static int x = 0;
static int y = 0;
static int w = 0;
static int h = 0;
static GBool bbox = gFalse;
static GBool bboxLayout = gFalse;
static GBool physLayout = gFalse;
static double fixedPitch = 0;
static GBool rawOrder = gFalse;
static GBool htmlMeta = gFalse;
static char textEncName[128] = "";
static char textEOL[16] = "";
static GBool noPageBreaks = gFalse;
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static GBool quiet = gFalse;
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;
static GBool printEnc = gFalse;

static const ArgDesc argDesc[] = {
  {"-f",       argInt,      &firstPage,     0,
   "first page to convert"},
  {"-l",       argInt,      &lastPage,      0,
   "last page to convert"},
  {"-r",      argFP,       &resolution,    0,
   "resolution, in DPI (default is 72)"},
  {"-x",      argInt,      &x,             0,
   "x-coordinate of the crop area top left corner"},
  {"-y",      argInt,      &y,             0,
   "y-coordinate of the crop area top left corner"},
  {"-W",      argInt,      &w,             0,
   "width of crop area in pixels (default is 0)"},
  {"-H",      argInt,      &h,             0,
   "height of crop area in pixels (default is 0)"},
  {"-layout",  argFlag,     &physLayout,    0,
   "maintain original physical layout"},
  {"-fixed",   argFP,       &fixedPitch,    0,
   "assume fixed-pitch (or tabular) text"},
  {"-raw",     argFlag,     &rawOrder,      0,
   "keep strings in content stream order"},
  {"-htmlmeta", argFlag,   &htmlMeta,       0,
   "generate a simple HTML file, including the meta information"},
  {"-enc",     argString,   textEncName,    sizeof(textEncName),
   "output text encoding name"},
  {"-listenc",argFlag,     &printEnc,      0,
   "list available encodings"},
  {"-eol",     argString,   textEOL,        sizeof(textEOL),
   "output end-of-line convention (unix, dos, or mac)"},
  {"-nopgbrk", argFlag,     &noPageBreaks,  0,
   "don't insert page breaks between pages"},
  {"-bbox", argFlag,     &bbox,  0,
   "output bounding box for each word and page size to html.  Sets -htmlmeta"},
  {"-bbox-layout", argFlag,     &bboxLayout,  0,
   "like -bbox but with extra layout bounding box data.  Sets -htmlmeta"},
  {"-opw",     argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",     argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  {"-q",       argFlag,     &quiet,         0,
   "don't print any messages or errors"},
  {"-v",       argFlag,     &printVersion,  0,
   "print copyright and version info"},
  {"-h",       argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",    argFlag,     &printHelp,     0,
   "print usage information"},
  {"--help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"-?",       argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};

static std::string myStringReplace(const std::string &inString, const std::string &oldToken, const std::string &newToken) {
  std::string result = inString;
  size_t foundLoc;
  int advance = 0;
  do {
    foundLoc = result.find(oldToken, advance);
    if (foundLoc != std::string::npos){
      result.replace(foundLoc, oldToken.length(), newToken);
      advance = foundLoc + newToken.length();
    }
  } while (foundLoc != std::string::npos );
  return result;
}

static std::string myXmlTokenReplace(const char *inString){
  std::string myString(inString);
  myString = myStringReplace(myString, "&",  "&amp;" );
  myString = myStringReplace(myString, "'",  "&apos;" );
  myString = myStringReplace(myString, "\"", "&quot;" );
  myString = myStringReplace(myString, "<",  "&lt;" );
  myString = myStringReplace(myString, ">",  "&gt;" );
  return myString;
}

#if defined(MIKTEX)
int Main(int argc, char** argv) {
#else
int main(int argc, char *argv[]) {
#endif
  PDFDoc *doc;
  GooString *fileName;
  GooString *textFileName;
  GooString *ownerPW, *userPW;
  TextOutputDev *textOut;
  FILE *f;
  UnicodeMap *uMap;
  Object info;
  GBool ok;
  char *p;
  int exitCode;

  exitCode = 99;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (bboxLayout) {
    bbox = gTrue;
  }
  if (bbox) {
    htmlMeta = gTrue;
  }
  if (!ok || (argc < 2 && !printEnc) || argc > 3 || printVersion || printHelp) {
    fprintf(stderr, "pdftotext version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdftotext", "<PDF-file> [<text-file>]", argDesc);
    }
    if (printVersion || printHelp)
      exitCode = 0;
    goto err0;
  }

  // read config file
  globalParams = new GlobalParams();

  if (printEnc) {
    printEncodings();
    delete globalParams;
    exitCode = 0;
    goto err0;
  }

  fileName = new GooString(argv[1]);
  if (fixedPitch) {
    physLayout = gTrue;
  }

  if (textEncName[0]) {
    globalParams->setTextEncoding(textEncName);
  }
  if (textEOL[0]) {
    if (!globalParams->setTextEOL(textEOL)) {
      fprintf(stderr, "Bad '-eol' value on command line\n");
    }
  }
  if (noPageBreaks) {
    globalParams->setTextPageBreaks(gFalse);
  }
  if (quiet) {
    globalParams->setErrQuiet(quiet);
  }

  // get mapping to output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    error(errCommandLine, -1, "Couldn't get text encoding");
    delete fileName;
    goto err1;
  }

  // open PDF file
  if (ownerPassword[0] != '\001') {
    ownerPW = new GooString(ownerPassword);
  } else {
    ownerPW = NULL;
  }
  if (userPassword[0] != '\001') {
    userPW = new GooString(userPassword);
  } else {
    userPW = NULL;
  }

  if (fileName->cmp("-") == 0) {
      delete fileName;
      fileName = new GooString("fd://0");
  }

  doc = PDFDocFactory().createPDFDoc(*fileName, ownerPW, userPW);

  if (userPW) {
    delete userPW;
  }
  if (ownerPW) {
    delete ownerPW;
  }
  if (!doc->isOk()) {
    exitCode = 1;
    goto err2;
  }

#ifdef ENFORCE_PERMISSIONS
  // check for copy permission
  if (!doc->okToCopy()) {
    error(errNotAllowed, -1, "Copying of text from this document is not allowed.");
    exitCode = 3;
    goto err2;
  }
#endif

  // construct text file name
  if (argc == 3) {
    textFileName = new GooString(argv[2]);
  } else if (fileName->cmp("fd://0") == 0) {
     error(errCommandLine, -1, "You have to provide an output filename when reading form stdin.");
     goto err2;
  } else {
    p = fileName->getCString() + fileName->getLength() - 4;
    if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")) {
      textFileName = new GooString(fileName->getCString(),
				 fileName->getLength() - 4);
    } else {
      textFileName = fileName->copy();
    }
    textFileName->append(htmlMeta ? ".html" : ".txt");
  }

  // get page range
  if (firstPage < 1) {
    firstPage = 1;
  }
  if (lastPage < 1 || lastPage > doc->getNumPages()) {
    lastPage = doc->getNumPages();
  }
  if (lastPage < firstPage) {
    error(errCommandLine, -1,
          "Wrong page range given: the first page ({0:d}) can not be after the last page ({1:d}).",
          firstPage, lastPage);
    goto err3;
  }

  // write HTML header
  if (htmlMeta) {
    if (!textFileName->cmp("-")) {
      f = stdout;
    } else {
      if (!(f = fopen(textFileName->getCString(), "wb"))) {
	error(errIO, -1, "Couldn't open text file '{0:t}'", textFileName);
	exitCode = 2;
	goto err3;
      }
    }
    fputs("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">", f);
    fputs("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n", f);
    fputs("<head>\n", f);
    info = doc->getDocInfo();
    if (info.isDict()) {
      Object obj = info.getDict()->lookup("Title");
      if (obj.isString()) {
        printInfoString(f, info.getDict(), "Title", "<title>", "</title>\n", uMap);
      } else {
        fputs("<title></title>\n", f);
      }
      printInfoString(f, info.getDict(), "Subject",
		      "<meta name=\"Subject\" content=\"", "\"/>\n", uMap);
      printInfoString(f, info.getDict(), "Keywords",
		      "<meta name=\"Keywords\" content=\"", "\"/>\n", uMap);
      printInfoString(f, info.getDict(), "Author",
		      "<meta name=\"Author\" content=\"", "\"/>\n", uMap);
      printInfoString(f, info.getDict(), "Creator",
		      "<meta name=\"Creator\" content=\"", "\"/>\n", uMap);
      printInfoString(f, info.getDict(), "Producer",
		      "<meta name=\"Producer\" content=\"", "\"/>\n", uMap);
      printInfoDate(f, info.getDict(), "CreationDate",
		    "<meta name=\"CreationDate\" content=\"\"/>\n");
      printInfoDate(f, info.getDict(), "LastModifiedDate",
		    "<meta name=\"ModDate\" content=\"\"/>\n");
    }
    fputs("</head>\n", f);
    fputs("<body>\n", f);
    if (!bbox) {
      fputs("<pre>\n", f);
      if (f != stdout) {
	fclose(f);
      }
    }
  }

  // write text file
  if (htmlMeta && bbox) { // htmlMeta && is superfluous but makes gcc happier
    textOut = new TextOutputDev(NULL, physLayout, fixedPitch, rawOrder, htmlMeta);

    if (textOut->isOk()) {
      if (bboxLayout) {
        printDocBBox(f, doc, textOut, firstPage, lastPage);
      }
      else {
        printWordBBox(f, doc, textOut, firstPage, lastPage);
      }
    }
    if (f != stdout) {
      fclose(f);
    }
  } else {
    textOut = new TextOutputDev(textFileName->getCString(),
				physLayout, fixedPitch, rawOrder, htmlMeta);
    if (textOut->isOk()) {
      if ((w==0) && (h==0) && (x==0) && (y==0)) {
	doc->displayPages(textOut, firstPage, lastPage, resolution, resolution, 0,
			  gTrue, gFalse, gFalse);
      } else {
	
	for (int page = firstPage; page <= lastPage; ++page) {
	  doc->displayPageSlice(textOut, page, resolution, resolution, 0,
			      gTrue, gFalse, gFalse, 
			      x, y, w, h);
	}
      }

    } else {
      delete textOut;
      exitCode = 2;
      goto err3;
    }
  }
  delete textOut;

  // write end of HTML file
  if (htmlMeta) {
    if (!textFileName->cmp("-")) {
      f = stdout;
    } else {
      if (!(f = fopen(textFileName->getCString(), "ab"))) {
	error(errIO, -1, "Couldn't open text file '{0:t}'", textFileName);
	exitCode = 2;
	goto err3;
      }
    }
    if (!bbox) fputs("</pre>\n", f);
    fputs("</body>\n", f);
    fputs("</html>\n", f);
    if (f != stdout) {
      fclose(f);
    }
  }

  exitCode = 0;

  // clean up
 err3:
  delete textFileName;
 err2:
  delete doc;
  delete fileName;
  uMap->decRefCnt();
 err1:
  delete globalParams;
 err0:

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return exitCode;
}

static void printInfoString(FILE *f, Dict *infoDict, const char *key,
			    const char *text1, const char *text2, UnicodeMap *uMap) {
  GooString *s1;
  GBool isUnicode;
  Unicode u;
  char buf[9];
  int i, n;

  Object obj = infoDict->lookup(key);
  if (obj.isString()) {
    fputs(text1, f);
    s1 = obj.getString();
    if ((s1->getChar(0) & 0xff) == 0xfe &&
	(s1->getChar(1) & 0xff) == 0xff) {
      isUnicode = gTrue;
      i = 2;
    } else {
      isUnicode = gFalse;
      i = 0;
    }
    while (i < obj.getString()->getLength()) {
      if (isUnicode) {
	u = ((s1->getChar(i) & 0xff) << 8) |
	    (s1->getChar(i+1) & 0xff);
	i += 2;
      } else {
	u = pdfDocEncoding[s1->getChar(i) & 0xff];
	++i;
      }
      n = uMap->mapUnicode(u, buf, sizeof(buf));
      buf[n] = '\0';
      const std::string myString = myXmlTokenReplace(buf);
      fputs(myString.c_str(), f);
    }
    fputs(text2, f);
  }
}

static void printInfoDate(FILE *f, Dict *infoDict, const char *key, const char *fmt) {
  Object obj = infoDict->lookup(key);
  if (obj.isString()) {
    char *s = obj.getString()->getCString();
    if (s[0] == 'D' && s[1] == ':') {
      s += 2;
    }
    fprintf(f, fmt, s);
  }
}

void printLine(FILE *f, TextLine *line) {
  double xMin, yMin, xMax, yMax;
  double lineXMin = 0, lineYMin = 0, lineXMax = 0, lineYMax = 0;
  TextWord *word;
  std::stringstream wordXML;
  wordXML << std::fixed << std::setprecision(6);

  for (word = line->getWords(); word; word = word->getNext()) {
    word->getBBox(&xMin, &yMin, &xMax, &yMax);

    if (lineXMin == 0 || lineXMin > xMin) lineXMin = xMin;
    if (lineYMin == 0 || lineYMin > yMin) lineYMin = yMin;
    if (lineXMax < xMax) lineXMax = xMax;
    if (lineYMax < yMax) lineYMax = yMax;

    const std::string myString = myXmlTokenReplace(word->getText()->getCString());
    wordXML << "          <word xMin=\"" << xMin << "\" yMin=\"" << yMin << "\" xMax=\"" <<
            xMax << "\" yMax=\"" << yMax << "\">" << myString << "</word>\n";
  }
  fprintf(f, "        <line xMin=\"%f\" yMin=\"%f\" xMax=\"%f\" yMax=\"%f\">\n",
          lineXMin, lineYMin, lineXMax, lineYMax);
  fputs(wordXML.str().c_str(), f);
  fputs("        </line>\n", f);
}

void printDocBBox(FILE *f, PDFDoc *doc, TextOutputDev *textOut, int first, int last) {
  double xMin, yMin, xMax, yMax;
  TextPage *textPage;
  TextFlow *flow;
  TextBlock *blk;
  TextLine *line;

  fprintf(f, "<doc>\n");
  for (int page = first; page <= last; ++page) {
    fprintf(f, "  <page width=\"%f\" height=\"%f\">\n",doc->getPageMediaWidth(page), doc->getPageMediaHeight(page));
    doc->displayPage(textOut, page, resolution, resolution, 0, gTrue, gFalse, gFalse);
    textPage = textOut->takeText();
    for (flow = textPage->getFlows(); flow; flow = flow->getNext()) {
      fprintf(f, "    <flow>\n");
      for (blk = flow->getBlocks(); blk; blk = blk->getNext()) {
        blk->getBBox(&xMin, &yMin, &xMax, &yMax);
        fprintf(f, "      <block xMin=\"%f\" yMin=\"%f\" xMax=\"%f\" yMax=\"%f\">\n", xMin, yMin, xMax, yMax);
        for (line = blk->getLines(); line; line = line->getNext()) {
          printLine(f, line);
        }
        fprintf(f, "      </block>\n");
      }
      fprintf(f, "    </flow>\n");
    }
    fprintf(f, "  </page>\n");
    textPage->decRefCnt();
  }
  fprintf(f, "</doc>\n");
}

void printWordBBox(FILE *f, PDFDoc *doc, TextOutputDev *textOut, int first, int last) {
  fprintf(f, "<doc>\n");
  for (int page = first; page <= last; ++page) {
    fprintf(f, "  <page width=\"%f\" height=\"%f\">\n",doc->getPageMediaWidth(page), doc->getPageMediaHeight(page));
    doc->displayPage(textOut, page, resolution, resolution, 0, gTrue, gFalse, gFalse);
    TextWordList *wordlist = textOut->makeWordList();
    const int word_length = wordlist != NULL ? wordlist->getLength() : 0;
    TextWord *word;
    double xMinA, yMinA, xMaxA, yMaxA;
    if (word_length == 0)
      fprintf(stderr, "no word list\n");

    for (int i = 0; i < word_length; ++i) {
      word = wordlist->get(i);
      word->getBBox(&xMinA, &yMinA, &xMaxA, &yMaxA);
      const std::string myString = myXmlTokenReplace(word->getText()->getCString());
      fprintf(f,"    <word xMin=\"%f\" yMin=\"%f\" xMax=\"%f\" yMax=\"%f\">%s</word>\n", xMinA, yMinA, xMaxA, yMaxA, myString.c_str());
    }
    fprintf(f, "  </page>\n");
    delete wordlist;
  }
  fprintf(f, "</doc>\n");
}
