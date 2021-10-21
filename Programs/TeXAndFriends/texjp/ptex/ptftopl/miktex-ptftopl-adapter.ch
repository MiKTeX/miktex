%% miktex-ptftopl-adapter.ch: tftopl-miktex.web to ptftopl.ch adapter
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

@x
\def\title{TF\lowercase{to}PL}
@y
\def\title{TF\lowercase{to}PL changes for C}
@z

% _____________________________________________________________________________
%
% [1.1] Introduction
% _____________________________________________________________________________

@x
@d banner=='This is TFtoPL, Version 3.3' {printed when the program starts}
@y
@d my_name=='tftopl'
@d banner=='This is TFtoPL, Version 3.3' {printed when the program starts}
@z

% _____________________________________________________________________________
%
% [1.2]
% _____________________________________________________________________________

@x
  begin print(banner);
@y
  begin
    tfm_file_array := xmalloc_array (byte, 1002);
    parse_arguments;
  print(banner);
@z

% _____________________________________________________________________________
%
% [2.7]
% _____________________________________________________________________________

@x
if (not miktex_open_tfm_file(tfm_file, c4p_argv[1])) then begin
@y
  print_ln (version_string);
if (not miktex_open_tfm_file(tfm_file, c4p_argv[1])) then begin
@z

% _____________________________________________________________________________
%
% [3.19]
% _____________________________________________________________________________

@x
@<Types...@>=
@!byte=0..255; {unsigned eight-bit quantity}
@!index=0..tfm_size; {address of a byte in |tfm|}

@ @<Glob...@>=
@!tfm:array [-1000..tfm_size] of byte; {the input data all goes here}
@y
@d index == index_type

@<Types...@>=
@!byte=0..255; {unsigned eight-bit quantity}
@!index=0..tfm_size; {address of a byte in |tfm|}

@ @<Glob...@>=
@!tfm:array [-1000..tfm_size] of byte; {the input data all goes here}
@!tfm_file_array: ^byte; {the input data all goes here}
@z

% _____________________________________________________________________________
%
% [7.99]
% _____________________________________________________________________________

@x
@p begin
c4p_begin_try_block(final_end);
initialize;@/
@y
@p begin initialize;@/
c4p_begin_try_block(final_end);
initialize;@/
@z

% _____________________________________________________________________________
%
% [8.100] System-dependent changes
% _____________________________________________________________________________

@x
@ We use an ``enumerated'' type to store the information.
@y
const n_options = 4; {Pascal won't count array lengths for us.}
      usage_help (TFTOPL_HELP, nil);
    end; {Else it was a flag; |getopt| has already done the assignment.}
@ An element with all zeros always ends the list.
@ We use an ``enumerated'' type to store the information.
@z
