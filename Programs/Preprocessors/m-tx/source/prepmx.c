/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "prepmx.pas" */


#include "p2c.h"


#ifndef CONTROL_H
#include "control.h"
#endif

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef GLOBALS_H
#include "globals.h"
#endif

#ifndef PREAMBLE_H
#include "preamble.h"
#endif

#ifndef LYRICS_H
#include "lyrics.h"
#endif

#ifndef MTX_H
#include "mtx.h"
#endif

#ifndef ANALYZE_H
#include "analyze.h"
#endif

#ifndef MTXLINE_H
#include "mtxline.h"
#endif

#ifndef STATUS_H
#include "status.h"
#endif

#ifndef UPTEXT_H
#include "uptext.h"
#endif

#ifndef NOTES_H
#include "notes.h"
#endif

#ifndef MULTFILE_H
#include "multfile.h"
#endif

#ifndef FILES_H
#include "files.h"
#endif

#ifndef UTILITY_H
#include "utility.h"
#endif


/* CMO: addition/change by Christian Mondrup */

/** M-Tx preprocessor to PMX     Dirk Laurie */

#define version         "0.63a"
#define version_date    "<8 April 2018>"

/** See file "Corrections" for updates */

/** To do next:
{* Current bugs:
{* Old bugs:
   Does not check PMX syntax
{* Weaknesses that may later cause bugs:
   Decisions sometimes made on note, sometimes on scan
   Can 'here' overrun?
   Where does one note stop and the next begin?
   See comment on '!' in uptext
*/

#define chord_flat      't'
#define chord_left      'l'

#define blind           true
#define visible         false


Static boolean last_bar;
Static Char repeat_sign[256];
/*, bars_of_rest*/
Static short bar_of_line;


/* --------------- Bars and rests --------------- */

Static void writeRepeat(Char *bar)
{
  Char repcode[256];
  Char STR1[256];

  if (*bar == '\0')
    return;
  *repcode = '\0';
  if (!strcmp(bar, "||"))
    strcpy(repcode, "Rd");
  else if (!strcmp(bar, "|]"))
    strcpy(repcode, "RD");
  else if (!strcmp(bar, "|:"))
    strcpy(repcode, "Rl");
  else if (!strcmp(bar, ":|:"))
    strcpy(repcode, "Rlr");
  else if (!strcmp(bar, ":|"))
    strcpy(repcode, "Rr");
  else if (last_bar && !strcmp(bar, "|"))
    strcpy(repcode, "Rb");
  if (*repcode != '\0') {
    sprintf(STR1, " %s", repcode);
    putLine(STR1);
  }
  *bar = '\0';
}


Static void supplyRests(voice_index voice)
{
  Char STR2[4];
  Char STR3[256];

  if (bar_of_line == 1 && pedanticWarnings()) {
    printf("Bar %d Voice %d", bar_no, voice);
    warning(" Filling missing voice with rests", !print);
  }
  if (pickup > 0)
    put(rests(STR3, pickup, meterdenom, visible), nospace);
  sprintf(STR2, "%s ", pause);
  put(STR2, putspace);
}


/* Local variables for processLine: */
struct LOC_processLine {
  voice_index voice;
  Char chords[256], note[256], cutnote[256], pretex[256];
  short ngrace, nmulti;
  boolean no_chords, no_uptext;

  short l;
  boolean in_group;
} ;

Local void output(Char *note_, struct LOC_processLine *LINK)
{
  Char note[256];
  Char STR2[256];

  strcpy(note, note_);
  if (*note != '\0')
  {  /* pretex is saved up until a spacing note or rest comes */
    if (endsWith(note, "\\") && note[1] != '\\') {
      curtail(note, '\\');
      strcat(LINK->pretex, note);
      *note = '\0';
      return;
    }
    if (*LINK->pretex != '\0' && isNoteOrRest(note)) {
      sprintf(note, "%s\\ %s", LINK->pretex, strcpy(STR2, note));
      *LINK->pretex = '\0';
    }
    if (outlen > 0 && note[0] != ' ')
      sprintf(note, " %s", strcpy(STR2, note));
    put(note, nospace);
  }
  if (thisNote(LINK->voice) == nextvoice)
    putLine("");
}

