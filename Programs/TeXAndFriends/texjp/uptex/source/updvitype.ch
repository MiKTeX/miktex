@x
@d my_name=='pdvitype'
@d banner=='This is pDVItype, Version 3.6-p0.5'
@y
@d my_name=='updvitype'
@d banner=='This is upDVItype, Version 3.6-p0.5-u1.29'
@z

@x procedure initialize
  print (banner);
  print_ln (version_string);
@y
  print (banner);
  print (' (');
  print (conststringcast(get_enc_string));
  print (')');
  print_ln (version_string);
@z

@x procedure out_kanji
  if text_ptr>=line_length-3 then flush_text;
  c:=toBUFF(fromDVI(c));
  incr(text_ptr); text_buf[text_ptr]:= Hi(c);
  incr(text_ptr); text_buf[text_ptr]:= Lo(c);
@y
  if text_ptr>=line_length-5 then flush_text;
  c:=toBUFF(fromDVI(c));
  if BYTE1(c)<>0 then begin incr(text_ptr); text_buf[text_ptr]:=BYTE1(c); end;
  if BYTE2(c)<>0 then begin incr(text_ptr); text_buf[text_ptr]:=BYTE2(c); end;
  if BYTE3(c)<>0 then begin incr(text_ptr); text_buf[text_ptr]:=BYTE3(c); end;
                            incr(text_ptr); text_buf[text_ptr]:=BYTE4(c);
@z

@x
      usage_help (PDVITYPE_HELP, 'issue@@texjp.org');
@y
      usage_help (UPDVITYPE_HELP, 'issue@@texjp.org');
@z

