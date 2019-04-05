/* misc.c */

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

  Copyright (C) 2002-2015 Jan-Åke Larsson

************************************************************************/

#include "dvipng.h"
#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#else
#define basename xbasename
#endif
#include <fcntl.h> /* open/close */
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif
#include <sys/stat.h>

static char *programname;

/*-->DecodeArgs*/
/*********************************************************************/
/***************************** DecodeArgs ****************************/
/*********************************************************************/
bool DecodeArgs(int argc, char ** argv)
{
  int     i;                 /* argument index for options     */
  bool ppused=false;         /* Flag when -pp is used          */
  int32_t number;            /* Temporary storage for numeric parameter */
  char *dviname=NULL;        /* Name of dvi file               */
  char *outname=NULL;        /* Name of output file            */
  char *base;                /* basename of argv[0]            */

#ifdef HAVE_LIBKPATHSEA
  /* we certainly don't want to modify argv[0].  */
  programname = xstrdup (argv[0] ? argv[0] : PACKAGE_NAME);
# ifdef HAVE_KPSE_PROGRAM_BASENAME
  base = kpse_program_basename (programname);
# else
  base = xstrdup (xbasename (programname));
  {
    char *dot = strrchr (base, '.');
    if (dot && FILESTRCASEEQ (dot, ".exe"))
      *dot = 0;
  }
# endif
#else
  /* we certainly don't want to modify argv[0].  */
  programname = strdup (argv[0] ? argv[0] : PACKAGE_NAME);
  base = strrchr (programname, '/');
  if (base)
    base++;
  else
    base = programname;
#endif
#ifdef HAVE_GDIMAGEGIF
# ifdef HAVE_LIBKPATHSEA
  if (FILESTRCASEEQ (base, "dvigif"))
# else
  if (strncmp(programname,"dvigif",6)==0)
# endif
    option_flags |= GIF_OUTPUT;
#endif
  Message(BE_NONQUIET,"This is %s",programname);
  if (strcmp(base,PACKAGE_NAME)!=0)
    Message(BE_NONQUIET," (%s)", PACKAGE_NAME);
  Message(BE_NONQUIET," %s Copyright 2002-2015 Jan-Ake Larsson\n",
	  PACKAGE_VERSION);

  for (i=1; i<argc; i++) {
    if (*argv[i]=='-') {
      char *p=argv[i]+2 ;
      char c=argv[i][1] ;
      if (c=='-') {
	p++;
	c=argv[i][2];
      }
      switch (c) {
      case 'd':       /* selects Debug output */
	if (*p>'9' && *p!='-') {
	  if (strncmp(p,"vinum",5)==0) {
	    if (p[5] != '0') {
	      option_flags |= DVI_PAGENUM;
	      Message(PARSE_STDIN,"DVI page number output on\n",p);
	    } else {
	      option_flags &= ~DVI_PAGENUM;
	      Message(PARSE_STDIN,"DVI page number output off\n");
	    }
	    break;
	  } else if (strncmp(p,"epth",4)==0) { /* Depth reporting */
	    if (p[4] != '0') {
	      option_flags |= REPORT_DEPTH;
	      Message(PARSE_STDIN,"Depth reporting on\n",p);
	    } else {
	      option_flags &= ~REPORT_DEPTH;
	      Message(PARSE_STDIN,"Depth reporting off\n");
	    }
	    break;
	  }
	  goto DEFAULT;
	} else {
#ifdef DEBUG
	  if (*p == 0 && argv[i+1])
	    p = argv[i+1];
	  debug = atoi(p);
	  if (debug > 0) {
	    if (p == argv[i+1]) i++;
	  } else
	    debug = DEBUG_DEFAULT;
	  Message(PARSE_STDIN,"Debug output enabled\n");
#ifdef HAVE_LIBKPATHSEA
	  kpathsea_debug = debug / LASTDEBUG / 2 ;
#endif
#else
	  Warning("This instance of %s was compiled without the debug (-d) option",
		  programname);
#endif
	  break;
	}
      case 'o':       /* Output file is specified */
	if (*p == 0 && argv[i+1])
	  p = argv[++i] ;
        outname=p;
        /* remove .png extension */
	Message(PARSE_STDIN,"Output file: %s\n",
		outname);
        break;
#ifdef MAKETEXPK
      case 'M':
        /* -M, -M1 => don't make font; -M0 => do.  */
        makeTexPK = (*p == '0');
#ifdef HAVE_LIBKPATHSEA
        kpse_set_program_enabled (kpse_pk_format, makeTexPK, kpse_src_cmdline);
#endif
	if (makeTexPK)
	  Message(PARSE_STDIN,"MakeTeXPK enabled\n");
	else
	  Message(PARSE_STDIN,"MakeTeXPK disabled\n");
        break;
#endif /* MAKETEXPK */
      case 'm':
	if (strcmp(p,"ode") == 0 ) {
	  if (argv[i+1])
	    user_mfmode = argv[++i] ;
	  Message(PARSE_STDIN,"MetaFont mode: %s\n",user_mfmode);
	  break;
	}
	goto DEFAULT;
      case 'h':
	if (strcmp(p,"elp") == 0 ) {
	  break;
	} else if (strncmp(p,"eight",5) == 0 ) { /* Height reporting */
	  if (p[5] != '0') {
	    option_flags |= REPORT_HEIGHT;
	    Message(PARSE_STDIN,"Height reporting on\n",p);
	  } else {
	    option_flags &= ~REPORT_HEIGHT;
	    Message(PARSE_STDIN,"Height reporting off\n");
	  }
	  break;
	}
	goto DEFAULT;
      case 'O' : /* Offset */
	if (*p == 0 && argv[i+1])
	  p = argv[++i] ;
	handlepapersize(p, &x_offset_def, &y_offset_def) ;
	Message(PARSE_STDIN,"Offset: (%d,%d)\n",x_offset_def,y_offset_def);
        break ;
      case 'e':
	if (strncmp(p,"xpand-bbox",10) == 0 ) {
	  if (p[10] != '0') {
	    option_flags |= EXPAND_BBOX;
	    Message(PARSE_STDIN,"BBox expansion on\n",p);
	  } else {
	    option_flags &= ~EXPAND_BBOX;
	    Message(PARSE_STDIN,"BBox expansion off\n");
	  }
	  break;
	}
	goto DEFAULT;
      case 'T' :
	if (*p == 0 && argv[i+1])
	  p = argv[++i];
	if (strcmp(p,"bbox")==0) {
	  x_width_def=0;
	  y_width_def=0;
	  Message(PARSE_STDIN,"Pagesize: (bbox)\n");
	} else if (strcmp(p,"tight")==0) {
	  x_width_def=-1;
	  y_width_def=-1;
	  Message(PARSE_STDIN,"Pagesize: (tight bbox)\n");
	} else {
	  handlepapersize(p, &x_width_def, &y_width_def) ;
	  Message(PARSE_STDIN,"Pagesize: (%d,%d)\n",
		  x_width_def,y_width_def);
	}
	break ;
      case 't':       /* specify paper format, only for cropmarks */
#ifdef HAVE_GDIMAGECREATETRUECOLOR
	/* Truecolor */
	if (strncmp(p,"ruecolor",8)==0) {
	  if (p[8] != '0') {
	    option_flags |= FORCE_TRUECOLOR;
	    Message(PARSE_STDIN,"Truecolor mode on\n",p);
	  } else {
	    option_flags &= ~FORCE_TRUECOLOR;
	    Message(PARSE_STDIN,"Truecolor mode off\n");
	  }
	} else
#endif
	  { /* cropmarks not implemented yet */
	    if (*p == 0 && argv[i+1])
	      p = argv[++i] ;
	    if (strcmp(p,"a4")==0) {
	      handlepapersize("210mm,297mm",&x_pwidth,&y_pwidth);
	    } else if (strcmp(p,"letter")==0) {
	      handlepapersize("8.5in,11in",&x_pwidth,&y_pwidth);
	    } else if (strcmp(p,"legal")==0) {
	      handlepapersize("8.5in,14in",&x_pwidth,&y_pwidth);
	    } else if (strcmp(p,"executive")==0) {
	      handlepapersize("7.25in,10.5in",&x_pwidth,&y_pwidth);
	    } else
	      Fatal("papersize %s is not implemented",p);
	    Message(PARSE_STDIN,"Papersize: %s\n",p);
	  }
	break;
/*       case 'c': */
/* 	if (strncmp(p,"acheimages",10)==0) {  */
/* 	  if (p[10] != '0') { */
/* 	    option_flags |= CACHE_IMAGES; */
/* 	    Message(PARSE_STDIN,"Caching images\n",p); */
/* 	  } else {  */
/* 	    option_flags &= ~CACHE_IMAGES; */
/* 	    Message(PARSE_STDIN,"Not caching images\n"); */
/* 	  } */
/* 	  break; */
/* 	} */
/* 	goto DEFAULT; */
      case 'b':
	if ( *p == 'g' ) { /* -bg background color */
	  p++;
	  if (*p == 0 && argv[i+1])
	    p = argv[++i] ;
	  if (strncmp(p,"Transparent",11) == 0 )
	    option_flags |= BG_TRANSPARENT_ALPHA;
	  else if (strncmp(p,"transparent",11) == 0 )
	    option_flags |= BG_TRANSPARENT;
	  else
	    background(p);
	  if (option_flags & BG_TRANSPARENT)
	    Message(PARSE_STDIN,"Transp. background (fallback rgb %d,%d,%d)\n",
		    cstack[0].red,cstack[0].green,cstack[0].blue);
	  else
	    Message(PARSE_STDIN,"Background: rgb %d,%d,%d\n",
		    cstack[0].red,cstack[0].green,cstack[0].blue);
	  break;
	} else if (strcmp(p, "dpi")==0) {
	  p+=3;
	  if (*p == 0 && argv[i+1])
	    p = argv[++i];
	  number = atoi(p);
	  if (number < 10 || number > 10000)
	    Warning("Bad --bdpi parameter, ignored");
	  else {
	    user_bdpi=number;
	    Message(PARSE_STDIN,"Bdpi: %d\n",user_bdpi);
	  }
	  break;
	} else if ( *p == 'd' ) { /* -bd border width */
	  int tmpi;
	  char* tmps;
	  p++;
	  if (*p == 0 && argv[i+1])
	    p = argv[++i] ;
	  tmpi = strtol(p, &tmps, 10);
	  if (p<tmps) {
	    borderwidth=tmpi;
	    Message(PARSE_STDIN,"Transp. border: %d dots\n",borderwidth);
	  }
	  if (*tmps) {
	    stringrgb(tmps, &bordercolor.red, &bordercolor.green, &bordercolor.blue);
	    userbordercolor=TRUE;
	    if (borderwidth==0) {
	      borderwidth=1;
	      Message(PARSE_STDIN,"Transp. border: %d dots\n",borderwidth);
	    }
	    Message(PARSE_STDIN,"Transp. border: rgb %d,%d,%d\n",
		    bordercolor.red, bordercolor.green, bordercolor.blue);
	  }
	  break;
	}
	goto DEFAULT;
      case 'f':
	if ( *p == 'g' ) { /* -fg foreground color */
	  p++;
	  if (*p == 0 && argv[i+1])
	    p = argv[++i] ;
	  resetcolorstack(p);
	  Message(PARSE_STDIN,"Foreground: rgb %d,%d,%d\n",
		  cstack[1].red,cstack[1].green,cstack[1].blue);
	  break;
#ifdef HAVE_FT2
	} else if ( strncmp(p,"reetype",7) == 0 ) { /* -freetype activation */
	  if (p[7] != '0') {
	    option_flags |= USE_FREETYPE;
	    Message(PARSE_STDIN,"FreeType rendering on\n",p);
	  } else {
	    option_flags &= ~USE_FREETYPE;
	    Message(PARSE_STDIN,"FreeType rendering off\n");
	  }
	  break;
#endif
	} else if (strcmp(p,"ollow") == 0 ) {
	  if (DVIFollowToggle())
	    Message(PARSE_STDIN,"Follow mode on\n");
	  else
	    Message(PARSE_STDIN,"Follow mode off\n");
	  break;
	}
	goto DEFAULT;
      case 'x' : case 'y' :
	if (*p == 0 && argv[i+1])
	  p = argv[++i] ;
	number = atoi(p);
	if (number < 1 || number > 1000000)
	  Warning("Bad magnification parameter (-x or -y), ignored");
	else {
	  usermag=number;
	  Message(PARSE_STDIN,"Magstep: %d\n",usermag);
	/*overridemag = (c == 'x' ? 1 : -1) ;*/
	}
	break ;
      case 'g' :
	if (strncmp(p,"amma",4)==0) { /* --gamma correction */
	  double gamma=0.0;

	  p+=4;
	  if (*p == 0 && argv[i+1])
	    p = argv[++i];
	  if (p!=NULL)
	    gamma=atof(p);
	  if (gamma==0.0) {
	    Warning("Bad gamma value, default is %f",DEFAULT_GAMMA);
	    gamma=DEFAULT_GAMMA;
	  }
	  Gamma(gamma);
	  Message(PARSE_STDIN,"Gamma value is %s\n", gamma);
	  break;
#ifdef HAVE_GDIMAGEGIF
	} else if (strncmp(p,"if",2)==0) { /* --gif output */
	  option_flags |= GIF_OUTPUT;
	  Message(PARSE_STDIN,"GIF output\n");
	  break;
#endif
	}
	goto DEFAULT;
      case 'p' :
	if (*p == 'p') {  /* a -pp specifier for a page list */
	  ppused=true;
	  p++ ;
	  if (*p == 0 && argv[i+1])
	    p = argv[++i];
	  Message(PARSE_STDIN,"Page list: %s\n",p);
	  if (ParsePages(p))
	    Fatal("bad page list specifier (-pp)");
	} else if (strncmp(p,"ng",2)==0) { /* --png output */
	  option_flags &= ~GIF_OUTPUT;
	  Message(PARSE_STDIN,"PNG output\n");
	} else if (strncmp(p,"icky",4)==0) {
	  if (p[4] != '0') {
	    option_flags |= MODE_PICKY;
	    Message(PARSE_STDIN,"No images output for pages with warnings\n",p);
	  } else {
	    option_flags &= ~MODE_PICKY;
	    Message(PARSE_STDIN,"Images output even for pages with warnings\n");
	  }
	} else if (strncmp(p,"alette",6)==0) {
	  if (p[6] != '0') {
	    option_flags |= FORCE_PALETTE;
	    Message(PARSE_STDIN,"Forcing 256-color PNG output\n",p);
	  } else {
	    option_flags &= ~FORCE_PALETTE;
	    Message(PARSE_STDIN,"Allows truecolor PNG output\n");
	  }
	} else {   /* a -p specifier for first page */
	  int32_t firstpage;
	  bool abspage=false;

	  if (*p == 0 && argv[i+1])
	    p = argv[++i] ;
	  if (*p == '=') {
	    abspage=true;
	    p++ ;
	  }
	  if ((*p>='0'&&*p<='9') || (*p=='-' && *(p+1)>='0' && *(p+1)<='9')) {
	    firstpage = atoi(p);
	    FirstPage(firstpage,abspage);
	    Message(PARSE_STDIN,"First page: %d\n",firstpage);
	  } else
	    Warning("Bad firstpage (-p) parameter, ignored");
	}
	break ;
      case 's' :
	if (strncmp(p,"trict",5)==0) {
	  if (p[5] != '0') {
	    option_flags |= MODE_STRICT;
	    Message(PARSE_STDIN,"Warnings are fatal\n",p);
	  } else {
	    option_flags &= ~MODE_STRICT;
	    Message(PARSE_STDIN,"Warnings are not fatal\n");
	  }
	} else
	  goto DEFAULT;
	break ;
      case 'n' :
	if (strncmp(p,"oghostscript",12)==0) {
	  if (p[12] != '0') {
	    option_flags |= NO_GHOSTSCRIPT;
	    Message(PARSE_STDIN,"No GhostScript calls\n",p);
	  } else {
	    option_flags &= ~NO_GHOSTSCRIPT;
	    Message(PARSE_STDIN,"GhostScript calls made\n");
	  }
	} else if (strncmp(p,"ogssafer",8)==0) {
	  if (p[8] != '0') {
	    option_flags |= NO_GSSAFER;
	    Message(PARSE_STDIN,"GhostScript calls does not use -dSAFER\n",p);
	  } else {
	    option_flags &= ~NO_GSSAFER;
	    Message(PARSE_STDIN,"GhostScript calls use -dSAFER\n");
	  }
	} else if (strncmp(p,"ogs",3)==0) {
	  if (p[3] != '0') {
	    option_flags |= NO_GHOSTSCRIPT;
	    Message(PARSE_STDIN,"No GhostScript calls\n",p);
	  } else {
	    option_flags &= ~NO_GHOSTSCRIPT;
	    Message(PARSE_STDIN,"GhostScript calls made\n");
	  }
	} else if (strncmp(p,"orawps",6)==0) {
	  if (p[6] != '0') {
	    option_flags |= NO_RAW_PS;
	    Message(PARSE_STDIN,"Conversion of raw PostScript will not be attempted\n",p);
	  } else {
	    option_flags &= ~NO_RAW_PS;
	    Message(PARSE_STDIN,"Conversion of raw PostScript will be attempted\n",p);
	  }
	} else
	  goto DEFAULT;
	break ;
      case 'l':
	{
	  int32_t lastpage;
	  bool abspage=false;

	  if (*p == 0 && argv[i+1])
	    p = argv[++i] ;
	  if (*p == '=') {
	    abspage=true;
	    p++ ;
	  }
	  if ((*p>='0'&&*p<='9') || (*p=='-' && *(p+1)>='0' && *(p+1)<='9')) {
	    lastpage = atoi(p);
	    LastPage(lastpage,abspage);
	    Message(PARSE_STDIN,"Last page: %d\n",lastpage);
	  } else
	    Warning("Bad lastpage (-l) parameter, ignored");
	}
	break ;
      case 'q':       /* quiet operation */
	if (*p != '0')
	  option_flags &= (~BE_NONQUIET & ~BE_VERBOSE);
        else
	  option_flags |= BE_NONQUIET;
	break;
      case 'r':       /* process pages in reverse order */
	if (*p != '0') {
	  Message(PARSE_STDIN,"Reverse order\n");
	  Reverse(true);
	} else {
	  Message(PARSE_STDIN,"Normal order\n");
	  Reverse(false);
	}
        break;
      case 'v':    /* verbatim mode */
	if (strcmp(p, "ersion")==0) {
	  if (strcmp(basename(programname),PACKAGE_NAME)!=0)
	    printf("%s (%s) %s\n",basename(programname),
		   PACKAGE_NAME,PACKAGE_VERSION);
	  else
	    puts(PACKAGE_STRING);
#ifdef HAVE_LIBKPATHSEA
	  puts (KPSEVERSION);
#endif
#ifdef HAVE_FT2
	  printf("Compiled with Freetype %d.%d.%d\n",
		 FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH);
#  ifdef HAVE_FT_LIBRARY_VERSION
	  if (FT_Init_FreeType( &libfreetype ))
	    Warning("the Freetype library seems unusable");
	  else {
	    FT_Int      amajor, aminor, apatch;

	    FT_Library_Version( libfreetype, &amajor, &aminor, &apatch );
	    printf("Using libft %d.%d.%d\n",amajor, aminor, apatch);
	    FT_Done_FreeType(libfreetype);
	  }
#  endif
#endif
	  puts ("Copyright (C) 2002-2015 Jan-Ake Larsson.\n\
There is NO warranty.  You may redistribute this software\n\
under the terms of the GNU Lesser General Public License\n\
version 3, see the COPYING file in the dvipng distribution\n\
or <http://www.gnu.org/licenses/>.");
	  exit (EXIT_SUCCESS);
	}
	if (*p != '0')
	  option_flags |= BE_NONQUIET | BE_VERBOSE;
	else
	  option_flags &= ~BE_VERBOSE;
        break;
      case 'D' :
	if (*p == 0 && argv[i+1])
	  p = argv[++i] ;
	number = atoi(p);
	if (number < 10 || number > 10000)
	  Warning("Bad resolution (-D) parameter, ignored") ;
	else {
	  dpi=number;
	  Message(PARSE_STDIN,"Dpi: %d\n",dpi);
	}
	break;
      case 'Q':       /* quality (= shrinkfactor) */
	if (*p == 0 && argv[i+1])
	  p = argv[++i];
	if (*p>='0'&&*p<='9') {
	  shrinkfactor = atoi(p);
	  Message(PARSE_STDIN,"Quality: %d\n",shrinkfactor);
	} else {
	  Warning("Non-numeric quality (-Q) value, ignored");
	}
	break;
      case 'w':
	if (strncmp(p,"idth",4) == 0 ) { /* Width reporting */
	  if (p[4] != '0') {
	    option_flags |= REPORT_WIDTH;
	    Message(PARSE_STDIN,"Width reporting on\n",p);
	  } else {
	    option_flags &= ~REPORT_WIDTH;
	    Message(PARSE_STDIN,"Width reporting off\n");
	  }
	  break;
	}
	goto DEFAULT;
#ifdef HAVE_GDIMAGEPNGEX
      case 'z':
	if (*p == 0 && argv[i+1])
	  p = argv[++i];
	number = atoi(p);
	if (number<1 || number>9)
	  Warning("Bad compression (-z) value, ignored");
	else {
	  compression=number;
	  Message(PARSE_STDIN,"Compression: %d\n",shrinkfactor);
	}
	break;
#endif
      case '\0':
	option_flags |= PARSE_STDIN;
	break;
      DEFAULT: default:
	Warning("%s is not a valid option", argv[i]);
      }
    } else {
      dviname=argv[i];
    }
  }
  if (dviname != NULL) {
    if (dvi != NULL && dvi->filep != NULL)
      DVIClose(dvi);
    dvi=DVIOpen(dviname,outname);
  }

  if (dvi==NULL) {
    fprintf(stdout,"\nUsage: %s [OPTION]... FILENAME[.dvi]\n", programname);
    fprintf(stdout,"Options are chosen to be similar to dvips' options where possible:\n");
#ifdef DEBUG
    fprintf(stdout,"  -d #         Debug (# is the debug bitmap, 1 if not given)\n");
#endif
    fprintf(stdout,"  -D #         Output resolution\n");
    fprintf(stdout,"  -l #         Last page to be output\n");
    fprintf(stdout,"  -o f         Output file, '%%d' is pagenumber\n");
    fprintf(stdout,"  -O c         Image offset\n");
    fprintf(stdout,"  -p #         First page to be output\n");
    fprintf(stdout,"  -pp #,#..    Page list to be output\n");
    fprintf(stdout,"  -q*          Quiet operation\n");
    fprintf(stdout,"  -T c         Image size (also accepts '-T bbox' and '-T tight')\n");
    fprintf(stdout,"  -v*          Verbose operation\n");
    fprintf(stdout,"  -            Interactive query of options\n");
    fprintf(stdout,"\nThese do not correspond to dvips options:\n");
    fprintf(stdout,"  -bd #        Transparent border width in dots\n");
    fprintf(stdout,"  -bd s        Transparent border fallback color (TeX-style color)\n");
    fprintf(stdout,"  -bg s        Background color (TeX-style color or 'Transparent')\n");
    fprintf(stdout,"  --depth*     Output the image depth on stdout\n");
    fprintf(stdout,"  --dvinum*    Use TeX page numbers in output filenames\n");
    fprintf(stdout,"  -fg s        Foreground color (TeX-style color)\n");
    fprintf(stdout,"  --follow*    Wait for data at end-of-file\n");
#ifdef HAVE_FT2
    fprintf(stdout,"  --freetype*  FreeType font rendering (preferred, default on)\n");
#endif
    fprintf(stdout,"  --gamma #    Control color interpolation\n");
#ifdef HAVE_GDIMAGEGIF
    fprintf(stdout,"  --gif        Output GIF images (dvigif default)\n");
#endif
    fprintf(stdout,"  --height*    Output the image height on stdout\n");
    fprintf(stdout,"  --nogs*      Don't use ghostscript for PostScript specials\n");
    fprintf(stdout,"  --nogssafer* Don't use -dSAFER in ghostscript calls\n");
    fprintf(stdout,"  --norawps*   Don't convert raw PostScript specials\n");
#ifdef HAVE_GDIMAGECREATETRUECOLOR
    fprintf(stdout,"  --palette*   Force palette output\n");
#endif
    fprintf(stdout,"  --picky      When a warning occurs, don't output image\n");
#ifdef HAVE_GDIMAGEGIF
    fprintf(stdout,"  --png        Output PNG images (dvipng default)\n");
#endif
    fprintf(stdout,"  --strict     When a warning occurs, exit\n");
#ifdef HAVE_GDIMAGECREATETRUECOLOR
    fprintf(stdout,"  --truecolor* Truecolor output\n");
#endif
    fprintf(stdout,"  -Q #         Quality (PK subsampling)\n");
    fprintf(stdout,"  --width*     Output the image width on stdout\n");
#ifdef HAVE_GDIMAGEPNGEX
    fprintf(stdout,"  -z #         PNG compression level\n");
#endif

    fprintf(stdout,"\n   # = number   f = file   s = string  * = suffix, '0' to turn off\n");
    fprintf(stdout,"       c = comma-separated dimension pair (e.g., 3.2in,-32.1cm)\n\n");
    /*#ifdef HAVE_LIBKPATHSEA
      {
      extern DllImport char *kpse_bug_address;
      putc ('\n', stdout);
      fputs (kpse_bug_address, stdout);
      }
      #endif*/
    if ((option_flags & PARSE_STDIN) == 0) {
      exit(EXIT_SUCCESS);
    }
  }
  if ((option_flags & PARSE_STDIN) == 0 && (!ppused))
    ParsePages("-");
  return((option_flags & PARSE_STDIN) != 0);
}

