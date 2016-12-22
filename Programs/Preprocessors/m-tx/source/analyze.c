/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "analyze.pas" */


#include "p2c.h"


#define ANALYZE_G
#include "analyze.h"


#ifndef MTX_H
#include "mtx.h"
#endif

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef LYRICS_H
#include "lyrics.h"
#endif

#ifndef MTXLINE_H
#include "mtxline.h"
#endif

#ifndef UPTEXT_H
#include "uptext.h"
#endif

#ifndef PREAMBLE_H
#include "preamble.h"
#endif

#ifndef UTILITY_H
#include "utility.h"
#endif


void includeStartString(void)
{
  voice_index voice;
  paragraph_index0 mus;
  voice_index FORLIM;
  Char STR1[256];
  Char STR2[256];

  FORLIM = nvoices;
  for (voice = 1; voice <= FORLIM; voice++) {
    mus = musicLineNo(voice);
    if (mus > 0) {
      sprintf(STR2, "%s%s", startString(STR1, voice), P[mus-1]);
      strcpy(P[mus-1], STR2);
    }
  }
}


Static Char *describe(Char *Result, short nbar, short extra)
{
  Char STR1[256];
  Char STR3[256];
  Char STR4[256];

  sprintf(Result, "%s bar%s + %s/64 notes",
	  toString(STR1, nbar), plural(STR3, nbar), toString(STR4, extra));
  return Result;
}


void testParagraph(void)
{
  voice_index0 voice;
  voice_index0 leader = 0, nv = 0;
  paragraph_index0 mus;
  short extra, l, nbar;
  voice_index0 FORLIM;
  Char STR2[256];
  Char STR4[256];

  nbars = 0;
  pickup = 0;
  nleft = 0;
  if (top > bottom)
    return;
  pickup = 0;
  multi_bar_rest = false;
  FORLIM = bottom;
  for (voice = top; voice <= FORLIM; voice++) {
    mus = musicLineNo(voice);
    if (mus > 0) {   /** -------------- Voice is present  ---- */
      nv++;
      line_no = orig_line_no[mus-1];
      scanMusic(voice, &l);
      if (multi_bar_rest && nv > 1)
	error("Multi-bar rest allows only one voice", print);
      if (!pmx_preamble_done) {
	if (voice == top)
	  pickup = l;
	else if (pickup != l)
	  error3(voice, "The same pickup must appear in all voices");
      }
      nbar = numberOfBars(voice);
      extra = ExtraLength(voice);
      if (multi_bar_rest && (nbar > 0 || extra > 0))
	error3(voice, "Multi-bar rest allows no other rests or notes");
      if (nbar > nbars || nbar == nbars && extra > nleft) {
	nbars = nbar;
	nleft = extra;
	leader = voice;
      }
      if (!final_paragraph && meternum > 0 && extra > 0) {
	printf("Line has %s\n", describe(STR2, nbar, extra));
	error("   Line does not end at complete bar", print);
      }
      if (pmx_preamble_done && l > 0 && meternum > 0) {
	printf("l=%d meternum=%d\n", l, meternum);
	error3(voice, "Short bar with no meter change");
      }
    }
  }
  if (!pmx_preamble_done) {
    xmtrnum0 = (double)pickup / one_beat;   /* Don't want an integer result */
    if (beVerbose())
      printf("Pickup = %d/64\n", pickup);
  }
  if (leader <= 0)
    return;
  FORLIM = bottom;
  for (voice = top; voice <= FORLIM; voice++) {
    if (musicLineNo(voice) > 0) {
      if (voice != leader) {
	mus = musicLineNo(voice);
	line_no = orig_line_no[mus-1];
	if (numberOfBars(voice) != numberOfBars(leader) ||
	    ExtraLength(voice) != ExtraLength(leader)) {
	  printf("Following line has %s\n",
		 describe(STR4, numberOfBars(voice), ExtraLength(voice)));
	  puts(musicLine(STR4, voice));
	  printf("Longest line has %s\n",
		 describe(STR2, numberOfBars(leader), ExtraLength(leader)));
	  puts(musicLine(STR2, leader));
	  error("Line duration anomaly", print);
	}
      }
    }
  }
}


void describeParagraph(void)
{
  voice_index0 voice;
  Char STR2[256];
  voice_index0 FORLIM;

  printf("---- Paragraph %d starting at line %d bar %d\n",
	 paragraph_no, orig_line_no[0], bar_no);
  FORLIM = bottom;
  for (voice = top; voice <= FORLIM; voice++)
    describeVoice(voice, lyricsReport(STR2, voice));
}


/* Local variables for paragraphSetup: */
struct LOC_paragraphSetup {
  voice_index0 *voice;
  short l;
  voice_index0 k;
  Char P_keep[256], w[256];
  boolean is_labelled;
} ;

