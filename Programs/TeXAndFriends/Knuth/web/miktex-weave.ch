%% miktex-weave.ch
%%
%% Copyright (C) 1991-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [1.3]
% _____________________________________________________________________________

 @x
@d stat==@{ {change this to `$\\{stat}\equiv\null$'
  when gathering usage statistics}
@d tats==@t@>@} {change this to `$\\{tats}\equiv\null$'
  when gathering usage statistics}
 @y
@d stat==
@d tats==
 @z

% _____________________________________________________________________________
%
% [1.8]
% _____________________________________________________________________________

@x
@!max_bytes=45000; {|1/ww| times the number of bytes in identifiers,
  index entries, and module names; must be less than 65536}
@!max_names=5000; {number of identifiers, index entries, and module names;
  must be less than 10240}
@!max_modules=2000;{greater than the total number of modules}
@!hash_size=353; {should be prime}
@!buf_size=100; {maximum length of input line}
@!longest_name=400; {module names shouldn't be longer than this}
@!long_buf_size=500; {|buf_size+longest_name|}
@!line_length=80; {lines of \TeX\ output have at most this many characters,
  should be less than 256}
@!max_refs=30000; {number of cross references; must be less than 65536}
@!max_toks=30000; {number of symbols in \PASCAL\ texts being parsed;
  must be less than 65536}
@!max_texts=2000; {number of phrases in \PASCAL\ texts being parsed;
  must be less than 10240}
@!max_scraps=1000; {number of tokens in \PASCAL\ texts being parsed}
@!stack_size=200; {number of simultaneous output levels}
@y
@!max_bytes=65535; {|1/ww| times the number of bytes in identifiers,
  index entries, and module names; must be less than 65536}
@!max_names=10239; {number of identifiers, index entries, and module names;
  must be less than 10240}
@!max_modules=5000;{greater than the total number of modules}
@!hash_size=353; {should be prime}
@!buf_size=5000; {maximum length of input line}
@!longest_name=2000; {module names shouldn't be longer than this}
@!long_buf_size=7000; {|buf_size+longest_name|}
@!line_length=80; {lines of \TeX\ output have at most this many characters,
  should be less than 256}
@!max_refs=65535; {number of cross references; must be less than 65536}
@!max_toks=65535; {number of symbols in \PASCAL\ texts being parsed;
  must be less than 65536}
@!max_texts=10239; {number of phrases in \PASCAL\ texts being parsed;
  must be less than 10240}
@!max_scraps=10000; {number of tokens in \PASCAL\ texts being parsed}
@!stack_size=2000; {number of simultaneous output levels}
@z

% _____________________________________________________________________________
%
% [2.12]
% _____________________________________________________________________________

@x
@d text_char == char {the data type of characters in text files}
@d first_text_char=0 {ordinal number of the smallest element of |text_char|}
@d last_text_char=255 {ordinal number of the largest element of |text_char|}
@y
@d text_char == char {the data type of characters in text files}
@d first_text_char=-128 {ordinal number of the smallest element of |text_char|}
@d last_text_char=127 {ordinal number of the largest element of |text_char|}
@z

% _____________________________________________________________________________
%
% [2.18]
% _____________________________________________________________________________

@x
xord[' ']:=" ";
@y
xord[' ']:=" ";
xord[xchr[carriage_return]]:=" ";
@z

% _____________________________________________________________________________
%
% [3.21]
% _____________________________________________________________________________

@x
rewrite(term_out,'TTY:'); {send |term_out| output to the terminal}
@y
term_out:=output;
@z

% _____________________________________________________________________________
%
% [3.24]
% _____________________________________________________________________________

@x
begin reset(web_file); reset(change_file);
@y
begin if (c4pargc < 3) then
  fatal_error('Usage: weave inputfile changefile outputfile');
c4p_fopen (web_file,c4p_argv[1],c4p_r_mode,true); reset(web_file);
c4p_fopen (change_file,c4p_argv[2],c4p_r_mode,true); reset(change_file);
@z

@x
rewrite(tex_file);
@y
if (c4pargc < 4) then
  fatal_error('Usage: weave inputfile changefile outputfile');
c4p_fopen(tex_file,c4p_argv[3],c4p_wb_mode,true); rewrite(tex_file);
@z

% _____________________________________________________________________________
%
% [4.33]
% _____________________________________________________________________________

@x
begin goto end_of_WEAVE;
@y
begin c4p_throw(end_of_WEAVE);
@z

% _____________________________________________________________________________
%
% [5.37]
% _____________________________________________________________________________

@x
@d ww=2 {we multiply the byte capacity by approximately this amount}
@y
@d ww=5 {we multiply the byte capacity by approximately this amount}
@z

% _____________________________________________________________________________
%
% [20.259]
% _____________________________________________________________________________

@x
reset(term_in,'TTY:','/I'); {open |term_in| as the terminal, don't do a |get|}
@y
term_in:=input;
@z

% _____________________________________________________________________________
%
% [21.261] The main program
% _____________________________________________________________________________

@x
begin initialize; {beginning of the main program}
print_ln(banner); {print a ``banner line''}
@y
begin {beginning of the main program}
c4p_begin_try_block(end_of_WEAVE);
initialize;
print(banner);
miktex_print_miktexbanner(term_out);
new_line; {print a ``banner line''}
@z

@x
end_of_WEAVE:
@y
end_of_WEAVE:
c4p_end_try_block(end_of_WEAVE);
@z

% _____________________________________________________________________________
%
% [21.263]
% _____________________________________________________________________________

@x
end {there are no other cases}
@y
end; {there are no other cases}
new_line;
if (history > harmless_message) then
  c4p_exit (history);
@z
