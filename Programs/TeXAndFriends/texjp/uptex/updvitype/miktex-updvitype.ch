%% miktex-updvitype.ch:
%% 
%% Copyright (C) 2021 Christian Schenk
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
% [1.3]
% _____________________________________________________________________________

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
