/* vf.c */

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

#define VF_ID 202
#define LONG_CHAR 242

int32_t SetVF(struct char_entry* ptr)
{
  struct font_entry* currentvf;
  unsigned char *command,*end;

  currentvf=currentfont;
  BeginVFMacro(currentvf);
  command = ptr->data;
  end = command + ptr->length;
  while (command < end)  {
    DEBUG_PRINT(DEBUG_DVI,("\n  VF MACRO:\t%s ", dvi_commands[*command]));
    DrawCommand(command,currentvf);
    command += CommandLength(command);
  }
  EndVFMacro();
  currentfont=currentvf;
  return(ptr->tfmw);
}



void InitVF(struct font_entry * tfontp)
{
  unsigned char* position;
  int length;
  struct char_entry *tcharptr;
  uint32_t c=0;
  struct font_num *tfontnump;  /* temporary font_num pointer   */

  DEBUG_PRINT((DEBUG_DVI|DEBUG_VF),("\n  OPEN FONT:\t'%s'", tfontp->name));
  Message(BE_VERBOSE,"<%s>", tfontp->name);
  if (MmapFile(tfontp->name,&(tfontp->fmmap)))
    Fatal("font file %s unusable", tfontp->name);
  position=(unsigned char*)tfontp->fmmap.data;
  if (*(position) != PRE)
    Fatal("unknown font format in file %s",tfontp->name);
  if (*(position+1) != VF_ID)
      Fatal( "wrong version %d of vf file %s (should be 202)",
	     (int)*(position+1),tfontp->name);
  DEBUG_PRINT(DEBUG_VF,("\n  VF_PRE:\t'%.*s'",
		(int)*(position+2), position+3));
  position = position+3 + *(position+2);
  c=UNumRead(position, 4);
  DEBUG_PRINT(DEBUG_VF,(" %d", c));
  CheckChecksum (tfontp->c, c, tfontp->name);
  tfontp->designsize = UNumRead(position+4,4);
  DEBUG_PRINT(DEBUG_VF,(" %d", tfontp->designsize));
  tfontp->type = FONT_TYPE_VF;
  tfontp->vffontnump=NULL;
  /* Read font definitions */
  position += 8;
  while(*position >= FNT_DEF1 && *position <= FNT_DEF4) {
    DEBUG_PRINT(DEBUG_VF,("\n  @%ld VF:\t%s",
			  (long)position - (long)tfontp->fmmap.data,
			  dvi_commands[*position]));
    FontDef(position,tfontp);
    length = dvi_commandlength[*position];
    position += length + *(position + length-1) + *(position+length-2);
  }
  /* Default font is the first defined */
  tfontnump = tfontp->vffontnump;
  while (tfontnump->next != NULL) {
    tfontnump = tfontnump->next;
  }
  tfontp->defaultfont=tfontnump->k;
  /* Read char definitions */
  while(*position < FNT_DEF1) {
    DEBUG_PRINT(DEBUG_VF,("\n@%ld VF CHAR:\t",
			  (long)position - (long)tfontp->fmmap.data));
    if ((tcharptr=malloc(sizeof(struct char_entry)))==NULL)
      Fatal("cannot allocate memory for VF char entry");
    switch (*position) {
    case LONG_CHAR:
      tcharptr->length = UNumRead(position+1,4);
      c = UNumRead(position+5,4);
      tcharptr->tfmw = UNumRead(position+9,4);
      position += 13;
      break;
    default:
      tcharptr->length = UNumRead(position,1);
      c = UNumRead(position+1,1);
      tcharptr->tfmw = UNumRead(position+2,3);
      position += 5;
    }
    DEBUG_PRINT(DEBUG_VF,("%d %d %d",tcharptr->length,c,tcharptr->tfmw));
    tcharptr->tfmw = (int32_t)
      ((int64_t) tcharptr->tfmw * tfontp->s / (1 << 20));
    DEBUG_PRINT(DEBUG_VF,(" (%d)",tcharptr->tfmw));
    if (c >= NFNTCHARS) /* Only positive for now */
      Fatal("VF font %s exceeds char numbering limit",tfontp->name);
    tfontp->chr[c] = tcharptr;
    tcharptr->data=position;
    position += tcharptr->length;
  }
}


void DoneVF(struct font_entry *tfontp)
{
  int c=FIRSTFNTCHAR;

  UnMmapFile(&(tfontp->fmmap));
  while(c<=LASTFNTCHAR) {
    if (tfontp->chr[c]!=NULL) {
      free(tfontp->chr[c]);
      tfontp->chr[c]=NULL;
    }
    c++;
  }
  FreeFontNumP(tfontp->vffontnump);
  tfontp->vffontnump=NULL;
  if (tfontp->name!=NULL)
    free(tfontp->name);
  tfontp->name=NULL;
}
