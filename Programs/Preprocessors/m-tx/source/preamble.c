/* Output from p2c 1.21alpha-07.Dec.93, the Pascal-to-C translator */
/* From input file "preamble.pas" */


#include "p2c.h"


#define PREAMBLE_G
#include "preamble.h"


#ifndef CONTROL_H
#include "control.h"
#endif

#ifndef MTXLINE_H
#include "mtxline.h"
#endif

#ifndef STRINGS_H
#include "strings.h"
#endif

#ifndef MULTFILE_H
#include "multfile.h"
#endif

#ifndef FILES_H
#include "files.h"
#endif

#ifndef STATUS_H
#include "status.h"
#endif

#ifndef UTILITY_H
#include "utility.h"
#endif


#define blank_          ' '
#define colon_          ':'
#define semicolon       ';'
#define comma_          ','

#define max_styles      24


typedef enum {
  none, title, composer, pmx, tex, options, msize, bars, shortnote, style,
  sharps, flats, meter, space, pages, systems, width, height, enable, disable,
  range, name, indent, poet, part, only, octave_, start
} command_type;

typedef enum {
  unknown, colon_line, command_line, comment_line, plain_line
} line_type;

typedef char style_index;

typedef char style_index0;


#define c1              title
#define cn              start


Static short known_styles = 12;
Static boolean warn_redefine = false;

Static Char commands[28][17] = {
  "NONE", "TITLE", "COMPOSER", "PMX", "TEX", "OPTIONS", "SIZE", "BARS/LINE",
  "SHORT", "STYLE", "SHARPS", "FLATS", "METER", "SPACE", "PAGES", "SYSTEMS",
  "WIDTH", "HEIGHT", "ENABLE", "DISABLE", "RANGE", "NAME", "INDENT", "POET",
  "PART", "ONLY", "OCTAVE", "START"
};

/* short */
/* systems */

Static Char cline[28][256] = {
  "", "", "", "", "", "", "", "", "1/4", "", "0", "", "C", "", "1", "1",
  "190mm", "260mm", "", "", "", "", "", "", "", "", "", ""
};

Static boolean redefined[28] = {
  false, false, false, false, false, false, false, false, false, false, false,
  false, false, false, false, false, false, false, false, false, false, false,
  false, false, false, false, false, false
};

/** Known styles */

Static Char known_style[max_styles][256] = {
  "SATB:    Voices S,A T,B; Choral; Clefs G F",
  "SATB4:   Voices S A T B; Choral; Clefs G G G8 F",
  "SINGER:  Voices Voice; Vocal; Clefs G",
  "PIANO:   Voices RH LH; Continuo; Clefs G F",
  "ORGAN:   Voices RH LH Ped; Continuo; Clefs G F F",
  "SOLO:    Voices V; Clefs G", "DUET:    Voices V1 Vc; Clefs G F",
  "TRIO:    Voices V1 Va Vc; Clefs G C F",
  "QUARTET: Voices V1 V2 Va Vc; Clefs G G C F",
  "QUINTET: Voices V1 V2 Va Vc1 Vc2; Clefs G G C F F",
  "SEXTET:  Voices V1 V2 Va1 Va2 Vc1 Vc2; Clefs G G C C F F",
  "SEPTET:  Voices V1 V2 Va1 Va2 Vc1 Vc2 Cb; Clefs G G C C F F F", "", "", "",
  "", "", "", "", "", "", "", "", ""
};

Static style_index old_known_styles;
Static boolean style_used[max_styles];
Static boolean omit_line[lines_in_paragraph];
Static style_index0 orig_style_line[max_styles];
Static short orig_range_line;

Static short nclefs, n_pages, n_systems, n_sharps, ngroups;
Static Char part_line[256], title_line[256], composer_line[256],
	    pmx_line[256], options_line[256], start_line[256], voices[256],
	    clefs[256];
Static short group_start[maxgroups], group_stop[maxgroups];
Static Char instr_name[maxstaves][41];
Static boolean style_supplied;


/* ------------------ Styles ------------------ */

Static voice_index0 voiceCount(Char *s_)
{
  Char s[256];
  short i, l;

  strcpy(s, s_);
  l = strlen(s);
  for (i = 0; i <= l - 1; i++) {
    if (s[i] == comma_)
      s[i] = blank_;
  }
  return (wordCount(s));
}


