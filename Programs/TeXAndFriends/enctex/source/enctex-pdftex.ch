% $Id$
% enctex1.ch is applied first.
% Then this change is applied for pdfTeX et al. (but not TeX),
% assuming that tracingstacklevels.ch, partoken.ch, and showstream.ch
% have been included, as specified in the *.am files.

% encTeX: \mubytein \mubyteout \mubytelog and \specialout
@x [17.236] l.4954
@d web2c_int_pars=web2c_int_base+6 {total number of web2c's integer parameters}
@y
@d mubyte_in_code=web2c_int_base+6{if positive then reading mubytes is active}
@d mubyte_out_code=web2c_int_base+7{if positive then printing mubytes is active}
@d mubyte_log_code=web2c_int_base+8{if positive then print mubytes to log and terminal}
@d spec_out_code=web2c_int_base+9 {if positive then print specials by mubytes}
@d web2c_int_pars=web2c_int_base+10 {total number of web2c's integer parameters}
@z

% see pdftex.web for more about this.
@x [53.1353] - encTeX: late \special stores specialout and mubyteout values
begin new_whatsit(latespecial_node,write_node_size); write_stream(tail):=null;
p:=scan_toks(false,false); write_tokens(tail):=def_ref;
@y
begin new_whatsit(latespecial_node,write_node_size);
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
p:=scan_toks(false,false); write_tokens(tail):=def_ref;
write_noexpanding := false;
@z
