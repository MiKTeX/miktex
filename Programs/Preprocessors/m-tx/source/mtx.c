/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "mtx.pas" */


#include "p2c.h"


#define MTX_G
#include "mtx.h"


#ifndef CONTROL_H
#include "control.h"
#endif

#ifndef MTXLINE_H
#include "mtxline.h"
#endif

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef STATUS_H
#include "status.h"
#endif

#ifndef NOTES_H
#include "notes.h"
#endif


#define flagged         "8136"


typedef boolean sticky['z' + 1 - 'a'];


Static sticky note_attrib[maxvoices], rest_attrib[maxvoices];


Static short logTwo(short denom)
{
  short l = 0;

  while (denom > 1) {
    l++;
    denom /= 2;
  }
  return l;
}


Local boolean dotted(short n)
{
  boolean Result;

/* p2c: mtx.pas: Note: Eliminated unused assignment statement [338] */
  if (n == 0)
    return false;
  if (n != n / 3 * 3)
    return false;
  n /= 3;
  while ((n & 1) == 0)
    n /= 2;
  return (n == 1);
}


/* RESTS should have a third parameter OFFSET.  At present we in effect
   assume OFFSET+LENGTH to be a multiple of METERDENOM. */
Char *rests(Char *Result, short len, short meterdenom, boolean blind)
{
  Char r[256], bl[256];
  Char STR1[256];

  if (blind)
    strcpy(bl, "b");
  else
    *bl = '\0';
  if (len == 0)
    return strcpy(Result, "");
  else {
    if (len == meterdenom * 128) {
      sprintf(Result, "r9%s ", bl);
      return Result;
    } else {
      if (dotted(len)) {
	rests(r, len - len / 3, meterdenom, false);
	r[2] = 'd';
	sprintf(Result, "%s%s ", r, bl);
	return Result;
      } else if (len >= 64 / meterdenom) {
	sprintf(Result, "%sr%c%s ",
		rests(STR1, len - 64 / meterdenom, meterdenom, blind),
		durations[whole + logTwo(meterdenom) - 1], bl);
	return Result;
      } else
	return (rests(Result, len, meterdenom * 2, blind));
    }
  }
}


Local Char *attribs(Char *Result, Char *note)
{
  Char a[256];
  short i = 2;
  short l;
  Char n;

  *a = '\0';
  l = strlen(note);
  while (i <= l) {
    n = note[i-1];
    if (n == 'x')
      i = l;
    else if (islower(n))
      sprintf(a + strlen(a), "%c", n);
    i++;
  }
  return strcpy(Result, a);
}


Static void checkSticky(Char *note, boolean *attrib)
{
  short i = 2;
  short l;
  Char c;
  Char a[256];

  if (*note == '\0')
    return;
  l = strlen(note);
  while (i <= l) {
    c = note[i-1];
    if (islower(c)) {
      if (i < l && note[i] == ':') {
	delete1(note, i + 1);
	l--;
	attrib[c - 'a'] = !attrib[c - 'a'];
      } else
	attrib[c - 'a'] = false;
    }
    i++;
  }
  attribs(a, note);
  for (c = 'z'; c >= 'a'; c--) {
    if (attrib[c - 'a'] && pos1(c, a) == 0)
      insertChar(c, note, 3);
  }
}


/* Routine was patched by Hiroaki Morimoto, revised 2003/08/15.
   Rewritten by DPL 2004/05/16 to be more understandable.
   octaveCode returns octave adjustments in the order
   [absolute or =],[relative] */
void checkOctave(voice_index voice, Char *note)
{
  Char code;

  code = octaveCode(note);
  if (code == '=') {
    setOctave(voice);
    removeOctaveCode(code, note);
  }
  if (octave(voice) == blank)
    return;
  code = octaveCode(note);
  if (isdigit(code)) {
    resetOctave(voice);
    return;
  }
  while (code == '+' || code == '-') {
    newOctave(voice, code);
    removeOctaveCode(code, note);
    code = octaveCode(note);
  }
  if (code != ' ')
    error3(voice, "You may have only one absolute octave assignment");
  insertOctaveCode(octave(voice), note);
  checkRange(voice, note);
  resetOctave(voice);
}


void markDebeamed(voice_index voice, Char *note)
{
  if (isVocal(voice) && afterSlur(voice) == 0 && unbeamVocal() &&
      pos1(duration(voice), flagged) > 0)
    insertChar('a', note, 2);
}


Static short barLength(Char *meter)
{
  short n1, n2, pn1, pn2;

  getMeter(meter, &n1, &n2, &pn1, &pn2);
  return (n1 * (64 / n2));
}


