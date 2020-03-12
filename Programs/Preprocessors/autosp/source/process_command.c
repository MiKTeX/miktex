
# include "process_score.h"

# define APPOGG_SPACING SP(64)+SP(256)  /* not a legitimate spacing */
# define notespp "\\vnotes2.95\\elemskip"
# define Notespp "\\vnotes3.95\\elemskip"
# define NOtespp "\\vnotes4.95\\elemskip"
# define NOTespp "\\vnotes6.95\\elemskip"
# define NOTEsp  "\\vnotes9.52\\elemskip"
# define APPOGG_NOTES "\\vnotes1.45\\elemskip"

char terminator[MAX_STAFFS];    /* one of '&' "|', '$'                */

char *notes[MAX_STAFFS];        /* note segment for ith staff         */
char *current[MAX_STAFFS];

int spacings[MAX_STAFFS];       /* spacing for ith staff              */

int vspacing[MAX_STAFFS];       /* virtual-note (skip) spacing        */
bool vspacing_active[MAX_STAFFS]; /* virtual-note spacing active?       */
                      /* used to preclude unnecessary pre-accidental skips    */

bool nonvirtual_notes;          /* used to preclude output of *only* virtual notes */

int cspacing[MAX_STAFFS];       /* nominal collective-note spacing    */
char collective[MAX_STAFFS][SHORT_LEN];
                                    /* prefixes for collective note sequences */
bool first_collective[MAX_STAFFS];

char deferred_bar[SHORT_LEN];   /* deferred \bar (or \endpiece etc.)  */

int beaming[MAX_STAFFS];        /* spacing for beamed notes           */
int new_beaming;
int semiauto_beam_notes[MAX_STAFFS]; /* semi-automatic beam notes     */

int spacing_staff;              /* staff that determines current spacing  */

/* save-restore state for a staff; used in process_xtuplet */
int beamingi;  
char *currenti;
int cspacingi;
int vspacingi;
char collectivei[SHORT_LEN];
bool first_collectivei;

int xtuplet[MAX_STAFFS];        /* x for xtuplet in staff i          */

bool appoggiatura;

char outstrings[MAX_STAFFS][LINE_LEN];  
                                     /* accumulate commands to be output    */
char *n_outstrings[MAX_STAFFS];

int global_skip;  
   /* = 1, 2, 3, or 4 for (non-standard) commands \QQsk \HQsk \TQsk \Qsk    */
   /* = 5 for five commas and double-flat accidental spacing                */
   /* = 6 for six commas                                                    */


char s[SHORT_LEN];  /* string for ps()  */
char *s_n;
char *ps(int spacing);
void note_segment (char *s);
void status (int i);
void status_spacing (void);
void status_all (void);
void status_collective (int i);
void status_beam (int i);


int spacing_note (int i);
void output_notes (int i);
void initialize_notes ();
void process_xtuplet (void);

PRIVATE
void analyze_notes (char **ln) 
/* divide material from *ln to \en into notes[i] segments
   and initialize terminator[i] etc.                          */
{
  int i; char *s; char *t;  
  int instr=1;
  int newlines = 0;
  s = *ln+1;  /* skip "/"  */
  while (isalpha(*s)) {s++;}  /* skip rest of the initial command  */
  while (true)
  { /* look for \en */
    t = strstr(s, "\\en");
    if (t != NULL) break;
    /* replace EOL by a blank and append another line of input */
    { char new_line[LINE_LEN];  
      char *nl;
      if (fgets (new_line, LINE_LEN, infile) == NULL)
        error ("Unexpected EOF.");
      if (strlen (new_line) == LINE_LEN-1) 
        error ("Line too long.");
      nl = new_line;
      while (*nl == ' ') nl++;  /* avoid spaces */
      t = strpbrk (s, "\n%");
      if (t == NULL) error ("Missing EOL.");
      if (*t == '\n') 
      { *t = ' '; t++;}
      *t = '\0';
      if (append (line, &t, nl, LINE_LEN) >= LINE_LEN)
        error ("Line too long.");
      newlines++;
    }
  }
  /* t -> "\en"  */
  if ( prefix("\\enotes", t) ) *ln = t+7;
  else *ln = t+3;
  *t = '$'; /* replace '\' by '$' (a distinctive terminating character) */
  for (i=1; i <= nstaffs; i++)      
  { char *tt;
    while (*s == ' ') s++;  /* skip spaces  */
    if (active[i])
    { notes[i] = s; 
      current[i] = s; 
    }
    tt = strpbrk (s, "|&$");
    if (tt == NULL) error ("can't parse note-spacing command.");
    s = tt; 
    terminator[i] = *s;
    staff_instr[i] = instr;
    if (terminator[i] == '&') instr++;
    if (*s != '$') s++;
  }
  lineno = lineno + newlines;

  /* initialize: */
  spacing = MAX_SPACING;
  old_spacing = MAX_SPACING;
  appoggiatura = false;
  global_skip = 0;
  for (i=1; i <= nstaffs; i++)
  {
    if (active[i])
    { n_outstrings[i] = outstrings[i];
      *n_outstrings[i] = '\0';
      vspacing[i] = 0;  
      vspacing_active[i] = false;  
      collective[i][0] = '\0'; 
      cspacing[i] = MAX_SPACING; 
      first_collective[i] = false;
      xtuplet[i] = 1;
    }
  }
  if (debug) 
  { fprintf (logfile, "\nAfter analyze_notes:\n");
    status_all ();
  }
}

