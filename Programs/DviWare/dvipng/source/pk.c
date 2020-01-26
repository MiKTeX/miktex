/* pk.c */

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

  Copyright (C) 2002-2009, 2019 Jan-Åke Larsson

************************************************************************/

#include "dvipng.h"

#define PK_POST 245
#define PK_PRE 247
#define PK_ID 89

unsigned char   dyn_f;
int             repeatcount;
int             poshalf;

static unsigned char getnyb(unsigned char** pos)
{
  if (poshalf == 0) {
    poshalf=1;
    return(**pos / 16);
  } else {
    poshalf=0;
    return(*(*pos)++ & 15);
  }
}

static uint32_t pk_packed_num(unsigned char** pos)
{
  register int    i;
  uint32_t        j;

  i = (int)getnyb(pos);
  if (i == 0) {
    do {
      j = (uint32_t)getnyb(pos);
      i++;
    } while (j == 0);
    while (i > 0) {
      j = j * 16 + (uint32_t)getnyb(pos);
      i--;
    };
    return (j - 15 + (13 - dyn_f) * 16 + dyn_f);
  } else if (i <= (int)dyn_f) {
    return ((uint32_t)i);
  } else if (i < 14) {
    return ((i-(uint32_t)dyn_f - 1) * 16 + (uint32_t)getnyb(pos)
	    + dyn_f + 1);
  } else {
    if (i == 14) {
      repeatcount = (int)pk_packed_num(pos);
    } else {
      repeatcount = 1;
    }
    return (pk_packed_num(pos));    /* tail end recursion !! */
  }
}

static unsigned char* skip_specials(unsigned char* pos, unsigned char* end)
{
  uint32_t    i;

  while (pos < end && *pos >= 240 && *pos != PK_POST) {
    i=0;
    switch (*pos++) {
    case 243:
      i = *pos++;
    case 242:
      if (pos >= end) break;
      i = 256 * i + *pos++;
    case 241:
      if (pos >= end) break;
      i = 256 * i + *pos++;
    case 240:
      if (pos >= end) break;
      i = 256 * i + *pos++;
      DEBUG_PRINT(DEBUG_PK,("\n  PK SPECIAL\t'%.*s' ",(int)i,pos));
      pos += i;
      break;
    case 244:
#ifdef DEBUG
      {
	uint32_t c;
	c=UNumRead(pos,4);
	DEBUG_PRINT(DEBUG_PK,("\n  PK SPECIAL\t%d",c));
      }
#endif
      pos += 4;
      break;
    case 245:
      break;
    case 246:
      DEBUG_PRINT(DEBUG_PK,("\n  PK\tNOP "));
      break;
    case 247: case 248: case 249: case 250:
    case 251: case 252: case 253: case 254:
    case 255:
      Fatal("unexpected PK flagbyte %d", (int)*pos);
    }
  }
  return(pos);
}


