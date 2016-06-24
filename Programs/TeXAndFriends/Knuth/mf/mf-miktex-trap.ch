%% mf-miktex-trap.ch: traptest changes for METAFONT
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

@x
@d debug==@{ {change this to `$\\{debug}\equiv\null$' when debugging}
@d gubed==@t@>@} {change this to `$\\{gubed}\equiv\null$' when debugging}
@y
@d debug==
@d gubed==
@z

 @x
@d stat==@{ {change this to `$\\{stat}\equiv\null$' when gathering
  usage statistics}
@d tats==@t@>@} {change this to `$\\{tats}\equiv\null$' when gathering
  usage statistics}
 @y
@d stat==
@d tats==
 @z

@x
@!screen_width=768; {number of pixels in each row of screen display}
@y
@!screen_width=100; {number of pixels in each row of screen display}
@z

@x
@!screen_depth=1024; {number of pixels in each column of screen display}
@y
@!screen_depth=200; {number of pixels in each column of screen display}
@z

@x
@!gf_buf_size=8192; {size of the output buffer, must be a multiple of 8}
@y
@!gf_buf_size=8; {size of the output buffer, must be a multiple of 8}
@z