Static style_index0 findStyle(Char *s_)
{
  style_index0 Result = 0;
  Char s[256];
  style_index0 i = 0;

  strcpy(s, s_);
  sprintf(s + strlen(s), "%c", colon_);
  while (i < known_styles) {
    i++;
    if (startsWithIgnoreCase(known_style[i-1], s))
      return i;
  }
  return Result;
}


Static void addStyle(Char *S)
{
  style_index0 sn;
  Char STR1[256];

  sn = findStyle(NextWord(STR1, S, colon_, dummy));
  if (sn > 0) {
    strcpy(known_style[sn-1], S);
    return;
  }
  if (known_styles < max_styles) {
    known_styles++;
    strcpy(known_style[known_styles-1], S);
  } else
    error("Can't add another style - table full", print);
}


Static void readStyles(void)
{
  boolean eofstyle;
  Char S[256];
  style_index0 l = 0;
  Char *TEMP;

  if (styleFileFound())
    eofstyle = true;
  else
    eofstyle = P_eof(stylefile);
  while (!eofstyle) {
    fgets(S, 256, stylefile);
    TEMP = strchr(S, '\n');
    if (TEMP != NULL)
      *TEMP = 0;
    if (*S != '\0') {
      addStyle(S);
      l++;
      orig_style_line[known_styles-1] = l;
    }
    eofstyle = P_eof(stylefile);
  }
}


Static void applyStyle(Char *s_, Char *stylename, stave_index first_inst,
		       stave_index first_stave)
{
  Char s[256];
  Char clef[256], subline[256], subcommand[256];
  stave_index0 i, last_inst, last_stave;
  boolean continuo = false, toosoon = false, vocal = false;
  Char STR2[256];

  strcpy(s, s_);
  last_inst = first_inst - 1;
  GetNextWord(subline, s, blank_, colon_);
  while (*s != '\0') {
    GetNextWord(subline, s, semicolon, dummy);
    i = curtail(subline, semicolon);
    GetNextWord(subcommand, subline, blank_, dummy);
    if (equalsIgnoreCase(subcommand, "VOICES")) {
      sprintf(voices + strlen(voices), " %s", subline);
      last_stave = first_stave + wordCount(subline) - 1;
      last_inst = first_inst + voiceCount(subline) - 1;
    } else if (equalsIgnoreCase(subcommand, "CLEFS")) {
      strcpy(clef, subline);
      sprintf(clefs + strlen(clefs), " %s", clef);
    } else if (equalsIgnoreCase(subcommand, "VOCAL")) {
      if (last_inst < first_inst)
	toosoon = true;
      else {
	some_vocal = true;
	vocal = true;
	for (i = first_inst; i <= last_inst; i++)
	  setVocal(i, true);
      }
    } else if (equalsIgnoreCase(subcommand, "CHORAL") ||
	       equalsIgnoreCase(subcommand, "GROUP")) {
      if (last_inst < first_inst)
	toosoon = true;
      else {
	if (equalsIgnoreCase(subcommand, "CHORAL")) {
	  some_vocal = true;
	  vocal = true;
	  for (i = first_inst; i <= last_inst; i++)
	    setVocal(i, true);
	}
	if (ngroups == maxgroups)
	  error("Too many groups", print);
	else {
	  ngroups++;
	  group_start[ngroups-1] = first_stave;
	  group_stop[ngroups-1] = last_stave;
	}
      }
    } else if (equalsIgnoreCase(subcommand, "CONTINUO"))
      continuo = true;
    else {
      sprintf(STR2, "Subcommand %s in STYLE unknown", subcommand);
      error(STR2, print);
    }
    if (toosoon) {
      sprintf(STR2, "You must first give VOICES before specifying %s",
	      subcommand);
      error(STR2, print);
    }
  }
  if (vocal && continuo)
    error("A continuo instrument may not be vocal", print);
  if (wordCount(clef) != last_stave - first_stave + 1)
    error("Number of clefs does not match number of voices", print);
  if (first_stave == last_stave || continuo) {
    strcpy(instr_name[first_stave-1], stylename);
/* p2c: preamble.pas, line 185:
 * Note: Possible string truncation in assignment [145] */
  } else {
    for (i = first_stave - 1; i <= last_stave - 1; i++)
      *instr_name[i] = '\0';
  }
  if (continuo) {
    ninstr++;
    stave[ninstr-1] = first_stave;
    for (i = first_stave - 1; i <= last_stave - 1; i++)
      instr[i] = ninstr;
    return;
  }
  for (i = first_stave; i <= last_stave; i++) {
    ninstr++;
    stave[ninstr-1] = i;
    instr[i-1] = ninstr;
  }
}