Local void maybeDotted(Char *note, struct LOC_processLine *LINK)
{
  if (strlen(note) < 2)
    return;
  if (note[1] == '"') {
    note[1] = note[0];
    strcat(LINK->pretex, "\\mtxDotted");
  }
}

/* Local variables for addChords: */
struct LOC_addChords {
  struct LOC_processLine *LINK;
  Char nt[256], labels[256];
  boolean tieme;
  Char lab;
  int5 pitches;
} ;

Local void outChord(struct LOC_addChords *LINK)
{
  short k;
  Char STR2[256];
  short FORLIM;

  if (*LINK->nt == '\0')
    return;
  LINK->tieme = (LINK->nt[0] == '{');
  if (LINK->tieme)
    predelete(LINK->nt, 1);
  if (*LINK->nt == '\0' || LINK->nt[0] < 'a' || LINK->nt[0] > 'g') {
    sprintf(STR2, "Must have a note name at the start of \"%s\", not \"%c\"",
	    LINK->nt, LINK->nt[0]);
    error(STR2, print);
  }
  renewChordPitch(LINK->LINK->voice, LINK->nt);
  sprintf(LINK->LINK->chords + strlen(LINK->LINK->chords), " z%s", LINK->nt);
  *LINK->nt = '\0';
  FORLIM = strlen(LINK->labels);
  for (k = 0; k <= FORLIM - 1; k++) {
    if (chordPitch(LINK->LINK->voice) == LINK->pitches[k]) {
      sprintf(LINK->LINK->chords + strlen(LINK->LINK->chords), " s%ct",
	      LINK->labels[k]);
      LINK->labels[k] = ' ';
    }
  }
  if (LINK->tieme) {
    chordTie(LINK->LINK->voice, &LINK->lab);
    sprintf(LINK->LINK->chords + strlen(LINK->LINK->chords), " s%ct",
	    LINK->lab);
  }
}

Local void addChords(struct LOC_processLine *LINK)
{
  struct LOC_addChords V;
  Char w[256];
  short j = 1;
  short mus_line;
  paragraph_index0 chord_line;
  boolean arpeggio;
  Char STR1[256];
  short FORLIM;

  V.LINK = LINK;
  saveStatus(LINK->voice);
  getChordTies(LINK->voice, V.pitches, V.labels);
  *LINK->chords = '\0';
  chord_line = chordLineNo(LINK->voice);
  if (chord_line == 0)
    LINK->no_chords = true;
  if (LINK->no_chords)
    return;
  GetNextWord(w, P[chord_line-1], blank, dummy);
  sprintf(STR1, "%c", barsym);
  if (!strcmp(w, STR1) || *w == '\0')
    LINK->no_chords = true;
  sprintf(STR1, "%c", tilde);
  if (!strcmp(w, STR1) || LINK->no_chords)
    return;
  mus_line = line_no;
  line_no = orig_line_no[chord_line-1];
  arpeggio = (w[0] == '?');
  if (arpeggio) {
    strcpy(LINK->chords, " ?");
    predelete(w, 1);
  }
  *V.nt = '\0';
  while (j <= strlen(w)) {
    if (w[j-1] == '{' || strcmp(V.nt, "{") && w[j-1] >= 'a' && w[j-1] <= 'g')
      outChord(&V);
    else if (w[j-1] == chord_flat)
      w[j-1] = 'f';
    else if (w[j-1] == chord_left)
      w[j-1] = 'e';
    sprintf(V.nt + strlen(V.nt), "%c", w[j-1]);
    j++;
  }
  if (*V.nt != '\0')
    outChord(&V);
  if (arpeggio)
    strcat(LINK->chords, " ?");
  strcat(LINK->note, LINK->chords);
  FORLIM = strlen(V.labels);
  for (j = 0; j <= FORLIM - 1; j++) {
    if (V.labels[j] != ' ')
      error("Tie on chord note started but not finished", print);
  }
  line_no = mus_line;
}

