/* psspec.c
 * Page spec routines for page rearrangement
 *
 * (c) Reuben Thomas 2012-2014
 * (c) Angus J. C. Duggan 1991-1997
 * See file LICENSE for details.
 */

#include "config.h"

#include "psutil.h"
#include "psspec.h"

#include <string.h>

double width = -1;
double height = -1;

/* create a new page spec */
PageSpec *newspec(void)
{
   PageSpec *temp = (PageSpec *)malloc(sizeof(PageSpec));
   if (temp == NULL)
      die("out of memory");
   temp->pageno = temp->flags = temp->rotate = 0;
   temp->scale = 1;
   temp->xoff = temp->yoff = 0;
   temp->next = NULL;
   return (temp);
}

/* dimension parsing routines */
int parseint(char **sp)
{
   char *s = *sp;
   int num = atoi(s);

   while (isdigit((unsigned char)*s))
      s++;
   if (*sp == s) argerror();
   *sp = s;
   return (num);
}

double parsedouble(char **sp)
{
   char *s = *sp;
   double num = atof(s);

   while (isdigit((unsigned char)*s) || *s == '-' || *s == '.')
      s++;
   if (*sp == s) argerror();
   *sp = s;
   return (num);
}

double parsedimen(char **sp)
{
   double num = parsedouble(sp);
   char *s = *sp;

   if (strncmp(s, "pt", 2) == 0) {
      s += 2;
   } else if (strncmp(s, "in", 2) == 0) {
      num *= 72;
      s += 2;
   } else if (strncmp(s, "cm", 2) == 0) {
      num *= 28.346456692913385211;
      s += 2;
   } else if (strncmp(s, "mm", 2) == 0) {
      num *= 2.8346456692913385211;
      s += 2;
   } else if (*s == 'w') {
      if (width < 0)
	 die("width not initialised");
      num *= width;
      s++;
   } else if (*s == 'h') {
      if (height < 0)
	 die("height not initialised");
      num *= height;
      s++;
   }
   *sp = s;
   return (num);
}

double singledimen(char *str)
{
   double num = parsedimen(&str);
   if (*str) usage();
   return (num);
}

static const char *prologue[] = { /* PStoPS procset */
   /* Wrap these up with our own versions.  We have to  */
   "userdict begin",
   "[/showpage/erasepage/copypage]{dup where{pop dup load",
   " type/operatortype eq{ /PStoPSenablepage cvx 1 index"
   " load 1 array astore cvx {} bind /ifelse cvx 4 array"
   " astore cvx def}{pop}ifelse}{pop}ifelse}forall"
   " /PStoPSenablepage true def",
   "[/letter/legal/executivepage/a4/a4small/b5/com10envelope",	/* nullify */
   " /monarchenvelope/c5envelope/dlenvelope/lettersmall/note",	/* paper */
   " /folio/quarto/a5]{dup where{dup wcheck{exch{}put}",	/* operators */
   " {pop{}def}ifelse}{pop}ifelse}forall",
   "/setpagedevice {pop}bind 1 index where{dup wcheck{3 1 roll put}",
   " {pop def}ifelse}{def}ifelse",
   "/PStoPSmatrix matrix currentmatrix def",
   "/PStoPSxform matrix def/PStoPSclip{clippath}def",
   "/defaultmatrix{PStoPSmatrix exch PStoPSxform exch concatmatrix}bind def",
   "/initmatrix{matrix defaultmatrix setmatrix}bind def",
   "/initclip[{matrix currentmatrix PStoPSmatrix setmatrix",
   " [{currentpoint}stopped{$error/newerror false put{newpath}}",
   " {/newpath cvx 3 1 roll/moveto cvx 4 array astore cvx}ifelse]",
   " {[/newpath cvx{/moveto cvx}{/lineto cvx}",
   " {/curveto cvx}{/closepath cvx}pathforall]cvx exch pop}",
   " stopped{$error/errorname get/invalidaccess eq{cleartomark",
   " $error/newerror false put cvx exec}{stop}ifelse}if}bind aload pop",
   " /initclip dup load dup type dup/operatortype eq{pop exch pop}",
   " {dup/arraytype eq exch/packedarraytype eq or",
   "  {dup xcheck{exch pop aload pop}{pop cvx}ifelse}",
   "  {pop cvx}ifelse}ifelse",
   " {newpath PStoPSclip clip newpath exec setmatrix} bind aload pop]cvx def",
   "/initgraphics{initmatrix newpath initclip 1 setlinewidth",
   " 0 setlinecap 0 setlinejoin []0 setdash 0 setgray",
   " 10 setmiterlimit}bind def",
   "end",
   NULL
   };

