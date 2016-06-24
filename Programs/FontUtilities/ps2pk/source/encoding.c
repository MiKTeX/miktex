/* MODULE encoding (encoding.c)
 * interface:  typedef char *encoding[256];
 *             void getenc(
 *		      char **fontname, **encname,  
 *                         ( foundry name of font and name of this encoding )
 *                    encoding ev,      output encoding
 *                    int width[256]);  output widths
 *
 * depends on: MODULEs binary_table, strings
 *             <ctype.h>
 *             <stdio.h>
 *             <stdlib.h>
 *             <string.h>
 *
 * globals:    void fatal();
 *
 * Author:     Piet Tutelaers
 * (see README for license)
 */

#include "types.h"
#include "basics.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *my_string(char *);
static void remove_string(void);
static void addcc(char *name, int charcode);
static int getcc(char *name);

/* return a pointer to first non space character after occurence of string t,
 * the scan starts at pointer s 
 */
static char *value_after(char *s, const char *t)
{  int l;
   l = strlen(t);
   for (;;) {
      while (*s != *t && *s != '\n') s++;
      if (*s == '\n') return NULL;
      if (strncmp(s, t, l) == 0) break;
      s++;
   }
   s += l;
   while (isspace((unsigned char)*s)) s++;
   return s;
}

static int decimal(char *s)
{  int sign = 1, d = 0;

   while (isspace((unsigned char)*s)) s++;
   if (*s == '-') { sign = -1; s++; }
   while (isdigit((unsigned char)*s)) { d = d * 10 + (*s - '0'); s++; }
   return sign*d;
}

/* Nextsymbol() reads lines from enc until a '[', '/' or a ']' is 
 * encountered. In that case pline points to the character following
 * this symbol otherwise a fatal error is generated.
 */

FILE *enc;

#define LINEBUF 1024
char line[LINEBUF], *pline = NULL;

static char nextsymbol(void)
{
   for (;;) {
      if (pline == NULL) {
         if (fgets(line, LINEBUF-1, enc) == NULL)
            fatal("unexpected EOF while reading %s\n", encfile);
         pline = line;
      }
      if (*pline == '[' || *pline == ']' || *pline == '/') break;
      if (*pline == '%' || *pline == '\n' || *pline == '\0') 
      {  pline = NULL; 
         continue; 
      }
      pline++;
   }
   pline++; return *(pline-1);
}

static char *nextpsname(void)
{  int i = 0;
   char the_nextname[MAXSTRLEN], *name;

   for (;;) {
      if (pline == NULL) {
         if (fgets(line, LINEBUF-1, enc) == NULL)
            fatal("unexpected EOF while reading %s\n", encfile);
         pline = line;
      }
      if (*pline == '%' || *pline == '\n' || *pline == '\0') 
      {  pline = NULL; 
         continue; 
      }
      if (isspace((unsigned char)*pline)) {
         pline++;
	 continue;
      }
      if (!isalpha((unsigned char)*pline)) fatal("invalid name in %s\n", encfile);
      /* pt981009: added '_' Staszek Wawrykiewicz <staw@gust.org.pl> */
      while (isalnum((unsigned char)*pline) || *pline == '-' || *pline == '_') {
         if (i > MAXSTRLEN-2)
            fatal("name too long in %s (%s)\n", line, encfile);
         the_nextname[i++] = *pline++;
      }
      the_nextname[i] = '\0';
      name = malloc(i+1);
      if (name == NULL) fatal("Out of memory\n");
      strcpy(name, the_nextname);
      break;
   }
   /* pt981009: removed pline++ Staszek Wawrykiewicz <staw@gust.org.pl> */
   return name;
}

