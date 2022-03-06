%% miktex-tex-finish.ch
%%
%% Derived from:
%% tex.ch for C compilation with web2c, derived from various other
%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%% others.

% _____________________________________________________________________________
%
% [1.11]
% _____________________________________________________________________________

@x
@!pool_name=TEXMF_POOL_NAME; {this is configurable, for the sake of ML-\TeX}
  {string of length |file_name_size|; tells where the string pool appears}
@!engine_name=TEXMF_ENGINE_NAME; {the name of this engine}
@y
@z

% _____________________________________________________________________________
%
% [24.341]
% _____________________________________________________________________________

@x
var k:0..buf_size; {an index into |buffer|}
@y
var k:0..sup_buf_size; {an index into |buffer|}
@z

% _____________________________________________________________________________
%
% [24.363]
% _____________________________________________________________________________

@x
var k:0..buf_size; {an index into |buffer|}
@y
var k:0..sup_buf_size; {an index into |buffer|}
@z

% _____________________________________________________________________________
%
% [54.1379] \[54] System-dependent changes
% _____________________________________________________________________________

@x
@* \[54] System-dependent changes.
This section should be replaced, if necessary, by any special
modifications of the program
that are necessary to make \TeX\ work at a particular installation.
It is usually best to design your change file so that all changes to
previous sections preserve the section numbering; then everybody's version
will be consistent with the published program. More extensive changes,
which introduce new sections, can be inserted here; then only the index
itself will get a new section number.
@^system dependencies@>
@y
@* \[54] System-dependent changes.
@z
