/* fontmap.c */

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

static struct filemmap psfont_mmap;
#ifdef HAVE_FT2
static struct filemmap ttfont_mmap;
#endif
static struct psfontmap *psfontmap=NULL;

static char* newword(char** buffer, char* end)
{
  char *word,*pos=*buffer;

  while(pos<end && *pos!=' ' && *pos!='\t' && *pos!='"') pos++;
  if ((word=malloc(pos-*buffer+1))==NULL)
    Fatal("cannot malloc space for string");
  strncpy(word,*buffer,pos-*buffer);
  word[pos-*buffer]='\0';
  *buffer=pos;
  return(word);
}

char* copyword(char* orig)
{
  char *word;

  if (orig==NULL)
    return(NULL);
  if ((word=malloc(strlen(orig)+1))==NULL)
    Fatal("cannot malloc space for string");
  strcpy(word,orig);
  return(word);
}

static char* find_format(const char* name)
{
  /* Cater for both new (first case) and old (second case) kpathsea */
  char* format =
    kpse_find_file(name,kpse_fontmap_format,false);

  if (format==NULL)
    format = kpse_find_file(name,kpse_dvips_config_format,false);
  return(format);
}

void InitPSFontMap(void)
{
  char* psfont_name=NULL;
  /* Prefer ps2pk.map, fonts are present more often */
  psfont_name=find_format("ps2pk.map");
  if (psfont_name==NULL)
    psfont_name=find_format("psfonts.map");
  if (psfont_name==NULL) {
    Warning("cannot find ps2pk.map, nor psfonts.map");
  } else {
    DEBUG_PRINT(DEBUG_FT,
		("\n  OPEN PSFONT MAP:\t'%s'", psfont_name));
    if (MmapFile(psfont_name,&psfont_mmap)) {
      Warning("psfonts map %s could not be opened", psfont_name);
    }
    free(psfont_name);
  }
#ifdef HAVE_FT2
  psfont_name=find_format("ttfonts.map");
  if (psfont_name!=NULL) {
    DEBUG_PRINT(DEBUG_FT,("\n  OPEN TTFONT MAP:\t'%s'", psfont_name));
    if (MmapFile(psfont_name,&ttfont_mmap)) {
      Warning("ttfonts map %s could not be opened", psfont_name);
    }
    free(psfont_name);
  }
#endif
}

struct psfontmap *NewPSFont(struct psfontmap* copyfrom)
{
  struct psfontmap *newentry=NULL;
  if ((newentry=malloc(sizeof(struct psfontmap)))==NULL)
    Fatal("cannot malloc psfontmap space");
  if (copyfrom!=NULL) {
    newentry->line = copyfrom->line;
    newentry->tfmname = copyword(copyfrom->tfmname);
    newentry->psfile = copyword(copyfrom->psfile);
    newentry->encname = copyword(copyfrom->encname);
    newentry->encoding = copyfrom->encoding;
#ifdef HAVE_FT2
    newentry->ft_transformp = copyfrom->ft_transformp;
    newentry->subfont = copyfrom->subfont;
#endif
    newentry->end = copyfrom->end;
  } else {
    newentry->line = NULL;
    newentry->tfmname = NULL;
    newentry->psfile = NULL;
    newentry->encname = NULL;
    newentry->encoding = NULL;
#ifdef HAVE_FT2
    newentry->ft_transformp = NULL;
    newentry->subfont = NULL;
#endif
    newentry->end = NULL;
  }
  newentry->next=psfontmap;
  psfontmap=newentry;
  return(newentry);
}

static struct psfontmap *SearchPSFontMap(char* fontname,
					 struct filemmap* search_mmap)
{
  static char *pos=NULL,*end=NULL;
  static struct filemmap* searching_mmap=NULL;
  struct psfontmap *entry=NULL;

  if (pos==end && search_mmap!=searching_mmap) {
    searching_mmap=search_mmap;
    pos=searching_mmap->data;
    end=searching_mmap->data+searching_mmap->size;
  }
  while(pos<end && (entry==NULL || strcmp(entry->tfmname,fontname)!=0)) {
    while(pos < end
	  && (*pos=='\r' || *pos=='\n' || *pos==' ' || *pos=='\t'
	      || *pos=='%' || *pos=='*' || *pos==';' || *pos=='#')) {
      while(pos < end && *pos!='\r' && *pos!='\n') pos++; /* skip comments/empty rows */
      pos++;
    }
    if (pos < end) {
      entry=NewPSFont(NULL);
      entry->line = pos;
      /* skip <something and quoted entries */
      while(pos < end && (*pos=='<' || *pos=='"')) {
	if (*pos=='<')
	  while(pos < end && *pos!=' ' && *pos!='\t') pos++;
	else
	  while(pos < end && *pos!='"') pos++;
	while(pos < end && (*pos==' ' || *pos=='\t')) pos++;
      }
      /* first word is font name */
      entry->tfmname = newword(&pos,end);
      while(pos < end && *pos!='\r' && *pos!='\n') pos++;
      entry->end = pos;
    }
    pos++;
  }
  if (entry!=NULL && strcmp(entry->tfmname,fontname)!=0)
    entry=NULL;
  return(entry);
}

void ClearPSFontMap(void)
{
  struct psfontmap *entry;

  while(psfontmap!=NULL) {
    entry=psfontmap;
    psfontmap=psfontmap->next;
    free(entry->tfmname);
    if (entry->psfile!=NULL)
      free(entry->psfile);
    if (entry->encname!=NULL)
      free(entry->encname);
    free(entry);
  }
  UnMmapFile(&psfont_mmap);
#ifdef HAVE_FT2
  UnMmapFile(&ttfont_mmap);
#endif
}