PRIVATE
void checkc (char *s, char c)
{ if (*s != c) 
  {
    printf ("Error on line %d: Expected %c but found %d:\n%s\n", lineno, c, *s, line);
    exit (EXIT_FAILURE);
  }
}

PRIVATE
void checkn (char *s)
{ if (strpbrk (s, "0123456789") != s) 
  {
    printf ("Error on line %d: Expected digit but found %c:\n%s\n", lineno, *s, line);
    exit (EXIT_FAILURE);
  }
}

PRIVATE
void filter_output (int i)
{ /* discard \sk \bsk \Qsk \TQsk \HQsk \QQsk and \Cpause */
  char *s = notes[i];
  while (s < current[i])
  { char *t;
    t = strpbrk (s+1, "\\&|$");
    if (t == NULL || t > current[i]) t = current[i];
    if (!prefix ("\\sk", s)
     && !prefix ("\\bsk", s)
     && !prefix ("\\Cpause", s) 
     && !prefix ("\\Qsk", s) 
     && !prefix ("\\HQsk", s) 
     && !prefix ("\\TQsk", s) 
     && !prefix ("\\QQsk", s) 
     && !prefix ("\\Triolet", s)
     && !prefix ("\\Xtuplet", s) )
    {
      while (s < t) 
      { while (*s == ',') s++; /* global skips */
        *n_outstrings[i] = *s; 
        n_outstrings[i]++; s++; 
      }
      *(n_outstrings[i]) = '\0';
    }
    s = t;
  }
  notes[i] = s;
}

void output_notes (int i)
/* append from notes[i] up to current[i] to outstrings[i] */
{ if (debug) 
  { fprintf (logfile, "\nEntering output_notes:\n");
    status (i);
  }
  if (debug)
  {
    fprintf (logfile, "\nAppending %s for global_skip=%i\n", global_skip_str[global_skip], global_skip);
  }
  append (outstrings[i], &(n_outstrings[i]), global_skip_str[global_skip], LINE_LEN);
  if (vspacing[i] > 0)
  { 
    if (nonvirtual_notes) 
      append (outstrings[i], &(n_outstrings[i]), "\\sk", LINE_LEN); 
    vspacing[i] = vspacing[i] - spacing;
  }
  else if (collective[i][0])
  { 
    if (debug) 
    { fprintf (logfile, "\nBefore outputting collective note:\n");
      status (i);
      status_collective (i);
    }
    if (!first_collective[i])
      append (outstrings[i], &(n_outstrings[i]), collective[i], LINE_LEN);
    first_collective[i] = false;
    filter_output (i);
    append (outstrings[i], &(n_outstrings[i]), "}", LINE_LEN);
    if (*notes[i] == '}')
    { collective[i][0] = '\0';
      cspacing[i] = 0;
      notes[i]++;
    }
    if (*notes[i] == '}')  /* close of {\tinynotesize..{}}? */
    { append (outstrings[i], &(n_outstrings[i]), "}", LINE_LEN);
      notes[i]++;
    }
    if (debug)
    { fprintf (logfile, "\nAfter outputting collective note:\n");
      status (i);
      status_collective (i);
    }
  }
  else
    filter_output (i);
  if (debug)
  { fprintf (logfile, "\nAfter output_notes:\n");
    status (i);
    status_spacing ();
  }
}


int spacing_note (int i);
/*  Search from current[i] for the next spacing note in the ith staff.  
 *  Returns the spacing value and updates current[i].
 */

PRIVATE void
output_rests (void)
{ /* outputs a multi-bar rest and the deferred_bar command */
  int i;
  fprintf ( outfile, "\\NOTes\\sk\\en%%\n" );
  fprintf (outfile, "\\def\\atnextbar{\\znotes");
  if (restbars == 1)
     fprintf ( outfile, "\\centerpause" );
  else  
     fprintf ( outfile, "\\centerHpause{%d}", restbars );
  fprintf (outfile, "\\en}%%\n");
  if (Changeclefs) /* \Changeclefs has to be output after \def\atnextbar...  */
  {
    fprintf ( outfile, "\\Changeclefs%%\n");
    Changeclefs = false;
  }
  if (restbars > 1) 
    fprintf ( outfile, "\\advance\\barno%d%%\n", restbars-1 );
  restbars = 0; 
  for (i=1; i <= nstaffs; i++) bar_rest[i] = false;
  fprintf ( outfile, "%s%%\n", deferred_bar); 
  deferred_bar[0] = '\0';
}

