%% tex-miktex-src.ch:
%% 
%% Copyright (C) 1991-2020 Christian Schenk
%% 
%% This file is free software; you can redistribute it and/or modify it
%% under the terms of the GNU General Public License as published by the
%% Free Software Foundation; either version 2, or (at your option) any
%% later version.
%% 
%% This file is distributed in the hope that it will be useful, but
%% WITHOUT ANY WARRANTY; without even the implied warranty of
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
%% General Public License for more details.
%% 
%% You should have received a copy of the GNU General Public License
%% along with This file; if not, write to the Free Software Foundation,
%% 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

%% ////////////////////////////////////////////////////////////////////////////
%% //                                                                        //
%% //                      INSPIRED BY WEB2C'S TEX.CH                        //
%% //                                                                        //
%% ////////////////////////////////////////////////////////////////////////////

% _____________________________________________________________________________
%
% [17.222]
% _____________________________________________________________________________

@x
@d frozen_null_font=frozen_control_sequence+10
  {permanent `\.{\\nullfont}'}
@y
@d frozen_special=frozen_control_sequence+10
  {permanent `\.{\\special}'}
@d frozen_null_font=frozen_control_sequence+11
  {permanent `\.{\\nullfont}'}
@z

% _____________________________________________________________________________
%
% [37.774]
% _____________________________________________________________________________

 @x
if every_cr<>null then begin_token_list(every_cr,every_cr_text);
 @y
if (miktex_insert_src_special_every_cr and head<>tail) then insert_src_special;
if every_cr<>null then begin_token_list(every_cr,every_cr_text);
 @z

% _____________________________________________________________________________
%
% [37.799]
% _____________________________________________________________________________

 @x
if every_cr<>null then begin_token_list(every_cr,every_cr_text);
 @y
if (miktex_insert_src_special_every_cr) then insert_src_special;
if every_cr<>null then begin_token_list(every_cr,every_cr_text);
 @z

% _____________________________________________________________________________
%
% [46.1034]
% _____________________________________________________________________________

@x
@<Append character |cur_chr|...@>=
@y
@<Append character |cur_chr|...@>=
if ((head=tail) and (mode>0)) then begin
  if (miktex_insert_src_special_auto) then append_src_special;
end;
@z

% _____________________________________________________________________________
%
% [47.1091]
% _____________________________________________________________________________

@x
if indented then
  begin tail:=new_null_box; link(head):=tail; width(tail):=par_indent;@+
  end;
@y
if indented then
  begin tail:=new_null_box; link(head):=tail; width(tail):=par_indent;
  if (miktex_insert_src_special_every_par) then insert_src_special;@+
  end;
@z

% _____________________________________________________________________________
%
% [48.1139]
% _____________________________________________________________________________

@x
if every_math<>null then begin_token_list(every_math,every_math_text);
@y
if (miktex_insert_src_special_every_math) then insert_src_special;
if every_math<>null then begin_token_list(every_math,every_math_text);
@z

% _____________________________________________________________________________
%
% [48.1167]
% _____________________________________________________________________________

@x
  if every_vbox<>null then begin_token_list(every_vbox,every_vbox_text);
@y
  if (miktex_insert_src_special_every_vbox) then insert_src_special;
  if every_vbox<>null then begin_token_list(every_vbox,every_vbox_text);
@z

% _____________________________________________________________________________
%
% [53.1344]
% _____________________________________________________________________________

@x
primitive("special",extension,special_node);@/
@y
primitive("special",extension,special_node);@/
text(frozen_special):="special"; eqtb[frozen_special]:=eqtb[cur_val];@/
@z

% _____________________________________________________________________________
%
% [54.1379]
% _____________________________________________________________________________

@x
@* \[54/\MiKTeX] System-dependent changes for \MiKTeX.
@y
@* \[54/\MiKTeX] System-dependent changes for \MiKTeX.

@ This routine inserts a source special.

@<Declare action procedures for use by |main_control|@>=

procedure insert_src_special;
var toklist, p, q : pointer;
begin
  if (source_filename_stack[in_open] > 0
      and miktex_is_new_source(source_filename_stack[in_open], line)) then
  begin
    toklist := get_avail;
    p := toklist;
    info(p) := cs_token_flag+frozen_special;
    link(p) := get_avail; p := link(p);
    info(p) := left_brace_token+"{";
    q :=
      str_toks(miktex_make_src_special(source_filename_stack[in_open], line));
    link(p) := link(temp_head);
    p := q;
    link(p) := get_avail; p := link(p);
    info(p) := right_brace_token+"}";
    ins_list (toklist);
    miktex_remember_source_info (source_filename_stack[in_open], line);
  end;
end;

@ This routine appends a source special.

@<Declare action procedures for use by |main_control|@>=

procedure append_src_special;
var p : pointer;
begin
  if (source_filename_stack[in_open] > 0
      and miktex_is_new_source(source_filename_stack[in_open], line)) then
  begin
    new_whatsit (special_node, write_node_size);
    write_stream(tail) := 0;
    def_ref := get_avail;
    token_ref_count(def_ref) := null;
    str_toks (miktex_make_src_special(source_filename_stack[in_open], line));
    link(def_ref) := link(temp_head);
    write_tokens(tail) := def_ref;
    miktex_remember_source_info (source_filename_stack[in_open], line);
  end;
end;

@ Forward declaration of \MiKTeX\ functions.

@<Declare \MiKTeX\ functions@>=

function miktex_insert_src_special_auto : boolean; forward;@t\2@>@/
function miktex_insert_src_special_every_par : boolean; forward;@t\2@>@/
function miktex_insert_src_special_every_parend : boolean; forward;@t\2@>@/
function miktex_insert_src_special_every_cr : boolean; forward;@t\2@>@/
function miktex_insert_src_special_every_math : boolean; forward;@t\2@>@/
function miktex_insert_src_special_every_hbox : boolean; forward;@t\2@>@/
function miktex_insert_src_special_every_vbox : boolean; forward;@t\2@>@/
function miktex_insert_src_special_every_display : boolean; forward;@t\2@>@/
@z
