% Change file for BibTeX in C, originally by Howard Trickey.
%
% 05/28/84      Initial implementation, version 0.41 of BibTeX
% 07/01/84      Version 0.41a of BibTeX.
% 12/17/84      Version 0.97c of BibTeX.
% 02/12/85      Version 0.98c of BibTeX.
% 02/25/85      Newer version 0.98c of BibTeX.
% 03/25/85      Version 0.98f of BibTeX
% 05/23/85      Version 0.98i of BibTeX
% 02/11/88      Version 0.99b of BibTeX
% 04/04/88      Version 0.99c; converted for use with web2c (ETM).
% 11/30/89      Use FILENAMESIZE instead of 1024 (KB).
% 03/09/90	`int' is a bad variable name for C.
% (more recent changes in the ChangeLog)
%
%  5/ 2/89      Version 0.20 of JBiBTeX by Shouichi Matsui
% 12/25/90      Version 0.30 of JBibTeX by Shouichi Matsui
%  1/ 1/91      last update for JBibTeX 0.30 by Shouichi Matsui
% 10/30/92      last update for JBibTeX 0.31 for bug fix by Shouichi Matsui
% 11/02/94      Version 0.32 for use with web2c 6.1, by Takafumi Sakurai
%
% 2002          Version 0.33 add kanji option by ASCII Corporation
%
% 2009          pTeXenc, pbibtex N. Tsuchimura
% 2010          Version 0.99d of BibTeX for TeX Live
%
% 2022-02-08    Version 0.34 by H. Yamashita
%   * Avoid breaking BBL lines at white space after a Japanese character, to
%     preserve spacing within BIB entry for subsequent pTeX line-end operations.
% 2022-02-20    Still version 0.34 by H. Yamashita (-> TL'22 version)
%   * Improve substring$ to truncate at least one character when trying to
%     start counting from the middle byte of the first or last Japanese character.
% 2022-05-15    Version 0.35 by Takuji Tanaka
%   * Accept multibyte characters by int.to.chr$ and chr.to.int$.
% 2022-12-03    Version 0.36 by Takuji Tanaka (-> TL'23 version)
%   * Merge pBibTeX source/binary into upBibTeX.

@x [0] only print chnages
\def\title{\BibTeX\ }
@y
\let\maybe=\iffalse
\def\title{J\BibTeX\ 0.36 Changes for C Version \BibTeX\ }
@z

@x
 \def\titlepage{F}
 \centerline{\:\titlefont The {\:\ttitlefont \BibTeX} preprocessor}
 \vskip 15pt \centerline{(Version 0.99d---\today)} \vfill}
@y
 \def\titlepage{F}
 \centerline{\:\titlefont The {\:\ttitlefont J\BibTeX} preprocessor}
 \vskip 15pt \centerline{(Version 0.99d-j0.36---\today)} \vfill}
@z

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% [1] banner
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
@x
@d my_name=='bibtex'
@d banner=='This is BibTeX, Version 0.99d' {printed when the program starts}
@y
@d my_name=='upbibtex'
@d banner=='This is upBibTeX, Version 0.99d-j0.36-u1.29'
  {printed when the program starts}
@z

@x
  print (banner);
@y
  print (banner);
  print (' (', conststringcast(get_enc_string), ')');
@z

@x Changes for JBibTeX by Shouichi Matsui [27]
for i:=0 to @'37 do xchr[i]:=chr(i);
for i:=@'177 to @'377 do xchr[i]:=chr(i);
@y
for i:=1 to @'37 do xchr[i]:=' ';
xchr[tab]:=chr(tab);
for i:=@'200 to 255 do xchr[i]:=chr(i);
xchr[@'33]:=chr(@'33); {escape char}
@z

@x Changes for JBibTeX by Shouichi Matsui [28]
for i:=first_text_char to last_text_char do xord[xchr[i]]:=i;
@y
for i:=first_text_char to last_text_char do xord[i]:=invalid_code;
for i:=1 to @'176 do xord[i]:= i;
for i:=@'200 to 255 do xord[i]:=i;
@z


@x Changes for JBibTeX by Shouichi Matsui  for ver. 0.30
@d period = "."                 {these are three}
@d question_mark = "?"          {string-ending characters}
@d exclamation_mark = "!"       {of interest in \.{add.period\$}}
@y
@d period = "."                 {these are three}
@d question_mark = "?"          {string-ending characters}
@d exclamation_mark = "!"       {of interest in \.{add.period\$}}
@d e_ss3 = @"8F                 {single shift three in EUC}
@d e_pun_first = @"A1           {Zenkaku punctuation first byte; in EUC}
@d e_space = @"A1A1             {Zenkaku space; in EUC}
@d e_toten = @"A1A2             {Zenkaku kuten; in EUC}
@d e_kuten = @"A1A3             {Zenkaku toten; in EUC}
@d e_comma = @"A1A4             {Zenkaku comman; in EUC}
@d e_period = @"A1A5            {Zenkaku period; in EUC}
@d e_question = @"A1A9          {Zenkaku question mark; in EUC}
@d e_exclamation =@"A1AA        {Zenkaku exclamation mark; in EUC}
@d u_pun_first1 = @"E3          {Zenkaku punctuation first byte(1); in UTF-8}
@d u_pun_first2 = @"EF          {Zenkaku punctuation first byte(2); in UTF-8}
@d u_space = @"3000             {Zenkaku space; in UCS}
@d u_toten = @"3001             {Zenkaku toten; in UCS}
@d u_kuten = @"3002             {Zenkaku kuten; in UCS}
@d u_comma = @"FF0C             {Zenkaku comman; in UCS}
@d u_period = @"FF0E            {Zenkaku period; in UCS}
@d u_question = @"FF1F          {Zenkaku question mark; in UCS}
@d u_exclamation = @"FF01       {Zenkaku exclamation mark; in UCS}
@d u_double_question = @"2047   {Zenkaku double question mark; in UCS}
@d u_double_exclam   = @"203C   {Zenkaku double exclamation mark; in UCS}
@d u_interrobang     = @"203D   {Zenkaku interrobang; in UCS}
@d u_question_exclam = @"2048   {Zenkaku question exclamation mark; in UCS}
@d u_exclam_question = @"2049   {Zenkaku exclamation question mark; in UCS}
@z

% [32] Make RET a `white_space' character, so we won't choke on DOS
% files, which use CR/LF for line endings.
@x Changes for JBibTeX by Shouichi Matsui [32]
@<Set initial values of key variables@>=
for i:=0 to @'177 do lex_class[i] := other_lex;
for i:=@'200 to @'377 do lex_class[i] := alpha;
for i:=0 to @'37 do lex_class[i] := illegal;
lex_class[invalid_code] := illegal;
lex_class[tab] := white_space;
lex_class[13] := white_space;
lex_class[space] := white_space;
lex_class[tie] := sep_char;
lex_class[hyphen] := sep_char;
for i:=@'60 to @'71 do lex_class[i] := numeric;
for i:=@'101 to @'132 do lex_class[i] := alpha;
for i:=@'141 to @'172 do lex_class[i] := alpha;
@y
@<Set initial values of key variables@>=
for i:=0 to @'377 do lex_class[i] := other_lex;
for i:=0 to @'37 do lex_class[i] := illegal;
lex_class[invalid_code] := illegal;
lex_class[tab] := white_space;
lex_class[13] := white_space;
lex_class[space] := white_space;
lex_class[tie] := sep_char;
lex_class[hyphen] := sep_char;
for i:=@'60 to @'71 do lex_class[i] := numeric;
for i:=@'101 to @'132 do lex_class[i] := alpha;
for i:=@'141 to @'172 do lex_class[i] := alpha;
@z

