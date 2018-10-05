%% hbf2gf-miktex.ch: CWEB change file for hbf2gf
%% 
%% Copyright (C) 1998-2016 Christian Schenk
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
#define WRITE_BIN   "w"
#define WRITE_TXT   "w"
#define READ_BIN    "r"
#define READ_TXT    "r"
@y
#define WRITE_BIN   "wb"
#define WRITE_TXT   "w"
#define READ_BIN    "rb"
#define READ_TXT    "r"
@z

@x
int main(int argc, char *argv[])
@y
int MIKTEXDLLEXPORT Main(int argc, char *argv[])
@z

% _____________________________________________________________________________
%
% [3.13]
% _____________________________________________________________________________

@x
@d header " hbf2gf output "
@y
@d headerxxx " hbf2gf output "
@z

% _____________________________________________________________________________
%
% [3.14]
% _____________________________________________________________________________

@x
    strcpy(out_s, header);@#
@y
    strcpy(out_s, headerxxx);@#
@z

% _____________________________________________________________________________
%
% [4.46]
% _____________________________________________________________________________

@x
                slant * (tfm_output_size_y + tfm_offset_y));
       }@#

    fclose(out);
@y
                slant * (tfm_output_size_y + tfm_offset_y));
       }@#
    fprintf(out, "\n");
    fclose(out);
@z

% _____________________________________________________________________________
%
% [8.69] File searching
% _____________________________________________________________________________

@x
#include "miktex.h"
@y
#if defined(__cplusplus)
#include <miktex/Core/Core>
#else
#include <miktex/Core/c/api.h>
#endif
@z

% _____________________________________________________________________________
%
% [8.80]
% _____________________________________________________________________________

@x
    if (!miktex_find_input_file("hbf2gf", *name, result))
@y
    if (!miktex_find_input_file("hbf2gf", name, result))
@z
 
@x
    if (!miktex_find_miscfont_file(*name, result))
@y
    if (!miktex_find_hbf_file(name, result))
@z