Static void applyStyles(void)
{
  short n1, n2, sn;
  Char s[256];
  Char STR2[256];

  *voices = '\0';
  *clefs = '\0';
  ninstr = 0;
  while (*cline[(long)style] != '\0') {
    n1 = voiceCount(voices) + 1;
    n2 = wordCount(voices) + 1;
    GetNextWord(s, cline[(long)style], blank_, comma_);
    curtail(s, comma_);
    sn = findStyle(s);
    if (sn == 0) {
      sprintf(STR2, "Style %s unknown", s);
      error(STR2, print);
    }
    line_no = orig_style_line[sn-1];
    applyStyle(known_style[sn-1], s, n1, n2);
    style_used[sn-1] = true;
  }
}


/* ------------------------------------------------------------- */

Static void wipeCommands(void)
{
  command_type c;

  for (c = c1; c <= cn; c = (command_type)((long)c + 1))
    *cline[(long)c] = '\0';
}


boolean omitLine(paragraph_index line)
{
  return (line > 0 && omit_line[line-1]);
}


Static void setName(void)
{
  short i, FORLIM;

  if (!redefined[(long)name])
    return;
  setFeature("instrumentNames", true);
  FORLIM = ninstr;
  for (i = 0; i <= FORLIM - 1; i++) {
    GetNextWord(instr_name[i], cline[(long)name], blank_, dummy);
/* p2c: preamble.pas, line 226:
 * Note: Possible string truncation in assignment [145] */
  }
}


Static void setIndent(void)
{
  if (redefined[(long)indent])
    strcpy(fracindent, cline[(long)indent]);
}


Static void setInitOctave(void)
{
  if (redefined[(long)octave_])
    initOctaves(cline[(long)octave_]);
}


Local void checkLabel(Char *w)
{
  voice_index j, FORLIM;
  Char STR1[256], STR2[256];

  FORLIM = nvoices;
  for (j = 0; j <= FORLIM - 1; j++) {
    if (!strcmp(w, voice_label[j])) {
      sprintf(STR2, "Voice label %s not unique", w);
      warning(STR2, print);
      return;
    }
  }
  if (strlen(w) > 2)
    return;
  if (pos1(w[0], "CLU") > 0) {
    if (strlen(w) > 1) {
      if (pos1(w[1], "123456789") == 0)
	return;
    }
  } else if (pos1(w[0], "123456789") == 0)
    return;
  sprintf(STR1, "Voice label %s conflicts with reserved label", w);
  error(STR1, print);
}


Static void setVoices(Char *line)
{
  short k;
  Char s[256], w[256];
  Char STR1[256];
  short FORLIM;

  nvoices = 0;
  nstaves = 0;
  do {
    GetNextWord(s, line, blank_, dummy);
    if (*s != '\0') {
      nstaves++;
      k = 0;
      first_on_stave[nstaves-1] = nvoices + 1;
      do {
	GetNextWord(w, s, blank_, comma_);
	curtail(w, comma_);
	if (*w != '\0') {
	  k++;
	  if (k <= 2) {
	    nvoices++;
	    checkLabel(w);
	    strcpy(voice_label[nvoices-1], w);
	    if (*instr_name[nstaves-1] == '\0') {
	      strcpy(instr_name[nstaves-1], w);
/* p2c: preamble.pas, line 266:
 * Note: Possible string truncation in assignment [145] */
	    }
	    setStavePos(nvoices, nstaves, k);
	  }
	}
      } while (*w != '\0');
      if (k > 2) {
	sprintf(STR1, "More than two voices per stave: %s", s);
	error(STR1, print);
      }
      if (k == 2) {
	sprintf(instr_name[nstaves-1], "\\mtxTwoInstruments{%s}{%s}",
		voice_label[nvoices-2], voice_label[nvoices-1]);
/* p2c: preamble.pas, line 273:
 * Note: Possible string truncation in assignment [145] */
      }
      number_on_stave[nstaves-1] = k;
    }
  } while (*line != '\0');
  FORLIM = nvoices;
  for (k = 0; k <= FORLIM - 1; k++)
    selected[k] = true;
}


