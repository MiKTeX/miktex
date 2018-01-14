/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "mtxline.pas" */


#include "p2c.h"


#define MTXLINE_G
#include "mtxline.h"


boolean bind_left[22] = {
  false, false, true, false, false, true, true, false, true, true, true,
  false, true, false, true, false, true, false, false, false, false, false
};

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef CONTROL_H
#include "control.h"
#endif

#ifndef NOTES_H
#include "notes.h"
#endif

#ifndef UTILITY_H
#include "utility.h"
#endif


typedef music_word word_scan[max_words];

typedef struct line_info {
  word_index0 here, nword;
  bar_index0 nbar;
  paragraph_index0 voice_pos, voice_stave, mus, chord;
  short extra;
  boolean vocal;
  word_index0 bar_bound[max_bars + 1];
  short word_bound[max_words + 1], orig_word_bound[max_words + 1];
  word_scan scan;
} line_info;


Static Char name[22][10] = {
  "?", "note", "znote", "lyricsTag", "(", ")", ")(", "[", "]", "_", "PMX<",
  "macro", "endMacro", "meter", "ornament", "rest", "//", "BAR", "TeX", "@",
  "firstonly", "ERROR"
};

Static line_info info[maxvoices];


/* ------------------------------------------------------------- */

short beatsPerLine(void)
{
  short Result;
  voice_index voice, FORLIM;
  line_info *WITH;

  FORLIM = nvoices;
  for (voice = 1; voice <= FORLIM; voice++) {
    WITH = &info[voice-1];
    if (WITH->nbar > 0 || WITH->extra > 0) {
      if (WITH->extra % one_beat > 0) {
/* p2c: mtxline.pas, line 99:
 * Note: Using % for possibly-negative arguments [317] */
	error3(voice, "Line length not an integer number of beats");
      }
      Result = WITH->nbar * meternum + WITH->extra / one_beat;
    }
  }
  return Result;
}


void skipChordBar(voice_index voice)
{
  line_info *WITH;
  Char STR1[256];

  WITH = &info[voice-1];
  if (WITH->chord > 0) {
    sprintf(STR1, "%c", barsym);
    if (!strcmp(P[WITH->chord - 1], STR1))
      predelete(P[WITH->chord - 1], 1);
  }
}


Char *getBar(Char *Result, voice_index voice, short bar)
{
  line_info *WITH;

  WITH = &info[voice-1];
  return (substr_(Result, P[WITH->mus - 1],
		  WITH->word_bound[WITH->bar_bound[bar-1]] + 1,
		  WITH->word_bound[WITH->bar_bound[bar]] -
		  WITH->word_bound[WITH->bar_bound[bar-1]]));
}


Char *musicLine(Char *Result, voice_index voice)
{
  return strcpy(Result, P[musicLineNo(voice) - 1]);
}


paragraph_index0 musicLineNo(voice_index voice)
{
  return (info[voice-1].mus);
}


void setMusicLineNo(voice_index voice, paragraph_index lno)
{
  info[voice-1].mus = lno;
}


paragraph_index0 chordLineNo(voice_index voice)
{
  return (info[voice-1].chord);
}


void setChordLineNo(voice_index voice, paragraph_index lno)
{
  info[voice-1].chord = lno;
}


void setVocal(voice_index voice, boolean voc)
{
  info[voice-1].vocal = voc;
}


boolean isVocal(voice_index voice)
{
  return (info[voice-1].vocal);
}


void setStavePos(voice_index voice, stave_index stave, stave_index pos)
{
  line_info *WITH;

  WITH = &info[voice-1];
  WITH->voice_pos = pos;
  WITH->voice_stave = stave;
}


stave_index voiceStave(voice_index voice)
{
  return (info[voice-1].voice_stave);
}


stave_index voicePos(voice_index voice)
{
  return (info[voice-1].voice_pos);
}


voice_index companion(voice_index voice)
{
  short s;

  s = info[voice-1].voice_stave;
  if (number_on_stave[s-1] == 1)
    return voice;
  else if (info[voice-1].voice_pos == 1)
    return (voice + 1);
  else
    return (voice - 1);
}


