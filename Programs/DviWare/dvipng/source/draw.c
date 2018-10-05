/* draw.c */

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

#ifdef DEBUG
#include <ctype.h> /* isprint */
#endif

struct stack_entry {
  dviunits    h, v, w, x, y, z; /* stack entry                           */
  subpixels   hh,vv;
} stack[STACK_SIZE+1];          /* stack + space for current pos         */
struct stack_entry* dvi_stack=stack;

#define MAXDRIFT 1
#define CHECK_MAXDRIFT(x,xx) \
  if ( xx-PIXROUND(x,dvi->conv*shrinkfactor) < -MAXDRIFT ) {            \
    DEBUG_PRINT(DEBUG_DVI,(" add 1 to"));		                \
    xx += 1;						                \
  }									\
  if ( xx-PIXROUND(x,dvi->conv*shrinkfactor) > MAXDRIFT ) {		\
    DEBUG_PRINT(DEBUG_DVI,(" sub 1 to"));				\
    xx -= 1;								\
  }									\
  if (PIXROUND(dvi_stack->h,dvi->conv*shrinkfactor) != dvi_stack->hh	\
      || PIXROUND(dvi_stack->v,dvi->conv*shrinkfactor) != dvi_stack->vv)\
    DEBUG_PRINT(DEBUG_DVI,                                              \
                (" drift (%d,%d)",					\
		 dvi_stack->hh-PIXROUND(dvi_stack->h,dvi->conv*shrinkfactor), \
		 dvi_stack->vv-PIXROUND(dvi_stack->v,dvi->conv*shrinkfactor)));

#define MoveRight(x) \
  temp=x; dvi_stack->h += temp;		                                \
  if ( currentfont==NULL						\
       || temp > currentfont->s/6 || temp < -currentfont->s/6*4 )	\
    dvi_stack->hh = PIXROUND(dvi_stack->h,dvi->conv*shrinkfactor);	\
  else									\
    dvi_stack->hh += PIXROUND( temp,dvi->conv*shrinkfactor );		\
  CHECK_MAXDRIFT(dvi_stack->h,dvi_stack->hh)

#define MoveDown(x) \
  temp=x; dvi_stack->v += temp;		                                \
  if ( currentfont==NULL						\
       || temp > currentfont->s/6*5 || temp < currentfont->s/6*(-5) )	\
    dvi_stack->vv = PIXROUND(dvi_stack->v,dvi->conv*shrinkfactor);	\
  else									\
    dvi_stack->vv += PIXROUND( temp,dvi->conv*shrinkfactor );		\
  CHECK_MAXDRIFT(dvi_stack->v,dvi_stack->vv)

#define DO_VFCONV(a) ((((struct font_entry*) parent)->type==DVI_TYPE)?a:\
  (dviunits)((int64_t) a * ((struct font_entry*) parent)->s / (1 << 20)))


dviunits SetChar(int32_t c)
{
  struct char_entry* ptr=NULL;

  if (currentfont==NULL)
    Fatal("faulty DVI, trying to set character from null font");
  if (c<0 || c>LASTFNTCHAR) {
    Warning("glyph index out of range (%d), skipping",c);
    return(0);
  }
  ptr=currentfont->chr[c];
  if (ptr==NULL) {
    Warning("unable to draw glyph %d, skipping",c);
    return(0);
  }
#ifdef DEBUG
  switch (currentfont->type) {
  case FONT_TYPE_VF: DEBUG_PRINT(DEBUG_DVI,("\n  VF CHAR:\t")); break;
  case FONT_TYPE_PK: DEBUG_PRINT(DEBUG_DVI,("\n  PK CHAR:\t")); break;
  case FONT_TYPE_FT: DEBUG_PRINT(DEBUG_DVI,("\n  FT CHAR:\t")); break;
  default: DEBUG_PRINT(DEBUG_DVI,("\n  NO CHAR:\t"))
  }
  if (debug & DEBUG_DVI && c>=0 && c<=UCHAR_MAX && isprint(c))
    DEBUG_PRINT(DEBUG_DVI,("'%c' ",c));
  DEBUG_PRINT(DEBUG_DVI,("%d at (%d,%d) tfmw %d", c,
			 dvi_stack->hh,dvi_stack->vv,ptr?ptr->tfmw:0));
#endif
  if (currentfont->type==FONT_TYPE_VF) {
    return(SetVF(ptr));
  } else {
    if (ptr->data == NULL)
      switch(currentfont->type) {
      case FONT_TYPE_PK:	LoadPK(c, ptr); break;
#ifdef HAVE_FT2
      case FONT_TYPE_FT:	LoadFT(c, ptr); break;
#endif
      default:
	Fatal("undefined fonttype %d",currentfont->type);
      }
    if (page_imagep != NULL)
      return(SetGlyph(ptr, dvi_stack->hh, dvi_stack->vv));
    else {
      /* Expand bounding box if necessary */
      min(x_min,dvi_stack->hh - ptr->xOffset/shrinkfactor);
      min(y_min,dvi_stack->vv - ptr->yOffset/shrinkfactor);
      max(x_max,dvi_stack->hh - ptr->xOffset/shrinkfactor + ptr->w);
      max(y_max,dvi_stack->vv - ptr->yOffset/shrinkfactor + ptr->h);
      return(ptr->tfmw);
    }
  }
  return(0);
}

