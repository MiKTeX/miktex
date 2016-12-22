/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "notes.pas" */


#include "p2c.h"


#define NOTES_G
#include "notes.h"


short count64['9' + 1 - '0'] = {
  64, 4, 32, 2, 16, 0, 1, 0, 8, 128
};

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef GLOBALS_H
#include "globals.h"
#endif


typedef struct parsedNote {
  Char name;
  Char duration[2];
  Char octave[9];
  Char accidental[17], whatever[17], dotgroup[17], xtuplet[17];
  Char shortcut[33];
} parsedNote;


Static void printNote(parsedNote n)
{
  printf("%c|%s|%s|%s|%s|%s|%s|%s\n",
	 n.name, n.duration, n.octave, n.accidental, n.whatever, n.dotgroup,
	 n.xtuplet, n.shortcut);
}


/* If rearrangeNote is TRUE, translate original note to the following form:
   1. Note name.
   2. Duration.
   3. Octave adjustments.
   4. Everything except the other six items.
   5. Accidental with adjustments (rest: height adjustment)
   6. Dot with adjustments.
   7. Xtuplet group.
*/

void translateSolfa(Char *nt)
{
  short k;

  if (!solfaNoteNames())
    return;
  k = pos1(*nt, solfa_names);
  if (k > 0)
    *nt = has_duration[k-1];
}


Char durationCode(Char *note)
{
  Char Result = unspecified;
  Char code;

  if (strlen(note) <= 1)
    return Result;
  code = note[1];
  if (pos1(code, durations) > 0)
    return code;
  return Result;
}


Static Char half(Char dur)
{
  Char Result;
  short k;
  Char STR1[20];
  Char STR2[24];

  k = pos1(dur, durations);
  Result = dur;
  if (k == 0) {
    sprintf(STR1, "Invalid duration %c", dur);
    error(STR1, print);
    return Result;
  }
  if (k <= ndurs)
    return (durations[k]);
  sprintf(STR2, "%c is too short to halve", dur);
  error(STR2, print);
  return Result;
}


Static void addDuration(Char *note, Char dur)
{
  if (insertDuration())
    insertChar(dur, note, 2);
}


/* Extract procedures.  All of these remove part of "note" (sometimes
   the part is empty) and put it somewhere else.  The part may be anywhere
   in "note", except when otherwise specified.*/

/* Unconditionally extracts the first character. */

Static void extractFirst(Char *note, Char *first)
{
  *first = note[0];
  predelete(note, 1);
}


/* Extracts at most one of the characters in "hits". */

Static void extractOneOf(Char *note, Char *hits, Char *hit)
{
  short i, l;

  l = strlen(note);
  *hit = '\0';
  for (i = 1; i <= l; i++) {
    if (pos1(note[i-1], hits) > 0) {
      sprintf(hit, "%c", note[i-1]);
      delete1(note, i);
      return;
    }
  }
}


/* Extracts contiguous characters in "hits" until no more are found.
   There may be more later. */

Static void extractContiguous(Char *note, Char *hits, Char *hit)
{
  short i, l, len;

  l = strlen(note);
  len = l;
  *hit = '\0';
  for (i = 1; i <= l; i++) {
    if (pos1(note[i-1], hits) > 0) {
      do {
	if (pos1(note[i-1], hits) == 0)
	  return;
	sprintf(hit + strlen(hit), "%c", note[i-1]);
	delete1(note, i);
	len--;
      } while (len >= i);
      return;
    }
  }
}


/* Extracts the specified character and everything after it. */

Static void extractAfter(Char *note, Char delim, Char *tail)
{
  short newlen;

  newlen = pos1(delim, note);
  *tail = '\0';
  if (newlen == 0)
    return;
  newlen--;
  strcpy(tail, note);
  predelete(tail, newlen);
  note[(long)((int)newlen)] = '\0';
/* p2c: notes.pas, line 109:
 * Note: Modification of string length may translate incorrectly [146] */
}


/* Extracts the dot shortcut part of a note: comma shortcut is no problem
   because a comma cannot be part of a number. */

Static void extractDotShortcut(Char *note, Char *tail)
{
  Char names[256], tail2[256];
  short l = 1;
  short lt;
  Char ch;

  extractAfter(note, '.', tail);
  lt = strlen(tail);
  if (l < lt && tail[1] == '.')
    l = 2;
  if (solfaNoteNames())
    strcpy(names, solfa_names);
  else
    strcpy(names, has_duration);
  if (l < lt && pos1(tail[l], names) > 0) {
    translateSolfa(&tail[l]);
    return;
  }
  if (l == 2)
    error("\"..\" followed by non-note", print);
  if (l >= lt) {
    strcat(note, tail);
    *tail = '\0';
    return;
  }
  ch = tail[0];
  predelete(tail, 1);
  extractDotShortcut(tail, tail2);
  sprintf(note + strlen(note), "%c%s", ch, tail);
  strcpy(tail, tail2);
}