void LoadPK(int32_t c, register struct char_entry * ptr)
{
  unsigned short   shrunk_width,shrunk_height;
  unsigned short   width,height;
  short   xoffset,yoffset;
  unsigned short   i_offset,j_offset;
  int   i,j,k,n;
  int   count=0;
  bool  paint_switch;
  unsigned char *pos,*buffer;

  DEBUG_PRINT(DEBUG_PK,("\n  LOAD PK CHAR\t%d",c));
  pos=ptr->pkdata;
  if ((ptr->flag_byte & 7) == 7) n=4;
  else if ((ptr->flag_byte & 4) == 4) n=2;
  else n=1;
  dyn_f = ptr->flag_byte / 16;
  paint_switch = ((ptr->flag_byte & 8) != 0);
  /*
   *  Read character preamble
   */
  if (n != 4) {
    ptr->tfmw = UNumRead(pos, 3);
    /* +n:   vertical escapement not used */
    pos+=3+n;
  } else {
    ptr->tfmw = UNumRead(pos, 4);
    /* +4:  horizontal escapement not used */
    /* +n:   vertical escapement not used */
    pos+=8+n;
  }
  DEBUG_PRINT(DEBUG_PK,(" %d",ptr->tfmw));
  ptr->tfmw = (dviunits)
    ((int64_t) ptr->tfmw * currentfont->s / 0x100000 );
  DEBUG_PRINT(DEBUG_PK,(" (%d)",ptr->tfmw));

  width   = UNumRead(pos, n);
  height  = UNumRead(pos+=n, n);
  DEBUG_PRINT(DEBUG_PK,(" %dx%d",width,height));

  if (width > 0x7fff || height > 0x7fff)
    Fatal("character %d too large in file %s", c, currentfont->name);

  /*
   * Hotspot issues: Shrinking to the topleft corner rather than the
     hotspot will displace glyphs a fraction of a pixel. We deal with
     this in as follows: The glyph is shrunk to its hotspot by
     offsetting the bitmap somewhat to put the hotspot in the lower
     left corner of a "shrink square". Shrinking to the topleft corner
     will then act as shrinking to the hotspot. This may enlarge the
     bitmap somewhat, of course.  (Also remember that the below
     calculation of i/j_offset is in integer arithmetics.)

     There will still be a displacement from rounding the dvi
     position, but vertically it will be equal for all glyphs on a
     line, so we displace a whole line vertically by fractions of a
     pixel. This is acceptible, IMHO. Sometime there will be support
     for subpixel positioning, horizontally. Will do for now, I
     suppose.
   */
  xoffset = SNumRead(pos+=n, n);
  i_offset = ( shrinkfactor - xoffset % shrinkfactor ) % shrinkfactor;
  width += i_offset;
  ptr->xOffset = xoffset+i_offset;

  yoffset = SNumRead(pos+=n, n);
  j_offset = ( shrinkfactor - (yoffset-(shrinkfactor-1)) % shrinkfactor )
    % shrinkfactor;
  height += j_offset;
  ptr->yOffset = yoffset+j_offset;

  DEBUG_PRINT(DEBUG_PK,(" (%dx%d)",width,height));
  /*
     Extra marginal so that we do not crop the image when shrinking.
  */
  shrunk_width = (width + shrinkfactor - 1) / shrinkfactor;
  shrunk_height = (height + shrinkfactor - 1) / shrinkfactor;
  ptr->w = shrunk_width;
  ptr->h = shrunk_height;
  pos+=n;
  if ((buffer = calloc(shrunk_width*shrunk_height*
											 shrinkfactor*shrinkfactor,sizeof(char)))==NULL)
    Fatal("cannot allocate space for pk buffer");
  DEBUG_PRINT(DEBUG_GLYPH,("\nDRAW GLYPH %d\n", (int)c));
  /* Raster char */
  if (dyn_f == 14) {	/* get raster by bits */
    int bitweight = 0;
    for (j = j_offset; j < (int) height; j++) {	/* get all rows */
      for (i = i_offset; i < (int) width; i++) {    /* get one row */
	bitweight /= 2;
	if (bitweight == 0) {
	  count = *pos++;
	  bitweight = 128;
	}
	if (count & bitweight) {
	  buffer[i+j*width]=1;
#ifdef DEBUG
	  DEBUG_PRINT(DEBUG_GLYPH,("+"));
	} else {
	  DEBUG_PRINT(DEBUG_GLYPH,(" "));
#endif
	}
      }
      DEBUG_PRINT(DEBUG_GLYPH,("|\n"));
    }
  } else {		/* get packed raster */
    poshalf=0;
    repeatcount = 0;
    for(i=i_offset, j=j_offset; j<height; ) {
      count = pk_packed_num(&pos);
      while (count > 0) {
	if (i+count < width) {
	  if (paint_switch)
	    for(k=0;k<count;k++) {
	      buffer[k+i+j*width]=1;
	      DEBUG_PRINT(DEBUG_GLYPH,("*"));
	    }
#ifdef DEBUG
	  else for(k=0;k<count;k++)
	    DEBUG_PRINT(DEBUG_GLYPH,(" "));
#endif
	  i += count;
	  count = 0;
	} else {
	  if (paint_switch)
	    for(k=i;k<width;k++) {
	      buffer[k+j*width]=1;
	      DEBUG_PRINT(DEBUG_GLYPH,("#"));
	    }
#ifdef DEBUG
	  else for(k=i;k<width;k++)
	    DEBUG_PRINT(DEBUG_GLYPH,(" "));
#endif
	  DEBUG_PRINT(DEBUG_GLYPH,("|\n"));
	  j++;
	  count -= width-i;
	  /* Repeat row(s) */
	  for (;repeatcount>0; repeatcount--,j++) {
	    for (i = i_offset; i<width; i++) {
	      buffer[i+j*width]=buffer[i+(j-1)*width];
#ifdef DEBUG
	      if (buffer[i+j*width]>0) {
		DEBUG_PRINT(DEBUG_GLYPH,("="));
	      } else {
		DEBUG_PRINT(DEBUG_GLYPH,(" "));
	      }
#endif
	    }
	    DEBUG_PRINT(DEBUG_GLYPH,("|\n"));
	  }
	  i=i_offset;
	}
      }
      paint_switch = 1 - paint_switch;
    }
    if (i>i_offset)
      Fatal("wrong number of bits stored: char. %c, font %s",
	    (char)c, currentfont->name);
    if (j>height)
      Fatal("bad PK file %s, too many bits", currentfont->name);
  }
  /*
    Shrink raster while doing antialiasing. (See above. The
    single-glyph output seems better than what xdvi at 300 dpi,
    shrinkfactor 3 produces.)
  */
  if ((ptr->data = calloc(shrunk_width*shrunk_height,sizeof(char))) == NULL)
    Fatal("unable to malloc image space for char %c", (char)c);
  for (j = 0; j < (int) height; j++) {
    for (i = 0; i < (int) width; i++) {
      /* if (((i % shrinkfactor) == 0) && ((j % shrinkfactor) == 0))
	 ptr->data[i/shrinkfactor+j/shrinkfactor*shrunk_width] =
	 buffer[i+j*width];
	 else */
      ptr->data[i/shrinkfactor+j/shrinkfactor*shrunk_width] +=
	buffer[i+j*width];
    }
  }
  for (j = 0; j < shrunk_height; j++) {
    for (i = 0; i < shrunk_width; i++) {
      ptr->data[i+j*shrunk_width] = ptr->data[i+j*shrunk_width]
	*255/shrinkfactor/shrinkfactor;
      DEBUG_PRINT(DEBUG_GLYPH,("%3u ",ptr->data[i+j*shrunk_width]));
    }
    DEBUG_PRINT(DEBUG_GLYPH,("|\n"));
  }
  free(buffer);
}

