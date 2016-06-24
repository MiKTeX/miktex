% omstr.ch: String manipulation
%
% This file is part of the Omega project, which
% is based on the web2c distribution of TeX.
%
% Copyright (c) 1994--2000 John Plaice and Yannis Haralambous
% 
% This library is free software; you can redistribute it and/or
% modify it under the terms of the GNU Library General Public
% License as published by the Free Software Foundation; either
% version 2 of the License, or (at your option) any later version.
% 
% This library is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% Library General Public License for more details.
% 
% You should have received a copy of the GNU Library General Public
% License along with this library; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
%
%---------------------------------------
@x [2] m.17 l.510 - Omega
In order to make \TeX\ readily portable to a wide variety of
computers, all of its input text is converted to an internal eight-bit
@y
In order to make $\Omega$ readily portable to a wide variety of
computers, all of its input text is converted to an internal 31-bit
@z
%---------------------------------------
@x [2] m.18 l.537 - Omega
@!ASCII_code=0..255; {eight-bit numbers}
@y
@!ASCII_code=0..biggest_char;
@z
%---------------------------------------
@x [2] m.19 l.567 - Omega
@d last_text_char=255 {ordinal number of the largest element of |text_char|}
@y
@d last_text_char=biggest_char 
   {ordinal number of the largest element of |text_char|}
@z
%---------------------------------------
@x [2] m.20 l.577 - Omega
@!xord: array [text_char] of ASCII_code;
  {specifies conversion of input characters}
@!xchr: array [ASCII_code] of text_char;
  {specifies conversion of output characters}
@y
@!xchr: array [0..255] of text_char;
  {specifies conversion of output characters}
@z
%---------------------------------------
@x [2] m.23 l.724 - Omega
for i:=0 to @'37 do xchr[i]:=' ';
for i:=@'177 to @'377 do xchr[i]:=' ';
@y
for i:=0 to @'37 do xchr[i]:=chr(i);
for i:=@'177 to @'377 do xchr[i]:=chr(i);
init_eqtb_table;
@z
%---------------------------------------
@x [2] m.24 l.734 - Omega
for i:=first_text_char to last_text_char do xord[chr(i)]:=invalid_code;
for i:=@'200 to @'377 do xord[xchr[i]]:=i;
for i:=0 to @'176 do xord[xchr[i]]:=i;
@y
@z
%---------------------------------------
@x [3] m.25 l.772 - Omega
@!eight_bits=0..255; {unsigned one-byte quantity}
@!alpha_file=packed file of text_char; {files that contain textual data}
@!byte_file=packed file of eight_bits; {files that contain binary data}
@y
@!eight_bits=0..biggest_char; {unsigned two-byte quantity}
@!real_eight_bits=0..255; {dvi output uses exactly 8 bits}
@!alpha_file=packed file of text_char; {files that contain textual data}
@!byte_file=packed file of real_eight_bits; {files that contain binary data}
@z
%---------------------------------------
@x [3] m.31 l.947 - Omega
    buffer[last]:=xord[f^]; get(f); incr(last);
@y
    buffer[last]:=f^; get(f); incr(last);
