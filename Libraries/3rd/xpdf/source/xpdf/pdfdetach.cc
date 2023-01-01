//========================================================================
//
// pdfdetach.cc
//
// Copyright 2010 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>
#include <stdio.h>
#include "gtypes.h"
#include "gmem.h"
#include "gmempp.h"
#include "parseargs.h"
#include "GlobalParams.h"
#include "PDFDoc.h"
#include "CharTypes.h"
#include "UnicodeMap.h"
#include "UTF8.h"
#include "Error.h"
#include "config.h"

static GBool doList = gFalse;
static int saveNum = 0;
static GBool saveAll = gFalse;
static char savePath[1024] = "";
static char textEncName[128] = "";
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static char cfgFileName[256] = "";
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static ArgDesc argDesc[] = {
  {"-list",   argFlag,     &doList,        0,
   "list all embedded files"},
  {"-save",   argInt,      &saveNum,       0,
   "save the specified embedded file"},
  {"-saveall", argFlag,    &saveAll,       0,
   "save all embedded files"},
  {"-o",      argString,   savePath,       sizeof(savePath),
   "file name for the saved embedded file"},
  {"-enc",    argString,   textEncName,    sizeof(textEncName),
   "output text encoding name"},
  {"-opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",    argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  {"-cfg",        argString,      cfgFileName,    sizeof(cfgFileName),
   "configuration file to use in place of .xpdfrc"},
  {"-v",      argFlag,     &printVersion,  0,
   "print copyright and version info"},
  {"-h",      argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"--help",  argFlag,     &printHelp,     0,
   "print usage information"},
  {"-?",      argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};

int main(int argc, char *argv[]) {
  char *fileName;
  UnicodeMap *uMap;
  GString *ownerPW, *userPW;
  PDFDoc *doc;
  Unicode *name;
  char uBuf[8];
  GString *path;
  GBool ok;
  int exitCode;
  int nFiles, nameLen, n, i, j;

  exitCode = 99;

  // parse args
  fixCommandLine(&argc, &argv);
  ok = parseArgs(argDesc, &argc, argv);
  if ((doList ? 1 : 0) +
      ((saveNum != 0) ? 1 : 0) +
      (saveAll ? 1 : 0) != 1) {
    ok = gFalse;
  }
  if (!ok || argc != 2 || printVersion || printHelp) {
    fprintf(stderr, "pdfdetach version %s [www.xpdfreader.com]\n", xpdfVersion);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdfdetach", "<PDF-file>", argDesc);
    }
    goto err0;
  }
  fileName = argv[1];

  // read config file
  if (cfgFileName[0] && !pathIsFile(cfgFileName)) {
    error(errConfig, -1, "Config file '{0:s}' doesn't exist or isn't a file",
	  cfgFileName);
  }
  globalParams = new GlobalParams(cfgFileName);
  if (textEncName[0]) {
    globalParams->setTextEncoding(textEncName);
  }

  // get mapping to output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    error(errConfig, -1, "Couldn't get text encoding");
    goto err1;
  }

  // open PDF file
  if (ownerPassword[0] != '\001') {
    ownerPW = new GString(ownerPassword);
  } else {
    ownerPW = NULL;
  }
  if (userPassword[0] != '\001') {
    userPW = new GString(userPassword);
  } else {
    userPW = NULL;
  }
  doc = new PDFDoc(fileName, ownerPW, userPW);
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

  nFiles = doc->getNumEmbeddedFiles();

  // list embedded files
  if (doList) {
    printf("%d embedded files\n", nFiles);
    for (i = 0; i < nFiles; ++i) {
      printf("%d: ", i+1);
      name = doc->getEmbeddedFileName(i);
      nameLen = doc->getEmbeddedFileNameLength(i);
      for (j = 0; j < nameLen; ++j) {
	n = uMap->mapUnicode(name[j], uBuf, sizeof(uBuf));
	fwrite(uBuf, 1, n, stdout);
      }
      fputc('\n', stdout);
    }

  // save all embedded files
  } else if (saveAll) {
    for (i = 0; i < nFiles; ++i) {
      if (savePath[0]) {
	path = new GString(savePath);
	path->append('/');
      } else {
	path = new GString();
      }
      name = doc->getEmbeddedFileName(i);
      nameLen = doc->getEmbeddedFileNameLength(i);
      for (j = 0; j < nameLen; ++j) {
	n = mapUTF8(name[j], uBuf, sizeof(uBuf));
	path->append(uBuf, n);
      }
      if (!doc->saveEmbeddedFileU(i, path->getCString())) {
	error(errIO, -1, "Error saving embedded file as '{0:t}'", path);
	delete path;
	exitCode = 2;
	goto err2;
      }
      delete path;
    }

  // save an embedded file
  } else {
    if (saveNum < 1 || saveNum > nFiles) {
      error(errCommandLine, -1, "Invalid file number");
      goto err2;
    }
    if (savePath[0]) {
      path = new GString(savePath);
    } else {
      name = doc->getEmbeddedFileName(saveNum - 1);
      nameLen = doc->getEmbeddedFileNameLength(saveNum - 1);
      path = new GString();
      for (j = 0; j < nameLen; ++j) {
	n = mapUTF8(name[j], uBuf, sizeof(uBuf));
	path->append(uBuf, n);
      }
    }
    if (!doc->saveEmbeddedFileU(saveNum - 1, path->getCString())) {
      error(errIO, -1, "Error saving embedded file as '{0:t}'", path);
      delete path;
      exitCode = 2;
      goto err2;
    }
    delete path;
  }

  exitCode = 0;

  // clean up
 err2:
  uMap->decRefCnt();
  delete doc;
 err1:
  delete globalParams;
 err0:

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return exitCode;
}
