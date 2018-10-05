/* j2000.cpp: compute number of days since Jan 1, 2000

   Copyright (C) 2000-2016 Christian Schenk

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

#include <cstdio>
#include <ctime>
#include <sys/stat.h>

int
main (/*[in]*/ int		argc,
      /*[in]*/ const char **	argv)
{
  time_t tQuery;
  const time_t t2000 = 946681200;
  if (argc > 1)
    {
      struct stat statbuf;
      if (stat(argv[1], &statbuf) != 0)
	{
	  return (0);
	}
      tQuery = statbuf.st_mtime;
    }
  else
    {
      time (&tQuery);
    }
  int days = static_cast<int>((tQuery - t2000) / (60 * 60 * 24));
  printf ("%d\n", days);
  return (0);
}
