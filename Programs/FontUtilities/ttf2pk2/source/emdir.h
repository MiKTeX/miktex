/* emdir.h  -- Written by Eberhard Mattes, donated to the public domain */

#if defined(DJGPP)

/* djgpp support by Hartmut Schirmer (hsc@techfak.uni-kiel.de), May 30, 1997 */
#include <dos.h>
#define ll_findbuffer find_t
#define attr attrib

#else

struct ll_findbuffer
{
  char reserved[21];
  unsigned char attr;
  unsigned time;
  unsigned date;
  long size;
  char name[257];
};

#endif

int ll_findfirst (const char *path, int attr, struct ll_findbuffer *buffer);
int ll_findnext (struct ll_findbuffer *buffer);
