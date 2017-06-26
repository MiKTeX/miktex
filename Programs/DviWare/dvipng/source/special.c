/* special.c */

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
#if defined(MIKTEX)
#  include <miktex/Core/c/api.h>
#  define _TCHAR char
#  define _MAX_PATH 260
#endif

#ifndef MIKTEX
#ifndef WIN32
#include <sys/wait.h>
#else /* WIN32 */
#include <fcntl.h>
#include <io.h>
#include <process.h>
#define pipe(p) _pipe(p, 65536, O_BINARY | _O_NOINHERIT)
#define snprintf _snprintf
#endif /* WIN32 */
#endif

#define SKIPSPACES(s) while(s && *s==' ' && *s!='\0') s++

/* PostScript can come as a string (headers and raw specials) or
   a memory-mapped file (headers and included EPS figures). */

struct pscode {
  struct pscode*  next;
  char*           special;  /* complete special */
  char*           code;     /* PS string, null if a file */
  char*           filename; /* file name, null if a string */
  char*           postcode; /* post PS string */
  struct filemmap fmmap;    /* file mmap */
};

struct pscode* psheaderp=NULL; /* static, DVI-specific header list */

static void PSCodeInit(struct pscode *entry, char *special)
{
  entry->next=NULL;
  entry->special=special;
  entry->code=NULL;
  entry->filename=NULL;
  entry->postcode=NULL;
  entry->fmmap.data=NULL;
  if (special==NULL)
    return;
  if (strncmp(special,"header=",7)==0)
    entry->filename=special+7;
  else if (strncmp(special,"ps:: plotfile ",14)==0)
    entry->filename=special+14;
  else if (special[0]=='"' || special[0]=='!')
    entry->code=special+1;
  else if (strncmp(special,"ps::[begin]",11)==0)
    entry->code=special+11;
  else if (strncmp(special,"ps::[end]",9)==0)
    entry->code=special+9;
  else if (strncmp(special,"ps::",4)==0)
    entry->code=special+4;
  else if (strncmp(special,"ps:",3)==0)
    entry->code=special+3;
  else
    entry->code=special;
#ifdef DEBUG
  if (entry->code!=NULL)
    DEBUG_PRINT(DEBUG_GS,(" '%s'",entry->code));
  if (entry->filename!=NULL)
    DEBUG_PRINT(DEBUG_GS,(" {%s}",entry->filename));
  if (entry->postcode!=NULL)
    DEBUG_PRINT(DEBUG_GS,(" '%s'",entry->postcode));
#endif
}



void ClearPSHeaders(void)
{
  struct pscode *temp=psheaderp;

  while(temp!=NULL) {
    psheaderp=psheaderp->next;
    if (temp->fmmap.data!=NULL)
      UnMmapFile(&(temp->fmmap));
    free(temp);
    temp=psheaderp;
  }
}

static void writepscode(FILE* psstream,struct pscode* pscodep)
{
  while (pscodep!=NULL) {
    if (pscodep->code!=NULL) {
      fputs(pscodep->code,psstream);
      putc('\n',psstream);
      DEBUG_PRINT(DEBUG_GS,("\n  PS CODE:\t%s",pscodep->code));
    }
    if (pscodep->filename!=NULL && pscodep->fmmap.data==NULL) {
      char* filepath=
	kpse_find_file(pscodep->filename,kpse_tex_ps_header_format,false);
      if (filepath==NULL) {
	Warning("Cannot find PostScript file %s, ignored", pscodep->filename);
	page_flags |= PAGE_GAVE_WARN;
      } else if (MmapFile(filepath,&(pscodep->fmmap))) {
	Warning("PostScript file %s unusable, ignored", pscodep->filename);
	page_flags |= PAGE_GAVE_WARN;
      }
    }
    if (pscodep->fmmap.data!=NULL) {
      unsigned char* position;

      DEBUG_PRINT(DEBUG_GS,("\n  PS FILE:\t%s",pscodep->filename));
      position=(unsigned char*)pscodep->fmmap.data;
      while(position
	    < (unsigned char*)pscodep->fmmap.data + pscodep->fmmap.size) {
	putc(*position,psstream);
	position++;
      }
    }
    if (pscodep->postcode!=NULL) {
      fputs(pscodep->postcode,psstream);
      putc('\n',psstream);
      DEBUG_PRINT(DEBUG_GS,("\n  PS POST CODE:\t%s",pscodep->postcode));
    }
    pscodep=pscodep->next;
  }
}


