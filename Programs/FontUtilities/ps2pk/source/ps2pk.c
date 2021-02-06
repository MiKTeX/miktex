/*
 * NAME
 *	ps2pk - creates a PK font from a type1 PostScript font
 * SYNOPSIS:
 *	ps2pk [options] type1 [pkname]
 * DESCRIPTION
 *	This program renders a given type1 PostScript font at a given
 *	pointsize (default 10.0 points) and resolution (default 300dpi)
 *      into a TeX PK font.
 *
 *      To generate the PK font ps2pk needs a valid type1 fontname (for
 *      example Utopia-Regular.pfa) and its corresponding AFMfile
 *	(Utopia-Regular.afm). The program accepts both the MSDOS binary
 *	type1 format (Utopia-Regula.pfb) and its UNIX ASCII equivalent
 *	(Utopia-Regula.pfa). The resulting PK font does contain all
 *	characters for which the C-code is non negative. This can be
 *	overruled by specifying an explicit <encoding> via the -e<encoding>
 *	option. Character codes in AFM files are specified as integer values
 *	after the C, for example Utopia-Regular.afm provides:
 *	   C 251 ; WX 601 ; N germandbls ; B 22 -12 573 742 ;
 *	   C  -1 ; WX 380 ; N onesuperior ; B 81 272 307 680 ;
 *
 *	The `germandbls' has code 251 and `onesuperior' -1 which means
 *	that this character is not part of the default AdobeStandard-
 *	Encoding scheme. To get this `onesuperior' you have to add it to
 *	an encoding vector as described in the PostScript manuals. But of
 *	course it is better to use standard encodings, like the one provided
 *	with the new psfonts. This `TeXBase1Encoding' encoding does contain
 *	all names of characters as defined in the 256 character TeX fonts.
 *
 *	The location of files for ps2pk, since version 1.5, has been
 *	improved using PostScript resource databases. These databases
 *	can be created using Adobe's mkpsres(1) utility. On UNIX the
 *	default name is PSres.upr and can be found in directories like
 *	/usr/lib/DPS depending on the machine you are using. The
 *	following example of an UPR file contains three resources
 *	   PS-Resources-1.0
 *	   Encoding
 *	   FontAFM
 *	   FontOutline
 *	   .
 *	   //usr/local/tex/fonts/type1faces
 *	   Encoding
 *	   ECEncoding=EC.enc
 *	   .
 *	   FontAFM
 *	   Courier=ResidentFonts/Courier.pfa
 *	   .
 *	   FontOutline
 *	   Courier=ResidentFonts/Courier.afm
 *	   .
 *
 *	When this UPR is in the PSRESOURCEPATH ps2pk can make an EC encoded
 *	PK font for Courier with:
 *
 *	   ps2pk -eECEncoding Courier
 *
 *	It is also possible without the UPR file but then you have to
 *	set T1INPUTS to the directories containing the files you need,
 *	for example:
 *
 *	   setenv T1INPUTS /usr/local/tex/fonts/type1faces//
 *
 *	and to run ps2pk by providing the proper filenames:
 *
 *	   ps2pk -eEC.enc Courier.pfa
 *
 *	In both cases the result will be the PK font Courier10.300pk.
 *	
 *	PostScript resource databases eliminate the need for recursive
 *	directory search, this is done by mkpsres(1) instead. And all
 *	resources can be stored in one directory per computer keeping 
 *	maintenance easy. Another advantage is the mapping of internal
 *	PostScript names, which are case sensitive and not restricted to
 *	MSDOS limitations, to external platform dependant filenames in
 *	a very natural way. On a MSDOS computer psres.dpr (the equivalent
 *	of PSres.upr) could be:
 *	   PS-Resources-1.0
 *	   Encoding
 *	   FontAFM
 *	   FontOutline
 *	   .
 *	   Encoding
 *	   TeXBase1Encoding=c:\emtex\dvips\8r.enc
 *	   .
 *	   FontAFM
 *	   Courier=c:\psfonts\com_____.pfb
 *	   .
 *	   FontOutline
 *	   Courier=c:\psfonts\afm\com_____.afm
 *	   .
 *
 *	Given this psres.dpr resource database
 *
 *	   ps2pk -eTeXBase1Encoding Courier
 *
 *	will create Courier10.300pk. Unfortunately MSDOS will abbreviate
 *	this name to COURIER1.300. Mkpsres(1) is an improved version
 *	of Adobe's makepsres(1) that runs on UNIX and MSDOS. This program
 *	will treat PFB fonts and PFM fonts correctly and recognize
 *	filenames ending with ".enc" as encoding vectors.
 *
 *	If the name of <type1font> does not contain an explicit extension
 *	or directory separator it is considered as the name of a resource.
 *	Otherwise as a filename. You can not mix up resource names and file-
 *	names. Filenames must have an extension!
 *
 * OPTIONS AND ARGUMENTS
 *       -a<AFM>        The name of the AFM resource or file can be
 *			overruled with this option.
 *       -e<encoding>   The encoding scheme (default the encoding from the 
 *                      AFM file is used).
 *	 -E<extension>	The extension factor (real value, default 1.0).
 *	 -O		Create old checksums (for compatibility)
 *	 -P<pointsize>	The desired pointsize (real value, default 10.0
 *			points). PK fonts created with a value different
 *			to 10.0 will cause dvips warnings `dvips: Design
 *			size mismatch in font ...'.
 *	 -R<baseres>	The base resolution (integer value, default 300
 *			dpi). Only used for creating METAFONT compatible 
 *			identification strings in PK postamble.
 *	 -S<slant>	The slant (real value, default 0.0).
 *	 -X<xres>	The resolution (integer value) in the X direction 
 *			(default 300 dpi).
 *	 -Y<yres>	The resolution (integer value) in the Y direction 
 *			(defaults to the value of <xres>).
 *	 -d		Debug stat() calls during recursive path searching
 *	 -v		Verbose flag. (Tells what the program is doing.)
 *	
 *	 type1font	The name of the PostScript type 1 font. When no
 *                      explicit AFMfile is given via the -a option the
 *                      name of the AFMfile will be constructed from this
 *                      name by removing the extension (if supplied) and
 *                      adding ".afm". 
 *	 [pkname]	The name of the resulting PK font can be overruled
 *			with this name. The default name of the PK font is
 *			derived from the basename of the type1 font, the
 *			pointsize and <xres>. For example:
 *			   ps2pk -P17.28 Utopia-Regular
 *			will result in:
 *				Utopia-Regular17.300pk
 *
 * ENVIRONMENTS
 *
 *   PSRESOURCEPATH
 *	The path used for locating PS resource database files (PSres.upr
 *	on UNIX and psres.dpr on MSDOS). The default of the path will be
 *	shown by running ps2pk with the -v flag and a non-existant resource
 *	name.
 *
 *   T1INPUTS
 *	The path used for locating PS type1 fonts, their AFM files and
 *	encoding vectors. Path may contain '//' (UNIX) or '!!' (MSDOS) to
 *	force ps2pk to look into subdirectories. Using PSRESOURCEPATH
 *	instead of T1INPUTS avoids recursion. The default of the path will
 *	be shown by running ps2pk with the -v flag and a non-existant
 *	filename.
 *	
 * ACKNOWLEDGEMENTS
 *	This program uses the type1 hinting and rendering software IBM 
 *	has donated to the X-consortium.
 *
 *      Since version 1.5 the lookup of PS resources as described in
 *      `Programming the Display PostScript System with X (Appendix A:
 *      Locating PostScript Resources)' from Adobe Systems is used.
 * SEE ALSO
 *	``METAFONT: The Program'', Donald Knuth.
 *	``The GFtoPK processor'', belonging to the METAFONTware.
 *	afm2tfm(1)
 *      mkpsres(1)
 *	mtpk(1)
 *	pk2bm(1)
 *	pkfonts(1)
 * AUTHOR
 *	Piet Tutelaers
 */

