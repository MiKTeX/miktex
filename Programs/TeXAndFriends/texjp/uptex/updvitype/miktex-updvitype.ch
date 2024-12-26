%% miktex-updvitype.ch
%%
%% Copyright (C) 2021-2024 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [1.3]
% _____________________________________________________________________________

@x
var @<Globals in the outer block@>@/
@y
var @<Globals in the outer block@>@/
function isinternalUPTEX : boolean; forward;
@z

@x
  parse_arguments;
  print (banner);
  print (' (');
  print (conststringcast(get_enc_string));
  print (')');
  print_ln (version_string);
@y
  print (banner);
  miktex_print_enc_string(output);
@z

 @x
  parse_arguments;
 @y
 @z

 @x
  REMOVE_THIS_BEGIN;
  print (banner);
  print (' (');
  print (conststringcast(get_enc_string));
  print (')');
  print_ln (version_string);
  REMOVE_THIS_END;
 @y
  
  miktex_print_enc_string(output);
 @z

% _____________________________________________________________________________
%
% [4.23]
% _____________________________________________________________________________

@x
  full_name := kpse_find_ofm (cur_name);
  if full_name then begin
    tfm_file := fopen (full_name, FOPEN_RBIN_MODE);
  end else begin
    full_name := kpse_find_tfm (cur_name);
    if full_name then begin
      tfm_file := fopen (full_name, FOPEN_RBIN_MODE);
    end else begin
      tfm_file := nil;
    end
  end;
@y
begin
  if not miktex_open_ofm_file(tfm_file, cur_name) then begin
    miktex_open_tfm_file(tfm_file, cur_name)
  end;
end;
@z

% _____________________________________________________________________________
%
% [8.78]
% _____________________________________________________________________________

@x
ifdef('HEX_CHAR_CODE')
procedure print_hex_number(c:integer);
var n:integer;
    b:array[1..8] of integer;
begin
  n:=1;
  while (n<8) and (c<>0) do
    begin b[n]:=c mod 16; c:=c div 16; n:=n+1 end;
  print('"');
  if n=1 then print(xchr['0'])
  else
    begin
      n:=n-1;
      while n>0 do
        begin print_hex_digit(b[n]); n:=n-1 end
    end
end;
endif('HEX_CHAR_CODE')
@y
@z

% _____________________________________________________________________________
%
% [9.89]
% _____________________________________________________________________________

@x
ifdef('HEX_CHAR_CODE')
          print('(');
          print_hex_number(p);
          print(')');
endif('HEX_CHAR_CODE')
@y
@z

@x
ifdef('HEX_CHAR_CODE')
          print('(');
          print_hex_number(p);
          print(')');
endif('HEX_CHAR_CODE')
@y
@z

@x
ifdef('HEX_CHAR_CODE')
        print('(');
        print_hex_number(p);
        print(')');
endif('HEX_CHAR_CODE')
@y
@z

% _____________________________________________________________________________
%
% [9.96]
% _____________________________________________________________________________

@x
ifdef('HEX_CHAR_CODE')
  print(' (');
  print_hex_number(p);
  print(')');
endif('HEX_CHAR_CODE')
@y
@z

% _____________________________________________________________________________
%
% [14.120] System-dependent changes
% _____________________________________________________________________________

@x
const n_options = 10; {Pascal won't count array lengths for us.}
      usage_help (UPDVITYPE_HELP, 'issue@@texjp.org');
    end else if argument_is ('kanji') then begin
      set_prior_file_enc;
      if (not set_enc_string(optarg,optarg)) then begin
        write_ln('Bad kanji encoding "', stringcast(optarg), '".');
      end;

    end; {Else it was a flag; |getopt| has already done the assignment.}
@ Decide kanji encode
@.-kanji@>

@<Define the option...@> =
long_options[current_option].name := 'kanji';
long_options[current_option].has_arg := 1;
long_options[current_option].flag := 0;
long_options[current_option].val := 0;
incr (current_option);

@ An element with all zeros always ends the list.
@y
@z
