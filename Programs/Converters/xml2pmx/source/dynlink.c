/*
 * dynlink.c
 * 
 * This file is part of the Oxford Oberon-2 compiler
 * Copyright (c) 2006--2016 J. M. Spivey
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
Initially, the procedure descriptor for each such primitive has the
trap handler |dltrap| as its interpreter, and the CP_CODE field of
the descriptor points to the name of the primitive as a string.
When the primitive is first called, the |dltrap| primitive looks up
the symbol and binds the primitive to its value for future use.
Finally, it calls the newly-loaded primitive to complete the first
call.

Function |load_lib| loads a dynamic library.  Each Oberon module that
links to a dynamic library should call |DynLink.Load("path")| 
in its initialization part.
*/

#include "obx.h"
#include <string.h>
#include <stdlib.h>

#ifdef DYNLINK

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <dlfcn.h>

#ifdef USE_FFI
#include <ffi.h>
#endif

void load_lib(char *fname) {
     char buf[128];
	
     /* If the library name starts with '@', look in the OBC lib directory 
        and append the extension ".so" or ".dylib" au chois */
     if (fname[0] == '@') {
	  char *dir = getenv("OBC_LIB");
	  if (dir == NULL) dir = libpath;
	  if (dir == NULL) panic("no runtime library");
	  strcpy(buf, dir);
	  strcat(buf, "/");
	  strcat(buf, fname+1);
	  strcat(buf, DLEXT);
	  fname = buf;
     }

     /* Load the library */
     if (dlopen(fname, RTLD_LAZY|RTLD_GLOBAL) == NULL) 
	  panic(dlerror());
}

#ifdef USE_FFI
#define MAXP 16

typedef struct {
     void (*fun)(void);
     ffi_cif cif;
} wrapper;

static ffi_type *ffi_decode(char c) {
     switch (c) {
     case 'C':
     case 'S':
     case 'I':
          return &ffi_type_sint32;
     case 'L':
          return &ffi_type_sint64;
     case 'F':
          return &ffi_type_float;
     case 'D':
          return &ffi_type_double;
     case 'P':
     case 'Q':
     case 'X':
          return &ffi_type_pointer;
     case 'V':
          return &ffi_type_void;
     default:
          panic("Bad type %c", c);
          return NULL;
     }
}

value *dlstub(value *bp) {
     value *cp = valptr(bp[CP]);
     char *tstring = pointer(cp[CP_CODE]);
     value *sp = bp;

     ffi_raw avals[MAXP], rval[2];
     int i, p = 0, q = 0;
     double d; longint z;
     
     FPINIT;

     for (i = 0; tstring[i+1] != '\0'; i++) {
          switch (tstring[i+1]) {
          case 'C':
               avals[q].sint = align_byte(bp[HEAD+p].i);
               p += 1; q += 1; break;
          case 'S':
               avals[q].sint = align_short(bp[HEAD+p].i);
               p += 1; q += 1; break;
          case 'I':
               avals[q].sint = bp[HEAD+p].i;
               p += 1; q += 1; break;
          case 'L':
               z = get_long(&bp[HEAD+p]);
               memcpy(avals[q].data, &z, sizeof(longint));
               p += 2; q += sizeof(longint)/sizeof(ffi_raw); break;
          case 'F':
               avals[q].flt = bp[HEAD+p].f;
               p += 1; q += 1; break;
          case 'D':
               d = get_double(&bp[HEAD+p]);
               memcpy(avals[q].data, &d, sizeof(double));
               p += 2; q += sizeof(double)/sizeof(ffi_raw); break;
          case 'P':
               avals[q].ptr = pointer(bp[HEAD+p]);
               p += 1; q += 1; break;
          case 'X':
               avals[q].ptr = pointer(bp[HEAD+p]);
               p += 2; q += 1; break;
          case 'Q':
               avals[q].ptr = ptrcast(uchar, get_long(&bp[HEAD+p]));
               p += 2; q += 1; break;
#ifdef SPECIALS
          case 'S':
               /* Static link for compilers course -- ignored */
               p += 1; break;
#endif
          default:
               panic("Bad type 2 %c", tstring[i+1]);
          }
     }

     wrapper *w = pointer(cp[CP_CONST]);
     ffi_raw_call(&w->cif, w->fun, rval, avals);
     
     switch (tstring[0]) {
     case 'C':
     case 'S':
     case 'I':
          (*--sp).i = rval->sint;
          break;
     case 'L':
          memcpy(&z, rval, sizeof(longint));
          sp -= 2;
          put_long(sp, z);
          break;
     case 'F':
          (*--sp).f = rval->flt;
          break;
     case 'D':
          memcpy(&d, rval, sizeof(double));
          sp -= 2;
          put_double(sp, d);
          break;
     case 'P':
          (*--sp).a = rval->uint;
          break;
     case 'Q':
          sp -= 2;
          put_long(sp, (ptrtype) rval->ptr);
          break;
     case 'V':
          break;
     default:
          panic("Bad type 3");
     }

     return sp;
}
#endif

primitive *find_prim(char *name) {
     return (primitive *) dlsym(RTLD_DEFAULT, name);
}

#else

void load_lib(char *fname) {
}

primitive *find_prim(char *name) {
     int i;

     for (i = 0; primtab[i].p_name != NULL; i++) {
	  if (strcmp(name, primtab[i].p_name) == 0)
	       return primtab[i].p_prim;
     }

     return NULL;
}

#endif

value *dltrap(value *bp) {
     value *cp = valptr(bp[CP]);
     char *tstring = pointer(cp[CP_CODE]);
     char *name = tstring + strlen(tstring) + 1;
     primitive *prim = NULL;

     if (*name == '=') name++;

     if (tstring[0] == '*')
          prim = find_prim(name);
     else {
          /* Look for a static wrapper */
          char primname[32];
          sprintf(primname, "P_%s", name);
          prim = find_prim(primname);
     }

     if (prim != NULL) {
          cp[CP_PRIM].a = wrap_prim(prim);
          return (*prim)(bp);
     }

#ifdef DYNLINK
#ifdef USE_FFI
     /* Build a wrapper with FFI */
     void (*fun)(void) = (void(*)(void)) dlsym(RTLD_DEFAULT, name);

     if (fun != NULL && tstring[0] != '*') {
          int np = strlen(tstring)-1;
          ffi_type *rtype = ffi_decode(tstring[0]);
          ffi_type **atypes = scratch_alloc_atomic(np * sizeof(ffi_type *));
          for (int i = 0; tstring[i+1] != '\0'; i++)
               atypes[i] = ffi_decode(tstring[i+1]);

          word a = virtual_alloc_atomic(sizeof(wrapper));
          wrapper *w = ptrcast(wrapper, a);
          w->fun = fun;
          ffi_prep_cif(&w->cif, FFI_DEFAULT_ABI, np, rtype, atypes);

          cp[CP_PRIM].a = dynstub;
          cp[CP_CONST].a = a;

          return dlstub(bp);
     }
#endif
#endif

     panic("Couldn't find primitive %s", name);
     return NULL;
}