void DrawCommand(unsigned char* command, void* parent /* dvi/vf */)
     /* To be used both in plain DVI drawing and VF drawing */
{
  dviunits temp;

  if (/*command >= SETC_000 &&*/ *command <= SETC_127) {
    temp = SetChar((int32_t)*command);
    dvi_stack->h += temp;
    dvi_stack->hh += PIXROUND(temp,dvi->conv*shrinkfactor);
    CHECK_MAXDRIFT(dvi_stack->h,dvi_stack->hh);
  } else if (*command >= FONT_00 && *command <= FONT_63) {
    SetFntNum((int32_t)*command - FONT_00,parent);
  } else switch (*command)  {
  case PUT1: case PUT2: case PUT3: case PUT4:
    DEBUG_PRINT(DEBUG_DVI,(" %d",
		 UNumRead(command+1, dvi_commandlength[*command]-1)));
    (void) SetChar(UNumRead(command+1, dvi_commandlength[*command]-1));
    break;
  case SET1: case SET2: case SET3: case SET4:
    DEBUG_PRINT(DEBUG_DVI,(" %d",
		 UNumRead(command+1, dvi_commandlength[*command]-1)));
    {
      temp = SetChar(UNumRead(command+1, dvi_commandlength[*command]-1));
      dvi_stack->h += temp;
      dvi_stack->hh += PIXROUND(temp,dvi->conv*shrinkfactor);
      CHECK_MAXDRIFT(dvi_stack->h,dvi_stack->hh);
    }
    break;
  case SET_RULE:
    DEBUG_PRINT(DEBUG_DVI,(" %d %d",
		 UNumRead(command+1, 4), UNumRead(command+5, 4)));
    temp = SetRule(DO_VFCONV(UNumRead(command+1, 4)),
		   DO_VFCONV(UNumRead(command+5, 4)),
		   dvi_stack->hh, dvi_stack->vv);
    dvi_stack->h += temp;
    dvi_stack->hh += PIXROUND(temp,dvi->conv*shrinkfactor);
    CHECK_MAXDRIFT(dvi_stack->h,dvi_stack->hh);
    break;
  case PUT_RULE:
    DEBUG_PRINT(DEBUG_DVI,(" %d %d",
		 UNumRead(command+1, 4), UNumRead(command+5, 4)));
    (void) SetRule(DO_VFCONV(UNumRead(command+1, 4)),
		   DO_VFCONV(UNumRead(command+5, 4)),
		   dvi_stack->hh, dvi_stack->vv);
    break;
  case BOP:
    Fatal("BOP occurs within page");
    break;
  case EOP:
    break;
  case PUSH:
    /* is next item on stack? */
    if (dvi_stack == &stack[STACK_SIZE-1])
      Fatal("DVI stack overflow");
    {
      struct stack_entry *next=dvi_stack+1;
      next->h = dvi_stack->h;
      next->v = dvi_stack->v;
      next->w = dvi_stack->w;
      next->x = dvi_stack->x;
      next->y = dvi_stack->y;
      next->z = dvi_stack->z;
      next->hh = dvi_stack->hh;
      next->vv = dvi_stack->vv;
      dvi_stack=next;
    }
    break;
  case POP:
    if (dvi_stack == stack)
      Fatal("DVI stack underflow");
    dvi_stack--;
    break;
  case RIGHT1: case RIGHT2: case RIGHT3: case RIGHT4:
    DEBUG_PRINT(DEBUG_DVI,(" %d",
		 SNumRead(command+1, dvi_commandlength[*command]-1)));
    MoveRight(DO_VFCONV(SNumRead(command+1, dvi_commandlength[*command]-1)));
    break;
  case W1: case W2: case W3: case W4:
    dvi_stack->w = SNumRead(command+1, dvi_commandlength[*command]-1);
    DEBUG_PRINT(DEBUG_DVI,(" %d",dvi_stack->w));
  case W0:
    MoveRight(DO_VFCONV(dvi_stack->w));
    break;
  case X1: case X2: case X3: case X4:
    dvi_stack->x = SNumRead(command+1, dvi_commandlength[*command]-1);
    DEBUG_PRINT(DEBUG_DVI,(" %d",dvi_stack->x));
  case X0:
    MoveRight(DO_VFCONV(dvi_stack->x));
    break;
  case DOWN1: case DOWN2: case DOWN3: case DOWN4:
    DEBUG_PRINT(DEBUG_DVI,(" %d",
		 SNumRead(command+1, dvi_commandlength[*command]-1)));
    MoveDown(DO_VFCONV(SNumRead(command+1, dvi_commandlength[*command]-1)));
    break;
  case Y1: case Y2: case Y3: case Y4:
    dvi_stack->y = SNumRead(command+1, dvi_commandlength[*command]-1);
    DEBUG_PRINT(DEBUG_DVI,(" %d",dvi_stack->y));
  case Y0:
    MoveDown(DO_VFCONV(dvi_stack->y));
    break;
  case Z1: case Z2: case Z3: case Z4:
    dvi_stack->z = SNumRead(command+1, dvi_commandlength[*command]-1);
    DEBUG_PRINT(DEBUG_DVI,(" %d",dvi_stack->z));
  case Z0:
    MoveDown(DO_VFCONV(dvi_stack->z));
    break;
  case FNT1: case FNT2: case FNT3: case FNT4:
    DEBUG_PRINT(DEBUG_DVI,(" %d",
		 UNumRead(command+1, dvi_commandlength[*command]-1)));
    SetFntNum(UNumRead(command+1, dvi_commandlength[*command]-1),parent);
    break;
  case XXX1: case XXX2: case XXX3: case XXX4:
    DEBUG_PRINT(DEBUG_DVI,(" %d",
		 UNumRead(command+1, dvi_commandlength[*command]-1)));
    SetSpecial((char*)command + dvi_commandlength[*command],
	       (char*)command + dvi_commandlength[*command]
	       +UNumRead(command+1, dvi_commandlength[*command]-1),
	       dvi_stack->hh,dvi_stack->vv);
    break;
  case FNT_DEF1: case FNT_DEF2: case FNT_DEF3: case FNT_DEF4:
    if (((struct font_entry*)parent)->type==DVI_TYPE) {
      FontDef(command, parent);
    } else {
      Fatal("%s within VF macro from %s",dvi_commands[*command],
	    ((struct font_entry*)parent)->n);
    }
    break;
  case PRE: case POST: case POST_POST:
    Fatal("%s occurs within page",dvi_commands[*command]);
    break;
  case NOP:
    break;
  default:
    Fatal("%s is an undefined command",dvi_commands[*command]);
    break;
  }
}

