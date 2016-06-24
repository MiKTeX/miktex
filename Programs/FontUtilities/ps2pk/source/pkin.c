/*
 * NAME
 *	pkin.c - implementation of readchar()
 * DESCRIPTION
 *	This implementation of readchar() uses parts of the program dvips
 *	written by Tomas Rokicki--the inventor of the pkformat--(loadfont.c,
 *	download.c and unpack.c). Dvips in turn is derived from pktype. 
 *	Pktype(TeX) is described in debt in ``The PKtype processor'', 
 *	which is available as pktype.weave as part of the METAFONTware.
 *	What was needed to implement readchar() is rearranged in pkfile.c to 
 *	get more modularity in the style of MODULA2.
 * REDESIGN
 *	Piet Tutelaers
 */

#ifdef HAVE_CONFIG_H
#include "c-auto.h"
#endif

#ifdef DEBUG
#include <stdio.h>
#endif

#include <stdlib.h>	/* malloc() */
#include "basics.h"	/* basic definitions */
#include "pkin.h"

/*
 *   Now we have some routines to get stuff from the pk file.  pkbyte returns
 *   the next byte from the pk file.
 */

static FILE *pkfile ;

static shalfword
pkbyte(void)
{
   register shalfword i ;

   if ((i=getc(pkfile))==EOF)
      fatal("unexpected eof in pk file\n") ;
   return(i) ;
}

static shalfword
pksbyte(void)
{
   register shalfword i ;

   i = pkbyte() ;
   return i > 127 ? i -256 : i ;
}

static integer
pkquad(void)
{
   register integer i ;

   i = pksbyte() ;
   i = i * 256 + pkbyte() ;
   i = i * 256 + pkbyte() ;
   i = i * 256 + pkbyte() ;
   return(i) ;
}

static integer
pktrio(void)
{
   register integer i ;

   i = pkbyte() ;
   i = i * 256 + pkbyte() ;
   i = i * 256 + pkbyte() ;
   return(i) ;
}

static integer
pkpair(void)
{
  register integer i ;
  
  i = pkbyte() ;
  i = i * 256 + pkbyte() ;
  return (i);
}

static integer
pkspair(void)
{
  register integer i ;
  
  i = pksbyte() ;
  i = i * 256 + pkbyte();
  return (i);
}

/*
 *   pkopen opens the pk file.  This is system dependent.
 */

static Boolean	
pkopen(const char *name)
{
   if ((pkfile=fopen(name, RB))==NULL) {
      fatal("Could not open %s\n", name) ;
   } else
      return(1) ;
}

/*
 *   The next part is devoted to unpacking the character data.
 */

/*
 *   We need procedures to get a nybble, bit, and packed word from the
 *   packed data structure.
 */

static halfword inputbyte, flagbyte ; 
static halfword bitweight ; 
static halfword dynf ;
static halfword repeatcount ;

static shalfword
getnyb (void)
{   halfword temp;
    if ( bitweight == 0 ) 
    { bitweight = 16 ; 
      inputbyte = pkbyte();
      temp = inputbyte >> 4 ;
    } else {
      bitweight = 0 ;
      temp = inputbyte & 15 ;
    }
    return(temp);
} 

static Boolean
getbit (void)
{
    bitweight >>= 1 ; 
    if ( bitweight == 0 ) 
    { inputbyte = pkbyte();
      bitweight = 128 ;
    } 
    return(inputbyte & bitweight) ;
}

static halfword (*realfunc)() ;
long PKremainder ;
static halfword handlehuge(halfword i, halfword k) ;

static halfword
pkpackednum (void)
{
register halfword i, j ; 
    i = getnyb () ; 
    if ( i == 0 ) 
    { do { j = getnyb () ; 
        i++ ; 
        } while ( ! ( j != 0 ) ) ; 
      if ( i > 3 ) {
/*
 *   Damn, we got a huge count!  We *fake* it by giving an artificially
 *   large repeat count.
 */
         return( handlehuge ( i , j ) ) ;
      } else {
         while ( i > 0 ) 
           { j = j * 16 + getnyb () ; 
             i-- ; 
             } 
           return ( j - 15 + ( 13 - dynf ) * 16 + dynf ) ; 
         } 
      }
    else if ( i <= dynf ) return ( i ) ; 
    else if ( i < 14 ) return ( ( i - dynf - 1 ) * 16 + getnyb () + dynf + 1 
    ) ; 
    else 
    { if ( i == 14 ) repeatcount = pkpackednum () ; 
      else repeatcount = 1 ; 
#ifdef DEBUG
      printf("[%d]", repeatcount);
#endif
      return ( (*realfunc)() ) ;
      } 
    } 

