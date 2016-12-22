/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "lyrics.pas" */


#include "p2c.h"


#define LYRICS_G
#include "lyrics.h"


#ifndef CONTROL_H
#include "control.h"
#endif

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef MTXLINE_H
#include "mtxline.h"
#endif

#ifndef STATUS_H
#include "status.h"
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


/* Symbols used in strings of melismatype */

#define beam_melisma    '['
#define inhibit_beam_melisma  '<'
#define slur_melisma    '('
#define inhibit_slur_melisma  '{'


typedef char other_index;

typedef char other_index0;

typedef Char melismatype[256];

typedef enum {
  nolyr, haslyr
} haslyrtype;
typedef enum {
  normal, aux
} auxtype;
typedef enum {
  asbefore, newassign
} assigntype;
typedef enum {
  virgin, active
} inittype;

typedef struct lyrinfotype {
  short lyr_adjust, slur_level, slur_change, beam_level, beam_change;
  melismatype melisma;
  boolean numbered;
} lyrinfotype;

typedef enum {
  none_given, global_lyrics, local_lyrics
} lyrlinetype;

typedef struct lyrtagtype {
  lyrlinetype lyrsource;
  haslyrtype has_lyrics;
  auxtype auxiliary;
  assigntype new_assign;
  inittype initialized;
  short linecount;
  Char tags[256];
} lyrtagtype;


#define maxLyrNums      64
#define lyrtaglength    40


#define lyr_adjust_undef  (-12345)
#define interstave      24


Static short LyrNum = 0;

Static boolean lyrmodealter[maxstaves], oldlyrmodealter[maxstaves];
Static lyrtagtype tag[maxvoices], oldtag[maxvoices];
Static lyrinfotype lyrinfo[maxvoices];
Static Char numberedParagraph[maxLyrNums][lyrtaglength + 1];


/* Information flow for verse numbers: at the SetLyrics stage, the first
line of each lyrics paragraph is checked for a verse number, and if
found, the paragraph tag is remembered.  At the AssignLyrics stage,
the first time that paragraph is encountered, the particular voice
is marked in lyrinfo as being numbered.  When that voice is later processed,
the mark in lyrinfo is interrogated and turned off: if it was on, the
mtxVerse pre-TeX string is put into the output. */

boolean hasVerseNumber(voice_index voice)
{
  boolean Result;

  Result = lyrinfo[voice-1].numbered;
  lyrinfo[voice-1].numbered = false;
  return Result;
}


Static boolean isNumberedLyricsParagraph(Char *tag)
{
  short i, FORLIM;

  FORLIM = LyrNum;
  for (i = 0; i <= FORLIM - 1; i++) {
    if (!strcmp(tag, numberedParagraph[i]))
      return true;
  }
  return false;
}


Static boolean anyTagNumbered(Char *tags_)
{
  boolean Result;
  Char tags[256];
  Char s[256];

  strcpy(tags, tags_);
/* p2c: lyrics.pas: Note: Eliminated unused assignment statement [338] */
  if (curtail(tags, '}') > 0)
    delete1(tags, 1);
  do {
    GetNextWord(s, tags, ',', dummy);
    if (*s == '\0')
      return false;
    if (isNumberedLyricsParagraph(s))
      return true;
  } while (true);
  return false;
}


/* Save the tag in the numberedParagraph list, stripping off braces if any */
Static void markNumbered(Char *tag_)
{
  Char tag[256];

  strcpy(tag, tag_);
  if (LyrNum >= maxLyrNums) {
    error("Too many numbered lines in the lyrics", !print);
    return;
  }
  LyrNum++;
  if (curtail(tag, '}') > 0)
    delete1(tag, 1);
  strcpy(numberedParagraph[LyrNum-1], tag);
/* p2c: lyrics.pas, line 104:
 * Note: Possible string truncation in assignment [145] */
}


/* --- end of procedures to keep track of verse numbering --- */

