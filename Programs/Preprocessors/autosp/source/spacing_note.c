# include "process_command.h"

PRIVATE
void update_global_skip (int n)
/* global skips may be used in more than one staff */
{  
  if (n>6) 
  { warning ("Too many commas."); n = 6; }
  if (global_skip < n) global_skip = n; 
}

PRIVATE
int collective_note (int i)
{ char *s = current[i];
  int spacing = cspacing[i];
  if (debug)
  { fprintf (logfile, "\nEntering collective_note:\n");
    status (i);
    status_beam (i);
    status_collective (i);
  }
  if (beaming[i] > 0 &&  spacing != beaming[i])  
  { /* adjust cspacing for new beaming */
     cspacing[i] = beaming[i];   
  }
  while (true) /* search for alphabetic or numeric note (or asterisk) */
  { int n = 0;
    while (*s == ',') 
    { n++; s++; } /* global skips */
    update_global_skip (n);
      /* commas will be discarded by filter_output (i) */
    if (*s == '.' && new_beaming == 0 && !dottedbeamnotes) 
      spacing = spacing * 1.50; 
    else if ( (*s == '^' || *s == '_' || *s == '=' || *s == '>') 
              && !vspacing_active[i]  /* is additional spacing needed? */
            )
    /* leave space for normal accidentals */
    { update_global_skip (3) ;
      if (debug)
      { fprintf (logfile, "\nLeave space for accidental\n");
        status (i);
        status_beam (i);
        status_collective (i);
      }
    }
    else if ( (*s == '<') /* double-flat */ 
              && !vspacing_active[i]  /* is additional spacing needed? */
            )
    { update_global_skip (5);
      if (debug)
      { fprintf (logfile, "\nLeave space for double-flat\n");
        status (i);
        status_beam (i);
        status_collective (i);
      }
    }
    else if (isalnum (*s) || *s == '*')
    {  
      s++; 
      while (*s == '\'' || *s == '`' || *s == '!') 
      /* transposition characters */
        s++;
      current[i] = s;
      if (debug)
      { fprintf (logfile, "\nAfter collective_note:\n");
        status (i);
        status_collective (i);
      }
      vspacing_active[i] = false;
      return spacing; 
    }
    s++;
  }
}


PRIVATE
void beam_initiation (char *s, int i)
{ 
  if ( prefix ("\\ibbbbbb", s)
    || prefix ("\\Ibbbbbb", s)
    || prefix ("\\nbbbbbb", s) )
    beaming[i] = SP(256); 
  else if ( prefix ("\\ibbbbb", s)
    || prefix ("\\Ibbbbb", s)
    || prefix ("\\nbbbbb", s) )
    beaming[i] = SP(128); 
  else if ( prefix ("\\ibbbb", s)
    || prefix ("\\Ibbbb", s)
    || prefix ("\\nbbbb", s) )
    beaming[i] = SP(64); 
  else if ( prefix ("\\ibbb", s) 
    || prefix ("\\Ibbb", s)
    || prefix ("\\nbbb", s) ) 
    beaming[i] = SP(32); 
  else if ( prefix ("\\ibb", s) 
    || prefix ("\\Ibb", s)
    || prefix ("\\nbb", s) )
    beaming[i] = SP(16); 
  else if ( prefix ("\\ib", s) 
    || prefix ("\\Ib", s) )
    beaming[i] = SP(8); 
  if (debug)
  { fprintf (logfile, "\nAfter beam_initiation:\n");
    status (i);
    status_beam (i);
  }
}

PRIVATE
void beam_termination (char *s, int i)
{
  if (debug)
  { fprintf (logfile, "\nEntering beam_termination:\n");
    status (i);
    status_beam (i);
  }
  if ( prefix ("\\tbbbbbb", s) )
  { 
    new_beaming = SP(128);
    if (beaming[i] > SP(256))
      beaming[i] = SP(256);
  }
  else if ( prefix ("\\tbbbbb", s) )
  { 
    new_beaming = SP(64);
    if (beaming[i] > SP(128))
      beaming[i] = SP(128);
  }
  else if ( prefix ("\\tbbbb", s) )
  { 
    new_beaming = SP(32);
    if (beaming[i] > SP(64))
      beaming[i] = SP(64);
  }
  else if ( prefix ("\\tbbb", s) )
  { 
    new_beaming = SP(16);
    if (beaming[i] > SP(32))
      beaming[i] = SP(32);
  }
  else if ( prefix ("\\tbb", s) ) 
  { 
    new_beaming = SP(8);
    if (beaming[i] > SP(16))
      beaming[i] = SP(16);
  }
  else if ( prefix ("\\tb", s) )
  {
    new_beaming = 0; 
  }
  if (debug)
  { fprintf (logfile, "\nAfter beam_termination:\n");
    status (i);
    status_beam (i);
  }
}

