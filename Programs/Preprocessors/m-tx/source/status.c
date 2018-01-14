/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "status.pas" */


#include "p2c.h"


#define STATUS_G
#include "status.h"


#ifndef GLOBALS_H
#include "globals.h"
#endif

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef MTXLINE_H
#include "mtxline.h"
#endif

#ifndef CONTROL_H
#include "control.h"
#endif

#ifndef UTILITY_H
#include "utility.h"
#endif

#ifndef NOTES_H
#include "notes.h"
#endif


#define lowest_pitch    (-9)
#define highest_pitch   61


typedef struct line_status {
  short pitch, chord_pitch, octave_adjust, beam_level, slur_level, after_slur;
  Char octave, lastnote, chord_lastnote, duration, slurID, tieID;
  boolean beamnext, beamed, slurnext, slurred, no_beam_melisma;
  boolean no_slur_melisma[12];
  int5 chord_tie_pitch;
  Char chord_tie_label[6];
} line_status;


Static Char init_oct[256] = "";

Static Char range_name[56][256] = {
  "0c", "0d", "0e", "0f", "0g", "0a", "0b", "1c", "1d", "1e", "1f", "1g",
  "1a", "1b", "2c", "2d", "2e", "2f", "2g", "2a", "2b", "3c", "3d", "3e",
  "3f", "3g", "3a", "3b", "4c", "4d", "4e", "4f", "4g", "4a", "4b", "5c",
  "5d", "5e", "5f", "5g", "5a", "5b", "6c", "6d", "6e", "6f", "6g", "6a",
  "6b", "7c", "7d", "7e", "7f", "7g", "7a", "7b"
/* p2c: status.pas, line 76: 
 * Note: Line breaker spent 0.0 seconds, 5000 tries on line 58 [251] */
};

Static line_status current[maxvoices];
Static Char lastdur[maxvoices];
Static Char voice_range[maxvoices][256], range_low[maxvoices][256],
	    range_high[maxvoices][256];


/* Range limits are user-specified as e.g. 3c. To make comparisons
meaningful, a and b (which come after g) are translated to h and i. */

void checkRange(short voice, Char *note_)
{
  Char note[256];
  Char orig_note[256];
  Char STR1[4];
  Char STR3[256];

  strcpy(note, note_);
  if (*voice_range[voice-1] == '\0')
    return;
  strcpy(orig_note, note);
  if (strlen(note) > 2) {   /* assume usual PMX form with octave */
    sprintf(STR1, "%c%c", note[2], note[0]);
    strcpy(note, STR1);
  }
  if (note[1] == 'a')
    note[1] = 'h';
  if (note[1] == 'b')
    note[1] = 'i';
  if (strcmp(note, range_low[voice-1]) < 0 ||
      strcmp(note, range_high[voice-1]) > 0) {
    sprintf(STR3, "%s is out of range, specified as %s",
	    orig_note, voice_range[voice-1]);
    error3(voice, STR3);
  }
}


void defineRange(short voice, Char *range_)
{
  Char range[256];
  Char STR2[256];

  strcpy(range, range_);
  strcpy(voice_range[voice-1], range);
  if (*range == '\0')
    return;
  if ('0' > range[0] || range[0] > '7' || 'a' > range[1] || range[1] > 'g' ||
      range[2] != '-' || '0' > range[3] || range[3] > '7' || 'a' > range[4] ||
      range[4] > 'g') {
    sprintf(STR2,
	    "Badly formatted range \"%s\" for voice %s, must be e.g. 3c-4a",
	    range, voice_label[voice-1]);
    error(STR2, print);
  }
  if (range[1] == 'a')
    range[1] = 'h';
  if (range[1] == 'b')
    range[1] = 'i';
  if (range[4] == 'a')
    range[4] = 'h';
  if (range[4] == 'b')
    range[4] = 'i';
  substr_(range_low[voice-1], range, 1, 2);
  substr_(range_high[voice-1], range, 4, 2);
}


void chordTie(short voice, Char *lab)
{
  short n;
  line_status *WITH;

  WITH = &current[voice-1];
  n = strlen(WITH->chord_tie_label);
  if (n == 5)
    error3(voice, "Only five slur ties allowed per voice");
  if (n == 0)
    *lab = 'T';
  else
    *lab = WITH->chord_tie_label[n-1];
  (*lab)++;
  sprintf(WITH->chord_tie_label + strlen(WITH->chord_tie_label), "%c", *lab);
/* p2c: status.pas, line 115:
 * Note: Possible string truncation in assignment [145] */
  n++;
  WITH->chord_tie_pitch[n-1] = WITH->chord_pitch;
}


