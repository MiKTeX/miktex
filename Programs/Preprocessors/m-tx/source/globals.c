/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "globals.pas" */


#include "p2c.h"


#define GLOBALS_G
#include "globals.h"


Char double_comment[3] = "%%";
Char durations[ndurs + 1] = "90248136";
Char terminators[256] = ".x";
Char has_duration[9] = "abcdefgr";
Char solfa_names[8] = "drmfslt";
Char choice = ' ';
boolean outfile_open = false;
Char texdir[256] = "";
Char old_meter_word[256] = "";
short outlen = 0;
boolean ignore_input = false;

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef CONTROL_H
#include "control.h"
#endif

#ifndef UTILITY_H
#include "utility.h"
#endif


boolean isNoteOrRest(Char *w)
{
  return (pos1(w[0], has_duration) > 0);
}


boolean isPause(Char *note)
{
  return (startsWith(note, pause));
}


void cancel(short *num, short *denom, short lowest)
{
  while (((*num) & 1) == 0 && *denom > lowest) {
    *num /= 2;
    *denom /= 2;
  }
}


Char *meterWord(Char *Result, short num, short denom, short pnum,
		short pdenom)
{
  Char STR1[256], STR2[256], STR4[256], STR5[256];

  sprintf(Result, "m%s/%s/%s/%s",
	  toString(STR1, num), toString(STR2, denom), toString(STR4, pnum),
	  toString(STR5, pdenom));
  return Result;
}


Char *meterChange(Char *Result, short n1, short n2, boolean blind)
{
  short f = 64;
  short l;

  if (!blind)
    return (meterWord(Result, n1, PMXmeterdenom(n2), 0, 0));
  /* CMO: process denominator value with function PMXmeterdenom */
  l = n1 * (64 / n2);
  cancel(&l, &f, meterdenom);
  /* CMO: process denominator value with function PMXmeterdenom */
  meterWord(Result, l, PMXmeterdenom(f), 0, 0);
  if (meternum > 0)
    printf("Blind meter change to %d/%d on line %d\n", l, f, line_no);
  return Result;
}


void setSpace(Char *line_)
{
  Char line[256];
  short i = 0;
  Char word[256];

  strcpy(line, line_);
  while (i < ninstr) {
    GetNextWord(word, line, blank, dummy);
    if (*word == '\0')
      return;
    i++;
    getNum(word, &nspace[i-1]);
  }
}


Static void onumber(Char *s, short *j, short *n1)
{
  if (s[*j - 1] == 'o')
    *n1 = 1;
  else if (s[*j - 1] == '1') {
    *n1 = digit(s[*j]) + 10;
    (*j)++;
  } else
    *n1 = digit(s[*j - 1]);
  (*j)++;
}


Static void extractNumber(Char *s, short *k)
{
  Char w[256];

  GetNextWord(w, s, '/', dummy);
  getNum(w, k);
}


Static void readMeter(Char *meter_, short *meternum, short *meterdenom,
		      short *pmnum, short *pmdenom)
{
  Char meter[256];
  short j;

  strcpy(meter, meter_);
  if (meter[0] == 'm') {
    if (pos1('/', meter) == 0) {
      j = 2;
      onumber(meter, &j, meternum);
      onumber(meter, &j, meterdenom);
      onumber(meter, &j, pmnum);
      onumber(meter, &j, pmdenom);
      return;
    }
    predelete(meter, 1);
    extractNumber(meter, meternum);
    extractNumber(meter, meterdenom);
    extractNumber(meter, pmnum);
    extractNumber(meter, pmdenom);
    return;
  }
  getTwoNums(meter, meternum, meterdenom);
  *pmnum = *meternum;
  *pmdenom = *meterdenom;
}


void getMeter(Char *line_, short *meternum, short *meterdenom, short *pmnum,
	      short *pmdenom)
{
  Char line[256];
  Char meter[256];
  Char STR1[256];

  strcpy(line, line_);
  GetNextWord(meter, line, blank, dummy);
  if (!strcmp(meter, "C/") || !strcmp(meter, "mC/")) {
    *meternum = 2;
    *meterdenom = 2;
    *pmdenom = 5;
    *pmnum = 0;
  } else if (!strcmp(meter, "C") || !strcmp(meter, "mC")) {
    *meternum = 4;
    *meterdenom = 4;
    *pmdenom = 6;
    *pmnum = 0;
  } else
    readMeter(meter, meternum, meterdenom, pmnum, pmdenom);
  if (*meterdenom == 0) {
    sprintf(STR1, "%s: Meter denominator must be nonzero", meter);
    error(STR1, print);
  }
  /* CMO: Convert PMX syntax meter denominator '0' to '1' to be used for
     prepmx duration checks */
  /* if meterdenom=0 then meterdenom:=1; */
}


Static short whereInParagraph(short l)
{
  short Result = 0;
  short j, FORLIM;

  FORLIM = para_len;
  for (j = 1; j <= FORLIM; j++) {
    if (orig_line_no[j-1] == l)
      return j;
  }
  return Result;
}


void fatalerror(Char *message)
{
  setFeature("ignoreErrors", false);
  error(message, !print);
}


void error(Char *message, boolean printLine)
{
  short j;

  j = whereInParagraph(line_no);
  if (j > 0 && printLine)
    puts(orig_P[j-1]);
  printf("%s: ERROR on line %d\n", message, line_no);
  if (j > 0 && printLine) {
    printf("The line has been modified internally to:\n");
    puts(P[j-1]);
  }
  if (ignoreErrors())
    return;
  if (outfile_open) {
    if (outfile != NULL)
      fclose(outfile);
    outfile = NULL;
#if defined(MIKTEX)
    outfile = fopen(outfile_NAME, "wb");
#else
    outfile = fopen(outfile_NAME, "w");
#endif
    if (outfile == NULL)
      _EscIO2(FileNotFound, outfile_NAME);
    fclose(outfile);
    outfile = NULL;
  }
  if (line_no == 0)
    line_no = 10000;
  _Escape(line_no);
}


void warning(Char *message, boolean printLine)
{
  short j;

  if (line_no <= 0) {
    printf("%s: WARNING in preamble\n", message);
    return;
  }
  printf("%s: WARNING on line %d\n", message, line_no);
  if (!printLine)
    return;
  j = whereInParagraph(line_no);
  if (j > 0)
    puts(P[j-1]);
}


short PMXinstr(short stave)
{
  return (ninstr - instr[stave-1] + 1);
}


void setDefaultDuration(short meterdenom)
{
  switch (meterdenom) {

  case 1:
    default_duration = '0';
    break;

  case 2:
    default_duration = '2';
    break;

  case 4:
    default_duration = '4';
    break;

  case 8:
    default_duration = '8';
    break;

  case 16:
    default_duration = '1';
    break;

  case 32:
    default_duration = '3';
    break;

  case 64:
    default_duration = '6';
    break;
  }
}


short PMXmeterdenom(short denom)
{
  short Result;

  /* CMO: Convert M-Tx meter denominators to PMX syntax */
  switch (denom) {

  case 1:
    Result = 0;
    break;

  case 16:
    Result = 1;
    break;

  case 32:
    Result = 3;
    break;

  case 64:
    Result = 6;
    break;

  default:
    Result = denom;
    break;
  }
  return Result;
}




/* End. */