static gdImagePtr
ps2png(struct pscode* pscodep, const char *device, int hresolution, int vresolution,
       int llx, int lly, int urx, int ury, int bgred, int bggreen, int bgblue)
{
#if !defined(MIKTEX)
  int pspipe[2], pngpipe[2];
#define READ_END 0
#define WRITE_END 1
#endif
  FILE *psstream=NULL, *pngstream=NULL;
  char resolution[STRSIZE];
  /*   char devicesize[STRSIZE];  */
  gdImagePtr psimage=NULL;
#ifndef MIKTEX
#ifndef WIN32
  pid_t pid;
#else /* WIN32 */
  unsigned long nexitcode = STILL_ACTIVE;
  HANDLE hchild;
  int savestdin, savestdout;
#endif /* WIN32 */
#else /* MIKTEX */
#if !defined(MIKTEX)
  HANDLE hPngStream;
  HANDLE hPsStream;
  HANDLE hStdErr;
  PROCESS_INFORMATION pi;
#endif
  _TCHAR szCommandLine[2048];
  _TCHAR szGsPath[_MAX_PATH];
#define GS_PATH szGsPath
#if !defined(MIKTEX)
#define fdopen _tfdopen
#define close _close
#endif
#endif /* MIKTEX */

  snprintf(resolution,STRSIZE,"-r%dx%d",hresolution,vresolution);
  /* Future extension for \rotatebox
  status=sprintf(devicesize, "-g%dx%d",
		 //(int)((sin(atan(1.0))+1)*
		 (urx - llx)*hresolution/72,//),
		 //(int)((sin(atan(1.0))+1)*
		 (ury - lly)*vresolution/72);//);
  */
  DEBUG_PRINT(DEBUG_GS,
	      ("\n  GS CALL:\t%s %s %s %s %s %s %s %s %s %s %s",/* %s", */
	       GS_PATH, device, resolution, /*devicesize,*/
	       "-dBATCH", "-dNOPAUSE", "-q", "-sOutputFile=-",
	       "-dTextAlphaBits=4", "-dGraphicsAlphaBits=4",
	       (option_flags & NO_GSSAFER) ? "-": "-dSAFER",
	       (option_flags & NO_GSSAFER) ? "": "- "));
#ifndef MIKTEX
  if (pipe(pspipe) || pipe(pngpipe)) return(NULL);
#ifndef WIN32
  /* We have fork: execute gs in child */
  pid = fork();
  if (pid == 0) { /* Child, execute gs. */
    close(pspipe[WRITE_END]);
    dup2(pspipe[READ_END], STDIN_FILENO);
    close(pspipe[READ_END]);
    close(pngpipe[READ_END]);
    dup2(pngpipe[WRITE_END], STDOUT_FILENO);
    close(pngpipe[WRITE_END]);
    execlp(GS_PATH, GS_PATH, device, resolution, /*devicesize,*/
	   "-dBATCH", "-dNOPAUSE", "-q", "-sOutputFile=-",
	   "-dTextAlphaBits=4", "-dGraphicsAlphaBits=4",
	   (option_flags & NO_GSSAFER) ? "-": "-dSAFER",
	   (option_flags & NO_GSSAFER) ? NULL: "-",
	   NULL);
    _exit (EXIT_FAILURE);
  }
#else /* WIN32 */
  /* No fork but spawn: execute gs in present process environment.
     Save fileno's, attach pipes to this process' stdin and stdout. */
  savestdin = _dup(fileno(stdin));
  _dup2(pspipe[READ_END], fileno(stdin));
  savestdout = _dup(fileno(stdout));
  _dup2(pngpipe[WRITE_END], fileno(stdout));
  if ((hchild=
       (HANDLE)spawnlp(_P_NOWAIT, GS_PATH, GS_PATH, device, resolution,
		       "-dBATCH", "-dNOPAUSE", "-q", "-sOutputFile=-",
		       "-dTextAlphaBits=4", "-dGraphicsAlphaBits=4",
		       (option_flags & NO_GSSAFER) ? "-": "-dSAFER",
		       (option_flags & NO_GSSAFER) ? NULL : "-", NULL))==0)
    return NULL;
#endif /* WIN32 */
  close(pspipe[READ_END]);
  close(pngpipe[WRITE_END]);
#else /* MIKTEX */
  /* No fork but miktex_start_process3: execute gs using that.
     Attach file descriptors to that process' stdin and stdout. */
  if (! miktex_find_miktex_executable("mgs.exe", szGsPath)) {
      Warning("Ghostscript could not be found");
      return(NULL);
  }
#if defined(MIKTEX)
  snprintf(szCommandLine,2048,"%s %s %s %s %s %s %s %s %s %s",/* %s",*/
	   device, resolution, /*devicesize,*/
	   "-dBATCH", "-dNOPAUSE", "-q", "-sOutputFile=-", 
	   "-dTextAlphaBits=4", "-dGraphicsAlphaBits=4",
	   (option_flags & NO_GSSAFER) ? "-": "-dSAFER", 
	   (option_flags & NO_GSSAFER) ? "": "-");
  miktex_start_process(szGsPath, szCommandLine, 0, &psstream, &pngstream, 0, 0);
#else
  snprintf(szCommandLine,2048,"\"%s\" %s %s %s %s %s %s %s %s %s %s",/* %s",*/
	   szGsPath, device, resolution, /*devicesize,*/
	   "-dBATCH", "-dNOPAUSE", "-q", "-sOutputFile=-",
	   "-dTextAlphaBits=4", "-dGraphicsAlphaBits=4",
	   (option_flags & NO_GSSAFER) ? "-": "-dSAFER",
	   (option_flags & NO_GSSAFER) ? "": "-");
  if (! miktex_start_process_3(szCommandLine, &pi, INVALID_HANDLE_VALUE,
			       &hPsStream, &hPngStream, &hStdErr, 0)) {
      Warning("Ghostscript could not be started");
      return(NULL);
  }
  CloseHandle (pi.hThread);
  pspipe[WRITE_END] = _open_osfhandle((intptr_t)hPsStream, _O_WRONLY);
  pngpipe[READ_END] = _open_osfhandle((intptr_t)hPngStream, _O_RDONLY);
#endif
#endif /* MIKTEX */
#if !defined(MIKTEX)
  if (pspipe[WRITE_END] >= 0) {
    if ((psstream=fdopen(pspipe[WRITE_END],"wb")) == NULL)
      close(pspipe[WRITE_END]);
    else {
#endif
      writepscode(psstream,psheaderp);
      /* Page size */
      DEBUG_PRINT(DEBUG_GS,("\n  PS CODE:\t<</PageSize[%d %d]/PageOffset[%d %d[1 1 dtransform exch]{0 ge{neg}if exch}forall]>>setpagedevice",
			    urx - llx, ury - lly,llx,lly));
      fprintf(psstream, "<</PageSize[%d %d]/PageOffset[%d %d[1 1 dtransform exch]{0 ge{neg}if exch}forall]>>setpagedevice\n",
	      urx - llx, ury - lly,llx,lly);
      /* Background color */
      if ( bgred < 255 || bggreen < 255 || bgblue < 255 ) {
	DEBUG_PRINT(DEBUG_GS,("\n  PS CODE:\tgsave %f %f %f setrgbcolor clippath fill grestore",
			      bgred/255.0, bggreen/255.0, bgblue/255.0));
	fprintf(psstream, "gsave %f %f %f setrgbcolor clippath fill grestore\n",
		bgred/255.0, bggreen/255.0, bgblue/255.0);
      }
      writepscode(psstream,pscodep);
      fclose(psstream);
#if !defined(MIKTEX)
    }
  }
  if (pngpipe[READ_END] >= 0) {
    if((pngstream=fdopen(pngpipe[READ_END],"rb")) == NULL)
      close(pngpipe[READ_END]);
    else {
#endif
      psimage = gdImageCreateFromPng(pngstream);
      fclose(pngstream);
#if !defined(MIKTEX)
    }
  }
#endif
#ifndef MIKTEX
#ifndef WIN32
  /* Wait for child */
  waitpid(pid,NULL,0);
#else
  /* Wait for spawned process, restore stdin and stdout */
  while(nexitcode == STILL_ACTIVE)
    GetExitCodeProcess((HANDLE)hchild, &nexitcode);
  CloseHandle((HANDLE)hchild);
  _dup2(savestdin, fileno(stdin));
  _dup2(savestdout, fileno(stdout));
  close(savestdin);
  close(savestdout);
#endif /* WIN32 */
#else /* MIKTEX */
#if !defined(MIKTEX)
  /* Close miktex process */
  CloseHandle(pi.hProcess);
#endif
#endif /* MIKTEX */
#ifdef DEBUG
  if (psimage == NULL) {
    DEBUG_PRINT(DEBUG_GS,("\n  GS OUTPUT:\tNO IMAGE "));
  } else {
    DEBUG_PRINT(DEBUG_GS,("\n  GS OUTPUT:\t%dx%d image ",
			  gdImageSX(psimage),gdImageSY(psimage)));
  }
#endif
  return psimage;
}

