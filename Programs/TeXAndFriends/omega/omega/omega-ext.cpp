/* omega-ext.cpp: this is omega.c in the web2c world

This file is part of the Omega project, which
is based on the web2c distribution of TeX.

Copyright (c) 1994--2000 John Plaice and Yannis Haralambous

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */



#if defined(MIKTEX)
#define C4PEXTERN extern
#include "omega-miktex.h"
#define register
#endif

void
btestin(void)
{
    string fname =
    kpse_find_file (nameoffile + 1, kpse_program_binary_format, true);

    if (fname) {
#if 0				// MiKTeX
      libcfree(nameoffile);
      nameoffile = xmalloc(2+strlen(fname));
#endif
      namelength = strlen(fname);
      strcpy(nameoffile+1, fname);
    }
    else {
#if 0				// MiKTeX
      libcfree(nameoffile);
      nameoffile = xmalloc(2);
#endif
      namelength = 0;
      nameoffile[1] = 0;	// MiKTeX
      nameoffile[2] = 0;	// MiKTeX
    }
}

int
getfilemode P2C(alphafile &, f, int, def) // MiKTeX
{
    int c,m;
    if ((def==0)||(feof(f))) m=0;
    else {
       c = getc(f);
       if (c==EOF) m=0;
       else if (c==0x5c) {
          if (feof(f)) m=0;
          else {
             c = getc(f);
             if (c==EOF) m=0;
             else if (c==0) m=4;
             else m=1;
             ungetc(c,f);
             c=0x5c;
          }
       }
       else if (c==0x25) {
          if (feof(f)) m=0;
          else {
             c = getc(f);
             if (c==EOF) m=0;
             else if (c==0) m=4;
             else m=1;
             ungetc(c,f);
             c=0x25;
          }
       }
       else if (c==0xe0) m=2;
       else if (c==0x6c) m=2;
       else if (c==0) {
          if (feof(f)) m=0;
          else {
             c = getc(f);
             if (c==EOF) m=0;
             else if (c==0x5c) m=3;
             else if (c==0x25) m=3;
             else m=def;
             ungetc(c,f);
             c=0;
          }
       }
       else m=def;
       ungetc(c,f);
    }
    return m;
}

int
getc_two_LE P1C(FILE *, f)
{
    register int i,j;
    i = getc(f);
    if (i==EOF) { return i; }
    j = getc(f);
    if (j==EOF) { return j; }
    return ((j<<8)|i);
}

void
ungetc_two_LE P2C(int, c, FILE *, f)
{
    ungetc((c>>8), f);
    ungetc((c&0377), f);
}

int
getc_two P1C(FILE *, f)
{      
    register int i,j;
    i = getc(f);
    if (i==EOF) { return i; }
    j = getc(f);
    if (j==EOF) { return j; }
    return ((i<<8)|j);
}
 
void
ungetc_two P2C(int, c, FILE *, f)
{
    ungetc((c&0377), f);
    ungetc((c>>8), f);
}
 
extern boolean zpnewinputln ();

boolean
newinputln P4C(alphafile &,f, halfword,themode, halfword,translation, boolean,bypass) // MiKTeX
{
    return  pnewinputln((alphafile)f, // MiKTeX
                        (halfword)themode,
                        (halfword)translation,
                        (boolean) bypass);
}
 
boolean
new_input_line P2C(alphafile &, f, halfword, themode) // MiKTeX
{
  register int i=EOF;
 
  last = first;
  otpinputend = 0;
 
  if (themode==1) {
     while ((otpinputend < ocpbufsize) && ((i = getc (f)) != EOF) &&
            ((i != '\r') && (i != '\n')))
        otpinputbuf[++otpinputend] = i;
     if (i=='\r') {
         i=getc(f); if (i != '\n') ungetc(i,f);
     }
  } else if (themode==2) {
     while ((otpinputend < ocpbufsize) && ((i = getc (f)) != EOF) &&
            (i != 0x25))
        otpinputbuf[++otpinputend] = i;
  } else if (themode==3) {
     while ((otpinputend < ocpbufsize) && ((i = getc_two (f)) != EOF) &&
            ((i != '\r') && (i != '\n')))
        otpinputbuf[++otpinputend] = i;
     if (i=='\r') {
         i=getc_two(f); if (i != '\n') ungetc_two(i,f);
     }
  } else /* themode==4 */ {
     while ((otpinputend < ocpbufsize) && ((i = getc_two_LE (f)) != EOF) &&
            ((i != '\r') && (i != '\n')))
        otpinputbuf[++otpinputend] = i;
     if (i=='\r') {
         i=getc_two_LE(f); if (i != '\n') ungetc_two_LE(i,f);
     }
  }
 
  if (i == EOF && otpinputend == 0)
      return false;
 
  /* We didn't get the whole line because our buffer was too small.
*/
 if (i != EOF && (((themode!=2) && (i != '\n')) || ((themode==2) && (i != 0x25))))
    {
      (void) fprintf (stderr,
                     "! Unable to read an entire line---ocp_buf_size=%ld.\n",ocpbufsize);
      (void) fprintf (stderr, "Please increase ocp_buf_size in texmf.cnf.\n");
      uexit (1);
    }
 
    return true;
}