Static void setClefs(Char *line_)
{
  Char line[256];
  Char s[256];

  strcpy(line, line_);
  nclefs = 0;
  do {
    GetNextWord(s, line, blank_, dummy);
    if (*s != '\0') {
      nclefs++;
      if (strlen(s) == 1)
	clef[nclefs-1] = s[0];
      else
	clef[nclefs-1] = s[1];
    }
  } while (*s != '\0');
}


Static void setDimension(Char *line_, command_type lno)
{
  Char line[256];
  short l;
  short n = 0, p = 0;
  Char STR1[256];

  strcpy(line, line_);
  if (*line == '\0')
    return;
  l = strlen(line);
  do {
    n++;
    if (line[n-1] == '.')
      p++;
  } while (n <= l && (line[n-1] == '.' || isdigit(line[n-1])));
  if (n == p || p > 1 ||
      line[n-1] != 'i' && line[n-1] != 'm' && line[n-1] != 'p')
    error("Dimension must be a number followed by in, mm or pt", print);
  sprintf(cline[(long)lno], "w%s", substr_(STR1, line, 1, n));
}


Static void setSize(Char *line_)
{
  Char line[256];
  stave_index0 i = 0;
  Char word[256];
  stave_index0 FORLIM;

  strcpy(line, line_);
  while (i < ninstr) {
    GetNextWord(word, line, blank_, dummy);
    if (*word == '\0')
      break;
    i++;
    getNum(word, &musicsize);
    stave_size[i-1] = musicsize;
  }
  if ((unsigned)musicsize >= 32 || ((1L << musicsize) & 0x110000L) == 0) {
    FORLIM = ninstr;
    for (i = 0; i <= FORLIM - 1; i++) {
      if (stave_size[i] == unspec)
	stave_size[i] = musicsize;
    }
  }
  if (musicsize < 16)
    musicsize = 16;
  else if (musicsize > 20)
    musicsize = 20;
}


Static command_type findCommand(Char *command)
{
  command_type j;

  curtail(command, ':');
  if (equalsIgnoreCase(command, "STYLE"))
    style_supplied = true;
  for (j = c1; j <= cn; j = (command_type)((long)j + 1)) {
    if (equalsIgnoreCase(command, commands[(long)j]))
      return j;
  }
  return none;
}


boolean isCommand(Char *command_)
{
  Char command[256];

  strcpy(command, command_);
  return (findCommand(command) != none);
}


Static boolean mustAppend(command_type command)
{
  return (command == tex);
}


Static void doEnable(Char *line, boolean choice)
{
  Char word[256];
  Char STR1[256];

  do {
    GetNextWord(word, line, blank_, dummy);
    if (*word != '\0') {
      if (!setFeature(word, choice)) {
	sprintf(STR1, "No such feature: %s", word);
	error(STR1, !print);
      }
    }
  } while (*word != '\0');
}


Static void setRange(Char *line_)
{
  Char line[256];
  short v, p;
  Char vl[256];
  short FORLIM;
  Char STR1[256], STR2[256];

  strcpy(line, line_);
  line_no = orig_range_line;
  FORLIM = nvoices;
  for (v = 1; v <= FORLIM; v++) {
    strcpy(vl, voice_label[v-1]);
    sprintf(STR1, "%s=", vl);
    p = strpos2(line, STR1, 1);
    if (p > 0) {
      if (strlen(line) < p + 6) {
	sprintf(STR2, "At least five characters must follow \"%s=\"", vl);
	error(STR2, print);
      }
      defineRange(v, substr_(STR2, line, p + strlen(vl) + 1, 5));
    } else {
      sprintf(STR2, "No range defined for voice %s", vl);
      warning(STR2, print);
      defineRange(v, "");
    }
  }
}


/* TODO: This procedure should test for assertions in a comment
 or be removed */
Static boolean isAssertion(Char *line)
{
  return false;
}