void getChordTies(short voice, short *pitches, Char *labels)
{
  line_status *WITH;

  WITH = &current[voice-1];
  memcpy(pitches, WITH->chord_tie_pitch, sizeof(int5));
  strcpy(labels, WITH->chord_tie_label);
  *WITH->chord_tie_label = '\0';
}


void rememberDurations(void)
{
  voice_index v, FORLIM;

  FORLIM = nvoices;
  for (v = 1; v <= FORLIM; v++)
    lastdur[v-1] = duration(v);
}


void restoreDurations(void)
{
  voice_index v, FORLIM;

  FORLIM = nvoices;
  for (v = 1; v <= FORLIM; v++)
    resetDuration(v, lastdur[v-1]);
}


Char duration(short voice)
{
  return (current[voice-1].duration);
}


void resetDuration(short voice, Char dur)
{
  if (pos1(dur, durations) == 0) {
    printf("Trying to set duration to %c; ", dur);
    error3(voice, "M-Tx system error: resetDuration");
  }
  current[voice-1].duration = dur;
}


void activateBeamsAndSlurs(short voice)
{
  line_status *WITH;

  WITH = &current[voice-1];
  if (WITH->beamnext) {
    WITH->beamed = true;
    WITH->beamnext = false;
  }
  if (WITH->slurnext) {
    WITH->slurred = true;
    WITH->slurnext = false;
  }
  if (WITH->slurred)
    WITH->after_slur++;
}


void saveStatus(short voice)
{
  line_status *WITH;

  WITH = &current[voice-1];
  WITH->chord_pitch = WITH->pitch;
  WITH->chord_lastnote = WITH->lastnote;
}


boolean noBeamMelisma(short voice)
{
  return (current[voice-1].no_beam_melisma);
}


short afterSlur(short voice)
{
  short Result;
  line_status *WITH;

  WITH = &current[voice-1];
  Result = WITH->after_slur;
  if (WITH->after_slur > 0 && WITH->slur_level < 1)
    error3(voice, "M-Tx system error: afterSlur and slur_level incompatible)");
  return Result;
}


Char octave(short voice)
{
  return (current[voice-1].octave);
}


void resetOctave(short voice)
{
  current[voice-1].octave = ' ';
}


void initOctaves(Char *octaves)
{
  short i = 1;

  strcpy(init_oct, octaves);
  while (i <= strlen(init_oct)) {
    if (init_oct[i-1] == ' ')
      delete1(init_oct, i);
    else
      i++;
  }
}


Static Char initOctave(stave_index voice_stave)
{
  if (voice_stave > strlen(init_oct)) {
    if (pos1(clef[voice_stave-1], "Gt08") > 0)
      return '4';
    else
      return '3';
  } else
    return (init_oct[voice_stave-1]);
}


void setOctave(short voice)
{
  current[voice-1].octave = initOctave(voiceStave(voice));
}


void newOctave(short voice, Char dir)
{
  line_status *WITH;

  WITH = &current[voice-1];
  switch (dir) {

  case '+':
    WITH->octave++;
    break;

  case '-':
    WITH->octave--;
    break;
  }
}


Static short newPitch(short voice, Char *note_, short pitch, Char lastnote)
{
  Char note[256];
  short interval, npitch;
  Char oct;

  strcpy(note, note_);
  oct = octaveCode(note);
  if (oct == '=')
    oct = initOctave(voiceStave(voice));
  if (isdigit(oct)) {
    pitch = (oct - '0') * 7 - 3;
    lastnote = 'f';
    removeOctaveCode(oct, note);
    oct = octaveCode(note);
  }
  interval = note[0] - lastnote;
  if (interval > 3)
    interval -= 7;
  if (interval < -3)
    interval += 7;
  npitch = pitch + interval;
  while (oct != ' ') {
    if (oct == '+')
      npitch += 7;
    else if (oct == '-')
      npitch -= 7;
    removeOctaveCode(oct, note);
    oct = octaveCode(note);
  }
  return npitch;
}


