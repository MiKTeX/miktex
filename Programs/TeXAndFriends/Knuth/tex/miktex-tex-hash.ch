%%% miktex-tex-hash.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

% _____________________________________________________________________________
%
% [17.222]
% _____________________________________________________________________________

@x
for k:=active_base to undefined_control_sequence-1 do
  eqtb[k]:=eqtb[undefined_control_sequence];
@y
for k:=active_base to eqtb_top do
  eqtb[k]:=eqtb[undefined_control_sequence];
@z

% _____________________________________________________________________________
%
% [17.252]
% _____________________________________________________________________________

@x
else if n<glue_base then @<Show equivalent |n|, in region 1 or 2@>
@y
else if (n<glue_base) or ((n>eqtb_size)and(n<=eqtb_top)) then
  @<Show equivalent |n|, in region 1 or 2@>
@z

@x
@!eqtb:array[active_base..eqtb_size] of memory_word;
@y
@!zeqtb:^memory_word;
@!eqtb:^memory_word;
@z

% _____________________________________________________________________________
%
% [18.256]
% _____________________________________________________________________________

@x
@!hash: array[hash_base..undefined_control_sequence-1] of two_halves;
  {the hash table}
@!hash_used:pointer; {allocation pointer for |hash|}
@y
@!hash: ^two_halves; {the hash table}
@!yhash: ^two_halves; {auxiliary pointer for freeing hash}
@!hash_used:pointer; {allocation pointer for |hash|}
@!hash_extra:pointer; {|hash_extra=hash| above |eqtb_size|}
@!hash_top:pointer; {maximum of the hash array}
@!eqtb_top:pointer; {maximum of the |eqtb|}
@!hash_high:pointer; {pointer to next high hash location}
@z

% _____________________________________________________________________________
%
% [18.257]
% _____________________________________________________________________________

@x
next(hash_base):=0; text(hash_base):=0;
for k:=hash_base+1 to undefined_control_sequence-1 do hash[k]:=hash[hash_base];
@y
@z

% _____________________________________________________________________________
%
% [18.258]
% _____________________________________________________________________________

@x
hash_used:=frozen_control_sequence; {nothing is used}
@y
hash_used:=frozen_control_sequence; {nothing is used}
hash_high:=0;
@z

% _____________________________________________________________________________
%
% [18.260]
% _____________________________________________________________________________

@x
@ @<Insert a new control...@>=
begin if text(p)>0 then
  begin repeat if hash_is_full then overflow("hash size",hash_size);