void regroup(voice_index voice)
{
  word_index0 i, j;
  word_index0 j2 = 0;
  line_info *WITH;
  word_index0 FORLIM;

  WITH = &info[voice-1];
  if (debugMode())
    printf("Voice %d has %d bars at ", voice, WITH->nbar);
  if (debugMode()) {
    FORLIM = WITH->nbar + 1;
    for (i = 1; i <= FORLIM; i++)
      printf("%d ", WITH->bar_bound[i]);
  }
  FORLIM = WITH->nbar;
  for (i = 1; i <= FORLIM; i++) {
    j2 = WITH->bar_bound[i];
    j = j2 + 1;
    while (j <= WITH->here &&
	   (bind_left[(long)WITH->scan[j-1]] || WITH->scan[j-1] == barword)) {
      WITH->bar_bound[i]++;
      j++;
    }
  }
  if (WITH->extra > 0)
    WITH->bar_bound[WITH->nbar + 1] = WITH->here;
  if (debugMode()) {
    printf(" regrouped to ");
    FORLIM = WITH->nbar + 1;
    for (i = 1; i <= FORLIM; i++)
      printf("%d ", WITH->bar_bound[i]);
  }
  if (debugMode())
    putchar('\n');
  WITH->nword = WITH->here;
}


void resetInfo(voice_index voice, Char *buf)
{
  line_info *WITH;

  WITH = &info[voice-1];
  strcpy(buf, P[WITH->mus - 1]);
  *P[WITH->mus - 1] = '\0';
  WITH->bar_bound[0] = 0;
  WITH->word_bound[0] = 0;
  WITH->orig_word_bound[0] = 0;
  WITH->nbar = 0;
  WITH->here = 0;
}


void clearLabels(void)
{
  voice_index voice, FORLIM;
  line_info *WITH;

  FORLIM = nvoices;
  for (voice = 0; voice <= FORLIM - 1; voice++) {
    WITH = &info[voice];
    WITH->chord = 0;
    WITH->mus = 0;
  }
}


void appendNote(voice_index voice, music_word nscan)
{
  line_info *WITH;

  WITH = &info[voice-1];
  WITH->here++;
  if (WITH->here > max_words)
    error3(voice, "Words per line limit exceeded");
  WITH->scan[WITH->here - 1] = nscan;
}


void appendToLine(voice_index voice, Char *note)
{
  line_info *WITH;

  if (*note == '\0')
    return;
  WITH = &info[voice-1];
  sprintf(P[WITH->mus - 1] + strlen(P[WITH->mus - 1]), "%s%c", note, blank);
  WITH->word_bound[WITH->here] = strlen(P[WITH->mus - 1]);
  WITH->orig_word_bound[WITH->here] = nextWordBound(orig_P[WITH->mus - 1],
      note[0], WITH->orig_word_bound[WITH->here - 1]);
}


void markBar(voice_index voice)
{
  line_info *WITH;

  WITH = &info[voice-1];
  if (WITH->nbar == 0)
    error3(voice, "Empty bar");
  else
    WITH->bar_bound[WITH->nbar] = WITH->here;
}


short numberOfBars(voice_index voice)
{
  return (info[voice-1].nbar);
}


void barForward(voice_index voice, short nbars)
{
  line_info *WITH;

  WITH = &info[voice-1];
  if (WITH->nbar + nbars < 0)
    error3(voice, "Next voice before bar is full");
  if (WITH->nbar + nbars > max_bars)
    error3(voice, "Bars per line limit exceeded");
  WITH->nbar += nbars;
  if (nbars > 0)
    WITH->bar_bound[WITH->nbar] = WITH->here;
}


void setExtraLength(voice_index voice, short ext)
{
  line_info *WITH;

  WITH = &info[voice-1];
  WITH->extra = ext;
  WITH->scan[WITH->here] = other;
}


short ExtraLength(voice_index voice)
{
  return (info[voice-1].extra);
}


