%% tie-miktex.ch
%%
%% Copyright (C) 2000-2022 Christian Schenk
%% 
%% This file is free software; the copyright holder gives
%% unlimited permission to copy and/or distribute it, with or
%% without modifications, as long as this notice is preserved.

% _____________________________________________________________________________
%
% [1.5]
% _____________________________________________________________________________

@x
#define max_file_index 9
@y
#define max_file_index 100
@z

% _____________________________________________________________________________
%
% [2.11]
% _____________________________________________________________________________

@x
@d nl_mark    @'\n' /* \ASCII{} code used as line end marker */
@y
@d cr_mark    @'\r' /* \ASCII{} code used as carriage return marker */
@d nl_mark    @'\n' /* \ASCII{} code used as line end marker */
@z

% _____________________________________________________________________________
%
% [2.13]
% _____________________________________________________________________________

@x
xchr[nl_mark]='\n';
@y
xchr[cr_mark]='\r';
xchr[nl_mark]='\n';
@z

% _____________________________________________________________________________
%
% [5.27]
% _____________________________________________________________________________

@x
   if (c!=@' ' && c!=tab_mark)
@y
   if (c!=@' ' && c!=tab_mark && c!=cr_mark)
@z

% _____________________________________________________________________________
%
% [7.34]
% _____________________________________________________________________________

@x
    out_file=fopen(out_name,"w");
@y
    if(out_name[0] == '-' && out_name[1] == 0) out_file=stdout;
    else
    out_file=fopen(out_name,"w");
@z