void initialize_notes ()
{ int i;  
  if (debug)
  { fprintf (logfile, "\nEntering initialize_notes\n");
    status_all ();
  }
  if ( nastaffs == 1 && spacing != MAX_SPACING && restbars > 0) 
    output_rests ();

  fprintf (outfile, "\\scale");
  if (debug)
     fprintf (logfile, "spacing_staff = %i, staff_instr[spacing_staff] = %i, instrument_size[staff_instr[spacing_staff]] = %s\n",
                       spacing_staff,       staff_instr[spacing_staff],      instrument_size[staff_instr[spacing_staff]]);
  fprintf (outfile, "%s\n", instrument_size[staff_instr[spacing_staff]]);
  oldspacing_staff = spacing_staff;

  if (spacing == MAX_SPACING)
    fprintf (outfile, "\\znotes");
  else if (spacing == SP(1)+SP(2) || spacing == SP(1)+SP(2)+SP(4))
    fprintf (outfile, NOTEsp); 
  else if (spacing == SP(1))   
    fprintf (outfile, "\\NOTEs"); 
  else if (spacing == SP(2)+SP(4)+SP(8))
    fprintf (outfile, "%s", NOTespp);
  else if (spacing == SP(2)+SP(4) )
    fprintf (outfile, "\\NOTesp"); 
  else if (spacing == SP(2))
    fprintf (outfile, "\\NOTes"); 
  else if (spacing == SP(4)+SP(8) )
    fprintf (outfile, "\\NOtesp"); 
  else if (spacing == SP(4)+SP(8)+SP(16) )
    fprintf (outfile, "%s", NOtespp);
  else if (spacing == SP(4))
    fprintf (outfile, "\\NOtes"); 
  else if (spacing == SP(8)+SP(16) )
    fprintf (outfile, "\\Notesp"); 
  else if (spacing == SP(8)+SP(16)+SP(32))
    fprintf (outfile, "%s", Notespp);
  else if (spacing == SP(8))
    fprintf (outfile, "\\Notes"); 
  else if (spacing == SP(16)+SP(32)+SP(64))
    fprintf (outfile, notespp);
  else if (spacing == SP(16)+SP(32) ) 
    fprintf (outfile, "\\notesp"); 
  else if (spacing == SP(16) || spacing == SP(32)+SP(64) || spacing == SP(32)+SP(64)+SP(128))
    fprintf (outfile, "\\notes"); 
  else if (spacing == SP(32) || spacing == SP(64)+SP(128) || spacing == SP(64)+SP(128)+SP(256))
    fprintf (outfile, "\\nnotes");  
  else if (spacing == SP(64) )
    fprintf (outfile, "\\nnnotes"); 
  else if (spacing == APPOGG_SPACING)
    fprintf (outfile, "%s", APPOGG_NOTES); 
  else 
  { printf ("Error on line %d: spacing %s not recognized.\n", lineno, ps(spacing));
    exit (EXIT_FAILURE);
  }
  if (debug) 
  {  fprintf (logfile, "\noutputting \\Notes command for spacing=%s.\n", ps(spacing));
     status_all();
  }
  for (i=1; i <= nstaffs; i++) 
  { if (active[i])
    { n_outstrings[i] = outstrings[i];
    *n_outstrings[i] = '\0';
    }
  }
}

PRIVATE
void terminate_notes ()
{ int i;
  char *s, *t;
  if (debug)
  { fprintf (logfile, "\nEntering terminate_notes:\n");
    status_all ();
  }
  t = TransformNotes2;
  while (true)
  { s = strchr (t, '#');
    if (s == NULL) 
      break;
    while (t < s)  /* output any initial \transpose etc. */
    { putc (*t, outfile); t++; }
    t++; /* skip # */
    i = atoi (t) -1; t++;
    if (spacing == MAX_SPACING)
    { /* output any commands left in notes[i] */
      filter_output (i);
    }
    fprintf (outfile, "%s", outstrings[i]);
    if (debug) fprintf (logfile, "\noutputting %s from outstrings[%d].\n", outstrings[i], i);
    outstrings[i][0] = '\0';
    n_outstrings[i] = outstrings[i];
    if (spacing < MAX_SPACING && spacing > 2 * old_spacing)
    { /* add extra space *before* much longer notes */
      fprintf (outfile, "\\hqsk");
      if (debug) fprintf (logfile, "\nExtra half-notehead space before longer notes.\n");
    }
    if (*t != '\0') 
    { putc (*t, outfile); t++; }  /* terminator */
  }
  if (spacing == MAX_SPACING && old_spacing < SP(8) )
  { /* add extra space before \en */
    fprintf (outfile, "\\hqsk");
    if (debug) fprintf (logfile, "\nExtra half-notehead space before \\en.\n");
  }
  fprintf (outfile, "\\en"); 
  if (debug)
  { fprintf (logfile, "\noutputting \\en\n"); 
    fprintf (logfile, "\nAfter terminate_notes:\n");
    status_all ();
  }
}