Static line_type doCommand(Char *line_)
{
  line_type Result;
  Char line[256];
  Char command[256];
  command_type last_command;
  boolean starts_with_note;
  Char STR1[256];
  Char STR3[70];
  Char STR4[256];

  strcpy(line, line_);
  if (line[0] == comment && !isAssertion(line))
    return comment_line;
  starts_with_note = maybeMusicLine(line);
  GetNextWord(command, line, blank_, colon_);
  sprintf(STR1, "%c", colon_);
  if (endsWith(command, STR1)) {
    last_command = findCommand(command);
    Result = command_line;
    if (last_command == enable)
      doEnable(line, true);
    else if (last_command == disable)
      doEnable(line, false);
    else if (last_command == range)
      orig_range_line = line_no;
    if (last_command != none) {
      if (mustAppend(last_command) && redefined[(long)last_command]) {
	if (strlen(cline[(long)last_command]) + strlen(line) > 254) {
	  sprintf(STR3,
		  "Total length of preamble command %s must not exceed 255",
		  commands[(long)last_command]);
	  error(STR3, !print);
	}
	sprintf(cline[(long)last_command] + strlen(cline[(long)last_command]),
		"\n%s", line);
      } else {
	strcpy(cline[(long)last_command], line);
	if (warn_redefine && redefined[(long)last_command]) {
	  sprintf(STR4, "You have redefined preamble command %s", command);
	  warning(STR4, print);
	}
      }
      if (last_command == start)
	strcpy(start_line, line);
      redefined[(long)last_command] = true;
      return Result;
    }
    Result = colon_line;
    sprintf(STR4, "%s%c %s", command, colon_, line);
    addStyle(STR4);
    orig_style_line[known_styles-1] = line_no;
    return Result;
  } else if (starts_with_note)
    return plain_line;
  else
    return unknown;
  return Result;
}


void setOnly(Char *line_)
{
  Char line[256];
  short num, num1, num2, l;
  Char s[256];
  Char STR1[256];

  strcpy(line, line_);
  if (*line == '\0')
    return;
  if (startsWithIgnoreCase(line, "only"))
    GetNextWord(STR1, line, colon_, dummy);
  for (l = 0; l <= lines_in_paragraph - 1; l++)
    omit_line[l] = true;
  do {
    GetNextWord(s, line, blank_, comma_);
    if (*s == '\0')
      return;
    curtail(s, comma_);
    if (pos1('-', s) == 0) {
      getNum(s, &num);
      if (num > 0 && num <= lines_in_paragraph)
	omit_line[num-1] = false;
      else
	warning("Invalid line number in Only: is skipped", print);
    } else {
      getTwoNums(s, &num1, &num2);
      if (num1 > 0 && num2 <= lines_in_paragraph) {
	for (num = num1 - 1; num <= num2 - 1; num++)
	  omit_line[num] = false;
      } else
	warning("Invalid line range in Only: is skipped", print);
    }
  } while (true);
}


