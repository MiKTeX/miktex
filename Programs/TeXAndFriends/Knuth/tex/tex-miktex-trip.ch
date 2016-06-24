%% tex-miktex-trip.ch: triptest changes
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

%% ////////////////////////////////////////////////////////////////////////////
%% //                                                                        //
%% //                      INSPIRED BY WEB2C'S TEX.CH                        //
%% //                                                                        //
%% ////////////////////////////////////////////////////////////////////////////

% _____________________________________________________________________________
%
% [1.11]
% _____________________________________________________________________________

@x
@!dvi_buf_size=8192; {size of the output buffer; must be a multiple of 8}
@y
@!dvi_buf_size=800; {size of the output buffer; must be a multiple of 8}
@z

@x
@!trie_op_size=35111; {space for ``opcodes'' in the hyphenation patterns}
@y
@!trie_op_size=500; {space for ``opcodes'' in the hyphenation patterns}
@z

% _____________________________________________________________________________
%
% [1.12]
% _____________________________________________________________________________

@x
@d hash_size=60000 {maximum number of control sequences; it should be at most
@y
@d hash_size=2100 {maximum number of control sequences; it should be at most
@z

@x
@d hash_prime=51001 {a prime number equal to about 85\pct! of |hash_size|}
@y
@d hash_prime=1777 {a prime number equal to about 85\pct! of |hash_size|}
@z
