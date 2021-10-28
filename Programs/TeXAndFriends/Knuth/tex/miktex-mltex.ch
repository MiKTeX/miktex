% miktex-mltex.ch: mltex.ch modified for MiKTeX (inspired by Web2C's tex.ch)
% Modifications by Christian Schenk.

% This is MLTEX.CH (Version 2.2) in text format, as of Dec 17, 1995.
% WEB change file containing code for various features extending TeX; to
% be applied to TEX.WEB (Version 3.14159) in order to generate MLTEX.WEB.
 
% MLTeX is copyright (C) 1990-92 by Michael J. Ferguson; all rights are reserved.
% MLTeX Version 2.2 is copyright (C) 1995 by B. Raichle; all rights are reserved.
%
%
% The MLTeX changes are copyrighted so that we have some chance to
% forbid unauthorized copies; we explicitly authorize copying of
% correct MLTeX implementations, and not of incorrect ones!
%
% (This means that you can use the MLTeX changes as free as you can
% use TeX and its algorithm.)
%
% Copying of this file is authorized only if either
% (1) you make absolutely no changes to your copy, including name, or
% (2) if you do make changes, you name it to something other than
%     "mltex.ch", "char_sub.ch", or "charsub.ch".


% The TeX program is copyright (C) 1982 by D. E. Knuth.
% TeX is a trademark of the American Mathematical Society.

% MLTeX Version 2.2 was created by Bernd Raichle
% All line numbers refer to TEX.WEB 3.14159 as of March, 1995.


%============================================================
% INSTALLATION NOTICES:
%
% === Creating MLTeX (general information) ===
%
% 1) Merging the change files:
%
% In order to generate an implementation of MLTeX your implementation
% dependent change file and MLTeX's change file "mltex.ch" have to be
% applied to "tex.web", one after the other.
%   The process of merging several change files into "tex.web" should
% certainly not be performed by hand.  There are programs such as TIE
% and PATCHWEB that perform this process automatically.  If no such
% program is available, a TeX program "webmerge.tex" can be used.
%   Examples: To merge the two changes files "mltex.ch" and "tex.ch"
% using tie:
%
%      tie -c newtex.ch tex.web mltex.ch tex.ch
%
% To do the same using "webmerge.tex" you have to apply TeX to the
% following driver input file:
%
%      \input webmerge       % needs `webmerge.tex'
%      \webfile{tex.web}
%      \changefile{mltex.ch}
%      \changefile{tex.ch}
%      \outfile{newtex.ch}   % output file, start processing
%      \bye
%
%   Before merging the change files it might be necessary to make
% small changes in one of the two change files or, as a more elegant
% way, to add an additional change file for theses some changes which
% interfere between the change files.  One of these interferences is
% caused by the change of the banner line ([1] m.2 l.187).
%
% 2) Tangling TeX:
%
% Apply TANGLE on "tex.web" and the combined change file "newtex.ch":
%
%      tangle tex.web newtex.ch
%
% 3) Post-processing the Pascal source "tex.p":
%
% Because of a restriction in TANGLE's macros, you have to replace
% the strings "XLPAREN" and "XRPAREN" (resp. "xlparen"/"xrparen")
% with a left parenthesis "(" and a right parenthesis ")" by hand.
%   If the Unix tool "sed" is available you can use
%
%      sed  -e 's/xlparen/(/g' -e 's/xrparen/)/g' < tex.p > mltex.p
%      mv mltex.p tex.p
%
% 4) Compile TeX and apply the TRIP test:
%
% The final step will be the compilation of (ML)TeX.  You should
% apply the TRIP test to the resulting program to make sure that your
% MLTeX will be compatible to TeX unless one of the new feature is
% used.
%
%
% === Creating MLTeX using web2c v7.0 ===
%
% MLTeX v2.2 is part of the TeX in web2c v7.0, you can use the
% "-mltex" option to enable MLTeX.  Please read the documentation
% of web2c for more information.
%
%
% === Creating MLTeX using web2c prior to v7.0 ===
%
% To create a new change file for web2c v6.1, adapt the change of the
% banner line ([1] m.2 l.187) to the used TeX version.  Then use
% `tie' to merge the two change files:
%
%      tie -c newtex.ch tex.web mltex.ch tex.ch
%
% Make sure that the combined change file "newtex.ch" will identify
% the resulting TeX as a MLTeX, i.e. make sure that the banner line
% is changed accordingly.
%
% Use tangle:
%
%      tangle tex.web newtex.ch
%
% then replace the string "xlparen" with "(" and "xrparen" with `)'
% by using "sed":
%
%      sed  -e 's/xlparen/(/g' -e 's/xrparen/)/g' < tex.p > mltex.p 
%      mv mltex.p tex.p
%
% Finally convert the new Pascal source `tex.p' to a C Source using
% `web2c' and compile it.
%
%
%============================================================
% Differences between MLTeX version 2.1 and 2.2:
%
% * \charsubdefmax has an initial version of -1
% * All non-existing characters of a font are substituted if a
%   \charsubdef is given  (the former version doesn't substitute
%   characters with codes < highest font character code)
% * Changed log output texts.
% * There will be no |push| and |pop| operations in the dvi file
%   for substituted accent constructions
% * With \tracingoutput enabled missing characters are reported
%   _before_ the box contents is shown and the box will not contain
%   additional character nodes showing the substitutions
%
% Bugs fixed:
% * Error message ("Bad metric (TFM) file.") while loading a font
%   was shown in versions 2.01 and older in some cases.
% * No character substitution for large math delimiter character
% * If a character substitution was used and then removed (e.g. by
%   \charsubdef <char>=0 0 or \charsubdefmax=-1), all accesses
%   to |font_info| for this non-existing character are invalid and
%   undefined (=> runtime errors, access violations!).  These
%   accesses are always done in |char_info| and they occur during
%   the |line_break| pass and in |hpack|.
%   Fixed: MLTeX will now access the character information for the
%   first existing character in the font (normally \char0) instead.
%   This means that the character/box dimensions will be incorrect!
%
% ...and in versions before 2.2 (done by Michael J. Ferguson):
% * Errors during shipout to dvi file if \*leaders box contains
%   substituted characters
% * Incorrect spacing of |hpack| or |rebox| if two fonts with
%   different character metrics are used, e.g. use ArabTeX with
%   mixed arabic/latin text.  ArabTeX contains a fix on the macro
%   level, but it doesn't really fix this bug!
%
% Restrictions:
% * Do not change character substitutions!  Because the current
%   character substitutions are only read while creating a character
%   node in the horizontal list, but are _not_ saved in this node,
%   the same substitutions have to be valid during |shipout| of
%   this character node to the dvi file.
%   (Bugs if \charsubdefs are changed: 1) during |hpack| or
%   |shipout| missing character warnings are reported,
%   2) \unhbox will change the width of a box)
% * If an accentee in a \charsubdef has height <= 0 it will
%   overlap the base character (for \c c), a real "\underaccent"
%   primitive is still missing.
% * Accent placement routine is taken from TeX's \accent primitive,
%   adjustments of accent characters (cf. `german.sty' lowers them)
%   are not possible.
%
%
%============================================================
% ChangeLog:
%
% Dec 17, 1995 (br) -- MLTeX v2.2
% -- add flag |mltex_p| and |mltex_enabled_p|,
%    add code to dump and undump the flag and a magic constant
%    simplifying the implementation of a command line option `-mltex'
% -- print additional banner line "MLTeX v2.2 enabled" after the fmt
%    file is loaded; original banner line is left unmodified
%
% Dec 07, 1995 (br) -- MLTeX v2.2 (betatest)
% -- |effective_char| can return non-existing characters.
%    This is not valid after character substitutions are used and
%    then are disabled because a node for a non-existing character
%    can remain in a horizontal list.  (TeX assumes that all char
%    nodes are valid, no checks are done after the character node
%    is created.)  Introduced third argument for |effective_char|,
%    introduced function |effective_char_info| as shortcut.
% -- Do not try to substitute characters in math mode
%    (for compatibility with MLTeX v2.01)
% -- removed old change of [32] m.638 l.12638; with the changed dvi
%     character substitution routines |show_box| can be called in
%    |ship_out| before shipping the box to the dvi file---without
%    getting incompatible with TRIP
%
% Dec 05, 1995 (br) -- MLTeX v2.2 (alphatest)
% -- typo in test version: |..._slant| are |real|, not |scaled|
%    (Thanks, em!)
% -- bug while loading fonts which contains less than 256 characters
%    and a character substitution was defined for an existing
%    character but a non-existing base character; the analysis of
%    bug showed another one w.r.t. a non-existing boundary character
%    (fix: define macro |orig_char_info| which doesn't use
%    |effective_char| and use it while loading a new font).
%
% Oct 28, 1995 (br) -- MLTeX v2.1+ (not released)
% -- Subsumed "\charsubdef" under |shorthand_def| to simplify
%    the merge of multiple change files.
% -- Moved character substitution from |eqtb| region 1 to region 4
%    which will allow \subcharcode<charnum>=<integer> -- a TeX
%    construct analogous to \catcode et.al.  (This change is already
%    included in a prototype of MLTeX version 3 which will use
%    different data structures to allow \charsubdef changes on the fly
%    without the anomalies in version 2.)
% -- When substituting a character, test if c>font_ec and additionally
%    if c<font_bc.
% -- Removed the changes in part 2 ("The character set"), module 24 to
%    allow non-ASCII characters in the input files.  These changes
%    should be included in the implementation specific change files!
% -- addded \charsubdefmin parameter (which can not be accessed by
%    the user!)
% -- there was a bug using a \*leaders box with charsubdef'ed
%    characters as a \*leaders box; it will try to access characters
%    in the |null_font| (probably resulting in access violations)!
%    (Fix: additional test for |if f<>null_font ...| in [32] m.620)
%    Better fix: don't construct a box construction like \accent,
%    write the dvi code directly; this will also simplify other
%    changes for TeX--XeT or e-TeX which need a |prevp| pointer.
%
% Sep 9 + Sep 16, 1992 (br) -- MLTeX v2.1+ (test, not released)
% -- initialize char_sub_def_max with -1 (instead of 0)
% -- changed effective_char to blend with the rest of TeX.web
%    parameter `c' is an `eight_bit'
% -- removed change of \S708 because it is silly to substitute
%    (character parts of) a large math delimiter character
% -- added proposals from E. Mattes
% -- deleted `font_chs'  (unnecessary and inconsistent use)
%    replaced chs_accent, chs_char, chs_replace
% -- make changes as short as possible
% -- make sure that xchr[@'177] is initialized
%
%============================================================
% Comments from `char_sub.ch', the former MLTeX change file:
%
% Oct 29, 1993 (MJF)
% In hlist_out change, prevent nullfont from being sent out to .dvi
% file (proposed by B.Raichle).
%   ........ version 2.01
%
% Feb 3, 1992 (MJF)
% effective_char ... modified to include explicit font information. 
% This change ensures that the effective_char returns the character
% in the font when that character actually exists ... and not the
% base character in the substitution list.
% char_info and char_info-end have been modified to overcome a Web
% limitation on two parameters and a Web2c parsing problem. 
% The result is a tex.p(as) file that will NOT compile WITHOUT the
% following change ... (ugh!):
% Globally replace XIIIX with ( and XTTTX with ) in the tex.p(as)
% file.
%
% bug corrections ... made May 8, 1991 ... errors found by B. Raichle
%
% Trip Switch added by PCTeX ... May 1990 -- Section [*638] 
%
% \tracingcharsubdef added June 1990---to aid in debugging---non zero
% values will leave a record of a charsub definition in the log file.
%
%
% Charsub modification of TeX 3.+ to allow for the hyphenation of 
% words with current (May 1990) fonts and explicitly built accents.
%
%============================================================