Local void delins(Char *note, Char c1, Char c2, short l)
{
  short i, n;

  n = strlen(note);
  i = pos1(c1, note);
  if (i == 0)
    i = n + 1;
  while (l > 0 && i <= n) {
    delete1(note, i);
    n--;
    l--;
  }
  i = pos1(c2, note);
  if (i == 0) {
    if (strlen(note) < 2)
      error("M-Tx program error", print);
    else
      i = 3;
  }
  while (l > 0) {
    insertChar(c2, note, i);
    l--;
  }
}


Static void repitch(Char *note, short diff)
{
  diff /= 7;
  if (diff > 0)
    delins(note, '-', '+', diff);
  else
    delins(note, '+', '-', -diff);
}


void setUnbeamed(short voice)
{
  current[voice-1].beamed = false;
}


void setUnslurred(short voice)
{
  line_status *WITH;

  WITH = &current[voice-1];
  WITH->slurred = false;
  WITH->after_slur = 0;
}


void beginBeam(short voice, Char *note)
{
  line_status *WITH;

  WITH = &current[voice-1];
  if (WITH->beamed)
    error3(voice, "Starting a forced beam while another is open");
  if (WITH->beam_level > 0)
    error3(voice,
	   "Starting a forced beam while another is open (beamlevel>0)");
  WITH->beam_level++;
  WITH->beamnext = true;
  WITH->no_beam_melisma = startsWith(note, "[[");
  if (WITH->no_beam_melisma)
    predelete(note, 1);
}


void endBeam(short voice)
{
  line_status *WITH;

  WITH = &current[voice-1];
  if (WITH->beam_level < 1)
    error3(voice, "Closing a beam that was never opened");
  WITH->beam_level--;
  setUnbeamed(voice);
}


short slurLevel(short voice)
{
  return (current[voice-1].slur_level);
}


short beamLevel(short voice)
{
  return (current[voice-1].beam_level);
}


boolean noSlurMelisma(short voice, short history)
{
  line_status *WITH;

  WITH = &current[voice-1];
  return (WITH->no_slur_melisma[WITH->slur_level + history - 1]);
}


Static Char *slurLabel(Char *Result, short voice, Char *note)
{
  Char sl;

  if (*note == '\0')
    return strcpy(Result, "");
  if (strlen(note) < 2)
    return strcpy(Result, " ");
  if (note[1] >= '0' && note[1] <= 'Z')
    sl = note[1];
  else
    sl = ' ';
  if (sl >= 'I' && sl <= 'T')
    warning3(voice, "Slur label in the range I..T may cause conflict");
  sprintf(Result, "%c", sl);
  return Result;
}


Static void labelSlur(short voice, Char *note)
{
  Char sl;
  line_status *WITH;

  if (*note == '\0')
    return;
  WITH = &current[voice-1];
  if (note[0] == ')')
    WITH->slurID += 2;
  else if (note[0] == '}')
    WITH->tieID += 2;
  /** CMO 0.60a: replace assigning tieID to sl by space charater
    if (note[1]='(') or (note[1]=')') then sl:=slurID else sl:=tieID; */
  if (note[0] == '(' || note[0] == ')')
    sl = WITH->slurID;
  else
    sl = ' ';
  /** CMO 0.60d: omit insertchar in case of tie
    insertchar(sl,note,2); */
  if (note[0] == '(' || note[0] == ')')
    insertChar(sl, note, 2);
  if (note[0] == '(')
    WITH->slurID -= 2;
  else if (note[0] == '{')
    WITH->tieID -= 2;
  if (WITH->slurID < 'I')
    warning3(voice, "Too many nested slurs may cause conflict");
  if (WITH->tieID < 'I')
    warning3(voice, "Too many nested ties may cause conflict");
}


void beginSlur(short voice, Char *note)
{
  short posblind;
  line_status *WITH;
  Char STR1[256], STR2[256];

  WITH = &current[voice-1];
  WITH->slur_level++;
  if (WITH->slur_level > 12)
    error3(voice, "Too many open slurs");
  WITH->no_slur_melisma[WITH->slur_level - 1] = (startsWith(note, "((") ||
						 startsWith(note, "{{"));
  if (WITH->no_slur_melisma[WITH->slur_level - 1])
    predelete(note, 1);
  if (!strcmp(slurLabel(STR1, voice, note), "0"))
    delete1(note, 2);
  else {
    if (!strcmp(slurLabel(STR2, voice, note), " "))
      labelSlur(voice, note);
  }
  posblind = pos1('~', note);
  if (posblind > 0) {
    if (hideBlindSlurs())
      *note = '\0';
    else
      delete1(note, posblind);
  }
  WITH->slurnext = true;
}