void getenc(char **fontname, char **encname, encoding ev, int width[256])
{  int i, len, SCMseen, Ccnt, wx, cc;
   FILE *afm;
   char *name, ns;
   
   for (i=0; i < 256; i++) { ev[i] = NULL; width[i] = -1000; }
   if (encfile) {
      enc = fopen(encfile, "r");
      if (enc == NULL) fatal("Can't open %s\n", encfile);
      if (nextsymbol() != '/') fatal("missing '/encoding' in %s\n", encfile);
      *encname = nextpsname();
      if (nextsymbol() != '[') fatal("missing '[' in %s\n", encfile);
      i = 0;
      while (i < 256 && (ns = nextsymbol()) == '/') {
         name = my_string(pline);
         if (strcmp(name, ".notdef") == 0) { 
            i++; remove_string();
            continue; 
         }
         addcc(name, i++);
      }
      if (i != 256) fatal("missing %d symbols in %s\n", 256-i, encfile);
      if (nextsymbol() != ']') fatal("missing ']' in %s\n", encfile);
      fclose(enc);
   }
   afm = fopen(afmfile, "r");
   if (afm == NULL) fatal("Can't open %s\n", afmfile);
   /*
    * Scan header of AFM file and take the FontName and EncodingScheme
    * (used for identification purposes in the PK postamble)
    * Stop after reading `StartCharMetrics'.
    */
   SCMseen = 0; Ccnt = 0;
   while (fgets(line, LINEBUF-1, afm)) {
      if (strncmp(line, "FontName", 8) == 0) { 
	 pline = value_after(line, "FontName"); 
	 len = strlen(pline);
	 if (*(pline+len-1) == '\n') {
	    *(pline+len-1) = '\0'; len--;
	 }
	 *fontname = malloc(len + 1);
	 if (*fontname == NULL) fatal("Out of memory\n");
         strcpy(*fontname, pline);
      }
      else if (encname == NULL && strncmp(line, "EncodingScheme", 8) == 0) { 
	 pline = value_after(line, "EncodingScheme"); 
	 len = strlen(pline);
	 if (*(pline+len-1) == '\n') {
	    *(pline+len-1) = '\0'; len--;
	 }
	 *encname = malloc(len + 1);
	 if (*encname == NULL) fatal("Out of memory\n");
	 strcpy(*encname, pline);
      }
      else if (strncmp(line, "StartCharMetrics", 16) == 0) {
         SCMseen = 1; break;
      }
   }
   if (SCMseen == 0) fatal("%s: no metrics info\n", afmfile);
   while (fgets(line, LINEBUF-1, afm)) {
      if (strncmp(line, "EndCharMetrics", 14) == 0) break;
      if (strncmp(line, "C ", 2) != 0) 
         fatal("%s: unexpected line\n", afmfile);

      /* Gracefully terminate when the AFM file is not valid.  Every line */
      /* in the AFM file should specify a "C"haracter, a "WX"idth, and    */
      /* a "N"ame. (ndw)
      */
      pline = value_after(line, "C"); 
      if (pline == NULL)
         fatal("\nBad char metric in %s (no char):\n\t %s\n", afmfile, line);
      cc = decimal(pline);
      pline = value_after(pline, "WX"); 
      if (pline == NULL)
         fatal("\nBad char metric in %s (no width):\n\t %s\n", afmfile, line);
      wx = decimal(pline);
      pline = value_after(pline, "N"); 
      if (pline == NULL)
         fatal("\nBad char metric in %s (no name):\n\t %s\n", afmfile, line);
      name = my_string(pline);
      
      if (encfile) {
         if ((i = getcc(name)) == -1) {
            remove_string();
            continue;
	 }
         do { /* allow more occurences per name */
            ev[i] = name; width[i] = wx;
            i = getcc(name);
         }
         while (i >= 0);
      }
      else if (cc >= 0 && cc <= 255) { /* ndw: 10/13/92 */
	 ev[cc] = name; width[cc] = wx;
      }
      else if (cc > 255)
	 msg("Char code %d ignored (out of range).\n", cc);
      Ccnt++;
   }
   if (Ccnt == 0) fatal("%s: no characters selected\n", afmfile);   
}

/* END MODULE encoding */

/* MODULE binary_table
 * depends on: MODULE string
 *             <stdlib.h>
 *             <string.h>
 */

/*
 * TUG has accepted an extended font layout (EC.enc) where the hyphen 
 * occupies two positions ('055 and '177) to handle exeptional hyphenation
 * situations (see TUGboat#11 1990 (no. 4) pp. 514).
 * To handle this special case we allow two charcodes per symbol.
 */
  
/*
 * Two functions for a binary search table. One to add a name and one 
 * to lookup a name. This table is used to store all encoding names and 
 * its corresponding charcode.
 */

struct Node
{ char *name;                    /* pointer to a PostScript string */
  short cc[2];                   /* its charcode's (maximum of two) */
  short left, right;
} node[256];

static int freenode = 0;
static short root = -1;

static void addcc(char *name, int charcode)
{
   short p = 0, q;
   int r = 0;

   q = root;
   while (q != -1) {
      p = q; r = strcmp(name, node[p].name);
      if (r == 0) {
      	 if (node[p].cc[1] == -1) {
      	    node[p].cc[1] = charcode;
      	    return;
      	 }
      	 else 
      	    fatal("%s: more than two codes for %s (%d,%d,%d)\n", 
      	       encfile, name, node[p].cc[0], node[p].cc[1], charcode);
      }
      else if (r < 0) q=node[p].left;
      else q=node[p].right;
   }
   if (freenode == 256) 
      fatal("%s: too many PostScript names\n", encfile);
      /* not found hence insert */
   node[freenode].name = name;
   node[freenode].cc[0] = charcode;
   node[freenode].cc[1] = -1;
   node[freenode].left = node[freenode].right = -1;
   if (root == -1)
      root = freenode;
   else {
      if (r < 0) node[p].left=freenode;
      else node[p].right=freenode;
   }
   freenode++;
}

static int getcc(char *name)
{  short r, p, q, cc;

   q = root;
   while (q != -1) {
      p = q; r = strcmp(name, node[p].name);
      if (r == 0) {
      	 cc = node[p].cc[0];
      	 node[p].cc[0] = node[p].cc[1];
      	 node[p].cc[1] = -1;
      	 return cc; 
      }
      else if (r < 0) q = node[p].left;
      else q = node[p].right;
   }
   return -1;
}

/* END MODULE binary_table */

/* MODULE string: 
 * depends on: <ctype.h>
 *             <string.h>
 */

/* String() reads a string starting at a given character pointer. 
 * After removing leading whitespace the resulting string (with possible
 * `.' and '_' characters) is stored in a stringpool after which a pointer 
 * to it is returned.
 */

#define POOLSIZE 10000
static char stringpool[POOLSIZE];
static int poolsize,      /* occupied space annex free index */
           lastpoolsize;  /* previous poolsize */

static char *my_string(char *s)
{  int length; char *str;

   while (isspace((unsigned char)*s)) s++;
   str = s; length = 0;
   while (isalnum((unsigned char)*s) || *s == '.' || *s == '_') { length++; s++; }
   if (length == 0) return NULL;
   else {
      if (length + 1 > POOLSIZE - poolsize)
         fatal("Out of memory %s\n", s);
      strncpy(stringpool+poolsize, str, length);
      str = stringpool + poolsize;
      s = stringpool + poolsize + length;
      *s = '\0';
      lastpoolsize = poolsize;
      poolsize += length+1;
      return str; 
   }
}

/* remove last string from stringpool */
static void remove_string(void)
{
   poolsize = lastpoolsize;
}

/* END MODULE string */