void pstops(int modulo, int pps, int nobind, PageSpec *specs, double draw) {

  scanpages(NULL);
  pstops_write(modulo, pps, nobind, specs, draw, NULL);
}

void pstops_write(int modulo, int pps, int nobind, PageSpec *specs, double draw, off_t *ignorelist)
{
   int thispg, maxpage;
   int pageindex = 0;
   const char **pro;

   maxpage = ((pages+modulo-1)/modulo)*modulo;

   /* rearrange pages: doesn't cope properly with loaded definitions */
   writeheadermedia((maxpage/modulo)*pps, ignorelist, width, height);
   writestring("%%BeginProcSet: PStoPS");
   if (nobind)
      writestring("-nobind");
   writestring(" 1 15\n");
   for (pro = prologue; *pro; pro++) {
      writestring(*pro);
      writestring("\n");
   }
   if (nobind) /* desperation measures */
      writestring("/bind{}def\n");
   writestring("%%EndProcSet\n");
   /* save transformation from original to current matrix */
   if (writepartprolog()) {
      writestring("userdict/PStoPSxform PStoPSmatrix matrix currentmatrix\n");
      writestring(" matrix invertmatrix matrix concatmatrix\n");
      writestring(" matrix invertmatrix put\n");
   }
   writesetup();
   for (thispg = 0; thispg < maxpage; thispg += modulo) {
      int add_last = 0;
      PageSpec *ps;
      for (ps = specs; ps != NULL; ps = ps->next) {
	 int actualpg;
	 int add_next = ((ps->flags & ADD_NEXT) != 0);
	 if (ps->flags & REVERSED)
	    actualpg = maxpage-thispg-modulo+ps->pageno;
	 else
	    actualpg = thispg+ps->pageno;
	 if (actualpg < pages)
	    seekpage(actualpg);
	 if (!add_last) {	/* page label contains original pages */
	    PageSpec *np = ps;
	    char *eob = pagelabel;
	    char sep = '(';
	    do {
	       *eob++ = sep;
	       if (np->flags & REVERSED)
		  sprintf(eob, "%d", maxpage-thispg-modulo+np->pageno);
	       else
		  sprintf(eob, "%d", thispg+np->pageno);
	       eob = eob + strlen(eob);
	       sep = ',';
	    } while ((np->flags & ADD_NEXT) && (np = np->next));
	    strcpy(eob, ")");
	    writepageheader(pagelabel, ++pageindex);
	 }
	 writestring("userdict/PStoPSsaved save put\n");
	 if (ps->flags & GSAVE) {
	    char buffer[BUFSIZ];
	    writestring("PStoPSmatrix setmatrix\n");
	    if (ps->flags & OFFSET) {
	       sprintf(buffer, "%f %f translate\n", ps->xoff, ps->yoff);
	       writestring(buffer);
	    }
	    if (ps->flags & ROTATE) {
	       sprintf(buffer, "%d rotate\n", ps->rotate);
	       writestring(buffer);
	    }
	    if (ps->flags & HFLIP) {
	       sprintf(buffer, "[ -1 0 0 1 %f 0 ] concat\n", width*ps->scale);
	       writestring(buffer);
	    }
	    if (ps->flags & VFLIP) {
	       sprintf(buffer, "[ 1 0 0 -1 0 %f ] concat\n", height*ps->scale);
	       writestring(buffer);
	    }
	    if (ps->flags & SCALE) {
	       sprintf(buffer, "%f dup scale\n", ps->scale);
	       writestring(buffer);
	    }
	    writestring("userdict/PStoPSmatrix matrix currentmatrix put\n");
	    if (width > 0 && height > 0) {
	       writestring("userdict/PStoPSclip{0 0 moveto\n");
	       sprintf(buffer, " %f 0 rlineto 0 %f rlineto -%f 0 rlineto\n",
		       width, height, width);
	       writestring(buffer);
	       writestring(" closepath}put initclip\n");
	       if (draw > 0) {
		  sprintf(buffer, "gsave clippath 0 setgray %f setlinewidth stroke grestore\n", draw);
		  writestring(buffer);
	       }
	    }
	 }
	 if (add_next)
	    writestring("/PStoPSenablepage false def\n");
	 if (actualpg < pages) {
	    writepagesetup();
	    writestring("PStoPSxform concat\n");
	    writepagebody(actualpg);
	 } else {
	    writestring("PStoPSxform concat\n");
	    writestring("showpage\n");
	 }
	 writestring("PStoPSsaved restore\n");
	 add_last = add_next;
      }
   }
   writetrailer();
}