void BeginVFMacro(struct font_entry* currentvf)
{
  struct stack_entry *next=dvi_stack+1;
  if (dvi_stack == &stack[STACK_SIZE-1])
    Fatal("DVI stack overflow");
  next->h = dvi_stack->h;
  next->v = dvi_stack->v;
  next->w = next->x = next->y = next->z = 0;
  next->hh = dvi_stack->hh;
  next->vv = dvi_stack->vv;
  dvi_stack = next;
  DEBUG_PRINT(DEBUG_DVI,("\n  START VF:\tPUSH, W = X = Y = Z = 0"));
  SetFntNum(currentvf->defaultfont,currentvf);
}

void EndVFMacro(void)
{
  if (dvi_stack == stack)
    Fatal("DVI stack underflow");
  dvi_stack--;
  DEBUG_PRINT(DEBUG_DVI,("\n  END VF:\tPOP                                  "));
}


static void DrawPage(dviunits hoffset, dviunits voffset)
     /* To be used after having read BOP and will exit cleanly when
      * encountering EOP.
      */
{
  unsigned char*  command;  /* current command                  */

  dvi_stack->h = hoffset;
  dvi_stack->v = voffset;
  dvi_stack->w = dvi_stack->x = dvi_stack->y = dvi_stack->z = 0;
  dvi_stack->hh = PIXROUND( dvi_stack->h , dvi->conv*shrinkfactor );
  dvi_stack->vv = PIXROUND( dvi_stack->v , dvi->conv*shrinkfactor );
  currentfont = NULL;    /* No default font                  */

  command=DVIGetCommand(dvi);
  DEBUG_PRINT(DEBUG_DVI,("DRAW CMD:\t%s", dvi_commands[*command]));
  while (*command != EOP)  {
    DrawCommand(command,dvi);
    command=DVIGetCommand(dvi);
    DEBUG_PRINT(DEBUG_DVI,("DRAW CMD:\t%s", dvi_commands[*command]));
  }
}