void InitPK(struct font_entry * tfontp)
{
  unsigned char* position, *end;
  struct char_entry *tcharptr; /* temporary char_entry pointer  */
  uint32_t    hppp, vppp, packet_length;
  uint32_t    c;

  DEBUG_PRINT((DEBUG_DVI|DEBUG_PK),("\n  OPEN FONT:\t'%s'", tfontp->name));
  Message(BE_VERBOSE,"<%s>", tfontp->name);
  if (MmapFile(tfontp->name,&(tfontp->fmmap)))
    Fatal("font file %s unusable", tfontp->name);
  position=(unsigned char*)tfontp->fmmap.data;
  if (tfontp->fmmap.size < 3 || tfontp->fmmap.size < 3+*(position+2)+16)
    Fatal("PK file %s ends prematurely",tfontp->name);
  if (*position++ != PK_PRE)
    Fatal("unknown font format in file %s",tfontp->name);
  if (*position++ != PK_ID)
    Fatal( "wrong version %d of PK file %s (should be 89)",
	   (int)*(position-1),tfontp->name);
  DEBUG_PRINT(DEBUG_PK,("\n  PK_PRE:\t'%.*s'",(int)*position, position+1));
  position += *position + 1;

  tfontp->designsize = UNumRead(position, 4);
  DEBUG_PRINT(DEBUG_PK,(" %d", tfontp->designsize));
  tfontp->type = FONT_TYPE_PK;

  c = UNumRead(position+4, 4);
  DEBUG_PRINT(DEBUG_PK,(" %d", c));
  CheckChecksum (tfontp->c, c, tfontp->name);

  hppp = UNumRead(position+8, 4);
  vppp = UNumRead(position+12, 4);
  DEBUG_PRINT(DEBUG_PK,(" %d %d", hppp,vppp));
  if (hppp != vppp)
    Warning("aspect ratio is %d:%d (should be 1:1)", hppp, vppp);
  tfontp->magnification = (uint32_t)((uint64_t)hppp * 7227 * 5 / 65536l + 50)/100;
  position+=16;
  /* Read char definitions */
  end=(unsigned char *) tfontp->fmmap.data+tfontp->fmmap.size;
  position = skip_specials(position,end);
  while (position < end && *position != PK_POST) {
    DEBUG_PRINT(DEBUG_PK,("\n  @%ld PK CHAR:\t%d",
			  (long)((char *)position - tfontp->fmmap.data), *position));
    if ((tcharptr = malloc(sizeof(struct char_entry))) == NULL)
      Fatal("cannot allocate space for char_entry");
    tcharptr->flag_byte = *position;
    tcharptr->data = NULL;
    tcharptr->tfmw = 0;
    if ((*position & 7) == 7) {
      if (position >= end - 9) Fatal("PK file %s ends prematurely",tfontp->name);
      packet_length = UNumRead(position+1,4);
      c = UNumRead(position+5, 4);
      position += 9;
    } else if (*position & 4) {
      if (position >= end - 4) Fatal("PK file %s ends prematurely",tfontp->name);
      packet_length = (*position & 3) * 65536l + UNumRead(position+1, 2);
      c = UNumRead(position+3, 1);
      position += 4;
    } else {
      if (position >= end - 3) Fatal("PK file %s ends prematurely",tfontp->name);
      packet_length = (*position & 3) * 256 +	UNumRead(position+1, 1);
      c = UNumRead(position+2, 1);
      position += 3;
    }
  DEBUG_PRINT(DEBUG_PK,(" %d %d",packet_length,c));
  if (c > (LASTFNTCHAR))
    Fatal("PK font %s exceeds char numbering limit",tfontp->name);
  tcharptr->length = packet_length;
  tcharptr->pkdata = position;
  tfontp->chr[c]=tcharptr;
  position += packet_length;
  position = skip_specials(position, end);
  }
  if (position >= end) Fatal("PK file %s ends prematurely",tfontp->name);
}

static void UnLoadPK(struct char_entry *ptr)
{
  if (ptr->data!=NULL)
    free(ptr->data);
  ptr->data=NULL;
}

void DonePK(struct font_entry *tfontp)
{
  int c=FIRSTFNTCHAR;

  UnMmapFile(&(tfontp->fmmap));
  while(c<=LASTFNTCHAR) {
    if (tfontp->chr[c]!=NULL) {
      UnLoadPK((struct char_entry*)tfontp->chr[c]);
      free(tfontp->chr[c]);
    }
    c++;
  }
  if (tfontp->name!=NULL)
    free(tfontp->name);
  tfontp->name=NULL;
}
