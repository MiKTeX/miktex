/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "files.pas" */


#include "p2c.h"


#define FILES_G
#include "files.h"


#ifndef CONTROL_H
#include "control.h"
#endif

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef UTILITY_H
#include "utility.h"
#endif


#define param_leader    '-'


Static Char stylefilename[256] = "mtxstyle.txt";

Static short teststyle;


boolean endOfInfile(void)
{
  return (eofAll());
}


Static void putTeXlines(Char *s_)
{
  Char s[256];
  short p;
  Char STR1[256], STR3[256];

  strcpy(s, s_);
  p = pos1('\n', s);
  while (p > 0) {
    sprintf(STR3, "\\%s\\", substr_(STR1, s, 1, p - 1));
    putLine(STR3);
    predelete(s, p);
    p = pos1('\n', s);
  }
  if (*s != '\0') {
    sprintf(STR3, "\\%s\\", s);
    putLine(STR3);
  }
}


void TeXtype2(Char *s)
{
  Char STR2[256];

  if (!pmx_preamble_done) {
    putLine(s);
    return;
  }
  if (s[0] == '%') {
    putLine(s);
    return;
  }
  if (first_paragraph)
    putTeXlines(s);
  else {
    sprintf(STR2, "\\\\%s\\", s);
    putLine(STR2);
  }
}


void putLine(Char *line)
{
  if (outlen + strlen(line) >= PMXlinelength)
    putc('\n', outfile);
  fprintf(outfile, "%s\n", line);
  outlen = 0;
}


void put(Char *line_, boolean putspace_)
{
  Char line[256];
  short l;
  Char STR1[256];

  strcpy(line, line_);
  l = strlen(line);
  if (l > PMXlinelength)
    error("Item longer than PMX line length", print);
  if (outlen + l >= PMXlinelength) {
    putLine("");
    put(line, false);
    return;
  }
  if (putspace_ && outlen > 0 && line[0] != ' ')
    sprintf(line, " %s", strcpy(STR1, line));
  fputs(line, outfile);
  outlen += l;
}


boolean styleFileFound(void)
{
  return (teststyle != 0);
}


Static void helpmessage(void)
{
  printf("Usage: prepmx [-bcfnhimtuvwDH0123456789] MTXFILE [TEXDIR] [STYLEFILE]\n");
}


Static void bighelpmessage(void)
{
  helpmessage();
  printf("\nMTXFILE: name of .mtx file without its extension\n");
  printf("TEXDIR: directory where the TeX file made by PMX goes, default is ./\n");
  printf("STYLEFILE: name of a file containing style definitions.  Default is\n");
  printf("  mtxstyle.txt.  This feature is now deprecated; use Include: STYLEFILE\n");
  printf("  in the preamble of the .mtx file instead.\n");
  printf("Options:  (can also be entered separately: -b -c ...)\n");
  printf("  -b: disable unbeamVocal\n");
  printf("  -c: disable doChords\n");
  printf("  -f: enable solfaNoteNames\n");
  printf("  -h: display this message and quit\n");
  printf("  -i: enable ignoreErrors\n");
  printf("  -m: disable doLyrics\n");
  printf("  -n: enable instrumentNames\n");
  printf("  -t: disable doUptext\n");
  printf("  -u: disable uptextOnRests\n");
  printf("  -v: enable beVerbose\n");
  printf("  -w: enable pedanticWarnings\n");
  printf("  -D: enable debugMode\n");
  printf("  -0123456789: select Case\n");
  printf("  -H: print enabled status of all options\n");
  printf("All the above, and some other, options can be enabled or disabled\n");
  printf("  in the preamble.  What you do there overrides what you do here.\n");
}


void processOption(Char j)
{
  switch (j) {

  case 'b':
    setFeature("unbeamVocal", false);
    break;

  case 'c':
    setFeature("doChords", false);
    break;

  case 'f':
    setFeature("solfaNoteNames", true);
    break;

  case 'h':
    bighelpmessage();
    _Escape(255);
    break;

  case 'i':
    setFeature("ignoreErrors", true);
    break;

  case 'm':
    setFeature("doLyrics", false);
    break;

  case 'n':
    setFeature("instrumentNames", true);
    break;

  case 't':
    setFeature("doUptext", false);
    break;

  case 'u':
    setFeature("uptextOnRests", false);
    break;

  case 'v':
    setFeature("beVerbose", true);
    break;

  case 'w':
    setFeature("pedanticWarnings", true);
    break;

  case 'D':
    setFeature("debugMode", true);
    break;

  case 'H':
    printFeatures(true);
    break;

  default:
    if (isdigit(j))
      choice = j;
    else {
      putchar(j);
      error(": invalid option", !print);
    }
    break;
  }
}


/* Local variables for OpenFiles: */
struct LOC_OpenFiles {
  short testin;
  Char basename[256];
} ;

