%%% tex-miktex-synctex.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

% _____________________________________________________________________________
%
% [54.1379]
% _____________________________________________________________________________

@x
@* \[54/\MiKTeX] System-dependent changes for \MiKTeX.
@y
@* \[54/\MiKTeX] System-dependent changes for \MiKTeX.

@ @<Set init...@>=
synctexoption := miktex_get_synchronization_options;

@ @<Declare \MiKTeX\ functions@>=
function miktex_get_synchronization_options : integer; forward;@t\2@>@/
@z