void DecodeString(char *string)
{
#define PARSEARGS 10
  char    *strv[PARSEARGS];
  int     strc=1;
  strv[0]=NULL;                       /* No program name */

  while (*string==' ' || *string=='\t' || *string=='\r' || *string=='\n')
    string++;
  while (*string!='\0') {
    strv[strc++]=string;
    if (*string!='\'') {
      /* Normal split at whitespace */
      while (*string!=' ' && *string!='\t' \
	     && *string!='\n' && *string!='\r' && *string!='\0')
	string++;
    } else {
      /* String delimiter found , so search for next */
      strv[strc-1]=++string;
      while (*string!='\'' && *string!='\0')
	string++;
    }
    if (*string!='\0')
      *string++='\0';
    while (*string==' ' || *string=='\t' || *string=='\r' || *string=='\n')
      string++;
  }
  if (strc>1) /* Nonempty */
    (void) DecodeArgs(strc,strv);
}



uint32_t UNumRead(unsigned char* current, register int n)
{
  uint32_t x = (unsigned char) *(current)++; /* number being constructed */
  while(--n)
    x = (x << 8) | *(current)++;
  return(x);
}

int32_t SNumRead(unsigned char* current, register int n)
{
  int32_t x = (signed char) *(current)++; /* number being constructed */
  while(--n)
    x = (x << 8) | *(current)++;
  return(x);
}