#include "basics.h"	/* basic definitions and fatal() */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* PostScript Resource lookup functions */
/* 
#include "PSres.h"	
*/
#include "filenames.h"	/* newname(), extension(), ps_resource() */
#include "psearch.h"	/* path search functions */
#include "pkout.h"	/* PK output functions */

/* begin interface to type 1 software */
#include "ffilest.h"

#include "objects.h"
#include "spaces.h"
#include "paths.h"
#include "regions.h"
#include "t1stdio.h"
#include "util.h"
#include "fontfcn.h"

FontScalableRec vals;
FontEntryRec entry;
#define Succesful	85
/* end interface to type 1 software */

char *encfile = NULL, *afmfile;

char ps2pk_args[MAXSTRLEN] = "none";	/* essential ps2pk arguments */

#define POINTSPERINCH 72.27
#define DEFAULTRES 300
#define DEFAULTPOINTSIZE 10.0

double pointsize = 0.0; /* wanted pointsize */
int W, H,	        /* width and height of character glyph */
    base_resolution = DEFAULTRES,
    x_resolution = 0, 
    y_resolution = 0;

int verbose = 0, debug = 0;

/* Provide old (-O flag) and new (default) checksum function */
static uint32_t checksum(encoding, int [256]);
static uint32_t old_checksum(encoding, int [256]);
uint32_t (*pchecksum)(encoding, int [256]) = checksum;