static gdImagePtr
rescale(gdImagePtr psimage, int pngwidth, int pngheight)
{
  gdImagePtr scaledimage=psimage;
  /* Rescale unless correct size */
  if (psimage!=NULL
      && gdImageSX(psimage)!=pngwidth
      && gdImageSY(psimage)!=pngheight) {
    DEBUG_PRINT(DEBUG_DVI,
		("\n  RESCALE INCLUDED BITMAP \t(%d,%d) -> (%d,%d)",
		 gdImageSX(psimage),gdImageSY(psimage),
		 pngwidth,pngheight));
#ifdef HAVE_GDIMAGECREATETRUECOLOR
    scaledimage=gdImageCreateTrueColor(pngwidth,pngheight);
    /* Copy with overwrite, remember that this is the rescaled source
       image. The real target has alpha blending on. */
    gdImageAlphaBlending(scaledimage,0);
    gdImageCopyResampled(scaledimage,psimage,0,0,0,0,
			 pngwidth,pngheight,
			 gdImageSX(psimage),gdImageSY(psimage));
#else
    scaledimage=gdImageCreate(pngwidth,pngheight);
    gdImageCopyResized(scaledimage,psimage,0,0,0,0,
		       pngwidth,pngheight,
		       gdImageSX(psimage),gdImageSY(psimage));
#endif
    gdImageDestroy(psimage);
  }
  return(scaledimage);
}