Static boolean isMultiBarRest(Char *rest_)
{
  boolean Result;

/* p2c: mtx.pas: Note: Eliminated unused assignment statement [338] */
  if (strlen(rest_) < 3)
    return false;
  if (rest_[1] != 'm')
    return false;
  if (multi_bar_rest)
    error("Only one multibar rest allowed per line", print);
  multi_bar_rest = true;
  return true;
}


/* Double-length in xtuplet detected by a brute search for D anywhere.
   Could be more elegant. */

Static short macro_length[20];

Static Char macro_ID[20][3] = {
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14",
  "15", "16", "17", "18", "19", "20"
};


/* Local variables for scanMusic: */
struct LOC_scanMusic {
  voice_index voice;
  Char note[256];
  boolean doublex, store_macro;
  short bar, old_length, bar_length, count, l, mlen, ngrace, nmulti, macroID,
	playID;
  Char macro_type;
} ;

Local void decmulti(struct LOC_scanMusic *LINK)
{
  if (LINK->doublex)
    LINK->nmulti -= 2;
  else
    LINK->nmulti--;
}

/* Local variables for countIt: */
struct LOC_countIt {
  struct LOC_scanMusic *LINK;
} ;

Local void incbar(short nl, struct LOC_countIt *LINK)
{
  LINK->LINK->bar_length += nl;
  if (LINK->LINK->bar_length > LINK->LINK->bar && meternum > 0)
    error3(LINK->LINK->voice, "Bar end occurs in mid-note");
}

Local void countIt(struct LOC_scanMusic *LINK)
{
  struct LOC_countIt V;

  V.LINK = LINK;
  if (LINK->ngrace > 0) {
    LINK->ngrace--;
    return;
  }
  if (LINK->nmulti > 0) {
    decmulti(LINK);
    return;
  }
  if (LINK->count % 3 != 0 &&
      (LINK->note[0] != rest && note_attrib[LINK->voice-1]['d' - 'a'] ||
       LINK->note[0] == rest && rest_attrib[LINK->voice-1]['d' - 'a']))
    LINK->count += LINK->count / 2;
/* p2c: mtx.pas, line 150:
 * Note: Using % for possibly-negative arguments [317] */
  incbar(LINK->count, &V);
  LINK->l = pos1(multi_group, LINK->note);
  if (LINK->l <= 0)
    return;
  predelete(LINK->note, LINK->l);
  getNum(LINK->note, &LINK->nmulti);
  decmulti(LINK);
}

Local void maybeGroup(struct LOC_scanMusic *LINK)
{
  if (LINK->note[0] != grace_group)
    return;
  if (strlen(LINK->note) == 1)
    LINK->ngrace = 1;
  else
    LINK->ngrace = pos1(LINK->note[1], digits);
  /* bug if ngrace>9 */
  if (LINK->ngrace > 0)
    LINK->ngrace--;
}

Local short identifyMacro(Char *s_, struct LOC_scanMusic *LINK)
{
  Char s[256];
  short k;

  strcpy(s, s_);
  predelete(s, 2);
  for (k = 1; k <= 20; k++) {
    if (!strcmp(s, macro_ID[k-1]))
      return k;
  }
  return 0;
}

Local void examineMacro(struct LOC_scanMusic *LINK)
{
  if (!countMacro())
    return;
  if (debugMode())
    printf("%s: ", LINK->note);
  if (strlen(LINK->note) == 1) {
    LINK->mlen = LINK->bar_length - LINK->old_length;
    if (LINK->macroID < 1 || LINK->macroID > 20)
      error3(LINK->voice, "Invalid macro ID");
    macro_length[LINK->macroID-1] = LINK->mlen;
    if (debugMode())
      printf("Ending macro %d of type %c, length=%d\n",
	     LINK->macroID, LINK->macro_type, LINK->mlen);
    if (LINK->macro_type == 'S')
      LINK->bar_length = LINK->old_length;
    LINK->store_macro = false;
    return;
  }
  LINK->playID = identifyMacro(LINK->note, LINK);
  if (LINK->playID < 1 || LINK->playID > 20)
    error3(LINK->voice, "Invalid macro ID");
  if (LINK->note[1] == 'P') {
    if (debugMode())
      printf("Playing macro %d of length %d\n",
	     LINK->playID, macro_length[LINK->playID-1]);
    LINK->bar_length += macro_length[LINK->playID-1];
    return;
  }
  if (pos1(LINK->note[1], "SR") <= 0)
    return;
  LINK->old_length = LINK->bar_length;
  LINK->macro_type = LINK->note[1];
  LINK->macroID = LINK->playID;
  LINK->store_macro = true;
  if (debugMode())
    printf("Defining macro %d of type %c\n", LINK->macroID, LINK->note[1]);
}


