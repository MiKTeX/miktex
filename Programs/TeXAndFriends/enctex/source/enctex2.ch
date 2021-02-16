% $Id$
% enctex2.ch: bulk of the encTeX implementation, common to all engines
% supporting encTeX.

% encTeX: \mubytein \mubyteout \mubytelog and \specialout
@x [17.236] l.5016
@d tracing_char_sub_def==int_par(tracing_char_sub_def_code)
@y
@d tracing_char_sub_def==int_par(tracing_char_sub_def_code)
@d mubyte_in==int_par(mubyte_in_code)
@d mubyte_out==int_par(mubyte_out_code)
@d mubyte_log==int_par(mubyte_log_code)
@d spec_out==int_par(spec_out_code)
@z

% encTeX: \mubytein \mubyteout \mubytelog and \specialout
@x [17.237] l.5080
tracing_char_sub_def_code:print_esc("tracingcharsubdef");
@y
tracing_char_sub_def_code:print_esc("tracingcharsubdef");
mubyte_in_code:print_esc("mubytein");
mubyte_out_code:print_esc("mubyteout");
mubyte_log_code:print_esc("mubytelog");
spec_out_code:print_esc("specialout");
@z

% encTeX: \mubytein \mubyteout \mubytelog and \specialout
@x [17.238] l.5200
@!@:tracing_char_sub_def_}{\.{\\tracingcharsubdef} primitive@>
  end;
@y
@!@:tracing_char_sub_def_}{\.{\\tracingcharsubdef} primitive@>
  end;
if enctex_p then
  begin enctex_enabled_p:=true;
  primitive("mubytein",assign_int,int_base+mubyte_in_code);@/
@!@:mubyte_in_}{\.{\\mubytein} primitive@>
  primitive("mubyteout",assign_int,int_base+mubyte_out_code);@/
@!@:mubyte_out_}{\.{\\mubyteout} primitive@>
  primitive("mubytelog",assign_int,int_base+mubyte_log_code);@/
@!@:mubyte_log_}{\.{\\mubytelog} primitive@>
  primitive("specialout",assign_int,int_base+spec_out_code);@/
@!@:spec_out_}{\.{\\specialout} primitive@>
end;
@z

@x [18.262] - encTeX: control sequence to byte sequence
they may be unprintable.

@<Basic printing...@>=
procedure print_cs(@!p:integer); {prints a purported control sequence}
begin if p<hash_base then {single character}
@y
they may be unprintable.

The conversion from control sequence to byte sequence for enc\TeX is
implemented here. Of course, the simplest way is to implement an array
of string pointers with |hash_size| length, but we assume that only a
few control sequences will need to be converted. So |mubyte_cswrite|,
an array with only 128 items, is used. The items point to the token
lists. First token includes a csname number and the second points the
string to be output. The third token includes the number of another
csname and fourth token its pointer to the string etc. We need to do
the sequential searching in one of the 128 token lists.

@<Basic printing...@>=
procedure print_cs(@!p:integer); {prints a purported control sequence}
var q: pointer;
    s: str_number;
begin
  if active_noconvert and (not no_convert) and
     (eq_type(p) = let) and (equiv(p) = normal+11) then { noconvert }
  begin
     no_convert := true;
     return;
  end;
  s := 0;
  if cs_converting and (not no_convert) then
  begin
    q := mubyte_cswrite [p mod 128] ;
    while q <> null do
    if info (q) = p then
    begin
      s := info (link(q)); q := null;
    end else  q := link (link (q));
  end;
  no_convert := false;
  if s > 0 then print (s)
  else if p<hash_base then {single character}
@z

@x [18.262] - encTeX: exit label for print_cs
  print_char(" ");
  end;
end;
@y
  print_char(" ");
  end;
exit: end;
@z

@x [18.265] - encTeX: \endmubyte primitive
primitive("endcsname",end_cs_name,0);@/
@!@:end_cs_name_}{\.{\\endcsname} primitive@>
@y
primitive("endcsname",end_cs_name,0);@/
@!@:end_cs_name_}{\.{\\endcsname} primitive@>
if enctex_p then
begin
  primitive("endmubyte",end_cs_name,10);@/
@!@:end_mubyte_}{\.{\\endmubyte} primitive@>
end;
@z

@x [18.266] - encTeX: \endmubyte primitive
end_cs_name: print_esc("endcsname");
@y
end_cs_name: if chr_code = 10 then print_esc("endmubyte")
             else print_esc("endcsname");
@z

@x [22.318] encTeX - native buffer printing
if j>0 then for i:=start to j-1 do
  begin if i=loc then set_trick_count;
  print(buffer[i]);
  end
@y
i := start; mubyte_skeep := mubyte_keep;
mubyte_sstart := mubyte_start; mubyte_start := false;
if j>0 then while i < j do
begin
  if i=loc then set_trick_count;
  print_buffer(i);
end;
mubyte_keep := mubyte_skeep; mubyte_start := mubyte_sstart
@z

@x [24.332] encTeX: insert the added functions
appear on that line. (There might not be any tokens at all, if the
|end_line_char| has |ignore| as its catcode.)
@y
appear on that line. (There might not be any tokens at all, if the
|end_line_char| has |ignore| as its catcode.)

Some additional routines used by the enc\TeX extension have to be
declared at this point.

@p @t\4@>@<Declare additional routines for enc\TeX@>@/
@z

@x [24.341] - encTeX: more declarations in expand processor
var k:0..buf_size; {an index into |buffer|}
@!t:halfword; {a token}
@y
var k:0..buf_size; {an index into |buffer|}
@!t:halfword; {a token}
@!i,@!j: 0..buf_size; {more indexes for encTeX}
@!mubyte_incs: boolean; {control sequence is converted by mubyte}
@!p:pointer;  {for encTeX test if noexpanding}
@z

@x [24.343] - encTeX: access the buffer via read_buffer
  begin cur_chr:=buffer[loc]; incr(loc);
@y
  begin
    { Use |k| instead of |loc| for type correctness. }
    k := loc;
    cur_chr := read_buffer (k);
    loc := k; incr (loc);
    if (mubyte_token > 0) then
    begin
      state := mid_line;
      cur_cs := mubyte_token - cs_token_flag;
      goto found;
    end;
@z

@x [24.354] - encTeX: access the buffer via read_buffer
else  begin start_cs: k:=loc; cur_chr:=buffer[k]; cat:=cat_code(cur_chr);
  incr(k);
@y
else  begin start_cs:
   mubyte_incs := false; k := loc; mubyte_skeep := mubyte_keep;
   cur_chr := read_buffer (k); cat := cat_code (cur_chr);
   if (mubyte_in>0) and (not mubyte_incs) and
     ((mubyte_skip>0) or (cur_chr<>buffer[k])) then mubyte_incs := true;
   incr (k);
   if mubyte_token > 0 then
   begin
     state := mid_line;
     cur_cs := mubyte_token - cs_token_flag;
     goto found;
   end;
@z

@x [24.354] - encTeX: noexpanding the marked control sequence
  cur_cs:=single_base+buffer[loc]; incr(loc);
  end;
found: cur_cmd:=eq_type(cur_cs); cur_chr:=equiv(cur_cs);
if cur_cmd>=outer_call then check_outer_validity;
@y
  mubyte_keep := mubyte_skeep;
  cur_cs:=single_base + read_buffer(loc); incr(loc);
  end;
found: cur_cmd:=eq_type(cur_cs); cur_chr:=equiv(cur_cs);
if cur_cmd>=outer_call then check_outer_validity;
if write_noexpanding then
begin
  p := mubyte_cswrite [cur_cs mod 128];
  while p <> null do
    if info (p) = cur_cs then
    begin
      cur_cmd := relax; cur_chr := 256; p := null;
    end else p := link (link (p));
end;
@z

@x [24.355] - encTeX: deactivated when reading such \^^ab control sequences
    limit:=limit-d; first:=first-d;
@y
    limit:=limit-d; first:=first-d;
    if mubyte_in>0 then mubyte_keep := k-loc;
@z

@x [24.356] - encTeX: access the buffer via read_buffer
begin repeat cur_chr:=buffer[k]; cat:=cat_code(cur_chr); incr(k);
until (cat<>letter)or(k>limit);
@<If an expanded...@>;
if cat<>letter then decr(k);
  {now |k| points to first nonletter}
if k>loc+1 then {multiletter control sequence has been scanned}
  begin cur_cs:=id_lookup(loc,k-loc); loc:=k; goto found;
  end;
end
@y
begin
  repeat cur_chr := read_buffer (k); cat := cat_code (cur_chr);
    if mubyte_token>0 then cat := escape;
    if (mubyte_in>0) and (not mubyte_incs) and (cat=letter) and
      ((mubyte_skip>0) or (cur_chr<>buffer[k])) then mubyte_incs := true;
    incr (k);
  until (cat <> letter) or (k > limit);
  @<If an expanded...@>;
  if cat <> letter then
  begin
    decr (k); k := k - mubyte_skip;
  end;
  if k > loc + 1 then { multiletter control sequence has been scanned }
  begin
    if mubyte_incs then { multibyte in csname occurrs }
    begin
      i := loc; j := first; mubyte_keep := mubyte_skeep;
      if j - loc + k > max_buf_stack then
      begin
        max_buf_stack := j - loc + k;
        if max_buf_stack >= buf_size then
        begin
          max_buf_stack := buf_size;
          overflow ("buffer size", buf_size);
        end;
      end;
      while i < k do
      begin
        buffer [j] := read_buffer (i);
        incr (i); incr (j);
      end;
      if j = first+1 then
        cur_cs := single_base + buffer [first]
      else
        cur_cs := id_lookup (first, j-first);
    end else cur_cs := id_lookup (loc, k-loc) ;
    loc := k;
    goto found;
  end;
end
@z

@x [24.357] - encTeX: noexpanding the marked control sequence
      else check_outer_validity;
@y
      else check_outer_validity;
    if write_noexpanding then
    begin
      p := mubyte_cswrite [cur_cs mod 128];
      while p <> null do
        if info (p) = cur_cs then
        begin
          cur_cmd := relax; cur_chr := 256; p := null;
        end else p := link (link (p));
    end;
@z

@x [24.363] encTeX - native buffer printing
  if start<limit then for k:=start to limit-1 do print(buffer[k]);
@y
  k := start;
  while k < limit do begin print_buffer(k) end;
@z

@x [25.372] - encTeX: we need to distinguish \endcsname and \endmubyte
if cur_cmd<>end_cs_name then @<Complain about missing \.{\\endcsname}@>;
@y
if (cur_cmd<>end_cs_name) or (cur_chr<>0) then @<Complain about missing \.{\\endcsname}@>;
@z

@x [26.414] l.8358 - encTeX: accessing xord/xchr/xprn
if m=math_code_base then scanned_result(ho(math_code(cur_val)))(int_val)
@y
if m=xord_code_base then scanned_result(xord[cur_val])(int_val)
else if m=xchr_code_base then scanned_result(xchr[cur_val])(int_val)
else if m=xprn_code_base then scanned_result(xprn[cur_val])(int_val)
else if m=math_code_base then scanned_result(ho(math_code(cur_val)))(int_val)
@z

@x [29.534] l.10293 - enc\TeX: add enc\TeX banner after loading fmt file
  begin wlog_cr; wlog('MLTeX v2.2 enabled');
  end;
@y
  begin wlog_cr; wlog('MLTeX v2.2 enabled');
  end;
if enctex_enabled_p then
  begin wlog_cr; wlog(encTeX_banner); wlog(', reencoding enabled');
    if translate_filename then
      begin wlog_cr;
        wlog(' (\xordcode, \xchrcode, \xprncode overridden by TCX)');
    end;
  end;
@z

@x [48.1138] l.21648 - encTeX: \endmubyte primitive
begin print_err("Extra "); print_esc("endcsname");
@.Extra \\endcsname@>
help1("I'm ignoring this, since I wasn't doing a \csname.");
@y
begin
if cur_chr = 10 then
begin
  print_err("Extra "); print_esc("endmubyte");
@.Extra \\endmubyte@>
  help1("I'm ignoring this, since I wasn't doing a \mubyte.");
end else begin
  print_err("Extra "); print_esc("endcsname");
@.Extra \\endcsname@>
  help1("I'm ignoring this, since I wasn't doing a \csname.");
end;
@z

@x [49.1211] - encTeX: extra variables for \mubyte primitive
@!p,@!q:pointer; {for temporary short-term use}
@y
@!p,@!q,@!r:pointer; {for temporary short-term use}
@z

@x [49.1219] - encTeX: \mubyte and \noconvert primitives
primitive("futurelet",let,normal+1);@/
@!@:future_let_}{\.{\\futurelet} primitive@>
@y
primitive("futurelet",let,normal+1);@/
@!@:future_let_}{\.{\\futurelet} primitive@>
if enctex_p then
begin
  primitive("mubyte",let,normal+10);@/
@!@:mubyte_}{\.{\\mubyte} primitive@>
  primitive("noconvert",let,normal+11);@/
@!@:noconvert_}{\.{\\noconvert} primitive@>
end;
@z

@x [49.1220] - encTeX: \mubyte primitive
let: if chr_code<>normal then print_esc("futurelet")@+else print_esc("let");
@y
let: if chr_code<>normal then
      if chr_code = normal+10 then print_esc("mubyte")
      else if chr_code = normal+11 then print_esc("noconvert")
      else print_esc("futurelet")
  else print_esc("let");
@z

@x [49.1221] - encTeX: \mubyte primitive
let:  begin n:=cur_chr;
@y
let:  if cur_chr = normal+11 then do_nothing  { noconvert primitive }
      else if cur_chr = normal+10 then        { mubyte primitive }
      begin
        selector:=term_and_log;
        get_token;
        mubyte_stoken := cur_tok;
        if cur_tok <= cs_token_flag then mubyte_stoken := cur_tok mod 256;
        mubyte_prefix := 60;  mubyte_relax := false;
        mubyte_tablein := true; mubyte_tableout := true;
        get_x_token;
        if cur_cmd = spacer then get_x_token;
        if cur_cmd = sub_mark then
        begin
          mubyte_tableout := false; get_x_token;
          if cur_cmd = sub_mark then
          begin
            mubyte_tableout := true; mubyte_tablein := false;
            get_x_token;
          end;
        end else if (mubyte_stoken > cs_token_flag) and
                    (cur_cmd = mac_param) then
                 begin
                   mubyte_tableout := false;
                   scan_int; mubyte_prefix := cur_val; get_x_token;
                   if mubyte_prefix > 50 then mubyte_prefix := 52;
                   if mubyte_prefix <= 0 then mubyte_prefix := 51;
                 end
        else if (mubyte_stoken > cs_token_flag) and (cur_cmd = relax) then
             begin
               mubyte_tableout := true; mubyte_tablein := false;
               mubyte_relax := true; get_x_token;
             end;
        r := get_avail; p := r;
        while cur_cs = 0 do begin store_new_token (cur_tok); get_x_token; end;
        if (cur_cmd <> end_cs_name) or (cur_chr <> 10) then
        begin
          print_err("Missing "); print_esc("endmubyte"); print(" inserted");
          help2("The control sequence marked <to be read again> should")@/
("not appear in <byte sequence> between \mubyte and \endmubyte.");
          back_error;
        end;
        p := link(r);
        if (p = null) and mubyte_tablein then
        begin
          print_err("The empty <byte sequence>, ");
          print_esc("mubyte"); print(" ignored");
          help2("The <byte sequence> in")@/
("\mubyte <token> <byte sequence>\endmubyte should not be empty.");
          error;
        end else begin
          while p <> null do
          begin
            append_char (info(p) mod 256);
            p := link (p);
          end;
          flush_list (r);
          if (str_start [str_ptr] + 1 = pool_ptr) and
            (str_pool [pool_ptr-1] = mubyte_stoken) then
          begin
            if mubyte_read [mubyte_stoken] <> null
               and mubyte_tablein then  { clearing data }
                  dispose_munode (mubyte_read [mubyte_stoken]);
            if mubyte_tablein then mubyte_read [mubyte_stoken] := null;
            if mubyte_tableout then mubyte_write [mubyte_stoken] := 0;
            pool_ptr := str_start [str_ptr];
          end else begin
            if mubyte_tablein then mubyte_update;    { updating input side }
            if mubyte_tableout then  { updating output side }
            begin
              if mubyte_stoken > cs_token_flag then { control sequence }
              begin
                dispose_mutableout (mubyte_stoken-cs_token_flag);
                if (str_start [str_ptr] < pool_ptr) or mubyte_relax then
                begin       { store data }
                  r := mubyte_cswrite[(mubyte_stoken-cs_token_flag) mod 128];
                  p := get_avail;
                  mubyte_cswrite[(mubyte_stoken-cs_token_flag) mod 128] := p;
                  info (p) := mubyte_stoken-cs_token_flag;
                  link (p) := get_avail;
                  p := link (p);
                  if mubyte_relax then begin
                    info (p) := 0; pool_ptr := str_start [str_ptr];
                  end else info (p) := slow_make_string;
                  link (p) := r;
                end;
              end else begin                       { single character  }
                if str_start [str_ptr] = pool_ptr then
                  mubyte_write [mubyte_stoken] := 0
                else
                  mubyte_write [mubyte_stoken] := slow_make_string;
              end;
            end else pool_ptr := str_start [str_ptr];
          end;
        end;
      end else begin   { let primitive }
        n:=cur_chr;
@z

@x [49.1230] l.22936 - encTeX: \xordcode, \xchrcode, \xprncode primitives
primitive("catcode",def_code,cat_code_base);
@!@:cat_code_}{\.{\\catcode} primitive@>
@y
primitive("catcode",def_code,cat_code_base);
@!@:cat_code_}{\.{\\catcode} primitive@>
if enctex_p then
begin
  primitive("xordcode",def_code,xord_code_base);
@!@:xord_code_}{\.{\\xordcode} primitive@>
  primitive("xchrcode",def_code,xchr_code_base);
@!@:xchr_code_}{\.{\\xchrcode} primitive@>
  primitive("xprncode",def_code,xprn_code_base);
@!@:xprn_code_}{\.{\\xprncode} primitive@>
end;
@z

@x [49.1231] l.22956 - encTeX: \xordcode, \xchrcode, \xprncode primitives
def_code: if chr_code=cat_code_base then print_esc("catcode")
@y
def_code: if chr_code=xord_code_base then print_esc("xordcode")
  else if chr_code=xchr_code_base then print_esc("xchrcode")
  else if chr_code=xprn_code_base then print_esc("xprncode")
  else if chr_code=cat_code_base then print_esc("catcode")
@z

@x [49.1232] l.22969 - encTeX: setting a new value to xchr/xord/xprn
  p:=cur_chr; scan_char_num; p:=p+cur_val; scan_optional_equals;
  scan_int;
@y
  p:=cur_chr; scan_char_num;
  if p=xord_code_base then p:=cur_val
  else if p=xchr_code_base then p:=cur_val+256
  else if p=xprn_code_base then p:=cur_val+512
  else p:=p+cur_val;
  scan_optional_equals;
  scan_int;
@z

@x [49.1232] l.22980 - encTeX: setting a new value to xchr/xord/xprn
  if p<math_code_base then define(p,data,cur_val)
@y
  if p<256 then xord[p]:=cur_val
  else if p<512 then xchr[p-256]:=cur_val
  else if p<768 then xprn[p-512]:=cur_val
  else if p<math_code_base then define(p,data,cur_val)
@z

@x [49.1279] - encTeX: implement \noconvert
old_setting:=selector; selector:=new_string;
token_show(def_ref); selector:=old_setting;
@y
old_setting:=selector; selector:=new_string;
message_printing := true; active_noconvert := true;
token_show(def_ref);
message_printing := false; active_noconvert := false;
selector:=old_setting;
@z

% encTeX: |slow_print| is too eager to expand printed strings.  To
% selectively suppress or enable expansion (needed to \noconvert)
% |print| will look at |message_printing|.  So we bypass |slow_print|
% and go directly to |print| instead.
@x [49.1279] - encTeX: to handle \noconvert in messages go directly to |print|
slow_print(s); update_terminal;
@y
print(s); update_terminal;
@z

@x [49.1279] - encTeX: to handle \noconvert in messages go directly to |print|
begin print_err(""); slow_print(s);
@y
begin print_err(""); print(s);
@z

% encTeX: dump encTeX-specific data to fmt file.
@x [50.1302] l.23694
@<Dump ML\TeX-specific data@>;
@y
@<Dump ML\TeX-specific data@>;
@<Dump enc\TeX-specific data@>;
@z

% encTeX: undump encTeX-specific data from fmt file.
@x [50.1303] l.23694
@<Undump ML\TeX-specific data@>;
@y
@<Undump ML\TeX-specific data@>;
@<Undump enc\TeX-specific data@>;
@z

@x [51.1337] l.24371 - enc\TeX: add. enc\TeX banner after loading fmt file
  begin wterm_ln('MLTeX v2.2 enabled');
  end;
@y
  begin wterm_ln('MLTeX v2.2 enabled');
  end;
if enctex_enabled_p then
  begin wterm(encTeX_banner); wterm_ln(', reencoding enabled.');
    if translate_filename then begin
      wterm_ln(' (\xordcode, \xchrcode, \xprncode overridden by TCX)');
    end;
  end;
@z

@x [53.1341] - encTeX: keep track of mubyte value for \write
@d write_stream(#) == info(#+1) {stream number (0 to 17)}
@y
@d write_stream(#) == type(#+1) {stream number (0 to 17)}
@d mubyte_zero == 64
@d write_mubyte(#) == subtype(#+1) {mubyte value + |mubyte_zero|}
@z

@x [53.1350] - encTeX: \write stores mubyte_out value
write_stream(tail):=cur_val;
@y
write_stream(tail):=cur_val;
if mubyte_out + mubyte_zero < 0 then write_mubyte(tail) := 0
else if mubyte_out + mubyte_zero >= 2*mubyte_zero then
       write_mubyte(tail) := 2*mubyte_zero - 1
     else write_mubyte(tail) := mubyte_out + mubyte_zero;
@z

@x [53.1353] - encTeX: \special stores specialout and mubyteout values
begin new_whatsit(special_node,write_node_size); write_stream(tail):=null;
p:=scan_toks(false,true); write_tokens(tail):=def_ref;
@y
begin new_whatsit(special_node,write_node_size);
if spec_out + mubyte_zero < 0 then write_stream(tail) := 0
else if spec_out + mubyte_zero >= 2*mubyte_zero then
       write_stream(tail) := 2*mubyte_zero - 1
     else write_stream(tail) := spec_out + mubyte_zero;
if mubyte_out + mubyte_zero < 0 then write_mubyte(tail) := 0
else if mubyte_out + mubyte_zero >= 2*mubyte_zero then
       write_mubyte(tail) := 2*mubyte_zero - 1
     else write_mubyte(tail) := mubyte_out + mubyte_zero;
if (spec_out = 2) or (spec_out = 3) then
  if (mubyte_out > 2) or (mubyte_out = -1) or (mubyte_out = -2) then
    write_noexpanding := true;
p:=scan_toks(false,true); write_tokens(tail):=def_ref;
write_noexpanding := false;
@z

@x [53.1355] - encTeX: \write prints \mubyteout value
else print_char("-");
@y
else print_char("-");
if (s = "write") and (write_mubyte (p) <> mubyte_zero) then
begin
  print_char ("<"); print_int (write_mubyte(p)-mubyte_zero); print_char (">");
end;
@z

@x [53.1356] - encTeX: \special prints \specialout and \mubyteout values
special_node:begin print_esc("special");
@y
special_node:begin print_esc("special");
if write_stream(p) <> mubyte_zero then
begin
  print_char ("<"); print_int (write_stream(p)-mubyte_zero);
  if (write_stream(p)-mubyte_zero = 2) or
     (write_stream(p)-mubyte_zero = 3) then
  begin
    print_char (":"); print_int (write_mubyte(p)-mubyte_zero);
  end;
  print_char (">");
end;
@z

@x [53.1368] - encTeX: conversions in \special
old_setting:=selector; selector:=new_string;
@y
old_setting:=selector; selector:=new_string;
spec_sout := spec_out;  spec_out := write_stream(p) - mubyte_zero;
mubyte_sout := mubyte_out;  mubyte_out := write_mubyte(p) - mubyte_zero;
active_noconvert := true;
mubyte_slog := mubyte_log;
mubyte_log := 0;
if (mubyte_out > 0) or (mubyte_out = -1) then mubyte_log := 1;
if (spec_out = 2) or (spec_out = 3) then
begin
  if (mubyte_out > 0) or (mubyte_out = -1) then
  begin
    special_printing := true; mubyte_log := 1;
  end;
  if mubyte_out > 1 then cs_converting := true;
end;
@z

@x [53.1368] - encTeX: conversions in \special
for k:=str_start[str_ptr] to pool_ptr-1 do dvi_out(so(str_pool[k]));
@y
if (spec_out = 1) or (spec_out = 3) then
  for k:=str_start[str_ptr] to pool_ptr-1 do
    str_pool[k] := si(xchr[so(str_pool[k])]);
for k:=str_start[str_ptr] to pool_ptr-1 do dvi_out(so(str_pool[k]));
spec_out := spec_sout; mubyte_out := mubyte_sout; mubyte_log := mubyte_slog;
special_printing := false; cs_converting := false;
active_noconvert := false;
@z

@x [53.1370] l.24770 - encTeX
begin @<Expand macros in the token list
@y
begin
mubyte_sout := mubyte_out;  mubyte_out := write_mubyte(p) - mubyte_zero;
if (mubyte_out > 2) or (mubyte_out = -1) or (mubyte_out = -2) then
  write_noexpanding := true;
@<Expand macros in the token list
@z

@x [53.1370] - encTeX: conversion in parameter of \write
token_show(def_ref); print_ln;
@y
active_noconvert := true;
if mubyte_out > 1 then cs_converting := true;
mubyte_slog := mubyte_log;
if (mubyte_out > 0) or (mubyte_out = -1) then mubyte_log := 1
else mubyte_log := 0;
token_show(def_ref); print_ln;
cs_converting := false; write_noexpanding := false;
active_noconvert := false;
mubyte_out := mubyte_sout; mubyte_log := mubyte_slog;
@z

@x[54.1376] l.24903 - enc\TeX
@* \[54] System-dependent changes.
@y
@* \[54/enc\TeX] System-dependent changes for enc\TeX.

@d encTeX_banner == ' encTeX v. Jun. 2004'

@ The boolean variable |enctex_p| is set by web2c according to the given
command line option (or an entry in the configuration file) before any
\TeX{} function is called.

@<Global...@> =
@!enctex_p: boolean;


@ The boolean variable |enctex_enabled_p| is used to enable enc\TeX's
primitives.  It is initialised to |false|.  When loading a \.{FMT} it
is set to the value of the boolean |enctex_p| saved in the \.{FMT} file.
Additionally it is set to the value of |enctex_p| in Ini\TeX.

@<Glob...@>=
@!enctex_enabled_p:boolean;  {enable encTeX}


@ @<Set init...@>=
enctex_enabled_p:=false;


@ Auxiliary functions/procedures for enc\TeX{} (by Petr Olsak) follow.
These functions implement the \.{\\mubyte} code to convert
the multibytes in |buffer| to one byte or to one control
sequence. These functions manipulate a mubyte tree: each node of
this tree is token list with n+1 tokens (first token consist the byte
from the byte sequence itself and the other tokens point to the
branches). If you travel from root of the tree to a leaf then you
find exactly one byte sequence which we have to convert to one byte or
control sequence. There are two variants of the leaf: the ``definitive
end'' or the ``middle leaf'' if a longer byte sequence exists and the mubyte
tree continues under this leaf. First variant is implemented as one
memory word where the link part includes the token to
which we have to convert and type part includes the number 60 (normal
conversion) or 1..52 (insert the control sequence).
The second variant of ``middle leaf'' is implemented as two memory words:
first one has a type advanced by 64 and link points to the second
word where info part includes the token to which we have to convert
and link points to the next token list with the branches of
the subtree.

The inverse: one byte to multi byte (for log printing and \.{\\write}
printing) is implemented via a pool. Each multibyte sequence is stored
in a pool as a string and |mubyte_write|[{\it printed char\/}] points
to this string.

@d new_mubyte_node ==
  link (p) := get_avail; p := link (p); info (p) := get_avail; p := info (p)
@d subinfo (#) == subtype (#)

@<Basic printing...@>=
{ read |buffer|[|i|] and convert multibyte.  |i| should have been
  of type 0..|buf_size|, but web2c doesn't like that construct in
  argument lists. }
function read_buffer(var i:integer):ASCII_code;
var p: pointer;
    last_found: integer;
    last_type: integer;
begin
  mubyte_skip := 0; mubyte_token := 0;
  read_buffer := buffer[i];
  if mubyte_in = 0 then
  begin
    if mubyte_keep > 0 then mubyte_keep := 0;
    return ;
  end;
  last_found := -2;
  if (i = start) and (not mubyte_start) then
  begin
    mubyte_keep := 0;
    if (end_line_char >= 0) and (end_line_char < 256) then
      if mubyte_read [end_line_char] <> null then
      begin
        mubyte_start := true; mubyte_skip := -1;
        p := mubyte_read [end_line_char];
        goto continue;
      end;
  end;
restart:
  mubyte_start := false;
  if (mubyte_read [buffer[i]] = null) or (mubyte_keep > 0) then
  begin
    if mubyte_keep > 0 then decr (mubyte_keep);
    return ;
  end;
  p := mubyte_read [buffer[i]];
continue:
  if type (p) >= 64 then
  begin
    last_type := type (p) - 64;
    p := link (p);
    mubyte_token := info (p); last_found := mubyte_skip;
  end else if type (p) > 0 then
  begin
    last_type := type (p);
    mubyte_token := link (p);
    goto found;
  end;
  incr (mubyte_skip);
  if i + mubyte_skip > limit then
  begin
    mubyte_skip := 0;
    if mubyte_start then goto restart;
    return;
  end;
  repeat
    p := link (p);
    if subinfo (info(p)) = buffer [i+mubyte_skip] then
    begin
      p := info (p); goto continue;
    end;
  until link (p) = null;
  mubyte_skip := 0;
  if mubyte_start then goto restart;
  if last_found = -2 then return;  { no found }
  mubyte_skip := last_found;
found:
  if mubyte_token < 256 then  { multibyte to one byte }
  begin
    read_buffer := mubyte_token;  mubyte_token := 0;
    i := i + mubyte_skip;
    if mubyte_start and (i >= start) then mubyte_start := false;
    return;
  end else begin     { multibyte to control sequence }
    read_buffer := 0;
    if last_type = 60 then { normal conversion }
      i := i + mubyte_skip
    else begin            { insert control sequence }
      decr (i); mubyte_keep := last_type;
      if i < start then mubyte_start := true;
      if last_type = 52 then mubyte_keep := 10000;
      if last_type = 51 then mubyte_keep := mubyte_skip + 1;
      mubyte_skip := -1;
    end;
    if mubyte_start and (i >= start) then mubyte_start := false;
    return;
  end;
exit: end;

@ @<Declare additional routines for enc\TeX@>=
procedure mubyte_update; { saves new string to mubyte tree }
var j: pool_pointer;
    p: pointer;
    q: pointer;
    in_mutree: integer;
begin
  j := str_start [str_ptr];
  if mubyte_read [so(str_pool[j])] = null then
  begin
    in_mutree := 0;
    p := get_avail;
    mubyte_read [so(str_pool[j])] := p;
    subinfo (p) := so(str_pool[j]); type (p) := 0;
  end else begin
    in_mutree := 1;
    p := mubyte_read [so(str_pool[j])];
  end;
  incr (j);
  while j < pool_ptr do
  begin
    if in_mutree = 0 then
    begin
      new_mubyte_node; subinfo (p) := so(str_pool[j]); type (p) := 0;
    end else { |in_mutree| = 1 }
      if (type (p) > 0) and (type (p) < 64) then
      begin
        type (p) := type (p) + 64;
        q := link (p); link (p) := get_avail; p := link (p);
        info (p) := q;
        new_mubyte_node; subinfo (p) := so(str_pool[j]); type (p) := 0;
        in_mutree := 0;
      end else begin
        if type (p) >= 64 then p := link (p);
        repeat
          p := link (p);
          if subinfo (info(p)) = so(str_pool[j]) then
          begin
            p := info (p);
            goto continue;
          end;
        until link (p) = null;
        new_mubyte_node; subinfo (p) := so(str_pool[j]); type (p) := 0;
        in_mutree := 0;
      end;
continue:
    incr (j);
  end;
  if in_mutree = 1 then
  begin
    if type (p) = 0 then
    begin
       type (p) := mubyte_prefix + 64;
       q := link (p);  link (p) := get_avail; p := link (p);
       link (p) := q; info (p) := mubyte_stoken;
       return;
    end;
    if type (p) >= 64 then
    begin
      type (p) := mubyte_prefix + 64;
      p := link (p); info (p) := mubyte_stoken;
      return;
    end;
  end;
  type (p) := mubyte_prefix;
  link (p) := mubyte_stoken;
exit: end;
@#
procedure dispose_munode (p: pointer); { frees a mu subtree recursivelly }
var q: pointer;
begin
  if (type (p) > 0) and (type (p) < 64) then free_avail (p)
  else begin
    if type (p) >= 64 then
    begin
      q := link (p); free_avail (p); p := q;
    end;
    q := link (p); free_avail (p); p := q;
    while p <> null do
    begin
      dispose_munode (info (p));
      q := link (p);
      free_avail (p);
      p := q;
    end;
  end;
end;
@#
procedure dispose_mutableout (cs: pointer); { frees record from out table }
var p, q, r: pointer;
begin
  p := mubyte_cswrite [cs mod 128];
  r := null;
  while p <> null do
    if info (p) = cs then
    begin
      if r <> null then link (r) := link (link (p))
      else mubyte_cswrite[cs mod 128] := link (link (p));
      q := link (link(p));
      free_avail (link(p)); free_avail (p);
      p := q;
    end else begin
      r := link (p); p := link (r);
    end;
end;

@ The |print_buffer| procedure prints one character from |buffer|[|i|].
It also increases |i| to the next character in the buffer.

@<Basic printing...@>=
{ print one char from |buffer|[|i|]. |i| should have been of type
  0..|buf_size|, but web2c doesn't like that construct in argument lists. }
procedure print_buffer(var i:integer);
var c: ASCII_code;
begin
  if mubyte_in = 0 then print (buffer[i]) { normal TeX }
  else if mubyte_log > 0 then print_char (buffer[i])
       else begin
         c := read_buffer (i);
         if mubyte_token > 0 then print_cs (mubyte_token-cs_token_flag)
         else print (c);
       end;
  incr (i);
end;

@ Additional material to dump for enc\TeX.  This includes whether
enc\TeX is enabled, and if it is we also have to dump the \.{\\mubyte}
arrays.

@<Dump enc\TeX-specific data@>=
dump_int(@"45435458);  {enc\TeX's magic constant: "ECTX"}
if not enctex_p then dump_int(0)
else begin
  dump_int(1);
  dump_things(mubyte_read[0], 256);
  dump_things(mubyte_write[0], 256);
  dump_things(mubyte_cswrite[0], 128);
end;

@ Undumping the additional material we dumped for enc\TeX.  This includes
conditionally undumping the \.{\\mubyte} arrays.

@<Undump enc\TeX-specific data@>=
undump_int(x);   {check magic constant of enc\TeX}
if x<>@"45435458 then goto bad_fmt;
undump_int(x);   {undump |enctex_p| flag into |enctex_enabled_p|}
if x=0 then enctex_enabled_p:=false
else if x<>1 then goto bad_fmt
else begin
  enctex_enabled_p:=true;
  undump_things(mubyte_read[0], 256);
  undump_things(mubyte_write[0], 256);
  undump_things(mubyte_cswrite[0], 128);
end;


@* \[54] System-dependent changes.
@z
