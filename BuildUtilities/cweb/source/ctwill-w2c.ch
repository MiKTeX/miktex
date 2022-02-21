@q Changes for CWEB in TeX Live from numerous contributors.              @>
@q This file is in the Public Domain.                                    @>

@q Most of the original Kpathsea changes by Wlodek Bzyl and Olaf Weber   @>
@q were merged with the set of change files of the CWEBbin project;      @>
@q see https://github.com/ascherer/cwebbin for the original parts.       @>

@q This stripped change file {comm,ctang,cweav,ctwill,cwebman}-w2c.ch    @>
@q has been created from the elaborate set of change files               @>
@q {comm,ctang,cweav,cwebman}-patch.ch,                                  @>
@q {comm,cweav,cwebman}-extensions.ch, {comm,ctang,cweav}-output.ch,     @>
@q {comm,ctang,cweav}-i18n.ch, and cweav-twill.ch for CTWILL, and        @>
@q {comm,ctang,cweav,ctwill,cwebman}-texlive.ch with the 'tie' processor @>
@q and is used as a monolithic changefile for {common,ctangle,cweave}.w  @>
@q and cwebman.tex in TeX Live.                                          @>

@q Please send comments, suggestions, etc. to tex-k@@tug.org.            @>

@x
% This file is part of CWEB.
% This program by Silvio Levy and Donald E. Knuth
% is based on a program by Knuth.
% It is distributed WITHOUT ANY WARRANTY, express or implied.
@y
% This program by Don Knuth is based on CWEAVE by Levy and Knuth.
% It's somewhat flaky, so you probably shouldn't try to use it.
@z

@x
% Here is TeX material that gets inserted after \input cwebmac
@y
% Here is TeX material that gets inserted after \input ctwimac
\def\contentspagenumber{0} % default page number for table of contents
\datethis % Put timestamp before section 1
@z

@x
\def\title{CWEAVE (Version 4.7)}
@y
\def\title{CTWILL (Version 4.7 [\TeX~Live])}
@z

@x
  \centerline{\titlefont The {\ttitlefont CWEAVE} processor}
@y
  \centerline{\titlefont The {\ttitlefont CTWILL} processor}
@z

@x
  \centerline{(Version 4.7)}
@y
  \centerline{(Version 4.7 [\TeX~Live])}
@z

@x
\def\botofcontents{\vfill
@y
\def\covernote{\vbox{%
@z

@x
}
@y
}}\datecontentspage
@z

@x
This is the \.{CWEAVE} program by Silvio Levy and Donald E. Knuth,
based on \.{WEAVE} by Knuth.
We are thankful to Steve Avery,
Nelson Beebe, Hans-Hermann Bode (to whom the original \CPLUSPLUS/ adaptation
is due), Klaus Guntermann, Norman Ramsey, Tomas Rokicki, Joachim Schnitter,
Joachim Schrod, Lee Wittenberg, Saroj Mahapatra, Cesar Augusto Rorato
Crusius, and others who have contributed improvements.

The ``banner line'' defined here should be changed whenever \.{CWEAVE}
is modified.