void scanMusic(voice_index voice_, short *left_over)
{
  struct LOC_scanMusic V;
  Char buf[256], enote[256], xnote[256];
  boolean has_next = false, done = false;
  Char dur1, lastdur;
  music_word nscan;
  Char STR1[256];

  V.voice = voice_;
  strcpy(terminators, "d.x");
  resetInfo(V.voice, buf);
  *left_over = 0;
  V.store_macro = false;
  V.bar = full_bar;
  V.bar_length = 0;
  V.ngrace = 0;
  V.nmulti = 0;
  if (meternum == 0)
    V.bar = 32000;
  dur1 = duration(V.voice);
  lastdur = dur1;
  do {
    getNextMusWord(buf, V.note, &nscan);
    if (*V.note == '\0')
      break;
    V.count = 0;
    /*    if isNoteOrRest(note) and not (isPause(note) or isMultibarRest(note))
          then note:=toStandard(note); */
    V.doublex = (pos1('D', V.note) > 0);
    if (nscan == mword) {
      if (*V.note == '\0')
	error3(V.voice, "You may not end a line with a meter change");
      if (V.bar_length > 0)
	error3(V.voice, "Meter change only allowed at start of bar");
      else
	V.bar = barLength(V.note);
    } else if (nscan == rword) {
      if (!(isPause(V.note) || isMultiBarRest(V.note))) {
	processNote(V.note, xnote, dur1, &lastdur, &V.count);
	checkSticky(V.note, rest_attrib[V.voice-1]);
      }
    }
    if (*V.note != '\0')
      appendNote(V.voice, nscan);
    strcpy(enote, V.note);
    if (nscan == macro || nscan == endmacro)
      examineMacro(&V);
    if (nscan == abcdefg) {
      if (!multi_bar_rest && V.ngrace + V.nmulti == 0) {
	processNote(enote, xnote, dur1, &lastdur, &V.count);
	if (*xnote != '\0') {
	  checkSticky(enote, note_attrib[V.voice-1]);
	  appendToLine(V.voice, enote);
	  appendNote(V.voice, nscan);
	  strcpy(enote, xnote);
	}
	checkSticky(enote, note_attrib[V.voice-1]);
      }
    }
    appendToLine(V.voice, enote);
    if (*V.note == '\0')   /* !!! else word_bound[here]:=length(line); */
      done = true;
    sprintf(STR1, "%c", barsym);
    if (!strcmp(V.note, STR1)) {
      if (meternum == 0)
	error3(V.voice, "You may not use bar lines in barless music");
      else if (V.bar_length == 0)
	markBar(V.voice);
      else if (numberOfBars(V.voice) == 0 && V.bar_length < V.bar) {
	if (has_next)
	  has_next = false;   /*Should check whether pickups are equal*/
	else if (*left_over > 0)
	  error3(V.voice, "Bar is too short");
	*left_over = V.bar_length;
	V.bar_length = 0;
      }
    }
    if (nscan == nextvoice) {
      if (V.bar_length > 0)
	error3(V.voice, "Next voice before bar is full");
      else
	barForward(V.voice, -1);
      has_next = true;
    } else if (isPause(V.note))
      V.bar_length += V.bar;
    else if (!multi_bar_rest) {   /*do nothing*/
      if (!done && isNoteOrRest(V.note))
	countIt(&V);
      else
	maybeGroup(&V);
    }
    dur1 = lastdur;
    if (V.bar_length >= V.bar && V.ngrace + V.nmulti == 0 && !V.store_macro) {
      if (debugMode())
	printf("%d %d\n", V.voice, V.bar_length);
      barForward(V.voice, V.bar_length / V.bar);
      V.bar_length %= V.bar;
/* p2c: mtx.pas, line 268:
 * Note: Using % for possibly-negative arguments [317] */
    }
  } while (!done);
  setExtraLength(V.voice, V.bar_length);
  resetDuration(V.voice, dur1);
  regroup(V.voice);
}


void initMTX(void)
{
  voice_index i;
  Char j;

  for (i = 0; i <= maxvoices - 1; i++) {
    for (j = 'a'; j <= 'z'; j++)
      note_attrib[i][j - 'a'] = false;
  }
  memcpy(rest_attrib, note_attrib, maxvoices * sizeof(sticky));
}




/* End. */
