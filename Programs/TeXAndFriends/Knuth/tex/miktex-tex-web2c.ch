%% miktex-tex-web2c.ch
%%
%% Derived from:
%% tex.ch for C compilation with web2c, derived from various other
%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%% others.

% _____________________________________________________________________________
%
% [50.1303]
% _____________________________________________________________________________

@x
@!x: integer; {something undumped}
@!w: four_quarters; {four ASCII codes}
@y
@!x: integer; {something undumped}
@!format_engine: ^text_char;
@!dummy_xord: ASCII_code;
@!dummy_xchr: text_char;
@!dummy_xprn: ASCII_code;
@z

% _____________________________________________________________________________
%
% [54.1379] \[54] System-dependent changes
% _____________________________________________________________________________

@x
@* \[54/ML\TeX] System-dependent changes for ML\TeX.
@y
@ Define Web2C shims.

@<Declare \MiKTeX\ functions@>=
function@?eight_bit_p : boolean; forward;@t\2@>@/
function@?insert_src_special_auto : boolean; forward;@t\2@>@/
function@?restrictedshell : boolean; forward;@t\2@>@/
function@?shellenabledp : boolean; forward;@t\2@>@/
function@?src_specials_p : boolean; forward;@t\2@>@/
function@?translate_filename : boolean; forward;@t\2@>@/


@* \[54/ML\TeX] System-dependent changes for ML\TeX.
@z