@z
%---------------------------------------
@x [4] m.38 l.1099 - Omega
String numbers 0 to 255 are reserved for strings that correspond to single
ASCII characters. This is in accordance with the conventions of \.{WEB},
@y
String numbers 0 to |biggest_char| are reserved for strings that correspond to 
single ASCII characters. This is in accordance with the conventions of \.{WEB},
@z
%---------------------------------------
@x [4] m.38 l.1109 - Omega
ASCII character, so the first 256 strings are used to specify exactly what
should be printed for each of the 256 possibilities.
@y
ASCII character, so the first |biggest_char+1| strings are used to specify 
exactly what should be printed for each of the |biggest_char+1| possibilities.
@z
%---------------------------------------
@x [4] m.38-9 l.1122 - Omega
@d si(#) == # {convert from |ASCII_code| to |packed_ASCII_code|}
@d so(#) == # {convert from |packed_ASCII_code| to |ASCII_code|}

@<Types...@>=
@!pool_pointer = 0..pool_size; {for variables that point into |str_pool|}
@!str_number = 0..max_strings; {for variables that point into |str_start|}
@!packed_ASCII_code = 0..255; {elements of |str_pool| array}

@ @<Glob...@>=
@!str_pool:packed array[pool_pointer] of packed_ASCII_code; {the characters}
@!str_start : array[str_number] of pool_pointer; {the starting pointers}
@y
@d si(#) == # {convert from |ASCII_code| to |packed_ASCII_code|}
@d so(#) == # {convert from |packed_ASCII_code| to |ASCII_code|}
@d str_start(#) == str_start_ar[#-too_big_char]

@<Types...@>=
@!pool_pointer = 0..pool_size; {for variables that point into |str_pool|}
@!str_number = 0..max_strings; {for variables that point into |str_start|}
@!packed_ASCII_code = 0..65535; {elements of |str_pool| array}

@ @<Glob...@>=
@!str_pool:packed array[pool_pointer] of packed_ASCII_code; {the characters}
@!str_start_ar : array[str_number] of pool_pointer; {the starting pointers}
@z
%---------------------------------------
@x [4] m.40 l.1145 - Omega
@d length(#)==(str_start[#+1]-str_start[#]) {the number of characters
  in string number \#}
@y
@p function length(s:str_number):integer;
   {the number of characters in string number |s|}
begin if (s>=@"10000) then length:=str_start(s+1)-str_start(s)
else if (s>=@"20) and (s<@"7F) then length:=1
else if (s<=@"7F) then length:=3
else if (s<@"100) then length:=4
else length:=8
end;
@z
%---------------------------------------
@x [4] m.41 l.1150 - Omega
@d cur_length == (pool_ptr - str_start[str_ptr])
@y
@d cur_length == (pool_ptr - str_start(str_ptr))
@z
%---------------------------------------
@x [4] m.43 l.1181 - Omega
incr(str_ptr); str_start[str_ptr]:=pool_ptr;
@y
incr(str_ptr); str_start(str_ptr):=pool_ptr;
@z
%---------------------------------------
@x [4] m.44 l.1187 - Omega
@d flush_string==begin decr(str_ptr); pool_ptr:=str_start[str_ptr];
@y
@d flush_string==begin decr(str_ptr); pool_ptr:=str_start(str_ptr);
@z
%---------------------------------------
@x [4] m.45 l.1201 - Omega
begin j:=str_start[s];
while j<str_start[s+1] do
@y
begin j:=str_start(s);
while j<str_start(s+1) do
@z
%---------------------------------------
@x [4] m.46 l.1222 - Omega
@p function str_eq_str(@!s,@!t:str_number):boolean;
  {test equality of strings}
label not_found; {loop exit}
var j,@!k: pool_pointer; {running indices}
@!result: boolean; {result of comparison}
begin result:=false;
if length(s)<>length(t) then goto not_found;
j:=str_start[s]; k:=str_start[t];
while j<str_start[s+1] do
  begin if str_pool[j]<>str_pool[k] then goto not_found;
  incr(j); incr(k);
  end;
result:=true;
not_found: str_eq_str:=result;
end;
@y
@p function str_eq_str(@!s,@!t:str_number):boolean;
  {test equality of strings}
label not_found; {loop exit}
var j,@!k: pool_pointer; {running indices}
@!result: boolean; {result of comparison}
begin result:=false;
if length(s)<>length(t) then goto not_found;
if (length(s)=1) then begin
  if s<65536 then begin
    if t<65536 then begin
      if s<>t then goto not_found;
      end
    else begin
      if s<>str_pool[str_start(t)] then goto not_found;
      end;
    end
  else begin
    if t<65536 then begin
      if str_pool[str_start(s)]<>t then goto not_found;
      end
    else begin
      if str_pool[str_start(s)]<>str_pool[str_start(t)] then
        goto not_found;
      end;
    end;
  end
else begin
  j:=str_start(s); k:=str_start(t);
  while j<str_start(s+1) do
    begin if str_pool[j]<>str_pool[k] then goto not_found;
    incr(j); incr(k);
    end;
  end;
result:=true;
not_found: str_eq_str:=result;
end;
@z
%---------------------------------------
@x [4] m.47 l.1240 - Omega
var k,@!l:0..255; {small indices or counters}
@y
var k,@!l:0..biggest_char; {small indices or counters}
@z
%---------------------------------------
@x [4] m.47 l.1245 - Omega
begin pool_ptr:=0; str_ptr:=0; str_start[0]:=0;
@<Make the first 256 strings@>;
@<Read the other strings from the \.{TEX.POOL} file and return |true|,
@y
begin pool_ptr:=0; str_ptr:=0; str_start_ar[0]:=0; str_start_ar[1]:=0;
@<Make the first strings@>;
@<Read the other strings from the \.{OMEGA.POOL} file and return |true|,
@z
%---------------------------------------
% A hack is made for strings 256 through to 65535.
% String 256 becomes "^^^^????".  All the strings
% from 256 to 65535 are made to point at this string,
% which will never be printed:  it's just a placeholder.

@x [4] m.48 l.1255 - Omega
@<Make the first 256...@>=
for k:=0 to 255 do
  begin if (@<Character |k| cannot be printed@>) then
    begin append_char("^"); append_char("^");
    if k<@'100 then append_char(k+@'100)
    else if k<@'200 then append_char(k-@'100)
    else begin app_lc_hex(k div 16); app_lc_hex(k mod 16);
      end;
    end
  else append_char(k);
  g:=make_string;
  end
@y
@<Make the first ...@>=
begin
str_ptr:=too_big_char;
end
@z
%---------------------------------------
@x [4] m.49 l.1279 - Omega
Unprintable characters of codes 128--255 are, similarly, rendered
\.{\^\^80}--\.{\^\^ff}.
@y
Unprintable characters of codes 128--255 are, similarly, rendered
\.{\^\^80}--\.{\^\^ff}.

Unprintable characters of codes 256--|biggest_char| are, similarly, rendered
\.{\^\^\^\^0100}--\.{\^\^\^\^ffff}.

@z
%---------------------------------------
@x [4] m.49 l.1295 - Omega
@<Character |k| cannot be printed@>=
  (k<" ")or(k>"~")
@y
@z
%---------------------------------------
@x [4] m.50 l.1298 - Omega
@ When the \.{WEB} system program called \.{TANGLE} processes the \.{TEX.WEB}
description that you are now reading, it outputs the \PASCAL\ program
\.{TEX.PAS} and also a string pool file called \.{TEX.POOL}. The \.{INITEX}
@y
@ When the \.{WEB} system program called \.{OTANGLE} processes the \.{OMEGA.WEB}
description that you are now reading, it outputs the \PASCAL\ program
\.{OMEGA.PAS} and also a string pool file called \.{OMEGA.POOL}. The \.{INITEX}
@z
%---------------------------------------
@x [4] m.51 l.1322 - Omega
else  bad_pool('! I can''t read TEX.POOL.')
@.I can't read TEX.POOL@>
@y
else  bad_pool('! I can''t read OMEGA.POOL.')
@.I can't read OMEGA.POOL@>
@z
%---------------------------------------
@x [4] m.52 l.1326 - Omega
begin if eof(pool_file) then bad_pool('! TEX.POOL has no check sum.');
@.TEX.POOL has no check sum@>
@y
begin if eof(pool_file) then bad_pool('! OMEGA.POOL has no check sum.');
@.OMEGA.POOL has no check sum@>
@z
%---------------------------------------
@x [4] m.52 l.1332 - Omega
else  begin if (xord[m]<"0")or(xord[m]>"9")or@|
      (xord[n]<"0")or(xord[n]>"9") then
    bad_pool('! TEX.POOL line doesn''t begin with two digits.');
@.TEX.POOL line doesn't...@>
  l:=xord[m]*10+xord[n]-"0"*11; {compute the length}
  if pool_ptr+l+string_vacancies>pool_size then
    bad_pool('! You have to increase POOLSIZE.');
@.You have to increase POOLSIZE@>
  for k:=1 to l do
    begin if eoln(pool_file) then m:=' '@+else read(pool_file,m);
    append_char(xord[m]);
@y
else  begin if (m<"0")or(m>"9")or@|
      (n<"0")or(n>"9") then
    bad_pool('! OMEGA.POOL line doesn''t begin with two digits.');
@.OMEGA.POOL line doesn't...@>
  l:=m*10+n-"0"*11; {compute the length}
  if pool_ptr+l+string_vacancies>pool_size then
    bad_pool('! You have to increase POOLSIZE.');
@.You have to increase POOLSIZE@>
  for k:=1 to l do
    begin if eoln(pool_file) then m:=' '@+else read(pool_file,m);
    append_char(m);
@z
%---------------------------------------
@x [4] m.53 l.1347 - Omega
end of this \.{TEX.POOL} file; any other value means that the wrong pool
@y
end of this \.{OMEGA.POOL} file; any other value means that the wrong pool
@z
%---------------------------------------
@x [4] m.53 l.1354 - Omega
loop@+  begin if (xord[n]<"0")or(xord[n]>"9") then
  bad_pool('! TEX.POOL check sum doesn''t have nine digits.');
@.TEX.POOL check sum...@>
  a:=10*a+xord[n]-"0";
@y
loop@+  begin if (n<"0")or(n>"9") then
  bad_pool('! OMEGA.POOL check sum doesn''t have nine digits.');
@.OMEGA.POOL check sum...@>
  a:=10*a+n-"0";
@z
%---------------------------------------
@x [4] m.53 l.1360 - Omega
done: if a<>@$ then bad_pool('! TEX.POOL doesn''t match; TANGLE me again.');
@.TEX.POOL doesn't match@>
@y
done: if a<>@$ then bad_pool('! OMEGA.POOL doesn''t match; OTANGLE me again.');
@.OMEGA.POOL doesn't match@>
@z
%---------------------------------------
% When we print a string, we must make sure we do the appropriate
% thing for strings 256 through to 65535.  We must generate the
% strings on the fly.

@x [5] m.59 l.1496 - Omega
@ An entire string is output by calling |print|. Note that if we are outputting
the single standard ASCII character \.c, we could call |print("c")|, since
|"c"=99| is the number of a single-character string, as explained above. But
|print_char("c")| is quicker, so \TeX\ goes directly to the |print_char|
routine when it knows that this is safe. (The present implementation
assumes that it is always safe to print a visible ASCII character.)
@^system dependencies@>

@<Basic print...@>=
procedure print(@!s:integer); {prints string |s|}
label exit;
var j:pool_pointer; {current character code position}
@!nl:integer; {new-line character to restore}
begin if s>=str_ptr then s:="???" {this can't happen}
@.???@>
else if s<256 then
  if s<0 then s:="???" {can't happen}
  else begin if selector>pseudo then
      begin print_char(s); return; {internal strings are not expanded}
      end;
    if (@<Character |s| is the current new-line character@>) then
      if selector<pseudo then
        begin print_ln; return;
        end;
    nl:=new_line_char; new_line_char:=-1;
      {temporarily disable new-line character}
    j:=str_start[s];
    while j<str_start[s+1] do
      begin print_char(so(str_pool[j])); incr(j);
      end;
    new_line_char:=nl; return;
    end;
j:=str_start[s];
while j<str_start[s+1] do
@y
@ An entire string is output by calling |print|. Note that if we are outputting
the single standard ASCII character \.c, we could call |print("c")|, since
|"c"=99| is the number of a single-character string, as explained above. But
|print_char("c")| is quicker, so \TeX\ goes directly to the |print_char|
routine when it knows that this is safe. (The present implementation
assumes that it is always safe to print a visible ASCII character.)
@^system dependencies@>

@d print_lc_hex(#)==l:=#;
   if l<10 then print_char(l+"0") else print_char(l-10+"a");

@<Basic print...@>=
procedure print(@!s:integer); {prints string |s|}
label exit;
var j:pool_pointer; {current character code position}
@!nl:integer; {new-line character to restore}
@!l:integer; {for printing 16-bit characters}
begin if s>=str_ptr then s:="???" {this can't happen}
@.???@>
else if s<biggest_char then
  if s<0 then s:="???" {can't happen}
  else begin if selector>pseudo then
      begin print_char(s); return; {internal strings are not expanded}
      end;
    if (@<Character |s| is the current new-line character@>) then
      if selector<pseudo then
        begin print_ln; return;
        end;
    nl:=new_line_char;
    @<Set newline character to -1@>;
    if s<@"20 then begin
      print_char(@"5E); print_char(@"5E); print_char(s+@'100);
      end
    else if s<@"7F then
      print_char(s)
    else if s=@"7F then begin
      print_char(@"5E); print_char(@"5E); print_char(s-@'100);
      end
    else if s<@"100 then begin
      print_char(@"5E); print_char(@"5E);
      print_lc_hex((s mod @"100) div @"10); print_lc_hex(s mod @"10);
      end
    else begin {Here is where we generate the strings on the fly.}
      print_char(@"5E); print_char(@"5E);
      print_char(@"5E); print_char(@"5E);
      print_lc_hex(s div @"1000); print_lc_hex((s mod @"1000) div @"100);
      print_lc_hex((s mod @"100) div @"10); print_lc_hex(s mod @"10);
      end;
    @<Set newline character to nl@>;
    return;
    end;
j:=str_start(s);
while j<str_start(s+1) do
@z
%---------------------------------------
@x [5] m.60 l.1539 - Omega
procedure slow_print(@!s:integer); {prints string |s|}
var j:pool_pointer; {current character code position}
begin if (s>=str_ptr) or (s<256) then print(s)
else begin j:=str_start[s];
  while j<str_start[s+1] do
@y
procedure slow_print(@!s:integer); {prints string |s|}
var j:pool_pointer; {current character code position}
begin if (s>=str_ptr) or (s<=biggest_char) then print(s)
else begin j:=str_start(s);
  while j<str_start(s+1) do
@z
%---------------------------------------
@x [5] m.61 l.1556 - Omega
wterm(banner);
if format_ident=0 then wterm_ln(' (no format preloaded)')
else  begin slow_print(format_ident); print_ln;
  end;
update_terminal;
@y
wterm(banner);
if format_ident=0 then wterm_ln(' (no format preloaded)')
else  begin slow_print(format_ident); print_ln;
  end;
wterm_ln('Copyright (c) 1994--2000 John Plaice and Yannis Haralambous');
update_terminal;
@z
%---------------------------------------
@x [5] m.63 l.1576 - Omega
procedure print_esc(@!s:str_number); {prints escape character, then |s|}
var c:integer; {the escape character code}
begin  @<Set variable |c| to the current escape character@>;
if c>=0 then if c<256 then print(c);
@y
procedure print_esc(@!s:str_number); {prints escape character, then |s|}
var c:integer; {the escape character code}
begin  @<Set variable |c| to the current escape character@>;
if c>=0 then if c<=biggest_char then print(c);
@z
%---------------------------------------
@x [5] m.69 l.1654 - Omega
@p procedure print_roman_int(@!n:integer);
label exit;
var j,@!k: pool_pointer; {mysterious indices into |str_pool|}
@!u,@!v: nonnegative_integer; {mysterious numbers}
begin j:=str_start["m2d5c2l5x2v5i"]; v:=1000;
@y
@p procedure print_roman_int(@!n:integer);
label exit;
var j,@!k: pool_pointer; {mysterious indices into |str_pool|}
@!u,@!v: nonnegative_integer; {mysterious numbers}
begin j:=str_start("m2d5c2l5x2v5i"); v:=1000;
@z
%---------------------------------------
@x [5] m.70 l.1674 - Omega
@p procedure print_current_string; {prints a yet-unmade string}
var j:pool_pointer; {points to current character code}
begin j:=str_start[str_ptr];
@y
@p procedure print_current_string; {prints a yet-unmade string}
var j:pool_pointer; {points to current character code}
begin j:=str_start(str_ptr);
@z
%---------------------------------------
@x [6] m.94 l.2052 - Omega
print_err("TeX capacity exceeded, sorry [");
@y
print_err("Omega capacity exceeded, sorry [");
@z
%---------------------------------------
@x [18] m.260 l.5532 - Omega
while pool_ptr>str_start[str_ptr] do
@y
while pool_ptr>str_start(str_ptr) do
@z
%---------------------------------------
@x [18] m.264 l.5610 - Omega
else  begin k:=str_start[s]; l:=str_start[s+1]-k;
@y
else  begin k:=str_start(s); l:=str_start(s+1)-k;
@z
%---------------------------------------
@x [24] m.341 l.7219 - Omega
@!c,@!cc:ASCII_code; {constituents of a possible expanded code}
@!d:2..3; {number of excess characters in an expanded code}
@y
@!c,@!cc,@!ccc,@!cccc:ASCII_code; {constituents of a possible expanded code}
@!d:2..7; {number of excess characters in an expanded code}
@z
%---------------------------------------
@x [24] m.352 l.7349 - Omega
  else cur_chr:=16*cur_chr+cc-"a"+10
@y
  else cur_chr:=16*cur_chr+cc-"a"+10
@d long_hex_to_cur_chr==
  if c<="9" then cur_chr:=c-"0" @+else cur_chr:=c-"a"+10;
  if cc<="9" then cur_chr:=16*cur_chr+cc-"0"
  else cur_chr:=16*cur_chr+cc-"a"+10;
  if ccc<="9" then cur_chr:=16*cur_chr+ccc-"0"
  else cur_chr:=16*cur_chr+ccc-"a"+10;
  if cccc<="9" then cur_chr:=16*cur_chr+cccc-"0"
  else cur_chr:=16*cur_chr+cccc-"a"+10
 
@z
%---------------------------------------
@x [24] m.352 l.7353 - Omega
  begin c:=buffer[loc+1]; @+if c<@'200 then {yes we have an expanded char}
@y
  begin if (cur_chr=buffer[loc+1]) and (cur_chr=buffer[loc+2]) and
           ((loc+6)<=limit) then 
     begin c:=buffer[loc+3]; cc:=buffer[loc+4]; 
       ccc:=buffer[loc+5]; cccc:=buffer[loc+6];
       if is_hex(c) and is_hex(cc) and is_hex(ccc) and is_hex(cccc) then 
       begin loc:=loc+7; long_hex_to_cur_chr; goto reswitch;
       end;
     end;
  c:=buffer[loc+1]; @+if c<@'200 then {yes we have an expanded char}
@z
%---------------------------------------
@x [24] m.355 l.7416 - Omega
begin if buffer[k]=cur_chr then @+if cat=sup_mark then @+if k<limit then
  begin c:=buffer[k+1]; @+if c<@'200 then {yes, one is indeed present}
    begin d:=2;
    if is_hex(c) then @+if k+2<=limit then
      begin cc:=buffer[k+2]; @+if is_hex(cc) then incr(d);
      end;
    if d>2 then
      begin hex_to_cur_chr; buffer[k-1]:=cur_chr;
      end
    else if c<@'100 then buffer[k-1]:=c+@'100
    else buffer[k-1]:=c-@'100;
    limit:=limit-d; first:=first-d;
    while k<=limit do
      begin buffer[k]:=buffer[k+d]; incr(k);
      end;
    goto start_cs;
    end;
  end;
end

@y
begin if buffer[k]=cur_chr then @+if cat=sup_mark then @+if k<limit then
  begin if (cur_chr=buffer[k+1]) and (cur_chr=buffer[k+2]) and 
           ((k+6)<=limit) then 
     begin c:=buffer[k+3]; cc:=buffer[k+4]; 
       ccc:=buffer[k+5]; cccc:=buffer[k+6];
       if is_hex(c) and is_hex(cc) and is_hex(ccc) and is_hex(cccc) then 
       begin d:=7; long_hex_to_cur_chr; buffer[k-1]:=cur_chr;
             while k<=limit do
                begin buffer[k]:=buffer[k+d]; incr(k);
                end;
             goto start_cs;
       end
     end
     else begin 
       c:=buffer[k+1]; @+if c<@'200 then {yes, one is indeed present}
       begin 
          d:=2;
          if is_hex(c) then @+if k+2<=limit then
            begin cc:=buffer[k+2]; @+if is_hex(cc) then incr(d);
            end;
          if d>2 then
            begin hex_to_cur_chr; buffer[k-1]:=cur_chr;
            end
          else if c<@'100 then buffer[k-1]:=c+@'100
          else buffer[k-1]:=c-@'100;
          limit:=limit-d; first:=first-d;
          while k<=limit do
            begin buffer[k]:=buffer[k+d]; incr(k);
            end;
          goto start_cs;
       end
     end
  end
end
@z
%---------------------------------------
@x [26] m.407 l.8216 - Omega
begin p:=backup_head; link(p):=null; k:=str_start[s];
while k<str_start[s+1] do
  begin get_x_token; {recursion is possible here}
@^recursion@>
  if (cur_cs=0)and@|
   ((cur_chr=so(str_pool[k]))or(cur_chr=so(str_pool[k])-"a"+"A")) then
    begin store_new_token(cur_tok); incr(k);
    end
  else if (cur_cmd<>spacer)or(p<>backup_head) then
    begin back_input;
    if p<>backup_head then back_list(link(backup_head));
    scan_keyword:=false; return;
    end;
  end;
@y
begin p:=backup_head; link(p):=null;
if s<too_big_char then begin
  while true do
    begin get_x_token; {recursion is possible here}
@^recursion@>
    if (cur_cs=0)and@|
       ((cur_chr=s)or(cur_chr=s-"a"+"A")) then
      begin store_new_token(cur_tok);
      flush_list(link(backup_head)); scan_keyword:=true; return;
      end
    else if (cur_cmd<>spacer)or(p<>backup_head) then
      begin back_input;
      if p<>backup_head then back_list(link(backup_head));
      scan_keyword:=false; return;
      end;
    end;
  end;
k:=str_start(s);
while k<str_start(s+1) do
  begin get_x_token; {recursion is possible here}
@^recursion@>
  if (cur_cs=0)and@|
   ((cur_chr=so(str_pool[k]))or(cur_chr=so(str_pool[k])-"a"+"A")) then
    begin store_new_token(cur_tok); incr(k);
    end
  else if (cur_cmd<>spacer)or(p<>backup_head) then
    begin back_input;
    if p<>backup_head then back_list(link(backup_head));
    scan_keyword:=false; return;
    end;
  end;
@z
%---------------------------------------
@x [29] m.517 l.10005 - Omega
@p procedure end_name;
begin if str_ptr+3>max_strings then
  overflow("number of strings",max_strings-init_str_ptr);
@:TeX capacity exceeded number of strings}{\quad number of strings@>
if area_delimiter=0 then cur_area:=""
else  begin cur_area:=str_ptr;
  str_start[str_ptr+1]:=str_start[str_ptr]+area_delimiter; incr(str_ptr);
  end;
if ext_delimiter=0 then
  begin cur_ext:=""; cur_name:=make_string;
  end
else  begin cur_name:=str_ptr;
  str_start[str_ptr+1]:=str_start[str_ptr]+ext_delimiter-area_delimiter-1;
@y
@p procedure end_name;
begin if str_ptr+3>max_strings then
  overflow("number of strings",max_strings-init_str_ptr);
@:TeX capacity exceeded number of strings}{\quad number of strings@>
if area_delimiter=0 then cur_area:=""
else  begin cur_area:=str_ptr;
  str_start(str_ptr+1):=str_start(str_ptr)+area_delimiter; incr(str_ptr);
  end;
if ext_delimiter=0 then
  begin cur_ext:=""; cur_name:=make_string;
  end
else  begin cur_name:=str_ptr;
  str_start(str_ptr+1):=str_start(str_ptr)+ext_delimiter-area_delimiter-1;
@z
%---------------------------------------
@x [29] m.519 l.10039 - Omega
  if k<=file_name_size then name_of_file[k]:=xchr[c];
  end

@p procedure pack_file_name(@!n,@!a,@!e:str_number);
var k:integer; {number of positions filled in |name_of_file|}
@!c: ASCII_code; {character being packed}
@!j:pool_pointer; {index into |str_pool|}
begin k:=0;
for j:=str_start[a] to str_start[a+1]-1 do append_to_name(so(str_pool[j]));
for j:=str_start[n] to str_start[n+1]-1 do append_to_name(so(str_pool[j]));
for j:=str_start[e] to str_start[e+1]-1 do append_to_name(so(str_pool[j]));
@y
  if k<=file_name_size then name_of_file[k]:=xchr[c];
  end

@p procedure pack_file_name(@!n,@!a,@!e:str_number);
var k:integer; {number of positions filled in |name_of_file|}
@!c: ASCII_code; {character being packed}
@!j:pool_pointer; {index into |str_pool|}
begin k:=0;
for j:=str_start(a) to str_start(a+1)-1 do append_to_name(so(str_pool[j]));
for j:=str_start(n) to str_start(n+1)-1 do append_to_name(so(str_pool[j]));
for j:=str_start(e) to str_start(e+1)-1 do append_to_name(so(str_pool[j]));
@z
%---------------------------------------
@x [29] m.523 l.10095 - Omega
for j:=1 to n do append_to_name(xord[TEX_format_default[j]]);
for j:=a to b do append_to_name(buffer[j]);
for j:=format_default_length-format_ext_length+1 to format_default_length do
  append_to_name(xord[TEX_format_default[j]]);
@y
for j:=1 to n do append_to_name(TEX_format_default[j]);
for j:=a to b do append_to_name(buffer[j]);
for j:=format_default_length-format_ext_length+1 to format_default_length do
  append_to_name(TEX_format_default[j]);
@z
%---------------------------------------
@x [29] m.524 l.10155 - Omega
else  begin for k:=1 to name_length do append_char(xord[name_of_file[k]]);
@y
else  begin for k:=1 to name_length do append_char(name_of_file[k]);
@z
%---------------------------------------
@x [29] m.536 l.10324 - Omega
begin wlog(banner);
slow_print(format_ident); print("  ");
print_int(day); print_char(" ");
months:='JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC';
for k:=3*month-2 to 3*month do wlog(months[k]);
print_char(" "); print_int(year); print_char(" ");
print_two(time div 60); print_char(":"); print_two(time mod 60);
end
@y
begin wlog(banner);
slow_print(format_ident); print_nl("");
print_int(day); print_char(" ");
months:='JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC';
for k:=3*month-2 to 3*month do wlog(months[k]);
print_char(" "); print_int(year); print_char(" ");
print_two(time div 60); print_char(":"); print_two(time mod 60);
print_nl("Copyright (c) 1994--2000 John Plaice and Yannis Haralambous");
end
@z
%---------------------------------------
@x [32] m.595 l.11860 - Omega
@!dvi_buf:array[dvi_index] of eight_bits; {buffer for \.{DVI} output}
@y
@!dvi_buf:array[dvi_index] of real_eight_bits; {buffer for \.{DVI} output}
@z
%---------------------------------------
@x [32] m.617 l.12262 - Omega
  print(" TeX output "); print_int(year); print_char(".");
@y
  print("Omega output, Version 3.14159--1.15, ");
  print_int(year); print_char(".");
@z
%---------------------------------------
@x [32] m.617 l.12267 - Omega
  for s:=str_start[str_ptr] to pool_ptr-1 do dvi_out(so(str_pool[s]));
  pool_ptr:=str_start[str_ptr]; {flush the current string}
@y
  for s:=str_start(str_ptr) to pool_ptr-1 do dvi_out(so(str_pool[s]));
  pool_ptr:=str_start(str_ptr); {flush the current string}
@z
%---------------------------------------
@x [36] m.765 l.15021 - Omega
magic_offset:=str_start[math_spacing]-9*ord_noad
@y
magic_offset:=str_start(math_spacing)-9*ord_noad
@z
%---------------------------------------
@x [42] m.931 l.18172 - Omega
  begin j:=1; u:=str_start[k];
@y
  begin j:=1; u:=str_start(k);
@z
%---------------------------------------
@x [42] m.940 l.18296 - Omega
u:=str_start[k]; v:=str_start[s];
repeat if str_pool[u]<str_pool[v] then goto found;
if str_pool[u]>str_pool[v] then goto not_found;
incr(u); incr(v);
until u=str_start[k+1];
@y
u:=str_start(k); v:=str_start(s);
repeat if str_pool[u]<str_pool[v] then goto found;
if str_pool[u]>str_pool[v] then goto not_found;
incr(u); incr(v);
until u=str_start(k+1);
@z
%---------------------------------------
@x [49] m.1252 l.23232 - Omega
    print_err("Patterns can be loaded only by INITEX");
@y
    print_err("Patterns can be loaded only by INIOMEGA");
@z
%---------------------------------------
@x [50] m.1301 l.23682 - Omega
format_ident:=" (INITEX)";
@y
format_ident:=" (INIOMEGA)";
@z
%---------------------------------------
% We do not store any information for strings between 257 and 65535.

@x [50] m.1309 l.23814 - Omega
for k:=0 to str_ptr do dump_int(str_start[k]);
@y
for k:=too_big_char to str_ptr do dump_int(str_start(k));
@z
%---------------------------------------
% We recreate the information for strings between 257 and 65535.

@x [50] m.1310 l.23831 - Omega
for k:=0 to str_ptr do undump(0)(pool_ptr)(str_start[k]);
@y
for k:=too_big_char to str_ptr do undump(0)(pool_ptr)(str_start(k));
@z
%---------------------------------------
@x [50] m.1328 l.24125 - Omega - Year 2000
print_int(year mod 100); print_char(".");
@y
print_int(year); print_char(".");
@z
%---------------------------------------
@x [51] m.1334 l.24266 - Omega
  wlog_ln('Here is how much of TeX''s memory',' you used:');
@y
  wlog_ln('Here is how much of Omega''s memory',' you used:');
@z
%---------------------------------------
@x [51] m.1335 l.24338 - Omega
  print_nl("(\dump is performed only by INITEX)"); return;
@:dump_}{\.{\\dump...only by INITEX}@>
@y
  print_nl("(\dump is performed only by INIOMEGA)"); return;
@:dump_}{\.{\\dump...only by INIOMEGA}@>
@z
%---------------------------------------
@x [53] m.1368 l.24747 - Omega
for k:=str_start[str_ptr] to pool_ptr-1 do dvi_out(so(str_pool[k]));
pool_ptr:=str_start[str_ptr]; {erase the string}
@y
for k:=str_start(str_ptr) to pool_ptr-1 do dvi_out(so(str_pool[k]));
pool_ptr:=str_start(str_ptr); {erase the string}
@z
%---------------------------------------
