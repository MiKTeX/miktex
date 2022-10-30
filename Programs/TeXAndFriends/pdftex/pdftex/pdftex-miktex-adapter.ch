%% pdftex-miktex-adapter.ch
%%
%% Copyright (C) 2021-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [1.2]
% _____________________________________________________________________________

@x
@d TeX_banner=='This is TeX, Version 3.141592653' {printed when \TeX\ starts}
@#
@d banner==pdfTeX_banner
@y
@d banner=='This is TeX, Version 3.141592653' {printed when \TeX\ starts}
@z

% _____________________________________________________________________________
%
% [18.254]
% _____________________________________________________________________________

@x
@d tex_int_pars=55 {total number of \TeX's integer parameters}
@#
@d pdftex_first_integer_code = tex_int_pars {base for \pdfTeX's integer parameters}
@y
@d int_pars=55 {total number of integer parameters}
@#
@d etex_int_base=tex_int_pars {base for \eTeX's integer parameters}
@z