void lyricsParagraph(void)
{
  Char first[256], w[256];
  paragraph_index0 l, line;
  other_index0 i;
  other_index0 nother = 0;
  Char other_[10][lyrtaglength + 1];
  boolean numbered;
  Char STR1[256], STR3[256];
  paragraph_index0 FORLIM;
  Char STR4[256];
  Char STR5[256];

  if (!doLyrics())
    return;
  if (para_len < 2)
    fatalerror("empty lyrics paragraph");
  NextWord(w, P[0], blank, dummy);
  l = strlen(w);
  line_no = orig_line_no[0];
  if (w[l-1] != '}')
    strcat(w, "}");
  GetNextWord(first, w, dummy, '}');
  while (*w != '\0') {
    if (w[0] == '=')
      predelete(w, 1);
    if (w[0] != '{')
      sprintf(w, "{%s", strcpy(STR1, w));
    nother++;
    GetNextWord(other_[nother-1], w, dummy, '}');
/* p2c: lyrics.pas, line 123:
 * Note: Possible string truncation in assignment [145] */
  }
  if (beVerbose()) {
    printf("---- Paragraph %d starting at line %d has lyrics headed %s",
	   paragraph_no, line_no, first);
    for (i = 0; i <= nother - 1; i++)
      printf("=%s", other_[i]);
    putchar('\n');
  }
  sprintf(STR5, "%c Paragraph %s line %s bar %s",
	  comment, toString(STR1, paragraph_no), toString(STR3, line_no),
	  toString(STR4, bar_no));
  TeXtype2(STR5);
  sprintf(STR3, "\\mtxSetLyrics%s{%%", first);
  TeXtype2(STR3);
  FORLIM = para_len;
  for (line = 2; line <= FORLIM; line++) {
    lyrTranslate(P[line-1], &numbered);
    if (numbered) {
      if (line > 2)
	warning("Verse number not in first line of paragraph treated as lyrics",
		print);
      else {
	markNumbered(first);
	for (i = 0; i <= nother - 1; i++)
	  markNumbered(other_[i]);
      }
    }
    line_no = orig_line_no[line-1];
    if (strlen(P[line-1]) > max_lyrics_line_length && pmx_preamble_done)
      error("Lyrics line too long", print);
    if (pmx_preamble_done) {
      if (line == 2) {
	sprintf(STR3, "\\\\\\:%s", P[line-1]);
	put(STR3, putspace);
      } else {
	sprintf(STR3, "\\\\\\ %s", P[line-1]);
	put(STR3, putspace);
      }
    } else
      put(P[line-1], putspace);
    if (line < para_len) {
      if (pmx_preamble_done)
	putLine(" %\\");
      else
	putLine(" %");
    } else if (pmx_preamble_done)
      putLine("}\\");
    else
      putLine("}");
  }
  for (i = 0; i <= nother - 1; i++) {
    sprintf(STR1, "\\mtxCopyLyrics%s%s", first, other_[i]);
    TeXtype2(STR1);
  }
}


Static Char *songraise(Char *Result, voice_index voice)
{
  Char s[256];
  lyrtagtype *WITH;
  lyrinfotype *WITH1;
  Char STR1[256], STR3[256];

  WITH = &tag[voice-1];
  WITH1 = &lyrinfo[voice-1];
  if (WITH->initialized == virgin && WITH1->lyr_adjust == 0)
    return strcpy(Result, "");
  else {
    if (WITH->auxiliary == aux)
      strcpy(s, "Aux");
    else
      *s = '\0';
    sprintf(Result, "\\mtx%sLyricsAdjust{%s}{%s}",
	    s, toString(STR1, PMXinstr(voiceStave(voice))),
	    toString(STR3, WITH1->lyr_adjust));
    return Result;
  }
}


Char *lyricsReport(Char *Result, voice_index voice)
{
  Char l[256];
  lyrtagtype *WITH;

  WITH = &tag[voice-1];
  if (WITH->has_lyrics == nolyr ||
      WITH->lyrsource == none_given && *WITH->tags == '\0')
    return strcpy(Result, " but has no own lyrics");
  else {
    strcpy(l, " with ");
    if (WITH->auxiliary == aux)
      strcat(l, "auxiliary ");
    strcat(l, "lyrics ");
    if (WITH->lyrsource == local_lyrics)
      strcat(l, "locally defined as \"");
    else
      strcat(l, "labelled \"");
    sprintf(l + strlen(l), "%s\"", WITH->tags);
    if (anyTagNumbered(WITH->tags))
      strcat(l, " with verse numbers");
    return strcpy(Result, l);
  }
}