/* Prototypes */
static int next_pixel(void);
#if 0
static void print_pixmap(void);
#endif
static void first_pixel(CharInfoRec *);
static int32_t TFMwidth(int);
static int h_escapement(int);
static void add_option(const char *, const char *);

int main(int argc, char *argv[])
{
   char c;
   int done, i;
   const char *myname = "ps2pk";
   char *psname, *psbasename, *afmname = NULL,
	*encname = NULL, *psfile = NULL, *psfilebn, pkname[80],
	*AFM_fontname = NULL,
	*encodingscheme = NULL;

   FontPtr fontptr;
   unsigned char glyphcode[1]; /* must be an array */
   CharInfoRec *glyphs[1];
   unsigned int count;
   int charcode, rc = -1, charno;
   char comment[256];
   long cs;
   encoding ev;
   int WX[256];

   float efactor = 1.0, slant = 0.0;

#ifdef KPATHSEA
   kpse_set_program_name(argv[0], "ps2pk");
#endif

   while (--argc > 0 && (*++argv)[0] == '-') {
      done=0;
      while ((!done) && (c = *++argv[0]))  /* allow -bcK like options */
      	 switch (c) {
      	 case 'a':
      	    if (*++argv[0] == '\0') {
      	       argc--; argv++;
      	    }
      	    if (argv[0] == NULL) goto invalid;
	    afmname = argv[0]; 
	    done = 1;
      	    break;
      	 case 'd':
      	    debug = 1;
	    break;
      	 case 'e':
      	    if (*++argv[0] == '\0') {
      	       argc--; argv++;
      	    }
      	    if (argv[0] == NULL) goto invalid;
	    encname = argv[0]; 
	    add_option("-e", encname);
	    done = 1;
      	    break;
      	 case 'E':
      	    if (*++argv[0] == '\0') {
      	       argc--; argv++;
      	    }
      	    if (argv[0] == NULL) goto invalid;
	    efactor = atof(argv[0]);
	    add_option("-E", argv[0]);
	    done = 1;
      	    break;
      	 case 'O':
      	    pchecksum = old_checksum;
	    add_option("-O", "");
	    break;
      	 case 'P':
      	    if (*++argv[0] == '\0') {
      	       argc--; argv++;
      	    }
      	    if (argv[0] == NULL) goto invalid;
	    pointsize = atof(argv[0]); 
	    add_option("-P", argv[0]);
	    done = 1;
      	    break;
      	 case 'R':
      	    if (*++argv[0] == '\0') {
      	       argc--; argv++;
      	    }
      	    if (argv[0] == NULL) goto invalid;
	    base_resolution = atoi(argv[0]); 
	    add_option("-R", argv[0]);
	    done = 1;
      	    break;
      	 case 'S':
      	    if (*++argv[0] == '\0') {
      	       argc--; argv++;
      	    }
      	    if (argv[0] == NULL) goto invalid;
	    slant = atof(argv[0]);
	    add_option("-S", argv[0]);
	    done = 1;
      	    break;
      	 case 'X':
      	    if (*++argv[0] == '\0') {
      	       argc--; argv++;
      	    }
      	    if (argv[0] == NULL) goto invalid;
	    x_resolution = atoi(argv[0]); done = 1;
	    if (y_resolution == 0) y_resolution = x_resolution;
	    add_option("-X", argv[0]);
      	    break;
      	 case 'Y':
      	    if (*++argv[0] == '\0') {
      	       argc--; argv++;
      	    }
      	    if (argv[0] == NULL) goto invalid;
	    y_resolution = atoi(argv[0]); done = 1;
	    if (x_resolution == 0) x_resolution = y_resolution;
	    add_option("-Y", argv[0]);
      	    break;
      	 case 'v':
      	    verbose = 1;
	    break;
invalid:
      	 default:
      	    fatal("%s: %c invalid option\n", myname, c);
      	 }
      }

   if (argc < 1 || argc >2) {
      msg  ("ps2pk version " PACKAGE_VERSION " (" TL_VERSION ")\n");
      msg  ("Usage: %s [options] type1font [pkname]\n", myname);
      msg  ("options: -d -v -e<enc> -X<xres> -E<expansion> -S<slant>\n");
      msg  ("options: -O -P<pointsize> -Y<yres> -a<AFM> -R<baseres>\n");
      fatal("\nEmail bug reports to %s.\n", PACKAGE_BUGREPORT);
   }

   psname = argv[0]; argc--; argv++;

#ifndef KPATHSEA
   if (ps_resource(psname)) {
      /* if psname does neither contain a DIRSEP or `.' character
       * then find the filename of this resource in one of the PS
       * resource database(s) available in PSRESOURCEPATH.
       */
      char **names, **files; int i, count;

      psrespath = path(PSRESOURCEPATH, getenv("PSRESOURCEPATH"));
      if (verbose) msg("PSRESOURCEPATH := %s\n", psrespath);

      count = ListPSResourceFiles(psrespath, NULL,
   	"FontOutline", psname, &names, &files);
      if (count == 0)
	 fatal("No FontOutline resource found for %s\n", psname);
      if (verbose && count > 1) {
   	 msg("More than one FontOutline resource for %s\n", psname);
	 for (i = 0; i < count; i++) msg("\t<%s>\n", files[i]);
      }
      psfile = files[0];
      if (verbose) msg("Resource <%s>\n", psfile);
      psbasename = psname;
   }
   
   if (psfile != NULL) {
      char **names, **files; int count;

      if (afmname == NULL || ps_resource(afmname)) {
	 if (afmname == NULL) afmname = psname;
         count = ListPSResourceFiles(psrespath, NULL,
      	   "FontAFM", afmname, &names, &files);
         if (count == 0)
	    fatal("No FontAFM resource found for %s\n", afmname);
         if (verbose && count > 1) {
   	    msg("More than one FontAFM resource for %s\n", afmname);
	    for (i = 0; i < count; i++) msg("\t<%s>\n", files[i]);
   	 }
         afmfile = files[0];
         if (verbose) msg("Resource <%s>\n", afmfile);
      }
      else fatal("%s: invalid resource name\n", afmname);

      if (encname != NULL) {
	 if (!ps_resource(encname)) 
            fatal("%s: invalid Encoding resource\n", encname);
         count = ListPSResourceFiles(psrespath, NULL,
      	    "Encoding", encname, &names, &files);
         if (count == 0)
	    fatal("No Encoding resource found for %s\n", encname);
         if (verbose && count > 1) {
   	    msg("More than one Encoding resource for %s\n", encname);
	    for (i = 0; i < count; i++) msg("\t<%s>\n", files[i]);
         }
         encfile = files[0];
         if (verbose) msg("Resource <%s>\n", encfile);
      }
   }
   else { /* No PS resource database or no resource for type1font */
#endif   
#ifdef KPATHSEA
     psfile = kpse_find_file(psname,kpse_type1_format,0);
     if (!psfile) {
       /* kpse_find_file does not look for alternate suffixes */
       char *altname = newname(psname, ".pfb");
       psfile = kpse_find_file(altname, kpse_type1_format, 0);
       free(altname);
     }
     if (!psfile)
       fatal("%s: PS file %s not found!\n", kpse_invocation_name,
	     psname);
     if (afmname == NULL)
       afmname = newname(psname, ".afm");
     afmfile = kpse_find_file(afmname,kpse_afm_format,0);
     if (!afmfile)
       fatal("%s: AFM file %s not found!\n", kpse_invocation_name,
	     afmname);
     
     if (encname) {
       encfile = kpse_find_file(encname,kpse_enc_format,0);
       if (!encfile)
	 fatal("%s: enc file %s not found!\n", kpse_invocation_name,
	       encname);
     }
     else
       encfile = NULL;
     printf("psfile = %s\n", psfile);
     printf("afmfile = %s\n", afmfile);
     if (encfile) printf("encfile = %s\n", encfile);
#else
     t1inputs = path(T1INPUTS, getenv("T1INPUTS"));
     if (verbose) msg("T1INPUTS := %s\n", t1inputs);
     
     psfile = search_file(t1inputs, psname, FATAL);
     if (verbose) msg("Font file <%s>\n", psfile);
     
     if (afmname ==  NULL) afmname = newname(psfile, ".afm");
     afmfile = search_file(t1inputs, afmname, FATAL);
     if (verbose) msg("AFM file <%s>\n", afmfile);
     
     /* get encoding when -e<enc> provided */
     if (encname) {
       encfile = search_file(t1inputs, encname, FATAL);
       if (verbose) msg("Encoding file <%s>\n", encfile);
     }
#endif
     psbasename = basename(psname, extension(psname));
#ifndef KPATHSEA
   }
#endif
   psfilebn = basename(psfile, NULL);

   if (pointsize == 0.0) pointsize = DEFAULTPOINTSIZE;
   if (x_resolution == 0) x_resolution = DEFAULTRES;
   if (y_resolution == 0) y_resolution = x_resolution;
   if (verbose)
      msg("Loading encoding vector from %s ...",
	 encname? encname: basename(afmfile, NULL));
   getenc(&AFM_fontname, &encodingscheme, ev, WX);
   if (efactor != 1.0)
      for (i=0; i < 256; i++) {
         if (ev[i] == NULL) continue;
         WX[i] = WX[i] * efactor + 0.5;
      }
   if (verbose) msg(" done\n");
   
   if (argc == 1) strcpy(pkname,  argv[0]);
   else 
      sprintf(pkname, "%s%d.%dpk", psbasename,
         (int) (pointsize + 0.5), x_resolution);
    
   /* next values are needed! */
   vals.x =     x_resolution;
   vals.y =     y_resolution;
   vals.point = 10.0 * pointsize + 0.5; /* DECIPOINTS */
   vals.pixel = pointsize * y_resolution / POINTSPERINCH + 0.5;
   
   if (verbose) msg("Checking type1 font %s ...", psfilebn);
   Type1RegisterFontFileFunctions();
   if (verbose) msg(" done\n");

	/* next line prevents UNIX core dumps */
   entry.name.name = "-adobe-utopia-medium-r-normal--0-0-0-0-p-0-iso8859-1";
   if (verbose)
      msg("Creating character glyphs for %s ...", psfilebn);
   rc = Type1OpenScalable(ev, &fontptr, 0, &entry, 
            psfile, &vals, 0, 0, efactor, slant);
   if (rc != Succesful) 
      fatal("Type1OpenScalable error (result: %d) on %s \n",  rc, psfile);
   if (verbose) msg(" done\n");

   pk_open(pkname);
   sprintf(comment, "%s rendered at %f points", psbasename, pointsize);
   cs = pchecksum(ev, WX);
   pk_preamble(comment, pointsize, cs, x_resolution, y_resolution);
   charno = 0;
   if (verbose) msg("Creating %s from %s\n", pkname, psfilebn);
   for (charcode = 0; charcode < 256; charcode++) {
      if (ev[charcode] == NULL) continue;
      glyphcode[0] = charcode;
      (fontptr->get_glyphs)(fontptr, 1, glyphcode, 0, &count, glyphs);
      if (count > 0) {
         if (debug) {
            msg("'%03o ", charcode); charno++;
            if (charno == 8) { 
               msg("%c", '\n'); charno = 0;
            }
         }
	 first_pixel(glyphs[0]); /* assigns W and H */
         pk_char(charcode,			   /* charcode */
            TFMwidth(WX[charcode]), 		   /* TFMwidth */
            h_escapement(WX[charcode]),		   /* h_escapement */
            W, H,  				   /* width and height */
            - glyphs[0]->metrics.leftSideBearing,  /* h_offset */
            glyphs[0]->metrics.ascent,             /* v_offset */
            next_pixel);			   /* pixel generator */
      }
   }
   if (debug) msg("%c", '\n');
   ps2pk_postamble(psbasename, encodingscheme,
              base_resolution, x_resolution, y_resolution, pointsize,
	      ps2pk_args);
   pk_close();
   exit(0);
}

