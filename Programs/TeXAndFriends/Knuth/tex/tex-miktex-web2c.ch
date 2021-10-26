%%% tex-miktex-web2c.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

@x
@* \[54/\MiKTeX] System-dependent changes for \MiKTeX.
@y
@* \[54/\MiKTeX] System-dependent changes for \MiKTeX.

@ Define Web2C compatibility functions.

@<Declare \MiKTeX\ functions@>=

function translate_filename : boolean; forward;@t\2@>@/
function eight_bit_p : boolean; forward;@t\2@>@/
function insert_src_special_auto : boolean; forward;@t\2@>@/
function src_specials_p : boolean; forward;@t\2@>@/
@z