@x [1] m.2 l.187 - MLTeX: change banner line
November 1984].
@y
November 1984].

ML\TeX{} will add new primitives changing the behaviour of \TeX.  The
|banner| string has to be changed.  We do not change the |banner|
string, but will output an additional line to make clear that this is
a modified \TeX{} version.

@z


%---------------------------------------

@x [15] m.209 l.4165 - MLTeX: \charsubdef primitive
@d shorthand_def=95 {code definition ( \.{\\chardef}, \.{\\countdef}, etc.~)}
@y
@d shorthand_def=95 {code definition ( \.{\\chardef}, \.{\\countdef}, etc.~)}
  {or \.{\\charsubdef}}
@z


%---------------------------------------

@x [17] m.220 l.4448 - MLTeX: char_sub_code_base
paragraph shape.
@y
paragraph shape.
Additionally region~4 contains the table with ML\TeX's character
substitution definitions.
@z

@x [17] m.230 l.4731 - MLTeX: char_sub_code_base
@d int_base=math_code_base+256 {beginning of region 5}
@y
@d char_sub_code_base=math_code_base+256 {table of character substitutions}
@d int_base=char_sub_code_base+256 {beginning of region 5}
@z

@x [17] m.230 l.4752 - MLTeX: char_sub_code_base
  {Note: |math_code(c)| is the true math code plus |min_halfword|}
