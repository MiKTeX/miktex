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
% 2022-05-15    Version 0.35 by Takuji Tanaka (-> TL'23 version)
%   * Accept multibyte characters by int.to.chr$ and chr.to.int$.

@x [0] only print chnages
\def\title{\BibTeX\ }
@y
\let\maybe=\iffalse
\def\title{J\BibTeX\ 0.35 Changes for C Version \BibTeX\ }
@z

@x
 \def\titlepage{F}
 \centerline{\:\titlefont The {\:\ttitlefont \BibTeX} preprocessor}
 \vskip 15pt \centerline{(Version 0.99d---\today)} \vfill}
@y
 \def\titlepage{F}
 \centerline{\:\titlefont The {\:\ttitlefont J\BibTeX} preprocessor}
 \vskip 15pt \centerline{(Version 0.99d-j0.35---\today)} \vfill}
@z

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% [1] banner
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
@x
@d my_name=='bibtex'
@d banner=='This is BibTeX, Version 0.99d' {printed when the program starts}
@y
@d my_name=='pbibtex'
@d banner=='This is pBibTeX, Version 0.99d-j0.35'
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
for i:=@'200 to @'237 do xchr[i]:=chr(i-@'200);
for i:=@'240 to 255 do xchr[i]:=chr(i);
xchr[@'33]:=chr(@'33); {escape char}
@z

@x Changes for JBibTeX by Shouichi Matsui [28]
for i:=first_text_char to last_text_char do xord[xchr[i]]:=i;
@y
for i:=first_text_char to last_text_char do xord[i]:=invalid_code;
for i:=1 to @'176 do xord[i]:= i;
for i:=@'200 to @'237 do xord[i]:= i-@'200;
for i:=@'240 to 255 do xord[i]:=i;
@z


@x Changes for JBibTeX by Shouichi Matsui  for ver. 0.30
@d period = "."                 {these are three}
@d question_mark = "?"          {string-ending characters}
@d exclamation_mark = "!"       {of interest in \.{add.period\$}}
@y
@d period = "."                 {these are three}
@d question_mark = "?"          {string-ending characters}
@d exclamation_mark = "!"       {of interest in \.{add.period\$}}
@d zen_pun_first = 161          {Zenkaku punctuation first byte; in EUC}
@d zen_space = 161              {Zenkaku space first, second byte; in EUC}
@d zen_kuten = 162              {Zenkaku kuten second byte; in EUC}
@d zen_ten = 163                {Zenkaku ten second byte; in EUC}
@d zen_comma = 164              {Zenkaku comman second byte; in EUC}
@d zen_period = 165             {Zenkaku period second byte; in EUC}
@d zen_question = 169           {Zenkaku question mark second byte; in EUC}
@d zen_exclamation = 170        {Zenkaku exclamation mark second byte; in EUC}
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
for i:=@'200 to @'237 do lex_class[i] := illegal;
for i:=@'240 to 255 do lex_class[i] := alpha;
lex_class[@'33]:=alpha;
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
for i:=@'200 to @'237 do id_class[i] := illegal_id_char;
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
for i:=@'240 to 254 do char_width[i]:=514;
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

@x
@<Procedures and functions for the reading and processing of input files@>=
procedure get_the_top_level_aux_file_name;
label aux_found,@!aux_not_found;
begin
@y
@<Procedures and functions for the reading and processing of input files@>=
procedure get_the_top_level_aux_file_name;
label aux_found,@!aux_not_found;
begin
  if (not set_enc_string (nil,'EUC')) then uexit(1);
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
    zen_ten,
    zen_period,
    zen_question,
    zen_exclamation:
        if( str_pool[sp_ptr-1] = zen_pun_first ) then
            repush_string
        else
            @<Add the |period| (it's necessary) and push@>;
    othercases
        @<Add the |period| (it's necessary) and push@>
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
        push_lit_stk(toDVI(fromBUFF(str_pool, str_start[pop_lit1]+2, str_start[pop_lit1])),stk_int)
                                        { a KANJI char is 2byte long }
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
    if( (ex_buf[ex_buf_ptr-1]=zen_comma) or (ex_buf[ex_buf_ptr-1]=zen_kuten) )
    then ex_buf_ptr := ex_buf_ptr - 2
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
     zen_pun_first:
        begin
          if((ex_buf[ex_buf_ptr+1]=zen_comma) or
             (ex_buf[ex_buf_ptr+1]=zen_kuten) ) then
                begin
                  preceding_white := false;
                  and_found  := true
                end
          else if(ex_buf[ex_buf_ptr+1]=zen_space) then
               begin
                  ex_buf[ex_buf_ptr]   := space;
                  ex_buf[ex_buf_ptr+1] := space;
                  preceding_white := true;
               end;
          ex_buf_ptr := ex_buf_ptr + 2;
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
                if( ex_buf[ex_buf_ptr] > 127 ) then
                        ex_buf_ptr := ex_buf_ptr +2
                else
                        incr(ex_buf_ptr);
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
            if name_buf[name_bf_ptr]>127 then begin
                append_ex_buf_char_and_check (name_buf[name_bf_ptr]);
                incr(name_bf_ptr);
                append_ex_buf_char_and_check (name_buf[name_bf_ptr]);
            end
            else
                append_ex_buf_char_and_check (name_buf[name_bf_ptr]);
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
    str_room(2);
    if (pop_lit1>127) then begin
        append_char (Hi(k));
        append_char (Lo(k));
    end
    else
        append_char (pop_lit1);
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
@!pop_lit2_saved: integer;
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
{ 2 bytes Kanji code break check }
tps:=str_start[pop_lit3];
tpe:=tps;
while tpe < str_start[pop_lit3+1] do begin
    if str_pool[tpe] > 127 then begin
        if str_start[pop_lit3+1] < tpe+2 then
            break;
        tpe := tpe + 2;
        end
    else begin
        if str_start[pop_lit3+1] < tpe+1 then
            break;
        tpe := tpe + 1;
        end;
    if tpe<=sp_ptr then
        tps := tpe;
    if sp_end<=tpe then break;
end;
if (pop_lit2_saved > 1) and (tps = str_start[pop_lit3])
    then tps := tps + 2; {truncate at least one}
if (pop_lit2_saved < -1) and (tpe = str_start[pop_lit3+1])
    then tpe := tpe - 2; {truncate at least one}
if tps > tpe then tpe := tps;
sp_ptr := tps;
sp_end := tpe;

str_room(sp_end - sp_ptr);
while (sp_ptr < sp_end) do                      {shift the substring}
    begin
    if str_pool[sp_ptr] >127 then begin
         append_char (str_pool[sp_ptr]); incr(sp_ptr);
         append_char (str_pool[sp_ptr]); incr(sp_ptr);
         end
    else begin
         append_char (str_pool[sp_ptr]); incr(sp_ptr);
         end;
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
            incr(sp_xptr1); num_text_chars:=num_text_chars+2;
        end
    else
        incr(num_text_chars);
    end;
@z

@x
const n_options = 4; {Pascal won't count array lengths for us.}
@y
const n_options = 8; {Pascal won't count array lengths for us.}
@z

@x
      usage_help (BIBTEX_HELP, nil);
@y
      usage_help (PBIBTEX_HELP, 'issue@@texjp.org');
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
begin kpse_set_program_name (argv[0], 'bibtex');
@y
begin kpse_set_program_name (argv[0], 'pbibtex');
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
begin
    pop_lit_stk(pop_lit1, pop_typ1);
    if pop_typ1<> stk_str then
    begin print_wrong_stk_lit(pop_lit1,pop_typ1,stk_str);
          push_lit_stk(0,stk_int);
    end else begin
        sp_ptr := str_start[pop_lit1];
        sp_end := str_start[pop_lit1+1];
        while sp_ptr<sp_end do begin
            if str_pool[sp_ptr]>127 then begin
                push_lit_stk(1,stk_int);
                return;
            end else begin
                incr(sp_ptr);
            end;
        end;
        push_lit_stk(0,stk_int);
    end;
exit:end;
@z