static void newpsheader(const char* special) {
  struct pscode* tmp;
  char* txt;

  if (psheaderp==NULL && strcmp(special,"header=tex.pro")!=0) {
    newpsheader("header=tex.pro");
    newpsheader("header=color.pro");
    newpsheader("header=special.pro");
  }
  if (strcmp(special+strlen(special)-4,".xcp")==0
      && strncmp(special,"header=",7)==0)
    InitXColorPrologue(special+7);
  if (strncmp(special,"! /pgfH",7)==0)
    newpsheader("! TeXDict begin");
  if (psheaderp==NULL) {
    if ((tmp=psheaderp=malloc(sizeof(struct pscode)))==NULL)
      Fatal("cannot malloc space for PostScript header struct");
  } else {
    /* No duplicates. This still misses pre=..., because we still
       change that. To be fixed */
    tmp=psheaderp;
    if (strcmp(tmp->special,special)==0)
      return;
    while(tmp->next!=NULL) {
      tmp=tmp->next;
      if (strcmp(tmp->special,special)==0)
	return;
    }
    if ((tmp->next=malloc(sizeof(struct pscode)))==NULL)
      Fatal("cannot malloc space for PostScript header struct");
    tmp=tmp->next;
  }
  DEBUG_PRINT(DEBUG_GS,("\n  PS HEADER "));
  if ((txt=malloc(strlen(special)+1))==NULL)
    Fatal("cannot malloc space for PostScript header");
  strcpy(txt,special);
  PSCodeInit(tmp,txt);
}

/*********************************************************************/
/****************************  SetSpecial  ***************************/
/*********************************************************************/