void interpretCommands(void)
{
  short i, num, den, nbars;
  Char STR2[256];
  short FORLIM;

  strcpy(title_line, cline[(long)title]);
  strcpy(part_line, cline[(long)part]);
  if (*cline[(long)poet] != '\0' || *cline[(long)composer] != '\0')
    sprintf(composer_line, "\\mtxComposerLine{%s}{%s}",
	    cline[(long)poet], cline[(long)composer]);
  else
    *composer_line = '\0';
  strcpy(pmx_line, cline[(long)pmx]);
  GetNextWord(options_line, cline[(long)options], blank_, dummy);
  FORLIM = known_styles;
  for (i = 0; i <= FORLIM - 1; i++)
    style_used[i] = false;
  applyStyles();
  setVoices(voices);
  FORLIM = known_styles;
  for (i = old_known_styles; i <= FORLIM - 1; i++) {
    if (!style_used[i]) {
      warning("The following style was supplied but not used", !print);
      puts(known_style[i]);
    }
  }
  setClefs(clefs);
  if (!redefined[(long)meter]) {
    sprintf(STR2, "You have not defined Meter, assuming \"%s\" ",
	    cline[(long)meter]);
    warning(STR2, !print);
  }
  getMeter(cline[(long)meter], &meternum, &meterdenom, &pmnum, &pmdenom);
  setDefaultDuration(meterdenom);
  if (meternum == 0 &&
      !(redefined[(long)pages] || redefined[(long)systems] ||
	redefined[(long)bars])) {
    strcpy(cline[(long)bars], "1");
    redefined[(long)bars] = true;
  }
  if (redefined[(long)pages] || redefined[(long)systems]) {
    if (redefined[(long)bars])
      warning("BARS/LINE ignored since you specified PAGES or SYSTEMS", print);
    if (redefined[(long)systems])
      getNum(cline[(long)systems], &n_systems);
    else
      warning("PAGES specified but not SYSTEMS", !print);
    if (redefined[(long)pages])
      getNum(cline[(long)pages], &n_pages);
    else
      warning("SYSTEMS specified but not PAGES", !print);
  } else if (redefined[(long)bars]) {
    getNum(cline[(long)bars], &nbars);
    if (nbars > 0) {
      n_pages = 0;
      n_systems = nbars;
    }
  }
  getNum(cline[(long)sharps], &n_sharps);
  setSpace(cline[(long)space]);
  setSize(cline[(long)msize]);
  getTwoNums(cline[(long)shortnote], &num, &den);
  if (den == 0)
    den = 1;
  short_note = num * 64 / den;
  if (*cline[(long)flats] != '\0') {
    getNum(cline[(long)flats], &n_sharps);
    n_sharps = -n_sharps;
  }
  setName();
  setIndent();
  setInitOctave();
  setOnly(cline[(long)only]);
  setRange(cline[(long)range]);
  setDimension(cline[(long)width], width);
  setDimension(cline[(long)height], height);
  if (*options_line != '\0') {
    warning("\"Options\" is cryptic and obsolescent.", !print);
    printf("  Use \"Enable\" and \"Disable\" instead.\n");
  }
  FORLIM = strlen(options_line);
  for (i = 0; i <= FORLIM - 1; i++)
    processOption(options_line[i]);
}


void preambleDefaults(void)
{
  short i;

  xmtrnum0 = 0.0;
  strcpy(fracindent, "0");
  musicsize = 20;
  *start_line = '\0';
  some_vocal = false;
  ngroups = 0;
  style_supplied = false;
  for (i = 1; i <= maxvoices; i++)
    setVocal(i, false);
  for (i = 0; i <= maxstaves - 1; i++)
    stave_size[i] = unspec;
  for (i = 0; i <= maxstaves; i++)
    nspace[i] = unspec;
  /* next line seems to be spurious.  0.63a RDT */
  /* begin  nspace[i]:=unspec;  stave_size[i]:=unspec;  end; */
  n_pages = 1;
  n_systems = 1;
  readStyles();
  old_known_styles = known_styles;
  for (i = 0; i <= lines_in_paragraph - 1; i++)
    omit_line[i] = false;
}


Static void preambleGuess(voice_index maybe_voices)
{
  switch (maybe_voices) {

  case 1:
    strcpy(cline[(long)style], "Solo");
    break;

  case 2:
    strcpy(cline[(long)style], "Duet");
    break;

  case 3:
    strcpy(cline[(long)style], "Trio");
    break;

  case 4:
    strcpy(cline[(long)style], "Quartet");
    break;

  case 5:
    strcpy(cline[(long)style], "Quintet");
    break;

  case 6:
    strcpy(cline[(long)style], "Sextet");
    break;

  case 7:
    strcpy(cline[(long)style], "Septet");
    break;

  default:
    error("I cannot guess a style", !print);
    return;
    break;
  }
  printf("I guess this piece is a %s for strings in C major.\n",
	 cline[(long)style]);
  printf("  Why not provide a STYLE in the setup paragraph to make sure?\n");
}


/* ------------------------------------------------------------------ */

void nonMusic(void)
{
  paragraph_index i, FORLIM;

  FORLIM = para_len;
  for (i = 0; i <= FORLIM - 1; i++)
    doCommand(P[i]);
  setOnly(cline[(long)only]);
  wipeCommands();
}


boolean thisCase(void)
{
  boolean Result;

/* p2c: preamble.pas: Note: Eliminated unused assignment statement [338] */
  if (!startsWithIgnoreCase(P[0], "case:"))
    return true;
  Result = (choice != ' ' && pos1(choice, P[0]) > 0);
  strcpy(P[0], "%");
  return Result;
}