voice_index0 findVoice(Char *w_)
{
  voice_index0 Result = 0;
  Char w[256];
  short i, FORLIM;

  strcpy(w, w_);
  curtail(w, ':');
  FORLIM = nvoices;
  for (i = 1; i <= FORLIM; i++) {
    if (!strcmp(w, voice_label[i-1]))
      return i;
  }
  getNum(w, &i);
  if (i == 0)
    return Result;
  if (i > 0 && i <= nvoices)
    return i;
  error("Numeric label outside range 1..nvoices", print);
  return Result;
}


Static void info3(voice_index voice)
{
  short p;
  line_info *WITH;

  WITH = &info[voice-1];
  printf("In voice \"%s\" near word %d:\n", voice_label[voice-1], WITH->here);
  p = WITH->orig_word_bound[WITH->here - 1] - 1;
  if (p < 0)
    p = 0;
  printf("%*cV\n", p, ' ');
}


void error3(voice_index voice, Char *message)
{
  Char STR1[256];

  info3(voice);
  sprintf(STR1, "   %s", message);
  error(STR1, print);
}


void warning3(voice_index voice, Char *message)
{
  Char STR1[256];

  info3(voice);
  sprintf(STR1, "   %s", message);
  warning(STR1, print);
}


Char *nextMusicWord(Char *Result, voice_index voice)
{
  return (MusicWord(Result, voice, info[voice-1].here));
}


Char *MusicWord(Char *Result, short voice, short n)
{
  line_info *WITH;

  WITH = &info[voice-1];
  if (n > 0 && n <= WITH->nword)
    return (substr_(Result, P[WITH->mus - 1], WITH->word_bound[n-1] + 1,
		    WITH->word_bound[n] - WITH->word_bound[n-1] - 1));
  else
    return strcpy(Result, "");
}


music_word thisNote(voice_index voice)
{
  line_info *WITH;

  WITH = &info[voice-1];
  return (WITH->scan[WITH->here - 2]);
}


music_word nextNote(voice_index voice)
{
  line_info *WITH;

  WITH = &info[voice-1];
  return (WITH->scan[WITH->here - 1]);
}


boolean endOfBar(voice_index voice, short bar_no)
{
  line_info *WITH;

  WITH = &info[voice-1];
  return (WITH->here > WITH->bar_bound[bar_no]);
}


void gotoBar(voice_index voice, short bar_no)
{
  line_info *WITH;

  WITH = &info[voice-1];
  WITH->here = WITH->bar_bound[bar_no-1] + 1;
}


Char *getMusicWord(Char *Result, voice_index voice)
{
  line_info *WITH;

  WITH = &info[voice-1];
  line_no = orig_line_no[WITH->mus - 1];
  MusicWord(Result, voice, WITH->here);
  WITH->here++;
  return Result;
}


boolean maybeMusicLine(Char *l_)
{
  Char l[256];
  Char w[256];
  music_word nscan;

  strcpy(l, l_);
  GetNextWord(w, l, blank, dummy);
/* p2c: mtxline.pas: Note: Eliminated unused assignment statement [338] */
  if (pos1(w[0], "abcdefgr()[]{}CMm") == 0)
    return false;
  if (pos1(':', w) == 0)
    return true;
  getNextMusWord(w, l, &nscan);
  return (nscan == abcdefg);
}


#define nmacros         99


Static boolean macro_initialized = false;
Static Char macro_text[nmacros][256];


Static void macroInit(void)
{
  short i;

  if (macro_initialized)
    return;
  macro_initialized = true;
  for (i = 0; i <= nmacros - 1; i++)
    *macro_text[i] = '\0';
}


Static short identifyMacro(Char *s_)
{
  Char s[256];
  short k;

  strcpy(s, s_);
  predelete(s, 2);
  getNum(s, &k);
  return k;
}


/* Local variables for getNextMusWord: */
struct LOC_getNextMusWord {
  Char *buf, *note;
} ;