Local Char *processOther(Char *Result, Char *note_,
			 struct LOC_processLine *LINK)
{
  Char note[256];

  strcpy(note, note_);
  switch (thisNote(LINK->voice)) {

  case other:
    if (note[0] == grace_group) {
      if (strlen(note) == 1)
	LINK->ngrace = 1;
      else
	LINK->ngrace = pos1(note[1], digits);
      if (LINK->ngrace > 0)
	LINK->ngrace--;
    }
    break;

  /* For a zword, take note of pitch but do not change the contents */
  /* Add spurious duration because repitch expects duration to be present */
  case zword:
    strcpy(LINK->cutnote, note);
    predelete(LINK->cutnote, 1);
    insertChar('4', LINK->cutnote, 2);
    checkOctave(LINK->voice, LINK->cutnote);
    renewPitch(LINK->voice, LINK->cutnote);
    break;

  case lyrtag:
    extractLyrtag(LINK->voice, note);
    break;

  case rbrac:
    endBeam(LINK->voice);
    break;

  case rparen:
  case rlparen:
    endSlur(LINK->voice, note);
    break;

  case lbrac:
    beginBeam(LINK->voice, note);
    break;

  case lparen:
    maybeDotted(note, LINK);
    beginSlur(LINK->voice, note);
    break;

  case mword:
    error3(LINK->voice, "Meter change must be the first word of its bar");
    break;

  case atword:
    lyricsAdjust(LINK->voice, note);
    break;
  }
  return strcpy(Result, note);
}

Local void lookahead(struct LOC_processLine *LINK)
{
  Char STR2[256];
  Char STR3[256];

  while (bind_left[(long)nextNote(LINK->voice)])
    sprintf(LINK->note + strlen(LINK->note), " %s",
	    processOther(STR2, getMusicWord(STR3, LINK->voice), LINK));
}

Local void processUsual(struct LOC_processLine *LINK)
{
  if (hasVerseNumber(LINK->voice))
    strcat(LINK->pretex, "\\mtxVerse");
  LINK->l = pos1(multi_group, LINK->note);
  if (LINK->l > 0)
    scan1(LINK->note, LINK->l + 1, &LINK->nmulti);
  activateBeamsAndSlurs(LINK->voice);
  LINK->in_group = false;
  if (LINK->ngrace > 0) {
    LINK->in_group = true;
    LINK->ngrace--;
  } else {
    if (LINK->nmulti > 0) {
      LINK->in_group = true;
      LINK->nmulti--;
    }
  }
  checkOctave(LINK->voice, LINK->note);
  renewPitch(LINK->voice, LINK->note);
  if (!LINK->in_group) {
    resetDuration(LINK->voice, durationCode(LINK->note));
    markDebeamed(LINK->voice, LINK->note);
  }
  lookahead(LINK);
  getSyllable(LINK->voice, LINK->pretex);
  addUptext(LINK->voice, &LINK->no_uptext, LINK->pretex);
  addChords(LINK);
}


/* ---------------------------------------------------------------- */

