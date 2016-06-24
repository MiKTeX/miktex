/* papersiz.c */

/************************************************************************

  Part of the dvipng distribution

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program. If not, see
  <http://www.gnu.org/licenses/>.

  Copyright (C) 2002-2010 Jan-Åke Larsson

************************************************************************/

#include "dvipng.h"

const char *lengthnames[]={ "sp", "pt", "bp", 
			    "dd", "mm", "pc", 
			    "cc", "cm", "in" };
const int32_t lengthsp[]={ 1L, 65536L, 65782L,
			   70124L, 186468L, 786432L, 
			   841489L, 1864680L, 4736286L };

/*
 *   Convert a double[unit], e.g., "3.2cm" or "1.0in" into length in pixels.
 *   Advance the passed pointer as well.
 */

static int32_t myatodim(const char ** p)
{ 
  double tmp; /* double accuracy is enough, I think */
  int i=0;
  char *q;

  tmp = strtod(*p,&q);
  while (*q == ' ') 
    q++;
  while (i<8 && (q[0]!=lengthnames[i][0] || q[1]!=lengthnames[i][1]))
    i++; 
  if (i==8 && (q[0]!=lengthnames[i][0] || q[1]!=lengthnames[i][1]))
    Warning("unrecognized length unit \"%.2s\", assuming inches",q);
  while (*q != ',' && *q !='\0')
    q++;
  tmp = tmp*lengthsp[i]*dpi/4736286L; /*   sp * dots/in / (sp/in), convert sp to pixels   */
  *p=q;
  return((int32_t) tmp);
}


/*
 *   The routine where we handle the paper size special.  We need to pass in
 *   the string after the `papersize=' specification.
 */

void handlepapersize(const char * p, int32_t * x, int32_t * y)
{ 
   while (*p == ' ')
      p++ ;
   *x = myatodim(&p) ;
   while (*p == ' ' || *p == ',')
      p++ ;
   *y = myatodim(&p) ;
}