void endSlur(short voice, Char *note)
{
  short poscontinue, posblind;
  Char contslur[256];
  line_status *WITH;
  Char STR1[256], STR2[256];

  WITH = &current[voice-1];
  *contslur = '\0';
  if (WITH->slur_level < 1)
    error3(voice, "Ending a slur that was never started");
  if (note[0] == ')')
    poscontinue = pos1('(', note);
  else if (note[0] == '}')
    poscontinue = pos1('{', note);
  if (poscontinue == 0)
    WITH->slur_level--;
  else {
    poscontinue--;
    strcpy(contslur, note);
    predelete(contslur, poscontinue);
    shorten(note, poscontinue);
  }
  if (WITH->slur_level == 0)
    setUnslurred(voice);
  if (!strcmp(slurLabel(STR1, voice, note), "0"))
    delete1(note, 2);
  else {
    if (!strcmp(slurLabel(STR2, voice, note), " "))
      labelSlur(voice, note);
  }
  if (!strcmp(slurLabel(STR1, voice, contslur), "0"))
    delete1(contslur, 2);
  else {
    if (!strcmp(slurLabel(STR2, voice, contslur), " "))
      labelSlur(voice, contslur);
  }
  if (poscontinue > 0) {
    if (note[0] == '}')
      strcat(note, "t");
    note[0] = 's';
    if (contslur[0] == '{')
      strcat(contslur, "t");
    contslur[0] = 's';
  }
  posblind = pos1('~', note);
  if (posblind > 0) {
    if (hideBlindSlurs())
      *note = '\0';
    else
      delete1(note, posblind);
  }
  if (*note != '\0' && *contslur != '\0')
    sprintf(note + strlen(note), " %s", contslur);
}


/* This is the routine called by prepmx.pas for every normal and chordal
note, immediately after checkOctave. It does two things: adjusts pitch
of notes following a C: chord to what it would have been without that
chord, and checks for pitch overruns. */
void renewPitch(short voice, Char *note)
{
  short pstat;
  line_status *WITH;

  WITH = &current[voice-1];
  pstat = newPitch(voice, note, WITH->chord_pitch, WITH->chord_lastnote);
  WITH->pitch = newPitch(voice, note, WITH->pitch, WITH->lastnote);
  if (WITH->pitch != pstat)
    repitch(note, WITH->pitch - pstat);
  checkRange(voice, range_name[WITH->pitch + 6]);
  if (WITH->pitch < lowest_pitch && checkPitch()) {
    printf("Pitch of note %s following %c reported as %d",
	   note, WITH->lastnote, WITH->pitch);
    error3(voice, "Pitch too low");
  }
  if (WITH->pitch > highest_pitch && checkPitch()) {
    printf("Pitch of note %s following %c reported as %d",
	   note, WITH->lastnote, WITH->pitch);
    error3(voice, "Pitch too high");
  }
  WITH->lastnote = note[0];
}


short chordPitch(short voice)
{
  return (current[voice-1].chord_pitch);
}


void renewChordPitch(short voice, Char *note)
{
  line_status *WITH;

  WITH = &current[voice-1];
  WITH->chord_pitch = newPitch(voice, note, WITH->chord_pitch,
			       WITH->chord_lastnote);
  if (WITH->chord_pitch < lowest_pitch)
    error3(voice, "Pitch in chord too low");
  if (WITH->chord_pitch > highest_pitch)
    error3(voice, "Pitch in chord too high");
  WITH->chord_lastnote = note[0];
}


void initStatus(void)
{
  short voice, FORLIM;
  line_status *WITH;

  FORLIM = nvoices;
  for (voice = 1; voice <= FORLIM; voice++) {
    WITH = &current[voice-1];
    WITH->duration = default_duration;
    WITH->octave_adjust = 0;
    WITH->slur_level = 0;
    WITH->beam_level = 0;
    WITH->beamed = false;
    WITH->beamnext = false;
    WITH->slurred = false;
    WITH->slurnext = false;
    WITH->after_slur = 0;
    WITH->octave = initOctave(voiceStave(voice));
    WITH->slurID = 'S';
    WITH->tieID = 'T';
    WITH->lastnote = 'f';
    WITH->pitch = (WITH->octave - '0') * 7 - 3;
    *WITH->chord_tie_label = '\0';
    saveStatus(voice);
  }
}




/* End. */