Local void expandThisMacro(struct LOC_getNextMusWord *LINK)
{
  Char playtag[11];
  short playID;
  Char w[256];
  Char STR1[72];
  Char STR2[42];
  Char STR3[68];
  Char STR4[54];
  Char STR5[256], STR7[256];

  macroInit();
  if (strlen(LINK->note) == 1)
    error("Can't terminate a macro that has not been started", print);
  playID = identifyMacro(LINK->note);
  toString(playtag, playID);
/* p2c: mtxline.pas, line 317:
 * Note: Possible string truncation in assignment [145] */
  if (playID < 1 || playID > 99) {
    sprintf(STR2, "Macro ID %s is not in range 1..99", playtag);
    error(STR2, print);
  }
  if (LINK->note[1] == 'P') {
    if (*macro_text[playID-1] == '\0') {
      sprintf(STR3,
	      "Macro %s inserts empty text: did you define it before use?",
	      playtag);
      warning(STR3, print);
    }
    if (strlen(macro_text[playID-1]) + strlen(LINK->buf) <= 255) {
      if (debugMode()) {
	printf("Inserting macro %s text \"%s\"\n",
	       playtag, macro_text[playID-1]);
	printf("Buffer before insert: %s\n", LINK->buf);
      }
      sprintf(LINK->buf, "%s%s",
	      macro_text[playID-1], strcpy(STR5, LINK->buf));
      return;
    }
    sprintf(STR4, "Expansion of macro %s causes buffer overflow", playtag);
    error(STR4, print);
  }
  if (pos1(LINK->note[1], "SR") == 0) {
    sprintf(STR4, "Second character %c of macro word should be in \"PRS\"",
	    LINK->note[1]);
    error(STR4, print);
  }
  *macro_text[playID-1] = '\0';
  do {
    GetNextWord(w, LINK->buf, blank, dummy);
    if (*w == '\0') {
      sprintf(STR1,
	"Macro definition %s should be terminated on the same input line",
	playtag);
      error(STR1, print);
    }
    if (!strcmp(w, "M")) {
      if (debugMode())
	printf("Macro %s is: %s\n", playtag, macro_text[playID-1]);
      break;
    }
    if (w[0] == 'M' && strlen(w) > 1) {
      if (w[1] != 'P' || identifyMacro(w) == playID) {
	sprintf(STR7, "%s not allowed inside macro definition %s", w, playtag);
	error(STR7, print);
      }
    }
    sprintf(macro_text[playID-1] + strlen(macro_text[playID-1]), "%s ", w);
  } while (true);
  if (LINK->note[1] == 'R') {
    LINK->note[1] = 'P';
    sprintf(LINK->buf, "%s %s", LINK->note, strcpy(STR7, LINK->buf));
  }
}


void getNextMusWord(Char *buf_, Char *note_, music_word *nscan)
{
  struct LOC_getNextMusWord V;
  Char maybe_error[256];
  Char STR2[256];

  V.buf = buf_;
  V.note = note_;
  GetNextWord(V.note, V.buf, blank, dummy);
  if (*V.note == '\0')
    return;
  if (V.note[0] == 'M' && expandMacro()) {
    expandThisMacro(&V);
    getNextMusWord(V.buf, V.note, nscan);
    return;
  }
  if (V.note[0] == '\\') {
    *maybe_error = '\0';
    if (V.note[strlen(V.note) - 1] != '\\')
      strcpy(maybe_error, V.note);
    while (*V.buf != '\0' && V.note[strlen(V.note) - 1] != '\\')
      sprintf(V.note + strlen(V.note), " %s",
	      GetNextWord(STR2, V.buf, blank, dummy));
    if (V.note[strlen(V.note) - 1] != '\\')
      error("Unterminated TeX literal", print);
    *nscan = texword;
    if (*maybe_error != '\0') {
      sprintf(STR2, "Possible unterminated TeX literal: %s", maybe_error);
      warning(STR2, print);
    }
    return;
  }
  if (solfaNoteNames()) {
    translateSolfa(V.note);
    if (V.note[0] == '"')
      predelete(V.note, 1);
  }
  switch (V.note[0]) {

  case '_':
    *nscan = pmxprefix;
    delete1(V.note, 1);
    break;

  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'g':
    *nscan = abcdefg;
    break;

  case 'z':
    *nscan = zword;
    break;

  case '(':
    *nscan = lparen;
    break;

  case '{':
    if (V.note[strlen(V.note) - 1] == '}')
      *nscan = lyrtag;
    else
      *nscan = lparen;
    break;

  case ')':
    if (pos1('(', V.note) == 0)
      *nscan = rparen;
    else
      *nscan = rlparen;
    break;

  case '}':
    if (pos1('{', V.note) == 0)
      *nscan = rparen;
    else
      *nscan = rlparen;
    break;

  case '[':
    *nscan = lbrac;
    break;

  case ']':
    if (!strcmp(V.note, "]["))
      *nscan = other;
    else
      *nscan = rbrac;
    break;

  case '@':
    *nscan = atword;
    break;

  case 'm':
    *nscan = mword;
    break;

  case 'r':
    *nscan = rword;
    break;

  case '#':
  case '-':
  case 'n':
  case 'x':
  case '?':
  case 's':
  case 't':
  case 'D':
    *nscan = pmxl;
    break;

  case 'M':
    if (!strcmp(V.note, "M"))
      *nscan = endmacro;
    else
      *nscan = macro;
    break;

  case 'G':
    if (pos1('A', V.note) > 0)
      *nscan = pmxl;
    else
      *nscan = other;
    break;

  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    if (pos1('/', V.note) > 0)
      *nscan = mword;
    else
      *nscan = pmxl;
    break;

  case 'o':
    *nscan = oword;
    break;

  case 'A':
  case 'V':
    *nscan = FirstOnly;
    break;

  case '/':
    if (!strcmp(V.note, "//"))
      *nscan = nextvoice;
    else if (!strcmp(V.note, "/")) {
      strcpy(V.note, "//");
      warning("/ changed to //", print);
      *nscan = nextvoice;
    } else {
      error("Word starts with /: do you mean \\?", print);
      *nscan = err;
    }
    break;

  default:
    if (pos1('|', V.note) > 0)
      *nscan = barword;
    else
      *nscan = other;
    break;
  }
}


