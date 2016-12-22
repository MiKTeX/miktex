# define CFUNCS_C
#include "cfuncs.h"

int P_argc;
char **P_argv;
int P_ioresult;
int _EscIO2 (int errorcode, char* filename)
{ fprintf(stderr,"Input file %s not found\n",filename); 
  exit(errorcode); return errorcode; }
int _EscIO (int errorcode){ return _EscIO2(errorcode,""); }
int _Escape (int exitcode)
{ exit(exitcode); return exitcode; }
int _SETIO (int OK, int errorcode){ return P_ioresult = OK ? 0 : errorcode; }
int P_eof (FILE * infile)
{ int x;
  if (feof(infile)) return 1; 
  x = fgetc(infile);
  if (x==EOF) return 1;
  ungetc(x,infile); return 0;
  }
void *__MallocTemp__;
int _OutMem(void) { fprintf(stderr,"Out of memory\n"); exit(-2); return -2; }
void PASCAL_MAIN (int npars, char ** pars)
{ P_argc = npars; P_argv = pars; }

void scan1(char *s, short p, short *n)
{ sscanf(s+p-1,"%hd",n);
}

short pos1(char c, char *s)
{ char *t = strchr(s,c);
  if (t) return (short) (t-s+1); else return 0;
}

void delete1(char *s, short p)
{ char *t = (s+=p); s=t-1;
  while (*s) *s++ = *t++;
}

void predelete(char *s, short l)
{ char *t = s+l;
  while (*t) *s++ = *t++;
  *s = *t;
}

void shorten(char *s, short new_length)
{ s[new_length] = '\0';
}

short posNot(char c, char *s)
{ char *t = s;
  while (*t == c) t++;
  if (*t) return t-s+1;
  else return 0;
}

void getNum(char *line, short *k)
{ int j=sscanf(line,"%hd",k);
  if (j<1) *k=0;
}


void getTwoNums(char *line, short *k1, short *k2)
{ int param = sscanf(line,"%hd/%hd",k1,k2);
  if (param<2) *k2 = 0;
}

void toUpper(char *s)
{ while (*s) { *s = toupper(*s); s++; }
}

boolean startsWith(char *s1, char *s2)
{ while (*s2) { if (*s1++ != *s2++) return false; }
  return true;
}

void insertChar(char c, char *s, short p)
{ char x;
   s += p-1;
   do { x=c; c=*s; *s++=x; } while (x);
}

char *substr_(char *Result, char *s, short start, short count)
{ char *R = Result;
  s += start-1;
  while (count--) *R++ = *s++;
  *R = '\0';
  return Result;
}
 
short strpos2 (char *s1, char *s2, short p)
{ char *r =  strstr (s1+p, s2); 
  if (r == NULL) return 0;
  else return r-s1;
}

short nextWordBound(char *s, char trigger, short p)
{
  do {
    p++;
  } while (p <= strlen(s) && s[p-1] != trigger);
  while (p < strlen(s) && s[p] != ' ')
    p++;
  return p;
}

# undef CFUNCS_C
