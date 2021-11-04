%%% miktex-tex-hyph.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

% _____________________________________________________________________________
%
% [42.920]
% _____________________________________________________________________________

@x
Comparatively few different number sequences $n_0\ldots n_k$ actually occur,
since most of the |n|'s are generally zero. Therefore the number sequences
are encoded in such a way that |trie_op|$(z_k)$ is only one byte long.
If |trie_op(@t$z_k$@>)<>min_quarterword|, when $p_1\ldots p_k$ has matched
the letters in |hc[(l-k+1)..l@,]| of language |t|,
we perform all of the required operations
for this pattern by carrying out the following little program: Set
|v:=trie_op(@t$z_k$@>)|. Then set |v:=v+op_start[t]|,
|hyf[l-hyf_distance[v]]:=@tmax@>(hyf[l-hyf_distance[v]], hyf_num[v])|,
and |v:=hyf_next[v]|; repeat, if necessary, until |v=min_quarterword|.
@y
The theory that comparatively few different number sequences $n_0\ldots n_k$
actually occur, since most of the |n|'s are generally zero, seems to fail
at least for the large German hyphenation patterns.
Therefore the number sequences cannot any longer be encoded in such a way
that |trie_op|$(z_k)$ is only one byte long.
We have introduced a new constant |max_trie_op| for the maximum allowable
hyphenation operation code value; |max_trie_op| might be different for
\TeX\ and \.{INITEX} and must not exceed |max_halfword|.
An opcode will occupy a halfword if |max_trie_op| exceeds |max_quarterword|
or a quarterword otherwise.
@^system dependencies@>
If |trie_op(@t$z_k$@>)<>min_trie_op|, when $p_1\ldots p_k$ has matched
the letters in |hc[(l-k+1)..l@,]| of language |t|,
we perform all of the required operations
for this pattern by carrying out the following little program: Set
|v:=trie_op(@t$z_k$@>)|. Then set |v:=v+op_start[t]|,
|hyf[l-hyf_distance[v]]:=@tmax@>(hyf[l-hyf_distance[v]], hyf_num[v])|,
and |v:=hyf_next[v]|; repeat, if necessary, until |v=min_trie_op|.
@z

@x
@!trie_pointer=0..trie_size; {an index into |trie|}
@y
@!trie_pointer=0..sup_trie_size; {an index into |trie|}
@!trie_opcode=min_trie_op..max_trie_op; {a trie opcode}
@z

% _____________________________________________________________________________
%
% [42.921]
% _____________________________________________________________________________

