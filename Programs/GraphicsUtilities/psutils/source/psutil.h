/* psutil.h
 * PSUtils utility functions
 *
 * (c) Reuben Thomas 2012-2014
 * (c) Angus J. C. Duggan 1991-1997
 * See file LICENSE for details.
 */
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#if defined(MIKTEX)
#include <sys/types.h>
#include <getopt.h>
#include <fcntl.h>
#endif

/* Definitions for functions found in psutil.c */
extern void usage(void);
extern void die(const char *format, ...);
extern void argerror(void);
extern int paper_size(const char *paper_name, double *width, double *height);
extern FILE *seekable(FILE *fp);
extern void writepage(int p);
extern void seekpage(int p);
extern void writepageheader(const char *label, int p);
extern void writepagesetup(void);
extern void writepagebody(int p);
extern void writeheader(int p, off_t *ignorelist);
extern void writeheadermedia(int p, off_t *ignorelist, double width, double height);
extern int writepartprolog(void);
extern void writeprolog(void);
extern void writesetup(void);
extern void writetrailer(void);
extern void writeemptypage(void);
extern void scanpages(off_t *sizeheaders);
extern void writestring(const char *s);

/* These variables are exported to the client program */
extern int pages;
extern int verbose;
extern FILE *infile;
extern FILE *outfile;
extern char pagelabel[BUFSIZ];
extern int pageno;

/* Variables imported from the client program */
extern const char *syntax;
extern const char *argerr_message;

/* Useful macros */
#define MIN(x,y) ((x) > (y) ? (y) : (x))
#define COPYRIGHT_STRING "(c) Reuben Thomas <rrt@sc3d.org> 2014\n(c) Angus J. C. Duggan 1991-1997\nSee file LICENSE for details.\n"
