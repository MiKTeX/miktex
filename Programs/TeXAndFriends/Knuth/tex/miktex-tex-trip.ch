%%% miktex-tex-trip.ch:
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
@!dvi_buf_size=8192; {size of the output buffer; must be a multiple of 8}
@y
@!dvi_buf_size=800; {size of the output buffer; must be a multiple of 8}
@z

@x
@!trie_op_size=35111; {space for ``opcodes'' in the hyphenation patterns}
@y
@!trie_op_size=500; {space for ``opcodes'' in the hyphenation patterns}
@z

% _____________________________________________________________________________
%
% [1.12]
% _____________________________________________________________________________

@x
@d hash_size=60000 {maximum number of control sequences; it should be at most
@y
@d hash_size=2100 {maximum number of control sequences; it should be at most
@z

@x
@d hash_prime=51001 {a prime number equal to about 85\pct! of |hash_size|}
@y
@d hash_prime=1777 {a prime number equal to about 85\pct! of |hash_size|}
@z
