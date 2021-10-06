% $Id$
% Public domain. See partoken.ch (and pdfTeX manual) for main information.

% New internal constant for the main command |\partokenname|.
% This redefinition of max_command varies, but the rest of the change
% file works across all engines, so separate it out. This version is
% used for pdftex and xetex. For the pTeX family, the max_command update
% is done in the main change files (ptex-base.ch and uptex-m.ch).
@x
@d max_command=102 {the largest command code seen at |big_switch|}
@y
@d partoken_name=103 {set |par_token| name}
@d max_command=103 {the largest command code seen at |big_switch|}
@z
