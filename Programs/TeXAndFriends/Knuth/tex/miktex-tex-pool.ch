%%% miktex-tex-pool.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

% _____________________________________________________________________________
%
% [4.47]
% _____________________________________________________________________________

@x
@!m,@!n:text_char; {characters input from |pool_file|}
@!g:str_number; {garbage}
@!a:integer; {accumulator for check sum}
@!c:boolean; {check sum has been checked}
@y
@!g:str_number; {garbage}
@z

% _____________________________________________________________________________
%
% [4.51]
% _____________________________________________________________________________

@x
@ @d bad_pool(#)==begin wake_up_terminal; write_ln(term_out,#);
  a_close(pool_file); get_strings_started:=false; return;
  end
@<Read the other strings...@>=
name_of_file:=pool_name; {we needn't set |name_length|}
if a_open_in(pool_file) then
  begin c:=false;
  repeat @<Read one string, but return |false| if the
    string memory space is getting too tight for comfort@>;
  until c;
  a_close(pool_file); get_strings_started:=true;
  end
else  bad_pool('! I can''t read TEX.POOL.')
@.I can't read TEX.POOL@>
@y
@ Empty module
@z

% _____________________________________________________________________________
%
% [4.52]
% _____________________________________________________________________________

@x
@ @<Read one string...@>=
begin if eof(pool_file) then bad_pool('! TEX.POOL has no check sum.');
@.TEX.POOL has no check sum@>
read(pool_file,m,n); {read two digits of string length}
if m='*' then @<Check the pool check sum@>
else  begin if (xord[m]<"0")or(xord[m]>"9")or@|
      (xord[n]<"0")or(xord[n]>"9") then
    bad_pool('! TEX.POOL line doesn''t begin with two digits.');
@.TEX.POOL line doesn't...@>
  l:=xord[m]*10+xord[n]-"0"*11; {compute the length}
  if pool_ptr+l+string_vacancies>pool_size then
    bad_pool('! You have to increase POOLSIZE.');
@.You have to increase POOLSIZE@>
  for k:=1 to l do
    begin if eoln(pool_file) then m:=' '@+else read(pool_file,m);
    append_char(xord[m]);
    end;
  read_ln(pool_file); g:=make_string;
  end;
end
@y
@ Empty module
@z

% _____________________________________________________________________________
%
% [4.53]
% _____________________________________________________________________________

@x
@ The \.{WEB} operation \.{@@\$} denotes the value that should be at the
end of this \.{TEX.POOL} file; any other value means that the wrong pool
file has been loaded.
@^check sum@>

@<Check the pool check sum@>=
begin a:=0; k:=1;
loop@+  begin if (xord[n]<"0")or(xord[n]>"9") then
  bad_pool('! TEX.POOL check sum doesn''t have nine digits.');
@.TEX.POOL check sum...@>
  a:=10*a+xord[n]-"0";
  if k=9 then goto done;
  incr(k); read(pool_file,n);
  end;
done: if a<>@$ then bad_pool('! TEX.POOL doesn''t match; TANGLE me again.');
@.TEX.POOL doesn't match@>
c:=true;
end
@y
@ @<Read the other strings...@>=
  g := loadpoolstrings((pool_size-string_vacancies));
  if g=0 then begin
     wake_up_terminal; write_ln(term_out,'! You have to increase POOLSIZE.');
     get_strings_started:=false;
     return;
  end;
  get_strings_started:=true;
@z
