# include "process_command.h"

PRIVATE
void save_state (int i)
/* used in process_xtuplet */
{
  beamingi = beaming[i];
  currenti = current[i];
  cspacingi = cspacing[i];
  vspacingi = vspacing[i];
  first_collectivei = first_collective[i];
  collectivei[0] = '\0';
  append (collectivei, NULL, collective[i], SHORT_LEN);
}

PRIVATE
void restore_state (int i)
/* used in process_xtuplet */
{
  beaming[i] = beamingi;
  current[i] = currenti;
  cspacing[i] = cspacingi;
  vspacing[i] = vspacingi;
  first_collective[i] = first_collectivei;
  collective[i][0] = '\0';
  append (collective[i], NULL, collectivei, SHORT_LEN);
}

PRIVATE
void pseudo_output_notes (int i)
{ /* process notes like output_notes but without outputting  */
  if (vspacing[i]  > 0) 
    vspacing[i] = vspacing[i] - spacing;
  else if (collective[i][0])
  {
    first_collective[i] = false;
    if (*current[i] == '}')
    { collective[i][0] = '\0';
      current[i]++;
    }
  }
}

void process_xtuplet (void)
{ 
  int i, xi=0;
  int xspacing = MAX_SPACING;         /* xtuplet total spacing              */
  int normalized_xspacing;        /* (xspacing / xtuplet) * (xtuplet - 1);  */
  int xsp;
  double multnoteskip;

  if (debug)
  { fprintf (logfile, "\nEntering process_xtuplet:\n");
    for (i=1; i <= nstaffs; i++)
      fprintf (logfile, "i=%d xtuplet[i]=%d\n", i, xtuplet[i]);
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

  if (debug) 
  { fprintf (logfile, "\nDetermine xtuplet duration:\n");
    fflush (logfile);
  }
  for (i=1; i <= nstaffs; i++)
    if (xtuplet[i] > 1)
    {
      xi = i;
      save_state (xi);
      pseudo_output_notes (xi);
      xspacing = spacings[xi];
      while (xspacing % xtuplet[xi] != 0)
      { 
        xspacing +=  spacing_note (xi);
        pseudo_output_notes (xi);
        if (xspacing >= MAX_SPACING) 
          error ("Can't determine xtuplet duration.");
      }
      restore_state (xi);
      break;
    }
  if (debug) fprintf (logfile, "\nxspacing=%s\n", ps(xspacing));
  if (debug) fprintf (logfile, "xi=%d  xtuplet[xi]=%d\n", xi, xtuplet[xi]);
  normalized_xspacing = (xspacing / xtuplet[xi]) * (xtuplet[xi] - 1); 
  if (debug) 
  { fprintf (logfile, "normalized_xspacing=%s\n", ps(normalized_xspacing));
  }
  spacing = xspacing / xtuplet[xi];
  if (debug) 
  { 
    fprintf (logfile, "\nDetermine minimal spacing over all active staffs:\n");
  }
  for (i=1; i <= nstaffs; i++)
    if (active[i] && xtuplet[i] == 1)
    { save_state (i);
      pseudo_output_notes (i);
      xsp = spacings[i]; 
      if (spacings[i] < spacing) 
        spacing = spacings[i];
      while (xsp < normalized_xspacing) 
      { int spi = spacing_note (i);
        pseudo_output_notes (i);
        xsp += spi;
        if (spi < spacing)
        { spacing = spi;
          if (debug) fprintf (logfile, "for i=%d  spi=%s\n", i, ps(spi));
        }
      }
      restore_state (i);
    }
  if (debug) fprintf (logfile, "\nxspacing=%s\n", ps(spacing));

  if (debug) fprintf (logfile, "\nCreate a new notes command for the xtuplet:\n");
  initialize_notes ();


  if (debug) fprintf (logfile, "\nProcess non-xtuplet staffs:\n");
  for (i=1; i <= nstaffs; i++)
    vspacing[i] = 0;
  xsp = 0;
  while (true)
  {
    for (i=1; i <= nstaffs; i++)
      if (active[i] && xtuplet[i] == 1)
        output_notes (i);
    xsp += spacing;
    for (i=1; i <= nstaffs; i++)
    {
      /* virtual notes needed?  */
      if (active[i] && xtuplet[i] == 1 && spacings[i] != 0 && spacings[i] != spacing && vspacing[i] == 0)
      {
        vspacing[i] = spacings[i] - spacing;
      } 
    }
    if (xsp >= normalized_xspacing) break;
    global_skip = 0;
    for (i=1; i <= nstaffs; i++)
      if (active[i] && xtuplet[i] == 1) 
        spacings[i] = spacing_note(i);
  }
  if (debug)
  { fprintf (logfile, "\nAfter processing non-xtuplet staffs:\n");
    status_all ();
  }
  if (debug)
  {
    fprintf (logfile, "Generate \\multnoteskip factors.\n");
    fprintf (logfile, "xi=%d xtuplet[xi]=%d\n", xi, xtuplet[xi]);
  }
  multnoteskip = (double)  (xtuplet[xi] - 1) / xtuplet[xi]; 
  for (i=1; i <= nstaffs; i++)
    if (xtuplet[i] > 1)
    {
      n_outstrings[i] += sprintf (n_outstrings[i], "\\multnoteskip{%5.3f}", multnoteskip); 
      if (debug) 
      { fprintf (logfile, "\noutstrings[%d]=", i);
        note_segment (outstrings[i]); 
      }
    }
  if (debug) fprintf (logfile, "\nRe-process xtuplet staffs:\n");
  for (i=1; i <= nstaffs; i++)
    if (xtuplet[i] > 1)
    {
      xsp = 0;
      while (true)
      { output_notes (i);
        xsp += spacing;
        if (spacings[i] != spacing && vspacing[i] == 0)
          vspacing[i] = spacings[i] - spacing;
        if (xsp >= xspacing) break;
        global_skip = 0;
        spacings[i] = spacing_note (i);
      }
    }

  /* Restore normal \noteskip in the xtuplet staffs. */
  if (debug)
  {
    fprintf (logfile, "Restore \\multnoteskip factors.\n");
    fprintf (logfile, "xi=%d xtuplet[xi]=%d\n", xi, xtuplet[xi]);
  }
  multnoteskip = (double)  xtuplet[xi] / (xtuplet[xi]-1);
  for (i=1; i <= nstaffs; i++)
    if (xtuplet[i] > 1)
    {
      n_outstrings[i] += sprintf (n_outstrings[i], "\\multnoteskip{%5.3f}", multnoteskip); 
      xtuplet[i] = 1;
    }
  nonvirtual_notes = true;
  if (debug)
  { fprintf (logfile, "\nAfter process_xtuplet:\n");
    status_all ();
  }
  return;
}
