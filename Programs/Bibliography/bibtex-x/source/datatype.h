/*-
******************************************************************************
******************************************************************************
**
**  MODULE
**
**      file: datatype.h
**
**  DESCRIPTION
**
**      A 32-bit implementation of BibTeX v0.99c for MS-DOS, OS/2 2.x, 
**      Unix and VMS.  This C language implementation is based on the 
**      original WEB source but it has been enhanced to support 8-bit input
**      characters and a very large processing capacity.
**
**      For documentation describing how to use and build this program, 
**      see the 00README.TXT file that accompanies this distribution.
**
**  MODULE CONTENTS
**
**      This module defines the data types used in the BibTeX WEB source.  C
**      isn't as flexible as PASCAL when defining data types, so some of the
**      types defined are fairly abitrary.
**
**      There are particular problems when defining data types for variables
**      that will be used for indexing arrays.  Whilst the "short" data
**      type will often suffice, "unsigned short" has to be used to index
**      the string pool, which has 65,000 elements.  Care must be taken when 
**      mixing signed and unsigned values, especially if the signed value
**      can be negative.
**
**  AUTHORS
**
**      Original WEB translation to C, conversion to "big" (32-bit) capacity,
**      addition of run-time selectable capacity and 8-bit support extensions
**      by:
**
**          Niel Kempson
**          Snowy Owl Systems Limited, Cheltenham, England
**          E-mail: kempson@snowyowl.co.uk
**      
**      8-bit support extensions also by:
**
**          Alejandro Aguilar-Sierra
**          Centro de Ciencias de la Atm\'osfera, 
**          Universidad Nacional Aut\'onoma de M\'exico, M\'exico
**          E-mail: asierra@servidor.unam.mx
**
**  COPYRIGHT
**
**      This implementation copyright (c) 1991-1995 by Niel Kempson
**           and copyright (c) 1995 by Alejandro Aguilar-Sierra.
**
**      This program is free software; you can redistribute it and/or
**      modify it under the terms of the GNU General Public License as
**      published by the Free Software Foundation; either version 1, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**      General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program; if not, write to the Free Software
**      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**      In other words, you are welcome to use, share and improve this
**      program.  You are forbidden to forbid anyone else to use, share
**      and improve what you give them.  Help stamp out software-hoarding!
**
**  ACKNOWLEDGEMENT
**      
**      The original BibTeX was written by Oren Patashnik using Donald 
**      Knuth's WEB system.  This format produces a PASCAL program for 
**      execution and a TeX documented version of the source code. This 
**      program started as a (manual) translation of the WEB source into C.
**  
**  CHANGE LOG
**
**      $Log: datatype.h,v $
**      Revision 3.71  1996/08/18  20:37:06  kempson
**      Official release 3.71 (see HISTORY file for details).
**
**      Revision 3.70  1996/04/08  10:08:40  kempson
**      Final documentation & cosmetic changes for official release 3.70.
**
**      Revision 3.5  1995/09/24  20:44:37  kempson
**      Many changes for final beta test version.
**
**      Revision 3.4  1995/04/09  22:15:44  kempson
**      Placed under RCS control
**
******************************************************************************
******************************************************************************
*/
#ifndef __DATATYPE_H__
# define __DATATYPE_H__             1


/*-
**============================================================================
** Define the generic data types.  For the BIG version of BibTeX, we declare
** that all integers are 32 bit.  This will only work on systems with a
** 'flat' 32 bit addressing scheme and lots of virtual memory.
**============================================================================
*/
typedef char                Boolean_T;
typedef unsigned char       UChar_T;
typedef char                Char_T;
typedef char                Integer8_T;
typedef FILE               *File_ptr;
typedef jmp_buf		    LongJumpBuf_T;


/*-
** Define some data types for 16-bit BibTex
**
** typedef short               Integer16_T;
** typedef long                Integer32_T;
** typedef unsigned short      Unsigned16_T;
** typedef unsigned long       Unsigned32_T;
** typedef Integer16_T         Integer_T;
*/

/*-
** Define some data types for 32-bit BibTeX
*/
typedef long                Integer16_T;
typedef long                Integer32_T;
typedef unsigned long       Unsigned16_T;
typedef unsigned long       Unsigned32_T;
typedef long                Integer_T;


/*-
**============================================================================
** The generic data types defined above will be used to approximate the
** data types used in the BibTeX WEB source.
**============================================================================
*/
typedef UChar_T             ASCIICode_T;
typedef File_ptr            AlphaFile_T;
typedef Integer16_T         AuxNumber_T;
typedef Integer16_T         BibNumber_T;
typedef Integer16_T         BltInRange_T;
typedef Integer16_T         BufPointer_T;
typedef ASCIICode_T        *BufType_T;
typedef Integer16_T         CiteNumber_T;
typedef Integer16_T         FieldLoc_T;
typedef Integer8_T          FnClass_T;

/*
**typedef Integer16_T         HashLoc_T;
**typedef Integer16_T         HashPointer_T;
**typedef Integer16_T         HashPtr2_T;
*/
typedef Integer32_T         HashLoc_T;
typedef Integer32_T         HashPointer_T;
typedef Integer32_T         HashPtr2_T;

typedef Integer8_T          IDType_T;
typedef Integer16_T         IntEntLoc_T;
typedef Integer8_T          LexType_T;
typedef Integer16_T         LitStkLoc_T;
typedef UChar_T             PdsLen_T;
typedef Integer8_T          PdsLoc_T;
typedef const Char_T       *PdsType_T;
typedef Unsigned16_T        PoolPointer_T;
typedef Integer8_T          StkType_T;
typedef Integer16_T         StrEntLoc_T;
typedef Integer8_T          StrGlobLoc_T;
typedef Integer8_T          StrIlk_T;
typedef Integer16_T         StrNumber_T;
typedef Integer16_T         WizFnLoc_T;



#endif                          /* __DATATYPE_H__ */
