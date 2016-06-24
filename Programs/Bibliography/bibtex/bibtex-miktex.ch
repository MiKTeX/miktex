%% bibtex-miktex.ch: WEB change file for BibTeX
%% 
%% Copyright (C) 1996-2016 Christian Schenk
%% 
%% This file is free software; you can redistribute it and/or modify it
%% under the terms of the GNU General Public License as published by the
%% Free Software Foundation; either version 2, or (at your option) any
%% later version.
%% 
%% This file is distributed in the hope that it will be useful, but
%% WITHOUT ANY WARRANTY; without even the implied warranty of
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
%% General Public License for more details.
%% 
%% You should have received a copy of the GNU General Public License
%% along with This file; if not, write to the Free Software Foundation,
%% 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

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
if not miktex_get_quiet_flag then begin
  print(banner);
  miktex_print_miktex_banner(output);
  write_ln;
end;@/
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
if (history > 1) then
begin
  c4p_exit (1); {\MiKTeX: throw an exception}
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
@!buf_size=1000; {maximum number of characters in an input line (or string)}
@y
@!buf_size=200000; {maximum number of characters in an input line (or string)}
@z

@x
@!aux_stack_size=20; {maximum number of simultaneous open \.{.aux} files}
@y
@!aux_stack_size=20; {maximum number of simultaneous open \.{.aux} files}
@z

@x
@!max_bib_files=20; {maximum number of \.{.bib} files allowed}
@y
@!max_bib_files_max=1000000; {maximum number of \.{.bib} files allowed}
@z

@x
@!pool_size=65000; {maximum number of characters in strings}
@y
@!pool_size_max=1000000; {maximum number of characters in strings}
@z