extern memoryword ziniteqtbentry () ;

hashword hashtable[HASHTABLESIZE];

void
inithhashtable P1H(void)
{
   int i;
   for (i=0; i<HASHTABLESIZE; i++) {
      hashtable[i].p = -1;
   }
      
}

hashword *
createeqtbpos P1C(int, p)
{
   hashword *runner= &(hashtable[p%HASHTABLESIZE]);
   if (runner->p==p) return runner;
   while (runner->p != -1) {
      runner = runner->ptr; 
      if (runner->p == p) return runner;
   } 
   runner->p = p;
   runner->mw = ziniteqtbentry(p);
   runner->ptr = (hashword *) xmalloc(sizeof(hashword));
   (runner->ptr)->p = -1;
   return runner;
}

memoryword *
createeqtbptr P1C(int, p)
{
   hashword *runner= &(hashtable[p%HASHTABLESIZE]);
   if (runner->p==p) return (&(runner->mw));
   while (runner->p != -1) {
      runner = runner->ptr; 
      if (runner->p == p) return (&(runner->mw));
   } 
   runner->p = p;
   runner->mw = ziniteqtbentry(p);
   runner->ptr = (hashword *) xmalloc(sizeof(hashword));
   (runner->ptr)->p = -1;
   return (&(runner->mw));
}

hashword *
createxeqlevel P1C(int, p)
{
   hashword *runner= &(hashtable[p%HASHTABLESIZE]);
   if (runner->p==p) return runner;
   while (runner->p != -1) {
      runner = runner->ptr;
      if (runner->p == p) return runner;
   }
   runner->p = p;
   runner->mw.cint = 1;
   runner->mw.cint1 = 0;
   runner->ptr = (hashword *) xmalloc(sizeof(hashword));
   (runner->ptr)->p = -1;
   return runner;
}

hashword *
createhashpos P1C(int, p)
{
   hashword *runner= &(hashtable[p%HASHTABLESIZE]);
   if (runner->p==p) return runner;
   while (runner->p != -1) {
      runner = runner->ptr; 
      if (runner->p == p) return runner;
   } 
   runner->p = p;
   runner->mw.cint = 0;
   runner->mw.cint1 = 0;
   runner->ptr = (hashword *) xmalloc(sizeof(hashword));
   (runner->ptr)->p = -1;
   return runner;
}

void
dumphhashtable P1H(void)
{
   int i;
   hashword *runner;
   for (i=0; i<HASHTABLESIZE; i++) {
      runner=&(hashtable[i]);
      if (runner->p != -1) {
         dumpint(-i);
         while (runner->p != -1) {
            dumpint(runner->p);
            dumpwd(runner->mw);
            runner=runner->ptr;
         }
      }
   }
   dumpint(-HASHTABLESIZE);
}

void
undumphhashtable P1H(void)
{
   int i,j;
   hashword *runner;
   j=0;
   undumpint(i);
   while (j<HASHTABLESIZE) {
      i = (-i);
      while (j<i) {
         hashtable[j].p = -1;
         j++;
      }
      if (i<HASHTABLESIZE) {
         runner = &(hashtable[j]);
         undumpint(i);
         while (i>=0) {
            runner->p = i;
            undumpwd(runner->mw);
            undumpint(i);
            runner->ptr = (hashword *) xmalloc(sizeof(hashword));
            runner = runner->ptr;
         }
         runner->p = -1;
         j++;
      }
   }
}

#if 0				// MiKTeX
void
odateandtime P4C(int,timecode, int,daycode, int,monthcode, int,yearcode)
{
    integer tc,dc,mc,yc;
    dateandtime(tc,dc,mc,yc);
    setneweqtbint(timecode,tc);
    setneweqtbint(daycode,dc);
    setneweqtbint(monthcode,mc);
    setneweqtbint(yearcode,yc);
}
#endif

#if 0				// MiKTeX
memoryword **fonttables;
#endif
static int font_entries = 0;

void
allocatefonttable P2C(int, font_number, int, font_size)
{
    int i;
    if (font_entries==0) {
      fonttables = (memoryword **) xmalloc(256*sizeof(memoryword**));
      font_entries=256;
    } else if ((font_number==256)&&(font_entries==256)) {
      fonttables = (memoryword **) xrealloc(fonttables, 65536);	// MiKTeX
      font_entries=65536;
    }
    fonttables[font_number] =
       (memoryword *) xmalloc((font_size+1)*sizeof(memoryword));
    fonttables[font_number][0].cint = font_size;
    fonttables[font_number][0].cint1 = 0;
    for (i=1; i<=font_size; i++) {
        fonttables[font_number][i].cint  = 0;
        fonttables[font_number][i].cint1 = 0;
    }
}

