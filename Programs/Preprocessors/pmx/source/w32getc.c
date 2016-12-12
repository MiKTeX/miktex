#include <stdio.h>
int w32getc(FILE *f)
{
   int c;
   c = getc(f);
   if(c=='\r') {
      c = getc(f);
      if(c!='\n') {
         ungetc(c,f);
         c='\r';
      }
   }
   return c;
}