void SetSpecial(char *start, char *end, int32_t hh, int32_t vv)
/* interpret a \special command, made up of keyword=value pairs,
 * or !header or ps:raw_PostScript
 */
{
  char *buffer,*special;

  if ((buffer=malloc(end-start+1))==NULL)
      Fatal("Cannot allocate space for special");
  special=memcpy(buffer,start,end-start);
  special[end-start]='\0';
  DEBUG_PRINT(DEBUG_DVI,(" '%s'",special));
  SKIPSPACES(special);
  /********************** Color specials ***********************/
  if (strncmp(special,"background ",11)==0) {
    background(special+11);
    free(buffer);
    return;
  }
  if (strncmp(special,"color ",6)==0) {
    special+=6;
    SKIPSPACES(special);
    if (strncmp(special,"push ",5)==0) {
      pushcolor(special+5);
    } else {
      if (strcmp(special,"pop")==0)
	popcolor();
      else
	resetcolorstack(special);
    }
    free(buffer);
    return;
  }

  /******************* Image inclusion ********************/

  /* Needed tests for regression: PNG, GIF, JPEG and EPS inclusion,
   * for different gd versions */

  if (strncmp(special,"PSfile=",7)==0) { /* PSfile */
    char* psname = special+7;
    int llx=0,lly=0,urx=0,ury=0,rwi=0,rhi=0;
    bool clip=false;
    int hresolution,vresolution;
    int pngheight,pngwidth;

    /* Remove quotation marks around filename. If no quotation marks,
       use first word as filename */
    if (*psname=='"') {
      psname++;
      special=strrchr(psname,'"');
    } else {
      special=strchr(psname,' ');
    }
    if (special!=NULL) {
      *special='\0';
      special++;
    }

    /* Retrieve parameters */
    SKIPSPACES(special);
    while(special && *special) {
      if (strncmp(special,"llx=",4)==0)
	llx = strtol(special+4,&special,10);
      else if (strncmp(special,"lly=",4)==0)
	lly = strtol(special+4,&special,10);
      else if (strncmp(special,"urx=",4)==0)
	urx = strtol(special+4,&special,10);
      else if (strncmp(special,"ury=",4)==0)
	ury = strtol(special+4,&special,10);
      else if (strncmp(special,"rwi=",4)==0)
	rwi = strtol(special+4,&special,10);
      else if (strncmp(special,"rhi=",4)==0)
	rhi = strtol(special+4,&special,10);
      else if (strncmp(special,"clip",4)==0)
	{clip = true; special=special+4;}
      while (*special && *special!=' ') special++;
      SKIPSPACES(special);
    }

    /* Calculate resolution, and use our base resolution as a fallback. */
    /* The factor 10 is magic, the dvips graphicx driver needs this.    */
    hresolution = ((dpi*rwi+urx-llx-1)/(urx - llx)+9)/10;
    vresolution = ((dpi*rhi+ury-lly-1)/(ury - lly)+9)/10;
    if (vresolution==0) vresolution = hresolution;
    if (hresolution==0) hresolution = vresolution;
    if (hresolution==0) hresolution = vresolution = dpi;

    /* Convert from postscript 72 dpi resolution to our given resolution */
    pngwidth  = (dpi*rwi+719)/720; /* +719: round up */
    pngheight = (dpi*rhi+719)/720;
    if (pngwidth==0)
      pngwidth  = ((dpi*rhi*(urx-llx)+ury-lly-1)/(ury-lly)+719)/720;
    if (pngheight==0)
      pngheight = ((dpi*rwi*(ury-lly)+urx-llx-1)/(urx-llx)+719)/720;
    if (pngheight==0) {
      pngwidth  = (dpi*(urx-llx)+71)/72;
      pngheight = (dpi*(ury-lly)+71)/72;
    }
    if (page_imagep != NULL) { /* Draw into image */
      struct pscode image;
      gdImagePtr psimage=NULL;
#ifndef HAVE_GDIMAGECREATEFROMPNGPTR
      FILE* psstream;
#endif

      PSCodeInit(&image, NULL);
      image.filename=kpse_find_file(psname,kpse_pict_format,0);
      if (MmapFile(image.filename,&(image.fmmap)) || image.fmmap.size==0) {
	Warning("Image file %s unusable, image will be left blank",
		image.filename);
	page_flags |= PAGE_GAVE_WARN;
	free(buffer);
	return;
      }
      Message(BE_NONQUIET," <%s",psname);
      switch ((unsigned char)*image.fmmap.data) {
      case 0x89: /* PNG magic: "\211PNG\r\n\032\n" */
	DEBUG_PRINT(DEBUG_DVI,("\n  INCLUDE PNG \t%s",image.filename));
#ifdef HAVE_GDIMAGECREATEFROMPNGPTR
	psimage=gdImageCreateFromPngPtr(image.fmmap.size,image.fmmap.data);
#else
	psstream=fopen(image.filename,"rb");
	psimage=gdImageCreateFromPng(psstream);
	fclose(psstream);
#endif
	psimage=rescale(psimage,pngwidth,pngheight);
	break;
      case 'G': /* GIF magic: "GIF87" or "GIF89" */
	DEBUG_PRINT(DEBUG_DVI,("\n  INCLUDE GIF \t%s",image.filename));
#ifdef HAVE_GDIMAGEGIF
	psimage=rescale(gdImageCreateFromGifPtr(image.fmmap.size,
						image.fmmap.data),
			pngwidth,pngheight);
#else
	DEBUG_PRINT(DEBUG_DVI,(" (NO GIF DECODER)"));
#endif
	break;
      case 0xff: /* JPEG magic: 0xffd8 */
	DEBUG_PRINT(DEBUG_DVI,("\n  INCLUDE JPEG \t%s",image.filename));
#ifdef HAVE_GDIMAGECREATEFROMJPEG
#ifdef HAVE_GDIMAGECREATEFROMPNGPTR
	psimage=gdImageCreateFromJpegPtr(image.fmmap.size,image.fmmap.data);
#else
	psstream=fopen(image.filename,"rb");
	psimage=gdImageCreateFromJpeg(psstream);
	fclose(psstream);
#endif
	psimage=rescale(psimage,pngwidth,pngheight);
#else
	DEBUG_PRINT(DEBUG_DVI,(" (NO JPEG DECODER)"));
#endif
	break;
      default:  /* Default, PostScript magic: "%!PS-Adobe" */
      	if (option_flags & NO_GHOSTSCRIPT) {
	  Warning("GhostScript calls disallowed by --nogs" );
	  page_flags |= PAGE_GAVE_WARN;
	} else {
	  /* Ensure one (and only one) showpage */
	  image.code=" /DVIPNGDICT 100 dict def DVIPNGDICT begin /showpage {} def ";
	  image.postcode=" end showpage\n";
	  /* Use alpha blending, and render transparent postscript
	     images. The alpha blending works correctly only from
	     libgd 2.0.12 upwards */
#ifdef HAVE_GDIMAGEPNGEX
	  if (page_imagep->trueColor) {
	    int tllx=llx,tlly=lly,turx=urx,tury=ury;

	    DEBUG_PRINT((DEBUG_DVI | DEBUG_GS),
			("\n  GS RENDER \t%s -> pngalpha ",image.filename));
	    if (!clip) {
	      /* Render across the whole image */
	      tllx=llx-(hh+1)*72/hresolution;
	      tlly=lly-(gdImageSY(page_imagep)-vv-1)*72/vresolution;
	      turx=llx+(gdImageSX(page_imagep)-hh)*72/hresolution;
	      tury=lly+(vv+1)*72/vresolution;
	      DEBUG_PRINT((DEBUG_DVI | DEBUG_GS),
			  ("\n  EXPAND BBOX \t%d %d %d %d -> %d %d %d %d",
			   llx,lly,urx,ury,tllx,tlly,turx,tury));
#ifdef DEBUG
	    } else {
	      DEBUG_PRINT((DEBUG_DVI | DEBUG_GS),(", CLIPPED TO BBOX"));
#endif
	    }
	    psimage = ps2png(&image, "-sDEVICE=pngalpha",
			     hresolution, vresolution,
			     tllx, tlly, turx, tury,
			     255,255,255);
	    if (psimage==NULL)
	      Warning("No GhostScript pngalpha output, opaque image inclusion");
	  } else
	    Warning("Palette output, opaque image inclusion");
#endif
	  if (psimage==NULL) {
	    /* png256 gives inferior result */
	    DEBUG_PRINT((DEBUG_DVI | DEBUG_GS),
			("\n  GS RENDER \t%s -> png16m", image.filename));
	    psimage = ps2png(&image, "-sDEVICE=png16m",
			     hresolution, vresolution,
			     llx, lly, urx, ury,
			     cstack[0].red,cstack[0].green,cstack[0].blue);
	    clip=true;
	    page_flags |= PAGE_GAVE_WARN;
	  }
	  if (!clip) {
	    /* Rendering across the whole image */
	    hh=0;
	    vv=gdImageSY(psimage)-1;
	  }
	}
      }
      UnMmapFile(&(image.fmmap));
      if (psimage!=NULL) {
	DEBUG_PRINT(DEBUG_DVI,
		    ("\n  GRAPHIC(X|S) INCLUDE \t%s (%d,%d) res %dx%d at (%d,%d)",
		     psname,gdImageSX(psimage),gdImageSY(psimage),
		     hresolution,vresolution,hh,vv));
#ifdef HAVE_GDIMAGECREATETRUECOLOR
	if (psimage->trueColor && !page_imagep->trueColor)
	  gdImageTrueColorToPalette(psimage,0,256);
#endif
#ifdef HAVE_GDIMAGEPNGEX
	gdImageAlphaBlending(page_imagep,1);
#else
	Warning("Using libgd < 2.0.12, opaque image inclusion");
	page_flags |= PAGE_GAVE_WARN;
#endif
	gdImageCopy(page_imagep, psimage,
		    hh, vv-gdImageSY(psimage)+1,
		    0,0,
		    gdImageSX(psimage),gdImageSY(psimage));
#ifdef HAVE_GDIMAGEPNGEX
	gdImageAlphaBlending(page_imagep,0);
#endif
	gdImageDestroy(psimage);
      } else {
        Warning("Unable to load %s, image will be left blank",image.filename);
        page_flags |= PAGE_GAVE_WARN;
      }
      free(image.filename);
      Message(BE_NONQUIET,">");
    } else { /* Don't draw */
      page_flags |= PAGE_TRUECOLOR;
      DEBUG_PRINT(DEBUG_DVI,
		  ("\n  GRAPHIC(X|S) INCLUDE \t%s (%d,%d) res %dx%d at (%d,%d)",
		   psname,pngheight,pngwidth,
		   hresolution,vresolution,hh,vv));
      min(x_min,hh);
      min(y_min,vv-pngheight+1);
      max(x_max,hh+pngwidth);
      max(y_max,vv+1);
    }
    free(buffer);
    return;
  }

  /******************* Raw PostScript ********************/

  if (strncmp(special,"!/preview@version(",18)==0) {
    int length=0;
    special+=18;
    while (special[length]!='\0' && special[length]!=')')
      length++;
    if (page_imagep==NULL)
      Message(BE_NONQUIET," (preview-latex version %.*s)",length,special);
    free(buffer);
    return;
  }

  /* preview-latex' tightpage option */
  if (strncmp(special,"!/preview@tightpage",19)==0) {
    special+=19;
    SKIPSPACES(special);
    if (strncmp(special,"true",4)==0) {
      if (page_imagep==NULL)
	Message(BE_NONQUIET," (preview-latex tightpage option detected, will use its bounding box)");
      dvi->flags |= DVI_PREVIEW_LATEX_TIGHTPAGE;
    }
    free(buffer);
    return;
  }
  if (strncmp(special,"!userdict",9)==0
      && strstr(special+10,"7{currentfile token not{stop}if 65781.76 div")!=NULL) {
    if (page_imagep==NULL && ~dvi->flags & DVI_PREVIEW_LATEX_TIGHTPAGE)
      Message(BE_NONQUIET," (preview-latex <= 0.9.1 tightpage option detected, will use its bounding box)");
    dvi->flags |= DVI_PREVIEW_LATEX_TIGHTPAGE;
    free(buffer);
    return;
  }

  /* preview-latex' dvips bop-hook redefinition */
  if (strncmp(special,"!userdict",9)==0
      && strstr(special+10,"preview-bop-")!=NULL) {
    dvi->flags |= DVI_PREVIEW_BOP_HOOK;
    if (page_imagep==NULL)
      Message(BE_VERBOSE," (preview-latex beginning-of-page-hook detected)");
    free(buffer);
    return;
  }

  if (dvi->flags & DVI_PREVIEW_BOP_HOOK && ~page_flags & PAGE_PREVIEW_BOP
      && strncmp(special,"ps::",4)==0) {
    /* Hokay, decode bounding box */
    dviunits adj_llx,adj_lly,adj_urx,adj_ury,ht,dp,wd;
    adj_llx = strtol(special+4,&special,10);
    adj_lly = strtol(special,&special,10);
    adj_urx = strtol(special,&special,10);
    adj_ury = strtol(special,&special,10);
    ht = strtol(special,&special,10);
    dp = strtol(special,&special,10);
    wd = strtol(special,&special,10);
    page_flags |= PAGE_PREVIEW_BOP;
    if (wd>0) {
      x_offset_tightpage =
	(-adj_llx+dvi->conv*shrinkfactor-1)/dvi->conv/shrinkfactor;
      x_width_tightpage  = x_offset_tightpage
	+(wd+adj_urx+dvi->conv*shrinkfactor-1)/dvi->conv/shrinkfactor;
    } else {
      x_offset_tightpage =
	(-wd+adj_urx+dvi->conv*shrinkfactor-1)/dvi->conv/shrinkfactor;
      x_width_tightpage  = x_offset_tightpage
	+(-adj_llx+dvi->conv*shrinkfactor-1)/dvi->conv/shrinkfactor;
    }
    /* y-offset = height - 1 */
    y_offset_tightpage =
      (((ht>0)?ht:0)+adj_ury+dvi->conv*shrinkfactor-1)/dvi->conv/shrinkfactor-1;
    y_width_tightpage  = y_offset_tightpage+1
      +(((dp>0)?dp:0)-adj_lly+dvi->conv*shrinkfactor-1)/dvi->conv/shrinkfactor;
    free(buffer);
    return;
  }

  if (special[0]=='"' || strncmp(special,"ps:",3)==0) { /* Raw PostScript */
    if (option_flags & NO_RAW_PS) {
      Warning("Raw PostScript rendering disallowed by --norawps" );
      page_flags |= PAGE_GAVE_WARN;
      free(buffer);
      return;
    }
    if (page_imagep != NULL) { /* Draw into image */
      static struct pscode *pscodep=NULL;
      static bool psenvironment=false;
      bool nextisps;
      struct pscode *tmp;
      gdImagePtr psimage=NULL;
      char *txt;
      const char *specialend=special+strlen(special);
      const char *newspecial=NULL;

      /* hyperref non-rendering PostScript specials. */
      if (strcmp(specialend-11,"pdfmark end")==0
	  || strcmp(specialend-7,"H.A end")==0
	  || strcmp(specialend-7,"H.B end")==0
	  || strcmp(specialend-7,"H.L end")==0
	  || strcmp(specialend-7,"H.R end")==0
	  || strcmp(specialend-7,"H.S end")==0
	  || strcmp(specialend-7,"H.V end")==0
	  || strncmp(special,"ps:SDict begin /product",23)==0)
	if (pscodep==NULL) {
	  free(buffer);
	  return;
	} else
	  newspecial="";
      /* pgf PostScript specials. */
      else if (strcmp(special,"ps:: pgfo")==0)
	/* pgf page start. The first numbers are generally valid for
	   the bop instruction, and the latter code is to move the
	   origin to the right place. */
	newspecial="ps:: 39139632 55387786 1000 600 600 (tikzdefault.dvi) @start 1 0 bop pgfo 0 0 matrix defaultmatrix transform itransform translate";
      else if (strcmp(special,"ps:: pgfc")==0)
	newspecial="ps:: pgfc eop end";
      /* Some packages split their raw PostScript code into
	 several specials. Check for those, and concatenate them so
	 that they're given to one and the same invocation of gs */
      else if (strncmp(special,"ps::[begin]",11)==0)
	psenvironment=true;
      else if (strncmp(special,"ps::[end]",9)==0)
	psenvironment=false;
      if (pscodep==NULL) {
	Message(BE_NONQUIET," <raw PostScript");
	if ((tmp=pscodep=malloc(sizeof(struct pscode)))==NULL)
	  Fatal("cannot malloc space for raw PostScript struct");
      } else {
	tmp=pscodep;
	while(tmp->next != NULL)
	  tmp=tmp->next;
	if ((tmp->next=malloc(sizeof(struct pscode)))==NULL)
	  Fatal("cannot malloc space for raw PostScript struct");
	tmp=tmp->next;
      }
      nextisps=DVIIsNextPSSpecial(dvi);
      if (psenvironment || nextisps) {
	if (!nextisps) {
	  Warning("PostScript environment contains DVI commands");
	  page_flags |= PAGE_GAVE_WARN;
	}
	DEBUG_PRINT(DEBUG_GS,("\n  PS SPECIAL "));
	if (newspecial == NULL)
	  newspecial=special;
	if ((txt=malloc(strlen(newspecial)+1))==NULL)
	  Fatal("cannot allocate space for raw PostScript special");
	strcpy(txt,newspecial);
	PSCodeInit(tmp,txt);
	free(buffer);
	return;
      }
      DEBUG_PRINT(DEBUG_DVI,("\n  LAST PS SPECIAL "));
      if (newspecial != NULL) {
	if ((special=malloc(strlen(newspecial)+1))==NULL)
	  Fatal("cannot allocate space for raw PostScript special");
	strcpy(special,newspecial);
      }
      PSCodeInit(tmp,special);
      /* Now, render image */
      if (option_flags & NO_GHOSTSCRIPT)
	Warning("GhostScript calls disallowed by --nogs" );
      else {
	/* Use alpha blending, and render transparent postscript
	   images. The alpha blending works correctly only from
	   libgd 2.0.12 upwards */
#ifdef HAVE_GDIMAGEPNGEX
	if (page_imagep->trueColor) {
	  /* Render across the whole image */
	  psimage = ps2png(pscodep, "-sDEVICE=pngalpha",
			   dpi,dpi,
			   -(hh+1)*72/dpi,
			   -(gdImageSY(page_imagep)-vv-1)*72/dpi,
			   (gdImageSX(page_imagep)-hh)*72/dpi,
			   (vv+1)*72/dpi,
			   255,255,255);
	  if (psimage!=NULL) {
	    gdImageAlphaBlending(page_imagep,1);
	    gdImageCopy(page_imagep, psimage,
			0,0,0,0,
			gdImageSX(psimage),gdImageSY(psimage));
	    gdImageAlphaBlending(page_imagep,0);
	    gdImageDestroy(psimage);
	  } else
	    Warning("No image output from inclusion of raw PostScript");
	} else
	  Warning("Palette output, cannot include raw PostScript");
#else
	Warning("Using libgd < 2.0.12, unable to include raw PostScript");
#endif
      }
      while(pscodep->next != NULL) {
	tmp=pscodep->next;
	free(pscodep->special);
	free(pscodep);
	pscodep=tmp;
      }
      free(pscodep);
      pscodep=NULL;
      if (newspecial != NULL)
	free(special);
      if (psimage==NULL)
	page_flags |= PAGE_GAVE_WARN;
      Message(BE_NONQUIET,">");
    } else { /* Don't draw */
      page_flags |= PAGE_TRUECOLOR;
    }
    free(buffer);
    return;
  }

  if (strncmp(special,"papersize=",10)==0) { /* papersize spec, ignored */
    free(buffer);
    return;
  }

  if (special[0]=='!' || strncmp(special,"header=",7)==0) { /* PS header */
    newpsheader(special);
    free(buffer);
    return;
  }

  if (strncmp(special,"src:",4)==0) { /* source special */
    if ( page_imagep != NULL )
      Message(BE_NONQUIET," at (%ld,%ld) source \\special{%s}",
	      hh, vv, special);
    free(buffer);
    return;
  }
  if ( page_imagep != NULL ) {
    Warning("at (%ld,%ld) unimplemented \\special{%s}",
	    hh, vv, special);
    page_flags |= PAGE_GAVE_WARN;
  }
  free(buffer);
}
