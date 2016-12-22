/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "uptext.pas" */


#include "p2c.h"


#define UPTEXT_G
#include "uptext.h"


#ifndef GLOBALS_H
#include "globals.h"
#endif

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef MTXLINE_H
#include "mtxline.h"
#endif

#ifndef UTILITY_H
#include "utility.h"
#endif


typedef struct uptext_info {
  short uptext, uptext_adjust, uptext_lcz;
  Char uptext_font[256];
} uptext_info;


Static uptext_info U[maxvoices];


short uptextLineNo(short voice)
{
  return (U[voice-1].uptext);
}


void setUptextLineNo(short voice, short lno)
{
  U[voice-1].uptext = lno;
}


void clearUptext(void)
{
  voice_index voice, FORLIM;

  FORLIM = nvoices;
  for (voice = 0; voice <= FORLIM - 1; voice++)
    U[voice].uptext = 0;
}


void initUptext(void)
{
  voice_index voice, FORLIM;
  uptext_info *WITH;

  FORLIM = nvoices;
  for (voice = 0; voice <= FORLIM - 1; voice++) {
    WITH = &U[voice];
    WITH->uptext_adjust = 0;
    WITH->uptext_lcz = 3;
    *WITH->uptext_font = '\0';
  }
}


Static void textTranslate(Char *uptext, Char *font)
{
  short k;
  Char STR1[256], STR2[256], STR3[256], STR4[256];
  short FORLIM;

  if (*uptext == '\0')
    return;
  do {
    k = pos1('%', uptext);
    if (k > 0) {
      sprintf(STR4, "%s{\\mtxFlat}%s",
	      substr_(STR1, uptext, 1, k - 1),
	      substr_(STR3, uptext, k + 1, strlen(uptext) - k));
      strcpy(uptext, STR4);
    }
  } while (k != 0);
  do {
    k = pos1('#', uptext);
    if (k > 0) {
      sprintf(STR2, "%s{\\mtxSharp}%s",
	      substr_(STR1, uptext, 1, k - 1),
	      substr_(STR4, uptext, k + 1, strlen(uptext) - k));
      strcpy(uptext, STR2);
    }
  } while (k != 0);
  switch (uptext[0]) {

  case '<':
    if (!strcmp(uptext, "<"))
      strcpy(uptext, "\\mtxIcresc");
    else if (!strcmp(uptext, "<."))
      strcpy(uptext, "\\mtxTcresc");
    else {
      predelete(uptext, 1);
      sprintf(uptext, "\\mtxCresc{%s}", strcpy(STR4, uptext));
    }
    break;

  case '>':
    if (!strcmp(uptext, ">"))
      strcpy(uptext, "\\mtxIdecresc");
    else if (!strcmp(uptext, ">."))
      strcpy(uptext, "\\mtxTdecresc");
    else {
      predelete(uptext, 1);
      sprintf(uptext, "\\mtxDecresc{%s}", strcpy(STR1, uptext));
    }
    break;

  default:
    FORLIM = strlen(uptext);
    for (k = 0; k <= FORLIM - 1; k++) {
      if (pos1(uptext[k], "mpfzrs~") == 0)
	return;
    }
    break;
  }
  strcpy(font, "\\mtxPF");
}


#define default_        10
#define under           (-14)


/* Local variables for addUptext: */
struct LOC_addUptext {
  short voice;
  Char w[256];
  short adj;
} ;

Local void adjustUptext(struct LOC_addUptext *LINK)
{
  Char letter;
  boolean force = false;
  uptext_info *WITH1;

  delete1(LINK->w, 1);
  while (*LINK->w != '\0') {
/* p2c: uptext.pas: Note: Eliminated unused assignment statement [338] */
    letter = LINK->w[0];
    delete1(LINK->w, 1);
    WITH1 = &U[LINK->voice-1];
    switch (letter) {

    case '<':
      if (WITH1->uptext_lcz > 1)
	WITH1->uptext_lcz--;
      break;

    case '>':
      if (WITH1->uptext_lcz < 3)
	WITH1->uptext_lcz++;
      break;

    case '^':
      WITH1->uptext_adjust = 0;
      break;

    case 'v':
      WITH1->uptext_adjust = under;
      break;

    case '=':
      force = true;
      break;

    case '+':
    case '-':
      if (*LINK->w != '\0')
	getNum(LINK->w, &LINK->adj);
      else
	LINK->adj = 0;
      if (letter == '-')
	LINK->adj = -LINK->adj;
      if (force)
	WITH1->uptext_adjust = LINK->adj;
      else
	WITH1->uptext_adjust += LINK->adj;
      *LINK->w = '\0';
      break;

    default:
      error3(LINK->voice, "Unknown uptext adjustment");
      break;
    }
  }
  strcpy(LINK->w, "!");
}


void addUptext(short voice_, boolean *no_uptext, Char *pretex)
{
  struct LOC_addUptext V;
  Char font[256];
  static Char lcz[4] = "lcz";
  uptext_info *WITH;
  Char STR1[256], STR2[256], STR3[256];

  V.voice = voice_;
  WITH = &U[V.voice-1];
  if (WITH->uptext == 0)
    *no_uptext = true;
  if (*no_uptext)
    return;
  do {
    GetNextWord(V.w, P[WITH->uptext - 1], blank, dummy);
    sprintf(STR1, "%c", barsym);
    if (!strcmp(V.w, STR1) || *V.w == '\0')
      *no_uptext = true;
    sprintf(STR1, "%c", tilde);
    if (!strcmp(V.w, STR1) || *no_uptext)
      return;
    if (V.w[0] == '!') {
      strcpy(WITH->uptext_font, V.w);
      WITH->uptext_font[0] = '\\';
    }
    if (V.w[0] == '@')
      adjustUptext(&V);
  } while (V.w[0] == '!');   /* ! is a kludge, will get me in trouble later */
  strcpy(font, WITH->uptext_font);
  textTranslate(V.w, font);
  if (*font != '\0')
    sprintf(V.w, "%s{%s}", font, strcpy(STR1, V.w));
  switch (lcz[WITH->uptext_lcz - 1]) {

  case 'l':
    sprintf(V.w, "\\mtxLchar{%s}{%s}",
	    toString(STR1, default_ + WITH->uptext_adjust),
	    strcpy(STR3, V.w));
    break;

  case 'c':
    sprintf(V.w, "\\mtxCchar{%s}{%s}",
	    toString(STR1, default_ + WITH->uptext_adjust),
	    strcpy(STR2, V.w));
    break;

  case 'z':
    sprintf(V.w, "\\mtxZchar{%s}{%s}",
	    toString(STR1, default_ + WITH->uptext_adjust),
	    strcpy(STR3, V.w));
    break;
  }
  strcat(pretex, V.w);
}

#undef default_
#undef under




/* End. */
