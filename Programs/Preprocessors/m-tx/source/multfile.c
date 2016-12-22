/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "multfile.pas" */


#include "p2c.h"


#define MULTFILE_G
#include "multfile.h"


Char nextData[256] = "";


typedef struct filenode {
  Char name[256];
  FILE *actualfile;
  struct filenode *prev;
  short lineno;
  Char actualfile_NAME[_FNSIZE];
} filenode;


Static filenode *current = NULL;
Static short last_valid_line_no = 0;
Static boolean inputerror = false;
Static short reportitem = reportnewfile + reportoldfile + reporterror +
			  reportrecursive;


void report(short items)
{
  reportitem = items;
}


Static boolean recursive(Char *filename)
{
  boolean Result;
  filenode *previous;

  if (current == NULL)
    return false;
  previous = current;
/* p2c: multfile.pas: Note: Eliminated unused assignment statement [338] */
  while (previous != NULL) {
    if (!strcmp(filename, previous->name))
      return true;
    previous = previous->prev;
  }
  return false;
}


void pushFile(Char *filename)
{
  filenode *newnode;
  Char STR3[256];

  if (recursive(filename)) {
    printf("===! Ignoring recursive include of file %s\n", filename);
    return;
  }
#if defined(MIKTEX)
  newnode = Malloc(sizeof(filenode));
#else
  newnode = malloc(sizeof(filenode));
#endif
  newnode->actualfile = NULL;
  strcpy(newnode->name, filename);
  newnode->prev = current;
  newnode->lineno = 0;
  strcpy(newnode->actualfile_NAME, filename);
  if (newnode->actualfile != NULL)
    newnode->actualfile = freopen(newnode->actualfile_NAME, "r",
				  newnode->actualfile);
  else
    newnode->actualfile = fopen(newnode->actualfile_NAME, "r");
  _SETIO(newnode->actualfile != NULL, FileNotFound);
  inputerror = (P_ioresult != 0);
  if (inputerror)
    Free(newnode);
  else
    current = newnode;
  if (!inputerror && (reportitem & reportnewfile) > 0)
    printf("==>> Input from file %s\n", currentFilename(STR3));
  if (inputerror && (reportitem & reporterror) > 0)
    printf("==!! Could not open file %s\n", filename);
}


void popFile(void)
{
  filenode *previous;
  Char STR2[256];
  Char STR3[256];

  if (current == NULL)
    return;
  if ((reportitem & reportclose) > 0)
    printf("==>> Closing file %s at line number %d\n",
	   currentFilename(STR2), currentLineNo());
  if (current->actualfile != NULL)
    fclose(current->actualfile);
  current->actualfile = NULL;
  previous = current->prev;
  Free(current);
  current = previous;
  if (current != NULL && (reportitem & reportoldfile) > 0)
    printf("==>> Resuming input from file %s at line number %d\n",
	   currentFilename(STR3), currentLineNo());
}


void closeAll(void)
{
  do {
    popFile();
  } while (current != NULL);
}


boolean eofCurrent(void)
{
  return P_eof(current->actualfile);
}


Char *readLine(Char *Result)
{
  Char s[256];
  Char *TEMP;
  Char STR2[256];

  if (*nextData != '\0') {
    strcpy(Result, nextData);
    *nextData = '\0';
    return Result;
  }
  if (eofAll())
    return strcpy(Result, "");
  _SETIO(fgets(s, 256, current->actualfile) != NULL, EndOfFile);
  TEMP = strchr(s, '\n');
  if (TEMP != NULL)
    *TEMP = 0;
  strcpy(Result, s);
  inputerror = (P_ioresult != 0);
  if (!inputerror) {
    current->lineno++;
    last_valid_line_no = current->lineno;
  }
  if (inputerror && (reportitem & reporterror) > 0)
    printf("==!! Could not read from file %s\n", currentFilename(STR2));
  return Result;
}


boolean isEmpty(Char *s)
{
  short i, FORLIM;

  if (*s == '\0')
    return true;
  FORLIM = strlen(s);
  for (i = 0; i <= FORLIM - 1; i++) {
    if (s[i] != ' ')
      return false;
  }
  return true;
}


Char *readData(Char *Result)
{
  Char s[256];

  if (!isEmpty(nextData)) {
    strcpy(Result, nextData);
    *nextData = '\0';
    return Result;
  }
  while (!eofAll()) {
    readLine(s);
    if (!isEmpty(s))
      return strcpy(Result, s);
  }
  return strcpy(Result, "");
}


void skipBlanks(void)
{
  while (*nextData == '\0') {
    readData(nextData);
    if (eofAll())
      return;
  }
}


boolean eofAll(void)
{
  boolean Result;

/* p2c: multfile.pas: Note: Eliminated unused assignment statement [338] */
  if (current == NULL)
    return true;
  if (eofCurrent()) {
    popFile();
    return (eofAll());
  }
  return false;
}


short currentLineNo(void)
{
  return last_valid_line_no;
}


Char *currentFilename(Char *Result)
{
  if (current == NULL)
    return strcpy(Result, "No file open yet");
  else
    return strcpy(Result, current->name);
}


boolean fileError(void)
{
  return inputerror;
}




/* End. */