@d banner "This is CWEAVE (Version 4.7)"
@y
This is the \.{CTWILL} program by D. E. Knuth, based
on \.{CWEAVE} by Silvio Levy and D.~E. Knuth. It is also based on
\.{TWILL}, a private \.{WEB} program that Knuth wrote to produce
Volumes B and~D of {\sl Computers {\char`\&} Typesetting\/} in 1985.
\.{CTWILL} was hacked together hastily in June, 1992, to generate pages for
Knuth's book about the Stanford GraphBase, and updated even more hastily
in March, 1993, to generate final copy for that book.  The main idea was to
extend \.{CWEAVE} so that ``mini-indexes'' could appear.
No time was available to make \.{CTWILL} into a refined or complete system,
nor even to fully update the program documentation below. Subsequent changes
were made only to maintain compatibility with \.{CWEAVE}. Further information
can be found in Knuth's article ``Mini-indexes for literate programs,''
reprinted in {\sl Digital Typography\/} (1999), 225--245.

A kind of ``user manual'' for \.{CTWILL} can be found in the appendix
\X271:Mogrify \.{CWEAVE} into \.{CTWILL}\X~and beyond, together with
additional material specific to \.{CTWILL}. % FIXME
Until then, \.{CWEAVE}'s sequence of sections will be preserved.

The ``banner line'' defined here should be changed whenever \.{CTWILL} is
modified. The version number parallels the corresponding version of \.{CWEAVE}.

@d banner "This is CTWILL, Version 4.7"
  /* will be extended by the \TeX~Live |versionstring| */
@z

@x
  program=cweave;
@y
  program=ctwill;
@z

@x
  if (show_banner) puts(banner); /* print a ``banner line'' */
@y
  if (show_banner) cb_show_banner(); /* print a ``banner line'' */
@z

@x
@i common.h
@y
@i comm-w2c.h
@z

@x
@d max_refs 30000 /* number of cross-references; must be less than 65536 */
@d max_scraps 5000 /* number of tokens in \CEE/ texts being parsed */
@y
@d max_refs 65535 /* number of cross-references; must be less than 65536 */
@d max_scraps 5000 /* number of tokens in \CEE/ texts being parsed */
@z

@x
turned on during the first phase.

@<Private...@>=
static boolean change_exists; /* has any section changed? */
@y
turned on during the first phase---NOT!
@z

@x
@d append_xref(c) if (xref_ptr==xmem_end) overflow("cross-reference");
@y
@d append_xref(c) if (xref_ptr==xmem_end) overflow(_("cross-reference"));
@z

@x
@d max_toks 30000 /* number of symbols in \CEE/ texts being parsed;
  must be less than 65536 */
@d max_texts 8000 /* number of phrases in \CEE/ texts being parsed;
  must be less than 10240 */
@y
@d max_toks 65535 /* number of symbols in \CEE/ texts being parsed;
  must be less than 65536 */
@d max_texts 10239 /* number of phrases in \CEE/ texts being parsed;
  must be less than 10240 */
@z

@x
@ Here are the three procedures needed to complete |id_lookup|:
@y
@ Here are the three procedures needed to complete |id_lookup|:
@s perm_meaning int
@z

@x
  p->ilk=t; init_node(p);
@y
  struct perm_meaning *q=get_meaning(p);
  p->ilk=t; init_node(p);
  q->stamp=0;
  q->link=NULL;
  q->perm.id=p;
  q->perm.prog_no=q->perm.sec_no=0;
  strcpy(q->perm.tex_part,"\\uninitialized");
@z

@x
@ And here's a small helper function to simplify the code.

@d update_node(p) (p)->xref=(void *)xref_ptr
@y
@ And here are two small helper functions to simplify the code.

@d update_node(p) (p)->xref=(void *)xref_ptr
@d get_meaning(p) (p)-name_dir+cur_meaning
@z

@x
id_lookup("extern",NULL,int_like);
@y
@#
ext_loc=id_lookup("extern",NULL,int_like)-name_dir;
@#
@z

@x
id_lookup("int",NULL,raw_int);
@y
@#
int_loc=id_lookup("int",NULL,raw_int)-name_dir;
@#
@z

@x
@d ord 0207 /* control code for `\.{@@'}' */
@d join 0210 /* control code for `\.{@@\&}' */
@d thin_space 0211 /* control code for `\.{@@,}' */
@d math_break 0212 /* control code for `\.{@@\v}' */
@d line_break 0213 /* control code for `\.{@@/}' */
@d big_line_break 0214 /* control code for `\.{@@\#}' */
@d no_line_break 0215 /* control code for `\.{@@+}' */
@d pseudo_semi 0216 /* control code for `\.{@@;}' */
@d macro_arg_open 0220 /* control code for `\.{@@[}' */
@d macro_arg_close 0221 /* control code for `\.{@@]}' */
@d trace 0222 /* control code for `\.{@@0}', `\.{@@1}' and `\.{@@2}' */
@d translit_code 0223 /* control code for `\.{@@l}' */
@d output_defs_code 0224 /* control code for `\.{@@h}' */
@d format_code 0225 /* control code for `\.{@@f}' and `\.{@@s}' */
@d definition 0226 /* control code for `\.{@@d}' */
@d begin_C 0227 /* control code for `\.{@@c}' */
@d section_name 0230 /* control code for `\.{@@<}' */
@d new_section 0231 /* control code for `\.{@@\ }' and `\.{@@*}' */
@y
@d meaning 0207 /* control code for `\.{@@\$}' */
@d suppress 0210 /* control code for `\.{@@-}' */
@d temp_meaning 0211 /* control code for `\.{@@\%}' */
@d right_start 0212 /* control code for `\.{@@r}' */
@d ord 0213 /* control code for `\.{@@'}' */
@d join 0214 /* control code for `\.{@@\&}' */
@d thin_space 0215 /* control code for `\.{@@,}' */
@d math_break 0216 /* control code for `\.{@@\v}' */
@d line_break 0217 /* control code for `\.{@@/}' */
@d big_line_break 0220 /* control code for `\.{@@\#}' */
@d no_line_break 0221 /* control code for `\.{@@+}' */
@d pseudo_semi 0222 /* control code for `\.{@@;}' */
@d macro_arg_open 0224 /* control code for `\.{@@[}' */
@d macro_arg_close 0225 /* control code for `\.{@@]}' */
@d trace 0226 /* control code for `\.{@@0}', `\.{@@1}' and `\.{@@2}' */
@d translit_code 0227 /* control code for `\.{@@l}' */
@d output_defs_code 0230 /* control code for `\.{@@h}' */
@d format_code 0231 /* control code for `\.{@@f}' and `\.{@@s}' */
@d definition 0232 /* control code for `\.{@@d}' */
@d begin_C 0233 /* control code for `\.{@@c}' */
@d section_name 0234 /* control code for `\.{@@<}' */
@d new_section 0235 /* control code for `\.{@@\ }' and `\.{@@*}' */
@z

@x
ccode['\'']=ord;
@y
ccode['\'']=ord;
ccode['$']=meaning; ccode['%']=temp_meaning; ccode['-']=suppress;
ccode['r']=ccode['R']=right_start;
@z

@x
static eight_bits skip_TeX(void);

@ @c
@y
static eight_bits skip_TeX(void);

@ We look for a clue about the program's title, because this will become
part of all meanings.

@c
@z

@x
    if (loc>limit && get_line()==false) return;
@y
    if (loc>limit && get_line()==false) return;
    if (loc==buffer && strncmp(buffer,"\\def\\title{",11)==0) {
      loc=buffer+10;
      title_lookup(); /* this program's title will be code zero */
    }
@z

@x
skip_TeX(void) /* skip past pure \TEX/ code */
@y
skip_TeX(void)
@z

@x
\yskip\hang |xref_roman|, |xref_wildcard|, |xref_typewriter|, |TeX_string|,
@y
\yskip\hang |xref_roman|, |xref_wildcard|, |xref_typewriter|, |TeX_string|,
|meaning|, |suppress|,
@z

@x
@d right_preproc 0217 /* ends a preprocessor command */
@y
@d right_preproc 0223 /* ends a preprocessor command */
@z

@x
        err_print("! String didn't end"); loc=limit; break;
@y
        err_print(_("! String didn't end")); loc=limit; break;
@z

@x
        err_print("! Input ended in middle of string"); loc=buffer; break;
@y
        err_print(_("! Input ended in middle of string")); loc=buffer; break;
@z

@x
    fputs("\n! String too long: ",stdout);
@y
    fputs(_("\n! String too long: "),stdout);
@z

@x
  case translit_code: err_print("! Use @@l in limbo only"); continue;
@y
  case translit_code: err_print(_("! Use @@l in limbo only")); continue;
@z

@x
  case underline: xref_switch=def_flag; continue;
@y
  case underline: xref_switch=def_flag; continue;
  case temp_meaning: temp_switch=!temp_switch; continue;
  case right_start: right_start_switch=true; continue;
@z

@x
  case xref_roman: case xref_wildcard: case xref_typewriter: case noop:
@y
  case xref_roman: case xref_wildcard: case xref_typewriter: case noop:
  case meaning: case suppress:
@z

@x
    err_print("! Input ended in section name");
@y
    err_print(_("! Input ended in section name"));
@z

@x
  fputs("\n! Section name too long: ",stdout);
@y
  fputs(_("\n! Section name too long: "),stdout);
@z

@x
    err_print("! Section name didn't end"); break;
@y
    err_print(_("! Section name didn't end")); break;
@z

@x
    err_print("! Control codes are forbidden in section name"); break;
@y
    err_print(_("! Control codes are forbidden in section name")); break;
@z

@x
skip_restricted(void)
{
@y
skip_restricted(void)
{ int c=ccode[(eight_bits)*(loc-1)];
@z

@x
    err_print("! Control text didn't end"); loc=limit;
@y
    err_print(_("! Control text didn't end")); loc=limit;
@z

@x
      err_print("! Control codes are forbidden in control text");
@y
      err_print(_("! Control codes are forbidden in control text"));
@z

@x
@.Control codes are forbidden...@>
@y
@.Control codes are forbidden...@>
    if (c==meaning && phase==2) @<Process a user-generated meaning@>@;
    else if (c==suppress && phase==2) @<Suppress mini-index entry@>@;
@z

@x
if (loc>=limit) err_print("! Verbatim string didn't end");
@y
if (loc>=limit) err_print(_("! Verbatim string didn't end"));
@z

@x
  skip_limbo(); change_exists=false;
@y
  skip_limbo();
  @<Give a default title to the program, if necessary@>@;
@z

@x
  changed_section[section_count]=change_exists;
    /* the index changes if anything does */
@y
@z

@x
  if (++section_count==max_sections) overflow("section number");
@y
  if (++section_count==max_sections) overflow(_("section number"));
@z

@x
  changed_section[section_count]=changing;
     /* it will become |true| if any line changes */
@y
@z

@x
  if (changed_section[section_count]) change_exists=true;
@y
@z

@x
    case translit_code: err_print("! Use @@l in limbo only"); continue;
@y
    case translit_code: err_print(_("! Use @@l in limbo only")); continue;
@z

@x
    case xref_roman: case xref_wildcard: case xref_typewriter:
@y
    case xref_roman: case xref_wildcard: case xref_typewriter:
    case meaning: case suppress:
@z

@x
  err_print("! Missing left identifier of @@s");
@y
  err_print(_("! Missing left identifier of @@s"));
@z

@x
    err_print("! Missing right identifier of @@s");
@y
    err_print(_("! Missing right identifier of @@s"));
@z

@x
      fputs("\n! Never defined: <",stdout);
      print_section_name(p); putchar('>'); mark_harmless;
@y
      fputs(_("\n! Never defined: <"),stdout);
      print_section_name(p); putchar('>'); mark_harmless;
@z

@x
      fputs("\n! Never used: <",stdout);
      print_section_name(p); putchar('>'); mark_harmless;
@y
      fputs(_("\n! Never used: <"),stdout);
      print_section_name(p); putchar('>'); mark_harmless;
@z

@x
@ In particular, the |finish_line| procedure is called near the very
beginning of phase two. We initialize the output variables in a slightly
tricky way so that the first line of the output file will be
`\.{\\input cwebmac}'.

@<Start \TEX/...@>=
out_ptr=out_buf+1; out_line=1; active_file=tex_file;
tex_printf("\\input cwebma"); *out_ptr='c';
@y
@ In particular, the |finish_line| procedure is called near the very
beginning of phase two. We initialize the output variables in a slightly
tricky way so that the first line of the output file will be dependent of
the user language set by the `\.{+l}' option and its argument.  If you call
\.{CTWILL} with `\.{+lX}' (or `\.{-lX}' as well), where `\.X' is the
(possibly empty) string of characters to the right of~`\.l', `\.X'~will
be prepended to `\.{ctwimac.tex}', e.g., if you call \.{CTWILL} with
`\.{+ldeutsch}', you will receive the line `\.{\\input deutschctwimac}'.
Without this option the first line of the output file will be
`\.{\\input ctwimac}'. Or, if the user has specified proofing by
saying \.{+P} on the command line, it's `\.{\\input ctproofmac}' (resp.\
\.{\\input Xctproofmac} with option \.{+lX}), a set of macros used when
debugging mini-index entries.

@d proofing flags['P']

@<Start \TEX/...@>=
out_ptr=out_buf+1; out_line=1; active_file=tex_file;
tex_puts("\\input ");
tex_printf(use_language);
tex_puts(proofing?"ctproofma":"ctwima");
*out_ptr='c';
@z

@x
@d out(c) {if (out_ptr>=out_buf_end) break_out(); *(++out_ptr)=c;}
@y
@d out(c)
 {if (ms_mode) { /* outputting to |ministring_buf| */
    if (ministring_ptr<ministring_buf_end)
      *ministring_ptr++=c;
  } else {
     if (out_ptr>=out_buf_end) break_out();
     *(++out_ptr)=c;
   }
 }
@z

@x
  printf("\n! Line had to be broken (output l. %d):\n",out_line);
@y
  printf(_("\n! Line had to be broken (output l. %d):\n"),out_line);
@z

@x
|def_flag|, so it cannot have more than five decimal digits.  If
the section is changed, we output `\.{\\*}' just after the number.
@y
|def_flag|, so it cannot have more than five decimal digits.
@z

@x
  if (changed_section[n]) out_str("\\*");
@.\\*@>
@y
@z

@x
        default: err_print("! Double @@ should be used in limbo");
@y
        case right_start: right_start_switch=true; break;
        default: err_print(_("! Double @@ should be used in limbo"));
@z

@x
@d app_tok(c) {if (tok_ptr+2>tok_mem_end) overflow("token"); *(tok_ptr++)=c;}
@y
@d app_tok(c) {if (tok_ptr+2>tok_mem_end) overflow(_("token")); *(tok_ptr++)=c;}
@z

@x
          err_print("! Input ended in mid-comment");
@y
          err_print(_("! Input ended in mid-comment"));
@z

@x
        if (bal>1) err_print("! Missing } in comment");
@y
        if (bal>1) err_print(_("! Missing } in comment"));
@z

@x
      else {err_print("! Extra } in comment");
@y
      else {err_print(_("! Extra } in comment"));
@z

@x
  if (bal>1) err_print("! Missing } in comment");
@y
  if (bal>1) err_print(_("! Missing } in comment"));
@z

@x
    err_print("! Illegal use of @@ in comment");
@y
    err_print(_("! Illegal use of @@ in comment"));
@z

@x
@d attr_head 69 /* denotes beginning of attribute */
@y
@d attr_head 69 /* denotes beginning of attribute */
@d title 70 /* program name or header name in a ``meaning'' */
@z

@x
@i prod.w
@y
@i prod-twill.w
@z

@x
static scrap scrap_info[max_scraps]; /* memory array for scraps */
@y
static scrap scrap_info[max_scraps]; /* memory array for scraps */
static scrap null_scrap; /* a scrap with empty translation */
@z

@x
@ @<Set init...@>=
@y
@ @<Set init...@>=
null_scrap.trans=&tok_start[0];
@z

@x
@d inner_tok_flag (5*id_flag) /* signifies a token list in `\pb' */

@c
static void
print_text( /* prints a token list for debugging; not used in |main| */
@y
@d inner_tok_flag (5*id_flag) /* signifies a token list in `\pb' */

@<Predecl...@>=
#if 0
static void print_text(text_pointer p);
#endif

@ @c
#if 0
@t\4\4@>static void
print_text( /* prints a token list for debugging; not used in |main| */
@z

@x
  update_terminal;
@y
  puts("|"); update_terminal;
@z

@x
}
@y
}
#endif
@z

@x
@ @<Predecl...@>=@+static void print_text(text_pointer p);
@y
@z

@x
the |for| loop below.

@c
@y
the |for| loop below.

We use the fact that |make_underlined| has been called immediately preceding
|make_reserved|, hence |tok_loc| has been set.

@c
static token_pointer tok_loc; /* where the first identifier appears */
@#
@z

@x
  token_pointer tok_loc; /* pointer to |tok_value| */
  if ((tok_loc=find_first_ident(p->trans))<=operator_found)
    return; /* this should not happen */
@y
  if (tok_loc<=operator_found) return; /* this should not happen */
@z

@x
  token_pointer tok_loc; /* where the first identifier appears */
@y
@z

@x
@<Cases for |exp|@>=
if (cat1==lbrace || cat1==int_like || cat1==decl) {
  make_underlined(pp); big_app1(pp); big_app(dindent);
  reduce(pp,1,fn_decl,0,1);
}
@y
@<Cases for |exp|@>=
if(cat1==lbrace || cat1==int_like || cat1==decl) {
  make_underlined(pp); make_ministring(pp); big_app1(pp);
  if (indent_param_decl) big_app(dindent);
  reduce(pp,1,fn_decl,0,1);
}
@z

@x
  make_underlined (pp); squash(pp,2,tag,-1,7);
@y
  make_underlined (pp);
  if (tok_loc>operator_found) {
    strcpy(ministring_buf,"label");
    new_meaning(((*tok_loc)%id_flag)+name_dir);
  }
  squash(pp,2,tag,-1,7);
@z

@x
@ @<Cases for |decl_head|@>=
if (cat1==comma) {
  big_app2(pp); big_app(' '); reduce(pp,2,decl_head,-1,33);
}
else if (cat1==ubinop) {
  big_app1_insert(pp,'{'); big_app('}'); reduce(pp,2,decl_head,-1,34);
}
else if (cat1==exp && cat2!=lpar && cat2!=lbrack && cat2!=exp && cat2!=cast) {
  make_underlined(pp+1); squash(pp,2,decl_head,-1,35);
}
else if ((cat1==binop||cat1==colon) && cat2==exp && (cat3==comma ||
    cat3==semi || cat3==rpar))
  squash(pp,3,decl_head,-1,36);
else if (cat1==cast) squash(pp,2,decl_head,-1,37);
else if (cat1==lbrace || cat1==int_like || cat1==decl) {
  big_app(dindent); squash(pp,1,fn_decl,0,38);
}
else if (cat1==semi) squash(pp,2,decl,-1,39);
@y
@ @<Cases for |decl_head|@>=
if (cat1==comma) {
  big_app2(pp); big_app(' '); reduce(pp,2,decl_head,-1,33);
}
else if (cat1==ubinop) {
  big_app1_insert(pp,'{'); big_app('}');
  reduce(pp,2,decl_head,-1,34);
}
else if (cat1==exp && cat2!=lpar && cat2!=lbrack && cat2!=exp && cat2!=cast) {
  make_underlined(pp+1); make_ministring(pp+1);
  squash(pp,2,decl_head,-1,35);
}
else if ((cat1==binop||cat1==colon) && cat2==exp && (cat3==comma ||
    cat3==semi || cat3==rpar))
  squash(pp,3,decl_head,-1,36);
else if (cat1==cast) squash(pp,2,decl_head,-1,37);
else if (cat1==lbrace || cat1==int_like || cat1==decl) {
  if (indent_param_decl) big_app(dindent);
  squash(pp,1,fn_decl,0,38);
}
else if (cat1==semi) squash(pp,2,decl,-1,39);
@z

@x
@ @<Cases for |decl|@>=
if (cat1==decl) {
  big_app1_insert(pp,force); reduce(pp,2,decl,-1,40);
}
else if (cat1==stmt || cat1==function) {
  big_app1_insert(pp,big_force); reduce(pp,2,cat1,-1,41);
}
@y
@ @<Cases for |decl|@>=
if (cat1==decl) {
  big_app1_insert(pp,force); reduce(pp,2,decl,-1,40);
}
else if (cat1==stmt || cat1==function) {
  big_app1_insert(pp,order_decl_stmt ? big_force : force);
  reduce(pp,2,cat1,-1,41);
}
@z

@x
    make_underlined(pp+1); make_reserved(pp+1);
@y
    make_underlined(pp+1); make_reserved(pp+1); make_ministring(pp+1);
@z

@x
@ @<Cases for |fn_decl|@>=
if (cat1==decl) {
  big_app1_insert(pp,force); reduce(pp,2,fn_decl,0,51);
}
else if (cat1==stmt) {
  big_app1(pp); app(outdent); app(outdent); big_app(force);
  big_app1(pp+1); reduce(pp,2,function,-1,52);
}
@y
@ @<Cases for |fn_decl|@>=
if (cat1==decl) {
  big_app1_insert(pp,force); reduce(pp,2,fn_decl,0,51);
}
else if (cat1==stmt) {
  big_app1(pp);
  if (indent_param_decl) {
    app(outdent); app(outdent);
  }
  big_app(force); big_app1(pp+1); reduce(pp,2,function,-1,52);
}
@z

@x
  big_app1_insert(pp,dindent); reduce(pp,2,fn_decl,0,73);
@y
  big_app1(pp); if (indent_param_decl) big_app(dindent);
  big_app1(pp+1); reduce(pp,2,fn_decl,0,73);
@z

@x
  big_app1_insert(pp, (cat1==function || cat1==decl) ? big_force :
     force_lines ? force : break_space); reduce(pp,2,cat1,-1,76);
@y
  big_app1_insert(pp, (cat1==function || cat1==decl) ? @|
     ( order_decl_stmt ? big_force : force ) : @|
     ( force_lines ? force : break_space ) ); reduce(pp,2,cat1,-1,76);
@z

@x
if (cat1==define_like) make_underlined(pp+2);
@y
if (cat1==define_like) { /* \#\&{define} is analogous to \&{extern} */
  make_underlined(pp+2);
  if (tok_loc>operator_found) {
    /* no time to work out this case; I'll handle \&{define}s by brute force
       in the \.{aux} file, since they usually don't go in mini-index */
  }
}
@z

@x
if (cat1==prelangle) reduce(pp+1,0,langle,1,100);
else reduce(pp,0,exp,-2,101);
@y
if (cat1==prelangle) reduce(pp+1,0,langle,1,121);
else reduce(pp,0,exp,-2,122);
@z

@x
  big_app1_insert(pp,' '); reduce(pp,2,else_like,-2,102);
@y
  big_app1_insert(pp,' '); reduce(pp,2,else_like,-2,123);
@z

@x
@ @<Cases for |typedef_like|@>=
if ((cat1==int_like || cat1==cast) && (cat2==comma || cat2==semi))
  reduce(pp+1,0,exp,-1,115);
else if (cat1==int_like) {
  big_app1_insert(pp,' '); reduce(pp,2,typedef_like,0,116);
}
else if (cat1==exp && cat2!=lpar && cat2!=exp && cat2!=cast) {
  make_underlined(pp+1); make_reserved(pp+1);
  big_app1_insert(pp,' '); reduce(pp,2,typedef_like,0,117);
}
else if (cat1==comma) {
  big_app2(pp); big_app(' '); reduce(pp,2,typedef_like,0,118);
}
else if (cat1==semi) squash(pp,2,decl,-1,119);
else if (cat1==ubinop && (cat2==ubinop || cat2==cast)) {
  big_app('{'); big_app1_insert(pp+1,'}'); reduce(pp+1,2,cat2,0,120);
}
@y
@ Here \.{CTWILL} deviates from the normal productions introduced in
version 3.6, because those productions bypass |decl_head| (thereby
confusing |make_ministring|, which depends on the |decl_head| productions
to deduce the type). We revert to an older syntax that was
less friendly to \CPLUSPLUS/ but good enough for me.

@<Cases for |typedef_like|@>=
if (cat1==decl_head) {
  if ((cat2==exp&&cat3!=lpar&&cat3!=exp)||cat2==int_like) {
    make_underlined(pp+2); make_reserved(pp+2); make_ministring(pp+2);
    squash(pp+1,2,decl_head,0,200);
  }
  else if (cat2==semi) {
    big_app1(pp); big_app(' '); big_app2(pp+1); reduce(pp,3,decl,-1,201);
  }
} else if (cat1==int_like && cat2==raw_int &&
    (cat3==semi || cat3==comma)) reduce(pp+2,0,exp,1,202);
@z

@x
    overflow("token");
@y
    overflow(_("token"));
@z

@x
    overflow("text");
@y
    overflow(_("text"));
@z

@x
  if (tok_ptr+6>tok_mem_end) overflow("token");
@y
  if (tok_ptr+6>tok_mem_end) overflow(_("token"));
@z

@x
  printf("\nIrreducible scrap sequence in section %d:",(int)section_count);
@y
  printf(_("\nIrreducible scrap sequence in section %d:"),(int)section_count);
@z

@x
  printf("\nTracing after l. %d:\n",cur_line); mark_harmless;
@y
  printf(_("\nTracing after l. %d:\n"),cur_line); mark_harmless;
@z

@x
  case ignore: case xref_roman: case xref_wildcard:
@y
  case ignore: case xref_roman: case xref_wildcard:
  case meaning: case suppress:
@z

@x
  overflow("scrap/token/text");
@y
  overflow(_("scrap/token/text"));
@z

@x
      else err_print("! Double @@ should be used in strings");
@y
      else err_print(_("! Double @@ should be used in strings"));
@z

@x
      else app_scrap(p->ilk,maybe_math);
    }
  }
@y
      else app_scrap(p->ilk,maybe_math);
    }
  }
  @<Flag the usage of this identifier, for the mini-index@>@;
@z

@x
  if (next_control!='|') err_print("! Missing '|' after C text");
@y
  if (next_control!='|') err_print(_("! Missing '|' after C text"));
@z

@x
to \.{\\PB}.  Although \.{cwebmac} ignores \.{\\PB}, other macro packages
@y
to \.{\\PB}.  Although \.{ctwimac} ignores \.{\\PB}, other macro packages
@z

@x
        app(tok_flag+(int)(p-tok_start));
@y
        app(tok_flag+(int)(p-tok_start));
        app(inserted);
@z

@x
  if (stack_ptr==stack_end) overflow("stack");
@y
  if (stack_ptr==stack_end) overflow(_("stack"));
@z

@x
  fputs("\n! Illegal control code in section name: <",stdout);
@y
  fputs(_("\n! Illegal control code in section name: <"),stdout);
@z

@x
    fputs("\n! C text in section name didn't end: <",stdout);
@y
    fputs(_("\n! C text in section name didn't end: <"),stdout);
@z

@x
      if (j>buffer+long_buf_size-3) overflow("buffer");
@y
      if (j>buffer+long_buf_size-3) overflow(_("buffer"));
@z

@x
  if (j>buffer+long_buf_size-4) overflow("buffer");
@y
  if (j>buffer+long_buf_size-4) overflow(_("buffer"));
@z

@x
if (show_progress) fputs("\nWriting the output file...",stdout);
@y
if (show_progress) fputs(_("\nWriting the output file..."),stdout);
@z

@x
section_count=0; format_visible=true; copy_limbo();
@y
temp_switch=false; temp_meaning_ptr=temp_meaning_stack;
@<Read the \.{.aux} file, if present; then open it for output@>@;
section_count=0; format_visible=true; right_start_switch=false; copy_limbo();
@z

@x
static boolean group_found=false; /* has a starred section occurred? */

@ @<Translate the current section@>= {
  section_count++;
@y
static boolean group_found=false; /* has a starred section occurred? */
static boolean right_start_switch; /* has `\.{@@r}' occurred recently? */
static boolean temp_switch; /* has `\.{@@\%}' occurred recently? */

@ @d usage_sentinel (struct perm_meaning *)1
@<Translate the current section@>= {
  section_count++;
  temp_switch=false; temp_meaning_ptr=temp_meaning_stack;
  top_usage=usage_sentinel;
@z

@x
if (*(loc-1)!='*') out_str("\\M");
@y
if (*(loc-1)!='*') {
  if (right_start_switch) {
    out_str("\\shortpage\n"); right_start_switch=false;
@.\\shortpage@>
  }
  out_str("\\M");
}
@z

@x
@.\\N@>
@y
@.\\N@>
  if (right_start_switch) {
    out('N'); right_start_switch=false;
@.\\NN@>
  }
@z

@x
out('{'); out_section(section_count); out('}');
@y
out('{'); out_section(section_count); out('}');
flush_buffer(out_ptr,false,false);
@z

@x
    case '@@': out('@@'); break;
@y
    case '@@': out('@@'); break;
    case temp_meaning: temp_switch=!temp_switch; break;
    case right_start: right_start_switch=true; break;
@z

@x
    case section_name: loc-=2; next_control=get_next(); /* skip to \.{@@>} */
@y
    case meaning: case suppress:
    case section_name: loc-=2; next_control=get_next(); /* reprocess */
@z

@x
        err_print("! TeX string should be in C text only"); break;
@y
        err_print(_("! TeX string should be in C text only")); break;
@z

@x
        err_print("! You can't do that in TeX text"); break;
@y
        err_print(_("! You can't do that in TeX text")); break;
@z

@x
  outer_parse(); finish_C(format_visible); format_visible=true;
  doing_format=false;
}
@y
  outer_parse();
  if (is_macro) @<Make ministring for a new macro@>@;
  finish_C(format_visible); format_visible=true;
  doing_format=false;
}
@z

@x
@<Start a macro...@>= {
@y
@<Start a macro...@>= {
  is_macro=true;
@z

@x
    err_print("! Improper macro definition");
@y
    err_print(_("! Improper macro definition"));
@z

@x
@.Improper macro definition@>
  else {
    app_cur_id(false);
@y
@.Improper macro definition@>
  else {
    id_being_defined=id_lookup(id_first,id_loc,normal);
    app_cur_id(false);
    def_diff=(*loc!='(');
@z

@x
      default: err_print("! Improper macro definition"); break;
@y
      default: err_print(_("! Improper macro definition")); break;
@z

@x
@ @<Start a format...@>= {
  doing_format=true;
@y

@ @<Start a format...@>= {
  doing_format=true;
  is_macro=false;
@z

@x
  if (scrap_ptr!=scrap_info+2) err_print("! Improper format definition");
@y
  if (scrap_ptr!=scrap_info+2) err_print(_("! Improper format definition"));
@z

@x
  err_print("! You need an = sign after the section name");
@y
  err_print(_("! You need an = sign after the section name"));
@z

@x
  err_print("! You can't do that in C text");
@y
  err_print(_("! You can't do that in C text"));
@z

@x
out_str("\\fi"); finish_line();
@.\\fi@>
flush_buffer(out_buf,false,false); /* insert a blank line, it looks nice */
@y
finish_line(); out_str("\\mini"); finish_line();
@.\\mini@>
@<Output information about usage of id's defined in other sections@>@;
out_str("}\\FI"); finish_line();
@.\\FI@>
flush_buffer(out_buf,false,false); /* insert a blank line, it looks nice */
@z

@x
if (no_xref) {
  finish_line();
  out_str("\\end");
@.\\end@>
  finish_line();
}
@y
if (no_xref) {
  finish_line();
  out_str("\\end");
@.\\end@>
}
@z

@x
  phase=3; if (show_progress) fputs("\nWriting the index...",stdout);
@y
  phase=3; if (show_progress) fputs(_("\nWriting the index..."),stdout);
@z

@x
    fatal("! Cannot open index file ",idx_file_name);
@y
    fatal(_("! Cannot open index file "),idx_file_name);
@z

@x
  if (change_exists) {
    @<Tell about changed sections@>@; finish_line(); finish_line();
  }
@y
@z

@x
    fatal("! Cannot open section file ",scn_file_name);
@y
    fatal(_("! Cannot open section file "),scn_file_name);
@z

@x
@.\\end@>
  finish_line();
  fclose(active_file);
}
@y
@.\\end@>
}
finish_line(); fclose(active_file); active_file=tex_file=NULL;
if (check_for_change) @<Update the result when it has changed@>@;
@z

@x
  fputs("Done.",stdout);
@y
  fputs(_("Done."),stdout);
@z

@x
the index section itself.

@<Private...@>=
static sixteen_bits k_section; /* runs through the sections */
@y
the index section itself---NOT!
@z

@x
@ @<Tell about changed sections@>=
/* remember that the index is already marked as changed */
k_section=0;
while (!changed_section[++k_section]);
out_str("\\ch ");
@.\\ch@>
out_section(k_section);
while (k_section<section_count) {
  while (!changed_section[++k_section]);
  out_str(", "); out_section(k_section);
}
out('.');
@y
@ No need to tell about changed sections.
@z

@x
    if (sort_ptr>=scrap_info_end) overflow("sorting");
@y
    if (sort_ptr>=scrap_info_end) overflow(_("sorting"));
@z

@x
@ @<Output the name...@>=
switch (cur_name->ilk) {@+char *j;@+@t}\6{\4@>
@y
@ We don't format the index completely; the \.{twinx} program does the
rest of the job.

@<Output the name...@>=
switch (cur_name->ilk) {
@z

@x
  case normal: case func_template:
@y
  case normal:
@z

@x
    else {
@y
    else {@+char *j;@+@t}\6{@>
@z

@x
  case wildcard: out_str("\\9");@+ goto not_an_identifier;
@y
  case roman: out_str("  ");@+ goto not_an_identifier;
  case wildcard: out_str("\\9");@+ goto not_an_identifier;
@z

@x
  case roman: not_an_identifier: out_name(cur_name,false); goto name_done;
  case custom:
    out_str("$\\");
    for (j=cur_name->byte_start;j<(cur_name+1)->byte_start;j++)
      out(*j=='_'? 'x': *j=='$'? 'X': *j);
    out('$');
    goto name_done;
@y
not_an_identifier: out_name(cur_name,false); goto name_done;
  case custom: out_str("\\$"); break;
@.\\\$@>
@z

@x
out_name(cur_name,true);
@y
out_name(cur_name,proofing);
@z

@x
  puts("\nMemory usage statistics:");
@.Memory usage statistics:@>
  printf("%td names (out of %ld)\n",
            (ptrdiff_t)(name_ptr-name_dir),(long)max_names);
  printf("%td cross-references (out of %ld)\n",
            (ptrdiff_t)(xref_ptr-xmem),(long)max_refs);
  printf("%td bytes (out of %ld)\n",
            (ptrdiff_t)(byte_ptr-byte_mem),(long)max_bytes);
  puts("Parsing:");
  printf("%td scraps (out of %ld)\n",
            (ptrdiff_t)(max_scr_ptr-scrap_info),(long)max_scraps);
  printf("%td texts (out of %ld)\n",
            (ptrdiff_t)(max_text_ptr-tok_start),(long)max_texts);
  printf("%td tokens (out of %ld)\n",
            (ptrdiff_t)(max_tok_ptr-tok_mem),(long)max_toks);
  printf("%td levels (out of %ld)\n",
            (ptrdiff_t)(max_stack_ptr-stack),(long)stack_size);
  puts("Sorting:");
  printf("%td levels (out of %ld)\n",
            (ptrdiff_t)(max_sort_ptr-scrap_info),(long)max_scraps);
@y
  puts(_("\nMemory usage statistics:"));
@.Memory usage statistics:@>
  printf(_("%td names (out of %ld)\n"),
            (ptrdiff_t)(name_ptr-name_dir),(long)max_names);
  printf(_("%td cross-references (out of %ld)\n"),
            (ptrdiff_t)(xref_ptr-xmem),(long)max_refs);
  printf(_("%td bytes (out of %ld)\n"),
            (ptrdiff_t)(byte_ptr-byte_mem),(long)max_bytes);
  printf(_("%td temp meanings (out of %ld)\n"),
            (ptrdiff_t)(max_temp_meaning_ptr-temp_meaning_stack),
            (long)max_meanings);
  printf(_("%td titles (out of %ld)\n"),
            (ptrdiff_t)(title_code_ptr-title_code),(long)max_titles);
  puts(_("Parsing:"));
  printf(_("%td scraps (out of %ld)\n"),
            (ptrdiff_t)(max_scr_ptr-scrap_info),(long)max_scraps);
  printf(_("%td texts (out of %ld)\n"),
            (ptrdiff_t)(max_text_ptr-tok_start),(long)max_texts);
  printf(_("%td tokens (out of %ld)\n"),
            (ptrdiff_t)(max_tok_ptr-tok_mem),(long)max_toks);
  printf(_("%td levels (out of %ld)\n"),
            (ptrdiff_t)(max_stack_ptr-stack),(long)stack_size);
  puts(_("Sorting:"));
  printf(_("%td levels (out of %ld)\n"),
            (ptrdiff_t)(max_sort_ptr-scrap_info),(long)max_scraps);
@z

@x
@** Index.
@y
@q Section 271. @>
@** Mogrify {\tentex CWEAVE} into {\tentex CTWILL}.  The following sections
introduce material that is specific to \.{CTWILL}.

Care has been taken to keep the original section numbering of \.{CWEAVE}
up to this point intact, so this new material should nicely integrate
with the original ``\&{271.~Index}.''

@q Section 2->272. @>
@* {\tentex CTWILL} user manual.
Here is a sort of user manual for \.{CTWILL}---which is exactly like
\.{CWEAVE} except that it produces much better documentation, for which you
must work harder. As with \.{CWEAVE}, input comes from a source file
\.{foo.w} and from an optional (but now almost mandatory) change file
\.{foo.ch}; output goes to \.{foo.tex}, \.{foo.idx}, and \.{foo.scn}.
Unlike \.{CWEAVE}, there is an additional output file, \.{foo.aux},
which records all nonexternal definitions.  The \.{.aux} file also
serves as an input file on subsequent runs. You should run \.{CTWILL}
twice, once to prime the pump and once to get decent answers.

Moreover, you must run the output twice through \TeX. (This double duplicity
suggested the original name \.{TWILL}.) After `\.{tex} \.{foo}' you
will have output that looks like final pages except that the entries
of mini-indexes won't be alphabetized. \TeX\ will say `This is the first
pass', and it will produce a weird file called \.{foo.ref}. Say
$$\.{refsort < foo.ref > foo.sref}$$
and then another `\.{tex} \.{foo}' will produce alphabetized output.
While \TeX\ runs it emits messages filled with numeric data, indicating how
much space is consumed by each program section. If you can decipher these
numbers (see \.{ctwimac.tex}), you can use them to fine-tune the page
layout. You might be tempted to do fine tuning by editing \.{foo.tex}
directly, but it's better to incorporate all changes into \.{foo.ch}.

The mini-indexes list identifiers that are used but not defined on
each two-page spread. At the end of each section, \.{CTWILL} gives
\TeX\ a list of identifiers used in that section and information
about where they are defined. The macros in \.{ctwimac.tex} figure out
which identifiers should go in each mini-index, based on how the pages
break. (Yes, those macros are pretty hairy.)

The information that \.{CTWILL} concocts from \.{foo.w} is not always
correct. Sometimes you'll use an identifier that you don't want
indexed; for example, your exposition might talk about |f(x)| when you
don't mean to refer to program variables |f| or |x|. Sometimes you'll
use an identifier that's defined in a header file, unknown to
\.{CTWILL}. Sometimes you'll define a single identifier in several
different places, and \.{CTWILL} won't know which definition to choose.
But all is not lost. \.{CTWILL} guesses right most of the time, and you can
give it the necessary hints in other places via your change file.

If you think it's easy to write a completely automatic system that doesn't
make \.{CTWILL}'s mistakes and doesn't depend so much on change files,
please do so.

\.{CTWILL} uses a very simple method to generate mini-index info. By
understanding this method, you will understand how to fix it when things
go wrong. Every identifier has a current ``meaning,'' consisting of its
abstract type and the number of the section in which it was most recently
defined. For example, if your \Cee\ program says `\&{char}~$*$|s|' in section~3,
the meaning of~|s| gets changed to `\&{char}~$*$,~\S3' while \.{CTWILL}
is processing that section. If you refer to~|s| in section~10, and if
|s|~hasn't been redefined in the meantime, and if section~10 doesn't
wind up on the same two-page spread as section~3, the mini-index generated
by section~10 will say ``|s|:~\&{char}~$*$,~\S3.''

The current meaning of every identifier is initially `\.{\\uninitialized}'.
Then \.{CTWILL} reads the \.{.aux} file for your job, if any; this
\.{.aux} file contains all definitions of new meanings in the previous
run, so it tells \.{CTWILL} about definitions that will be occurring
in the future. If all identifiers have a unique definition, they will
have a unique and appropriate meaning in the mini-indexes.

But some identifiers, like parameters to procedures, may be defined
several times. Others may not be defined at all, because they are
defined elsewhere and mentioned in header files included by the \Cee\
preprocessor. To solve this problem, \.{CTWILL} provides mechanisms by which
the current meaning of an identifier can be temporarily or permanently
changed.

For example, the operation
$$\.{@@\$s \{FOO\}3 \\\&\{char\} \$*\$@@>}$$
changes the current meaning of |s| to the \TeX\ output of `\.{\\\&\{char\}}
\.{\$*\$}' in section~3 of program {\sc FOO}. All entries in the \.{.aux}
file are expressed in the form of this \.{@@\$} operator; therefore you
can use a text editor to paste such entries into a \.{.ch} file, whenever
you want to tell \.{CTWILL} about definitions that are out of order
or from other programs.

Before reading the \.{.aux} file, \.{CTWILL} actually looks for a file
called \.{system.bux}, which will be read if present. And after
\.{foo.aux}, a third possibility is \.{foo.bux}. The general
convention is to put definitions of system procedures such as |printf|
into \.{system.bux}, and to put definitions found in specifically
foo-ish header files into \.{foo.bux}. Like the \.{.aux}
files, \.{.bux} files should contain only \.{@@\$} specifications;
this rule corresponds to the fact that `bux' is the plural of `\$'.
The \.{.bux} files may also contain \.{@@i} includes.

A companion operation \.{@@\%} signifies that all \.{@@\$}
specifications from the present point to the beginning of the next
section will define {\it temporary\/} meanings instead of permanent
ones. Temporary meanings are placed into the
mini-index of the current section; the permanent (current) meaning of
the identifier will not be changed, nor will it appear in the
mini-index of the section. If several temporary meanings are assigned
to the same identifier in a section, all will appear in the mini-index.
Each \.{@@\%} toggles the temporary/permanent convention; thus, after
an even number of \.{@@\%} operations in a section, meanings specified
by \.{@@\$} are permanent.

The operation \.{@@-} followed by an identifier followed by \.{@@>}
specifies that the identifier should not generate a mini-index entry
in the current section (unless, of course, a temporary meaning is assigned).

If \.{@@-foo@@>} appears in a section where a new permanent meaning is
later defined by the semantics of~\Cee, the current meaning of \\{foo}
will not be redefined; moreover, this current meaning, which may have
been changed by \.{@@\$foo ...@@>}, will also be written to the
\.{.aux} file. Therefore you can control what \.{CTWILL} outputs; you
can keep it from repeatedly contaminating the \.{.aux} file with
things you don't like.

The meaning specified by \.{@@\$...@@>} generally has four components:
an identifier (followed by space), a program name (enclosed in braces),
a section number (followed by space), and a \TeX\ part. The \TeX\ part
must have fewer than 50 characters. If the \TeX\ part starts
with `\.=', the mini-index entry will contain an equals sign instead
of a colon; for example,
$$\.{@@\$buf\_size \{PROG\}10 =\\T\{200\}@@>}$$
generates either `$\\{buf\_size}=200$, \S10' or
`$\\{buf\_size}=200$, {\sc PROG}~\S10', depending on whether
`{\sc PROG}' is or isn't the title of the current program. If the
\TeX\ part is `\.{\\zip}', the mini-index entry will contain neither
colon nor equals, just a comma. The program name and section number
can also be replaced by a string. For example,
$$\.{@@\$printf "<stdio.h>" \\zip@@>}$$
will generate a mini-index entry like `\\{printf}, \.{<stdio.h>}.'.

A special ``proofmode'' is provided so that you can check \.{CTWILL}'s
conclusions about cross-references. Run \.{CTWILL} with the
flag \.{+P}, and \TeX\ will produce a specially formatted document
({\it without\/} mini-indexes)
in which you can check that your specifications are correct.
You should always do this before generating mini-indexes, because
mini-indexes can mask errors if page breaks are favorable but the
errors might reveal themselves later after your program has changed.
The proofmode output is much easier to check than the mini-indexes themselves.

The control code \.{@@r} or \.{@@R} causes \.{CTWILL} to emit
the \TeX\ macro `\.{\\shortpage}' just before starting the next
section of the program. This causes the section to appear at the top of
a right-hand page, if it would ordinarily have appeared near the bottom
of a left-hand page and split across the pages. (The \.{\\shortpage} macro
is fragile and should be used only in cases where it will not mess up
the output; insert it only when fine-tuning a set of pages.) If the
next section is a starred section, the behavior is slightly different
(but still fragile): The starred section will either be postponed to
a left-hand page, if it normally would begin on a right-hand page,
or vice versa. In other words, \.{@@r@@*} inverts the left/right logic.

\.{CTANGLE} does not recognize the operations \.{@@\$}, \.{@@\%}, \.{@@-},
and \.{@@r}, which are unique to \.{CTWILL}. But that is no problem,
since you use them only in change files set up for book publishing,
which are quite different from the change files you set up for tangling.

(End of user manual.)

@d max_tex_chars 50 /* limit on the \TeX\ part of a meaning */

@q Section 25->273. @>
@* Temporary and permanent meanings.
\.{CTWILL} has special data structures to keep track of current
and temporary meanings. These structures were not designed for maximum
efficiency; they were designed to be easily grafted into \.{CWEAVE}'s
existing code without major surgery.

@d max_meanings 100 /* max temporary meanings per section */
@d max_titles 100 /* max distinct program or header names in meanings */

@<Type...@>=
typedef struct {
  name_pointer id; /* identifier whose meaning is being recorded */
  sixteen_bits prog_no; /* title of program or header in which defined */
  sixteen_bits sec_no; /* section number in which defined */
  char tex_part[max_tex_chars]; /* \TeX\ part of meaning */
} meaning_struct;

@q Section 26->274. @>
@ @<Private...@>=
static struct perm_meaning {
  meaning_struct perm; /* current meaning of an identifier */
  int stamp; /* last section number in which further output suppressed */
  struct perm_meaning *link; /* another meaning to output in this section */
} cur_meaning[max_names]; /* the current ``permanent'' meanings */
static struct perm_meaning *top_usage; /* first meaning to output in this section */
static meaning_struct temp_meaning_stack[max_meanings]; /* the current ``temporary'' meanings */
static meaning_struct *temp_meaning_stack_end=temp_meaning_stack+max_meanings-1;
  /* end of |temp_meaning_stack| */
static meaning_struct *temp_meaning_ptr; /* first available slot in |temp_meaning_stack| */
static meaning_struct *max_temp_meaning_ptr; /* its maximum value so far */
static name_pointer title_code[max_titles]; /* program names seen so far */
static name_pointer *title_code_end=title_code+max_titles-1;
  /* end of |title_code| */
static name_pointer *title_code_ptr; /* first available slot in |title_code| */
static char ministring_buf[max_tex_chars]; /* \TeX\ code being generated */
static char *ministring_buf_end=ministring_buf+max_tex_chars-1;
  /* end of |ministring_buf| */
static char *ministring_ptr; /* first available slot in |ministring_buf| */
static boolean ms_mode; /* are we outputting to |ministring_buf|? */

@q Section 27->275. @>
@ @<Set init...@>=
max_temp_meaning_ptr=temp_meaning_stack;
title_code_ptr=title_code;
ms_mode=false;

@q Section 276. @>
@ @<Predec...@>=@+static void new_meaning(name_pointer);

@q Section 30->277. @>
@ The |new_meaning| routine changes the current ``permanent meaning''
when an identifier is redeclared. It gets the |tex_part| from
|ministring_buf|.

@c
static void
new_meaning(
  name_pointer p)
{
  struct perm_meaning *q=get_meaning(p);
  ms_mode=false;
  if (q->stamp!=section_count) {
    if (*(ministring_ptr-1)==' ') ministring_ptr--;
    if (ministring_ptr>=ministring_buf_end)
      strcpy(ministring_buf,"\\zip"); /* ignore |tex_part| if too long */
@.\\zip@>
    else *ministring_ptr='\0';
    q->perm.prog_no=0; /* |q->perm.id=p| */
    q->perm.sec_no=section_count;
    strcpy(q->perm.tex_part,ministring_buf);
  }
  @<Write the new meaning to the \.{.aux} file@>@;
}

@q Section 75->278. @>
@ @<Process a user-generated meaning@>=
{ char *first=id_first;
  while (xisspace(*first)) first++;
  loc=first;
  while (xisalpha(*loc)||xisdigit(*loc)||*loc=='_') loc++;
  if (*loc++!=' ')
    err_print(_("! Identifier in meaning should be followed by space"));
  else {@+ int n=0;
    name_pointer p=id_lookup(first,loc-1,normal);
    sixteen_bits t=title_lookup();
    if (*(loc-1)=='}')
      while (xisdigit(*loc)) n=10*n+(*loc++)-'0';
    if (*loc++!=' ')
      err_print(_("! Location in meaning should be followed by space"));
    else @<Digest the meaning of |p|, |t|, |n|@>@;
  }
  loc=id_loc+2;
}

@q Section 76->279. @>
@ @<Suppress mini-index entry@>=
{ char *first=id_first,*last=id_loc;
  while (xisspace(*first)) first++;
  while (xisspace(*(last-1))) last--;
  if (first<last) {
    struct perm_meaning *q=get_meaning(id_lookup(first,last,normal));
    q->stamp=section_count; /* this is what actually suppresses output */
  }
}

@q Section 77->280. @>
@ @<Digest...@>=
{ meaning_struct *m;
  struct perm_meaning *q=get_meaning(p);
  if (temp_switch) {
    m=temp_meaning_ptr++;
    if (temp_meaning_ptr>max_temp_meaning_ptr) {
      if (temp_meaning_ptr>=temp_meaning_stack_end)
        overflow(_("temp meanings"));
      max_temp_meaning_ptr=temp_meaning_ptr;
    }
  } else m=&(q->perm);
  m->id=p;
  m->prog_no=t;
  m->sec_no=n;
  if (id_loc-loc>=max_tex_chars) strcpy(m->tex_part,"\\zip");
@.\\zip@>
  else { char *q=m->tex_part;
    while (loc<id_loc) *q++=*loc++;
    *q='\0';
  }
}

@q Section 141->281/2. @>
@* Make ministrings.
 \.{CTWILL} needs the following procedure, which appends tokens of a
translated text until coming to |tok_loc|, then suppresses text that may
appear between parentheses or brackets. The calling routine |make_ministring|
should set |ident_seen=false| first. (This is admittedly tricky.)

@<Private var...@>=
static boolean ident_seen;

@ @c
static boolean app_supp(
  text_pointer p)
{ token_pointer j;
  if (ident_seen && **p>=tok_flag)
    switch (**(**p-tok_flag+tok_start)) {
    case '(': app_str("(\\,)"); goto catch14;
    case '[': app_str("[\\,]"); goto catch14;
    }
  for (j=*p;j<*(p+1);j++)
    if (*j<tok_flag) {
      if (*j==inserted) break;
      if (j==tok_loc) ident_seen=true;
      else app(*j);
    } else if (*j>=inner_tok_flag) confusion(_("inner"));
    else if (app_supp(*j-tok_flag+tok_start)) goto catch14;
  return false;
catch14: return *(*(p+1)-1)=='9'; /* was production 14 used? */
}

@q Section 283. @>
@ @<Predec...@>=@+static boolean app_supp(text_pointer);

@q Section 142->284. @>
@ The trickiest part of \.{CTWILL} is the procedure |make_ministring(pp+l)|,
with offset $l\in\{0,1,2\}$, which tries to figure out a symbolic form of
definition after |make_underlined(pp+l)| has been called. We rely heavily
on the existing productions, which force the translated texts to have a
structure that's decodable even though the underlying |cat| and |mathness|
codes have disappeared.

@c static void
make_ministring(scrap_pointer p)
{
  if (tok_loc<=operator_found) return;
  @<Append the type of the declaree; |return| if it begins with \&{extern}@>@;
  null_scrap.mathness=((p->mathness)%4)*5; big_app1(&null_scrap);
    /* now we're ready for the mathness that follows (I think);
       (without the mod 4 times 5, comments posed a problem,
       namely in cases like |int a(b,c)| followed by comment) */
  ident_seen=false;@+app_supp(p->trans);
  null_scrap.mathness=10; big_app1(&null_scrap);
   /* now |cur_mathness==no_math| */
  ms_mode=true; ministring_ptr=ministring_buf;
  if (p==pp+2) *ministring_ptr++='=';
  make_output(); /* translate the current text into a ministring */
  tok_ptr=*(--text_ptr); /* delete that text */
  new_meaning(((*tok_loc)%id_flag)+name_dir);
  cur_mathness=maybe_math; /* restore it */
}

@q Section 285. @>
@ @<Predec...@>=@+static void make_ministring(scrap_pointer);

@q Section 43->286. @>
@ @<Private...@>=
static sixteen_bits int_loc, ext_loc; /* locations of special reserved words */

@q Section 143->287. @>
@ Here we use the fact that a |decl_head| comes from |int_like| only in
production~27, whose translation is fairly easy to recognize. (Well,
production 28 has been added for \CPLUSPLUS/, but we hope that doesn't
mess us up.) And we also use other similar facts.

If an identifier is given an \&{extern} definition, we don't change
its current meaning, but we do suppress mini-index entries to its
current meaning in other sections.

@<Append the type of the declaree; |return| if it begins with \&{extern}@>=
if (p==pp) { app(int_loc+res_flag); app(' '); cur_mathness=no_math; }
else {
  text_pointer q=(p-1)->trans, r;
  token t;
  int ast_count=0; /* asterisks preceding the expression */
  boolean non_ast_seen=false; /* have we seen a non-asterisk? */
  while (true) {
    if (*(q+1)==*q+1) {
      r=q;@+break; /* e.g., \&{struct}; we're doing production 45 or 46 */
    }
    if (**q<tok_flag) confusion(_("find type"));
    r=**q-tok_flag+tok_start;
    if ((t=*(*(q+1)-2))>=tok_flag && **(t-tok_flag+tok_start)=='*') {
           /* production 34 */
      if (!non_ast_seen) ast_count++; /* count immediately preceding |*|'s */
    } else non_ast_seen=true;
    if (*(*q+1)==' ' && *(q+1)==*q+2) break; /* production 27 */
    if (*(*q+1)=='{' && *(*q+2)=='}' && *(*q+3)=='$' && *(*q+4)==' '@|
       && *(q+1)==*q+5) break; /* production 27 in disguise */
    q=r;
  }
  while (**r>=tok_flag) {
    if (*(r+1)>*r+9 && *(*r+1)=='{' && *(*r+2)=='}' && *(*r+3)=='$' @|
        && *(*r+4)==indent) q=**r-tok_flag+tok_start; /* production 49 */
    r=**r-tok_flag+tok_start;
  }
  if (**r==ext_loc+res_flag) return; /* \&{extern} gives no definition */
  @<Append tokens for type |q|@>@;
}

@q Section 144->288. @>
@ @<Append tokens for type |q|@>=
cur_mathness=no_math; /* it was |maybe_math| */
if (*(q+1)==*q+8 && *(*q+1)==' ' && *(*q+3)==' ') {
  app(**q);@+app(' ');@+app(*(*q+2)); /* production 46 */
} else if ((t=*(*(q+1)-1))>=tok_flag && **(r=t-tok_flag+tok_start)=='\\'
   && *(*r+1)=='{') app(**q); /* |struct_like| identifier */
else app((q-tok_start)+tok_flag);
while (ast_count) {
  big_app('{');@+app('*');@+app('}');@+ast_count--;
}

@q Section 253->289. @>
@ @<Private...@>=
static boolean is_macro; /* it's a macro def, not a format def */
static boolean def_diff; /* |false| iff the current macro has parameters */
static name_pointer id_being_defined; /* the definee */

@q Section 257->290. @>
@ @<Make ministring for a new macro@>=
{
  ms_mode=true; ministring_ptr=ministring_buf;
  *ministring_ptr++='=';
  if (def_diff) { /* parameterless */
    scrap_pointer s=scrap_ptr;
    text_pointer t;
    token_pointer j;
    while (s->cat==insert) s--;
    if ((s-1)->cat==dead && s->cat==exp && **(t=s->trans)=='\\'
         && *(*t+1)=='T') /* it's just a constant */
      for (j=*t;j<*(t+1);j++) *ministring_ptr++=*j;
    else out_str("macro");
  } else out_str("macro (\\,)");
  new_meaning(id_being_defined);
}

@q Section 246->291. @>
@* Process {\tentex .aux} files.

@<Private...@>=
static FILE *aux_file;
static char aux_file_name[max_file_name_length]; /* name of \.{.aux} file */

@q Section 247->292. @>
@ @<Read the \.{.aux} file, if present; then open it for output@>=
memcpy(aux_file_name,tex_file_name,strlen(tex_file_name)-4);
strcat(aux_file_name,".bux");
include_depth=1; /* we simulate \.{@@i} */
strcpy(cur_file_name,aux_file_name); /* first in, third out */
if ((cur_file=fopen(cur_file_name,"r"))) { cur_line=0; include_depth++; }
strcpy(aux_file_name+strlen(aux_file_name)-4,".aux");@/
strcpy(cur_file_name,aux_file_name); /* second in, second out */
if ((cur_file=fopen(cur_file_name,"r"))) { cur_line=0; include_depth++; }
strcpy(cur_file_name,"system.bux"); /* third in, first out */
if ((cur_file=fopen(cur_file_name,"r"))) cur_line=0;
else include_depth--;
if (include_depth) { /* at least one new file was opened */
  while (get_next()==meaning) ; /* new meaning is digested */
  if (include_depth) err_print(_("! Only @@$ is allowed in aux and bux files"));
  finish_line(); loc=buffer; /* now reading beginning of line 1 */
}
if ((aux_file=fopen(aux_file_name,"wb"))==NULL)
  fatal(_("! Cannot open aux output file "),aux_file_name);

@q Section 31->293. @>
@ @<Write the new meaning to the \.{.aux} file@>=
{@+int n=q->perm.prog_no;
  fprintf(aux_file,"@@$%.*s %.*s",@|
    (int)length(p),p->byte_start,@|
    (int)length(title_code[n]),title_code[n]->byte_start);
  if (*(title_code[n]->byte_start)=='{') fprintf(aux_file,"%d",q->perm.sec_no);
  fprintf(aux_file," %s@@>\n",q->perm.tex_part);
}

@q Section 268->294. @>
@* Usage of identifiers.
The following code is performed for each identifier parsed during
a section. Variable |top_usage| is always nonzero; it has the sentinel
value~1 initially, then it points to each variable scheduled for
possible citation. A variable is on this list if and only if its
|link| field is nonzero. All variables mentioned in the section are
placed on the list, unless they are reserved and their current
\TeX\ meaning is uninitialized.

@<Flag the usage of this identifier, for the mini-index@>=
{ struct perm_meaning *q=get_meaning(p);
  if (!abnormal(p) || strcmp(q->perm.tex_part,"\\uninitialized")!=0)
    if (q->link==NULL) {
      q->link=top_usage;
      top_usage=q;
    }
}

@q Section 269->295. @>
@ @<Output information about usage of id's defined in other sections@>=
{ struct perm_meaning *q;
  while (temp_meaning_ptr>temp_meaning_stack) {
    out_mini(--temp_meaning_ptr);
    q=get_meaning(temp_meaning_ptr->id);
    q->stamp=section_count; /* suppress output from ``permanent'' data */
  }
  while (top_usage!=usage_sentinel) {
    q=top_usage;
    top_usage=q->link;
    q->link=NULL;
    if (q->stamp!=section_count) out_mini(&(q->perm));
  }
}

@q Section 270->296. @>
@ @c static void
out_mini(
  meaning_struct *m)
{ char s[60];
  name_pointer cur_name=m->id;
  if (m->prog_no==0) { /* reference within current program */
    if (m->sec_no==section_count) return; /* defined in current section */
    sprintf(s,"\\[%d",m->sec_no);
  } else { name_pointer n=title_code[m->prog_no];
    if (*(n->byte_start)=='{')
      sprintf(s,"\\]%.*s%d",(int)length(n),n->byte_start,m->sec_no);
    else sprintf(s,"\\]%.*s",(int)length(n),n->byte_start);
  }
  out_str(s); out(' ');
  @<Mini-output the name at |cur_name|@>@;
  out(' '); out_str(m->tex_part); finish_line();
}

@q Section 271->297. @>
@ @<Predec...@>=@+static void out_mini(meaning_struct *);

@q Section 272->298. @>
@ Compare this code with section |@<Output the name...@>|.

@<Mini-output...@>=
switch (cur_name->ilk) {@+char *j;@+@t}\6{\4@>
  case normal: case func_template:
    if (is_tiny(cur_name)) out_str("\\|");
    else {
      for (j=cur_name->byte_start;j<(cur_name+1)->byte_start;j++)
        if (xislower(*j)) goto lowcase;
      goto allcaps;
lowcase: out_str("\\\\");
    }
  break;
@.\\|@>
@.\\.@>
@.\\\\@>
  case wildcard: out_str("\\9"); break;
@.\\9@>
  case typewriter: allcaps: out_str("\\.");
@.\\.@>
  case roman: break;
  case custom:
    out_str("$\\");
    for (j=cur_name->byte_start;j<(cur_name+1)->byte_start;j++)
      out(*j=='_'? 'x': *j=='$'? 'X': *j);
    out('$');
    goto name_done;
  default: out_str("\\&");
@.\\\&@>
}
out_name(cur_name,true);
name_done:@;

@q Section 28->299. @>
@* Handle program title.
Here's a routine that converts a program title from the buffer
into an internal number for the |prog_no| field of a meaning.
It advances |loc| past the title found.

@c static sixteen_bits title_lookup(void)
{
  char *first=loc,*last; /* boundaries */
  register name_pointer *p;
  if (*loc=='"') {
    while (++loc<=limit && *loc!='"') if (*loc=='\\') loc++;
  } else if (*loc=='{') {
    int balance=1; /* excess of left over right */
    while (++loc<=limit) {
      if (*loc==' ' && balance==1)
        *loc='}'; /* Skip ``version'' after module name and fall through */
      if (*loc=='}' && --balance==0) break;
      if (*loc=='{') balance++;
    }
  } else err_print(_("! Title should be enclosed in braces or doublequotes"));
  last=++loc;
  if (last>limit) err_print(_("! Title name didn't end"));
  if (title_code_ptr==title_code_end) overflow(_("titles"));
  *title_code_ptr=id_lookup(first,last,title);
  for (p=title_code;true;p++) if (*p==*title_code_ptr) break;
  if (p==title_code_ptr) title_code_ptr++;
  return p-title_code;
}

@q Section 300. @>
@ @<Predec...@>=@+static sixteen_bits title_lookup(void);

@q Section 29->301. @>
@ @<Give a default title to the program, if necessary@>=
if (title_code_ptr==title_code) { /* no \.{\\def\\title} found in limbo */
  char *saveloc=loc,*savelimit=limit; /* save */
  loc=limit+1; limit=loc;
  *limit++='{';
  memcpy(limit,tex_file_name,strlen(tex_file_name)-4);
  limit+=strlen(tex_file_name)-4;
  *limit++='}';
  title_lookup();@/
  loc=saveloc; limit=savelimit; /* restore */
}

@q Section 302. @>
@** Extensions to {\tentex CWEB}.  The following sections introduce new or
improved features that have been created by numerous contributors over the
course of a quarter century.

@* Formatting alternatives.
\.{CWEAVE} indents declarations after old-style function definitions and
long parameter lists of modern function definitions.
With the \.{-i} option they will come out flush left.

@d indent_param_decl flags['i'] /* should formal parameter declarations be indented? */

@<Set init...@>=
indent_param_decl=true;

@ The original manual described the \.{-o} option for \.{CWEAVE}, but this was
not yet present.  Here is a simple implementation.  The purpose is to suppress
the extra space between local variable declarations and the first statement in
a function block.

@d order_decl_stmt flags['o'] /* should declarations and statements be separated? */

@<Set init...@>=
order_decl_stmt=true;

@* Output file update. Most \CEE/ projects are controlled by a \.{Makefile}
that automatically takes care of the temporal dependecies between the different
source modules. It may be convenient that \.{CWEB} doesn't create new output
for all existing files, when there are only changes to some of them. Thus the
\.{make} process will only recompile those modules where necessary. You can
activate this feature with the `\.{+c}' command-line option. The idea and basic
implementation of this mechanism can be found in the program \.{NUWEB} by
Preston Briggs, to whom credit is due.

@<Update the result...@>= {
if((tex_file=fopen(tex_file_name,"r"))!=NULL) {
  boolean comparison=false;

  if((check_file=fopen(check_file_name,"r"))==NULL)
    fatal(_("! Cannot open output file "),check_file_name);
@.Cannot open output file@>

  @<Compare the temporary output...@>@;

  fclose(tex_file); tex_file=NULL;
  fclose(check_file); check_file=NULL;

  @<Take appropriate action depending on the comparison@>@;
} else
  rename(check_file_name,tex_file_name); /* This was the first run */

strcpy(check_file_name,""); /* We want to get rid of the temporary file */
}

@ We hope that this runs fast on most systems.

@<Compare the temporary output to the previous output@>=
do {
  char x[BUFSIZ],y[BUFSIZ];
  int x_size = fread(x,sizeof(char),BUFSIZ,tex_file);
  int y_size = fread(y,sizeof(char),BUFSIZ,check_file);
  comparison = (x_size == y_size) && !memcmp(x,y,x_size);
} while(comparison && !feof(tex_file) && !feof(check_file));

@ Note the superfluous call to |remove| before |rename|.  We're using it to
get around a bug in some implementations of |rename|.

@<Take appropriate action...@>=
if(comparison)
  remove(check_file_name); /* The output remains untouched */
else {
  remove(tex_file_name);
  rename(check_file_name,tex_file_name);
}

@* Print ``version'' information.
Don't do this at home, kids! Push our local macro to the variable in \.{COMMON}
for printing the |banner| and the |versionstring| from there.

@d max_banner 50

@<Common code...@>=
extern char cb_banner[];

@ @<Set init...@>=
  strncpy(cb_banner,banner,max_banner-1);

@** Index.
@z

