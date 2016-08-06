//========================================================================
//
// pdfunite.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2011-2015 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012 Arseny Solokha <asolokha@gmx.com>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2012, 2014 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2015 Arthur Stavisky <vovodroid@gmail.com>
//
//========================================================================

#if defined(MIKTEX)
#include "config.h"
#endif

#include <PDFDoc.h>
#include <GlobalParams.h>
#include "parseargs.h"
#if !defined(MIKTEX)
#include "config.h"
#endif
#include <poppler-config.h>
#include <vector>

static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static const ArgDesc argDesc[] = {
  {"-v", argFlag, &printVersion, 0,
   "print copyright and version info"},
  {"-h", argFlag, &printHelp, 0,
   "print usage information"},
  {"-help", argFlag, &printHelp, 0,
   "print usage information"},
  {"--help", argFlag, &printHelp, 0,
   "print usage information"},
  {"-?", argFlag, &printHelp, 0,
   "print usage information"},
  {NULL}
};

void doMergeNameTree(PDFDoc *doc, XRef *srcXRef, XRef *countRef, int oldRefNum, int newRefNum, Dict *srcNameTree, Dict *mergeNameTree, int numOffset) {
  Object mergeNameArray;
  Object srcNameArray;
  mergeNameTree->lookup("Names", &mergeNameArray);
  srcNameTree->lookup("Names", &srcNameArray);
  if (mergeNameArray.isArray() && srcNameArray.isArray()) {
    Object *newNameArray = new Object();
    newNameArray->initArray(srcXRef);
    int j = 0;
    for (int i = 0; i < srcNameArray.arrayGetLength() - 1; i += 2) {
      Object key;
      Object value;
      srcNameArray.arrayGetNF(i, &key);
      srcNameArray.arrayGetNF(i + 1, &value);
      if (key.isString() && value.isRef()) {
        while (j < mergeNameArray.arrayGetLength() - 1) {
          Object mkey;
          Object mvalue;
          mergeNameArray.arrayGetNF(j, &mkey);
          mergeNameArray.arrayGetNF(j + 1, &mvalue);
          if (mkey.isString() && mvalue.isRef()) {
            if (mkey.getString()->cmp(key.getString()) < 0) {
              Object *newKey = new Object();
	      newKey->initString(new GooString(mkey.getString()->getCString()));
              newNameArray->arrayAdd(newKey);
              Object *newValue = new Object();
              newValue->initRef(mvalue.getRef().num + numOffset, mvalue.getRef().gen);
              newNameArray->arrayAdd(newValue);
              delete newKey;
              delete newValue;
              j += 2;
            } else if (mkey.getString()->cmp(key.getString()) == 0) {
              j += 2;
            } else {
              mkey.free();
              mvalue.free();
              break;
            }
          } else {
            j += 2;
          }
          mkey.free();
          mvalue.free();
        }
        Object *newKey = new Object();
        newKey->initString(new GooString(key.getString()->getCString()));
        newNameArray->arrayAdd(newKey);
        Object *newValue = new Object();
        newValue->initRef(value.getRef().num, value.getRef().gen);
        newNameArray->arrayAdd(newValue);
        delete newKey;
        delete newValue;
      }
      key.free();
      value.free();
    }
    while (j < mergeNameArray.arrayGetLength() - 1) {
      Object mkey;
      Object mvalue;
      mergeNameArray.arrayGetNF(j, &mkey);
      mergeNameArray.arrayGetNF(j + 1, &mvalue);
      if (mkey.isString() && mvalue.isRef()) {
        Object *newKey = new Object();
        newKey->initString(new GooString(mkey.getString()->getCString()));
        newNameArray->arrayAdd(newKey);
        Object *newValue = new Object();
        newValue->initRef(mvalue.getRef().num + numOffset, mvalue.getRef().gen);
        newNameArray->arrayAdd(newValue);
        delete newKey;
        delete newValue;
      }
      j += 2;
      mkey.free();
      mvalue.free();
    }
    srcNameTree->set("Names", newNameArray);
    doc->markPageObjects(mergeNameTree, srcXRef, countRef, numOffset, oldRefNum, newRefNum);
    delete newNameArray;
  } else if (srcNameArray.isNull() && mergeNameArray.isArray()) {
    Object *newNameArray = new Object();
    newNameArray->initArray(srcXRef);
    for (int i = 0; i < mergeNameArray.arrayGetLength() - 1; i += 2) {
      Object key;
      Object value;
      mergeNameArray.arrayGetNF(i, &key);
      mergeNameArray.arrayGetNF(i + 1, &value);
      if (key.isString() && value.isRef()) {
        Object *newKey = new Object();
	newKey->initString(new GooString(key.getString()->getCString()));
        newNameArray->arrayAdd(newKey);
        Object *newValue = new Object();
        newValue->initRef(value.getRef().num + numOffset, value.getRef().gen);
        newNameArray->arrayAdd(newValue);
        delete newKey;
        delete newValue;
      }
      key.free();
      value.free();
    }
    srcNameTree->add(copyString("Names"), newNameArray);
    doc->markPageObjects(mergeNameTree, srcXRef, countRef, numOffset, oldRefNum, newRefNum);
  }
  mergeNameArray.free();
  srcNameArray.free();
}