@x Changes for JBibTeX by Shouichi Matsui [33]
@<Set initial values of key variables@>=
for i:=0 to @'377 do id_class[i] := legal_id_char;
for i:=0 to @'37 do id_class[i] := illegal_id_char;
id_class[space] := illegal_id_char;
id_class[tab] := illegal_id_char;
id_class[double_quote] := illegal_id_char;
id_class[number_sign] := illegal_id_char;
id_class[comment] := illegal_id_char;
id_class[single_quote] := illegal_id_char;
id_class[left_paren] := illegal_id_char;
id_class[right_paren] := illegal_id_char;
id_class[comma] := illegal_id_char;
id_class[equals_sign] := illegal_id_char;
id_class[left_brace] := illegal_id_char;
id_class[right_brace] := illegal_id_char;
@y
@<Set initial values of key variables@>=
for i:=0 to @'377 do id_class[i] := legal_id_char;
for i:=0 to @'37 do id_class[i] := illegal_id_char;
id_class[@'33] := legal_id_char;
id_class[space] := illegal_id_char;
id_class[tab] := illegal_id_char;
id_class[double_quote] := illegal_id_char;
id_class[number_sign] := illegal_id_char;
id_class[comment] := illegal_id_char;
id_class[single_quote] := illegal_id_char;
id_class[left_paren] := illegal_id_char;
id_class[right_paren] := illegal_id_char;
id_class[comma] := illegal_id_char;
id_class[equals_sign] := illegal_id_char;
id_class[left_brace] := illegal_id_char;
id_class[right_brace] := illegal_id_char;
@z

@x Changes for JBibTeX by Shouichi Matsui [35]
char_width[@'167] := 722;
char_width[@'170] := 528;
char_width[@'171] := 528;
char_width[@'172] := 444;
char_width[@'173] := 500;
char_width[@'174] :=1000;
char_width[@'175] := 500;
char_width[@'176] := 500;
@y
char_width[@'167] := 722;
char_width[@'170] := 528;
char_width[@'171] := 528;
char_width[@'172] := 444;
char_width[@'173] := 500;
char_width[@'174] :=1000;
char_width[@'175] := 500;
char_width[@'176] := 500;
@z

@x [48] JBibTeX and dynamic buf_size.
label loop_exit;
@y
label loop_exit;
var i:c_int_type;
@z
@x [still 48] JBibTeX and dynamic buf_size.
  while (not eoln(f)) do
    begin
    if (last >= buf_size) then
        buffer_overflow;
    buffer[last] := xord[getc (f)];
    incr (last);
    end;
  vgetc (f); {skip the eol}
@y
  last := input_line2(f,ustringcast(buffer),last,buf_size,address_of(i));
  while (not eof(f)) and (last > 0) and (i <> 10) and (i <> 13) do
  begin
    buffer_overflow;
    last := input_line2(f,ustringcast(buffer),last,buf_size,address_of(i));
  end;
@z

@x procedure lower_case
procedure lower_case (var buf:buf_type; @!bf_ptr,@!len:buf_pointer);
var i:buf_pointer;
begin
if (len > 0) then
  for i := bf_ptr to bf_ptr+len-1 do
    if ((buf[i]>="A") and (buf[i]<="Z")) then
        buf[i] := buf[i] + case_difference;
@y
procedure lower_case (var buf:buf_type; @!bf_ptr,@!len:buf_pointer);
var i:buf_pointer;
    @!ch:integer;
begin
if (len > 0) then
  for i := bf_ptr to bf_ptr+len-1 do
    if ((buf[i]>="A") and (buf[i]<="Z")) then begin
        buf[i] := buf[i] + case_difference;
    end
    else if ((is_internalUPTEX) and (buf[i]>=@"C3) and (buf[i]<=@"D4)) then begin
        ch := fromBUFF(buf,i+2,i);
        if (((ch>=@"C0) and (ch<=@"DE) and (ch<>@"D7)) or
            ((ch>=@"391) and (ch<=@"3AA) and (ch<>@"3A2)) or
            ((ch>=@"410) and (ch<=@"42F))) then begin
            ch := toBUFF(ch + case_difference);
            buf[i]   := BYTE3(ch);
            buf[i+1] := BYTE4(ch);
        end
        else if (((ch>=@"100) and (ch<=@"137) and ((ch mod 2)=0)) or
                 ((ch>=@"139) and (ch<=@"148) and ((ch mod 2)=1)) or
                 ((ch>=@"14A) and (ch<=@"177) and ((ch mod 2)=0)) or
                 ((ch>=@"179) and (ch<=@"17E) and ((ch mod 2)=1)) or
                 ((ch>=@"370) and (ch<=@"373) and ((ch mod 2)=0)) or
                 ( ch=@"376 ) or
                 ((ch>=@"3D8) and (ch<=@"3EF) and ((ch mod 2)=0)) or
                 ( ch=@"3F7 ) or  ( ch=@"3FA ) or
                 ((ch>=@"460) and (ch<=@"481) and ((ch mod 2)=0)) or
                 ((ch>=@"48A) and (ch<=@"4BF) and ((ch mod 2)=0)) or
                 ((ch>=@"4C1) and (ch<=@"4CE) and ((ch mod 2)=1)) or
                 ((ch>=@"4D0) and (ch<=@"52F) and ((ch mod 2)=0))) then begin
            ch := toBUFF(ch + 1);
            buf[i]   := BYTE3(ch);
            buf[i+1] := BYTE4(ch);
        end
        else if (ch=@"178) then begin
            ch := toBUFF(@"FF);
            buf[i]   := BYTE3(ch);
            buf[i+1] := BYTE4(ch);
        end
        else if ((ch>=@"400) and (ch<=@"40F)) then begin
            ch := toBUFF(ch + @"50);
            buf[i]   := BYTE3(ch);
            buf[i+1] := BYTE4(ch);
        end
        else if (ch=@"4C0) then begin
            ch := toBUFF(@"4CF);
            buf[i]   := BYTE3(ch);
            buf[i+1] := BYTE4(ch);
        end
    end
@z

@x procedure upper_case
var i:buf_pointer;
begin
if (len > 0) then
  for i := bf_ptr to bf_ptr+len-1 do
    if ((buf[i]>="a") and (buf[i]<="z")) then
        buf[i] := buf[i] - case_difference;
@y
var i:buf_pointer;
    @!ch:integer;
begin
if (len > 0) then
  for i := bf_ptr to bf_ptr+len-1 do
    if ((buf[i]>="a") and (buf[i]<="z")) then begin
        buf[i] := buf[i] - case_difference;
    end
    else if ((is_internalUPTEX) and (buf[i]>=@"C3) and (buf[i]<=@"D4)) then begin
        ch := fromBUFF(buf,i+2,i);
        if (((ch>=@"E0) and (ch<=@"FE) and (ch<>@"F7)) or
            ((ch>=@"3B1) and (ch<=@"3CA) and (ch<>@"3C2)) or
            ((ch>=@"430) and (ch<=@"44F))) then begin
            ch := toBUFF(ch - case_difference);
            buf[i]   := BYTE3(ch);
            buf[i+1] := BYTE4(ch);
        end
        else if (((ch>=@"100) and (ch<=@"137) and ((ch mod 2)=1)) or
                 ((ch>=@"139) and (ch<=@"148) and ((ch mod 2)=0)) or
                 ((ch>=@"14A) and (ch<=@"177) and ((ch mod 2)=1)) or
                 ((ch>=@"179) and (ch<=@"17E) and ((ch mod 2)=0)) or
                 ((ch>=@"370) and (ch<=@"373) and ((ch mod 2)=1)) or
                 ( ch=@"377 ) or
                 ((ch>=@"3D8) and (ch<=@"3EF) and ((ch mod 2)=1)) or
                 ( ch=@"3F8 ) or  ( ch=@"3FB ) or
                 ((ch>=@"460) and (ch<=@"481) and ((ch mod 2)=1)) or
                 ((ch>=@"48A) and (ch<=@"4BF) and ((ch mod 2)=1)) or
                 ((ch>=@"4C1) and (ch<=@"4CE) and ((ch mod 2)=0)) or
                 ((ch>=@"4D0) and (ch<=@"52F) and ((ch mod 2)=1))) then begin
            ch := toBUFF(ch - 1);
            buf[i]   := BYTE3(ch);
            buf[i+1] := BYTE4(ch);
        end
        else if (ch=@"DF) then begin { Latin Small Letter Sharp S }
            buf[i]   := "S";
            buf[i+1] := "S";
        end
        else if (ch=@"FF) then begin
            ch := toBUFF(@"178);
            buf[i]   := BYTE3(ch);
            buf[i+1] := BYTE4(ch);
        end
        else if ((ch>=@"450) and (ch<=@"45F)) then begin
            ch := toBUFF(ch - @"50);
            buf[i]   := BYTE3(ch);
            buf[i+1] := BYTE4(ch);
        end
        else if (ch=@"4CF) then begin
            ch := toBUFF(@"4C0);
            buf[i]   := BYTE3(ch);
            buf[i+1] := BYTE4(ch);
        end
    end
@z

@x
@<Procedures and functions for the reading and processing of input files@>=
procedure get_the_top_level_aux_file_name;
label aux_found,@!aux_not_found;
begin
@y
@<Procedures and functions for the reading and processing of input files@>=
procedure get_the_top_level_aux_file_name;
label aux_found,@!aux_not_found;
var i:0..last_text_char;    {this is the first one declared}
begin
  @<Initialize variables depending on Kanji code@>
@z

@x
parse_arguments;
@y
init_kanji;
parse_arguments;
@z

% pBibTeX: do not break at |white_space| after Japanese characters (2022-02-08 j0.34)
@x "Break that line"
while ((lex_class[out_buf[out_buf_ptr]] <> white_space) and
                                        (out_buf_ptr >= min_print_line)) do
    decr(out_buf_ptr);
@y
while (((lex_class[out_buf[out_buf_ptr]] <> white_space) or
       (out_buf[out_buf_ptr-1] > 127)) and (out_buf_ptr >= min_print_line)) do
    decr(out_buf_ptr);
@z
@x "Break that unbreakably long line"
    if (lex_class[out_buf[out_buf_ptr]] <> white_space) then
        incr(out_buf_ptr)
@y
    if (lex_class[out_buf[out_buf_ptr]] <> white_space) or
      (out_buf[out_buf_ptr-1] > 127) then
        incr(out_buf_ptr)
@z

@x Changes for JBibTeX by Shouichi Matsui [332]
@!b_write : hash_loc;           {\.{write\$}}
@!b_default : hash_loc;         {either \.{skip\$} or \.{default.type}}
@y
@!b_write : hash_loc;           {\.{write\$}}
@!b_is_kanji_str : hash_loc;    {\.{is.kanji.str\$}}
@!b_default : hash_loc;         {either \.{skip\$} or \.{default.type}}
@z

@x Changes for JBibTeX by Shouichi Matsui [334]
@d n_write = 36         {\.{write\$}}

@<Constants in the outer block@>=
@!num_blt_in_fns = 37;  {one more than the previous number}
@y
@d n_write = 36         {\.{write\$}}
@d n_is_kanji_str = 37  {\.{is.kanji.str\$}}

@<Constants in the outer block@>=
@!num_blt_in_fns = 38;  {one more than the previous number}
@z

@x Changes for JBibTeX by Shouichi Matsui [335]
build_in('write$      ',6,b_write,n_write);
@y
build_in('write$      ',6,b_write,n_write);
build_in('is.kanji.str$',13,b_is_kanji_str,n_is_kanji_str);
@z

@x Changes for JBibTeX by Shouichi Matsui [342]
    n_write :           x_write;
    othercases confusion ('Unknown built-in function')
endcases;
end
@y
    n_write :           x_write;
    n_is_kanji_str:     x_is_kanji_str;
    othercases confusion ('Unknown built-in function')
endcases;
end
@z

@x Changes for JBibTeX by Shouichi Matsui [343]
@<|execute_fn|({\.{write\$}})@>@;
@<|execute_fn| itself@>
@y
@<|execute_fn|({\.{write\$}})@>@;
@<|execute_fn|({\.{is.kanji.str\$}})@>@;
@<|execute_fn| itself@>
@z

@x Changes for JBibTeX by Shouichi Matsui   --- add.period$ for ver. 0.30
case (str_pool[sp_ptr]) of
    period,
    question_mark,
    exclamation_mark :
        repush_string;
    othercases
        @<Add the |period| (it's necessary) and push@>
@y
case (str_pool[sp_ptr]) of
    period,
    question_mark,
    exclamation_mark :
        repush_string;
    othercases
        begin
        if (is_internalEUC) then
            begin
            case (fromBUFF(str_pool, sp_ptr+1, sp_ptr-1)) of
                e_kuten,
                e_period,
                e_question,
                e_exclamation:
                    if (str_pool[sp_ptr-2]<>e_ss3) then
                       repush_string
                    else
                       @<Add the |period| (it's necessary) and push@>;
                othercases
                    @<Add the |period| (it's necessary) and push@>;
            endcases;
            end;
        if (is_internalUPTEX) then
            begin
            case (fromBUFF(str_pool, sp_ptr+1, sp_ptr-2)) of
                u_kuten,
                u_period,
                u_question,
                u_exclamation,
                u_double_question,
                u_double_exclam,
                u_interrobang,
                u_question_exclam,
                u_exclam_question:
                    repush_string;
                othercases
                    @<Add the |period| (it's necessary) and push@>;
            endcases;
            end;
        end;
@z

@x x_chr_to_int
else if (length(pop_lit1) <> 1) then
    begin
    print ('"');
    print_pool_str (pop_lit1);
    bst_ex_warn ('" isn''t a single character');
    push_lit_stk (0, stk_int);
    end
else
    push_lit_stk (str_pool[str_start[pop_lit1]], stk_int);
                                        {push the (|ASCII_code|) integer}
@y
else if (length(pop_lit1) = multibytelen(str_pool[str_start[pop_lit1]])) then
    begin
    if (length(pop_lit1) = 1) then
        push_lit_stk(str_pool[str_start[pop_lit1]],stk_int)
                                        {push the (|ASCII_code|) integer}
    else
        push_lit_stk(toDVI(fromBUFF(str_pool, str_start[pop_lit1]+length(pop_lit1), str_start[pop_lit1])),stk_int)
                                        { a KANJI char is |2..4|byte long }
    end
else
    begin
    print ('"');
    print_pool_str (pop_lit1);
    bst_ex_warn ('" isn''t a single character');
    push_lit_stk (0, stk_int);
    end
@z

@x by Shouichi Matsui for Zenkaku comma
if (ex_buf_ptr < ex_buf_length) then            {remove the ``and''}
    ex_buf_ptr := ex_buf_ptr - 4;
@y
if (ex_buf_ptr < ex_buf_length) then    {remove the ``and'', or Zenkau comma}
  begin
    if (is_internalEUC) then
      if((fromBUFF(ex_buf, ex_buf_length, ex_buf_ptr-2) = e_comma) or
         (fromBUFF(ex_buf, ex_buf_length, ex_buf_ptr-2) = e_toten))
      then ex_buf_ptr := ex_buf_ptr - 2
      else ex_buf_ptr := ex_buf_ptr - 4;
    if (is_internalUPTEX) then
      if((fromBUFF(ex_buf, ex_buf_length, ex_buf_ptr-3) = u_comma) or
         (fromBUFF(ex_buf, ex_buf_length, ex_buf_ptr-3) = u_toten))
      then ex_buf_ptr := ex_buf_ptr - 3
      else ex_buf_ptr := ex_buf_ptr - 4;
  end;
@z

@x Changes for JBibTeX by Shouichi Matsui for Zenkaku comma
    "a", "A" :
        begin
        incr(ex_buf_ptr);
        if (preceding_white) then
            @<See if we have an ``and''@>;      {if so, |and_found := true|}
        preceding_white := false;
        end;
@y
    "a", "A" :
        begin
        incr(ex_buf_ptr);
        if (preceding_white) then
            @<See if we have an ``and''@>;      {if so, |and_found := true|}
        preceding_white := false;
        end;
     e_pun_first:
        if (is_internalEUC) then
        begin
          if ((fromBUFF(ex_buf, ex_buf_length, ex_buf_ptr) = e_comma) or 
              (fromBUFF(ex_buf, ex_buf_length, ex_buf_ptr) = e_toten)) then
                begin
                  preceding_white := false;
                  and_found  := true
                end
          else if (fromBUFF(ex_buf,ex_buf_length,ex_buf_ptr) = e_space) then
               begin
                  ex_buf[ex_buf_ptr]   := space;
                  ex_buf[ex_buf_ptr+1] := space;
                  preceding_white := true;
               end;
          ex_buf_ptr := ex_buf_ptr + 2;
        end else begin
            ex_buf_ptr := ex_buf_ptr + multibytelen(ex_buf[ex_buf_ptr]);
            preceding_white := false;
        end;
     u_pun_first1,
     u_pun_first2:
        if (is_internalUPTEX) then
        begin
          if ((fromBUFF(ex_buf, ex_buf_length, ex_buf_ptr) = u_comma) or 
              (fromBUFF(ex_buf, ex_buf_length, ex_buf_ptr) = u_toten)) then
                begin
                  preceding_white := false;
                  and_found  := true
                end
          else if (fromBUFF(ex_buf,ex_buf_length,ex_buf_ptr) = u_space) then
               begin
                  ex_buf[ex_buf_ptr]   := space;
                  ex_buf[ex_buf_ptr+1] := space;
                  ex_buf[ex_buf_ptr+2] := space;
                  preceding_white := true;
               end;
          ex_buf_ptr := ex_buf_ptr + 3;
        end else begin
            ex_buf_ptr := ex_buf_ptr + multibytelen(ex_buf[ex_buf_ptr]);
            preceding_white := false;
        end;
@z

@x Changes for JBibTeX by Shouichi Matsui for Zenkaku comma[385]
    othercases
        if (lex_class[ex_buf[ex_buf_ptr]] = white_space) then
            begin
            incr(ex_buf_ptr);
            preceding_white := true;
            end
        else
            begin
            incr(ex_buf_ptr);
            preceding_white := false;
            end
  endcases;
check_brace_level (pop_lit_var);
end;
@y
    othercases
        if (lex_class[ex_buf[ex_buf_ptr]] = white_space) then
            begin
            incr(ex_buf_ptr);
            preceding_white := true;
            end
        else
            begin
            ex_buf_ptr := ex_buf_ptr + multibytelen(ex_buf[ex_buf_ptr]);
            preceding_white := false;
            end
  endcases;
check_brace_level (pop_lit_var);
end;
@z

@x Changes for JBibTeX by Shouichi Matsui [415]
    if (lex_class[name_buf[name_bf_ptr]] = alpha) then
        begin
        append_ex_buf_char_and_check (name_buf[name_bf_ptr]);
        goto loop_exit;
        end
    else if ((name_buf[name_bf_ptr] = left_brace) and
@y
    if (lex_class[name_buf[name_bf_ptr]] = alpha) then
        begin
        append_ex_buf_char_and_check (name_buf[name_bf_ptr]);
        if multibytelen(name_buf[name_bf_ptr]) > 1 then
            append_ex_buf_char_and_check (name_buf[name_bf_ptr+1]);
        if multibytelen(name_buf[name_bf_ptr]) > 2 then
            append_ex_buf_char_and_check (name_buf[name_bf_ptr+2]);
        if multibytelen(name_buf[name_bf_ptr]) > 3 then
            append_ex_buf_char_and_check (name_buf[name_bf_ptr+3]);
        name_bf_ptr := name_bf_ptr + multibytelen(name_buf[name_bf_ptr])-1;
        goto loop_exit;
        end
    else if ((name_buf[name_bf_ptr] = left_brace) and
@z

@x x_int_to_chr
procedure x_int_to_chr;
begin
pop_lit_stk (pop_lit1,pop_typ1);
if (pop_typ1 <> stk_int) then
    begin
    print_wrong_stk_lit (pop_lit1,pop_typ1,stk_int);
    push_lit_stk (s_null, stk_str);
    end
else if ((pop_lit1 < 0) or (pop_lit1 > 127)) then
    begin
    bst_ex_warn (pop_lit1:0,' isn''t valid ASCII');
    push_lit_stk (s_null, stk_str);
    end
else
    begin
    str_room(1);
    append_char (pop_lit1);
    push_lit_stk (make_string, stk_str);
    end;
end;
@y
procedure x_int_to_chr;
var k:integer;
begin
pop_lit_stk (pop_lit1,pop_typ1);
if (pop_typ1 <> stk_int) then
    begin
    print_wrong_stk_lit (pop_lit1,pop_typ1,stk_int);
    push_lit_stk (s_null, stk_str);
    end
else begin
k:=pop_lit1;
if (pop_lit1 > 127) then k:=fromDVI(pop_lit1);
if ((pop_lit1 < 0) or ((pop_lit1 > 127) and (k = 0))) then
    begin
    bst_ex_warn (pop_lit1:0,' isn''t valid character code');
    push_lit_stk (s_null, stk_str);
    end
else
    begin
    str_room(4);
    k:=toBUFF(k);
    if (BYTE1(k)>0) then
        append_char (BYTE1(k));
    if (BYTE2(k)>0) then
        append_char (BYTE2(k));
    if (BYTE3(k)>0) then
        append_char (BYTE3(k));
    { always }
        append_char (BYTE4(k));
    push_lit_stk (make_string, stk_str);
    end;
end;
end;
@z

@x Changes for JBibTeX by Shouichi Matsui [437]
@<|execute_fn|({\.{substring\$}})@>=
procedure x_substring;
label exit;
begin
@y
@<|execute_fn|({\.{substring\$}})@>=
procedure x_substring;
label exit;
var tps,tpe:pool_pointer; {temporary pointer}
@!pop_lit2_saved,@!mbl_tpe: integer;
begin
@z

@x
@<Form the appropriate substring@>=
begin
@y
@<Form the appropriate substring@>=
begin
pop_lit2_saved := pop_lit2; {save before negate}
@z

@x Changes for JBibTeX by Shouichi Matsui [438] + fix (2022-02-20 j0.34)
str_room(sp_end - sp_ptr);
while (sp_ptr < sp_end) do                      {shift the substring}
    begin
    append_char (str_pool[sp_ptr]);
    incr(sp_ptr);
    end;
@y
{ |2..4| bytes Kanji code break check }
tps:=str_start[pop_lit3];
tpe:=tps;
mbl_tpe:=0;
while tpe < str_start[pop_lit3+1] do begin
    if multibytelen(str_pool[tpe])<0 {just in case}
        or (str_start[pop_lit3+1] < tpe+multibytelen(str_pool[tpe])) then
        break;
    mbl_tpe := multibytelen(str_pool[tpe]);
    tpe := tpe + mbl_tpe;
    if tpe<=sp_ptr then
        tps := tpe;
    if sp_end<=tpe then break;
end;
if (pop_lit2_saved > 1) and (tps = str_start[pop_lit3]) then
    if multibytelen(str_pool[tps])>=0 then {just in case}
        tps := tps + multibytelen(str_pool[tps]); {truncate at least one}
if (pop_lit2_saved < -1) and (tpe = str_start[pop_lit3+1]) then
    tpe := tpe - mbl_tpe; {truncate at least one}
if tps > tpe then tpe := tps;
sp_ptr := tps;
sp_end := tpe;

str_room(sp_end - sp_ptr);
while (sp_ptr < sp_end) do                      {shift the substring}
    begin
    append_char (str_pool[sp_ptr]);
    incr(sp_ptr);
    end;
@z

@x Changes for JBibTeX by Shouichi Matsui [445]
    else if (str_pool[sp_xptr1-1] = right_brace) then
        begin
        if (sp_brace_level > 0) then
            decr(sp_brace_level);
        end
    else
        incr(num_text_chars);
    end;
@y
    else if (str_pool[sp_xptr1-1] = right_brace) then
        begin
        if (sp_brace_level > 0) then
            decr(sp_brace_level);
        end
    else if (str_pool[sp_xptr1-1]>127) then
        begin {kanji char}
            num_text_chars := num_text_chars + multibytelen(str_pool[sp_xptr1-1]);
            sp_xptr1 := sp_xptr1-1 + multibytelen(str_pool[sp_xptr1-1]);
        end
    else
        incr(num_text_chars);
    end;
@z

@x
const n_options = 4; {Pascal won't count array lengths for us.}
@y
const n_options = 9; {Pascal won't count array lengths for us.}
@z

@x
      usage_help (BIBTEX_HELP, nil);
@y
      usage_help (UPBIBTEX_HELP, 'issue@@texjp.org');
@z

@x
    end; {Else it was a flag; |getopt| has already done the assignment.}
@y
    end else if argument_is ('kanji') then begin
      if (not set_enc_string(optarg, nil)) then
        write_ln('Bad kanji encoding "', stringcast(optarg), '".');

    end else if argument_is ('guess-input-enc') then begin
        enable_guess_file_enc;

    end else if argument_is ('no-guess-input-enc') then begin
        disable_guess_file_enc;

    end else if argument_is ('kanji-internal') then begin
      if (not (set_enc_string(nil,optarg) and
               (is_internalEUC or is_internalUPTEX))) then
        write_ln('Bad internal kanji encoding "', stringcast(optarg), '".');

    end; {Else it was a flag; |getopt| has already done the assignment.}
@z

@x
long_options[current_option].name := 'version';
long_options[current_option].has_arg := 0;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr (current_option);
@y
long_options[current_option].name := 'version';
long_options[current_option].has_arg := 0;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr (current_option);

@ Kanji option.
@.-kanji@>

@<Define the option...@> =
long_options[current_option].name := 'kanji';
long_options[current_option].has_arg := 1;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr(current_option);
long_options[current_option].name := 'guess-input-enc';
long_options[current_option].has_arg := 0;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr(current_option);
long_options[current_option].name := 'no-guess-input-enc';
long_options[current_option].has_arg := 0;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr(current_option);
@z

@x
@ An element with all zeros always ends the list.
@y
@ Kanji-internal option.
@.-kanji-internal@>

@<Define the option...@> =
long_options[current_option].name := 'kanji-internal';
long_options[current_option].has_arg := 1;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr(current_option);

@ An element with all zeros always ends the list.
@z

@x
begin kpse_set_program_name (argv[0], 'bibtex');
@y
begin kpse_set_program_name (argv[0], 'upbibtex');
@z

@x
  until j_prime;
  incr (k);
  hash_prime := j;
  primes[k] := hash_prime;
  end;
end;

@y
  until j_prime;
  incr (k);
  hash_prime := j;
  primes[k] := hash_prime;
  end;
end;

@ modules for JBibTeX

@<|execute_fn|({\.{is.kanji.str\$}})@>=
procedure x_is_kanji_str;
label exit;
var ctmp,clen:integer;
begin
    pop_lit_stk(pop_lit1, pop_typ1);
    if pop_typ1<> stk_str then
    begin print_wrong_stk_lit(pop_lit1,pop_typ1,stk_str);
          push_lit_stk(0,stk_int);
    end else begin
        sp_ptr := str_start[pop_lit1];
        sp_end := str_start[pop_lit1+1];
        while sp_ptr<sp_end do begin
            clen := multibytelen(str_pool[sp_ptr]);
            if sp_ptr+clen<=sp_end then
                ctmp := fromBUFF(str_pool, sp_ptr+clen, sp_ptr)
            else
                ctmp := str_pool[sp_ptr];
            if is_char_kanji_upbibtex(ctmp) then begin
                push_lit_stk(1,stk_int);
                return;
            end else begin
                if sp_ptr+clen<=sp_end then
                    sp_ptr := sp_ptr + clen
                else
                    incr(sp_ptr);
            end;
        end;
        push_lit_stk(0,stk_int);
    end;
exit:end;

@ @<Procedures and functions for handling numbers, characters, and strings@>=
function is_char_kanji_upbibtex(@!c:integer):boolean;
label exit;
var k:integer;
begin
  { based on upTeX-1.29 kcatcode status: 16,17,19->true / 15,18->false }
  is_char_kanji_upbibtex := true;
  if (is_internalUPTEX) then begin { should be in sync with |kcat_code| of uptex-m.ch }
    k := kcatcodekey(c);
    if k=@"25 then return { Hangul Jamo }
    else if (k>=@"68)and(k<=@"69) then return { CJK Radicals Supplement, Kangxi Radicals }
    else if (k>=@"6C)and(k<=@"6D) then return { Hiragana, Katakana }
    else if k=@"6E then return { Bopomofo }
    else if k=@"6F then return { Hangul Compatibility Jamo }
    else if (k>=@"70)and(k<=@"72) then return { Kanbun .. CJK Strokes }
    else if k=@"73 then return { Katakana Phonetic Extensions }
    else if k=@"76 then return { CJK Unified Ideographs Extension A }
    else if k=@"78 then return { CJK Unified Ideographs }
    else if k=@"88 then return { Hangul Jamo Extended-A }
    else if k=@"93 then return { Hangul Syllables }
    else if k=@"94 then return { Hangul Jamo Extended-B }
    else if k=@"99 then return { CJK Compatibility Ideographs }
    else if (k>=@"10D)and(k<=@"110) then return { Kana Extended-B .. Small Kana Extension }
    else if (k>=@"13B)and(k<=@"142) then return { CJK Unified Ideographs Extension B .. H }
    else if k=@"1FE then return { Fullwidth digit and latin alphabet }
    else if k=@"1FF then return; { Halfwidth katakana }
    end
  else { is_internalEUC }
    if is_char_kanji(c) then return;
  is_char_kanji_upbibtex := false;
exit:end;

@ @<Initialize variables depending on Kanji code@>=
if (is_internalUPTEX) then
  begin
    for i:=@"80 to @"BF do lex_class[i] := alpha; { trail bytes }
    for i:=@"C0 to @"C1 do lex_class[i] := illegal;
    for i:=@"C2 to @"F4 do lex_class[i] := alpha; { lead bytes }
    for i:=@"F5 to @"FF do lex_class[i] := illegal;
    for i:=@"C0 to @"C1 do id_class[i] := illegal_id_char;
    for i:=@"F5 to @"FF do id_class[i] := illegal_id_char;
    for i:=@"80 to @"BF do char_width[i]:=257; { trail bytes }
    for i:=@"C2 to @"DF do char_width[i]:=771; { lead bytes (2bytes) }
    for i:=@"E0 to @"EF do char_width[i]:=514; { lead bytes (3bytes) }
    for i:=@"F0 to @"F4 do char_width[i]:=257; { lead bytes (4bytes) }
  end
else
  begin { is_internalEUC }
    for i:=@'200 to @'240 do lex_class[i] := illegal;
    for i:=@'241 to 254 do lex_class[i] := alpha;
    lex_class[255]:=illegal;
    for i:=@'200 to @'240 do id_class[i] := illegal_id_char;
    id_class[255]:=illegal_id_char;
    lex_class[@'33]:=alpha;
    lex_class[e_ss3]:=alpha;
    for i:=@'241 to 254 do char_width[i]:=514;
    char_width[e_ss3]:=0;
  end;
@z
