# include "autosp.h"

# define MAX_STAFFS 9

# define SMALL_NOTE 512
# define SP(note) (SMALL_NOTE/note)  
/* note = { 256 | 128 | 64 | 32 | 16 | 8 | 4 | 2 | 1 }  */
# define MAX_SPACING 2*SP(1)
int staffs[MAX_STAFFS]; /* number of staffs for ith instrument*/
bool active[MAX_STAFFS];        /* is staff i active?                 */
bool bar_rest[MAX_STAFFS];
int spacing;                    /* spacing for current notes          */
int restbars;
char global_skip_str[7][16];   
int ninstr;                 /* number of instruments              */
int nstaffs;                /* number of staffs                   */
int nastaffs;               /* number of active staffs;           */
int old_spacing;
int oldspacing_staff;
bool Changeclefs;               /* output \Changeclefs after \def\atnextbar */
char TransformNotes2[SHORT_LEN];      /* 2nd argument of \TransformNotes    */
bool TransformNotesDefined;
char instrument_size[MAX_STAFFS][SHORT_LEN];
     /* instrument size as set by setsize or by default ("\\@one") */
float instr_numsize[MAX_STAFFS];
     /* nummerical intrument size */
int staff_instr[MAX_STAFFS];  /* instrument for each staff */

char line[LINE_LEN];            /* line of input                      */

void process_line ();

void process_command (char **ln);

void process_line ()
{
  char  *ln; 
  ln = &line[0];
  while ( *ln != '\0') 
  {
    while (*ln == ' ') { ln++; fputc (' ', outfile); }
    if (*ln == '%') {
      fprintf (outfile, "%s", ln);
      return;
    }
    process_command (&ln);
  }     
}
void process_score ()
/* process .aspc file */
{
  int c; int i;
  lineno = 0;
  for (i=1; i < MAX_STAFFS; i++) 
  {
    staffs[i] = 1; 
    active[i] = true;
    bar_rest[i] = false;
    append (instrument_size[i], NULL, "\\normalvalue", SHORT_LEN);
    instr_numsize[i] = 1.0;
  }
  spacing = MAX_SPACING;
  restbars = 0;
  ninstr = 1;                 /* number of instruments              */
  nstaffs = 1;                /* number of staffs                   */
  nastaffs = 1;               /* number of active staffs;           */
  old_spacing = MAX_SPACING;
  oldspacing_staff = 0;
  Changeclefs = false; /* output \Changeclefs after \def\atnextbar */
  
  TransformNotes2[0] = '\0';      
  TransformNotesDefined = false;

  append (global_skip_str[0], NULL,  "", 16);
  append (global_skip_str[1], NULL, "\\qqsk", 16);
  append (global_skip_str[2], NULL, "\\hqsk", 16);
  append (global_skip_str[3], NULL,  "\\tqsk", 16);
  append (global_skip_str[4], NULL, "\\qsk", 16);
  append (global_skip_str[5], NULL, "\\qsk\\qqsk", 16);
  append (global_skip_str[6], NULL, "\\qsk\\hqsk", 16);

  c = getc (infile);
  while ( c != EOF )
  {
    ungetc (c, infile);
    if (fgets(line, LINE_LEN, infile) == NULL) 
      error ("Unexpected EOF.");
    if (strlen (line) == LINE_LEN-1) 
      error ("Line too long.");
    lineno++;
    process_line ();
    c = getc (infile);
  }     /* c == EOF  */
}     