/*
 * The checksum should garantee that our PK file belongs to the correct TFM
 * file! Exactly the same as the afm2tfm (dvips5487) calculation.
 */
static uint32_t old_checksum(encoding ev, int width[256])
{
   int i;
   uint32_t s1 = 0, s2 = 0;
   char *p ;

   for (i=0; i<256; i++) {
      if (ev[i] == NULL) continue;
      s1 = (s1<<1) ^ width[i];                   /* left shift */
      for (p=ev[i]; *p; p++)
	 s2 = s2 * 3 + *p ;
   }
   return (s1<<1) ^ s2 ;
}

/*
 * The new checksum algorithm.
 */
static uint32_t checksum(encoding ev, int width[256])
{
   int i;
   uint32_t s1 = 0, s2 = 0;
   char *p ;

   for (i=0; i<256; i++) {
      if (ev[i] == NULL) continue;
      s1 = ((s1<<1) ^ (s1>>31)) ^ width[i];   /* cyclic left shift */
      for (p=ev[i]; *p; p++)
	 s2 = s2 * 3 + *p ;
   }
   return (s1<<1) ^ s2 ;
}

static int row, col;
static int data, bitno;
static unsigned char *p_data;

static int
next_pixel(void)
{
   int pixel;

   while (row < H) {
       if (col++ < W) { 
           if (bitno == 0) { data = *p_data++; bitno = 8; }
           pixel = data & 0x80? BLACK: WHITE; 
           data <<= 1; bitno--; return pixel;
       }
       col = 0; row++; bitno = 0;
   }
   fatal("Not that many pixels!\n");
} 