@:TeX capacity exceeded hash size}{\quad hash size@>
  decr(hash_used);
  until text(hash_used)=0; {search for an empty location in |hash|}
  next(p):=hash_used; p:=hash_used;
  end;
@y
@ @<Insert a new control...@>=
begin if text(p)>0 then
  begin if hash_high<hash_extra then
      begin incr(hash_high);
      next(p):=hash_high+eqtb_size; p:=hash_high+eqtb_size;
      end
    else begin
      repeat if hash_is_full then overflow("hash size",hash_size+hash_extra);
@:TeX capacity exceeded hash size}{\quad hash size@>
      decr(hash_used);
      until text(hash_used)=0; {search for an empty location in |hash|}
    next(p):=hash_used; p:=hash_used;
    end;
  end;
@z

% _____________________________________________________________________________
%
% [18.262]
% _____________________________________________________________________________

@x
else if p>=undefined_control_sequence then print_esc("IMPOSSIBLE.")
@y
else if ((p>=undefined_control_sequence)and(p<=eqtb_size))or(p>eqtb_top) then
  print_esc("IMPOSSIBLE.")
@z

@x
else if (text(p)<0)or(text(p)>=str_ptr) then print_esc("NONEXISTENT.")
@y
else if (text(p)>=str_ptr) then print_esc("NONEXISTENT.")
@z

% _____________________________________________________________________________
%
% [19.283]
% _____________________________________________________________________________

@x
if p<int_base then
@y
if (p<int_base)or(p>eqtb_size) then
@z

% _____________________________________________________________________________
%
% [20.290]
% _____________________________________________________________________________

@x
if cs_token_flag+undefined_control_sequence>max_halfword then bad:=21;
@y
if cs_token_flag+eqtb_size+hash_extra>max_halfword then bad:=21;
if (hash_offset<0)or(hash_offset>hash_base) then bad:=42;
@z

% _____________________________________________________________________________
%
% [49.1215]
% _____________________________________________________________________________

@x
if (cur_cs=0)or(cur_cs>frozen_control_sequence) then
@y
if (cur_cs=0)or(cur_cs>eqtb_top)or
  ((cur_cs>frozen_control_sequence)and(cur_cs<=eqtb_size)) then
@z

% _____________________________________________________________________________
%
% [50.1307]
% _____________________________________________________________________________

@x
dump_int(max_halfword);@/
@y
dump_int(max_halfword);@/
dump_int(hash_high);
@z

% _____________________________________________________________________________
%
% [50.1308]
% _____________________________________________________________________________

@x
if x<>max_halfword then goto bad_fmt; {check |max_halfword|}
@y
if x<>max_halfword then goto bad_fmt; {check |max_halfword|}
undump_int(hash_high);
  if (hash_high<0)or(hash_high>sup_hash_extra) then goto bad_fmt;
  if hash_extra<hash_high then hash_extra:=hash_high;
  eqtb_top:=eqtb_size+hash_extra;
  if hash_extra=0 then hash_top:=undefined_control_sequence else
        hash_top:=eqtb_top;
  yhash:=miktex_reallocate(yhash, 1+hash_top-hash_offset);
  hash:=yhash - hash_offset;
  next(hash_base):=0; text(hash_base):=0;
  for x:=hash_base+1 to hash_top do hash[x]:=hash[hash_base];
  zeqtb:=miktex_reallocate(zeqtb, eqtb_top+1);
  eqtb:=zeqtb;

  eq_type(undefined_control_sequence):=undefined_cs;
  equiv(undefined_control_sequence):=null;
  eq_level(undefined_control_sequence):=level_zero;
  for x:=eqtb_size+1 to eqtb_top do
    eqtb[x]:=eqtb[undefined_control_sequence];
@z

% _____________________________________________________________________________
%
% [50.1314]
% _____________________________________________________________________________

@x
undump(hash_base)(frozen_control_sequence)(par_loc);
par_token:=cs_token_flag+par_loc;@/
undump(hash_base)(frozen_control_sequence)(write_loc);@/
@y
undump(hash_base)(hash_top)(par_loc);
par_token:=cs_token_flag+par_loc;@/
undump(hash_base)(hash_top)(write_loc);@/
@z

% _____________________________________________________________________________
%
% [50.1315]
% _____________________________________________________________________________

@x
while k<l do
  begin dump_wd(eqtb[k]); incr(k);
  end;
@y
dump_things(eqtb[k], l-k);
@z

% _____________________________________________________________________________
%
% [50.1316]
% _____________________________________________________________________________

@x
while k<l do
  begin dump_wd(eqtb[k]); incr(k);
  end;
@y
dump_things(eqtb[k], l-k);
@z

@x
k:=j+1; dump_int(k-l);
until k>eqtb_size
@y
k:=j+1; dump_int(k-l);
until k>eqtb_size;
if hash_high>0 then dump_things(eqtb[eqtb_size+1],hash_high);
  {dump |hash_extra| part}
@z

% _____________________________________________________________________________
%
% [50.1317]
% _____________________________________________________________________________

@x
for j:=k to k+x-1 do undump_wd(eqtb[j]);
@y
undump_things(eqtb[k], x);
@z

@x
until k>eqtb_size
@y
until k>eqtb_size;
if hash_high>0 then undump_things(eqtb[eqtb_size+1],hash_high);
  {undump |hash_extra| part}
@z

% _____________________________________________________________________________
%
% [50.1318]
% _____________________________________________________________________________

@x
dump_int(hash_used); cs_count:=frozen_control_sequence-1-hash_used;
@y
dump_int(hash_used); cs_count:=frozen_control_sequence-1-hash_used+hash_high;
@z

@x
for p:=hash_used+1 to undefined_control_sequence-1 do dump_hh(hash[p]);
@y
dump_things(hash[hash_used+1], undefined_control_sequence-1-hash_used);
if hash_high>0 then dump_things(hash[eqtb_size+1], hash_high);
@z

% _____________________________________________________________________________
%
% [50.1319]
% _____________________________________________________________________________

@x
for p:=hash_used+1 to undefined_control_sequence-1 do undump_hh(hash[p]);
@y
undump_things (hash[hash_used+1], undefined_control_sequence-1-hash_used);
if debug_format_file then begin
  print_csnames (hash_base, undefined_control_sequence - 1);
end;
if hash_high > 0 then begin
  undump_things (hash[eqtb_size+1], hash_high);
  if debug_format_file then begin
    print_csnames (eqtb_size + 1, hash_high - (eqtb_size + 1));
  end;
end;
@z

% _____________________________________________________________________________
%
% [51.1332]
% _____________________________________________________________________________

@x
miktex_allocate_memory;
@y
miktex_allocate_memory;
@+Init
  eqtb_top := eqtb_size+hash_extra;
  if hash_extra=0 then hash_top:=undefined_control_sequence else
        hash_top:=eqtb_top;
  yhash:=miktex_reallocate(yhash, 1+hash_top-hash_offset);
  hash:=yhash - hash_offset;   {Some compilers require |hash_offset=0|}
  next(hash_base):=0; text(hash_base):=0;
  for hash_used:=hash_base+1 to hash_top do hash[hash_used]:=hash[hash_base];
  zeqtb:=miktex_reallocate(zeqtb, eqtb_top);
  eqtb:=zeqtb;
@+Tini
@z

% _____________________________________________________________________________
%
% [51.1334]
% _____________________________________________________________________________

@x
  wlog_ln(' ',cs_count:1,' multiletter control sequences out of ',
    hash_size:1);@/
@y
  wlog_ln(' ',cs_count:1,' multiletter control sequences out of ',
    hash_size:1, '+', hash_extra:1);@/
@z

% _____________________________________________________________________________
%
% [51.1337]
% _____________________________________________________________________________

@x
  w_close(fmt_file);
@y
  w_close(fmt_file);
  eqtb:=zeqtb;
@z
