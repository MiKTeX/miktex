%%% pdftex-post.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

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
@d int_pars=web2c_int_pars {total number of integer parameters}
@#
@d pdftex_first_integer_code = tex_int_pars {base for \pdfTeX's integer parameters}
@y
@d pdftex_first_integer_code = web2c_int_pars {base for \pdfTeX's integer parameters}
@z