@y
  {Note: |math_code(c)| is the true math code plus |min_halfword|}
@d char_sub_code(#)==equiv(char_sub_code_base+#)
  {Note: |char_sub_code(c)| is the true substitution info plus |min_halfword|}
@z


%---------------------------------------

@x [17] m.236 l.4954 - MLTeX: \charsubdefmax and \tracingcharsubdef
@d int_pars=55 {total number of integer parameters}
@y
@d tex_int_pars=55 {total number of \TeX's integer parameters}
@#
@d web2c_int_base=tex_int_pars {base for web2c's integer parameters}
@d char_sub_def_min_code=web2c_int_base {smallest value in the charsubdef list}
@d char_sub_def_max_code=web2c_int_base+1 {largest value in the charsubdef list}
@d tracing_char_sub_def_code=web2c_int_base+2 {traces changes to a charsubdef def}
@d web2c_int_pars=web2c_int_base+3 {total number of web2c's integer parameters}
@#
@d int_pars=web2c_int_pars {total number of integer parameters}
@z

@x [17] m.236 l.5016
@d error_context_lines==int_par(error_context_lines_code)
@y
@d error_context_lines==int_par(error_context_lines_code)
@d char_sub_def_min==int_par(char_sub_def_min_code)
@d char_sub_def_max==int_par(char_sub_def_max_code)
@d tracing_char_sub_def==int_par(tracing_char_sub_def_code)
@z

@x [17] m.237 l.5080
error_context_lines_code:print_esc("errorcontextlines");
@y
error_context_lines_code:print_esc("errorcontextlines");
char_sub_def_min_code:print_esc("charsubdefmin");
char_sub_def_max_code:print_esc("charsubdefmax");
tracing_char_sub_def_code:print_esc("tracingcharsubdef");
@z

@x [17] m.238 l.5200
@!@:error_context_lines_}{\.{\\errorcontextlines} primitive@>
@y
@!@:error_context_lines_}{\.{\\errorcontextlines} primitive@>
if mltex_p then
  begin mltex_enabled_p:=true;  {enable character substitution}
  if false then {remove the if-clause to enable \.{\\charsubdefmin}}
  primitive("charsubdefmin",assign_int,int_base+char_sub_def_min_code);@/
@!@:char_sub_def_min_}{\.{\\charsubdefmin} primitive@>
  primitive("charsubdefmax",assign_int,int_base+char_sub_def_max_code);@/
@!@:char_sub_def_max_}{\.{\\charsubdefmax} primitive@>
  primitive("tracingcharsubdef",assign_int,int_base+tracing_char_sub_def_code);@/
@!@:tracing_char_sub_def_}{\.{\\tracingcharsubdef} primitive@>
  end;
@z

@x [17] m.240 l.5213
for k:=int_base to del_code_base-1 do eqtb[k].int:=0;
@y
for k:=int_base to del_code_base-1 do eqtb[k].int:=0;
char_sub_def_min:=256; char_sub_def_max:=-1;
{allow \.{\\charsubdef} for char 0}@/
{|tracing_char_sub_def:=0| is already done}@/
@z


%---------------------------------------

@x [29] m.534 l.10293 - MLTeX: add. MLTeX banner after loading fmt file
@<Print the banner line, including the date and time@>;
@y
@<Print the banner line, including the date and time@>;
if mltex_enabled_p then
  begin wlog_cr; wlog('MLTeX v2.2 enabled');
  end;
@z


%---------------------------------------

@x [30] m.554 l.10795 - MLTeX: |effective_char| in |char_info|
as fast as possible under the circumstances.
@^inner loop@>

@d char_info_end(#)==#].qqqq
@d char_info(#)==font_info[char_base[#]+char_info_end
@y
as fast as possible under the circumstances.

ML\TeX{} will assume that a character |c| exists iff either exists in
the current font or a character substitution definition for this
character was defined using \.{\\charsubdef}.  To avoid the
distinction between these two cases, ML\TeX{} introduces the notion
``effective character'' of an input character |c|.  If |c| exists in
the current font, the effective character of |c| is the character |c|
itself.  If it doesn't exist but a character substitution is defined,
the effective character of |c| is the base character defined in the
character substitution.  If there is an effective character for a
non-existing character |c|, the ``virtual character'' |c| will get
appended to the horizontal lists.

The effective character is used within |char_info| to access
appropriate character descriptions in the font.  For example, when
calculating the width of a box, ML\TeX{} will use the metrics of the
effective characters.  For the case of a substitution, ML\TeX{} uses
the metrics of the base character, ignoring the metrics of the accent
character.

If character substitutions are changed, it will be possible that a
character |c| neither exists in a font nor there is a valid character
substitution for |c|.  To handle these cases |effective_char| should
be called with its first argument set to |true| to ensure that it
will still return an existing character in the font.  If neither |c|
nor the substituted base character in the current character
substitution exists, |effective_char| will output a warning and
return the character |font_bc[f]| (which is incorrect, but can not be
changed within the current framework).

Sometimes character substitutions are unwanted, therefore the
original definition of |char_info| can be used using the macro
|orig_char_info|.  Operations in which character substitutions should
be avoided are, for example, loading a new font and checking the font
metric information in this font, and character accesses in math mode.

(Because of retrictions in \.{TANGLE}'s macro capabilities you have
to replace \.{XLPAREN} resp.\ \.{xlparen} with an opening brace and
\.{XRPAREN} resp.\ \.{xrparen} with a closing brace after tangling
\TeX!)
@^inner loop@>

@d char_list_exists(#)==(char_sub_code(#)>hi(0))
@d char_list_accent(#)==(ho(char_sub_code(#)) div 256)
@d char_list_char(#)==(ho(char_sub_code(#)) mod 256)
@#
@d char_info_end(#)== #@=)@>].qqqq
@d char_info(#)==
  font_info[char_base[#]+effective_char @=(@> true,#,char_info_end
@#
@d orig_char_info_end(#)==#].qqqq
@d orig_char_info(#)==font_info[char_base[#]+orig_char_info_end
@#
@z

@x [30] m.560 l.10876 - MLTeX: |effective_char| in |char_info|
@p function read_font_info(@!u:pointer;@!nom,@!aire:str_number;
@y
@p @t\4@>@<Declare additional functions for ML\TeX@>@/

function read_font_info(@!u:pointer;@!nom,@!aire:str_number;
@z


% Bug report:  (Nov/Dec 95)
% When
%  1) character substitution \charsubdef x1=x2 x3 is active and
%  2) font Y with character x1 but the base char x3 doesn't exist
% the error message 
%    ! Font \test=test not loadable: Bad metric (TFM) file.
% will appear when loading Y.
%
% Bug analysis:
% !! |font_bc[f]| and |font_ec[f]| are set at the end of the loading
% !! process in module 576.  |effective_char| uses these two
% !! values, therefore never use |effective_char| or |char_info|
% !! until the end of |read_font_info|.
%
% There are three places in which |char_info| is called:
%  a) @<Check for charlist cycle@>
%     no box dimensions are read at this time, i.e.
%     width index is 0 for each character and
%     font_ec/bc have undefined values
%     => |effective_char| assumes that char x1 doesn't exist
%     ==> x1 gets replaced by the "effective" character x3
%         which doesn't exist!  ==> error message
%  b) @<Read ligature/kern program@>, macro |check_existence|
%     box dimensions are already read, i.e. width index<>0 and
%     ligature/kerning program contains "real" characters,
%     nonetheless font_bc/ec have undefined values!
%     => |effective_char| assumes that char x1 doesn't exist
%        in most cases (because font_bc/ec are normally zeroed)
%     ==> error message
%  c) @<Make final adjustments...@>
%     code decides if boundary character exists in the font,
%     it will be ok---normally, but font_bc/ec... :-(
%     => |effective_char| assumes that char x1 doesn't exist
%        in most cases (because font_bc/ec are normally zeroed)
%     ==> wrong _if_ effective character exits but not the
%         boundary character and vice versa
%     This is another bug, because the value of the flag
%     |font_false_bchar| depends on current substitution.
%
% The cause of the error message was the undefined values of
% |font_bc[f]| and |font_ec[f]|, and even if they will have
% defined values there would be error because of a) and, if a
% font contains a boundary character, c).
%
% Bug fix:
% Don't use |effective_char|, use the original definition of
% |char_info| named |orig_char_info|.
%

@x [30] m.570 l.11064 - MLTeX: fix for bug while loading font
  begin qw:=char_info(f)(d);
@y
  begin qw:=orig_char_info(f)(d);
@z
@x [30] m.573 l.11116 - MLTeX: fix for bug while loading font
  qw:=char_info(f)(#); {N.B.: not |qi(#)|}
@y
  qw:=orig_char_info(f)(#); {N.B.: not |qi(#)|}
@z
@x [30] m.576 l.11180 - MLTeX: fix for bug while loading font
  begin qw:=char_info(f)(bchar); {N.B.: not |qi(bchar)|}
@y
  begin qw:=orig_char_info(f)(bchar); {N.B.: not |qi(bchar)|}
@z


%---------------------------------------

@x [30] m.582 l.11276 - MLTeX: call |effective_char| in |new_character|
@p function new_character(@!f:internal_font_number;@!c:eight_bits):pointer;
label exit;
var p:pointer; {newly allocated node}
begin if font_bc[f]<=c then if font_ec[f]>=c then
  if char_exists(char_info(f)(qi(c))) then
@y

This allows a character node to be used if there is an equivalent
in the |char_sub_code| list.

@p function new_character(@!f:internal_font_number;@!c:eight_bits):pointer;
label exit;
var p:pointer; {newly allocated node}
@!ec:quarterword;  {effective character of |c|}
begin ec:=effective_char(false,f,qi(c));
if font_bc[f]<=qo(ec) then if font_ec[f]>=qo(ec) then
  if char_exists(orig_char_info(f)(ec)) then  {N.B.: not |char_info|}
@z


%---------------------------------------

@x [32] m.619 l.12294 - MLTeX: substitute character in |hlist_out|
procedure hlist_out; {output an |hlist_node| box}
label reswitch, move_past, fin_rule, next_p;
@y
procedure hlist_out; {output an |hlist_node| box}
label reswitch, move_past, fin_rule, next_p, continue, found;
@z

@x [32] m.620 l.12326 - MLTeX: replace virtual character in |hlist_out|
reaching a non-|char_node|. The program uses the fact that |set_char_0=0|.
@^inner loop@>
@y
reaching a non-|char_node|. The program uses the fact that |set_char_0=0|.

In ML\TeX{} this part looks for the existence of a substitution
definition for a character |c|, if |c| does not exist in the font,
and create appropriate \.{DVI} commands.  Former versions of ML\TeX{}
have spliced appropriate character, kern, and box nodes into the
horizontal list.
%
% 91/05/08 \charsubdefmax bug detected by Bernd Raichle
Because the user can change character substitions or
\.{\\charsubdefmax} on the fly, we have to test a again
for valid substitutions.
%
% 93/10/29 \leaders bug detected by Eberhard Mattes
%      ==> prevent nullfont from being sent to .dvi file (B.R.)
(Additional it is necessary to be careful---if leaders are used
the current hlist is normally traversed more than once!)
@^inner loop@>
@z

@x [32] m.620 l.12334 -- MLTeX: substitute character during |shipout|
  if c>=qi(128) then dvi_out(set1);
  dvi_out(qo(c));@/
  cur_h:=cur_h+char_width(f)(char_info(f)(c));
@y
  if font_ec[f]>=qo(c) then if font_bc[f]<=qo(c) then
    if char_exists(orig_char_info(f)(c)) then  {N.B.: not |char_info|}
      begin if c>=qi(128) then dvi_out(set1);
      dvi_out(qo(c));@/
      cur_h:=cur_h+char_width(f)(orig_char_info(f)(c));
      goto continue;
      end;
  if mltex_enabled_p then
    @<Output a substitution, |goto continue| if not possible@>;
continue:
@z


%---------------------------------------

% The MLTeX changes never dealt with the problems of character 
% substitutions in math mode.  With the new additions in v2.2,
% non-existing characters between |font_bc[f]| and |font_ec[f]|
% can be substituted => we have to avoid this in math mode
% (for compatibility reasons and to avoid other problems).

@x [35] m.708 l.13903 -- MLTeX: avoid substitution in |var_delimiter|
if (qo(y)>=font_bc[g])and(qo(y)<=font_ec[g]) then
  begin continue: q:=char_info(g)(y);
@y
if (qo(y)>=font_bc[g])and(qo(y)<=font_ec[g]) then
  begin continue: q:=orig_char_info(g)(y);
@z

@x [36] m.722 l.14172 -- MLTeX: avoid substitution in |fetch|
    cur_i:=char_info(cur_f)(cur_c)
@y
    cur_i:=orig_char_info(cur_f)(cur_c)
@z

@x [36] m.740 l.14486 -- MLTeX: avoid substitution in |make_math_accent|
  i:=char_info(f)(y);
@y
  i:=orig_char_info(f)(y);
@z

@x [36] m.749 l.14638 -- MLTeX: avoid substitution in |make_op|
    begin c:=rem_byte(cur_i); i:=char_info(cur_f)(c);
@y
    begin c:=rem_byte(cur_i); i:=orig_char_info(cur_f)(c);
@z

%---------------------------------------

%%
%% We can rewrite the original code after "main_loop_move+2" upto the
%% "tail_append(lig_stack)" in module 1036 as
%%
%
% main_loop_move+2:
% if font_bc[main_f]<=cur_chr then
%  if cur_chr<=font_ec[main_f] then
%    begin  main_i:=char_info(main_f)(cur_l);
%    if char_exists(main_i) goto main_loop_move+3;
%    end;
% char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
% main_loop_move+3:
% tail_append(lig_stack) {|main_loop_lookahead| is next}
%
%%
%% We can use the rewritten code above to include additional MLTeX
%% specific parts in the future.  Additionally it can be used when
%% optimizing |main_control| to minimize the call of the function
%% |effective_char|.
%%
%
%@x [46] m.1030 l.19977 - MLTeX: substitution in |main_control|
%  main_loop_move,main_loop_move+1,main_loop_move+2,main_loop_move_lig,
%@y
%  main_loop_move,main_loop_move+1,main_loop_move+2,main_loop_move+3,
%  main_loop_move_lig,
%@z

@x [46] m.1036 l.20138 - MLTeX: substitution in |main_control|
main_loop_move+2:if(cur_chr<font_bc[main_f])or(cur_chr>font_ec[main_f]) then
@y
main_loop_move+2:
if(qo(effective_char(false,main_f,qi(cur_chr)))>font_ec[main_f])or
  (qo(effective_char(false,main_f,qi(cur_chr)))<font_bc[main_f]) then
@z

@x [46] m.1036 l.20141 - MLTeX: substitution in |main_control|
main_i:=char_info(main_f)(cur_l);
@y
main_i:=effective_char_info(main_f,cur_l);
@z


%---------------------------------------

@x [49] m.1222 l.22794 - MLTeX: \charsubdef primitive
@d toks_def_code=6 {|shorthand_def| for \.{\\toksdef}}
@y
@d toks_def_code=6 {|shorthand_def| for \.{\\toksdef}}
@d char_sub_def_code=7 {|shorthand_def| for \.{\\charsubdef}}
@z

@x [49] m.1222 l.22810 - MLTeX: \charsubdef primitive
@!@:toks_def_}{\.{\\toksdef} primitive@>
@y
@!@:toks_def_}{\.{\\toksdef} primitive@>
if mltex_p then
  begin
  primitive("charsubdef",shorthand_def,char_sub_def_code);@/
@!@:char_sub_def_}{\.{\\charsubdef} primitive@>
  end;
@z

@x [49] m.1222 l.22820 - MLTeX: \charsubdef primitive
  othercases print_esc("toksdef")
@y
  char_sub_def_code: print_esc("charsubdef");
  othercases print_esc("toksdef")
@z

@x [49] m.1222 l.22833 - MLTeX: \charsubdef primitive
shorthand_def: begin n:=cur_chr; get_r_token; p:=cur_cs; define(p,relax,256);
@y
shorthand_def: if cur_chr=char_sub_def_code then
 begin scan_char_num; p:=char_sub_code_base+cur_val; scan_optional_equals;
  scan_char_num; n:=cur_val; {accent character in substitution}
  scan_char_num;
  if (tracing_char_sub_def>0) then
    begin begin_diagnostic; print_nl("New character substitution: ");
    print_ASCII(p-char_sub_code_base); print(" = ");
    print_ASCII(n); print_char(" ");
    print_ASCII(cur_val); end_diagnostic(false);
    end;
  n:=n*256+cur_val;
  define(p,data,hi(n));
  if (p-char_sub_code_base)<char_sub_def_min then
    word_define(int_base+char_sub_def_min_code,p-char_sub_code_base);
  if (p-char_sub_code_base)>char_sub_def_max then
    word_define(int_base+char_sub_def_max_code,p-char_sub_code_base);
 end
else begin n:=cur_chr; get_r_token; p:=cur_cs; define(p,relax,256);
@z


%---------------------------------------

@x [50] m.1302 l.23694 - MLTeX: dump |mltex_p| to fmt file
@<Dump constants for consistency check@>;
@y
@<Dump constants for consistency check@>;
@<Dump ML\TeX-specific data@>;
@z

@x [50] m.1303 l.23694 - MLTeX: undump |mltex_enabled_p| from fmt file
begin @<Undump constants for consistency check@>;
@y
begin @<Undump constants for consistency check@>;
@<Undump ML\TeX-specific data@>;
@z

@x [51] m.1337 l.24371 - MLTeX: add. MLTeX banner after loading fmt file
fix_date_and_time;@/
@y
if mltex_enabled_p then
  begin wterm_ln('MLTeX v2.2 enabled');
  end;
fix_date_and_time;@/
@z


%---------------------------------------

@x [54] m.1376 l.24903 - MLTeX: additional routines
@* \[54] System-dependent changes.
@y
@* \[54/ML\TeX] System-dependent changes for ML\TeX.

The boolean variable |mltex_p| is set by \MiKTeX\ according to the given
command line option (or an entry in the configuration file) before any
\TeX{} function is called.

@<Global...@> =
@!mltex_p: boolean;

@ The boolean variable |mltex_enabled_p| is used to enable ML\TeX's
character substitution.  It is initialised to |false|.  When loading
a \.{FMT} it is set to the value of the boolean |mltex_p| saved in
the \.{FMT} file.  Additionally it is set to the value of |mltex_p|
in Ini\TeX.

@<Glob...@>=
@!mltex_enabled_p:boolean;  {enable character substitution}


@ @<Declare \MiKTeX\ functions@>=
function miktex_mltex_p : boolean; forward;

@ @<Set init...@>=
mltex_p:=miktex_mltex_p;
mltex_enabled_p:=false;


@ The function |effective_char| computes the effective character with
respect to font information.  The effective character is either the
base character part of a character substitution definition, if the
character does not exist in the font or the character itself.

Inside |effective_char| we can not use |char_info| because the macro
|char_info| uses |effective_char| calling this function a second time
with the same arguments.

If neither the character |c| exists in font |f| nor a character
substitution for |c| was defined, you can not use the function value
as a character offset in |char_info| because it will access an
undefined or invalid |font_info| entry!  Therefore inside |char_info|
and in other places, |effective_char|'s boolean parameter |err_p| is
set to |true| to issue a warning and return the incorrect
replacement, but always existing character |font_bc[f]|.
@^inner loop@>

@<Declare additional functions for ML\TeX@>=
function effective_char(@!err_p:boolean;
                        @!f:internal_font_number;@!c:quarterword):integer;
label found;
var base_c: integer; {or |eightbits|: replacement base character}
@!result: integer; {or |quarterword|}
begin result:=c;  {return |c| unless it does not exist in the font}
if not mltex_enabled_p then goto found;
if font_ec[f]>=qo(c) then if font_bc[f]<=qo(c) then
  if char_exists(orig_char_info(f)(c)) then  {N.B.: not |char_info|(f)(c)}
    goto found;
if qo(c)>=char_sub_def_min then if qo(c)<=char_sub_def_max then
  if char_list_exists(qo(c)) then
    begin base_c:=char_list_char(qo(c));
    result:=qi(base_c);  {return |base_c|}
    if not err_p then goto found;
    if font_ec[f]>=base_c then if font_bc[f]<=base_c then
      if char_exists(orig_char_info(f)(qi(base_c))) then goto found;
    end;
if err_p then  {print error and return existing character?}
  begin begin_diagnostic;
  print_nl("Missing character: There is no "); print("substitution for ");
@.Missing character@>
  print_ASCII(qo(c)); print(" in font ");
  slow_print(font_name[f]); print_char("!"); end_diagnostic(false);
  result:=qi(font_bc[f]); {N.B.: not non-existing character |c|!}
  end;
found: effective_char:=result;
end;


@ The function |effective_char_info| is equivalent to |char_info|,
except it will return |null_character| if neither the character |c|
exists in font |f| nor is there a substitution definition for |c|.
(For these cases |char_info| using |effective_char| will access an
undefined or invalid |font_info| entry.  See the documentation of
|effective_char| for more information.)
@^inner loop@>

@<Declare additional functions for ML\TeX@>=
function effective_char_info(@!f:internal_font_number;
                             @!c:quarterword):four_quarters;
label exit;
var ci:four_quarters; {character information bytes for |c|}
@!base_c:integer; {or |eightbits|: replacement base character}
begin if not mltex_enabled_p then
  begin effective_char_info:=orig_char_info(f)(c); return;
  end;
if font_ec[f]>=qo(c) then if font_bc[f]<=qo(c) then
  begin ci:=orig_char_info(f)(c);  {N.B.: not |char_info|(f)(c)}
  if char_exists(ci) then
    begin effective_char_info:=ci; return;
    end;
  end;
if qo(c)>=char_sub_def_min then if qo(c)<=char_sub_def_max then
  if char_list_exists(qo(c)) then
    begin {|effective_char_info:=char_info(f)(qi(char_list_char(qo(c))));|}
    base_c:=char_list_char(qo(c));
    if font_ec[f]>=base_c then if font_bc[f]<=base_c then
      begin ci:=orig_char_info(f)(qi(base_c));  {N.B.: not |char_info|(f)(c)}
      if char_exists(ci) then
        begin effective_char_info:=ci; return;
        end;
      end;
    end;
effective_char_info:=null_character;
exit:end;


@ This code is called for a virtual character |c| in |hlist_out|
during |ship_out|.  It tries to built a character substitution
construct for |c| generating appropriate \.{DVI} code using the
character substitution definition for this character.  If a valid
character substitution exists \.{DVI} code is created as if
|make_accent| was used.  In all other cases the status of the
substituion for this character has been changed between the creation
of the character node in the hlist and the output of the page---the
created \.{DVI} code will be correct but the visual result will be
undefined.

Former ML\TeX\ versions have replaced the character node by a
sequence of character, box, and accent kern nodes splicing them into
the original horizontal list.  This version does not do this to avoid
a)~a memory overflow at this processing stage, b)~additional code to
add a pointer to the previous node needed for the replacement, and
c)~to avoid wrong code resulting in anomalies because of the use
within a \.{\\leaders} box.

@<Output a substitution, |goto continue| if not possible@>=
  begin
  @<Get substitution information, check it, goto |found|
  if all is ok, otherwise goto |continue|@>;
found: @<Print character substition tracing log@>;
  @<Rebuild character using substitution information@>;
  end


@ The global variables for the code to substitute a virtual character
can be declared as local.  Nonetheless we declare them as global to
avoid stack overflows because |hlist_out| can be called recursivly.
 
@<Glob...@>=
@!accent_c,@!base_c,@!replace_c:integer;
@!ia_c,@!ib_c:four_quarters; {accent and base character information}
@!base_slant,@!accent_slant:real; {amount of slant}
@!base_x_height:scaled; {accent is designed for characters of this height}
@!base_width,@!base_height:scaled; {height and width for base character}
@!accent_width,@!accent_height:scaled; {height and width for accent}
@!delta:scaled; {amount of right shift}


@ Get the character substitution information in |char_sub_code| for
the character |c|.  The current code checks that the substition
exists and is valid and all substitution characters exist in the
font, so we can {\it not\/} substitute a character used in a
substitution.  This simplifies the code because we have not to check
for cycles in all character substitution definitions.

@<Get substitution information, check it...@>=
  if qo(c)>=char_sub_def_min then if qo(c)<=char_sub_def_max then
    if char_list_exists(qo(c)) then
      begin  base_c:=char_list_char(qo(c));
      accent_c:=char_list_accent(qo(c));
      if (font_ec[f]>=base_c) then if (font_bc[f]<=base_c) then
        if (font_ec[f]>=accent_c) then if (font_bc[f]<=accent_c) then
          begin ia_c:=char_info(f)(qi(accent_c));
          ib_c:=char_info(f)(qi(base_c));
          if char_exists(ib_c) then
            if char_exists(ia_c) then goto found;
          end;
      begin_diagnostic;
      print_nl("Missing character: Incomplete substitution ");
@.Missing character@>
      print_ASCII(qo(c)); print(" = "); print_ASCII(accent_c);
      print(" "); print_ASCII(base_c); print(" in font ");
      slow_print(font_name[f]); print_char("!"); end_diagnostic(false);
      goto continue;
      end;
  begin_diagnostic;
  print_nl("Missing character: There is no "); print("substitution for ");
@.Missing character@>
  print_ASCII(qo(c)); print(" in font ");
  slow_print(font_name[f]); print_char("!"); end_diagnostic(false);
  goto continue


@ For |tracinglostchars>99| the substitution is shown in the log file.

@<Print character substition tracing log@>=
 if tracing_lost_chars>99 then
   begin begin_diagnostic;
   print_nl("Using character substitution: ");
   print_ASCII(qo(c)); print(" = ");
   print_ASCII(accent_c); print(" "); print_ASCII(base_c);
   print(" in font "); slow_print(font_name[f]); print_char(".");
   end_diagnostic(false);
   end


@ This outputs the accent and the base character given in the
substitution.  It uses code virtually identical to the |make_accent|
procedure, but without the node creation steps.
 
Additionally if the accent character has to be shifted vertically it
does {\it not\/} create the same code.  The original routine in
|make_accent| and former versions of ML\TeX{} creates a box node
resulting in |push| and |pop| operations, whereas this code simply
produces vertical positioning operations.  This can influence the
pixel rounding algorithm in some \.{DVI} drivers---and therefore will
probably be changed in one of the next ML\TeX{} versions.

@<Rebuild character using substitution information@>=
  base_x_height:=x_height(f);
  base_slant:=slant(f)/float_constant(65536);
@^real division@>
  accent_slant:=base_slant; {slant of accent character font}
  base_width:=char_width(f)(ib_c);
  base_height:=char_height(f)(height_depth(ib_c));
  accent_width:=char_width(f)(ia_c);
  accent_height:=char_height(f)(height_depth(ia_c));@/
  {compute necessary horizontal shift (don't forget slant)}
  delta:=round((base_width-accent_width)/float_constant(2)+
            base_height*base_slant-base_x_height*accent_slant);
@^real multiplication@>
@^real addition@>
  dvi_h:=cur_h;  {update |dvi_h|, similar to the last statement in module 620}@/
  {1. For centering/horizontal shifting insert a kern node.}
  cur_h:=cur_h+delta; synch_h;@/
  {2. Then insert the accent character possibly shifted up or down.}
  if ((base_height<>base_x_height) and (accent_height>0)) then
    begin {the accent must be shifted up or down}
    cur_v:=base_line+(base_x_height-base_height); synch_v;
    if accent_c>=128 then dvi_out(set1);
    dvi_out(accent_c);
    cur_v:=base_line;
    end
  else begin synch_v;
    if accent_c>=128 then dvi_out(set1);
    dvi_out(accent_c);
    end;
  cur_h:=cur_h+accent_width; dvi_h:=cur_h;@/
  {3. For centering/horizontal shifting insert another kern node.}
  cur_h:=cur_h+(-accent_width-delta);@/
  {4. Output the base character.}
  synch_h; synch_v;
  if base_c>=128 then dvi_out(set1);
  dvi_out(base_c);@/
  cur_h:=cur_h+base_width;
  dvi_h:=cur_h {update of |dvi_h| is unnecessary, will be set in module 620}

@ Dumping ML\TeX-related material.  This is just the flag in the
format that tells us whether ML\TeX{} is enabled.

@<Dump ML\TeX-specific data@>=
dump_int(@"4D4C5458);  {ML\TeX's magic constant: "MLTX"}
if mltex_p then dump_int(1)
else dump_int(0);

@ Undump ML\TeX-related material, which is just a flag in the format
that tells us whether ML\TeX{} is enabled.

@<Undump ML\TeX-specific data@>=
undump_int(x);   {check magic constant of ML\TeX}
if x<>@"4D4C5458 then goto bad_fmt;
undump_int(x);   {undump |mltex_p| flag into |mltex_enabled_p|}
if x=1 then mltex_enabled_p:=true
else if x<>0 then goto bad_fmt;

 
@* \[54] System-dependent changes.
@z
%
%%% --- end of MLTEX.CH ---