Static void processLine(voice_index voice_, short bar_no)
{
  struct LOC_processLine V;
  paragraph_index par_line;
  Char STR1[256];

  V.voice = voice_;
  *V.pretex = '\0';
  V.no_chords = false;
  V.no_uptext = false;
  par_line = musicLineNo(V.voice);
  V.nmulti = 0;
  V.ngrace = 0;
  line_no = orig_line_no[par_line-1];
  do {
    getMusicWord(V.note, V.voice);
    if (*V.note == '\0')
      return;
    /* if debugMode then writeln(voice,' ',note); */
    switch (thisNote(V.voice)) {

    case rword:
      if (*multi_bar_rest != '\0') {
	if (uptextOnRests())
	  addUptext(V.voice, &V.no_uptext, V.pretex);
      } else {
	V.l = pos1(multi_group, V.note);
	if (V.l > 0)
	  scan1(V.note, V.l + 1, &V.nmulti);
	if (V.nmulti > 0) {
	  V.in_group = true;
	  V.nmulti--;
	}
	if (uptextOnRests())
	  addUptext(V.voice, &V.no_uptext, V.pretex);
	if (!(isPause(V.note) || V.in_group))
	      /*0.63: allow rests in xtuples*/
		resetDuration(V.voice, durationCode(V.note));
      }
      break;

    case abcdefg:
      processUsual(&V);
      break;

    case barword:
      if (V.voice == nvoices) {
	if (endOfBar(V.voice, bar_no))
	  strcpy(repeat_sign, V.note);
	else
	  writeRepeat(V.note);
      }
      sprintf(STR1, "%c", barsym);
      if (strcmp(V.note, STR1))
	*V.note = '\0';
      V.no_chords = false;
      break;

    case FirstOnly:
      if (V.voice != nvoices)
	*V.note = '\0';
      else
	strcpy(V.note, processOther(STR1, V.note, &V));
      break;

    default:
      strcpy(V.note, processOther(STR1, V.note, &V));
      break;
    }
    output(V.note, &V);
  } while (!endOfBar(V.voice, bar_no));
  if (!V.no_chords)
    skipChordBar(V.voice);
}


/* ------------------------------------------------------------------- */

Static void getMeterChange(voice_index voice, Char *new_meter)
{
  short pn1, pn2;
  Char w[256], new_command[256];

  if (nextNote(voice) != mword)
    return;
  getMusicWord(w, voice);
  getMeter(w, &meternum, &meterdenom, &pn1, &pn2);
  full_bar = meternum * (64 / meterdenom);
  /* CMO: process denominator value with function PMXmeterdenom */
  meterWord(new_command, meternum, PMXmeterdenom(meterdenom), pn1, pn2);
  if (*new_meter != '\0' && strcmp(new_meter, new_command))
    error3(voice, "The same meter change must appear in all voices");
  strcpy(new_meter, new_command);
}


/* Local variables for musicParagraph: */
struct LOC_musicParagraph {
  Char new_meter[256];
} ;

Local void putPMXlines(struct LOC_musicParagraph *LINK)
{
  paragraph_index i, FORLIM;

  FORLIM = para_len;
  for (i = 0; i <= FORLIM - 1; i++) {
    if (startsWith(P[i], double_comment)) {
      predelete(P[i], 2);
      putLine(P[i]);
      strcpy(P[i], "%");
    }
  }
}

Local void processOneBar(struct LOC_musicParagraph *LINK)
{
  paragraph_index0 m, cm;
  voice_index voice, cvoice;
  boolean ignore_voice;
  boolean wrote_repeat = false;
  boolean alone;
  Char STR2[256];
  Char STR3[256];

  if (bar_of_line > 1) {
    sprintf(STR3, "%cBar %s", comment, toString(STR2, bar_no));
    putLine(STR3);
  }
  last_bar = (bar_of_line == nbars && final_paragraph);
  if (last_bar && !strcmp(repeat_sign, "|"))
    *repeat_sign = '\0';
  writeRepeat(repeat_sign);
  *LINK->new_meter = '\0';
  for (voice = nvoices; voice >= 1; voice--) {
    if (musicLineNo(voice) > 0) {
      gotoBar(voice, bar_of_line);
      getMeterChange(voice, LINK->new_meter);
    }
  }
  if (last_bar && *LINK->new_meter == '\0' && nleft > pickup && meternum > 0)
    meterChange(LINK->new_meter, nleft, 64, true);
  if (*LINK->new_meter != '\0')
    putLine(LINK->new_meter);
  for (voice = nvoices; voice >= 1; voice--) {
    ignore_voice = !selected[voice-1];
    cvoice = companion(voice);
    m = musicLineNo(voice);
    cm = musicLineNo(cvoice);
    alone = (voice == cvoice || m > 0 && cm == 0 ||
	     m == 0 && cm == 0 && voice < cvoice || !selected[cvoice-1]);
    if (selected[voice-1]) {
      if (m > 0)
	processLine(voice, bar_of_line);
      else if (alone)
	supplyRests(voice);
      else
	ignore_voice = true;
      if (last_bar && *repeat_sign != '\0' && !wrote_repeat) {
	writeRepeat(repeat_sign);
	wrote_repeat = true;
      }
      if (!ignore_voice) {
	if (alone || voicePos(voice) == 1)
	  putLine(" /");
	else
	  putLine(" //");
      }
    }
  }
  bar_no++;
  pickup = 0;
  putLine("");
}