Local void maybeUptext(short i, struct LOC_paragraphSetup *LINK)
{
  if (!doUptext())
    return;
  if (strlen(LINK->w) == 1) {
    if (*LINK->voice == nvoices)
      warning("Uptext line below bottom voice should be labelled", print);
  }
  if (strlen(LINK->w) == 1) {   /**  Standard chord line ------ */
    LINK->k = *LINK->voice + 1;
    if (LINK->k > nvoices)
      LINK->k--;
    setUptextLineNo(LINK->k, i);
    /**  Labelled chord line  ---- */
    return;
  }
  predelete(LINK->w, 1);
  LINK->k = findVoice(LINK->w);
  if (LINK->k == 0)
    error("Uptext line belongs to unknown voice", print);
  else
    setUptextLineNo(LINK->k, i);
}

Local void maybeChords(short i, struct LOC_paragraphSetup *LINK)
{
  if (!doChords())
    return;
  if (strlen(LINK->w) == 1 && *LINK->voice == 0 && pedanticWarnings())
    warning("Chord line above top voice should be labelled", print);
  if (strlen(LINK->w) == 1) {   /**  Standard chord line ------ */
    LINK->k = *LINK->voice;
    if (LINK->k == 0)
      LINK->k = 1;
    setChordLineNo(LINK->k, i);
    /**  Labelled chord line  ---- */
    return;
  }
  predelete(LINK->w, 1);
  LINK->k = findVoice(LINK->w);
  if (LINK->k == 0)
    error("Chord line belongs to unknown voice", print);
  else
    setChordLineNo(LINK->k, i);
}

Local void analyzeParagraph(struct LOC_paragraphSetup *LINK)
{
  paragraph_index i, FORLIM;

  *LINK->voice = 0;
  bottom = 0;
  top = nvoices + 1;
  clearLabels();
  clearTags();
  clearUptext();
  FORLIM = para_len;
  for (i = 1; i <= FORLIM; i++)
  {   /* ----- Paragraph analysis main loop ----- */
    if (*P[i-1] != '\0' && P[i-1][0] != comment) {
      NextWord(LINK->w, P[i-1], blank, colon);
      line_no = orig_line_no[i-1];
      LINK->l = strlen(LINK->w);
      LINK->is_labelled = (LINK->w[LINK->l-1] == colon &&
			   LINK->w[LINK->l-2] != barsym);
      if (LINK->is_labelled) {
	strcpy(LINK->P_keep, P[i-1]);
	predelete(P[i-1], LINK->l);
	shorten(LINK->w, LINK->l - 1);
	LINK->k = findVoice(LINK->w);   /* First look for a voice label */
	if (LINK->k > 0) {
	  *LINK->voice = LINK->k;
	  setMusicLineNo(*LINK->voice, i);
	} else if (LINK->w[0] == 'L')
	  maybeLyrics(*LINK->voice, i, LINK->w);
	else if (LINK->w[0] == 'C')
	  maybeChords(i, LINK);
	else if (LINK->w[0] == 'U')
	  maybeUptext(i, LINK);
	else {
	  if (startsWithIgnoreCase(LINK->w, "SPACE")) {
	    setSpace(P[i-1]);
	    must_respace = true;
	  } else {
	    if (startsWithIgnoreCase(LINK->w, "VOICES")) {
	      selectVoices(P[i-1]);
	      must_restyle = true;
	    } else {
	      strcpy(P[i-1], LINK->P_keep);
	      LINK->is_labelled = false;
	      if (!isNoteOrRest(LINK->w)) {
		error("Unknown line label", print);
		/** ------------ Maybe Space command ------------ */
		/** ------------ Maybe Voices command ------------ */
		/** Could be sticky attribute **/
	      }
	    }
	  }
	}
      }
      if (!LINK->is_labelled) {
	(*LINK->voice)++;
	setMusicLineNo(*LINK->voice, i);
      }
      if (*LINK->voice > bottom)
	bottom = *LINK->voice;
      if (*LINK->voice > 0 && *LINK->voice < top)
	top = *LINK->voice;
    }
  }
}

Local void obliterate(struct LOC_paragraphSetup *LINK)
{
  paragraph_index i;
  Char new_only[256];
  paragraph_index FORLIM;

  *new_only = '\0';
  FORLIM = para_len;
  for (i = 0; i <= FORLIM - 1; i++) {
    if (startsWithIgnoreCase(P[i], "only:")) {
      strcpy(new_only, P[i]);
      strcpy(P[i], "%");
    }
  }
  if (*new_only != '\0') {
    setOnly(new_only);
    return;
  }
  FORLIM = para_len;
  for (i = 1; i <= FORLIM; i++) {
    if (omitLine(i))
      strcpy(P[i-1], "%");
  }
}


void paragraphSetup(voice_index0 *voice_)
{
  struct LOC_paragraphSetup V;

  V.voice = voice_;
  obliterate(&V);
  analyzeParagraph(&V);
  reviseLyrics();
}




/* End. */
