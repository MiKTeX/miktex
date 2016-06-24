/* enc.c */

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

  Copyright (C) 2002-2008 Jan-Åke Larsson

************************************************************************/

#include "dvipng.h"

struct encoding* encodingp=NULL;

static struct encoding* InitEncoding(char* encoding) 
{
  char *pos,*max,*buf,*enc_file;
  int i;
  struct encoding* encp=NULL;
  struct filemmap fmmap;
  boolean mmapfailed;
  
#ifdef HAVE_KPSE_ENC_FORMATS
  enc_file=kpse_find_file(encoding,kpse_enc_format,false);
#else
  enc_file=kpse_find_file(encoding,kpse_tex_ps_header_format,false);
#endif
  if (enc_file == NULL) {
    Warning("encoding file %s could not be found",encoding);
    return(NULL);
  }
  DEBUG_PRINT((DEBUG_FT|DEBUG_ENC),("\n  OPEN ENCODING:\t'%s'", enc_file));
  mmapfailed = MmapFile(enc_file,&fmmap);
  free(enc_file);
  if (mmapfailed) 
    return(NULL);
  if ((encp = calloc(sizeof(struct encoding)+strlen(encoding)+1
		     +fmmap.size,1))==NULL) {
    Warning("cannot alloc space for encoding",encoding);
    UnMmapFile(&fmmap);
    return(NULL);
  }
  encp->name=(char*)encp+sizeof(struct encoding);
  strcpy(encp->name,encoding);
  pos=fmmap.data;
  max=fmmap.data+fmmap.size;
  buf=encp->name+strlen(encoding)+1;
#define SKIPCOMMENT(x) if (*x=='%') while (x<max && *x!='\r' && *x!='\n') x++;
  while(pos<max && *pos!='/') {
    SKIPCOMMENT(pos);
    pos++;
  }
  pos++;
  encp->charname[256]=buf;
  while(pos<max && *pos!='[' && *pos!='%'
	&& *pos!=' ' && *pos!='\t' && *pos!='\r' && *pos!='\n')
    *buf++=*pos++;
  *buf++='\0';
  DEBUG_PRINT(DEBUG_ENC,("\n  PS ENCODING '%s'",
			 encp->charname[256])); 
  while (pos < max && *pos!='[') {
    SKIPCOMMENT(pos);
    pos++;
  }
  while(pos<max && *pos!='/') {
    SKIPCOMMENT(pos);
    pos++;
  }
  i=0;
  while(pos<max && *pos!=']') {
    pos++;
    encp->charname[i++]=buf;
    while(pos<max && *pos!='%' && *pos!=' ' \
	  && *pos!='\t' && *pos!='\r' && *pos!='\n') 
      *buf++=*pos++;
    *buf++='\0';
    DEBUG_PRINT(DEBUG_ENC,("\n  PS ENCODING %d '%s'",
		 i-1,encp->charname[i-1])); 
    while(pos<max && *pos!='/' && *pos!=']') {
      SKIPCOMMENT(pos);
      pos++;
    }
  }
  UnMmapFile(&fmmap);
  return(encp);
}


struct encoding* FindEncoding(char* encoding) 
{
  struct encoding *temp=encodingp;

  /* printf("{%s} \n",encoding); */
  while(temp!=NULL && strcmp(encoding,temp->name)!=0) 
    temp=temp->next;
  if (temp==NULL) {
    temp=InitEncoding(encoding);
    if (temp!=NULL) {
      temp->next=encodingp;
      encodingp=temp;
    }
  }
  return(temp);
}

void ClearEncoding(void)
{
  struct encoding *temp=encodingp;

  while(temp!=NULL) {
    encodingp=encodingp->next;
    free(temp);
    temp=encodingp;
  }
}