/* Extracts a signed number. */

Static void extractSignedNumber(Char *note, Char *number)
{
  short k;
  Char note0[256];

  k = pos1('+', note);
  if (k == 0)
    k = pos1('-', note);
  *number = '\0';
  if (k == 0)
    return;
  strcpy(note0, note);
  do {
    sprintf(number + strlen(number), "%c", note[k-1]);
    delete1(note, k);
  } while (k <= strlen(note) &&
	   (note[k-1] == '0' || pos1(note[k-1], digits) != 0));
  if (strlen(number) == 1) {
    strcpy(note, note0);
    *number = '\0';
  }
}


/* Local variables for extractGroup: */
struct LOC_extractGroup {
  Char *group;
  short gl, k;
  boolean probe, nonumber;
} ;

Local void tryMore(struct LOC_extractGroup *LINK)
{
  while (LINK->k <= LINK->gl && LINK->group[LINK->k-1] == LINK->group[0])
    LINK->k++;
}

Local void try_(Char *s, struct LOC_extractGroup *LINK)
{
  LINK->probe = (LINK->k < LINK->gl && pos1(LINK->group[LINK->k-1], s) > 0);
  if (LINK->probe)
    LINK->k++;
}

Local void tryNumber(struct LOC_extractGroup *LINK)
{
  boolean dot_ = false;

  LINK->nonumber = true;
  while (LINK->k <= LINK->gl && pos1(LINK->group[LINK->k-1], digitsdot) > 0) {
    LINK->k++;
    if (LINK->group[LINK->k-1] == '.') {
      if (dot_)
	error("Extra dot in number", print);
      else
	dot_ = true;
    } else
      LINK->nonumber = false;
  }
}


/* Extracts a symbol followed by optional +- or <> shift indicators */

Static void extractGroup(Char *note, Char delim, Char *group_)
{
  struct LOC_extractGroup V;
  short k0;
  Char tail[256];

  V.group = group_;
  extractAfter(note, delim, V.group);
  if (*V.group == '\0')
    return;
  V.gl = strlen(V.group);
  V.k = 2;
  if (V.gl > 1 && V.group[1] == ':')
    V.k = 3;
  else {
    tryMore(&V);
    k0 = V.k;
    try_("+-<>", &V);
    if (V.probe)
      tryNumber(&V);
    if (V.nonumber)
      V.k = k0;
    k0 = V.k;
    try_("+-<>", &V);
    if (V.probe)
      tryNumber(&V);
    if (V.nonumber)
      V.k = k0;
  }
  strcpy(tail, V.group);
  V.k--;
  V.group[(long)((int)V.k)] = '\0';
/* p2c: notes.pas, line 170:
 * Note: Modification of string length may translate incorrectly [146] */
  predelete(tail, V.k);
  strcat(note, tail);
}


Static void parseNote(Char *note_, parsedNote *pnote)
{
  Char note[256];
  Char onlymidi[256];
  Char STR1[256];
  Char STR2[10];

  strcpy(note, note_);
  *pnote->shortcut = '\0';
  *pnote->xtuplet = '\0';
  *pnote->accidental = '\0';
  *pnote->dotgroup = '\0';
  *pnote->duration = '\0';
  *pnote->octave = '\0';
  *onlymidi = '\0';
  extractFirst(note, &pnote->name);
  extractAfter(note, 'x', pnote->xtuplet);
  extractAfter(note, ',', pnote->shortcut);
  if (*pnote->shortcut == '\0')
    extractDotShortcut(note, pnote->shortcut);
  if (pnote->name != rest) {
    extractGroup(note, 's', pnote->accidental);
    if (*pnote->accidental == '\0')
      extractGroup(note, 'f', pnote->accidental);
    if (*pnote->accidental == '\0')
      extractGroup(note, 'n', pnote->accidental);
  }
  /* Look for 'i' or 'c' anywhere in what is left of note.*/
  if (*pnote->accidental != '\0') {
    extractOneOf(note, "ic", onlymidi);
    strcat(pnote->accidental, onlymidi);
/* p2c: notes.pas, line 192:
 * Note: Possible string truncation in assignment [145] */
  }
  extractGroup(note, 'd', pnote->dotgroup);
  if (pnote->name == rest)
    extractSignedNumber(note, pnote->accidental);
  extractOneOf(note, durations, pnote->duration);
  sprintf(STR1, "%c", rest);
  if (strcmp(note, STR1))
    extractContiguous(note, "=+-", pnote->octave);
  if (*note != '\0' && isdigit(note[0])) {
    sprintf(STR2, "%c%s", note[0], pnote->octave);
    strcpy(pnote->octave, STR2);
/* p2c: notes.pas, line 198:
 * Note: Possible string truncation in assignment [145] */
    delete1(note, 1);
  }
  strcpy(pnote->whatever, note);
/* p2c: notes.pas, line 200:
 * Note: Possible string truncation in assignment [145] */
}


