/* numberit.cc: find web section numbers

   Copyright (C) 1991-2016 Christian Schenk

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

using namespace std;

#define SEP1 "\
%% _____________________________________________________________________________\n\
%%\n\
%% [%d.%d] %s\n\
%% _____________________________________________________________________________\n"

#define SEP2 "\
%% _____________________________________________________________________________\n\
%%\n\
%% [%d.%d]\n\
%% _____________________________________________________________________________\n"


int
main ()
{
  char szLine[2048];
  int iSecNum = 0;
  int iChapNum = 0;
  while (fgets(szLine, sizeof(szLine), stdin))
    {
      if (szLine[0] == '@'
	  && (szLine[1] == '*' || szLine[1] == ' ' || szLine[1] == '\n'))
	{
	  ++ iSecNum;
	  fputc ('\n', stdout);
	  if (szLine[1] == '*')
	    {
	      ++ iChapNum;
	      char * lpsz = &szLine[2];
	      while (*lpsz != 0 && isspace(*lpsz))
		{
		  ++ lpsz;
		}
	      string strTitle;
	      while (*lpsz != 0 && *lpsz != '.')
		{
		  strTitle += *lpsz;
		  ++ lpsz;
		}
	      fprintf (stdout, SEP1, iChapNum, iSecNum, strTitle.c_str());
	    }
	  else
	    {
	      fprintf (stdout, SEP2, iChapNum, iSecNum);
	    }
	  fputc ('\n', stdout);
	}
      fputs (szLine, stdout);
    }
  return (0);
}
