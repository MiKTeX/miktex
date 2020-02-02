# include "process_score.h"

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