static void
first_pixel(CharInfoRec *G)
{
   row = col = 0;
   p_data = (unsigned char *) G->bits;
   W = G->metrics.rightSideBearing - G->metrics.leftSideBearing;
   H = G->metrics.ascent + G->metrics.descent;
   bitno = 0; 
}

#if 0
static void
print_pixmap(void)
{
   int c, r;
   unsigned char *save_p_data;

   save_p_data = p_data;
   if (W*H == 0) return; /* easy */
   for (r = 0; r < H; r++) {
      for (c = 0; c < W; c++)
	 if (next_pixel() == BLACK) msg("%c", 'X');
	 else msg("%c", '.');
      msg("%c", '\n');
   }
   /* reset data for scanning pixmap */
   p_data = save_p_data;
   bitno = 0; row = 0; col = 0;
}
#endif

/* Next function computes the width as a fix_word. A fix_word is 
   an integer representation for fractions. The decimal point is 
   left from bit 20. (The method is `stolen' from afm2tfm.) */
 
static int32_t
TFMwidth(int wx)
{  
   return (((wx  / 1000) << 20) +
           (((wx % 1000) << 20) + 500) / 1000) ;
}

static float HXU = -1.0; /* horizontal pixels per design unit */

/* the horizontal escapent is the number of pixels to next origin */
static int
h_escapement(int wx)
{
   if (HXU == -1.0) 
      HXU = (pointsize * x_resolution) / 72270.0;
   return wx * HXU +  0.5;
}

/* add ps2pk option to string */
static void
add_option(const char *option, const char *value)
{
   static char *p_args = NULL;

   if (p_args == NULL) {
      p_args = ps2pk_args;
      sprintf(p_args, "%s%s", option, value);
   }
   else
      sprintf(p_args, " %s%s", option, value);
   p_args+= strlen(p_args);
}

/* Next stuff is needed by type1 rendering functions */

int CheckFSFormat(int format, int fmask, int *bit, int *Byte, int *scan,
                  int *glyph, int *image)
{
       *bit = *Byte = 1;
       *glyph = *scan = *image = 1;
       return Successful;
 
}
 
intptr_t MakeAtom(const char *p, unsigned int len, int foo)
{
       return (intptr_t)p;
}

#if 0
void GetClientResolutions(int *resP)
{
       *resP = 0;
}
#endif

void *Xalloc(size_t size)
{
   void *p;
   p = malloc(size);
   if (p == NULL) fatal("Out of memory\n");
   return p;
}
 
void Xfree(void *p)
{
       free(p);
}