@x
@!max_strings=4000; {maximum number of strings, including pre-defined;
@y
@!max_strings=35000; {maximum number of strings, including pre-defined;
@z

@x
@!max_cites=750; {maximum number of distinct cite keys; must be
@y
@!max_cites=5000; {maximum number of distinct cite keys; must be
@z

@x
@!min_crossrefs=2; {minimum number of cross-refs required for automatic
@y
@!min_crossrefs_def=2; {minimum number of cross-refs required for automatic
@z

@x
@!wiz_fn_space=3000; {maximum amount of |wiz_defined|-function space}
@y
@!wiz_fn_space_max=1000000; {maximum amount of |wiz_defined|-function space}
@z

@x
@!single_fn_space=100; {maximum amount for a single |wiz_defined|-function}
@y
@!single_fn_space=100; {maximum amount for a single |wiz_defined|-function}
@z

@x
@!max_ent_ints=3000; {maximum number of |int_entry_var|s
@y
@!max_ent_ints_max=1000000; {maximum number of |int_entry_var|s
@z

@x
@!max_ent_strs=3000; {maximum number of |str_entry_var|s
@y
@!max_ent_strs_max=1000000; {maximum number of |str_entry_var|s
@z

@x
@!ent_str_size=100; {maximum size of a |str_entry_var|; must be |<=buf_size|}
@y
@!ent_str_size=250; {maximum size of a |str_entry_var|; must be |<=buf_size|}
@z

@x
@!glob_str_size=1000; {maximum size of a |str_global_var|;
@y
@!glob_str_size=5000; {maximum size of a |str_global_var|;
@z

@x
@!max_fields=17250; {maximum number of fields (entries $\times$ fields,
@y
@!max_fields_max=1000000; {maximum number of fields (entries $\times$ fields,
@z

@x
@!lit_stk_size=100; {maximum number of literal functions on the stack}
@y
@!lit_stk_size=100; {maximum number of literal functions on the stack}
@z

@x
@d hash_size=5000       {must be |>= max_strings| and |>= hash_prime|}
@y
@d hash_size=35307       {must be |>= max_strings| and |>= hash_prime|}
@z

@x
@d hash_prime=4253      {a prime number about 85\% of |hash_size| and |>= 128|
@y
@d hash_prime=30011      {a prime number about 85\% of |hash_size| and |>= 128|
@z

@x
@d file_name_size=40    {file names shouldn't be longer than this}
@y
@d file_name_size=259   {file names shouldn't be longer than this}
@d file_name_size_plus_one=260 {one more for the string terminator}
@z

@x
@d max_glob_strs=10     {maximum number of |str_global_var| names}
@y
@d max_glob_strs=20     {maximum number of |str_global_var| names}
@z

% _____________________________________________________________________________
%
% [2.17]
% _____________________________________________________________________________

@x
if (hash_prime >= (16384-64)) then              bad:=10*bad+6;
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
@y
@!pool_pointer = 0..pool_size_max;  {for variables that point into |str_pool|}
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
overflow('pool size ',pool_size);
@y
pool_size := pool_size + pool_size;
miktex_bibtex_realloc ('str_pool', str_pool, pool_size);
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
% [8.100]
% _____________________________________________________________________________

@x
check_cmnd_line := false;                       {many systems will change this}
@y
check_cmnd_line := true;
if (c4p_argc <> 2) then begin
  write_ln (term_out, 'Need exactly one file argument');
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
c4p_strcpy(name_of_file,file_name_size,c4p_argv[1]);
aux_name_length:=c4p_strlen(name_of_file);
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
if (not miktex_has_extension(name_of_file, '.aux'))
then
  add_extension (s_aux_extension)        {this also sets |name_length|}
else
  aux_name_length := aux_name_length - 4; {set to length without \.{.aux}}
aux_ptr := 0;                           {initialize the \.{.aux} file stack}
if (not c4p_try_fopen(cur_aux_file,name_of_file,c4p_rb_mode)) then
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
log_pr_pool_str (cur_aux_str);
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
if not miktex_get_quiet_flag then begin
  print ('The top-level auxiliary file: ');
  print_aux_name;
end else begin
  log_pr ('The top-level auxiliary file: ');
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
procedure print_bib_name;
begin
print_pool_str (cur_bib_str);
print_pool_str (s_bib_extension);
print_newline;
end;
@#
procedure log_pr_bib_name;
begin
log_pr_pool_str (cur_bib_str);
log_pr_pool_str (s_bib_extension);
log_pr_newline;
end;
@z

% _____________________________________________________________________________
%
% [9.123]
% _____________________________________________________________________________

@x
if (bib_ptr = max_bib_files) then
    overflow('number of database files ',max_bib_files);
@y
if (bib_ptr = max_bib_files) then begin
  max_bib_files := max_bib_files + max_bib_files;
  miktex_bibtex_realloc ('bib_file', bib_file, max_bib_files);
  miktex_bibtex_realloc ('bib_list', bib_list, max_bib_files);
  miktex_bibtex_realloc ('s_preamble', s_preamble, max_bib_files);
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
if not miktex_get_quiet_flag then begin
  print ('The style file: ');
  print_bst_name;
end else begin
  log_pr ('The style file: ');
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
% [9.141]
% _____________________________________________________________________________

@x
    name_of_file[name_ptr] := ' ';
@y
    name_of_file[name_ptr] := chr(0);
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
bst_done: c4p_end_try_block(bst_done); a_close (bst_file);
no_bst_file: c4p_end_try_block(no_bst_file); a_close (bbl_file);
@z

% _____________________________________________________________________________
%
% [10.160]
% _____________________________________________________________________________

@x
@!wiz_fn_loc = 0..wiz_fn_space;  {|wiz_defined|-function storage locations}
@!int_ent_loc = 0..max_ent_ints;        {|int_entry_var| storage locations}
@!str_ent_loc = 0..max_ent_strs;        {|str_entry_var| storage locations}
@y
@!wiz_fn_loc = 0..wiz_fn_space_max;  {|wiz_defined|-function storage locations}
@!int_ent_loc = 0..max_ent_ints_max;    {|int_entry_var| storage locations}
@!str_ent_loc = 0..max_ent_strs_max;    {|str_entry_var| storage locations}
@z

@x
@!field_loc = 0..max_fields;            {individual field storage locations}
@y
@!field_loc = 0..max_fields_max;        {individual field storage locations}
@z

% _____________________________________________________________________________
%
% [10.161]
% _____________________________________________________________________________

@x
@!entry_strs : array[str_ent_loc] of
                                packed array[0..ent_str_size] of ASCII_code;
@y
@!entry_strs : array[str_ent_loc] of entry_string;
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
if (single_ptr + wiz_def_ptr > wiz_fn_space) then begin
  wiz_fn_space := wiz_fn_space + wiz_fn_space;
  miktex_bibtex_realloc ('wiz_functions', wiz_functions, wiz_fn_space);
end;
@z

% _____________________________________________________________________________
%
% [12.223]
% _____________________________________________________________________________

@x
    print ('Database file #',bib_ptr+1:0,': ');
    print_bib_name;@/
@y
    if not miktex_get_quiet_flag then begin
      print ('Database file #',bib_ptr+1:0,': ');
      print_bib_name;
    end else begin
      log_pr ('Database file #',bib_ptr+1:0,': ');
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
    max_fields := total_fields + max_fields;
    miktex_bibtex_realloc ('field_info', field_info, max_fields);
    for f_ptr := start_fields to max_fields - 1 do begin
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
  max_bib_files := max_bib_files + max_bib_files;
  miktex_bibtex_realloc ('bib_file', bib_file, max_bib_files);
  miktex_bibtex_realloc ('bib_list', bib_list, max_bib_files);
  miktex_bibtex_realloc ('s_preamble', s_preamble, max_bib_files);
end;
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
if (num_ent_ints*num_cites > max_ent_ints) then begin
  max_ent_ints := (num_ent_ints + 1) * (num_cites + 1);
  miktex_bibtex_realloc ('entry_ints', entry_ints, max_ent_ints);
end;
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
if (num_ent_strs*num_cites > max_ent_strs) then begin
  max_ent_strs := num_ent_strs * num_cites;
  miktex_bibtex_realloc ('entry_strs', entry_strs, max_ent_strs);
end;
@z

% _____________________________________________________________________________
%
% [16.467] System-dependent changes
% _____________________________________________________________________________

@x
itself will get a new section number.
@y
itself will get a new section number.

@ @<Types in the outer block@>=
@! entry_string = packed array[0..ent_str_size] of ASCII_code;

@ @<Globals in the outer block@>=
@! min_crossrefs : integer;
@! max_bib_files : integer;
@! max_ent_ints : integer;
@! max_ent_strs : integer;
@! wiz_fn_space : integer;
@! max_fields : integer;
@! pool_size : integer;

@ @<Begin try blocks@>=
c4p_begin_try_block(exit_program);
c4p_begin_try_block(close_up_shop);
c4p_begin_try_block(no_bst_file);
c4p_begin_try_block(bst_done);
c4p_begin_try_block(aux_done);

@ @<Forward declarations@>=
function miktex_get_quiet_flag : boolean; forward;
@z