void doMergeNameDict(PDFDoc *doc, XRef *srcXRef, XRef *countRef, int oldRefNum, int newRefNum, Dict *srcNameDict, Dict *mergeNameDict, int numOffset) {
  for (int i = 0; i < mergeNameDict->getLength(); i++) {
    const char *key = mergeNameDict->getKey(i);
    Object mergeNameTree;
    Object srcNameTree;
    mergeNameDict->lookup(key, &mergeNameTree);
    srcNameDict->lookup(key, &srcNameTree);
    if (srcNameTree.isDict() && mergeNameTree.isDict()) {
      doMergeNameTree(doc, srcXRef, countRef, oldRefNum, newRefNum, srcNameTree.getDict(), mergeNameTree.getDict(), numOffset);
    } else if (srcNameTree.isNull() && mergeNameTree.isDict()) {
      Object *newNameTree = new Object();
      newNameTree->initDict(srcXRef);
      doMergeNameTree(doc, srcXRef, countRef, oldRefNum, newRefNum, newNameTree->getDict(), mergeNameTree.getDict(), numOffset);
      srcNameDict->add(copyString(key), newNameTree);
    }
    srcNameTree.free();
    mergeNameTree.free();
  }
}

///////////////////////////////////////////////////////////////////////////
#if defined(MIKTEX)
int Main(int argc, char** argv)
#else
int main (int argc, char *argv[])
#endif
///////////////////////////////////////////////////////////////////////////
// Merge PDF files given by arguments 1 to argc-2 and write the result
// to the file specified by argument argc-1.
///////////////////////////////////////////////////////////////////////////
{
  int objectsCount = 0;
  Guint numOffset = 0;
  std::vector<Object> pages;
  std::vector<Guint> offsets;
  XRef *yRef, *countRef;
  FILE *f;
  OutStream *outStr;
  int i;
  int j, rootNum;
  std::vector<PDFDoc *>docs;
  int majorVersion = 0;
  int minorVersion = 0;
  char *fileName = argv[argc - 1];
  int exitCode;

  exitCode = 99;
  const GBool ok = parseArgs (argDesc, &argc, argv);
  if (!ok || argc < 3 || printVersion || printHelp) {
    fprintf(stderr, "pdfunite version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdfunite", "<PDF-sourcefile-1>..<PDF-sourcefile-n> <PDF-destfile>",
	argDesc);
    }
    if (printVersion || printHelp)
      exitCode = 0;
    return exitCode;
  }
  exitCode = 0;
  globalParams = new GlobalParams();

  for (i = 1; i < argc - 1; i++) {
    GooString *gfileName = new GooString(argv[i]);
    PDFDoc *doc = new PDFDoc(gfileName, NULL, NULL, NULL);
    if (doc->isOk() && !doc->isEncrypted()) {
      docs.push_back(doc);
      if (doc->getPDFMajorVersion() > majorVersion) {
        majorVersion = doc->getPDFMajorVersion();
        minorVersion = doc->getPDFMinorVersion();
      } else if (doc->getPDFMajorVersion() == majorVersion) {
        if (doc->getPDFMinorVersion() > minorVersion) {
          minorVersion = doc->getPDFMinorVersion();
        }
      }
    } else if (doc->isOk()) {
      error(errUnimplemented, -1, "Could not merge encrypted files ('{0:s}')", argv[i]);
      return -1;
    } else {
      error(errSyntaxError, -1, "Could not merge damaged documents ('{0:s}')", argv[i]);
      return -1;
    }
  }

  if (!(f = fopen(fileName, "wb"))) {
    error(errIO, -1, "Could not open file '{0:s}'", fileName);
    return -1;
  }
  outStr = new FileOutStream(f, 0);

  yRef = new XRef();
  countRef = new XRef();
  yRef->add(0, 65535, 0, gFalse);
  PDFDoc::writeHeader(outStr, majorVersion, minorVersion);

  // handle OutputIntents, AcroForm, OCProperties & Names
  Object intents;
  Object afObj;
  Object ocObj;
  Object names;
  if (docs.size() >= 1) {
    Object catObj;
    docs[0]->getXRef()->getCatalog(&catObj);
    Dict *catDict = catObj.getDict();
    catDict->lookup("OutputIntents", &intents);
    catDict->lookupNF("AcroForm", &afObj);
    Ref *refPage = docs[0]->getCatalog()->getPageRef(1);
    if (!afObj.isNull()) {
      docs[0]->markAcroForm(&afObj, yRef, countRef, 0, refPage->num, refPage->num);
    }
    catDict->lookupNF("OCProperties", &ocObj);
    if (!ocObj.isNull() && ocObj.isDict()) {
      docs[0]->markPageObjects(ocObj.getDict(), yRef, countRef, 0, refPage->num, refPage->num);
    }
    catDict->lookup("Names", &names);
    if (!names.isNull() && names.isDict()) {
      docs[0]->markPageObjects(names.getDict(), yRef, countRef, 0, refPage->num, refPage->num);
    }
    if (intents.isArray() && intents.arrayGetLength() > 0) {
      for (i = 1; i < (int) docs.size(); i++) {
        Object pagecatObj, pageintents;
        docs[i]->getXRef()->getCatalog(&pagecatObj);
        Dict *pagecatDict = pagecatObj.getDict();
        pagecatDict->lookup("OutputIntents", &pageintents);
        if (pageintents.isArray() && pageintents.arrayGetLength() > 0) {
          for (j = intents.arrayGetLength() - 1; j >= 0; j--) {
            Object intent;
            intents.arrayGet(j, &intent, 0);
            if (intent.isDict()) {
              Object idf;
              intent.dictLookup("OutputConditionIdentifier", &idf);
              if (idf.isString()) {
                GooString *gidf = idf.getString();
                GBool removeIntent = gTrue;
                for (int k = 0; k < pageintents.arrayGetLength(); k++) {
                  Object pgintent;
                  pageintents.arrayGet(k, &pgintent, 0);
                  if (pgintent.isDict()) {
                    Object pgidf;
                    pgintent.dictLookup("OutputConditionIdentifier", &pgidf);
                    if (pgidf.isString()) {
                      GooString *gpgidf = pgidf.getString();
                      if (gpgidf->cmp(gidf) == 0) {
                        pgidf.free();
                        removeIntent = gFalse;
                        break;
                      }
                    }
                    pgidf.free();
                  }
                }
                if (removeIntent) {
                  intents.arrayRemove(j);
                  error(errSyntaxWarning, -1, "Output intent {0:s} missing in pdf {1:s}, removed",
                   gidf->getCString(), docs[i]->getFileName()->getCString());
                }
              } else {
                intents.arrayRemove(j);
                error(errSyntaxWarning, -1, "Invalid output intent dict, missing required OutputConditionIdentifier");
              }
              idf.free();
            } else {
              intents.arrayRemove(j);
            }
            intent.free();
          }
        } else {
          error(errSyntaxWarning, -1, "Output intents differs, remove them all");
          intents.free();
          break;
        }
        pagecatObj.free();
        pageintents.free();
      }
    }
    if (intents.isArray() && intents.arrayGetLength() > 0) {
      for (j = intents.arrayGetLength() - 1; j >= 0; j--) {
        Object intent;
        intents.arrayGet(j, &intent, 0);
        if (intent.isDict()) {
          docs[0]->markPageObjects(intent.getDict(), yRef, countRef, numOffset, 0, 0);
        } else {
          intents.arrayRemove(j);
        }
        intent.free();
      }
    }
    catObj.free();
  }

  for (i = 0; i < (int) docs.size(); i++) {
    for (j = 1; j <= docs[i]->getNumPages(); j++) {
      PDFRectangle *cropBox = NULL;
      if (docs[i]->getCatalog()->getPage(j)->isCropped())
        cropBox = docs[i]->getCatalog()->getPage(j)->getCropBox();
      docs[i]->replacePageDict(j,
	    docs[i]->getCatalog()->getPage(j)->getRotate(),
	    docs[i]->getCatalog()->getPage(j)->getMediaBox(), cropBox);
      Ref *refPage = docs[i]->getCatalog()->getPageRef(j);
      Object page;
      docs[i]->getXRef()->fetch(refPage->num, refPage->gen, &page);
      Dict *pageDict = page.getDict();
      Dict *resDict = docs[i]->getCatalog()->getPage(j)->getResourceDict();
      if (resDict) {
        Object *newResource = new Object();
        newResource->initDict(resDict);
        pageDict->set("Resources", newResource);
        delete newResource;
      }
      pages.push_back(page);
      offsets.push_back(numOffset);
      docs[i]->markPageObjects(pageDict, yRef, countRef, numOffset, refPage->num, refPage->num);
      Object annotsObj;
      pageDict->lookupNF("Annots", &annotsObj);
      if (!annotsObj.isNull()) {
        docs[i]->markAnnotations(&annotsObj, yRef, countRef, numOffset, refPage->num, refPage->num);
        annotsObj.free();
      }
    }
    Object pageCatObj, pageNames;
    docs[i]->getXRef()->getCatalog(&pageCatObj);
    Dict *pageCatDict = pageCatObj.getDict();
    pageCatDict->lookup("Names", &pageNames);
    if (!pageNames.isNull() && pageNames.isDict()) {
      if (!names.isDict()) {
        names.free();
        names.initDict(yRef);
      }
      doMergeNameDict(docs[i], yRef, countRef, 0, 0, names.getDict(), pageNames.getDict(), numOffset);
    }
    pageNames.free();
    pageCatObj.free();
    objectsCount += docs[i]->writePageObjects(outStr, yRef, numOffset, gTrue);
    numOffset = yRef->getNumObjects() + 1;
  }

  rootNum = yRef->getNumObjects() + 1;
  yRef->add(rootNum, 0, outStr->getPos(), gTrue);
  outStr->printf("%d 0 obj\n", rootNum);
  outStr->printf("<< /Type /Catalog /Pages %d 0 R", rootNum + 1);
  // insert OutputIntents
  if (intents.isArray() && intents.arrayGetLength() > 0) {
    outStr->printf(" /OutputIntents [");
    for (j = 0; j < intents.arrayGetLength(); j++) {
      Object intent;
      intents.arrayGet(j, &intent, 0);
      if (intent.isDict()) {
        PDFDoc::writeObject(&intent, outStr, yRef, 0, NULL, cryptRC4, 0, 0, 0);
      }
      intent.free();
    }
    outStr->printf("]");
  }
  intents.free();
  // insert AcroForm
  if (!afObj.isNull()) {
    outStr->printf(" /AcroForm ");
    PDFDoc::writeObject(&afObj, outStr, yRef, 0, NULL, cryptRC4, 0, 0, 0);
    afObj.free();
  }
  // insert OCProperties
  if (!ocObj.isNull() && ocObj.isDict()) {
    outStr->printf(" /OCProperties ");
    PDFDoc::writeObject(&ocObj, outStr, yRef, 0, NULL, cryptRC4, 0, 0, 0);
    ocObj.free();
  }
  // insert Names
  if (!names.isNull() && names.isDict()) {
    outStr->printf(" /Names ");
    PDFDoc::writeObject(&names, outStr, yRef, 0, NULL, cryptRC4, 0, 0, 0);
    names.free();
  }
  outStr->printf(">>\nendobj\n");
  objectsCount++;

  yRef->add(rootNum + 1, 0, outStr->getPos(), gTrue);
  outStr->printf("%d 0 obj\n", rootNum + 1);
  outStr->printf("<< /Type /Pages /Kids [");
  for (j = 0; j < (int) pages.size(); j++)
    outStr->printf(" %d 0 R", rootNum + j + 2);
  outStr->printf(" ] /Count %zd >>\nendobj\n", pages.size());
  objectsCount++;

  for (i = 0; i < (int) pages.size(); i++) {
    yRef->add(rootNum + i + 2, 0, outStr->getPos(), gTrue);
    outStr->printf("%d 0 obj\n", rootNum + i + 2);
    outStr->printf("<< ");
    Dict *pageDict = pages[i].getDict();
    for (j = 0; j < pageDict->getLength(); j++) {
      if (j > 0)
	outStr->printf(" ");
      const char *key = pageDict->getKey(j);
      Object value;
      pageDict->getValNF(j, &value);
      if (strcmp(key, "Parent") == 0) {
        outStr->printf("/Parent %d 0 R", rootNum + 1);
      } else {
        outStr->printf("/%s ", key);
        PDFDoc::writeObject(&value, outStr, yRef, offsets[i], NULL, cryptRC4, 0, 0, 0);
      }
      value.free();
    }
    outStr->printf(" >>\nendobj\n");
    objectsCount++;
  }
  Goffset uxrefOffset = outStr->getPos();
  Ref ref;
  ref.num = rootNum;
  ref.gen = 0;
  Dict *trailerDict = PDFDoc::createTrailerDict(objectsCount, gFalse, 0, &ref, yRef,
                                                fileName, outStr->getPos());
  PDFDoc::writeXRefTableTrailer(trailerDict, yRef, gTrue, // write all entries according to ISO 32000-1, 7.5.4 Cross-Reference Table: "For a file that has never been incrementally updated, the cross-reference section shall contain only one subsection, whose object numbering begins at 0."
                                uxrefOffset, outStr, yRef);
  delete trailerDict;

  outStr->close();
  delete outStr;
  fclose(f);
  delete yRef;
  delete countRef;
  for (j = 0; j < (int) pages.size (); j++) pages[j].free();
  for (i = 0; i < (int) docs.size (); i++) delete docs[i];
  delete globalParams;
  return exitCode;
}