/* On input: "note" is a note word; "dur1" is the default duration.
   On output: "note" has possibly been modified;
     possibly been split into two parts, the second being "shortcut";
     "dur" is the suggested new default duration;
     "count" is the count of the total of "note" and "shortcut" */
void processNote(Char *note, Char *xnote, Char dur1, Char *dur, short *count)
{
  Char sc[3], origdur[3];
  short multiplicity, l;
  parsedNote pnote;
  Char STR1[36];
  Char STR3[256];

  *xnote = '\0';
  *dur = dur1;
  if (*note == '\0' || !isNoteOrRest(note) || isPause(note))
    return;
  parseNote(note, &pnote);
  if (debugMode()) {
    printf("%s => ", note);
    printNote(pnote);
  }
  if (pos1('.', pnote.whatever) > 0) {
    sprintf(STR3, "Suspicious dot in word %s", note);
    warning(STR3, print);
  }
  strcpy(origdur, pnote.duration);
  if (*pnote.duration == '\0')
    *dur = dur1;
  else
    *dur = pnote.duration[0];
  *count = count64[*dur - '0'];
  if (*pnote.dotgroup != '\0') {
    *count += *count / 2;
    if (startsWith(pnote.dotgroup, "dd"))
      *count += *count / 6;
  }
  sprintf(pnote.duration, "%c", *dur);
  if (*pnote.shortcut != '\0') {
    if (*pnote.dotgroup != '\0')
      error("You may not explicitly dot a note with a shortcut", print);
    sprintf(sc, "%c", pnote.shortcut[0]);
    predelete(pnote.shortcut, 1);
    if (!strcmp(sc, ".")) {
      multiplicity = 1;
      if (pnote.shortcut[0] == '.') {
	multiplicity++;
	predelete(pnote.shortcut, 1);
	strcat(sc, ".");
/* p2c: notes.pas, line 234:
 * Note: Possible string truncation in assignment [145] */
      }
      *count += *count;
      dur1 = pnote.duration[0];
      for (l = 1; l <= multiplicity; l++) {
	sprintf(pnote.dotgroup + strlen(pnote.dotgroup), "%c", dotcode);
/* p2c: notes.pas, line 237:
 * Note: Possible string truncation in assignment [145] */
	dur1 = half(dur1);
      }
      addDuration(pnote.shortcut, dur1);
    } else {
      addDuration(pnote.shortcut, half(pnote.duration[0]));
      *count += *count / 2;
    }
  }
  if (!insertDuration()) {
    strcpy(pnote.duration, origdur);
/* p2c: notes.pas, line 244:
 * Note: Possible string truncation in assignment [145] */
  }
  if (rearrangeNote())
    sprintf(note, "%c%s%s%s%s%s%s",
	    pnote.name, pnote.duration, pnote.octave, pnote.whatever,
	    pnote.accidental, pnote.dotgroup, pnote.xtuplet);
  else
    strcpy(pnote.shortcut, " ");
  if (!insertDuration() && *pnote.shortcut != '\0') {
    sprintf(STR1, "%s%s", sc, pnote.shortcut);
    strcpy(pnote.shortcut, STR1);
/* p2c: notes.pas, line 249:
 * Note: Possible string truncation in assignment [145] */
  }
  strcpy(xnote, pnote.shortcut);
}


Char octaveCode(Char *note)
{  /*if debugMode then write('Octave code in note "',note,'" is ');*/
  parsedNote pnote;

  parseNote(note, &pnote);
  /*if debugMode then writeln('"',octave,'"');*/
  if (*pnote.octave == '\0')
    return ' ';
  else
    return (pnote.octave[0]);
}


void removeOctaveCode(Char code, Char *note)
{   /*if debugMode then writeln('remove ',code,' from ',note);*/
  short k, l;

  l = strlen(note);
  for (k = 1; k <= l; k++) {
    if (note[k-1] == code) {
      if (k == l || note[k] < '0' || note[k] > '9') {
	delete1(note, k);
	return;
      }
    }
  }
  fatalerror("Code not found in note");
}


void insertOctaveCode(Char code, Char *note)
{  /*if debugMode then writeln('insert ',code,' into ',note); */
  short l;

  l = strlen(note);
  if (l < 2 || note[1] < '0' || note[1] > '9')
    fatalerror("Trying to insert octave into note without duration");
  if (l <= 2 || note[2] < '0' || note[2] > '9')
    insertChar(code, note, 3);
  else
    printf("Not inserting \"%c\", note already has octave code\"\n", code);
}




/* End. */
