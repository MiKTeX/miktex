%%% bibtex-miktex.ch: WEB change file for MiKTeX-BibTeX
%%% 
%%% Derived from:
%%% web2c/bibtex.ch, originally by Howard Trickey.

% _____________________________________________________________________________
%
% [1.2]
% _____________________________________________________________________________

@x
@d term_out == tty
@d term_in == tty
@y
@d term_out == output
@d term_in == i@&nput
@z

% _____________________________________________________________________________
%
% [1.3]
% _____________________________________________________________________________

@x
@d trace_pr_newline == begin write_ln(log_file); end
@y
@d trace_pr_newline == begin write_ln(log_file); end
@#
@d log_pr(#) == trace_pr(#)
@d log_pr_ln(#) == trace_pr_ln(#)
@d log_pr_newline == trace_pr_newline
@z

% _____________________________________________________________________________
%
% [1.4]
% _____________________________________________________________________________

@x
@d stat == @{           { remove the `|@{|' when keeping statistics }
@d tats == @t@>@}       { remove the `|@}|' when keeping statistics }
@y
@d stat == 
@d tats == 
@z

% _____________________________________________________________________________
%
% [1.9]
% _____________________________________________________________________________

@x
@d close_up_shop=9998           {jump here after fatal errors}
@d exit_program=9999            {jump here if we couldn't even get started}
@y
@d close_up_shop=9998           {jump here after fatal errors}
@d exit_program=9999            {jump here if we couldn't even get started}
@d goto_close_up_shop==c4p_throw(close_up_shop)
@d goto_exit_program==c4p_throw(exit_program)
@z

% _____________________________________________________________________________
%
% [2.10] The main program
% _____________________________________________________________________________

@x
begin
initialize;
print_ln(banner);@/
@y
begin
@<Begin try blocks@>@;
miktex_process_command_line_options;
initialize;
if miktex_get_verbose_flag then begin
  print(banner);
  miktex_print_miktex_banner(output);
  print_newline;
end else begin
  log_pr(banner);
  miktex_print_miktex_banner(log_file);
  log_pr_newline;
end;
log_pr_ln('Capacity: max_strings=', max_strings:1,
  ', hash_size=', hash_size:1,
  ', hash_prime=', hash_prime:1);
@z

@x
close_up_shop:
@<Clean up and leave@>;
exit_program:
@y
close_up_shop:
c4p_end_try_block(close_up_shop);
@<Clean up and leave@>;
exit_program:
c4p_end_try_block(exit_program);
if (history > 1) then begin
  c4p_exit(1);
end;
@z

% _____________________________________________________________________________
%
% [2.12]
% _____________________________________________________________________________

@x
@<Procedures and functions for about everything@>=
@y
@<Procedures and functions for about everything@>=
@<Forward declarations@>@;
@z

% _____________________________________________________________________________
%
% [2.13]
% _____________________________________________________________________________

@x
    goto exit_program;
@y
    goto_exit_program;
@z

% _____________________________________________________________________________
%
% [2.14]
% _____________________________________________________________________________

@x
@<Constants in the outer block@>=
@y
@<Constants in the outer block@>=
@!hash_base = empty + 1;  {lowest numbered hash-table location}
@!quote_next_fn = empty;  {special marker used in defining functions}
@z

@x
@!buf_size=1000; {maximum number of characters in an input line (or string)}
@y
@!buf_size_def=20000;
@!buf_size_max=9999999; {maximum number of characters in an input line (or string)}
@z

@x
@!aux_stack_size=20; {maximum number of simultaneous open \.{.aux} files}
@y
@!aux_stack_size=20; {maximum number of simultaneous open \.{.aux} files}
@z

@x
@!max_bib_files=20; {maximum number of \.{.bib} files allowed}
@y
@!max_bib_files_def=20;
@!max_bib_files_max=9999999; {maximum number of \.{.bib} files allowed}
@z

@x
@!pool_size=65000; {maximum number of characters in strings}
@y
@!pool_size_def=65000;
@!pool_size_max=9999999; {maximum number of characters in strings}
@z

@x
@!max_strings=4000; {maximum number of strings, including pre-defined;
@y
@!max_strings_max=9999999; {maximum number of strings, including pre-defined;
@z

@x
@!max_cites=750; {maximum number of distinct cite keys; must be
@y
@!max_cites_def=750;
@!max_cites_max=9999999; {maximum number of distinct cite keys; must be
@z

@x
@!min_crossrefs=2; {minimum number of cross-refs required for automatic
                                                        |cite_list| inclusion}
@y
@z

@x
@!wiz_fn_space=3000; {maximum amount of |wiz_defined|-function space}
@y
@!wiz_fn_space_def=3000;
@!wiz_fn_space_max=9999999; {maximum amount of |wiz_defined|-function space}
@z

@x
@!single_fn_space=100; {maximum amount for a single |wiz_defined|-function}
@y
@!single_fn_space_def=50;
@!single_fn_space_max=9999999; {maximum amount for a single |wiz_defined|-function}
@z

@x
@!max_ent_ints=3000; {maximum number of |int_entry_var|s
@y
@!max_ent_ints_def=3000;
@!max_ent_ints_max=9999999; {maximum number of |int_entry_var|s
@z

@x
@!max_ent_strs=3000; {maximum number of |str_entry_var|s
@y
@!max_ent_strs_def=3000;
@!max_ent_strs_max=9999999; {maximum number of |str_entry_var|s
@z

@x
@!ent_str_size=100; {maximum size of a |str_entry_var|; must be |<=buf_size|}
@y
@!ent_str_size_max=9999999; {maximum size of a |str_entry_var|; must be |<=buf_size|}
@z

@x
@!glob_str_size=1000; {maximum size of a |str_global_var|;
@y
@!glob_str_size_max=9999999; {maximum size of a |str_global_var|;
@z

@x
@!max_fields=17250; {maximum number of fields (entries $\times$ fields,
@y
@!max_glob_strs_def=10;
@!max_fields_def=5000;
@!max_fields_max=9999999; {maximum number of fields (entries $\times$ fields,
@z

@x
@!lit_stk_size=100; {maximum number of literal functions on the stack}
@y
@!lit_stk_size_def=50;
@!lit_stk_size_max=9999999; {maximum number of literal functions on the stack}
@z

% _____________________________________________________________________________
%
% [2.15]
% _____________________________________________________________________________

@x
@d hash_size=5000       {must be |>= max_strings| and |>= hash_prime|}
@d hash_prime=4253      {a prime number about 85\% of |hash_size| and |>= 128|
                                                and |< @t$2^{14}-2^6$@>|}
@y
@z

@x
@d file_name_size=40    {file names shouldn't be longer than this}
@y
@d file_name_size=259   {file names shouldn't be longer than this}
@d file_name_size_plus_one=260 {one more for the string terminator}
@z

@x
@d max_glob_strs=10     {maximum number of |str_global_var| names}
@d max_glb_str_minus_1 = max_glob_strs-1  {to avoid wasting a |str_global_var|}
@y
@d max_glob_strs_max=9999999    {maximum number of |str_global_var| names}
@d x_entry_strs_tail(#) == (#)]
@d x_entry_strs(#) == entry_strs[(#) * (ent_str_size+1) + x_entry_strs_tail
@d x_global_strs_tail(#) == (#)]
@d x_global_strs(#) == global_strs[(#) * (glob_str_size+1) + x_global_strs_tail
@z

% _____________________________________________________________________________
%
% [2.17]
% _____________________________________________________________________________

@x
if (hash_prime >= (16384-64)) then              bad:=10*bad+6;
@y
if (hash_base <> 1) then                        bad:=10*bad+6;
@z

@x
if (ent_str_size > buf_size) then               bad:=10*bad+9;
if (glob_str_size > buf_size) then              bad:=100*bad+11;
@y
@z

% _____________________________________________________________________________
%
% [3.22]
% _____________________________________________________________________________

@x
@!ASCII_code=0..127;    {seven-bit numbers}
@y
@!ASCII_code=0..255;    {eight-bit numbers}
@z

% _____________________________________________________________________________
%
% [3.24]
% _____________________________________________________________________________

@x
@!xord: array [text_char] of ASCII_code;
@y
@!xord: array [0..255] of ASCII_code;
@z

% _____________________________________________________________________________
%
% [3.27]
% _____________________________________________________________________________

@x
for i:=1 to @'37 do xchr[i]:=' ';
xchr[tab]:=chr(tab);
@y
do_nothing;
@z

% _____________________________________________________________________________
%
% [3.28]
% _____________________________________________________________________________

@x
for i:=first_text_char to last_text_char do xord[chr(i)]:=invalid_code;
for i:=1 to @'176 do xord[xchr[i]]:=i;
@y
miktex_initialize_char_tables;
@z

% _____________________________________________________________________________
%
% [4.37]
% _____________________________________________________________________________

@x
@!name_of_file:packed array[1..file_name_size] of char;
                         {on some systems this is a \&{record} variable}
@y
@!name_of_file:packed array[1..file_name_size_plus_one] of char;
                         {on some systems this is a \&{record} variable}
@z

% _____________________________________________________________________________
%
% [4.38]
% _____________________________________________________________________________

@x
function erstat(var f:file):integer; extern;    {in the runtime library}
@y
@z

@x
begin reset(f,name_of_file,'/O'); a_open_in:=reset_OK(f);
@y
begin a_open_in:=miktex_open_input_file(f);
@z

@x
begin rewrite(f,name_of_file,'/O'); a_open_out:=rewrite_OK(f);
@y
begin a_open_out:=c4p_try_fopen(f,name_of_file,c4p_w_mode);
@z

% _____________________________________________________________________________
%
% [4.39]
% _____________________________________________________________________________

@x
begin close(f);
@y
begin miktex_close_file(f);
@z

% _____________________________________________________________________________
%
% [4.42]
% _____________________________________________________________________________

@x
@!buf_pointer = 0..buf_size;                    {an index into a |buf_type|}
@y
@!buf_pointer = 0..buf_size_max;                {an index into a |buf_type|}
@z

% _____________________________________________________________________________
%
% [4.44]
% _____________________________________________________________________________

@x
                goto close_up_shop;
@y
                goto_close_up_shop;
@z

% _____________________________________________________________________________
%
% [4.45]
% _____________________________________________________________________________

@x
                 goto close_up_shop;
@y
                 goto_close_up_shop;
@z

% _____________________________________________________________________________
%
% [4.46]
% _____________________________________________________________________________

@x
overflow('buffer size ',buf_size);
@y
buf_size := buf_size + buf_size_def;
miktex_bibtex_realloc('buffer', buffer, buf_size);
miktex_bibtex_realloc('ex_buf', ex_buf, buf_size);
miktex_bibtex_realloc('name_sep_char', name_sep_char, buf_size);
miktex_bibtex_realloc('name_tok', name_tok, buf_size);
miktex_bibtex_realloc('out_buf', out_buf, buf_size);
miktex_bibtex_realloc('sv_buffer', sv_buffer, buf_size);
@z

% _____________________________________________________________________________
%
% [4.47]
% _____________________________________________________________________________

@x
function input_ln(var f:alpha_file) : boolean;
                                {inputs the next line or returns |false|}
label loop_exit;
begin
last:=0;
if (eof(f)) then input_ln:=false
else
  begin
  while (not eoln(f)) do
    begin
    if (last >= buf_size) then
        buffer_overflow;
    buffer[last]:=xord[f^];
    get(f); incr(last);
    end;
  get(f);
  while (last > 0) do           {remove trailing |white_space|}
    if (lex_class[buffer[last-1]] = white_space) then
      decr(last)
     else
      goto loop_exit;
loop_exit:
  input_ln:=true;
  end;
end;
@y
function input_ln(var f:alpha_file) : boolean; forward;
@z

% _____________________________________________________________________________
%
% [5.49]
% _____________________________________________________________________________

@x
@!pool_pointer = 0..pool_size;  {for variables that point into |str_pool|}
@!str_number = 0..max_strings;  {for variables that point into |str_start|}
@y
@!pool_pointer = 0..pool_size_max;  {for variables that point into |str_pool|}
@!str_number = 0..max_strings_max;  {for variables that point into |str_start|}
@z

% _____________________________________________________________________________
%
% [5.50]
% _____________________________________________________________________________

@x
@d trace_pr_pool_str(#) == begin
                           out_pool_str(log_file,#);
                           end
@y
@d trace_pr_pool_str(#) == begin
                           out_pool_str(log_file,#);
                           end
@#
@d log_pr_pool_str(#) == trace_pr_pool_str(#)
@z

% _____________________________________________________________________________
%
% [5.53]
% _____________________________________________________________________________

@x
  if (pool_ptr+# > pool_size) then
@y
  while (pool_ptr+# > pool_size) do
@z

@x
overflow('pool size ',pool_size);
@y
pool_size := pool_size + pool_size_def;
miktex_bibtex_realloc('str_pool', str_pool, pool_size);
@z

% _____________________________________________________________________________
%
% [5.60]
% _____________________________________________________________________________

@x
    name_of_file[name_ptr] := ' ';
@y
    name_of_file[name_ptr] := chr(0);
@z

% _____________________________________________________________________________
%
% [5.64]
% _____________________________________________________________________________

@x
@d hash_base = empty + 1                {lowest numbered hash-table location}
@d hash_max = hash_base + hash_size - 1 {highest numbered hash-table location}
@y
@z

@x
@!hash_loc=hash_base..hash_max;         {a location within the hash table}
@!hash_pointer=empty..hash_max;         {either |empty| or a |hash_loc|}
@y
@!hash_loc=0..9999999;             {a location within the hash table}
@!hash_pointer=integer;            {either |empty| or a |hash_loc|}
@z

% _____________________________________________________________________________
%
% [5.65]
% _____________________________________________________________________________

@x
@!hash_next : packed array[hash_loc] of hash_pointer;   {coalesced-list link}
@!hash_text : packed array[hash_loc] of str_number;     {pointer to a string}
@!hash_ilk : packed array[hash_loc] of str_ilk;         {the type of string}
@!ilk_info : packed array[hash_loc] of integer;         {|ilk|-specific info}
@!hash_used : hash_base..hash_max+1;    {allocation pointer for hash table}
@y
@!hash_next : ^hash_pointer;   {coalesced-list link}
@!hash_text : ^str_number;     {pointer to a string}
@!hash_ilk : ^str_ilk;         {the type of string}
@!ilk_info : ^integer;         {|ilk|-specific info}
@!hash_used : integer;         {allocation pointer for hash table}
@z

% _____________________________________________________________________________
%
% [5.68]
% _____________________________________________________________________________

@x
@d max_hash_value = hash_prime+hash_prime-2+127         {|h|'s maximum value}
@y
@z

@x
var h:0..max_hash_value;        {hash code}
@y
var h:integer;        {hash code}
@z

@x
@!old_string:boolean;   {set to |true| if it's an already encountered string}
@y
@z

@x
old_string := false;
@y
str_num := 0;           {set to |>0| if it's an already encountered string}
@z

% _____________________________________________________________________________
%
% [5.70]
% _____________________________________________________________________________

@x
            old_string := true;
@y
@z

% _____________________________________________________________________________
%
% [5.71]
% _____________________________________________________________________________

@x
if (old_string) then            {it's an already encountered string}
@y
if (str_num>0) then             {it's an already encountered string}
@z

% _____________________________________________________________________________
%
% [8.100]
% _____________________________________________________________________________

@x
check_cmnd_line := false;                       {many systems will change this}
@y
check_cmnd_line := true;
if (c4p_argc <> 2) then begin
  write_ln(term_out, 'Need exactly one file argument');
  goto_exit_program;
end;
@z

@x
aux_not_found:
    check_cmnd_line := false;
@y
aux_not_found:
  goto_exit_program;
@z

% _____________________________________________________________________________
%
% [8.102]
% _____________________________________________________________________________

@x
do_nothing;             {the ``default system'' doesn't use the command line}
@y
c4p_strcpy(name_of_file, file_name_size, c4p_argv[1]);
aux_name_length := c4p_strlen(name_of_file);
@z

% _____________________________________________________________________________
%
% [8.106]
% _____________________________________________________________________________

@x
add_extension (s_aux_extension);        {this also sets |name_length|}
aux_ptr := 0;                           {initialize the \.{.aux} file stack}
if (not a_open_in(cur_aux_file)) then
    sam_you_made_the_file_name_wrong;
@y
if (not miktex_has_extension(name_of_file, '.aux')) then
  add_extension(s_aux_extension)        {this also sets |name_length|}
else
  aux_name_length := aux_name_length - 4; {set to length without \.{.aux}}
aux_ptr := 0;                           {initialize the \.{.aux} file stack}
if (not c4p_try_fopen(cur_aux_file, name_of_file, c4p_rb_mode)) then
    sam_you_made_the_file_name_wrong;
@z

% _____________________________________________________________________________
%
% [8.108]
% _____________________________________________________________________________

@x
procedure print_aux_name;
begin
print_pool_str (cur_aux_str);
print_newline;
end;
@y
procedure print_aux_name;
begin
print_pool_str (cur_aux_str);
print_newline;
end;
@#
procedure log_pr_aux_name;
begin
log_pr_pool_str(cur_aux_str);
log_pr_newline;
end;
@z

% _____________________________________________________________________________
%
% [9.109] Reading the auxiliary file(s)
% _____________________________________________________________________________

@x
@d aux_done=31          {go here when finished with the \.{.aux} files}
@y
@d aux_done=31          {go here when finished with the \.{.aux} files}
@d goto_aux_done==c4p_throw(aux_done)
@z

% _____________________________________________________________________________
%
% [9.110]
% _____________________________________________________________________________

@x
print ('The top-level auxiliary file: ');
print_aux_name;
@y
if miktex_get_verbose_flag then begin
  print('The top-level auxiliary file: ');
  print_aux_name;
end else begin
  log_pr('The top-level auxiliary file: ');
  log_pr_aux_name;
end;
@z

@x
aux_done:
@y
aux_done: c4p_end_try_block(aux_done);
@z

% _____________________________________________________________________________
%
% [9.118]
% _____________________________________________________________________________

@x
@!bib_number = 0..max_bib_files;        {gives the |bib_list| range}
@y
@!bib_number = 0..max_bib_files_max;    {gives the |bib_list| range}
@z

% _____________________________________________________________________________
%
% [9.121]
% _____________________________________________________________________________

@x
procedure print_bib_name;
begin
print_pool_str (cur_bib_str);
print_pool_str (s_bib_extension);
print_newline;
end;
@y
{Return true if the |ext| string is at the end of the |s| string.  There
 are surely far more clever ways to do this, but it doesn't matter.}
function str_ends_with (@!s:str_number; @!ext:str_number) : boolean;
var i : integer;
    str_idx,ext_idx   : integer;
    str_char,ext_char : ASCII_code;
begin
  str_ends_with := false;
  if (length (ext) > length (s)) then
    return; {if extension is longer, they don't match}
  str_idx := length (s) - 1;
  ext_idx := length (ext) - 1;
  while (ext_idx >= 0) do begin {|>=| so we check the |'.'| char.}
    str_char := str_pool[str_start[s]+str_idx];
    ext_char := str_pool[str_start[ext]+ext_idx];
    if (str_char <> ext_char) then
      return;
    decr (str_idx);
    decr (ext_idx);
  end;
  str_ends_with := true;
exit: end;

{The above is needed because the file name specified in the
 \.{\\bibdata} command may or may not have the \.{.bib} extension. If it
 does, we don't want to print \.{.bib} twice.}
procedure print_bib_name;
begin
print_pool_str (cur_bib_str);
if not str_ends_with (cur_bib_str, s_bib_extension) then
  print_pool_str (s_bib_extension);
print_newline;
end;
@#
procedure log_pr_bib_name;
begin
log_pr_pool_str (cur_bib_str);
if not str_ends_with (cur_bib_str, s_bib_extension) then
  log_pr_pool_str (s_bib_extension);
log_pr_newline;
end;
@z

% _____________________________________________________________________________
%
% [9.123]
% _____________________________________________________________________________

@x
    overflow('number of database files ',max_bib_files);
@y
begin
  max_bib_files := max_bib_files + max_bib_files_def;
  miktex_bibtex_realloc('bib_file', bib_file, max_bib_files);
  miktex_bibtex_realloc('bib_list', bib_list, max_bib_files);
  miktex_bibtex_realloc('s_preamble', s_preamble, max_bib_files);
end;
@z

% _____________________________________________________________________________
%
% [9.127]
% _____________________________________________________________________________

@x
if (not a_open_in(bst_file)) then
@y
if (not miktex_open_bst_file(bst_file)) then
@z

@x
print ('The style file: ');
print_bst_name;
@y
if miktex_get_verbose_flag then begin
  print('The style file: ');
  print_bst_name;
end else begin
  log_pr('The style file: ');
  log_pr_bst_name;
end;
@z

% _____________________________________________________________________________
%
% [9.128]
% _____________________________________________________________________________

@x
procedure print_bst_name;
begin
print_pool_str (bst_str);
print_pool_str (s_bst_extension);
print_newline;
end;
@y
procedure print_bst_name;
begin
print_pool_str (bst_str);
print_pool_str (s_bst_extension);
print_newline;
end;
@#
procedure log_pr_bst_name;
begin
log_pr_pool_str (bst_str);
log_pr_pool_str (s_bst_extension);
log_pr_newline;
end;
@z

% _____________________________________________________________________________
%
% [9.129]
% _____________________________________________________________________________

@x
@!cite_number = 0..max_cites;   {gives the |cite_list| range}
@y
@!cite_number = 0..max_cites_max;   {gives the |cite_list| range}
@z

% _____________________________________________________________________________
%
% [9.138]
% _____________________________________________________________________________

@x
if (last_cite = max_cites) then
    begin
    print_pool_str (hash_text[cite_loc]);
    print_ln (' is the key:');
    overflow('number of cite keys ',max_cites);
@y
if (last_cite = max_cites) then begin
    max_cites := max_cites + max_cites_def;
    miktex_bibtex_realloc('cite_info', cite_info, max_cites);
    miktex_bibtex_realloc('cite_list', cite_list, max_cites);
    miktex_bibtex_realloc('entry_exists', entry_exists, max_cites);
    miktex_bibtex_realloc('type_list', type_list, max_cites);
    while (last_cite < max_cites) do begin
        type_list[last_cite] := empty;@/
        cite_info[last_cite] := any_value;  {to appeas \PASCAL's boolean evaluation}
        incr(last_cite);
    end;
@z

% _____________________________________________________________________________
%
% [9.141]
% _____________________________________________________________________________

@x
    name_of_file[name_ptr] := ' ';
@y
    name_of_file[name_ptr] := chr(0);
@z

@x
print ('A level-',aux_ptr:0,' auxiliary file: ');
print_aux_name;
@y
log_pr('A level-',aux_ptr:0,' auxiliary file: ');
log_pr_aux_name;
@z

% _____________________________________________________________________________
%
% [9.142]
% _____________________________________________________________________________

@x
    goto aux_done
@y
    goto_aux_done
@z

% _____________________________________________________________________________
%
% [10.146] Reading the style file
% _____________________________________________________________________________

@x
@d bst_done=32          {go here when finished with the \.{.bst} file}
@d no_bst_file=9932     {go here when skipping the \.{.bst} file}
@y
@d bst_done=32          {go here when finished with the \.{.bst} file}
@d no_bst_file=9932     {go here when skipping the \.{.bst} file}
@d goto_bst_done==c4p_throw(bst_done)
@d goto_no_bst_file==c4p_throw(no_bst_file)
@z

% _____________________________________________________________________________
%
% [10.149]
% _____________________________________________________________________________

@x
        goto bst_done
@y
        goto_bst_done
@z

% _____________________________________________________________________________
%
% [10.151]
% _____________________________________________________________________________

@x
    goto no_bst_file;   {this is a |goto| so that |bst_done| is not in a block}
@y
    goto_no_bst_file;   {this is a |goto| so that |bst_done| is not in a block}
@z

@x
        goto bst_done;
@y
        goto_bst_done;
@z

@x
bst_done: a_close (bst_file);
no_bst_file: a_close (bbl_file);
@y
bst_done: c4p_end_try_block(bst_done); a_close(bst_file);
no_bst_file: c4p_end_try_block(no_bst_file); a_close(bbl_file);
@z

% _____________________________________________________________________________
%
% [10.160]
% _____________________________________________________________________________

@x
@d quote_next_fn = hash_base - 1  {special marker used in defining functions}
@d end_of_def = hash_max + 1      {another such special marker}
@y
@z

@x
@!wiz_fn_loc = 0..wiz_fn_space;  {|wiz_defined|-function storage locations}
@!int_ent_loc = 0..max_ent_ints;        {|int_entry_var| storage locations}
@!str_ent_loc = 0..max_ent_strs;        {|str_entry_var| storage locations}
@!str_glob_loc = 0..max_glb_str_minus_1; {|str_global_var| storage locations}
@y
@!wiz_fn_loc = 0..wiz_fn_space_max;  {|wiz_defined|-function storage locations}
@!int_ent_loc = 0..max_ent_ints_max;    {|int_entry_var| storage locations}
@!str_ent_loc = 0..max_ent_strs_max;    {|str_entry_var| storage locations}
@!str_glob_loc = 0..max_glob_strs_max; {|str_global_var| storage locations}
@z

@x
@!field_loc = 0..max_fields;            {individual field storage locations}
@y
@!field_loc = 0..max_fields_max;        {individual field storage locations}
@z

@x
@!hash_ptr2 = quote_next_fn..end_of_def; {a special marker or a |hash_loc|}
@y
@!hash_ptr2 = integer; {a special marker or a |hash_loc|}
@z

% _____________________________________________________________________________
%
% [10.161]
% _____________________________________________________________________________

@x
@!entry_strs : array[str_ent_loc] of
                                packed array[0..ent_str_size] of ASCII_code;
@y
@!entry_strs : array[str_ent_loc] of ASCII_code;
@z

@x
@!str_glb_ptr : 0..max_glob_strs;       {general |str_global_var| location}
@y
@!str_glb_ptr : 0..max_glob_strs_max;   {general |str_global_var| location}
@z

@x
@!global_strs : array[str_glob_loc] of array[0..glob_str_size] of ASCII_code;
@!glb_str_end : array[str_glob_loc] of 0..glob_str_size;        {end markers}
@y
@!global_strs : array[str_glob_loc] of array[0..glob_str_size_max] of ASCII_code;
@!glb_str_end : array[str_glob_loc] of 0..glob_str_size_max;        {end markers}
@z

@x
@!num_glb_strs : 0..max_glob_strs; {number of distinct |str_global_var| names}
@y
@!num_glb_strs : 0..max_glob_strs_max; {number of distinct |str_global_var| names}
@z

% _____________________________________________________________________________
%
% [11.187]
% _____________________________________________________________________________

@x
type @!fn_def_loc = 0..single_fn_space; {for a single |wiz_defined|-function}
var singl_function : packed array[fn_def_loc] of hash_ptr2;
@y
type @!fn_def_loc = 0..single_fn_space_max; {for a single |wiz_defined|-function}
var singl_function : ^hash_ptr2;
@z

@x
begin
eat_bst_white_and_eof_check ('function');
@y
begin
single_fn_space := single_fn_space_def;
miktex_bibtex_alloc(singl_function, single_fn_space);
eat_bst_white_and_eof_check ('function');
@z

@x
exit:
end;
@y
exit:
miktex_bibtex_free(singl_function);
end;
@z

% _____________________________________________________________________________
%
% [11.188]
% _____________________________________________________________________________

@x
                            singl_fn_overflow;
@y
                            begin
			      single_fn_space := single_fn_space + single_fn_space_def;
                              miktex_bibtex_realloc('singl_function', singl_function, single_fn_space);
                            end;
@z

@x
procedure singl_fn_overflow;
begin
overflow('single function space ',single_fn_space);
end;
@y
@z

% _____________________________________________________________________________
%
% [11.200]
% _____________________________________________________________________________

@x
if (single_ptr + wiz_def_ptr > wiz_fn_space) then
    begin
    print (single_ptr + wiz_def_ptr : 0,': ');
    overflow('wizard-defined function space ',wiz_fn_space);
    end;
@y
while (single_ptr + wiz_def_ptr > wiz_fn_space) do begin
  wiz_fn_space := wiz_fn_space + wiz_fn_space_def;
  miktex_bibtex_realloc('wiz_functions', wiz_functions, wiz_fn_space);
end;
@z

% _____________________________________________________________________________
%
% [11.216]
% _____________________________________________________________________________

@x
    overflow('number of string global-variables ',max_glob_strs);
@y
    begin
      max_glob_strs := max_glob_strs + max_glob_strs_def;
      miktex_bibtex_realloc('glb_str_end', glb_str_end, max_glob_strs);
      miktex_bibtex_realloc('glb_str_ptr', glb_str_ptr, max_glob_strs);
      miktex_bibtex_realloc('global_strs', global_strs, glob_str_size * max_glob_strs);
      str_glb_ptr := num_glb_strs;
      while (str_glb_ptr < max_glob_strs) do begin {make new |str_global_var|s empty}
        glb_str_ptr[str_glb_ptr] := 0;
        glb_str_end[str_glb_ptr] := 0;
        incr(str_glb_ptr);
      end;
    end;
@z

% _____________________________________________________________________________
%
% [11.219]
% _____________________________________________________________________________

@x
@d undefined = hash_max + 1     {a special marker used for |type_list|}
@y
@z

% _____________________________________________________________________________
%
% [12.223]
% _____________________________________________________________________________

@x
    print ('Database file #',bib_ptr+1:0,': ');
    print_bib_name;@/
@y
    if miktex_get_verbose_flag then begin
      print('Database file #',bib_ptr+1:0,': ');
      print_bib_name;
    end else begin
      log_pr('Database file #',bib_ptr+1:0,': ');
      log_pr_bib_name;
    end;
@z

% _____________________________________________________________________________
%
% [12.226]
% _____________________________________________________________________________

@x
procedure check_field_overflow (@!total_fields : integer);
begin
if (total_fields > max_fields) then
    begin
    print_ln (total_fields:0,' fields:');
    overflow('total number of fields ',max_fields);
    end;
end;
@y
procedure check_field_overflow (@!total_fields : integer);
var @!f_ptr: field_loc;
    @!start_fields: field_loc;
begin
  if (total_fields > max_fields) then begin
    start_fields := max_fields;
    max_fields := total_fields + max_fields_def;
    miktex_bibtex_realloc('field_info', field_info, max_fields);
    for f_ptr := start_fields to max_fields - 1 do begin {Initialize to |missing|.}
      field_info[f_ptr] := missing;
    end;
  end;
end;
@z

% _____________________________________________________________________________
%
% [12.242]
% _____________________________________________________________________________

@x
    bib_err ('You''ve exceeded ',max_bib_files:0,' preamble commands');
@y
begin
  max_bib_files := max_bib_files + max_bib_files_def;
  miktex_bibtex_realloc('bib_file', bib_file, max_bib_files);
  miktex_bibtex_realloc('bib_list', bib_list, max_bib_files);
  miktex_bibtex_realloc('s_preamble', s_preamble, max_bib_files);
end;
@z

% _____________________________________________________________________________
%
% [12.251]
% _____________________________________________________________________________

@x
@d copy_char(#) == begin
                   if (field_end = buf_size) then
                       bib_field_too_long_err
                     else
                       begin
                       field_vl_str[field_end] := #;
                       incr(field_end);
                       end;
                   end
@y
@d copy_char(#) == begin
                   {We don't always increment by 1, so have to check |>=|.}
                   if (field_end >= buf_size) then begin
                       log_pr ('Field filled up at ', #, ', reallocating.');
                       log_pr_newline;
                       buffer_overflow; {reallocates all |buf_size| buffers}
                   end;
                   field_vl_str[field_end] := #;
                   incr(field_end);
                   end
@z

% _____________________________________________________________________________
%
% [12.263]
% _____________________________________________________________________________

@x
field_ptr := entry_cite_ptr * num_fields + fn_info[field_name_loc];
@y
field_ptr := entry_cite_ptr * num_fields + fn_info[field_name_loc];
if (field_ptr >= max_fields) then
    confusion('field_info index is out of range');
@z

% _____________________________________________________________________________
%
% [12.265]
% _____________________________________________________________________________

@x
check_field_overflow (num_fields*new_cite);
@y
check_field_overflow(num_fields * (new_cite + 1));
@z

% _____________________________________________________________________________
%
% [12.277]
% _____________________________________________________________________________

@x
@<Add cross-reference information@>=
begin
@y
@<Add cross-reference information@>=
begin
if ((num_cites - 1) * num_fields + crossref_num >= max_fields) then
    confusion('field_info index is out of range');
@z

% _____________________________________________________________________________
%
% [12.279]
% _____________________________________________________________________________

@x
@<Subtract cross-reference information@>=
begin
@y
@<Subtract cross-reference information@>=
begin
if ((num_cites - 1) * num_fields + crossref_num >= max_fields) then
    confusion('field_info index is out of range');
@z

% _____________________________________________________________________________
%
% [12.285]
% _____________________________________________________________________________

@x
@<Slide this cite key down to its permanent spot@>=
begin
@y
@<Slide this cite key down to its permanent spot@>=
begin
if ((cite_xptr + 1) * num_fields > max_fields) then
  confusion('field_info index is out of range');
@z

% _____________________________________________________________________________
%
% [12.287]
% _____________________________________________________________________________

@x
if (num_ent_ints*num_cites > max_ent_ints) then
    begin
    print (num_ent_ints*num_cites,': ');
    overflow('total number of integer entry-variables ',max_ent_ints);
    end;
@y
miktex_bibtex_alloc(entry_ints, (num_ent_ints + 1) * (num_cites + 1));
@z

% _____________________________________________________________________________
%
% [12.288]
% _____________________________________________________________________________

@x
if (num_ent_strs*num_cites > max_ent_strs) then
    begin
    print (num_ent_strs*num_cites,': ');
    overflow('total number of string entry-variables ',max_ent_strs);
    end;
@y
miktex_bibtex_alloc(entry_strs, (num_ent_strs + 1) * (num_cites + 1) * (ent_str_size + 1));
@z

@x
    entry_strs[str_ent_ptr][0] := end_of_string;
@y
    x_entry_strs(str_ent_ptr)(0) := end_of_string;
@z

% _____________________________________________________________________________
%
% [12.290]
% _____________________________________________________________________________

@x
@!ent_chr_ptr : 0..ent_str_size; {points at a |str_entry_var| character}
@!glob_chr_ptr : 0..glob_str_size; {points at a |str_global_var| character}
@y
@!ent_chr_ptr : 0..ent_str_size_max; {points at a |str_entry_var| character}
@!glob_chr_ptr : 0..glob_str_size_max; {points at a |str_global_var| character}
@z

% _____________________________________________________________________________
%
% [12.291]
% _____________________________________________________________________________

@x
@!lit_stk_loc = 0..lit_stk_size;        {the stack range}
@y
@!lit_stk_loc = 0..lit_stk_size_max;        {the stack range}
@z

% _____________________________________________________________________________
%
% [13.301]
% _____________________________________________________________________________

@x
var char_ptr : 0..ent_str_size;         {character index into compared strings}
@y
var char_ptr : 0..ent_str_size_max;     {character index into compared strings}
@z

@x
    char1 := entry_strs[ptr1][char_ptr];
    char2 := entry_strs[ptr2][char_ptr];
@y
    char1 := x_entry_strs(ptr1)(char_ptr);
    char2 := x_entry_strs(ptr2)(char_ptr);
@z

% _____________________________________________________________________________
%
% [13.307]
% _____________________________________________________________________________

@x
    overflow('literal-stack size ',lit_stk_size);
@y
    begin
      lit_stk_size := lit_stk_size + lit_stk_size_def;
      miktex_bibtex_realloc('lit_stack', lit_stack, lit_stk_size);
      miktex_bibtex_realloc('lit_stk_type', lit_stk_type, lit_stk_size);
    end;
@z

% _____________________________________________________________________________
%
% [13.320]
% _____________________________________________________________________________

@x
if (out_buf_length+(p_ptr2-p_ptr1) > buf_size) then
    overflow('output buffer size ',buf_size);
@y
while (out_buf_length+(p_ptr2-p_ptr1) > buf_size) do
    buffer_overflow;
@z

% _____________________________________________________________________________
%
% [13.327]
% _____________________________________________________________________________

@x
    field_ptr := cite_ptr*num_fields + fn_info[ex_fn_loc];
@y
    field_ptr := cite_ptr*num_fields + fn_info[ex_fn_loc];
    if (field_ptr >= max_fields) then
        confusion('field_info index is out of range');
@z

% _____________________________________________________________________________
%
% [13.329]
% _____________________________________________________________________________

@x
    while (entry_strs[str_ent_ptr][ex_buf_ptr] <> end_of_string) do
                                        {copy characters into the buffer}
        append_ex_buf_char (entry_strs[str_ent_ptr][ex_buf_ptr]);
@y
    while (x_entry_strs(str_ent_ptr)(ex_buf_ptr) <> end_of_string) do
                                        {copy characters into the buffer}
        append_ex_buf_char(x_entry_strs(str_ent_ptr)(ex_buf_ptr));
@z

% _____________________________________________________________________________
%
% [13.330]
% _____________________________________________________________________________

@x
        append_char (global_strs[str_glb_ptr][glob_chr_ptr]);
@y
        append_char(x_global_strs(str_glb_ptr)(glob_chr_ptr));
@z

% _____________________________________________________________________________
%
% [13.334]
% _____________________________________________________________________________

@x
build_in('width$      ',6,b_width,n_width);
build_in('while$      ',6,b_while,n_while);
build_in('width$      ',6,b_width,n_width);
@y
build_in('while$      ',6,b_while,n_while);
build_in('width$      ',6,b_width,n_width);
@z

% _____________________________________________________________________________
%
% [13.357]
% _____________________________________________________________________________

@x
    while (sp_ptr < sp_xptr1) do
        begin                   {copy characters into |entry_strs|}
        entry_strs[str_ent_ptr][ent_chr_ptr] := str_pool[sp_ptr];
        incr(ent_chr_ptr);
        incr(sp_ptr);
        end;
    entry_strs[str_ent_ptr][ent_chr_ptr] := end_of_string;
@y
    while (sp_ptr < sp_xptr1) do
        begin                   {copy characters into |entry_strs|}
        x_entry_strs(str_ent_ptr)(ent_chr_ptr) := str_pool[sp_ptr];
        incr(ent_chr_ptr);
        incr(sp_ptr);
        end;
    x_entry_strs(str_ent_ptr)(ent_chr_ptr) := end_of_string;
@z

% _____________________________________________________________________________
%
% [13.359]
% _____________________________________________________________________________

@x
            global_strs[str_glb_ptr][glob_chr_ptr] := str_pool[sp_ptr];
@y
            x_global_strs(str_glb_ptr)(glob_chr_ptr) := str_pool[sp_ptr];
@z

% _____________________________________________________________________________
%
% [13.388]
% _____________________________________________________________________________

@x
while ((ex_buf_xptr < ex_buf_ptr) and
                        (lex_class[ex_buf[ex_buf_ptr]] = white_space) and
                        (lex_class[ex_buf[ex_buf_ptr]] = sep_char)) do
        incr(ex_buf_xptr);                      {this removes leading stuff}
@y
@z

% _____________________________________________________________________________
%
% [13.444]
% _____________________________________________________________________________

@x
if (pop_lit2 >= cmd_str_ptr) then       {no shifting---merely change pointers}
@y
str_room(sp_brace_level + sp_end - sp_ptr);
if (pop_lit2 >= cmd_str_ptr) then       {no shifting---merely change pointers}
@z

% _____________________________________________________________________________
%
% [13.459]
% _____________________________________________________________________________

@x
    undefined : trace_pr ('unknown')
@y
    trace_pr('unknown')
@z

% _____________________________________________________________________________
%
% [13.460]
% _____________________________________________________________________________

@x
        while (entry_strs[str_ent_ptr][ent_chr_ptr] <> end_of_string) do
            begin
            trace_pr (xchr[entry_strs[str_ent_ptr][ent_chr_ptr]]);
            incr(ent_chr_ptr);
            end;
@y
        while (x_entry_strs(str_ent_ptr)(ent_chr_ptr) <> end_of_string) do begin
            trace_pr(xchr[x_entry_strs(str_ent_ptr)(ent_chr_ptr)]);
            incr(ent_chr_ptr);
        end;
@z

% _____________________________________________________________________________
%
% [13.462]
% _____________________________________________________________________________

@x
    field_ptr := cite_ptr * num_fields;
    field_end_ptr := field_ptr + num_fields;
@y
    field_ptr := cite_ptr * num_fields;
    field_end_ptr := field_ptr + num_fields;
    if (field_end_ptr > max_fields) then
        confusion('field_info index is out of range');
@z

% _____________________________________________________________________________
%
% [16.467] System-dependent changes
% _____________________________________________________________________________

@x
itself will get a new section number.
@y
itself will get a new section number.

@ We use the algorithm from Knuth's \.{primes.web} to compute |hash_prime|
as the smallest prime number not less than 85\% of |hash_size| (and
|>=128|).

@d primes == hash_next {array holding the first |k| primes}
@d mult == hash_text {array holding odd multiples of the first |o| primes}

@<Procedures and functions for about everything@>=
procedure compute_hash_prime;
var hash_want: integer; {85\% of |hash_size|}
@!k: integer; {number of prime numbers $p_i$ in |primes|}
@!j: integer; {a prime number candidate}
@!o: integer; {number of odd multiples of primes in |mult|}
@!square: integer; {$p_o^2$}
@!n: integer; {loop index}
@!j_prime: boolean; {is |j| a prime?}
begin hash_want := (hash_size div 20) * 17;
j := 1;
k := 1;
hash_prime := 2;
primes[k] := hash_prime;
o := 2;
square := 9;
while hash_prime < hash_want do
  begin
  repeat
    j := j + 2;
    if j = square then
      begin
      mult[o] := j;
      j := j + 2;
      incr (o);
      square := primes[o] * primes[o];
      end;
    n := 2;
    j_prime := true;
    while (n < o) and j_prime do
      begin
      while mult[n] < j do mult[n] := mult[n] + 2 * primes[n];
      if mult[n] = j then j_prime := false;
      incr (n);
      end;
  until j_prime;
  incr (k);
  hash_prime := j;
  primes[k] := hash_prime;
  end;
end;

@ @<Globals in the outer block@>=
@! buf_size: buf_pointer;
@! end_of_def: integer;
@! ent_str_size: integer;
@! glob_str_size: integer;
@! hash_max: integer;
@! hash_prime: integer;
@! hash_size: integer;
@! lit_stk_size: integer;
@! max_bib_files: integer;
@! max_cites: integer;
@! max_ent_ints: integer;
@! max_ent_strs: integer;
@! max_fields: integer;
@! max_glob_strs: integer;
@! max_strings: integer;
@! min_crossrefs: integer;
@! pool_size: integer;
@! single_fn_space: integer;
@! undefined: integer;
@! wiz_fn_space: integer;

@ @<Begin try blocks@>=
c4p_begin_try_block(exit_program);
c4p_begin_try_block(close_up_shop);
c4p_begin_try_block(no_bst_file);
c4p_begin_try_block(bst_done);
c4p_begin_try_block(aux_done);

@ @<Forward declarations@>=
function miktex_get_verbose_flag : boolean; forward;
@z
