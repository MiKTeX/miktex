/*
 * NAME
 *	pkin.h - header file for readchar()
 * SYNOPSIS
 *	#include "pkfile.h"
 *
 *	int readchar(name, c, cd)
 *	char name[]; shalfword c; chardesc *cd;
 * DESCRIPTION
 *	Readchar() opens the pkfile `name', checks if it's a legal pkfile,
 *	and reads the character description of `c' into `cd'. Readchar()
 *	returns TRUE (1) upon success, FALSE (0) if the character `c' is
 *	not found in the pkfile.
 *
 *	Parameter `cd' is a pointer to a the following structure:
 *	     typedef struct {
 *	   	integer charcode,	
 *	                cwidth, cheight,
 *		        xoff, yoff;
 *		halfword *raster ;
 *		} chardesc ;
 *
 *	Charcode is the code of the character returned, cwidth and cheight
 *	respectively the width and height of the character in pixels,
 *	xoff and yoff are the offsets of first pixel to the reference point
 *	of the character box.
 *	Raster is a pointer to the bitmap describing the character. This
 *	raster consists of cheight rows of (cwidth + 15) /16 words of 
 *	16 bits each. Each row contains from left to right all cwidth
 *	pixels of that row.
 * WARNING
 *	If the machine has a default integer size of 16 bits, and 32-bit
 *	integers must be manipulated with %ld, be carefull!
 * AUTHORS
 *	Piet Tutelaers
 */
 
#include <stdio.h>
#include <stdlib.h>
/*
 *   Type declarations.  integer must be a 32-bit signed; shalfword must
 *   be a sixteen-bit signed; halfword must be a sixteen-bit unsigned;
 *   quarterword must be an eight-bit unsigned.
 */
typedef long integer;
typedef short shalfword ;
typedef unsigned short halfword ;
typedef unsigned char quarterword ;
typedef short Boolean ;
/*
 *   A chardesc describes an individual character. 
 */
typedef struct {
   integer charcode,
           cwidth, cheight,
	   xoff, yoff;
   halfword *raster ;
} chardesc ;

int readchar(char *name, integer c, chardesc *cd);