/**********************************************************************/
/******************************  Fatal  *******************************/
/**********************************************************************/
void Fatal (const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  fflush(stdout);
  fprintf(stderr, "\n");
  fprintf(stderr, "%s: Fatal error, ", programname);
  vfprintf(stderr, fmt, args);

  fprintf(stderr, "\n\n");
  va_end(args);

  ClearFonts();
#ifdef HAVE_FT2
  if (libfreetype)
    (void) FT_Done_FreeType(libfreetype); /* at this point, ignore error */
#endif
  exit(EXIT_FATAL);
}

/*-->Warning*/
/**********************************************************************/
/*****************************  Warning  ******************************/
/**********************************************************************/
void Warning(const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);

  if ( option_flags & BE_NONQUIET ) {
    fflush(stdout);
    fprintf(stderr, "%s warning: ", programname);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, " ");
  }
  va_end(args);
}

/*-->Message*/
/**********************************************************************/
/*****************************  Message  ******************************/
/**********************************************************************/
void Message(int activeflags, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  if ( option_flags & activeflags ) {
    vfprintf(stdout, fmt, args);
  }
  va_end(args);
}


bool MmapFile (char *filename,struct filemmap *fmmap)
{
#if !defined(WIN32) || !defined(MIKTEX_ENABLE_WIN32_MMAP)
  struct stat stat;
#endif
#if defined(MIKTEX)
  int n;
#endif
#if defined(MIKTEX_WINDOWS)
  wchar_t wfilename[_MAX_PATH];
  miktex_utf8_to_wide_char (filename, _MAX_PATH, wfilename);
#endif

  DEBUG_PRINT(DEBUG_DVI,("\n  OPEN FILE:\t'%s'", filename));
  fmmap->data=NULL;
#if !defined(WIN32) || !defined(MIKTEX_ENABLE_WIN32_MMAP)
#if defined(MIKTEX_WINDOWS)
  if ((fmmap->fd = open(filename, _O_RDONLY | _O_BINARY)) == -1)
  {
#else
  if ((fmmap->fd = open(filename,O_RDONLY)) == -1) {
#endif
    Warning("cannot open file <%s>", filename);
    return(true);
  }
  fstat(fmmap->fd,&stat);
  fmmap->size=stat.st_size;
# ifdef HAVE_MMAP
  fmmap->data = mmap(NULL,fmmap->size, PROT_READ, MAP_SHARED,fmmap->fd,0);
  if (fmmap->data == (char*)-1) {
    Warning("cannot mmap file <%s>",filename);
    fmmap->data=NULL;
    close(fmmap->fd);
    return(true);
  }
# else /* HAVE_MMAP */
	if ((fmmap->data = malloc(fmmap->size+1)) == NULL) {
    Warning("cannot malloc space for <%s>",filename);
    close(fmmap->fd);
    return(true);
  }
  if (read(fmmap->fd,fmmap->data,fmmap->size)<fmmap->size) {
    Warning("too little data in <%s>",filename);
    free(fmmap->data);
    fmmap->data=NULL;
    close(fmmap->fd);
    return(true);
  }
  close(fmmap->fd);
# endif /* HAVE_MMAP */
#else /* WIN32 */
#if defined(MIKTEX_WINDOWS)
  fmmap->hFile = CreateFile(wfilename, GENERIC_READ, FILE_SHARE_READ, 0,
#else
  fmmap->hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0,
#endif
			    OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);
  if (fmmap->hFile == INVALID_HANDLE_VALUE) {
    Warning("cannot open file <%s>", filename);
    return(true);
  }
  fmmap->size = GetFileSize(fmmap->hFile, 0);
  fmmap->hMap = CreateFileMapping(fmmap->hFile, 0, PAGE_READONLY, 0, 0, 0);
  if (fmmap->hMap == 0) {
    CloseHandle (fmmap->hFile);
    Warning("cannot CreateFileMapping() file <%s>", filename);
    return(true);
  }
  fmmap->data = MapViewOfFile(fmmap->hMap, FILE_MAP_READ, 0, 0, 0);
  if (fmmap->data == NULL) {
    Warning("cannot MapViewOfFile() file <%s>", filename);
    CloseHandle (fmmap->hMap);
    CloseHandle (fmmap->hFile);
    return(true);
  }
#endif  /* WIN32 */
  return(false);
}

void UnMmapFile(struct filemmap* fmmap)
{
  if (fmmap->data!=NULL) {
#if !defined(WIN32) || !defined(MIKTEX_ENABLE_WIN32_MMAP)
# ifdef HAVE_MMAP
    if (munmap(fmmap->data,fmmap->size))
      Warning("cannot munmap file at 0x%X",fmmap->data);
    if (close(fmmap->fd))
      Warning("cannot close file descriptor %d",fmmap->fd);
# else /* HAVE_MMAP */
    free(fmmap->data);
# endif /* HAVE_MMAP */
#else  /* WIN32 */
    UnmapViewOfFile (fmmap->data);
    CloseHandle (fmmap->hMap);
    CloseHandle (fmmap->hFile);
#endif	/* WIN32 */
  }
  fmmap->data=NULL;
}