PRIVATE
void process_appogg (void)
{ int i;
  if (debug)
  { fprintf (logfile, "\nEntering process_appogg:\n");
    status_all();
  }

  if (old_spacing < MAX_SPACING) 
  { char *s, *t;
    if (debug) fprintf (logfile, "Terminate current notes command:\n");
    t = TransformNotes2;
    while (true)
    { s = strchr (t, '#');
      if (s == NULL) break;
      while (t < s)  /* output any initial \transpose etc. */
      { putc (*t, outfile); t++; }
      t++; /* skip # */
      i = atoi (t) - 1; t++;
      fprintf (outfile, "%s", outstrings[i]);
      if (debug) fprintf (logfile, "\noutputting %s from outstrings[%d].\n", outstrings[i], i);
      outstrings[i][0] = '\0';
      n_outstrings[i] = outstrings[i];
      if (*t != '\0') 
      { putc (*t, outfile); t++; }  /* terminator */
    }
    fprintf (outfile, "\\en");
    if (debug)
    { fprintf (logfile, "\\en");
      fprintf (logfile, "\nAfter termination of notes command:\n");
      status_all ();
    }
  }

  if (debug) fprintf (logfile, "\nCreate a new notes command for the appoggiatura:\n");
  spacing = APPOGG_SPACING;
  initialize_notes ();
  spacing = MAX_SPACING;
  for (i=1; i <= nstaffs; i++)
    if (active[i])
    { 
      if (spacings[i] == APPOGG_SPACING )
      {
        output_notes (i); 
        spacings[i] = spacing_note (i);
      }
      if (spacings[i] < spacing)
        spacing = spacings[i];
    }
  appoggiatura = false;
  nonvirtual_notes = true;
  old_spacing = APPOGG_SPACING;
  if (debug)
  { fprintf (logfile, "\nAfter process_appogg:\n");
    status_all ();
  }
  return;
}


PRIVATE
void generate_notes ()
{ int i;
  bool xtuplet_flag;
  while (true)
  { old_spacing = spacing; 
    spacing = MAX_SPACING;
    spacing_staff = 0;
    global_skip = 0;
    nonvirtual_notes = false;
    if (debug)
    { fprintf (logfile, "\nIn generate_notes:\n");
      status_all ();
    }
    for (i=1; i <= nstaffs; i++) 
      if (active[i])
      {
        spacings[i] = spacing_note (i);
        if (spacings[i] < spacing)
        {
          spacing = spacings[i];
          spacing_staff = i;
        }
        else if (spacings[i] == spacing && 
                  (vspacing[spacing_staff] > 0 || 
                   /* for staffs with equal spacing, use the one with larger instrument size  */
                   instr_numsize[staff_instr[i]] > instr_numsize[staff_instr[spacing_staff]])) 
          spacing_staff = i;
      }
    if (appoggiatura)
    {
      process_appogg ();
    }
    xtuplet_flag = false;
    for (i=1; i <= nstaffs; i++)
      if (xtuplet[i] > 1) xtuplet_flag = true;
    if (xtuplet_flag)
    {
      process_xtuplet ();
      continue;
    }
    
    if (spacing != old_spacing || spacing == MAX_SPACING || spacing_staff != oldspacing_staff)  
    { 
      if (old_spacing < MAX_SPACING) 
        terminate_notes ();
      if (spacing == MAX_SPACING || nonvirtual_notes == false) 
      {
        if (debug)
        { fprintf (logfile, "\nAfter generate_notes:\n");
          status_all ();
        }
        return;
      }
      if (old_spacing < MAX_SPACING) putc ('\n', outfile);

      initialize_notes ();
    }

    for (i=1; i <= nstaffs; i++)  /* append current notes to outstrings */
      if (active[i]) 
        output_notes (i);
    for (i=1; i <= nstaffs; i++)
    {
      /* virtual notes needed?  */
      if (active[i] && spacings[i] != MAX_SPACING && spacings[i] != spacing && vspacing[i] == 0 )
      {
        vspacing[i] = spacings[i];
        vspacing_active[i] = true;
        vspacing[i] = vspacing[i] - spacing;
        if (debug)
        { fprintf (logfile, "\nAfter vspacing initialization:\n");
          status (i);
        }
      } 
    }
  }
}


