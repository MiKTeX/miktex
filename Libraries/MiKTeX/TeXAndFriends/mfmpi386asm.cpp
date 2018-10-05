/* mfmpi386asm.cpp:

   Copyright (C) ???? Wayne Sullivan
   Copyright (C) ???? Fabrice Popineau
   Copyright (C) 1996-2016 Christian Schenk
 
   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

/* i386 assembly routines for inner loop fraction routines in Metafont
   and MetaPost. By Wayne Sullivan. */

/* Converted to inline assembler for Visual C++ [45].xx by Fabrice
   Popineau. */

/* Made it independent from host application.
   -- Christian Schenk */

#include "StdAfx.h"

#include "internal.h"

#pragma warning (disable: 4035)
#pragma warning (disable: 4740)

__declspec(naked) __declspec(dllexport) C4P_integer __stdcall MiKTeX::TeXAndFriends::TakeFraction (C4P_integer p, C4P_integer q, C4P_boolean & arithError)
{
  __asm {
        push ebp
        mov ebp, esp
        xor ecx, ecx
        mov eax, p
        cmp eax, 0x80000000
        jz LL5
        imul q
        or edx, edx
        jns LL2
        neg edx
        neg eax
        sbb edx, ecx
        inc ecx
LL2:
        add eax, 0x08000000
        adc edx, 0
        cmp edx, 0x07ffffff
        ja LL3
        shrd eax, edx, 28
LL1:
        jecxz LL4
        neg eax
LL4:
        mov esp, ebp
        pop ebp
        ret 12
LL5:     
        inc ecx
LL3:     
        mov eax, 0x7fffffff
  }
  arithError = true;
  __asm {
        jmp LL1
  }
}

__declspec(naked) __declspec(dllexport) C4P_integer __stdcall MiKTeX::TeXAndFriends::TakeScaled (C4P_integer p, C4P_integer q, C4P_boolean & arithError)
{
  __asm {
        push ebp
        mov ebp, esp
        xor ecx, ecx
        mov eax, p
        cmp eax, 0x80000000
        jz LL5
        imul q
        or edx, edx
        jns LL12
        neg edx
        neg eax
        sbb edx, ecx
        inc ecx
LL12:
        add eax, 0x00008000
        adc edx, 0x0
        cmp edx, 0x00007fff
        ja LL3
        shrd eax, edx, 16
LL1:
        jecxz LL4
        neg eax
LL4:
        mov esp, ebp
        pop ebp
        ret 12
LL5:     
        inc ecx
LL3:     
        mov eax, 0x7fffffff
  }
  arithError = true;
  __asm {
        jmp LL1
  }
}

__declspec(naked) __declspec(dllexport) C4P_integer __stdcall MiKTeX::TeXAndFriends::MakeScaled (C4P_integer p, C4P_integer q, C4P_boolean & arithError)
{
  __asm {
        mov cx, 16
        push ebp
        mov ebp, esp
        push ebx
        mov edx, p
        xor eax, eax
        or edx, edx
        jns LL32
        inc ch
        neg edx
LL32:
        mov ebx, q
        or ebx, ebx
        jns LL33
        dec ch
        neg ebx
        or ebx, ebx
        js LL34
LL33:
        or edx,edx
        js LL34
        shrd eax, edx, cl
        shr edx, cl
        cmp edx, ebx
        jae LL34
        div ebx
        add edx, edx
        inc edx
        sub ebx, edx
        adc eax, 0
        jc LL34
        cmp eax, 0x7fffffff
        ja LL34
LL31:   or ch, ch
        jz LL35
        neg eax
LL35:
        pop ebx
        mov esp, ebp
        pop ebp
        ret 12
LL34:    
        mov eax, 0x7fffffff
  }
  arithError = true;
  __asm {
        jmp LL31
  }
}

__declspec(naked) __declspec(dllexport) C4P_integer __stdcall MiKTeX::TeXAndFriends::MakeFraction (C4P_integer p, C4P_integer q, C4P_boolean & arithError)
{
  __asm {
        mov cx, 4
        push ebp
        mov ebp, esp
        push ebx
        mov edx, p
        xor eax, eax
        or edx, edx
        jns LL32
        inc ch
        neg edx
LL32:
        mov ebx, q
        or ebx, ebx
        jns LL33
        dec ch
        neg ebx
        or ebx, ebx
        js LL34
LL33:
        or edx, edx
        js LL34
        shrd eax, edx, cl
        shr edx, cl
        cmp edx, ebx
        jae LL34
        div ebx
        add edx, edx
        inc edx
        sub ebx, edx
        adc eax, 0
        jc LL34
        cmp eax, 0x7fffffff
        ja LL34
LL31:   or ch, ch
        jz LL35
        neg eax
LL35:
        pop ebx
        mov esp, ebp
        pop ebp
        ret 12
LL34:    
        mov eax, 0x7fffffff
  }
  arithError = true;
  __asm {
        jmp LL31
  }
}