void augmentPreamble(boolean control_para)
{
  paragraph_index i;
  line_type l;
  short s[5];
  paragraph_index FORLIM;

  if (!thisCase())
    return;
  for (l = unknown; l <= plain_line; l = (line_type)((long)l + 1))
    s[(long)l] = 0;
  FORLIM = para_len;
  for (i = 0; i <= FORLIM - 1; i++) {
    line_no = orig_line_no[i];
    l = doCommand(P[i]);
    s[(long)l]++;
    if (l == comment_line && P[i][1] == comment) {
      predelete(P[i], 2);
      putLine(P[i]);
    }
    if (!control_para && l == unknown)
      error("Unidentifiable line", print);
  }
  if (!control_para && s[(long)command_line] > 0 && s[(long)plain_line] > 0)
    error("Mixture of preamble commands and music", !print);
}


void doPreamble(void)
{
  paragraph_index i;
  voice_index0 maybe_voices = 0;
  paragraph_index FORLIM;

  if (style_supplied)  /*augmentPreamble(not known);*/
    return;
  if (true)
    warning("No STYLE supplied", !print);
  FORLIM = para_len;
  for (i = 0; i <= FORLIM - 1; i++) {
    if (maybeMusicLine(P[i]))
      maybe_voices++;
  }
  if (maybe_voices > 0)
    preambleGuess(maybe_voices);
  else
    error("No voices found", !print);
}


void respace(void)
{
  stave_index i, j;
  Char STR1[256];
  Char STR2[256], STR3[256];

  for (i = ninstr; i >= 2; i--) {
    j = ninstr - i + 1;
    if (nspace[j] != unspec) {
      sprintf(STR3, "\\mtxInterInstrument{%s}{%s}",
	      toString(STR1, i - 1), toString(STR2, nspace[j]));
      TeXtype2(STR3);
    }
  }
  if (nspace[ninstr] != unspec) {
    sprintf(STR3, "\\mtxStaffBottom{%s}", toString(STR1, nspace[ninstr]));
    TeXtype2(STR3);
  }
  must_respace = false;
}


void restyle(void)
{
  must_restyle = false;
}


Static short clefno(Char cl)
{
  short Result;
  Char STR2[44];

  switch (cl) {

  case 'G':
  case '0':
  case 't':
  case '8':
    Result = 0;
    break;

  case 's':
  case '1':
    Result = 1;
    break;

  case 'm':
  case '2':
    Result = 2;
    break;

  case 'a':
  case '3':
    Result = 3;
    break;

  case 'n':
  case '4':
    Result = 4;
    break;

  case 'r':
  case '5':
    Result = 5;
    break;

  case 'F':
  case 'b':
  case '6':
    Result = 6;
    break;

  case 'C':
    Result = 3;
    break;

  default:
    sprintf(STR2, "Unknown clef code \"%c\" - replaced by treble", cl);
    warning(STR2, print);
    Result = 0;
    break;
  }
  return Result;
}


Static void doTenorClefs(void)
{
  voice_index i;
  Char c;
  voice_index FORLIM;
  Char STR1[256];
  Char STR3[256];

  FORLIM = nclefs;
  for (i = 1; i <= FORLIM; i++) {
    c = clef[i-1];
    if (c == '8' || c == 't') {
      sprintf(STR3, "\\\\mtxTenorClef{%s}\\", toString(STR1, PMXinstr(i)));
      putLine(STR3);
    }
  }
}


Static void insertTeX(void)
{
  if (redefined[(long)tex])
    TeXtype2(cline[(long)tex]);
}


Local Char *pmxMeter(Char *Result)
{
  short denom, num;
  Char STR1[256], STR3[256], STR4[256], STR5[256];

  if (meternum == 0) {
    num = beatsPerLine();   /** denom := 0; **/
    meterChange(old_meter_word, num, meterdenom, true);
  } else
    num = meternum;   /** denom := pmdenom; **/
  /* CMO: unconditonally assign value of pmdenom to denom */
  denom = pmdenom;
  sprintf(Result, "%s %s %s %s",
	  toString(STR1, num), toString(STR3, PMXmeterdenom(meterdenom)),
	  toString(STR4, pmnum), toString(STR5, denom));
  return Result;
}