Local void checkExistingFile(struct LOC_OpenFiles *LINK)
{
  FILE *tryfile;
  Char tryfile_NAME[_FNSIZE];

  strcpy(tryfile_NAME, LINK->basename);
  tryfile = fopen(tryfile_NAME, "r+b");
  _SETIO(tryfile != NULL, FileNotFound);
  LINK->testin = P_ioresult;
  if (LINK->testin != 0)
    return;
  if (tryfile != NULL)
    fclose(tryfile);
  printf("There exists a file named %s.  I am treating this\n",
	 LINK->basename);
  error("  as a fatal error unless you specify -i", !print);
}


void OpenFiles(void)
{
  struct LOC_OpenFiles V;
  short i, j, l;
  short fileid = 0;
  Char infilename[256], outfilename[256], param[256], ext[256];
  short FORLIM1;
  Char STR3[256];

  line_no = 0;
  paragraph_no = 0;
  for (i = 1; i <= P_argc - 1; i++) {
    strcpy(param, P_argv[i]);
    if (param[0] == param_leader) {
      FORLIM1 = strlen(param);
      for (j = 1; j <= FORLIM1 - 1; j++)
	processOption(param[j]);
    } else if (fileid == 0)
      fileid = i;
    else if (*texdir == '\0')
      strcpy(texdir, param);
    else
      strcpy(stylefilename, param);
  }
  if (fileid == 0) {
    helpmessage();
    printf("Try \"prepmx -h\" for more information.\n");
    _Escape(255);
  }
  strcpy(V.basename, P_argv[fileid]);
  l = strlen(V.basename);
  if (l > 4 && V.basename[l-4] == '.') {
    substr_(ext, V.basename, l - 2, 3);
    toUpper(ext);
    if (!strcmp(ext, "MTX")) {
      warning(".mtx extension deleted from basename", !print);
      shorten(V.basename, l - 4);
    }
  }
  if (pos1('.', V.basename) > 0)
    checkExistingFile(&V);
  sprintf(infilename, "%s.mtx", V.basename);
  sprintf(outfilename, "%s.pmx", V.basename);
  pushFile(infilename);
  strcpy(outfile_NAME, outfilename);
  if (outfile != NULL)
    outfile = freopen(outfile_NAME, "w", outfile);
  else
    outfile = fopen(outfile_NAME, "w");
  _SETIO(outfile != NULL, FileNotFound);
  strcpy(stylefile_NAME, stylefilename);
  if (stylefile != NULL)
    stylefile = freopen(stylefile_NAME, "r", stylefile);
  else
    stylefile = fopen(stylefile_NAME, "r");
  _SETIO(stylefile != NULL, FileNotFound);
  teststyle = P_ioresult;
  if (teststyle != 0 && strcmp(stylefilename, "mtxstyle.txt"))
    _SETIO(printf("Can't read %s\n", stylefilename) >= 0, FileWriteError);
  if (fileError()) {
    sprintf(STR3, "Input file %s not found", infilename);
    fatalerror(STR3);
  }
  outfile_open = true;
  printf("Writing to %s.pmx\n", V.basename);
}


void CloseFiles(void)
{
  if (outfile != NULL)
    fclose(outfile);
  outfile = NULL;
  closeAll();
  if (teststyle != 0)
    return;
  if (stylefile != NULL)
    fclose(stylefile);
  stylefile = NULL;
}


void readParagraph(Char (*P)[256], short *no, paragraph_index0 *L)
{
  boolean another;
  Char filename[256], buffer[256];
  Char STR1[256], STR3[256];

  *L = 0;
  readData(buffer);
  line_no = currentLineNo();
  if (isEmpty(buffer))
    return;
  if (debugMode())
    printf(">>>> %s\n", buffer);
  paragraph_no++;
  /* Handle directives affecting the processing of the input file */
  do {
    another = false;
    if (startsWithIgnoreCase(buffer, "SUSPEND")) {
      ignore_input = true;
      another = true;
      if (beVerbose())
	printf("-->> Suspending input file %s at line %d\n",
	       currentFilename(STR3), line_no);
    }
    if (startsWithIgnoreCase(buffer, "RESUME")) {
      ignore_input = false;
      another = true;
      if (beVerbose())
	printf("-->> Resuming input file %s at line %d\n",
	       currentFilename(STR1), line_no);
    }
    if (startsWithIgnoreCase(buffer, "INCLUDE:")) {
      predelete(buffer, 8);
      NextWord(filename, buffer, ' ', ' ');
      pushFile(filename);
      another = true;
    }
    if (another) {
      readLine(buffer);
      line_no = currentLineNo();
    }
  } while (another);
  /* Normal paragraph input*/
  do {
    if (*L < lines_in_paragraph) {
      (*L)++;
      strcpy(P[*L - 1], buffer);
      *buffer = '\0';
      no[*L - 1] = line_no;
    } else
      warning("Paragraph too long: skipping line", !print);
    readLine(buffer);
    line_no = currentLineNo();
    if (debugMode())
      printf("%d >> %s\n", line_no, buffer);
  } while (!isEmpty(buffer));
  skipBlanks();   /* needed to identify final paragraph */
}




/* End. */
