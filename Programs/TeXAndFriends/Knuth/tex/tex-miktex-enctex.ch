%%% tex-miktex-enctex.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

% _____________________________________________________________________________
%
% [24.341]
% _____________________________________________________________________________

@x
@!i,@!j: 0..buf_size; {more indexes for encTeX}
@y
@!i,@!j: 0..sup_buf_size; {more indexes for encTeX}
@z


% _____________________________________________________________________________
%
% [29.534]
% _____________________________________________________________________________

@x
    if translate_filename then
@y
    if miktex_have_tcx_file_name then
@z

% _____________________________________________________________________________
%
% [51.1337]
% _____________________________________________________________________________

@x
    if translate_filename then begin
@y
    if miktex_have_tcx_file_name then begin
@z

% _____________________________________________________________________________
%
% [58.1415]
% _____________________________________________________________________________

@x
@!enctex_enabled_p:boolean;  {enable encTeX}
@y
@!enctex_enabled_p:boolean;  {enable encTeX}

@ Declare system-dependent enctex functions.
@<Declare \MiKTeX\ functions@>=
function miktex_enctex_p : boolean; forward;
@z

% _____________________________________________________________________________
%
% [58.1416]
% _____________________________________________________________________________

@x
enctex_enabled_p:=false;
@y
enctex_p:=miktex_enctex_p;
enctex_enabled_p:=false;
@z