static halfword rest (void)
{
   halfword i ;

   if (PKremainder < 0) {
      PKremainder = - PKremainder ;
      return ( 0 ) ;
   } else if (PKremainder > 0) {
      if (PKremainder > 4000) {
         PKremainder = 4000 - PKremainder ;
         return ( 4000 ) ;
      } else {
         i = PKremainder ;
         PKremainder = 0 ;
         realfunc = pkpackednum ;
         return ( i ) ;
      }
   } else {
      fatal("shouldn't happen\n") ;
   }

   /*NOTREACHED*/
   return 0;
}

static halfword
handlehuge (halfword i, halfword k)
{
   register long j = k ;

   while (i) {
      j = (j << 4L) + getnyb() ;
      i-- ;
   }
   PKremainder = j - 15 + ( 13 - dynf ) * 16 + dynf ;
   realfunc = rest ;
   return ( rest() ) ;
}

/*
 *   And now we have our unpacking routine.
 */

static halfword gpower[17] = { 0 , 1 , 3 , 7 , 15 , 31 , 63 , 127 ,
     255 , 511 , 1023 , 2047 , 4095 , 8191 , 16383 , 32767 , 65535 } ; 

static void
unpack(chardesc *cd)
{ 
  register integer i, j ; 
  register halfword word, wordweight ;
  halfword *raster;
  shalfword rowsleft ; 
  Boolean turnon ; 
  shalfword hbit ; 
  halfword count ; 
  shalfword  wordwidth ;

      wordwidth = (cd->cwidth + 15) / 16 ;
      i = 2 * cd->cheight * (long)wordwidth ;
      if (i <= 0)
         i = 2 ;
      cd->raster = (halfword *)malloc((unsigned)i) ;
      if (cd->raster == NULL)
         fatal("out of memory during allocation\n") ;
      raster = cd->raster;
      realfunc = pkpackednum ;
      dynf = flagbyte / 16 ; 
      turnon = flagbyte & 8 ; 
      if ( dynf == 14 ) 
      { bitweight = 0 ; 
        for ( i = 1 ; i <= cd->cheight ; i ++ ) 
          { word = 0 ; 
            wordweight = 32768 ; 
            for ( j = 1 ; j <= cd->cwidth ; j ++ ) 
              { if ( getbit () ) word += wordweight ; 
                wordweight >>= 1 ;
                if ( wordweight == 0 ) 
                { *raster++ = word ; 
                  word = 0 ;
                  wordweight = 32768 ; 
                  } 
                } 
              if ( wordweight != 32768 ) 
                 *raster++ = word ; 
            } 
      } else {
        rowsleft = cd->cheight ; 
        hbit = cd->cwidth ; 
        repeatcount = 0 ; 
        wordweight = 16 ; 
        word = 0 ; 
        bitweight = 0 ;
        while ( rowsleft > 0 ) 
          { count = (*realfunc)() ; 
#ifdef DEBUG
            if (turnon) printf("(%d) ",count);
            else printf("%d ",count);
#endif
            while ( count != 0 ) 
              { if ( ( count < wordweight ) && ( count < hbit ) ) 
                { if ( turnon ) word += gpower [ wordweight ] - gpower 
                  [ wordweight - count ] ; 
                  hbit -= count ; 
                  wordweight -= count ; 
                  count = 0 ; 
                  } 
                else if ( ( count >= hbit ) && ( hbit <= wordweight ) ) 
                { if ( turnon )
                     word += gpower [ wordweight ] - gpower 
                  [ wordweight - hbit ] ; 
                  *raster++ = word ; 
                  for ( i = 1 ; i <= repeatcount ; i ++ ) {
                    for ( j = 1 ; j <= wordwidth ; j ++ ) {
                      *raster = *(raster - wordwidth) ;
                      raster++ ;
                    }
                  }
                  rowsleft -= repeatcount + 1 ; 
                  repeatcount = 0 ; 
                  word = 0 ; 
                  wordweight = 16 ; 
                  count -= hbit ; 
                  hbit = cd->cwidth ; 
                  } 
                else 
                { if ( turnon ) word += gpower [ wordweight ] ; 
                  *raster++ = word ;
                  word = 0 ; 
                  count -= wordweight ; 
                  hbit -= wordweight ; 
                  wordweight = 16 ; 
                  } 
                } 
              turnon = ! turnon ; 
	  }
          putchar('\n') ;
          if ( ( rowsleft != 0 ) || ( hbit != cd->cwidth ) ) 
              fatal( "error while unpacking; more bits than required\n" ) ; 
        } 
}

