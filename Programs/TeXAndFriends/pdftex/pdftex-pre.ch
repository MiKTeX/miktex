%%% pdftex-pre.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

@x
@d TeX_banner=='This is TeX, Version 3.141592653' {printed when \TeX\ starts}
@#
@d banner==pdfTeX_banner
@y
@d banner=='This is TeX, Version 3.141592653' {printed when \TeX\ starts}
@z

@x
@d tex_int_pars=55 {total number of \TeX's integer parameters}
@#
@d pdftex_first_integer_code = tex_int_pars {base for \pdfTeX's integer parameters}
@y
@d int_pars=55 {total number of integer parameters}
@#
@d etex_int_base=tex_int_pars {base for \eTeX's integer parameters}
@z