Local void putMeter(Char *new_meter_word, struct LOC_musicParagraph *LINK)
{
  if (strcmp(new_meter_word, old_meter_word))
    putLine(new_meter_word);
  strcpy(old_meter_word, new_meter_word);
}

Local void processMBR(struct LOC_musicParagraph *LINK)
{
  short s, bars_of_rest;
  Char mbr[256];
  short FORLIM;
  Char STR1[256];
  Char STR3[256];

  strcpy(mbr, multi_bar_rest);
  predelete(mbr, 2);
  getNum(mbr, &bars_of_rest);
  bar_no += bars_of_rest;
  FORLIM = nstaves;
  for (s = 1; s <= FORLIM; s++) {
    if (pickup > 0)
      put(rests(STR1, pickup, meterdenom, visible), nospace);
    sprintf(STR3, "rm%s /", toString(STR1, bars_of_rest));
    putLine(STR3);
  }
  putLine("");
}


Static void musicParagraph(void)
{
  struct LOC_musicParagraph V;
  voice_index0 j, nvoice;
  Char lyrassign[256];
  Char STR1[256];
  Char STR2[256];
  Char STR3[256], STR4[256], STR5[256];
  voice_index0 FORLIM;
  short FORLIM1;

  paragraphSetup(&nvoice);
  if (nvoice == 0) {
    nonMusic();
    return;
  }
  if (nvoice > nvoices) {
    if (nvoice == 0)
      error("No voices! Did you remember to to supply a Style?", !print);
    sprintf(STR4, "Paragraph has %s voices but Style allows only %s",
	    toString(STR1, nvoice), toString(STR2, nvoices));
    error(STR4, !print);
    return;
  }
  if (first_paragraph)
    includeStartString();
  if (pmx_preamble_done && (!final_paragraph || nvoice > 0)) {
    sprintf(STR5, "%c Paragraph %s line %s bar %s",
	    comment, toString(STR1, paragraph_no),
	    toString(STR2, orig_line_no[0]), toString(STR3, bar_no));
    putLine(STR5);
  }
  testParagraph();
  rememberDurations();
  if (beVerbose())
    describeParagraph();
  /* ---- Knowing the score, we can start setting music ---------------- */
  if (!pmx_preamble_done) {
    doPMXpreamble();
    sprintf(STR3, "%c Paragraph %s line %s bar ",
	    comment, toString(STR1, paragraph_no),
	    toString(STR2, orig_line_no[0]));
    put(STR3, putspace);
    if (pickup > 0)
      putLine("0");
    else
      putLine("1");
  }
  putPMXlines(&V);
  if (must_restyle)
    restyle();
  if (some_vocal && (nvoice > 0 || !final_paragraph)) {
    FORLIM = ninstr;
    for (j = 1; j <= FORLIM; j++) {
      assignLyrics(j, lyrassign);
      if (*lyrassign != '\0') {
	sprintf(STR2, "\\\\%s\\", lyrassign);
	putLine(STR2);
      }
    }
  }
  if (must_respace)
    respace();
  if (meternum == 0)
    putMeter(meterChange(STR2, beatsPerLine(), meterdenom, true), &V);
  if (nleft > 0)
    nbars++;
  if (nbars == 0 && *multi_bar_rest != '\0')
    processMBR(&V);
  else {
    FORLIM1 = nbars;
    for (bar_of_line = 1; bar_of_line <= FORLIM1; bar_of_line++)
      processOneBar(&V);
  }
  restoreDurations();
}