void process_command (char **ln)
{ char *s, *t;

  if (debug)
  { fprintf (logfile, "\nProcessing command:%s\n", *ln);
    fflush (logfile);
  }
  if ( prefix("\\instrumentnumber", *ln) )
  { 
    s = strpbrk (*ln, "123456789");
    if ( s == NULL ) error ("\\instrumentnumber command unreadable.");
    ninstr = atoi (s);
    while (*ln <= s) { putc (**ln, outfile); (*ln)++;}
  }

  if ( prefix("\\def\\nbinstrument", *ln) )
  { 
    s = strpbrk (*ln, "123456789");
    if ( s == NULL ) error ("\\def\\nbinstrument command unreadable.");
    ninstr = atoi (s);
    while (*ln <= s) { putc (**ln, outfile); (*ln)++;}
  }

  else if ( prefix("\\setstaffs", *ln) )
  { /* for now, just record in staffs[i]  */
    int n, p;
    s = strpbrk (*ln, "123456789");
    if ( s == NULL ) error ("\\setstaffs command unreadable.");
    n = (int)(*s) - (int)('0'); /* instrument number  */
    s = strpbrk (s+1, "123456789");
    if ( s == NULL ) error ("\\setstaffs command unreadable.");
    p = (int)(*s) - (int)('0'); /* number of staffs */
    staffs[n] = p;
    while (*ln <= s) { putc (**ln, outfile); (*ln)++;}
  }

  else if ( prefix("\\setsize", *ln) )
  { int n; char *p;
    s = strpbrk (*ln, "123456789");
    if ( s == NULL ) error ("\\setsize command unreadable.");
    n = (int)(*s) - (int)('0'); /* instrument number  */
    s++;
    if (*s == '}') s++;
    if (*s == '{') s++;
    t = strpbrk (s, " }\n");
    if ( t == NULL ) error ("\\setsize command unreadable.");
    instrument_size[n][0] = '\0';
    p = instrument_size[n];
    while (s < t) 
    { *p = *s;
       p++; s++;
    }
    *p = '\0';
    if (debug)
    {
      fprintf (logfile, "instrument_size[%d] = %s\n", n, instrument_size[n]); 
      fflush (logfile);
    }
    /* determine numerical instrument size to allow numerical comparison  */
    if (prefix ("\\normalvalue", instrument_size[n]))
      instr_numsize[n] = 1.0;
    else if (prefix ("\\smallvalue", instrument_size[n]))
      instr_numsize[n] = 0.8;
    else if (prefix ("\\tinyvalue", instrument_size[n]))
      instr_numsize[n] = 0.64;
    else if (prefix ("\\largevalue", instrument_size[n]))
      instr_numsize[n] = 1.2;
    else if (prefix ("\\Largevalue", instrument_size[n]))
      instr_numsize[n] = 1.44;
    else 
      error ("\\setsize argument unreadable.");    
    if (debug)
    {
      fprintf (logfile, "instr_numsize[%d] = %f\n", n, instr_numsize[n]);
      fflush (logfile);
    }
    while (*ln <= s) { putc (**ln, outfile); (*ln)++;}
  }

  else if ( prefix("\\startpiece", *ln) )
  { 
    if (!TransformNotesDefined) /* create default TransformNotes2:  */
    { int i, j;
      t = TransformNotes2;
      nstaffs = 1; 
      sprintf (t, "#%1i", nstaffs+1); t = t+2; 
      for (j=2; j <= staffs[1]; j++)
      {  nstaffs++; sprintf (t, "|#%1i", nstaffs+1); t = t+3; 
         active[nstaffs] = true; 
      }
      for (i=2; i <= ninstr; i++) 
      { nstaffs++; sprintf (t, "&#%1i", nstaffs+1); t = t+3; 
        for (j=2; j <= staffs[i]; j++)
        {  nstaffs++; sprintf (t, "|#%1i", nstaffs+1); t = t+3; 
           active[nstaffs] = true;
        }
      }   
      nastaffs = nstaffs;
      if (debug)
        fprintf (logfile, "default TransformNotes2=%s\n", TransformNotes2);
    }
    if (nstaffs == 1) fprintf (outfile, "\\nostartrule\n");
    if (debug)
    {
      int j;
      fprintf (logfile, "ninstr=%d nstaffs=%d nastaffs=%d\n", ninstr, nstaffs, nastaffs);
      for (j=1; j <= nstaffs; j++)
        fprintf (logfile, "active[%d]=%d\n", j, active[j]);
    }
    fprintf (outfile, "\\startpiece");
    t = strpbrk (*ln+1, "\\%\n");
    *ln = t;
  }

  else if ( prefix("\\startextract", *ln) )
  { 
    if (!TransformNotesDefined) /* create default TransformNotes2:  */
    { int i, j;
      t = TransformNotes2;
      nstaffs = 1;
      sprintf (t, "#%1i", nstaffs+1); t = t+2; 
      for (j=2; j <= staffs[1]; j++)
      {  nstaffs++; sprintf (t, "|#%1i", nstaffs+1); t = t+3; 
         active[nstaffs] = true;
      }
      for (i=2; i <= ninstr; i++) 
      { nstaffs++; sprintf (t, "&#%1i", nstaffs+1); t = t+3; 
        for (j=2; j <= staffs[i]; j++)
        {  nstaffs++; sprintf (t, "|#%1i", nstaffs+1); t = t+3; 
           active[nstaffs] = true;
        }
      }   
      nastaffs = nstaffs;
      if (debug)
        fprintf (logfile, "default TransformNotes2=%s\n", TransformNotes2);
    }
    if (nstaffs == 1) fprintf (outfile, "\\nostartrule\n");
    fprintf (outfile, "\\startextract");
    t = strpbrk (*ln+1, "\\%\n");
    *ln = t;
  }

  else if ( prefix("\\TransformNotes", *ln) ) 
  { /* determine ninstr, nstaffs, nastaffs, staffs[i], and active[i] */
    int i;
    s = *ln + 16;  /* first parameter  */
    ninstr = 0;
    nstaffs = 0;
    while (true)
    { ninstr++; nstaffs++;
      staffs[ninstr] = 1;
      checkc (s, '#'); s++;
      checkn (s); s++;
      while (*s == '|')
      { staffs[ninstr]++; nstaffs++;
        s++;
        checkc (s, '#'); s++;
        checkn (s); s++; 
      }
      if (*s != '&') break;
      s++;
    }
    checkc (s, '}'); s++;
    s = strchr (s, '{');
    if (s == NULL) error ("Can't parse \\TransformNotes");
    s++;
    /*  determine TransformNotes2:  */
    t = TransformNotes2;
    do { *t = *s; t++; s++; }
    while (*s != '}');
    *t = '\0';  /* terminate TransformNotes2 */
    TransformNotesDefined = true;
    if (debug)
      fprintf (logfile, "defined TransformNotes2=%s\n", TransformNotes2);
    
    /* determine active staffs:  */
    for (i=1; i <= nstaffs; i++) active[i] = false;
    nastaffs = 0;
    t = strpbrk (TransformNotes2, "#}"); /* may have \transpose etc. before # */
    if (t == NULL) error ("Can't parse second argument of \\TransformNotes");
    while (*t == '#')
    { if (sscanf (t, "#%d", &i) != 1) 
        error ("sscanf for argument number fails");
      active[i-1] = true;  /* parameters start at 2 */
      nastaffs++;
      t = strpbrk (t+1, "#}"); 
      if (t == NULL) break;
    }
    if (debug)
    {
      int j;
      fprintf (logfile, "ninstr=%d nstaffs=%d nastaffs=%d\n", ninstr, nstaffs, nastaffs);
      for (j=1; j <= nstaffs; j++)
        fprintf (logfile, "active[%d]=%d\n", j, active[j]);
    }

    /* output \TransformNotes...  as a comment:  */
    putc ('%', outfile);  
    while (*ln <= s) 
    { putc (**ln, outfile); (*ln)++; }
  }

  else if (prefix("\\def\\atnextbar{\\znotes", *ln))
  { /*  whole-bar or multi-bar rest? */
    int i;
    bool centerpause = false;
    *ln = *ln + 15;  /*  skip "\def\atnextbar{"  */
    analyze_notes(ln);
    for (i=1; i <= nstaffs; i++)
    { 
      if (active[i]) 
      { char *t; 
        char *s; 
        t = strpbrk (notes[i], "&|$");
        if (t == NULL) t = notes[i] + strlen (notes[i]);
        s = strstr (notes[i], "\\centerpause");
        if (s != NULL && s < t && nastaffs == 1)
        {
          bar_rest[i] = true;
          centerpause = true;
        }
        break;
      }
    }
    if (!centerpause) /* generate transformed \znotes ... \en */
    { 
      fprintf ( outfile, "\\def\\atnextbar{\\znotes");
      t = TransformNotes2;
      while (true)
      {
        /*  output TransformNotes2 prefix (\transpose etc.):  */
        s = strchr (t, '#');
        if (s == NULL) break;
        while (t < s)  
        { putc (*t, outfile); t++; }
        t++;  /* skip '#' */
        /* output notes: */
        i = atoi (t) - 1; t++;  
        s = notes[i];
        while (*s != '&' && *s != '|' && *s != '$')
        { putc (*s, outfile); s++; }
        if (*t != '\0') 
        { putc (*t, outfile); t++; } /* output terminator */
      }
      fprintf (outfile, "\\en}%%\n");
    }
    t = strchr (*ln, '}');
    if (t == NULL) error ("Can't find }.");
    *ln = t+1;
    t = strpbrk (*ln, "%\\");
    if (t == NULL ) t = *ln + strlen(*ln);
    *ln = t;
  }

  else if ( prefix("\\def", *ln) )  /* copy to output */
  { t = *ln + strlen(*ln);
    while (*ln < t)
    { fputc (**ln, outfile); 
      (*ln)++;
    }
  }

  else if ( prefix("\\anotes", *ln) )
  { if (debug)
    { fprintf (logfile, "\nProcessing %s", *ln);
      fprintf (logfile, "lineno=%d\n",  lineno);
      fflush (logfile);
    }
    analyze_notes (ln);
    if (debug)
    { fprintf (logfile, "\nBefore generate_notes *ln=%s\n", *ln);
      fflush (logfile);
    }
    generate_notes (); 
    if (debug)
    { fprintf (logfile, "\nAfter generate_notes *ln=%s\n", *ln);
      fflush (logfile);
    }
    t = strpbrk (*ln, "%\\\n");
    if (t == NULL) t = *ln + strlen (*ln);
    if (*t == '\n') putc ('%', outfile);
    *ln = t;
  }
  else if ( prefix("\\nnnotes", *ln) ||
            prefix("\\nnnotes", *ln) ||
            prefix("\\nnotes", *ln)  ||
            prefix("\\notes", *ln)   || 
            prefix("\\Notes", *ln)   ||
            prefix("\\NOtes", *ln)   ||
            prefix("\\NOTes", *ln)   ||
            prefix("\\NOTEs", *ln)   || 
            prefix("\\znotes", *ln)  )
  { 
    char *s, *t;
    int nstaff;
    if (debug)
    { fprintf (logfile, "\nProcessing %s", *ln);
      fprintf (logfile, "lineno=%d\n",  lineno);
      fflush (logfile);
    }
    if (nastaffs == 1 && restbars > 0)
      output_rests ();
    t = *ln+1;
    while ( isalpha (*t) ) t++;
    s = *ln;
    while (s < t) { putc (*s, outfile); s++; }

    analyze_notes (ln);

    t = TransformNotes2;
    while (true)
    {
      /*  output TransformNotes2 prefix (\transpose etc.):  */
      s = strchr (t, '#');
      if (s == NULL) break;
      while (t < s)  
      { putc (*t, outfile); t++; }

      t++;  /* skip '#' */

      /* output notes: */
      nstaff = atoi (t) - 1; t++;  
      s = notes[nstaff];
      while (*s != '&' && *s != '|' && *s != '$')
      { putc (*s, outfile); s++; }
      if (*t != '\0') 
      { putc (*t, outfile); t++; } /* output terminator */
    }
    fprintf (outfile, "\\en\n");

    t = strpbrk (*ln, "%\\\n");
    if (t == NULL) t = *ln + strlen (*ln);
    if (*t == '\n') putc ('%', outfile);
    *ln = t;
  }

  else if (prefix("\\vnotes", *ln) )
  {
    char *s, *t;
    int nstaff;
    if (debug)
    { fprintf (logfile, "\nProcessing %s", *ln);
      fprintf (logfile, "lineno=%d\n",  lineno);
      fflush (logfile);
    }
    s = *ln;
    t = strchr (*ln+1, '\\');  /* find \elemskip  */
    *ln = t;
    t = t + 9;
    while (s < t) { putc (*s, outfile); s++; }

    
    analyze_notes (ln);

    t = TransformNotes2;
    while (true)
    {
      /*  output TransformNotes2 prefix (\transpose etc.):  */
      s = strchr (t, '#');
      if (s == NULL) break;
      while (t < s)  
      { putc (*t, outfile); t++; }

      t++;  /* skip '#' */

      /* output notes: */
      nstaff = atoi (t) - 1; t++;  
      s = notes[nstaff];
      while (*s != '&' && *s != '|' && *s != '$')
      { putc (*s, outfile); s++; }
      if (*t != '\0') 
      { putc (*t, outfile); t++; } /* output terminator */
    }
    fprintf (outfile, "\\en\n");

    t = strpbrk (*ln, "%\\\n");
    if (t == NULL) t = *ln + strlen (*ln);
    if (*t == '\n') putc ('%', outfile);
    *ln = t;
  }

  else if ( prefix ("\\bar", *ln) && !prefix ("\\barno", *ln))
  { int i;
    char *s, *t;
    bool atnextbar = false; 
    for (i=1; i <= nstaffs; i++)
      if (active[i] && bar_rest[i]) 
      { atnextbar = true; break; }
    if (nastaffs == 1 && atnextbar)
    { restbars++;
      sprintf (deferred_bar, "\\bar");
      if (debug) fprintf (logfile, "\nrestbars increased to %d\n", restbars);
      for (i=1; i<= nstaffs; i++)
        bar_rest[i] = false;
      *ln = *ln+4;
      t = strpbrk (*ln, "\\\n");
      if (t == NULL) t = *ln + strlen (*ln);
      if (*t == '\n') putc ('%', outfile);
      *ln = t;
    }
    else
    { if (atnextbar)
      { fprintf (outfile, "\\def\\atnextbar{\\znotes");
        t = TransformNotes2;
        while (true)
        {
          s = strchr (t, '#');
          if (s == NULL) 
            break;
          while (t < s)  /* output any initial \transpose etc. */
          { putc (*t, outfile); t++; }
          t++; /* skip # */
          i = atoi (t) -1; t++;
          if (active[i])
          {
            if (bar_rest[i])
              fprintf (outfile, "\\centerpause");
            bar_rest[i] = false;
          }
          if (*t != '\0') 
          { putc (*t, outfile); t++; }  /* terminator */
        }
        fprintf (outfile, "\\en}%%\n");
      }
      if (Changeclefs) /* \Changeclefs has to be output after \def\atnextbar...  */
      {
        fprintf ( outfile, "\\Changeclefs%%\n");
        Changeclefs = false;
      }
      t = strpbrk (*ln+1, "%\\\n");
      while (*ln < t)
      { fputc (**ln, outfile);
        (*ln)++;
      }
      if (*t == '\n') putc ('%', outfile);
      *ln = t;
    }
    if (debug)
    { fprintf (logfile, "\nAfter \\bar processing, *ln=%s\n", *ln);
      fflush (logfile);
    }
  }

  else if ( prefix ("\\endpiece", *ln)
         || prefix ("\\Endpiece", *ln)
         || prefix ("\\endextract", *ln)
         || prefix ("\\stoppiece", *ln)
         || prefix ("\\raggedstoppiece", *ln)
         || prefix ("\\rightrepeat", *ln)
         || prefix ("\\leftrepeat", *ln)
         || prefix ("\\alaligne", *ln)
         || prefix ("\\alapage", *ln)
         || prefix ("\\changecontext", *ln)
         || prefix ("\\Changecontext", *ln)
         || prefix ("\\zchangecontext", *ln)
         || prefix ("\\zalaligne", *ln)
         || prefix ("\\zalapage", *ln) )
  { int i;
    bool atnextbar = false; 
    for (i=1; i <= nstaffs; i++)
      if (active[i] && bar_rest[i]) 
      { atnextbar = true; break; }
    if (nastaffs == 1 && atnextbar)
    { restbars++;
      sprintf (deferred_bar, "%s", *ln);
      output_rests ();
      t = strpbrk (*ln+1, "%\\\n");
      if (*t == '\n') putc ('%', outfile);
      *ln = t;
    }
    else if (atnextbar)
    {
      fprintf (outfile, "\\def\\atnextbar{\\znotes");
      t = TransformNotes2;
      while (true)
      {
        s = strchr (t, '#');
        if (s == NULL) 
          break;
        while (t < s)  /* output any initial \transpose etc. */
        { putc (*t, outfile); t++; }
        t++; /* skip # */
        i = atoi (t) -1; t++;
        if (active[i])
        {
          if (bar_rest[i])
            fprintf (outfile, "\\centerpause");
          bar_rest[i] = false;
        }
        if (*t != '\0') 
        { putc (*t, outfile); t++; }  /* terminator */
      }
      fprintf (outfile, "\\en}%%\n");
      if (Changeclefs) /* \Changeclefs has to be output after \def\atnextbar...  */
      {
        fprintf ( outfile, "\\Changeclefs%%\n");
        Changeclefs = false;
      }
      t = strpbrk (*ln+1, "%\\\n");
      while (*ln < t)
      { fputc (**ln, outfile);
        (*ln)++;
      }
      if (*t == '\n') putc ('%', outfile);
      *ln = t;
    }
    else
    { if (nastaffs == 1 && restbars > 0)
        output_rests ();
      t = strpbrk (*ln+1, "%\\\n");
      while (*ln < t)
      { fputc (**ln, outfile);
        (*ln)++;
      }
      if (*t == '\n') putc ('%', outfile);
      *ln = t;
    }
    if (debug)
    { fprintf (logfile, "\nAfter \\bar processing, *ln=%s\n", *ln);
      fflush (logfile);
    }
  }

  else if  ( prefix ("\\znotes", *ln) )
  {
    if (nastaffs == 1 && restbars > 0)
      output_rests ();
    fputs (*ln, outfile);
    *ln = *ln + strlen(*ln);
  }
 
  else if ( prefix ("\\Changeclefs", *ln) )
  {  /* defer till after \def\atnextbar ... */
    Changeclefs = true;
    *ln = *ln + strlen(*ln);
  }

  else if ( prefix ("\\documentclass", *ln) )
  {
    
    if ( suffix( ".tex", outfilename) )
    { char newoutfilename[SHORT_LEN];
      char *newoutfilename_n = newoutfilename;
      append (newoutfilename, &newoutfilename_n, infilename, sizeof (newoutfilename));
      newoutfilename_n -= 4;
      *newoutfilename_n = '\0';
      append (newoutfilename, &newoutfilename_n, "ltx", sizeof (newoutfilename));
      if (rename ( outfilename, newoutfilename) > 0)
      { printf ("Can't rename %s as %s\n", outfilename, newoutfilename);
        exit (EXIT_FAILURE);
      }
      printf ("\\documentclass detected; now writing to %s.", newoutfilename);
      printf ("\n\n");

    }
    fputs (*ln, outfile);
    *ln = *ln + strlen(*ln);
  }

  else if ( prefix ("\\end ", *ln) 
         || prefix ("\\end%", *ln) 
         || prefix ("\\end\n", *ln) 
         || prefix ("\\end{document}", *ln) )
  {
    fprintf (outfile, "%s", *ln);
    exit(0);
  }

  else if ( prefix ("\\startmuflex", *ln) )
  {
    fputs ("\\startmuflex", outfile);
    *ln = *ln + 12;
  }

  else  /* everything else */
  { 
    fputs (*ln, outfile);
    *ln = *ln + strlen(*ln);
  }
}