void
dumpfonttable P2C(int, font_number, int, words)
{
    fonttables[font_number][0].cint=words;
    dumpthings(fonttables[font_number][0], fonttables[font_number][0].cint+1);
}

void
undumpfonttable(int font_number) // MiKTeX
//int font_number;
{
    memoryword sizeword;
    if (font_entries==0) {
      fonttables = (memoryword **) xmalloc(256*sizeof(memoryword**));
      font_entries=256;
    } else if ((font_number==256)&&(font_entries==256)) {
      fonttables = (memoryword **) xrealloc(fonttables, 65536);	// MiKTeX
      font_entries=65536;
    }

    undumpthings(sizeword,1);
    fonttables[font_number] =
        (memoryword *) xmalloc((sizeword.cint+1)*sizeof(memoryword));
    fonttables[font_number][0].cint = sizeword.cint;
    undumpthings(fonttables[font_number][1], sizeword.cint);
}

#if 0				// MiKTeX
memoryword **fontsorttables;
#endif
static int fontsort_entries = 0;

void
allocatefontsorttable P2C(int, fontsort_number, int, fontsort_size)
{
    int i;
    if (fontsort_entries==0) {
      fontsorttables = (memoryword **) xmalloc(256*sizeof(memoryword**));
      fontsort_entries=256;
    } else if ((fontsort_number==256)&&(fontsort_entries==256)) {
      fontsorttables = (memoryword **) xrealloc(fontsorttables, 65536);	// MiKTeX
      fontsort_entries=65536;
    }
    fontsorttables[fontsort_number] =
       (memoryword *) xmalloc((fontsort_size+1)*sizeof(memoryword));
    fontsorttables[fontsort_number][0].cint = fontsort_size;
    fontsorttables[fontsort_number][0].cint1 = 0;
    for (i=1; i<=fontsort_size; i++) {
        fontsorttables[fontsort_number][i].cint  = 0;
        fontsorttables[fontsort_number][i].cint1 = 0;
    }
}

void
dumpfontsorttable P2C(int, fontsort_number, int, words)
{
    fontsorttables[fontsort_number][0].cint=words;
    dumpthings(fontsorttables[fontsort_number][0],
               fontsorttables[fontsort_number][0].cint+1);
}

void
undumpfontsorttable(int fontsort_number)
//int fontsort_number;
{
    memoryword sizeword;
    if (fontsort_entries==0) {
      fontsorttables = (memoryword **) xmalloc(256*sizeof(memoryword**));
      fontsort_entries=256;
    } else if ((fontsort_number==256)&&(fontsort_entries==256)) {
      fontsorttables = (memoryword **) xrealloc(fontsorttables, 65536);	// MiKTeX
      fontsort_entries=65536;
    }

    undumpthings(sizeword,1);
    fontsorttables[fontsort_number] =
        (memoryword *) xmalloc((sizeword.cint+1)*sizeof(memoryword));
    fontsorttables[fontsort_number][0].cint = sizeword.cint;
    undumpthings(fontsorttables[fontsort_number][1], sizeword.cint);
}

#if 0				// MiKTeX
int **ocptables;
#endif
static int ocp_entries = 0;

void
allocateocptable P2C(int, ocp_number, int, ocp_size)
{
    int i;
    if (ocp_entries==0) {
      ocptables = (int **) xmalloc(256*sizeof(int**));
      ocp_entries=256;
    } else if ((ocp_number==256)&&(ocp_entries==256)) {
      ocptables = (int **) xrealloc(ocptables, 65536); // MiKTeX
      ocp_entries=65536;
    }
    ocptables[ocp_number] =
       (int *) xmalloc((1+ocp_size)*sizeof(int));
    ocptables[ocp_number][0] = ocp_size;
    for (i=1; i<=ocp_size; i++) {
        ocptables[ocp_number][i]  = 0;
    }
}

void
dumpocptable P1C(int, ocp_number)
{
    dumpthings(ocptables[ocp_number][0], ocptables[ocp_number][0]+1);
}

void
undumpocptable P1C(int, ocp_number)
{
    int sizeword;
    if (ocp_entries==0) {
      ocptables = (int **) xmalloc(256*sizeof(int**));
      ocp_entries=256;
    } else if ((ocp_number==256)&&(ocp_entries==256)) {
      ocptables = (int **) xrealloc(ocptables, 65536); // MiKTeX
      ocp_entries=65536;
    }
    undumpthings(sizeword,1);
    ocptables[ocp_number] =
        (int *) xmalloc((1+sizeword)*sizeof(int));
    ocptables[ocp_number][0] = sizeword;
    undumpthings(ocptables[ocp_number][1], sizeword);
}