PRIVATE
char *skip_balanced_text (char *s)
{
  char *t;
  do
  {
    t = strpbrk (s, "{}");
    if (t == NULL) error ("Expected '}'");
    if (*t == '{') t = skip_balanced_text (t+1);
  }
  while (*t != '}');
  return t+1;
}

PRIVATE 
char *skip_arg (char *s)
{ 
  while (*s == ' ') s++;
  if (*s == '{')
  { s++;
    skip_balanced_text (s);
  }
  else  
    s++;
  return s;
}

int spacing_note (int i)
/*  Search from current[i] for the next spacing note in the ith staff.  
 *  Returns the spacing value and updates current[i].
 */
{ 
  int spacing = MAX_SPACING;
  char *s, *t;
  bool tinynotesize = false;
  bool doubledotted = false;
  bool dotted = false;
  new_beaming = 0;
  if (!active[i]) return spacing;
  if (vspacing[i] > 0) 
  { /* virtual note (i.e., a skip) */
    return vspacing[i]; 
  } 
  if (collective[i][0])
  { 
    nonvirtual_notes = true;
    return collective_note (i); 
  }
  if (semiauto_beam_notes[i] > 0)
  { if (debug) 
      fprintf (logfile, "\nsemiauto_beam_notes reduced\n");
    semiauto_beam_notes[i]--;
    current[i]++;
    if (debug) status (i);
    nonvirtual_notes = true;
    return beaming[i];
  }
  s = strpbrk (current[i], "\\&|$");
  if (*s != '\\') 
  { return spacing; }
  if (debug)
  { fprintf (logfile, "\nIn spacing_note:\n");
    status (i);
  }
  while (true)
  { 
    if ( prefix ("\\wh", s)
      || prefix ("\\breve", s) 
      || prefix ("\\pause", s) 
      || prefix ("\\wq", s) )
    { spacing = SP(1); break; }
    
    if ( prefix ("\\ha", s)
      || (prefix ("\\hl", s)  && !prefix ( "\\hloff", s) )
         
      || prefix ("\\hu", s)
      || prefix ("\\hp", s)
      || prefix ("\\hpause", s) )
    { spacing = SP(2); break;}

    if ( prefix ("\\qa", s)
      || prefix ("\\ql", s)
      || prefix ("\\qu", s)
      || prefix ("\\qp", s) )
    { spacing = SP(4); break; }

    if ( ( prefix ("\\ca", s) 
        || prefix ("\\cl", s)
        || prefix ("\\cu", s) )
      && !prefix ("\\caesura", s ) )
    { spacing = SP(8); break; }

    if ( prefix ("\\ds", s ) 
       && !prefix ("\\dsh", s) ) 
    { spacing = SP(8); break; }

    if ( prefix ("\\cccc", s) ) 
    { spacing = SP(64); break; }

    if ( prefix ("\\ccc", s) ) 
    { spacing = SP(32); break; }

    if ( prefix ("\\cc", s) 
       && !prefix ("\\ccn", s) 
       && !prefix ("\\cchar", s) ) 
    { spacing = SP(16); break; }

    if ( prefix ("\\qs", s)
       && !prefix ("\\qsk", s) 
       && !prefix ("\\qspace", s) )
    { spacing = SP(16); break; }

    if ( prefix ("\\hs", s) 
       && !prefix ("\\hsk", s)  
       && !prefix ("\\hsp", s) ) 
    { spacing = SP(32); break;}

    if ( prefix ("\\qqs", s )
       && !prefix ("\\qqsk", s) )
    { spacing = SP(64); break; }

    if ( prefix ("\\qb", s))  /* beam note */
    { spacing = beaming[i]; 
      if (new_beaming != 0) /* set by preceding \tb... */
      { beaming[i] = new_beaming; 
        new_beaming = 0; 
      }
      if (debug)
      { fprintf (logfile, "\nBeam note:\n");
        status (i);
        status_beam (i);
      }
      break; 
    }
    
    if ( prefix ("\\Cpause", s) )
    { /* bar-centered rest */
      bar_rest[i] = true;
      spacing = MAX_SPACING;
      break;
    }

    if (prefix("\\tqqq", s) )
    { if (beaming[i] > SP(32)) 
        beaming[i] = SP(32);
      spacing = beaming[i];
      beaming[i] = 0;
      new_beaming = 0;
      if (debug)
      { fprintf (logfile, "\nAfter beam completion:\n");
        status (i);
        status_beam (i);
      }
      break;
    }

    if (prefix("\\nqqq", s) )
    { if (beaming[i] > SP(32)) 
        beaming[i] = SP(32);
      spacing = beaming[i];
      beaming[i] = SP(16);
      new_beaming = 0;
      if (debug)
      { fprintf (logfile, "\nAfter beam completion:\n");
        status (i);
        status_beam (i);
      }
      break;
    }

    if (prefix("\\tqq", s) )
    { if (beaming[i] > SP(16)) 
        beaming[i] = SP(16);
      spacing = beaming[i];
      beaming[i] = 0;
      new_beaming = 0;
      if (debug)
      { fprintf (logfile, "\nAfter beam completion:\n");
        status (i);
        status_beam (i);
      }
      break;
    }

    if (prefix("\\nqq", s) )
    { if (beaming[i] > SP(16)) 
        beaming[i] = SP(16);
      spacing = beaming[i];
      beaming[i] = SP(8);
      new_beaming = 0;
      if (debug)
      { fprintf (logfile, "\nAfter beam completion:\n");
        status (i);
        status_beam (i);
      }
      break;
    }

    if ( prefix("\\tq", s) 
      && !prefix("\\tqsk", s) )
    { spacing = beaming[i];
      beaming[i] = 0;
      new_beaming = 0;
      if (debug)
      { fprintf (logfile, "\nAfter beam completion:\n");
        status (i); 
        status_beam (i);
      }
      break; 
    }


/*  non-spacing commands:  */

    if ( prefix ("\\ib", s) 
      || prefix ("\\Ib", s)
      || prefix ("\\nb", s) )
      beam_initiation (s, i);

    else if ( prefix("\\tb", s) )
      beam_termination (s, i);

    else if ( prefix("\\ztq", s) )
    /* non-spacing beam termination */
      new_beaming = 0; 

    else if ( prefix("\\xtuplet", s) ||
              prefix("\\xxtuplet", s)||
              prefix("\\Xtuplet", s) )
    { char *t = s+1;
      while (!isdigit(*t)) t++;
      xtuplet[i] = atoi(t);
    }
    else 
    if ( prefix("\\triolet", s)
      || prefix("\\Triolet", s)
      || prefix("\\uptrio", s)
      || prefix("\\downtrio", s)
      || prefix("\\uptuplet", s)
      || prefix("\\downtuplet", s) )
    { 
      xtuplet[i] = 3;
    }

    else if ( prefix("\\zchar", s) 
       || prefix("\\lchar", s)
       || prefix("\\cchar", s) 
       || prefix("\\zcn", s)
       || prefix("\\lcn", s)
       || prefix("\\ccn", s) )
    { /* need to skip two arguments, possibly with embedded commands */
      char *t;
      t  = strpbrk (s+1, " {");
      if (t == NULL) error ("Argument expected");
      t = skip_arg (t);
      t = skip_arg (t);
    }

    else if ( prefix("\\tinynotesize", s) )
    { 
       tinynotesize = true;
       appoggiatura = true;
    }

    else if (prefix("\\ppt", s) 
       || prefix("\\pppt", s) )
    { 
      doubledotted = true;  /* triple-dotted spaced as double-dotted */
    }
    else if (prefix("\\pt", s) && !prefix("\\ptr", s)) 
    {
      dotted = true;
    }
    else if ( prefix ("\\Dqbb", s) )
    { semiauto_beam_notes[i] = 1;
      beaming[i] = SP(16);
      current[i] = current[i] + 8;
      nonvirtual_notes = true;
      return beaming[i];
    }
    else if ( prefix ("\\Dqb", s) )
    { if (debug) fprintf (logfile, "\nFound \\Dqb\n");
      semiauto_beam_notes[i] = 1;
      beaming[i] = SP(8);
      current[i] = current[i] + 7;
      nonvirtual_notes = true;
      if (debug) status (i);
      return beaming[i];
    }
    else if ( prefix ("\\Tqbb", s) )
    { semiauto_beam_notes[i] = 2;
      beaming[i] = SP(16);
      current[i] = current[i] + 8;
      nonvirtual_notes = true;
      return beaming[i];
    }
    else if ( prefix ("\\Tqb", s) )
    { semiauto_beam_notes[i] = 2;
      beaming[i] = SP(8);
      current[i] = current[i] + 7;
      nonvirtual_notes = true;
      return beaming[i];
    }
    else if ( prefix ("\\Qqbb", s) )
    { semiauto_beam_notes[i] = 3;
      beaming[i] = SP(16);
      current[i] = current[i] + 8;
      nonvirtual_notes = true;
      return beaming[i];
    }
    else if ( prefix ("\\Qqb", s) )
    { semiauto_beam_notes[i] = 3;
      beaming[i] = SP(8);
      current[i] = current[i] + 7;
      nonvirtual_notes = true;
      return beaming[i];
    }
    else if (prefix ("\\rlap", s) )
    /*  skip the argument, as it must be regarded as non-spacing  */
    { char *t;
      t  = strpbrk (s+1, " {\\");
      if (t == NULL) error ("Argument expected");
      if (*t == '\\') 
        s = t+1;
      else 
        s = skip_arg(t); 
    }
    else if (prefix ("\\Hsk", s) )
      update_global_skip (5); 
    else if (prefix ("\\Qsk", s) )
      update_global_skip (4); 
    else if (prefix ("\\TQsk", s) )
      update_global_skip (3); 
    else if (prefix ("\\HQsk", s) )
      update_global_skip (2); 
    else if (prefix ("\\QQsk", s) )
      update_global_skip (1);

    /* Command is non-spacing.         */
    /* Skip ahead to the next command. */
    s = strpbrk (s+1, "\\&|$");

    if ( *s != '\\')
    { current[i] = s; 
      if (debug)
      { fprintf (logfile, "\nAfter spacing_note:\n");
        status (i);
      }
      return spacing; 
    }
  } /* end of while (true) loop  */

  if (spacing < MAX_SPACING) 
    nonvirtual_notes = true;
  if (prefix ("\\whpp", s)
   || prefix ("\\hupp", s)
   || prefix ("\\hlpp", s)
   || prefix ("\\happ", s)
   || prefix ("\\hspp", s)
   || prefix ("\\hppp", s)
   || prefix ("\\hpausepp", s)
   || prefix ("\\qupp", s)
   || prefix ("\\qlpp", s)
   || prefix ("\\qapp", s)
   || prefix ("\\qspp", s)
   || prefix ("\\qppp", s)
   || prefix ("\\qqspp", s)
   || prefix ("\\cupp", s)
   || prefix ("\\clpp", s)
   || prefix ("\\ccupp", s)
   || prefix ("\\cclpp", s)
   || prefix ("\\cccupp", s)
   || prefix ("\\ccclpp", s)
   || prefix ("\\qbpp", s) 
   || prefix ("\\dspp", s) 
   || doubledotted  )
  { spacing *= 1.75; doubledotted = false;}
  else 
  if (prefix ("\\whp", s)
   || prefix ("\\hup", s)
   || prefix ("\\hlp", s)
   || prefix ("\\hap", s)
   || prefix ("\\hsp", s)
   || prefix ("\\hpp", s)
   || prefix ("\\hpausep", s)
   || prefix ("\\qup", s)
   || prefix ("\\qlp", s)
   || prefix ("\\qap", s)
   || prefix ("\\qsp", s)
   || prefix ("\\qqsp", s)
   || prefix ("\\qpp", s)
   || prefix ("\\cup", s)
   || prefix ("\\clp", s)
   || prefix ("\\ccup", s)
   || prefix ("\\cclp", s)
   || prefix ("\\cccup", s)
   || prefix ("\\ccclp", s)
   || prefix ("\\qbp", s) 
   || prefix ("\\dsp", s) 
    || dotted  ) 
  { spacing *= 1.5; dotted = false; }

  t = strpbrk (s+1, "{\\&|$"); /* collective coding?  */
  if (*t == '{')  /*  {...}  */
  { /* Save prefix in collective[i].
     * It will be output for every note in the collective */
    char *ss = s; 
    char *tt = collective[i];
    while (ss <= t)
    { *tt = *ss; tt++, ss++; }
    *tt = '\0';
    current[i] = t+1;
    if (tinynotesize) spacing = APPOGG_SPACING;
    cspacing[i] = spacing;
    first_collective[i] = true;
    return collective_note (i);
  } 
  if (tinynotesize) spacing = APPOGG_SPACING; 
  current[i] = strpbrk (s+1, "\\&|$");
  if (debug)
  { fprintf (logfile, "\nAfter spacing_note:\n");
    status (i);
  }
  vspacing_active[i] = false;
  return spacing;
}
