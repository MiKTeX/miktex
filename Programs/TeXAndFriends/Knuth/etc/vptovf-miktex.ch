%% vptovf-miktex.ch: WEB change file for VPtoVF
%% 
%% Copyright (C) 1991-2016 Christian Schenk
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
  begin print_ln(banner);@/
@y
  begin print (banner);
  miktex_print_miktex_banner (output);
  write_ln;@/
@z

% _____________________________________________________________________________
%
% [1.3]
% _____________________________________________________________________________

@x
@!buf_size=60; {length of lines displayed in error messages}
@!max_header_bytes=100; {four times the maximum number of words allowed in
  the \.{TFM} file header block, must be 1024 or less}
@!vf_size=10000; {maximum length of |vf| data, in bytes}
@!max_stack=100; {maximum depth of simulated \.{DVI} stack}
@!max_param_words=30; {the maximum number of \.{fontdimen} parameters allowed}
@!max_lig_steps=5000;
  {maximum length of ligature program, must be at most $32767-257=32510$}
@!max_kerns=500; {the maximum number of distinct kern values}
@!hash_size=5003; {preferably a prime number, a bit larger than the number
  of character pairs in lig/kern steps}
@y
@!buf_size=60; {length of lines displayed in error messages}
@!max_header_bytes=400; {four times the maximum number of words allowed in
  the \.{TFM} file header block, must be 1024 or less}
@!vf_size=100000; {maximum length of |vf| data, in bytes}
@!max_stack=100; {maximum depth of simulated \.{DVI} stack}
@!max_param_words=60; {the maximum number of \.{fontdimen} parameters allowed}
@!max_lig_steps=32000;
  {maximum length of ligature program, must be at most $32767-257=32510$}
@!max_kerns=32000; {the maximum number of distinct kern values}
@!hash_size=32003; {preferably a prime number, a bit larger than the number
  of character pairs in lig/kern steps}
@z

% _____________________________________________________________________________
%
% [2.6]
% _____________________________________________________________________________

@x
@ @<Set init...@>=
reset(vpl_file);
@y
@ @<Set init...@>=
if (c4pargc <> 4) then begin
  print_ln ('Usage: vptovf VPLFILE VFFILE TFMFILE');
  c4p_exit (1);
end;
if (not c4p_fopen(vpl_file, c4p_argv[1], c4p_r_mode, false)) then begin
  print_ln ('The VPL file could not be found.');
  c4p_exit (1);
end;
reset (vpl_file);
@z

% _____________________________________________________________________________
%
% [2.21]
% _____________________________________________________________________________

@x
@!vf_file:packed file of 0..255;
@!tfm_file:packed file of 0..255;
@y
@!vf_file:byte_file;
@!tfm_file:byte_file;
@z

% _____________________________________________________________________________
%
% [2.22]
% _____________________________________________________________________________

@x
@<Set init...@>=
rewrite(vf_file); rewrite(tfm_file);
@y
@<Set init...@>=
if (c4pargc <> 4) then begin
  print_ln ('Usage: vptovf VPLFILE VFFILE TFMFILE');
  c4p_exit (1);
end;
if (not c4p_fopen(vf_file, c4p_argv[2], c4p_wb_mode, false)) then begin
  print_ln ('The VF file could not be written.');
  c4p_exit (1);
end;
if (not c4p_fopen(tfm_file, c4p_argv[3], c4p_wb_mode, false)) then begin
  print_ln ('The TFM file could not be written.');
  c4p_exit (1);
end;
rewrite (vf_file);
rewrite (tfm_file);
@z

% _____________________________________________________________________________
%
% [3.23] Basic input routines
% _____________________________________________________________________________

@x
@!byte=0..255; {unsigned eight-bit quantity}
@y
@!byte=0..255; {unsigned eight-bit quantity}
@!byte_file=packed file of byte;
@z

% _____________________________________________________________________________
%
% [3.24]
% _____________________________________________________________________________

@x
@d first_ord=0 {ordinal number of the smallest element of |char|}
@d last_ord=127 {ordinal number of the largest element of |char|}
@y
@d first_ord=0 {ordinal number of the smallest element of |char|}
@d last_ord=127 {ordinal number of the largest element of |char|}
@z
