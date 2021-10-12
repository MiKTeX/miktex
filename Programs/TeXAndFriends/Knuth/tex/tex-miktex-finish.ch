%%% tex-miktex-finish.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

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