void initLyrics(void)
{
  /* at the start only */
  stave_index stave;
  voice_index voice, FORLIM;
  lyrinfotype *WITH;
  lyrtagtype *WITH1;
  stave_index FORLIM1;

  FORLIM = nvoices;
  for (voice = 0; voice <= FORLIM - 1; voice++) {
    WITH = &lyrinfo[voice];
    WITH1 = &tag[voice];
    WITH1->has_lyrics = nolyr;
    WITH->lyr_adjust = lyr_adjust_undef;
    *WITH->melisma = '\0';
    WITH->slur_level = 0;
    WITH->beam_level = 0;
    WITH1->auxiliary = aux;
    WITH1->lyrsource = none_given;
    WITH1->new_assign = asbefore;
    WITH1->initialized = virgin;
  }
  FORLIM1 = nstaves;
  for (stave = 0; stave <= FORLIM1 - 1; stave++)
    oldlyrmodealter[stave] = false;
}


Static void registerLyrics(voice_index voice, Char *w)
{
  lyrtagtype *WITH;

  WITH = &tag[voice-1];
  strcpy(oldtag[voice-1].tags, WITH->tags);
  oldtag[voice-1].lyrsource = WITH->lyrsource;
  WITH->lyrsource = global_lyrics;
  switch (strlen(w)) {

  case 0:
    *WITH->tags = '\0';
    break;

  case 1:
    fatalerror("M-Tx system error in registerLyrics");
    break;

  default:
    strcpy(WITH->tags, w);
    break;
  }
}


void extractLyrtag(voice_index voice, Char *note)
{
  /* inline lyrics change */
  lyrtagtype *WITH;
  Char STR1[256], STR2[256];

  WITH = &tag[voice-1];
  if (WITH->has_lyrics == nolyr) {
    error3(voice, "Inline lyrics change on no-lyrics line");
    return;
  }
  registerLyrics(voice, note);   /* was: ''); */
  sprintf(note, "\\mtxAssignLyrics{%s}%s",
	  toString(STR1, PMXinstr(voiceStave(voice))), strcpy(STR2, note));
  if (WITH->auxiliary == aux)
    sprintf(note, "\\mtxAuxLyr{%s}\\", strcpy(STR2, note));
  else
    strcat(note, "\\");
}


void clearTags(void)
{
  /* at start of paragraph analyis */
  voice_index voice, FORLIM;
  lyrtagtype *WITH;

  memcpy(oldtag, tag, maxvoices * sizeof(lyrtagtype));
  FORLIM = nvoices;
  for (voice = 0; voice <= FORLIM - 1; voice++) {
    WITH = &tag[voice];
    WITH->lyrsource = none_given;
    *WITH->tags = '\0';
    WITH->linecount = 0;
  }
}


#define maxlyrlen       (PMXlinelength - 15)


Local void convertlyrics(Char *lyn, voice_index voice, auxtype mx)
{
  static Char setlyr[256] = "%%\\\\\\mtxSetLyrics";
  Char btag[256], thistag[256], w[256];
  boolean numbered;
  Char STR1[256];
  lyrtagtype *WITH;
  Char STR2[256];

  NextWord(w, lyn, blank, dummy);
  WITH = &tag[voice-1];
  if (*w == '\0') {
    *WITH->tags = '\0';
    return;
  }
  WITH->has_lyrics = haslyr;
  WITH->auxiliary = mx;
  if (w[0] == '{') {
    registerLyrics(voice, w);
    return;
  }
  WITH->lyrsource = local_lyrics;
  WITH->linecount++;
  toString(thistag, voice * 10 + WITH->linecount);
  sprintf(btag, "{%s}", thistag);
  if (*WITH->tags == '\0')
    strcpy(WITH->tags, btag);
  else {
    WITH->tags[strlen(WITH->tags) - 1] = ',';
    sprintf(WITH->tags + strlen(WITH->tags), "%s}", thistag);
  }
  trim(lyn);
  lyrTranslate(lyn, &numbered);
  if (numbered)
    markNumbered(thistag);
  if (strlen(lyn) + strlen(btag) > maxlyrlen)
    sprintf(lyn, "%s%s{\\\n\\\\\\:%s}\\", setlyr, btag, strcpy(STR2, lyn));
  else
    sprintf(lyn, "%s%s{%s}\\", setlyr, btag, strcpy(STR1, lyn));
}