@x
@ @d trie_link(#)==trie[#].rh {``downward'' link in a trie}
@d trie_char(#)==trie[#].b1 {character matched at this trie location}
@d trie_op(#)==trie[#].b0 {program for hyphenation at this trie location}
@y
@ For more than 255 trie op codes, the three fields |trie_link|, |trie_char|,
and |trie_op| will no longer fit into one memory word; thus using \MiKTeX\
we define |trie| as three array instead of an array of records.
The variant will be implemented by reusing the opcode field later on with
another macro.

@d trie_link(#)==trie_trl[#] {``downward'' link in a trie}
@d trie_char(#)==trie_trc[#] {character matched at this trie location}
@d trie_op(#)==trie_tro[#] {program for hyphenation at this trie location}
@z

@x
@!trie:array[trie_pointer] of two_halves; {|trie_link|, |trie_char|, |trie_op|}
@y
@!trie_trl:^trie_pointer; {|trie_link|}
@!trie_trc:^quarterword; {|trie_char|}
@!trie_tro:^trie_pointer; {|trie_op|}
@Z

@x
@!hyf_next:array[1..trie_op_size] of quarterword; {continuation code}
@y
@!hyf_next:array[1..trie_op_size] of trie_opcode; {continuation code}
@z

% _____________________________________________________________________________
%
% [42.923]
% _____________________________________________________________________________

@x
    begin if trie_op(z)<>min_quarterword then
@y
    begin if trie_op(z)<>min_trie_op then
@z

% _____________________________________________________________________________
%
% [42.924]
% _____________________________________________________________________________

@x
until v=min_quarterword;
@y
until v=min_trie_op;
@z

% _____________________________________________________________________________
%
% [42.925]
% _____________________________________________________________________________

@x
different from $\alpha$, we can conclude that $\alpha$ is not in the table.
@y
different from $\alpha$, we can conclude that $\alpha$ is not in the table.
This is a clever scheme which saves the need for a hash link array.
However, it is difficult to increase the size of the hyphen exception
arrays. To make this easier, the ordered hash has been replaced by
a simple hash, using an additional array |hyph_link|. The value
|0| in |hyph_link[k]| means that there are no more entries corresponding
to the specific hash chain. When |hyph_link[k]>0|, the next entry in
the hash chain is |hyph_link[k]-1|. This value is used because the
arrays start at |0|.
@z

@x
@!hyph_pointer=0..hyph_size; {an index into the ordered hash table}
@y
@!hyph_pointer=0..sup_hyph_size; {index into hyphen exceptions hash table;
                     enlarging this requires changing (un)dump code}
@z

% _____________________________________________________________________________
%
% [42.926]
% _____________________________________________________________________________

@x
@!hyph_word:array[hyph_pointer] of str_number; {exception words}
@!hyph_list:array[hyph_pointer] of pointer; {lists of hyphen positions}
@!hyph_count:hyph_pointer; {the number of words in the exception dictionary}
@y
@!hyph_word: ^str_number; {exception words}
@!hyph_list: ^pointer; {lists of hyphen positions}
@!hyph_link: ^hyph_pointer; {link array for hyphen exceptions hash table}
@!hyph_count:integer; {the number of words in the exception dictionary}
@!hyph_next:integer; {next free slot in hyphen exceptions hash table}
@z

% _____________________________________________________________________________
%
% [42.928]
% _____________________________________________________________________________

@x
for z:=0 to hyph_size do
  begin hyph_word[z]:=0; hyph_list[z]:=null;
  end;
hyph_count:=0;
@y
for z:=0 to hyph_size do
  begin hyph_word[z]:=0; hyph_list[z]:=null; hyph_link[z]:=0;
  end;
hyph_count:=0;
hyph_next:=hyph_prime+1; if hyph_next>hyph_size then hyph_next:=hyph_prime;
@z

% _____________________________________________________________________________
%
% [42.930]
% _____________________________________________________________________________

@x
h:=hc[1]; incr(hn); hc[hn]:=cur_lang;
for j:=2 to hn do h:=(h+h+hc[j]) mod hyph_size;
loop@+  begin @<If the string |hyph_word[h]| is less than \(hc)|hc[1..hn]|,
    |goto not_found|; but if the two strings are equal,
    set |hyf| to the hyphen positions and |goto found|@>;
  if h>0 then decr(h)@+else h:=hyph_size;
  end;
not_found: decr(hn)
@y
h:=hc[1]; incr(hn); hc[hn]:=cur_lang;
for j:=2 to hn do h:=(h+h+hc[j]) mod hyph_prime;
loop@+  begin @<If the string |hyph_word[h]| is less than \(hc)|hc[1..hn]|,
    |goto not_found|; but if the two strings are equal,
    set |hyf| to the hyphen positions and |goto found|@>;
  h:=hyph_link[h]; if h=0 then goto not_found;
  decr(h);
  end;
not_found: decr(hn)
@z

% _____________________________________________________________________________
%
% [42.931]
% _____________________________________________________________________________

@x
@ @<If the string |hyph_word[h]| is less than \(hc)...@>=
k:=hyph_word[h]; if k=0 then goto not_found;
if length(k)<hn then goto not_found;
@y
@ @<If the string |hyph_word[h]| is less than \(hc)...@>=
{This is now a simple hash list, not an ordered one, so
the module title is no longer descriptive.}
k:=hyph_word[h]; if k=0 then goto not_found;
@z

@x
  repeat if so(str_pool[u])<hc[j] then goto not_found;
  if so(str_pool[u])>hc[j] then goto done;
@y
  repeat
  if so(str_pool[u])<>hc[j] then goto done;
@z

% _____________________________________________________________________________
%
% [42.934]
% _____________________________________________________________________________

@x
@!s,@!t:str_number; {strings being compared or stored}
@y
@!s:str_number; {strings being compared or stored}
@z

% _____________________________________________________________________________
%
% [42.939]
% _____________________________________________________________________________

@x
  begin h:=(h+h+hc[j]) mod hyph_size;
@y
  begin h:=(h+h+hc[j]) mod hyph_prime;
@z

% _____________________________________________________________________________
%
% [42.940]
% _____________________________________________________________________________

@x
@ @<Insert the \(p)pair |(s,p)|...@>=
if hyph_count=hyph_size then overflow("exception dictionary",hyph_size);
@:TeX capacity exceeded exception dictionary}{\quad exception dictionary@>
incr(hyph_count);
while hyph_word[h]<>0 do
  begin @<If the string |hyph_word[h]| is less than \(or)or equal to
  |s|, interchange |(hyph_word[h],hyph_list[h])| with |(s,p)|@>;
  if h>0 then decr(h)@+else h:=hyph_size;
  end;
hyph_word[h]:=s; hyph_list[h]:=p
@y
@ @<Insert the \(p)pair |(s,p)|...@>=
  if hyph_next <= hyph_prime then
     while (hyph_next>0) and (hyph_word[hyph_next-1]>0) do decr(hyph_next);
if (hyph_count=hyph_size)or(hyph_next=0) then
   overflow("exception dictionary",hyph_size);
@:TeX capacity exceeded exception dictionary}{\quad exception dictionary@>
incr(hyph_count);
while hyph_word[h]<>0 do
  begin @<If the string |hyph_word[h]| is less than \(or)or equal to
  |s|, interchange |(hyph_word[h],hyph_list[h])| with |(s,p)|@>;
  if hyph_link[h]=0 then
  begin
    hyph_link[h]:=hyph_next;
    if hyph_next >= hyph_size then hyph_next:=hyph_prime;
    if hyph_next > hyph_prime then incr(hyph_next);
  end;
  h:=hyph_link[h]-1;
  end;

found: hyph_word[h]:=s; hyph_list[h]:=p
@z

% _____________________________________________________________________________
%
% [42.941]
% _____________________________________________________________________________

@x
@ @<If the string |hyph_word[h]| is less than \(or)...@>=
k:=hyph_word[h];
if length(k)<length(s) then goto found;
if length(k)>length(s) then goto not_found;
@y
@ @<If the string |hyph_word[h]| is less than \(or)...@>=
{This is now a simple hash list, not an ordered one, so
the module title is no longer descriptive.}
k:=hyph_word[h];
if length(k)<>length(s) then goto not_found;
@z

@x
repeat if str_pool[u]<str_pool[v] then goto found;
if str_pool[u]>str_pool[v] then goto not_found;
@y
repeat if str_pool[u]<>str_pool[v] then goto not_found;
@z

@x
found:q:=hyph_list[h]; hyph_list[h]:=p; p:=q;@/
t:=hyph_word[h]; hyph_word[h]:=s; s:=t;
not_found:
@y
{repeat hyphenation exception; flushing old data}
flush_string; s:=hyph_word[h]; {avoid |slow_make_string|!}
decr(hyph_count);
{ We could also |flush_list(hyph_list[h]);|, but it interferes
  with \.{trip.log}. }
goto found;
not_found:
@z

% _____________________________________________________________________________
%
% [43.943]
% _____________________________________________________________________________

@x
|hyf_next[@t$v^\prime$@>]=min_quarterword|.
@y
|hyf_next[@t$v^\prime$@>]=min_trie_op|.
@z

@x
$$\hbox{|@t$v^\prime$@>:=new_trie_op(0,1,min_quarterword)|,\qquad
@y
$$\hbox{|@t$v^\prime$@>:=new_trie_op(0,1,min_trie_op)|,\qquad
@z

@x
@!trie_used:array[ASCII_code] of quarterword;
@y
@!trie_used:array[ASCII_code] of trie_opcode;
@z

@x
@!trie_op_val:array[1..trie_op_size] of quarterword;
@y
@!trie_op_val:array[1..trie_op_size] of trie_opcode;
@z

@x
tini
@y
tini@;
@!max_op_used:trie_opcode; {largest opcode used for any language}
@!small_op:boolean; {flag used while dumping or undumping}
@z

% _____________________________________________________________________________
%
% [43.944]
% _____________________________________________________________________________

@x
|new_trie_op| could return |min_quarterword| (thereby simply ignoring
@y
|new_trie_op| could return |min_trie_op| (thereby simply ignoring
@z

@x
function new_trie_op(@!d,@!n:small_number;@!v:quarterword):quarterword;
label exit;
var h:-trie_op_size..trie_op_size; {trial hash location}
@!u:quarterword; {trial op code}
@y
function new_trie_op(@!d,@!n:small_number;@!v:trie_opcode):trie_opcode;
label exit;
var h:-trie_op_size..trie_op_size; {trial hash location}
@!u:trie_opcode; {trial op code}
@z

@x
begin h:=abs(n+313*d+361*v+1009*cur_lang) mod (trie_op_size+trie_op_size)
  - trie_op_size;
@y
begin h:=abs(n+313*d+361*v+1009*cur_lang) mod (trie_op_size-neg_trie_op_size)
  + neg_trie_op_size;
@z

@x
    if u=max_quarterword then
      overflow("pattern memory ops per language",
        max_quarterword-min_quarterword);
    incr(trie_op_ptr); incr(u); trie_used[cur_lang]:=u;
@y
    if u=max_trie_op then
      overflow("pattern memory ops per language",
      max_trie_op-min_trie_op);
    incr(trie_op_ptr); incr(u); trie_used[cur_lang]:=u;
    if u>max_op_used then max_op_used:=u;
@z

% _____________________________________________________________________________
%
% [43.945]
% _____________________________________________________________________________

@x
op_start[0]:=-min_quarterword;
@y
op_start[0]:=-min_trie_op;
@z


% _____________________________________________________________________________
%
% [43.946]
% _____________________________________________________________________________

@x
for k:=0 to 255 do trie_used[k]:=min_quarterword;
@y
for k:=0 to 255 do trie_used[k]:=min_trie_op;
@z

@x
trie_op_ptr:=0;
@y
max_op_used:=min_trie_op;
trie_op_ptr:=0;
@z

% _____________________________________________________________________________
%
% [43.947]
% _____________________________________________________________________________

@x
@!init @!trie_c:packed array[trie_pointer] of packed_ASCII_code;
  {characters to match}
@t\hskip10pt@>@!trie_o:packed array[trie_pointer] of quarterword;
  {operations to perform}
@t\hskip10pt@>@!trie_l:packed array[trie_pointer] of trie_pointer;
  {left subtrie links}
@t\hskip10pt@>@!trie_r:packed array[trie_pointer] of trie_pointer;
  {right subtrie links}
@t\hskip10pt@>@!trie_ptr:trie_pointer; {the number of nodes in the trie}
@t\hskip10pt@>@!trie_hash:packed array[trie_pointer] of trie_pointer;
  {used to identify equivalent subtries}
tini
@y
@!init @!trie_c:^packed_ASCII_code;
  {characters to match}
@t\hskip10pt@>@!trie_o:^trie_opcode;
  {operations to perform}
@t\hskip10pt@>@!trie_l:^trie_pointer;
  {left subtrie links}
@t\hskip10pt@>@!trie_r:^trie_pointer;
  {right subtrie links}
@t\hskip10pt@>@!trie_ptr:trie_pointer; {the number of nodes in the trie}
@t\hskip10pt@>@!trie_hash:^trie_pointer;
  {used to identify equivalent subtries}
tini
@z

% _____________________________________________________________________________
%
% [43.950]
% _____________________________________________________________________________

@x
@d trie_back(#)==trie[#].lh {backward links in |trie| holes}
@y
@d trie_back(#)==trie_tro[#] {use the opcode field now for backward links}
@z

@x
@!init @!trie_taken:packed array[1..trie_size] of boolean;
  {does a family start here?}
@t\hskip10pt@>@!trie_min:array[ASCII_code] of trie_pointer;
  {the first possible slot for each character}
@t\hskip10pt@>@!trie_max:trie_pointer; {largest location used in |trie|}
@t\hskip10pt@>@!trie_not_ready:boolean; {is the trie still in linked form?}
tini
@y
@!init @!trie_taken: ^boolean;
  {does a family start here?}
@t\hskip10pt@>@!trie_min:array[ASCII_code] of trie_pointer;
  {the first possible slot for each character}
@t\hskip10pt@>@!trie_max:trie_pointer; {largest location used in |trie|}
@t\hskip10pt@>@!trie_not_ready:boolean; {is the trie still in linked form?}
tini
@z

% _____________________________________________________________________________
%
% [43.951]
% _____________________________________________________________________________

@x
trie_not_ready:=true; trie_root:=0; trie_c[0]:=si(0); trie_ptr:=0;
@y
trie_not_ready:=true;
@z

% _____________________________________________________________________________
%
% [43.958]
% _____________________________________________________________________________

@x
@<Move the data into |trie|@>=
h.rh:=0; h.b0:=min_quarterword; h.b1:=min_quarterword; {|trie_link:=0|,
  |trie_op:=min_quarterword|, |trie_char:=qi(0)|}
@y
@d clear_trie == {clear |trie[r]|}
  begin trie_link(r):=0;
  trie_op(r):=min_trie_op;
  trie_char(r):=min_quarterword; {|trie_char:=qi(0)|}
  end

@<Move the data into |trie|@>=
@z

@x
  begin for r:=0 to 256 do trie[r]:=h;
@y
  begin for r:=0 to 256 do clear_trie;
@z

@x
  repeat s:=trie_link(r); trie[r]:=h; r:=s;
@y
  repeat s:=trie_link(r); clear_trie; r:=s;
@z

% _____________________________________________________________________________
%
% [43.960]
% _____________________________________________________________________________

@x
@!v:quarterword; {trie op code}
@y
@!v:trie_opcode; {trie op code}
@z

@x
if trie_o[q]<>min_quarterword then
@y
if trie_o[q]<>min_trie_op then
@z

% _____________________________________________________________________________
%
% [43.964]
% _____________________________________________________________________________

@x
trie_c[p]:=si(c); trie_o[p]:=min_quarterword;
@y
trie_c[p]:=si(c); trie_o[p]:=min_trie_op;
@z

% _____________________________________________________________________________
%
% [43.965]
% _____________________________________________________________________________

@x
l:=k; v:=min_quarterword;
@y
l:=k; v:=min_trie_op;
@z

% _____________________________________________________________________________
%
% [43.966]
% _____________________________________________________________________________

@x
@!h:two_halves; {template used to zero out |trie|'s holes}
@y
@z

% _____________________________________________________________________________
%
% [54.1307]
% _____________________________________________________________________________

@x
dump_int(hyph_size)
@y
dump_int(hyph_prime);
@z

% _____________________________________________________________________________
%
% [54.1308]
% _____________________________________________________________________________

@x
if x<>hyph_size then goto bad_fmt
@y
if x<>hyph_prime then goto bad_fmt;
@z

% _____________________________________________________________________________
%
% [54.1324]
% _____________________________________________________________________________

@x
dump_int(hyph_count);
for k:=0 to hyph_size do if hyph_word[k]<>0 then
  begin dump_int(k); dump_int(hyph_word[k]); dump_int(hyph_list[k]);
  end;
@y
dump_int(hyph_count);
if hyph_next <= hyph_prime then hyph_next:=hyph_size;
dump_int(hyph_next);{minimum value of |hyphen_size| needed}
for k:=0 to hyph_size do if hyph_word[k]<>0 then
  begin dump_int(k+65536*hyph_link[k]);
        {assumes number of hyphen exceptions does not exceed 65535}
   dump_int(hyph_word[k]); dump_int(hyph_list[k]);
  end;
@z

@x
print_ln; print_int(hyph_count); print(" hyphenation exception");
if hyph_count<>1 then print_char("s");
@y
print_ln; print_int(hyph_count);
if hyph_count<>1 then print(" hyphenation exceptions")
else print(" hyphenation exception");
@z

@x
for k:=0 to trie_max do dump_hh(trie[k]);
@y
dump_things(trie_trl[0], trie_max+1);
dump_things(trie_tro[0], trie_max+1);
dump_things(trie_trc[0], trie_max+1);
@z

@x
for k:=1 to trie_op_ptr do
  begin dump_int(hyf_distance[k]);
  dump_int(hyf_num[k]);
  dump_int(hyf_next[k]);
  end;
@y
dump_things(hyf_distance[1], trie_op_ptr);
dump_things(hyf_num[1], trie_op_ptr);
dump_things(hyf_next[1], trie_op_ptr);
@z

@x
print(" has "); print_int(trie_op_ptr); print(" op");
if trie_op_ptr<>1 then print_char("s");
@y
print(" has "); print_int(trie_op_ptr);
if trie_op_ptr<>1 then print(" ops")
else print(" op");
@z

% _____________________________________________________________________________
%
% [54.1325]
% _____________________________________________________________________________

@x
undump(0)(hyph_size)(hyph_count);
for k:=1 to hyph_count do
  begin undump(0)(hyph_size)(j);
  undump(0)(str_ptr)(hyph_word[j]);
  undump(min_halfword)(max_halfword)(hyph_list[j]);
  end;
@y
undump_size(0)(hyph_size)('hyph_size')(hyph_count);
undump_size(hyph_prime)(hyph_size)('hyph_size')(hyph_next);
j:=0;
for k:=1 to hyph_count do
  begin undump_int(j); if j<0 then goto bad_fmt;
   if j>65535 then
   begin hyph_next:= j div 65536; j:=j - hyph_next * 65536; end
       else hyph_next:=0;
   if (j>=hyph_size)or(hyph_next>hyph_size) then goto bad_fmt;
   hyph_link[j]:=hyph_next;
  undump(0)(str_ptr)(hyph_word[j]);
  undump(min_halfword)(max_halfword)(hyph_list[j]);
  end;
  {|j| is now the largest occupied location in |hyph_word|}
  incr(j);
  if j<hyph_prime then j:=hyph_prime;
  hyph_next:=j;
  if hyph_next >= hyph_size then hyph_next:=hyph_prime else
  if hyph_next >= hyph_prime then incr(hyph_next);
@z

@x
for k:=0 to j do undump_hh(trie[k]);
@y
{These first three haven't been allocated yet unless we're \.{INITEX};
 we do that precisely so we don't allocate more space than necessary.}
undump_things(trie_trl[0], j+1);
undump_things(trie_tro[0], j+1);
undump_things(trie_trc[0], j+1);
@z

@x
for k:=1 to j do
  begin undump(0)(63)(hyf_distance[k]); {a |small_number|}
  undump(0)(63)(hyf_num[k]);
  undump(min_quarterword)(max_quarterword)(hyf_next[k]);
  end;
@y
{I'm not sure we have such a strict limitation (64) on these values, so
 let's leave them unchecked.}
undump_things(hyf_distance[1], j);
undump_things(hyf_num[1], j);
undump_upper_check_things(max_trie_op, hyf_next[1], j);
@z

% _____________________________________________________________________________
%
% [51.1337]
% _____________________________________________________________________________

@x
fix_date_and_time;@/
@y
fix_date_and_time;@/

@!init
if trie_not_ready then begin {initex without format loaded}
  trie_root:=0; trie_c[0]:=si(0); trie_ptr:=0;
end;
tini@/
@z