void DrawPages(void)
{
  struct page_list *dvi_pos;
  pixels x_width,y_width,x_offset,y_offset;
  int pagecounter=(option_flags & DVI_PAGENUM)?0:10;

  dvi_pos=NextPPage(dvi,NULL);
  if (dvi_pos!=NULL) {
    while(dvi_pos!=NULL) {
      SeekPage(dvi,dvi_pos);
      Message(BE_NONQUIET,"[%d", dvi_pos->count[pagecounter]);
      if (dvi_pos->count[pagecounter]!=dvi_pos->count[0])
	Message(BE_NONQUIET," (%d)", dvi_pos->count[0]);
      x_max = y_max = INT32_MIN;
      x_min = y_min = INT32_MAX;
      DrawPage((dviunits)0,(dviunits)0);
      /* Store background color. Background color of a page is given
	 by the color at EOP rather than the color at BOP. */
      StoreBackgroundColor(dvi_pos);
      /* Store pagesize */
      if (dvi->flags & DVI_PREVIEW_LATEX_TIGHTPAGE) {
	x_width_def=x_width_tightpage;
	y_width_def=y_width_tightpage;
	x_offset_def=x_offset_tightpage;
	y_offset_def=y_offset_tightpage;
      }
      if (x_width_def >= 0) { /* extend BBOX */
	min(x_min,-x_offset_def);
	max(x_max,x_min + x_width_def);
	min(y_min,-y_offset_def);
	max(y_max,y_min + y_width_def);
      }
      if (x_width_def <= 0 || option_flags & EXPAND_BBOX) {
	x_width = x_max-x_min;
	y_width = y_max-y_min;
	x_offset = -x_min; /* offset by moving topleft corner */
	y_offset = -y_min; /* offset by moving topleft corner */
      } else {
	x_width=x_width_def;
	y_width=y_width_def;
	x_offset=x_offset_def;
	y_offset=y_offset_def;
      }
      DEBUG_PRINT(DEBUG_DVI,("\n  IMAGE:\t%dx%d",x_width,y_width));
      SeekPage(dvi,dvi_pos);
      CreateImage(x_width,y_width);
#ifdef DEBUG
      DEBUG_PRINT(DEBUG_DVI,("\n@%d PAGE START:\tBOP",dvi_pos->offset));
      {
	int i;
	for (i=0;i<10;i++)
	  DEBUG_PRINT(DEBUG_DVI,(" %d",dvi_pos->count[i]));
	DEBUG_PRINT(DEBUG_DVI,(" (%d)\n",dvi_pos->count[10]));
      }
#endif
      Message(REPORT_DEPTH," depth=%d", y_width-y_offset-1);
      Message(REPORT_HEIGHT," height=%d", y_offset+1);
      Message(REPORT_WIDTH," width=%d", x_width);
      page_flags &= ~PAGE_PREVIEW_BOP;
      DrawPage(x_offset*dvi->conv*shrinkfactor,
	       y_offset*dvi->conv*shrinkfactor);
      if ( ! (option_flags & MODE_PICKY && page_flags & PAGE_GAVE_WARN )) {
	WriteImage(dvi->outname,dvi_pos->count[pagecounter]);
#ifdef TIMING
	++ndone;
#endif
      } else {
	exitcode=EXIT_FAILURE;
	Message(BE_NONQUIET,"(page not rendered)");
	DestroyImage();
      }
      Message(BE_NONQUIET,"] ");
      fflush(stdout);
      page_flags = 0;
      dvi_pos=NextPPage(dvi,dvi_pos);
    }
    Message(BE_NONQUIET,"\n");
    ClearPpList();
  }
}