#undef maxlyrlen


void maybeLyrics(voice_index voice, paragraph_index parline, Char *w_)
{
  /* during paragraph analysis, parline had L:, already stripped */
  Char w[256];
  voice_index0 k;

  /**  Labelled lyrics line -- */
  strcpy(w, w_);
  if (!doLyrics())
    return;
  if (strlen(w) == 1 && voice == 0)
    warning("Lyrics line above top voice should be labelled", print);
  if (strlen(w) == 1) {   /**  Standard lyrics line -------- */
    k = voice;
    if (k == 0)
      k = 1;
    convertlyrics(P[parline-1], k, normal);
    return;
  }
  predelete(w, 1);
  k = findVoice(w);
  if (k == 0)
    error("Lyrics line belongs to unknown voice", print);
  else
    convertlyrics(P[parline-1], k, aux);
}


void reviseLyrics(void)
{
  /* after paragraph analysis */
  voice_index voice;
  stave_index stave;
  voice_index FORLIM;
  lyrtagtype *WITH;
  stave_index FORLIM1;

  FORLIM = nvoices;
  for (voice = 0; voice <= FORLIM - 1; voice++) {
    WITH = &tag[voice];
    if (oldtag[voice].lyrsource == global_lyrics &&
	WITH->lyrsource == none_given) {
      strcpy(WITH->tags, oldtag[voice].tags);
      WITH->lyrsource = global_lyrics;
    }
    WITH->new_assign = (assigntype)(WITH->has_lyrics == haslyr &&
				    strcmp(WITH->tags, oldtag[voice].tags));
    if (*WITH->tags == '\0')
      WITH->has_lyrics = nolyr;
    strcpy(oldtag[voice].tags, WITH->tags);
    oldtag[voice].lyrsource = WITH->lyrsource;
  }
  FORLIM1 = nstaves;
  for (stave = 1; stave <= FORLIM1; stave++) {
    WITH = &tag[first_on_stave[stave-1] - 1];
    lyrmodealter[stave-1] = (!aloneOnStave(stave) &&
			     WITH->has_lyrics == haslyr &&
			     WITH->auxiliary == normal);
  }
}


void assignLyrics(stave_index stave, Char *lyrassign)
{
  /* at start of new block */
  Char atag[256], instr[256], l[256];
  voice_index v1, v2, voice;
  lyrtagtype *WITH;
  Char STR2[256];
  lyrinfotype *WITH1;

  *lyrassign = '\0';
  toString(instr, PMXinstr(stave));
  v1 = first_on_stave[stave-1];
  v2 = v1 + number_on_stave[stave-1] - 1;
  /* don't reassign if other voice takes over */
  if (tag[v1-1].auxiliary == tag[v2-1].auxiliary &&
      tag[v1-1].has_lyrics != tag[v2-1].has_lyrics) {
    for (voice = v1 - 1; voice <= v2 - 1; voice++) {
      WITH = &tag[voice];
      if (WITH->new_assign == newassign)
	WITH->new_assign = (assigntype)WITH->has_lyrics;
    }
  }
  for (voice = v1 - 1; voice <= v2 - 1; voice++)
    lyrinfo[voice].numbered = false;
  for (voice = v1; voice <= v2; voice++) {
    WITH = &tag[voice-1];
    if (WITH->new_assign == newassign) {
      strcpy(atag, WITH->tags);
      if (*atag == '\0')
	strcpy(atag, "{}");
      sprintf(l, "\\mtxAssignLyrics{%s}%s", instr, atag);
      if (WITH->auxiliary == aux)
	sprintf(l, "\\mtxAuxLyr{%s}", strcpy(STR2, l));
      strcat(lyrassign, l);
      if (*WITH->tags == '\0')
	WITH->has_lyrics = nolyr;
      if (WITH->has_lyrics == haslyr && WITH->initialized == virgin) {
	WITH1 = &lyrinfo[voice-1];
	if (WITH->auxiliary == aux && upper(voice))
	  WITH1->lyr_adjust = interstave;
	else
	  WITH1->lyr_adjust = 0;
	strcat(lyrassign, songraise(STR2, voice));
	WITH->initialized = active;
      }
      if (anyTagNumbered(atag))
	lyrinfo[voice-1].numbered = true;
    }
  }
  if (lyrmodealter[stave-1] == oldlyrmodealter[stave-1])
    return;
  if (lyrmodealter[stave-1])
    sprintf(lyrassign + strlen(lyrassign), "\\mtxLyrModeAlter{%s}", instr);
  else
    sprintf(lyrassign + strlen(lyrassign), "\\mtxLyrModeNormal{%s}", instr);
  oldlyrmodealter[stave-1] = lyrmodealter[stave-1];
}