Local Char *sizecode(Char *Result, short k)
{
  strcpy(Result, "\\mtxNormalSize");
  switch (k) {

  case 13:
    if (musicsize == 20)
      strcpy(Result, "\\mtxTinySize");
    else
      strcpy(Result, "\\mtxSmallSize");
    break;

  case 16:
    if (musicsize == 20)
      strcpy(Result, "\\mtxSmallSize");
    break;

  case 20:
    if (musicsize == 16)
      strcpy(Result, "\\mtxLargeSize");
    break;

  case 24:
    if (musicsize == 20)
      strcpy(Result, "\\mtxLargeSize");
    else
      strcpy(Result, "\\mtxHugeSize");
    break;

  case 29:
    strcpy(Result, "\\mtxHugeSize");
    break;

  default:
    error("Valid sizes are 13, 16, 20, 24, 29", print);
    break;
  }
  return Result;
}


void doPMXpreamble(void)
{
  static Char clefcode[9] = "0123456";
  short i, j;
  Char clefs[256];
  Char STR1[256], STR2[256];
  Char STR3[58];
  short FORLIM;
  Char STR5[256];

  if (*composer_line != '\0')
    putLine(composer_line);
  if (*title_line != '\0') {
    sprintf(STR2, "\\mtxTitleLine{%s}", title_line);
    putLine(STR2);
  }
  putLine("---");
  if (instrumentNames() && !redefined[(long)indent])
    strcpy(fracindent, "0.12");
  fprintf(outfile, "%d", nstaves);
  fprintf(outfile, " %d", -ninstr);
  stave[ninstr] = nstaves + 1;
  for (j = ninstr; j >= 1; j--)
    fprintf(outfile, " %d", stave[j] - stave[j-1]);
  fprintf(outfile, " %s %8.5f %d %d %d %d %s\n",
	  pmxMeter(STR1), xmtrnum0, n_sharps, n_pages, n_systems, musicsize,
	  fracindent);
  FORLIM = ninstr;
  for (i = 1; i <= FORLIM; i++) {
    if (!instrumentNames())
      putLine("");
    else {
      sprintf(STR3, "\\mtxInstrName{%s}", instr_name[ninstr - i]);
      putLine(STR3);
    }
  }
  *clefs = '\0';
  for (i = nclefs - 1; i >= 0; i--)
    sprintf(clefs + strlen(clefs), "%c", clefcode[clefno(clef[i])]);
  putLine(clefs);
  if (*texdir == '\0')
    strcpy(texdir, "./");
  putLine(texdir);

  pmx_preamble_done = true;
  insertTeX();
  respace();

  FORLIM = ngroups;
  for (j = 1; j <= FORLIM; j++)
    fprintf(outfile, "\\\\mtxGroup{%s}{%s}{%s}\\\n",
	    toString(STR1, j), toString(STR2, ninstr - group_start[j-1] + 1),
	    toString(STR5, ninstr - group_stop[j-1] + 1));
  FORLIM = ninstr;
  for (j = 1; j <= FORLIM; j++) {
    if (stave_size[j-1] != unspec) {
      sprintf(STR5, "\\\\mtxSetSize{%s}{%s}\\",
	      toString(STR1, ninstr - j + 1),
	      sizecode(STR2, stave_size[j-1]));
      putLine(STR5);
    }
  }
  if (*part_line != '\0') {
    putLine("Ti");
    putLine(part_line);
  }
  if (*composer_line != '\0') {
    putLine("Tc");
    putLine("\\mtxPoetComposer");
  }
  if (*title_line != '\0') {
    fprintf(outfile, "Tt");
    if (nspace[0] != unspec)
      fputs(toString(STR1, nspace[0]), outfile);
    putc('\n', outfile);
    putLine("\\mtxTitle");
  }
  if (*pmx_line != '\0')
    putLine(pmx_line);
  doTenorClefs();
  if (*cline[(long)width] != '\0')
    putLine(cline[(long)width]);
  wipeCommands();
}


Char *startString(Char *Result, voice_index0 voice)
{
  Char s[256], w[256];
  voice_index j;

  strcpy(s, start_line);
  for (j = 1; j <= voice; j++)
    GetNextWord(w, s, dummy, ';');
  curtail(w, ';');
  if (*w != '\0') {
    sprintf(Result, "%s ", w);
    return Result;
  } else
    return strcpy(Result, w);
}




/* End. */
