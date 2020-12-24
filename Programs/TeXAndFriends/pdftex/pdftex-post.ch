%% pdftex-post.ch:
%% 
%% Copyright (C) 2003-2020 Christian Schenk
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
% [1.2]
% _____________________________________________________________________________

@x
@d banner==TeX_banner
@d banner_k==TeX_banner_k
@y
@d banner==pdfTeX_banner
@d banner_k==pdfTeX_banner
@z

% _____________________________________________________________________________
%
% [18.254]
% _____________________________________________________________________________

@x
@d int_pars=miktex_int_pars {total number of integer parameters}
@#
@d pdftex_first_integer_code = tex_int_pars {base for \pdfTeX's integer parameters}
@y
@d pdftex_first_integer_code = miktex_int_pars {base for \pdfTeX's integer parameters}
@z