/*
 *   readchar(): the main routine
 *   Reads the character definition of character `c' into `cd' if available,
 *   return FALSE (0) otherwise.
 */
int
readchar(char *name, integer c, chardesc *cd)
{
   register shalfword i ;
   register integer k ;
   register integer length = 0;

   if (!pkopen(name)) return(0);
/*
 *   Check the preamble of the pkfile
 */
   if (pkbyte()!=247)
      fatal("bad pk file, expected pre\n") ;
   if (pkbyte()!=89)
      fatal("bad version of pk file\n") ;
   for(i=pkbyte(); i>0; i--)	/* creator of pkfile */
      (void)pkbyte() ; 		
   (void) pkquad() ;  /* design size */
   (void) pkquad() ;  /* checksum    */
   (void) pkquad() ;  /* hppp        */
   (void) pkquad() ;  /* vppp	     */
/*
 *   Now we skip to the desired character definition
 */
   while ((flagbyte=pkbyte())!=245) {
      if (flagbyte < 240) {
         switch (flagbyte & 7) {
case 0: case 1: case 2: case 3:
            length = (flagbyte & 7) * 256 + pkbyte() - 4 ;
            cd->charcode = pkbyte() ;
            (void) pktrio() ;		/* TFMwidth */
            (void) pkbyte() ; 		/* pixel width */
            break ;
case 4: case 5: case 6:
            length = (flagbyte & 3) * 65536 + pkpair() * 256 - 5 ;
            cd->charcode = pkbyte() ;
            (void) pktrio() ;		/* TFMwidth */
            (void) pkpair() ;           /* pixelwidth */
            break ;
case 7:
            length = pkquad() - 12 ;
            cd->charcode = pkquad() ;
            (void) pkquad() ;		/* TFMwidth */
	    (void) pkquad();		/* pixelwidth */
	    (void) pkquad();		/* pixelheight */
         }
         if (length <= 0)
            fatal("packet length (%d) too small\n", length) ;
         if (cd->charcode == c) {
            if (flagbyte & 4) {
               if ((flagbyte & 7) == 7) { /* long format */
                  cd->cwidth = pkquad() ;
                  cd->cheight = pkquad() ;
                  cd->xoff = pkquad() ;
                  cd->yoff = pkquad() ;
               } else { /* extended format */
                  cd->cwidth = pkpair(); ;
                  cd->cheight = pkpair() ;
                  cd->xoff = pkspair() ;
                  cd->yoff = pkspair() ;
               }
            } else { /* short format */
               cd->cwidth = pkbyte() ;
               cd->cheight = pkbyte() ;
               cd->xoff = pksbyte() ;
               cd->yoff = pksbyte() ;
            }
            unpack(cd);
            (void)fclose(pkfile) ;
            return(1);
         }
         for (; length>0; length--) /* skip this character */
            (void) pkbyte() ;
      } else {
         k = 0 ;
         switch (flagbyte) {
case 243:
            k = pksbyte() ;
case 242:
            k = k * 256 + pkbyte() ;
case 241:
            k = k * 256 + pkbyte() ;
case 240:
            k = k * 256 + pkbyte() ;
            while (k-- > 0)
               i = pkbyte() ;
            break ;
case 244:
            k = pkquad() ;
            break ;
case 246:
            break ;
default:
            fatal("unexpected command (%d)\n", flagbyte) ;
         }
      }
   }
   (void)fclose(pkfile) ;
   return(0); /* character not found */
}
