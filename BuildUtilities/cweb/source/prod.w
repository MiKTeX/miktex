% This file is part of CWEB.
% This program by Silvio Levy and Donald E. Knuth
% is based on a program by Knuth.
% It is distributed WITHOUT ANY WARRANTY, express or implied.
% Version 4.7 --- February 2022
%
@ Here is a table of all the productions.  Each production that
combines two or more consecutive scraps implicitly inserts a {\tt \$}
where necessary, that is, between scraps whose abutting boundaries
have different |mathness|.  In this way we never get double {\tt\$\$}.

% The following kludge is needed because \newcount, \newdimen, and \+
% are "\outer" control sequences that cannot be used in skipped text!
\fi \newcount\prodno \newdimen\midcol \let\+\relax \ifon

\def\v{\char'174} 
\mathchardef\RA="3221 % right arrow
\mathchardef\BA="3224 % double arrow

A translation is provided when the resulting scrap is not merely a
juxtaposition of the scraps it comes from.  An asterisk$^*$ next to a scrap
means that its first identifier gets an underlined entry in the index,
via the function |make_underlined|.  Two asterisks$^{**}$ means that both
|make_underlined| and |make_reserved| are called; that is, the
identifier's ilk becomes |raw_int|.  A dagger \dag\ before the
production number refers to the notes at the end of this section,
which deal with various exceptional cases.

We use \\{in}, \\{out}, \\{back}, \\{bsp}, and \\{din} as shorthands for
|indent|, |outdent|, |backup|, |break_space|, and |dindent|, respectively.

\begingroup \lineskip=4pt
\def\alt #1 #2
{$\displaystyle\Bigl\{\!\matrix{\strut\hbox{#1}\cr
   \strut\hbox{#2}\cr}\!\Bigr\}$ }
\def\altt #1 #2 #3
{$\displaystyle\Biggl\{\!\matrix{\strut\hbox{#1}\cr\hbox{#2}\cr
   \strut\hbox{#3}\cr}\!\Biggr\}$ }
\def\malt #1 #2
{$\displaystyle\!\matrix{\strut\hbox{#1}\hfill\cr\strut\hbox{#2}\hfill\cr}$}
\def\maltt #1 #2 #3
{$\displaystyle\!\matrix{\strut\hbox{#1}\hfill\cr\hbox{#2}\hfill\cr
   \strut\hbox{#3}\hfill\cr}$}
\yskip
\prodno=0 \midcol=2.5in
\def\theprodno{\number\prodno \global\advance\prodno by1\enspace}
\def\dagit{\dag\theprodno}
\def\+#1&#2&#3&#4\cr{\def\next{#1}%
 \line{\hbox to 2em{\hss
  \ifx\next\empty\theprodno\else\next\fi}\strut
  \ignorespaces#2\hfil\hbox to\midcol{$\RA$
  \ignorespaces#3\hfil}\quad \hbox to1.45in{\ignorespaces#4\hfil}}}
\+\relax & LHS & RHS \hfill Translation & Example\cr
\yskip
\+& \altt\\{any} {\\{any} \\{any}} {\\{any} \\{any} \\{any}}
|insert| & \altt\\{any} {\\{any} \\{any}} {\\{any} \\{any} \\{any}}
& stmt; \4\4 \C{comment}\cr
\+& |exp| \altt|lbrace| |int_like| |decl|
    & |fn_decl| \altt|lbrace| |int_like| |decl| \hfill $F=E^*\,\\{din}$
    & \malt {\\{main}(\,) $\{$}
            {$\\{main}(\\{ac},\\{av}){}$ \&{int} \\{ac};} \cr
\+& |exp| |unop| & |exp| & $x\PP$ \cr
\+& |exp| \alt |binop| |ubinop| |exp| & |exp| & \malt {$x/y$} {$x+y$} \cr
\+& |exp| |comma| |exp| & |exp| \hfill $E_1C\,|opt|9\,E_2$ & $f(x,y)$ \cr
\+& |exp| \alt {|lpar| |rpar|} |cast| |colon|
    & |exp| \alt {|lpar| |rpar|} |cast| |base|
    & \malt {\&C(\,) :} {\&C(\&{int} $i$) :} \cr
\+& |exp| |semi| & |stmt| & $x=0;$ \cr
\+& |exp| |colon| & |tag| \hfill $E^*C$ & \\{found}: \cr
\+& |exp| |rbrace| & |stmt| |rbrace| & end of \&{enum} list\cr
\+& |exp| \alt {|lpar| |rpar|} |cast| \alt|const_like| |case_like|
    & |exp| \alt {|lpar| |rpar|} |cast| \hfill
       \alt $R=R\.\ C$ $C_1=C_1\.\ C_2$
    & \malt {$f$(\,) \&{const}} {$f$(\&{int}) \&{throw}} \cr
\+& |exp| \alt |exp| |cast| & |exp| & \\{time}(\,) \cr
\+& |lpar| \alt |exp| |ubinop| |rpar| & |exp| & \malt{($x$)} {$(*)$} \cr
\+& |lpar| |rpar| & |exp| \hfill $L\.{\\,}R$ & functions, declarations\cr
\+& |lpar| \altt |decl_head| |int_like| |cast| |rpar| & |cast| & |(char*)|\cr
\+& |lpar| \altt |decl_head| |int_like| |exp| |comma| & |lpar| \hfill
     $L$\,\altt $D$ $I$ $E$ \unskip $C$\,|opt|9 & |(int,|\cr
\+& |lpar| \alt |stmt| |decl| & |lpar| \hfill \alt {$LS\.\ $} {$LD\.\ $}
    & \malt {$(k=5;$} {(\&{int} $k=5;$} \cr
\+& |unop| \alt |exp| |int_like| & |exp|
    & \malt {$\R x$} {$\CM\&C$} \cr
\+& |ubinop| |cast| |rpar| & |cast| |rpar| \hfill
      $C=\.\{U\.\}C$ & |*|\&{CPtr}) \cr
\+& |ubinop| \alt|exp| |int_like| & \alt|exp| |int_like| \hfill
  \.\{$U$\.\}\alt$E$ $I$ & \malt {${*}x$} {${*}\&{CPtr}$} \cr
\+& |ubinop| |binop| & |binop| \hfill $|math_rel|\,U\.\{B\.\}\.\}$ & |*=|\cr
\+& |binop| |binop| & |binop| \hfill
                        $|math_rel|\,\.\{B_1\.\}\.\{B_2\.\}\.\}$ & |>>=|\cr
\+& |cast| \alt |lpar| |exp| & \alt |lpar| |exp| \hfill
  \alt $CL$ $C\.\ E$ & \malt {$(\&{double})(x+2)$} {(\&{double}) $x$} \cr
\+& |cast| |semi| & |exp| |semi| & |(int);|\cr
\+& |sizeof_like| |cast| & |exp| & |sizeof (double)|\cr
\+& |sizeof_like| |exp| & |exp| \hfill $S\.\ E$ & \&{sizeof} $x$\cr
\+& |int_like| \alt|int_like| |struct_like| &
        \alt|int_like| |struct_like| \hfill $I\.\ $\alt $I$ $S$
        \unskip & |extern char|\cr
\+& |int_like| |exp| \alt|raw_int| |struct_like| &
         |int_like| \alt|raw_int| |struct_like| & |extern "Ada" int|\cr
\+& |int_like| \altt|exp| |ubinop| |colon| &
   |decl_head| \altt|exp| |ubinop| |colon| \hfill $D=I$\.\ %
    & \maltt {\&{int} $x$} {\&{int} ${}{*}x$} {\&{unsigned} :} \cr
\+& |int_like| \alt |semi| |binop| & |decl_head| \alt |semi| |binop|
    & \malt {\&{int} $x$;} {\&{int} $f(\&{int}=4)$} \cr
\+& |public_like| |colon| & |tag| & |private:|\cr
\+& |public_like| & |int_like| & |private|\cr
\+& |colcol| \alt|exp| |int_like| & \alt|exp| |int_like| \hfill
     |qualifier| $C$\alt$E$ $I$ & \&C\DC$x$\cr
\+& |colcol| |colcol| & |colcol| & \&C\DC\&B\DC\cr
\+& |decl_head| |comma| & |decl_head| \hfill $DC\.\ $ & \&{int} $x,{}$ \cr
\+& |decl_head| |ubinop| & |decl_head| \hfill $D\.\{U\.\}$ & |int *|\cr
\+\dagit& |decl_head| |exp| & |decl_head| \hfill $DE^*$ & \&{int} $x$ \cr
\+& |decl_head| \alt|binop| |colon| |exp| \altt|comma| |semi| |rpar| &
     |decl_head| \altt|comma| |semi| |rpar| \hfill
     $D=D$\alt $B$ $C$ \unskip $E$
    & \malt {\&{int} $f(\&{int}\ x=2)$} {\&{int} $b$ : 1} \cr
\+& |decl_head| |cast| & |decl_head| & \&{int} $f$(\&{int})\cr
\+& |decl_head| \altt|int_like| |lbrace| |decl| & |fn_decl|
                   \altt|int_like| |lbrace| |decl| \hfill $F=D\,\\{din}$
    & \&{long} \\{time}(\,) $\{$\cr
\+& |decl_head| |semi| & |decl| & \&{int} $n$;\cr
\+& |decl| |decl| & |decl| \hfill $D_1\,|force|\,D_2$
    & \&{int} $n$; \&{double} $x$;\cr
\+& |decl| \alt|stmt| |function| & \alt|stmt| |function|
        \hfill $D\,|big_force|\,$\alt $S$ $F$ \unskip
    & \&{extern} $n$; \\{main}(\,) $\{\,\}$\cr
\+& |base| \alt |int_like| |exp| |comma| & |base| \hfill
     $B$\.\ \alt $I$ $E$ \unskip $C$\,|opt|9
    & \malt {: \&{public} \&A,} {: $i$(5),} \cr
\+& |base| \alt |int_like| |exp| |lbrace| & |lbrace| \hfill
     $B$\.\ \alt $I$ $E$ \unskip \.\ $L$ & \&D : \&{public} \&A $\{$\cr
\+& |struct_like| |lbrace| & |struct_head| \hfill $S\.\ L$ & |struct {|\cr
\+& |struct_like| \alt|exp| |int_like| |semi| & |decl_head| |semi|
     \hfill $S\.\ $\alt $E^{**}$ $I^{**}$ & \&{struct} \&{forward};\cr
\+& |struct_like| \alt|exp| |int_like| |lbrace| & |struct_head| \hfill
     $S\.\ $\alt $E^{**}$ $I^{**}$ \unskip $\.\ L$
    & \&{struct} \&{name\_info} $\{$\cr
\+& |struct_like| \alt|exp| |int_like| |colon| &
    |struct_like| \alt|exp| |int_like| |base| & |class| \&C :\cr
\+\dagit& |struct_like| \alt|exp| |int_like| & |int_like|
        \hfill $S\.\ $\alt$E$ $I$ \unskip & \&{struct} \&{name\_info} $z$;\cr
\+& |struct_head| \altt|decl| |stmt| |function| |rbrace| & |int_like|\hfill
         $S\,\\{in}\,|force|$\altt$D$ $S$ $F$ \unskip $\\{out}\,|force|\,R$
    & |struct {| declaration |}|\cr
\+& |struct_head| |rbrace| & |int_like|\hfill $S\.{\\,}R$
    & \&{class} \&C $\{\,\}$\cr
\+& |fn_decl| |decl| & |fn_decl| \hfill $F\,|force|\,D$
    & $f(z)$ \&{double} $z$; \cr
\+& |fn_decl| |stmt| & |function| \hfill $F\,|out|\,|out|\,|force|\,S$
    & \\{main}() {\dots}\cr
\+& |function| \altt|stmt| |decl| |function| & \altt |stmt| |decl| |function|
   \hfill $F\,|big_force|\,$\altt $S$ $D$ $F$ \unskip & outer block\cr
\+& |lbrace| |rbrace| & |stmt| \hfill $L\.{\\,}R$ & empty statement\cr
\advance\midcol35pt
\+& |lbrace| \altt|stmt| |decl| |function| |rbrace| & |stmt| \hfill
     $|force|\,L\,\\{in}\,|force|\,S\,
                |force|\,\\{back}\,R\,\\{out}\,|force|$ & compound statement\cr
\advance\midcol-20pt
\+& |lbrace| |exp| [|comma|] |rbrace| & |exp| & initializer\cr
\+& |if_like| |exp| & |if_clause| \hfill $I\.{\ }E$ & \&{if} ($z$)\cr
\+& |else_like| |colon| & |else_like| |base| & |try :|\cr
\+& |else_like| |lbrace| & |else_head| |lbrace| & |else {|\cr
\+& |else_like| |stmt| & |stmt| \hfill
       $|force|\,E\,\\{in}\,\\{bsp}\,S\,\\{out}\,|force|$
    & \&{else} $x=0;$\cr
\+& |else_head| \alt|stmt| |exp| & |stmt| \hfill
       $|force|\,E\,\\{bsp}\,|noop|\,|cancel|\,S\,\\{bsp}$
    & $\!\!$ \&{else} $\{x=0;\}$\cr
\+& |if_clause| |lbrace| & |if_head| |lbrace| & \&{if} ($x$) $\{$\cr
\+& |if_clause| |stmt| |else_like| |if_like| & |if_like| \hfill
       $|force|\,I\,\\{in}\,\\{bsp}\,S\,\\{out}\,|force|\,E\,\.\ I$
    & $\!\!$ \&{if} ($x$) $y$; \&{else} \&{if}\cr
\+& |if_clause| |stmt| |else_like| & |else_like| \hfill
       $|force|\,I\,\\{in}\,\\{bsp}\,S\,\\{out}\,|force|\,E$
    & $\!\!$ \&{if} ($x$) $y$; \&{else}\cr
\+& |if_clause| |stmt| & |else_like| |stmt| & $\!\!$ \&{if} ($x$) $y$;\cr
\+& |if_head| \alt|stmt| |exp| |else_like| |if_like| & |if_like| \hfill
       $|force|\,I\,\\{bsp}\,|noop|\,|cancel|\,S\,|force|\,E\,\.\ I$
    & $\!\!$ \&{if} ($x$) $\{\,y;\,\}$ \&{else} \&{if}\cr
\+& |if_head| \alt|stmt| |exp| |else_like| & |else_like| \hfill
       $|force|\,I\,\\{bsp}\,|noop|\,|cancel|\,S\,|force|\,E$
    & $\!\!$ \&{if} ($x$) $\{\,y;\,\}$ \&{else}\cr
\+& |if_head| \alt|stmt| |exp| & |else_head| \alt|stmt| |exp|
    & $\!\!$ \&{if} ($x$) ${}\{\,y\,\}{}$\cr
\advance\midcol20pt
\+& |do_like| |stmt| |else_like| |semi| & |stmt| \hfill
      $D\,\\{bsp}\,|noop|\,|cancel|\,S\,|cancel|\,|noop|\,\\{bsp}\,ES$%
      & \&{do} $f$($x$); \&{while} ($g$($x$));\cr
\advance\midcol-20pt
\+& |case_like| |semi| & |stmt| & |return;|\cr
\+& |case_like| |colon| & |tag| & |default:|\cr
\+& |case_like| |exp| & |exp| \hfill $C\.\ E$ & |return 0|\cr
\+& |catch_like| \alt|cast| |exp| & |fn_decl| \hfill
    $C$\alt $C$ $E$ \unskip \\{din} & |catch (...)|\cr
\+& |tag| |tag| & |tag| \hfill $T_1\,\\{bsp}\,T_2$ & |case 0: case 1:|\cr
\+& |tag| \altt|stmt| |decl| |function| & \altt|stmt| |decl| |function|
       \hfill $|force|\,\\{back}\,T\,\\{bsp}\,S$
    & $\!\!$ \&{case} 0: $z=0;$\cr
\+\dagit& |stmt| \altt|stmt| |decl| |function| &
   \altt|stmt| |decl| |function|
      \hfill $S\,$\altt$|force|\,S$ $|big_force|\,D$ $|big_force|\,F$ \unskip
    & $x=1;$ $y=2;$\cr
\+& |semi| & |stmt| \hfill \.\ $S$& empty statement\cr
\+\dagit& |lproc| \altt |if_like| |else_like| |define_like| & |lproc| &
         \maltt {\#\&{include}} \#\&{else} \#\&{define} \cr
\+& |lproc| |rproc| & |insert| & \#\&{endif} \cr
\+& |lproc| \alt {|exp| [|exp|]} |function| |rproc| & |insert| \hfill
    $I$\.\ \alt {$E{[\.{\ \\5}E]}$} {$F$} &
 \malt{\#\&{define} $a$\enspace 1} {\#\&{define} $a$\enspace$\{\,b;\,\}$} \cr
\+& |section_scrap| |semi| & |stmt|\hfill $MS$ |force|
   &$\langle\,$section name$\,\rangle$;\cr
\+& |section_scrap| & |exp| &$\langle\,$section name$\,\rangle$\cr
\+& |insert| \\{any} & \\{any} & \.{\v\#include\v}\cr
\+& |prelangle| & |binop| \hfill \.< & $<$ not in template\cr
\+& |prerangle| & |binop| \hfill \.> & $>$ not in template\cr
\+& |langle| |prerangle| & |cast| \hfill $L\.{\\,}P$ & $\langle\,\rangle$\cr
\+& |langle| \altt|decl_head| |int_like| |exp| |prerangle| & |cast| &
     $\langle\&{class}\,\&C\rangle$\cr
\+& |langle| \altt|decl_head| |int_like| |exp| |comma| & |langle| \hfill
     $L$\,\altt $D$ $I$ $E$ \unskip $C$\,|opt|9 & $\langle\&{class}\,\&C,$\cr
\+& |template_like| |exp| |prelangle| & |template_like| |exp| |langle| &
  \&{template} $a\langle100\rangle$\cr
\+& |template_like| \alt|exp| |raw_int| & \alt|exp| |raw_int| \hfill
    $T$\.\ \alt$E$ $R$ & \&C\DC\&{template} $a$(\,)\cr
\+& |template_like| & |raw_int| & \&{template}$\langle\&{class}\,\&T\rangle$\cr
\+& |new_like| |lpar| |exp| |rpar| & |new_like| & \&{new}(\\{nothrow})\cr
\+& |new_like| |cast| & |exp| \hfill $N\.\ C$ & |new(int*)|\cr
\+\dagit& |new_like| & |new_exp| & \&{new} \&C(\,)\cr
\+& |new_exp| \alt|int_like| |const_like| & |new_exp| \hfill
    $N$\.\ \alt $I$ $C$ & |new const int|\cr
\+& |new_exp| |struct_like| \alt |exp| |int_like| & |new_exp| \hfill
    $N\.\ S$\.\ \alt $E$ $I$ & \&{new} \&{struct} \&S\cr
\+& |new_exp| |raw_ubin| & |new_exp| \hfill $N\.\{R\.\}$ & |new int*[2]|\cr
\+& |new_exp| \alt |lpar| |exp| & |exp| \alt |lpar| |exp| \hfill
    $E=N$\,\alt {} {\.\ } & \malt {|operator[](int)|} {|new int(2)|} \cr
\+\dagit& |new_exp| & |exp| & |new int;|\cr
\+& |ftemplate| |prelangle| & |ftemplate| |langle| & |make_pair<int,int>|\cr
\+& |ftemplate| & |exp| & |make_pair(1,2)|\cr
\+& |for_like| |exp| & |else_like| \hfill $F\.\ E$ & |while (1)|\cr
\+& |raw_ubin| |const_like| & |raw_ubin| \hfill $RC$\.{\\\ }
     & $*$\&{const} $x$\cr
\+& |raw_ubin| & |ubinop| & $*$ $x$\cr
\+& |const_like| & |int_like| & \&{const} $x$\cr
\+& |raw_int| |prelangle| & |raw_int| |langle| & \&C$\langle$\cr
\+& |raw_int| |colcol| & |colcol| & \&C\DC\cr
\+& |raw_int| |cast| & |raw_int| & \&C$\langle\&{class}\ \&T\rangle$\cr
\+& |raw_int| |lpar| & |exp| |lpar| & \&{complex}$(x,y)$\cr
\+\dagit& |raw_int| & |int_like|   & \&{complex} $z$\cr
\+\dagit& |operator_like| \altt|binop| |unop| |ubinop| & |exp|
    \hfill $O$\.\{\altt $B$ $U$ $U$ \unskip \.\} & |operator+|\cr
\+& |operator_like| \alt|new_like| |delete_like| & |exp| \hfill
    $O$\.\ \alt $N$ $S$ & |operator delete|\cr
\+& |operator_like| |comma| & |exp| & \&{operator},\cr
\+\dagit& |operator_like| & |new_exp| & |operator char*|\cr
\advance\midcol-8pt
\+& |typedef_like| \alt|int_like| |cast| \alt|comma| |semi| &
    |typedef_like| |exp| \alt|comma| |semi| & \&{typedef} \&{int} \&I,\cr
\advance\midcol+8pt
\+& |typedef_like| |int_like| & |typedef_like| \hfill $T\.\ I$ &
    \&{typedef} \&{char}\cr
\+\dagit& |typedef_like| |exp| & |typedef_like| \hfill $T\.\ E^{**}$ &
    \&{typedef} \&I \.{@@[@@]} (|*|\&P)\cr
\+& |typedef_like| |comma| & |typedef_like| \hfill $TC\.\ $ &
    \&{typedef} \&{int} \&x,\cr
\+& |typedef_like| |semi| & |decl| & \&{typedef} \&{int} $\&x,\&y$;\cr
\+& |typedef_like| |ubinop| \alt |cast| |ubinop| & 
    |typedef_like| \alt |cast| |ubinop| \hfill
    \alt $C=\.\{U\.\}C$ $U_2=\.\{U_1\.\}U_2$ \unskip &
    \&{typedef} |*|{}|*|(\&{CPtr})\cr
\+& |delete_like| |lpar| |rpar| & |delete_like|\hfill $DL\.{\\,}R$
    & \&{delete}[\,] \cr
\+& |delete_like| |exp| & |exp| \hfill $D\.\ E$ & \&{delete} $p$ \cr
\+\dagit& |question| |exp| \alt |colon| |base| & |binop|
    & \malt {$\?x:$} {$\?f(\,):$} \cr
\+& |begin_arg| |end_arg| & |exp| & \.{@@[}\&{char}$*$\.{@@]}\cr
\+& \\{any\_other} |end_arg| & |end_arg| &    \&{char}$*$\.{@@]}\cr
\+& |alignas_like| |decl_head| & |attr| & \&{alignas}(\&{struct} $s$ ${*})$ \cr
\+& |alignas_like| |exp| & |attr| & |alignas(8)| \cr
\+& |lbrack| |lbrack| & |attr_head| & attribute begins \cr
\+& |lbrack| & |lpar| & |[| elsewhere \cr
\+& |rbrack| & |rpar| & |]| elsewhere \cr
\+& |attr_head| |rbrack| |rbrack| & |attr| & |[[@t\dots@>]]| \cr
\+& |attr_head| |exp| & |attr_head| & |[[deprecated| \cr
\+& |attr_head| |using_like| |exp| |colon| & |attr_head|
    & [[\&{using} \.{NS}: \cr
\+& |attr| \alt|lbrace| |stmt| & \alt|lbrace| |stmt| \hfill
       $A$\.\ \alt $S$ $L$ & |[[likely]] {|\cr
\+& |attr| |tag| & |tag| \hfill $A\.\ T$ & |[[likely]] case 0:| \cr
\+& |attr| |semi| & |stmt| & |[[fallthrough]];| \cr
\+& |attr| |attr| & |attr| \hfill $A_1\.\ A_2$
    & \&{alignas}($x$) [[\hbox{\dots}]] \cr
\+& |attr| |decl_head| & |decl_head| & [[\\{nodiscard}]] $f$(\,) \cr
\+& |decl_head| |attr| & |decl_head| & (\&{int} $x$ [[\\{deprecated}]])\cr
\+& |using_like| & |int_like| & \&{using} not in attributes \cr
\+& |struct_like| |attr| & |struct_like| \hfill $S\.\ A$
    & \&{struct} [[\\{deprecated}]]\cr
\+& |exp| |attr| & |attr| \hfill $E\.\ A$ & \&{enum} $\{x\ [[\ldots]]\}$ \cr
\+& |attr| |typedef_like| & |typedef_like| \hfill $A\.\ T$
    & |[[deprecated]] typedef| \cr
\+& |raw_int| |lbrack| & |exp| & |int[3]| \cr
\+& |attr_head| |comma| & |attr_head| & $[[x,y$ \cr
\+& |if_head| |attr| & |if_head| \hfill $I\.\ A$
    & \&{if} ($x$) [[\\{unlikely}]] $\{$ \cr
\+& |lbrack| |lbrack| |rbrack| |rbrack| & |exp| & |[[]]| \cr
\+& |attr| |function| & |function| \hfill $A\.\ F$
    & attribute and function \cr
\+& |default_like| |colon| & |case_like| |colon| & |default:| \cr
\+& |default_like| & |exp| & $f(\,)=\&{default};$ \cr
\+& |struct_like| |struct_like| & |struct_like| \hfill $S_1\.\ S_2$
    & |enum class| \cr
\+& |exp| |colcol| |int_like| & |int_like| & $\\{std}\DC\&{atomic}$ \cr
\advance\midcol-30pt
\+\dagit& |langle| |struct_like| \alt |exp| |int_like| |comma| &
  |langle| \hfill $LS$\alt $E^{**}$ $I^{**}$ \unskip $C$
   & $\langle$\&{typename} $t,$\cr
\+\dagit& |langle| |struct_like| \alt |exp| |int_like| |prerangle| &
  |cast| \hfill $LS$\alt $E^{**}$ $I^{**}$ \unskip $P$
    & $\langle$\&{typename} $t\rangle$ \cr
\advance\midcol30pt
\+& |template_like| |cast| |struct_like| & |struct_like| \hfill $T\.\ CS$ &
  |template<@t\dots@>> class| \cr
\+& |tag| |rbrace| & |decl| |rbrace| & @q{@>|public: }| \cr
\+& |fn_decl| |attr| & |fn_decl| \hfill $F\.\ A$
    & \&{void} $f$(\,) \&{noexcept} \cr
\+& |alignas_like| |cast| & |attr| & |alignas(int)| \cr
\vfill\break
\parindent=0pt
\dag{\bf Notes}
\yskip
Rule 35: The |exp| must not be immediately followed by |lpar|, |lbrack|,
|exp|, or~|cast|.

Rule 48: The |exp| or |int_like| must not be immediately followed by |base|.

Rule 76: The |force| in the |stmt| line becomes \\{bsp} if \.{CWEAVE} has
been invoked with the \.{-f} option.

Rule 78: The |define_like| case calls |make_underlined| on the following scrap.

Rule 94: The |new_like| must not be immediately followed by |lpar|.

Rule 99: The |new_exp| must not be immediately followed by |raw_int|,
|struct_like|, or |colcol|.

Rule 110: The |raw_int| must not be immediately followed by |langle|.

Rule 111: The operator after |operator_like|
must not be immediately followed by a |binop|.

Rule 114: The |operator_like| must not be immediately followed by
|raw_ubin|.

Rule 117: The |exp| must not be immediately followed by |lpar|, |exp|,
or |cast|.

Rule 123: The mathness of the |colon| or |base| changes to `yes'.

Rules 153, 154: |make_reserved| is called only if \.{CWEAVE} has been invoked
with the \.{+t} option.

\endgroup