/* ----------------------------------------------------------------------- */

Static boolean no_commands_yet;


Static void doMusic(void)
{
  Char STR1[256], STR2[256];
  Char STR4[256];

  first_paragraph = true;
  pmx_preamble_done = false;
  bar_no = 1;
  *repeat_sign = '\0';
  must_respace = false;
  must_restyle = false;
  do {
    final_paragraph = endOfInfile();
    memcpy(orig_P, P, sizeof(paragraph));
    if (para_len > 0 && !ignore_input && thisCase()) {
      if (no_commands_yet) {
	interpretCommands();
	printFeatures(false);
	one_beat = 64 / meterdenom;
	full_bar = meternum * one_beat;
	if (nvoices > standardPMXvoices) {
	  sprintf(STR4, "You have %s voices; standard PMX can only handle %s",
		  toString(STR1, nvoices), toString(STR2, standardPMXvoices));
	  warning(STR4, !print);
	}
	initMTX();
	initUptext();
	initStatus();
	initLyrics();
	no_commands_yet = false;
      }
      if (startsWithBracedWord(P[0]))
	lyricsParagraph();
      else {
	musicParagraph();
	first_paragraph = false;
	writeRepeat(repeat_sign);
      }
    }
    readParagraph(P, orig_line_no, &para_len);
  } while (para_len != 0);
}


Static boolean control_paragraph, no_report_errors;


Static boolean isControlParagraph(Char (*P)[256], paragraph_index para_len)
{
  paragraph_index0 commands = 0, labels = 0, voices = 0, guff = 0;
  paragraph_index0 i;
  Char w[256];

/* p2c: prepmx.pas: Note: Eliminated unused assignment statement [338] */
  for (i = 0; i <= para_len - 1; i++) {
    if (!startsWith(P[i], "%")) {
      NextWord(w, P[i], ' ', ':');
      if (!endsWith(w, ":"))
	guff++;
      else if (strlen(w) < 3 || findVoice(w) > 0)
	voices++;
      else if (isCommand(w))
	commands++;
      else
	labels++;
    }
  }
  if (voices + guff > commands)
    return false;
  return true;
}


Static void topOfPMXfile(void)
{
  Char STR2[24];
  Char STR3[30];

  putLine("---");
  sprintf(STR2, "\\def\\mtxversion{%s}", version);
  putLine(STR2);
  sprintf(STR3, "\\def\\mtxdate{%s}", version_date);
  putLine(STR3);
  putLine("\\input mtx");
}


main(int argc, Char *argv[])
{  /* ---- Main program ------------------------ */
  PASCAL_MAIN(argc, argv);
  strcpy(this_version, version);
  strcpy(this_version_date, version_date);
  printf("==> This is M-Tx %s (Music from TeXt) %s\n", version, version_date);

  mtxLevel(version);
  OpenFiles();
  no_commands_yet = true;
  preambleDefaults();
  no_report_errors = false;
  topOfPMXfile();
  do {
    readParagraph(P, orig_line_no, &para_len);
    control_paragraph = isControlParagraph(P, para_len);
    if (control_paragraph) {
      augmentPreamble(no_report_errors);
      no_report_errors = true;
      if (para_len == 0)
	error("No music paragraphs!", !print);
    }
  } while (control_paragraph);
  doPreamble();
  doMusic();
  if (!pmx_preamble_done)
    error("No music paragraphs!", !print);
  putLine("% Coded by M-Tx");
  CloseFiles();
  printf("PrePMX done.  Now run PMX.\n");
  _Escape(0);
  exit(EXIT_SUCCESS);
}




/* End. */