void lyricsAdjust(voice_index voice, Char *note)
{
  /* inline at-word */
  short adj;
  boolean force, put_above, put_below;
  lyrinfotype *WITH;
  lyrtagtype *WITH1;
  Char STR2[256];

  WITH = &lyrinfo[voice-1];
  WITH1 = &tag[voice-1];
  predelete(note, 1);
  force = (note[0] == '=');
  if (force)
    predelete(note, 1);
  put_above = (note[0] == '^');
  if (put_above)
    predelete(note, 1);
  put_below = (note[0] == 'v');
  if (put_below)
    predelete(note, 1);
  if (*note != '\0')
    getNum(note, &adj);
  else
    adj = 0;
  if (WITH1->has_lyrics == nolyr) {
    *note = '\0';
    return;
  }
  if (put_above)
    WITH->lyr_adjust = interstave;
  else if (put_below)
    WITH->lyr_adjust = 0;
  if (force)
    WITH->lyr_adjust = adj;
  else
    WITH->lyr_adjust += adj;
  songraise(note, voice);
  if (*note != '\0')
    sprintf(note, "\\\\%s\\", strcpy(STR2, note));
}


void lyrTranslate(Char *P, boolean *numbered)
{   /* Test for starting number */
  short k, l, number;
  Char Q[256], w[256];

  NextWord(w, P, blank, dummy);
  *numbered = false;
  if (endsWith(w, ".")) {
    getNum(w, &number);
    *numbered = (number != 0);
  }
  /*Translate lyrics link */
  *Q = '\0';
  l = strlen(P);
  for (k = 1; k <= l - 1; k++) {
    if (P[k-1] != '_' || P[k] == '_' || P[k] == ' ' || pos1(P[k], digits) > 0)
      sprintf(Q + strlen(Q), "%c", P[k-1]);
    else if (k > 1 && P[k-2] == '\\')
      strcat(Q, "mtxLowLyrlink ");
    else
      strcat(Q, "\\mtxLyrlink ");
  }
  sprintf(Q + strlen(Q), "%c", P[l-1]);
  strcpy(P, Q);
}


typedef enum {
  beam, slur
} melismaEnd;


Static Char removeLast(Char *s, Char *t)
{
  Char Result;
  short i, l;

  l = strlen(s);
  for (i = l; i >= 1; i--) {
    if (pos1(s[i-1], t) > 0) {
      Result = s[i-1];
      delete1(s, i);
      return Result;
    }
  }
  return dummy;
}


Static boolean OpenMelisma(Char *s)
{
  return (pos1(slur_melisma, s) > 0 || pos1(beam_melisma, s) > 0);
}


/* Local variables for getSyllable: */
struct LOC_getSyllable {
  voice_index voice;
  boolean BM, EM;
} ;

Local void startMelismas(Char *t, struct LOC_getSyllable *LINK)
{
  boolean open_before, open_now;
  short i, k;
  lyrinfotype *WITH;

  k = strlen(t);
  for (i = 0; i <= k - 1; i++) {
    WITH = &lyrinfo[LINK->voice-1];
    open_before = OpenMelisma(WITH->melisma);
    sprintf(WITH->melisma + strlen(WITH->melisma), "%c", t[i]);
    open_now = OpenMelisma(WITH->melisma);
    LINK->BM = (LINK->BM || open_now && !open_before);
  }
}