boolean upper(voice_index voice)
{
  line_info *WITH;

  WITH = &info[voice-1];
  if (WITH->voice_pos == 1 && voice < nvoices)
    return (WITH->voice_stave == info[voice].voice_stave);
  else
    return false;
}


void describeVoice(voice_index voice, Char *describe_lyr)
{
  short bar, w;
  line_info *WITH;
  Char STR1[256];
  line_info *WITH1;
  short FORLIM;

  WITH = &info[voice-1];
  printf("Voice `%s' is on line %d", voice_label[voice-1], WITH->mus);
  if (WITH->vocal) {
    printf(", is vocal");
    puts(describe_lyr);
  } else {
    if (WITH->chord > 0)
      printf(" and has chords on line %d", WITH->chord);
    putchar('\n');
  }
  if (!debugMode())
    return;
  WITH1 = &info[voice-1];
  printf("Line has %d bars", WITH1->nbar);
  if (WITH1->extra > 0)
    printf(" + %d/64 notes\n", WITH1->extra);
  else
    putchar('\n');
  printf("Bars:");
  FORLIM = WITH1->nbar;
  for (bar = 1; bar <= FORLIM; bar++)
    printf(" %s ", getBar(STR1, voice, bar));
  printf("\nWord types:");
  FORLIM = WITH1->nword;
  for (w = 0; w <= FORLIM - 1; w++)
    printf(" %s", name[(long)WITH1->scan[w]]);
  putchar('\n');
}


boolean aloneOnStave(stave_index stave)
{
  voice_index v;

  if (number_on_stave[stave-1] != 2)
    return true;
  else {
    v = first_on_stave[stave-1];
    return (info[v-1].mus == 0 || info[v].mus == 0);
  }
}


void selectVoices(Char *line_)
{
  Char line[256];
  short i = 0;
  short k;
  Char word[256];
  short FORLIM;

  strcpy(line, line_);
  FORLIM = nvoices;
  for (k = 0; k <= FORLIM - 1; k++)
    selected[k] = false;
  printf("Voice change to: %s = ", line);
  while (i < nvoices) {
    GetNextWord(word, line, blank, dummy);
    if (*word == '\0') {
      putchar('\n');
      return;
    }
    i++;
    printf("%s ", word);
    k = findVoice(word);
    if (k == 0)
      error("This voice is not in the style", print);
    selected[k-1] = true;
  }
  putchar('\n');
}




/* End. */
