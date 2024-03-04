% $Id$
% (public domain)
% 
% Date: Wed, 21 Feb 2024 21:32:57 +0000 (UTC)
% From: David Fuchs <drfuchs@yahoo.com>
% To: Karl Berry <karl@freefriends.org>
% Subject: Missed optimization in TeX: |loc=null| implies |state=token_list|
% 
% The final comment in <Recover from an unbalanced output routine>
% explicitly notes that |null=min_halfword<=0|, and it assumes we
% also know that during file input, |loc| can only be as small as
% the initial value of |first|, which is 1. So |loc|
% is strictly larger than |null| during file input (i.e. when
% |state<>token_list|).
% 
% So, |loc=null| implies |state=token_list|.
% Thus, |(state=token_list)and(loc=null) = (loc=null)| always.
% 
% Ergo, we can simplify two statements in TeX (two cases of "conserve
% stack space", the first of which is inner-loop; might save a whole cycle).
% 
% In |back_input|:
@x [23.325] l.7025
begin while (state=token_list)and(loc=null)and(token_type<>v_template) do
@y
begin while (loc=null)and(token_type<>v_template) do
@z

% In <Feed the macro body and its parameters to the scanner>:
@x [25.390] l.7983
while (state=token_list)and(loc=null)and(token_type<>v_template) do
@y
while (loc=null)and(token_type<>v_template) do
@z