static void ReadPSFontMap(struct psfontmap *entry)
{
  char *pos,*end,*psname;
  int nameno = 0;

  DEBUG_PRINT(DEBUG_FT,("\n  PSFONTMAP: %s ",entry->tfmname));
  pos=entry->line;
  end=entry->end;
  while(pos < end) {
    if (*pos=='<') {                               /* filename follows */
      pos++;
      if (pos<end && *pos=='<') {           /* <<download.font */
	pos++;
	entry->psfile = newword((char**)&pos,end);
	DEBUG_PRINT(DEBUG_FT,("<%s ",entry->psfile));
      } else if (pos<end && *pos=='[') {    /* <[encoding.file */
	pos++;
	entry->encname = newword((char**)&pos,end);
	DEBUG_PRINT(DEBUG_FT,("<[%s ",entry->encname));
      } else {                                     /* <some.file      */
	char* word =newword((char**)&pos,end);
	if (strncmp(word+strlen(word)-4,".enc",4)==0) {/* <some.enc */
	  entry->encname=word;
	  DEBUG_PRINT(DEBUG_FT,("<[%s ",entry->encname));
	} else {                                   /* <font    */
	  entry->psfile=word;
	  DEBUG_PRINT(DEBUG_FT,("<%s ",entry->psfile));
	}
      }
    } else if (*pos=='"') { /* PS code: reencoding/tranformation exists */
      char *word;
      double cxx=1.0,cxy=0.0;
      pos++;
      DEBUG_PRINT(DEBUG_FT,("\""));
      while(pos < end && *pos!='"') {
	word=newword((char**)&pos,end);
	while(pos < end && (*pos==' ' || *pos=='\t')) pos++;
	if (pos+10<end && strncmp(pos,"ExtendFont",10)==0) {
	  cxx=strtod(word,NULL);
	  pos+=10;
	  DEBUG_PRINT(DEBUG_FT,("%f ExtendFont ",cxx));
	} else if (pos+9<end && strncmp(pos,"SlantFont",9)==0) {
	  pos+=9;
	  cxy=strtod(word,NULL);
	  DEBUG_PRINT(DEBUG_FT,("%f SlantFont ",cxy));
	} else if (pos+12<end && strncmp(pos,"ReEncodeFont",12)==0) {
	    pos+=12;
	    DEBUG_PRINT(DEBUG_FT,("%s ReEncodeFont ",word));
	} else {
	    DEBUG_PRINT(DEBUG_FT,("(?:%s) ",word));
	}
	free(word);
      }
#ifdef HAVE_FT2
      entry->ft_transformp=&(entry->ft_transform);
      entry->ft_transform.xx=(FT_Fixed)(cxx*0x10000);
      entry->ft_transform.xy=(FT_Fixed)(cxy*0x10000);
      entry->ft_transform.yx=0;
      entry->ft_transform.yy=0x10000;
#endif
      DEBUG_PRINT(DEBUG_FT,("\" "));
      pos++;
    } else {                                      /* bare word */
      switch (++nameno) {
      case 1: /* first word is tfmname and perhaps psname, NOT psfile */
	while(pos<end && *pos!=' ' && *pos!='\t') pos++;
	psname=entry->tfmname;
	break;
      case 2:                    /* second word is psname, NOT psfile */
	psname = newword((char**)&pos,end);
	DEBUG_PRINT(DEBUG_FT,("(%s) ",psname));
	free(psname);
	break;
      case 3:                             /* third word is encoding */
	entry->encname = newword((char**)&pos,end);
	DEBUG_PRINT(DEBUG_FT,("<[%s ",entry->encname));
	break;
      default:
	while(pos<end && *pos!=' ' && *pos!='\t') pos++;
	Warning("more than three bare words in font map entry");
      }
    }
    while(pos < end && (*pos==' ' || *pos=='\t')) pos++;
  }
  if (entry->psfile==NULL) {
    /* No psfile-name given, use tfmname */
    entry->psfile=copyword(entry->tfmname);
    DEBUG_PRINT(DEBUG_FT,(" <%s ",entry->psfile));
  }
  if (entry->encname!=NULL
      && (entry->encoding=FindEncoding(entry->encname))==NULL)
    Warning("unable to load font encoding '%s' for %s",
	    entry->encname,entry->tfmname);
}


struct psfontmap* FindPSFontMap(char* fontname)
{
  struct psfontmap *entry;
  static struct filemmap* search_mmap_p=&psfont_mmap;

  entry=psfontmap;
  while(entry!=NULL && strcmp(entry->tfmname,fontname)!=0)
    entry=entry->next;
  if(entry==NULL) {
    entry=SearchPSFontMap(fontname,search_mmap_p);
#ifdef HAVE_FT2
    if(entry==NULL && search_mmap_p!=&ttfont_mmap) {
      search_mmap_p=&ttfont_mmap;
      entry=SearchPSFontMap(fontname,search_mmap_p);
    }
  }
  if(entry==NULL) {
    struct psfontmap* entry_subfont=NULL;
    entry=psfontmap;
    while(entry!=NULL && strcmp(entry->tfmname,fontname)!=0) {
      while(entry!=NULL && strchr(entry->tfmname,'@')==NULL)
	entry=entry->next;
      if (entry!=NULL) {
	entry_subfont=FindSubFont(entry,fontname);
	if (entry_subfont!=NULL)
	  entry=entry_subfont;
	else
	  entry=entry->next;
      }
    }
#endif
  }
  if (entry!=NULL && entry->psfile==NULL)
    ReadPSFontMap(entry);
  if (entry!=NULL && entry->encname!=NULL && entry->encoding==NULL)
    /* Encoding given but it cannot be found. Unusable font */
    return(NULL);
  return(entry);
}