Local boolean endMelisma(short voice, struct LOC_getSyllable *LINK)
{
  Char found;
  short i;
  short count[2];
  melismaEnd t;
  short FORLIM1;
  lyrinfotype *WITH;
  Char STR1[4];

  LINK->EM = false;
  count[(long)slur] = -lyrinfo[voice-1].slur_change;
  count[(long)beam] = -lyrinfo[voice-1].beam_change;
  for (t = beam; t <= slur; t = (melismaEnd)((long)t + 1)) {
    FORLIM1 = count[(long)t];
    for (i = 1; i <= FORLIM1; i++) {
      WITH = &lyrinfo[voice-1];
      switch (t) {

      case slur:
	sprintf(STR1, "%c%c", slur_melisma, inhibit_slur_melisma);
	found = removeLast(WITH->melisma, STR1);
	break;

      case beam:
	sprintf(STR1, "%c%c", beam_melisma, inhibit_beam_melisma);
	found = removeLast(WITH->melisma, STR1);
	break;
      }
      if (found == dummy)
	error3(voice, "Ending a melisma that was never started");
      LINK->EM = (LINK->EM ||
	  (!OpenMelisma(WITH->melisma) &&
	   pos1(found, (sprintf(STR1, "%c%c", slur_melisma, beam_melisma),
			STR1)) > 0));
    }
  }
  return LINK->EM;
}

Local void startSlurMelisma(voice_index voice, struct LOC_getSyllable *LINK)
{
  static Char start[2] = {
    slur_melisma, inhibit_slur_melisma
  };

  Char slurs[256];
  short k, FORLIM;
  Char STR1[256];

  *slurs = '\0';
  FORLIM = lyrinfo[voice-1].slur_change;
  for (k = 1; k <= FORLIM; k++)
    sprintf(slurs, "%c%s",
	    start[noSlurMelisma(voice, 1 - k)], strcpy(STR1, slurs));
  startMelismas(slurs, LINK);
}

Local void startBeamMelisma(voice_index voice, struct LOC_getSyllable *LINK)
{
  static Char start[2] = {
    beam_melisma, inhibit_beam_melisma
  };

  Char beams[256];
  short k, FORLIM;
  Char STR1[256];

  *beams = '\0';
  FORLIM = lyrinfo[voice-1].beam_change;
  for (k = 1; k <= FORLIM; k++)
    sprintf(beams, "%c%s", start[noBeamMelisma(voice)], strcpy(STR1, beams));
  startMelismas(beams, LINK);
}

Local boolean startMelisma(short voice, struct LOC_getSyllable *LINK)
{
  LINK->BM = false;
  startSlurMelisma(voice, LINK);
  startBeamMelisma(voice, LINK);
  return LINK->BM;
}


/* Append mtxBM or mtxEM to pretex when appropriate */
void getSyllable(voice_index voice_, Char *pretex)
{
  struct LOC_getSyllable V;
  short t;
  lyrtagtype *WITH;
  lyrinfotype *WITH1;

  V.voice = voice_;
  WITH = &tag[V.voice-1];
  WITH1 = &lyrinfo[V.voice-1];
  if (WITH->has_lyrics != haslyr)
    return;
  t = WITH1->slur_level;
  WITH1->slur_level = slurLevel(V.voice);
  WITH1->slur_change = WITH1->slur_level - t;
  t = WITH1->beam_level;
  WITH1->beam_level = beamLevel(V.voice);
  WITH1->beam_change = WITH1->beam_level - t;
  if (startMelisma(V.voice, &V)) {
    if (WITH->auxiliary == aux)
      strcat(pretex, "\\mtxAuxBM");
    else
      strcat(pretex, "\\mtxBM");
  }
  if (!endMelisma(V.voice, &V))
    return;
  if (WITH->auxiliary == aux)
    strcat(pretex, "\\mtxAuxEM");
  else
    strcat(pretex, "\\mtxEM");
}


Static void getSyllable1(voice_index voice, Char *pretex)
{
  lyrinfotype *WITH;

  WITH = &lyrinfo[voice-1];
  printf("voice=%d, slurchange=%d, melisma=%s, beamchange=%d\n",
	 voice, WITH->slur_change, WITH->melisma, WITH->beam_change);
  getSyllable1(voice, pretex);
  if (*pretex != '\0')
    printf("pretex = %s\n", pretex);
}




/* End. */
