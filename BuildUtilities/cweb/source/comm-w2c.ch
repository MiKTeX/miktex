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
\def\title{Common code for CTANGLE and CWEAVE (Version 4.7)}
@y
\def\Kpathsea/{{\mc KPATHSEA\spacefactor1000}} \ifacro\sanitizecommand\Kpathsea{KPATHSEA}\fi
\def\title{Common code for CTANGLE and CWEAVE (4.7 [\TeX~Live])}
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
to both \.{CTANGLE} and \.{CWEAVE}, which roughly concerns the following
@y
to \.{CTANGLE}, \.{CWEAVE}, and \.{CTWILL},
which roughly concerns the following
@z

@x
sometimes use \.{CWEB} to refer to either of the two component
@y
sometimes use \.{CWEB} to refer to any of the three component
@z

@x
@i common.h
@y
@i comm-w2c.h
@z

@x
|program|.
@y
|program|. And \.{CTWILL} adds some extra twists.
@z

@x
boolean program; /* \.{CWEAVE} or \.{CTANGLE}? */
@y
cweb program; /* \.{CTANGLE} or \.{CWEAVE} or \.{CTWILL}? */
@z

@x
  @<Initialize pointers@>@;
@y
  @<Initialize pointers@>@;
  @<Set up |PROGNAME| feature and initialize the search path mechanism@>@;
@z

@x
  @<Set the default options common to \.{CTANGLE} and \.{CWEAVE}@>@;
@y
  @<Set locale and bind language catalogs@>@;
  @<Set the default options common to \.{CTANGLE} and \.{CWEAVE}@>@;
@z

@x
    if ((*(k++) = c) != ' ') limit = k;
@y
    if ((*(k++) = c) != ' ' && c != '\r') limit = k;
@z

@x
      ungetc(c,fp); loc=buffer; err_print("! Input line too long");
@y
      ungetc(c,fp); loc=buffer; err_print(_("! Input line too long"));
@z

@x
static char alt_web_file_name[max_file_name_length]; /* alternate name to try */
@y
@z

@x
    err_print("! Missing @@x in change file");
@y
    err_print(_("! Missing @@x in change file"));
@z

@x
    err_print("! Change file ended after @@x");
@y
    err_print(_("! Change file ended after @@x"));
@z

@x
      err_print("! Change file ended before @@y");
@y
      err_print(_("! Change file ended before @@y"));
@z

@x
        err_print("! CWEB file ended during a change");
@y
        err_print(_("! CWEB file ended during a change"));
@z

@x
  loc=buffer+2; err_print("! Where is the matching @@y?");
@y
  loc=buffer+2; err_print(_("! Where is the matching @@y?"));
@z

@x
    err_print("of the preceding lines failed to match");
@y
    err_print(_("of the preceding lines failed to match"));
@z

@x
if ((web_file=fopen(web_file_name,"r"))==NULL) {
  strcpy(web_file_name,alt_web_file_name);
  if ((web_file=fopen(web_file_name,"r"))==NULL)
       fatal("! Cannot open input file ", web_file_name);
}
@y
if ((found_filename=kpse_find_cweb(web_file_name))==NULL @|
    || (web_file=fopen(found_filename,"r"))==NULL)
  fatal(_("! Cannot open input file "), web_file_name);
else if (strlen(found_filename) < max_file_name_length) {
  /* Copy name for \#\&{line} directives. */
  if (strcmp(web_file_name, found_filename))
    strcpy(web_file_name, found_filename +
      ((strncmp(found_filename,"./",2)==0) ? 2 : 0));
  free(found_filename);
#if defined(MIKTEX)
  MiKTeX::Util::PathName sourceDir (web_file_name);
  sourceDir.RemoveFileSpec ();
  MIKTEX_SESSION()->AddInputDirectory(sourceDir, true);
#endif
} else fatal(_("! Filename too long\n"), found_filename);
@z

@x
if ((change_file=fopen(change_file_name,"r"))==NULL)
       fatal("! Cannot open change file ", change_file_name);
@y
if ((found_filename=kpse_find_cweb(change_file_name))==NULL @|
    || (change_file=fopen(found_filename,"r"))==NULL)
  fatal(_("! Cannot open change file "), change_file_name);
else if (strlen(found_filename) < max_file_name_length) {
  /* Copy name for \#\&{line} directives. */
  if (strcmp(change_file_name, found_filename))
    strcpy(change_file_name, found_filename +
      ((strncmp(found_filename,"./",2)==0) ? 2 : 0));
  free(found_filename);
} else fatal(_("! Filename too long\n"), found_filename);
@z

@x
      err_print("! Include file name not given");
@y
      err_print(_("! Include file name not given"));
@z

@x
      err_print("! Too many nested includes");
@y
      err_print(_("! Too many nested includes"));
@z

@x
@ When an \.{@@i} line is found in the |cur_file|, we must temporarily
stop reading it and start reading from the named include file.  The
\.{@@i} line should give a complete file name with or without
double quotes.
If the environment variable \.{CWEBINPUTS} is set, or if the compiler flag
of the same name was defined at compile time,
\.{CWEB} will look for include files in the directory thus named, if
it cannot find them in the current directory.
(Colon-separated paths are not supported.)
The remainder of the \.{@@i} line after the file name is ignored.
@y
@ When an \.{@@i} line is found in the |cur_file|, we must temporarily
stop reading it and start reading from the named include file.  The
\.{@@i} line should give a complete file name with or without
double quotes.
The actual file lookup is done with the help of the \Kpathsea/ library;
see section~\X93:File lookup with \Kpathsea/\X~for details. % FIXME
The remainder of the \.{@@i} line after the file name is ignored.
@^system dependencies@> @.CWEBINPUTS@>
@z

@x
        err_print("! Include file name too long"); goto restart;}
@y
        err_print(_("! Include file name too long")); goto restart;}
@z

@x
  char temp_file_name[max_file_name_length];
  char *cur_file_name_end=cur_file_name+max_file_name_length-1;
  char *kk, *k=cur_file_name;
  size_t l; /* length of file name */
@y
  char *cur_file_name_end=cur_file_name+max_file_name_length-1;
  char *k=cur_file_name;
@z

@x
  if ((cur_file=fopen(cur_file_name,"r"))!=NULL) {
@y
  if ((found_filename=kpse_find_cweb(cur_file_name))!=NULL @|
      && (cur_file=fopen(found_filename,"r"))!=NULL) {
    /* Copy name for \#\&{line} directives. */
    if (strlen(found_filename) < max_file_name_length) {
      if (strcmp(cur_file_name, found_filename))
        strcpy(cur_file_name, found_filename +
          ((strncmp(found_filename,"./",2)==0) ? 2 : 0));
      free(found_filename);
    } else fatal(_("! Filename too long\n"), found_filename);
@z

@x
  if ((kk=getenv("CWEBINPUTS"))!=NULL) {
@.CWEBINPUTS@>
    if ((l=strlen(kk))>max_file_name_length-2) too_long();
    strcpy(temp_file_name,kk);
  }
  else {
#ifdef CWEBINPUTS
    if ((l=strlen(CWEBINPUTS))>max_file_name_length-2) too_long();
    strcpy(temp_file_name,CWEBINPUTS);
#else
    l=0;
#endif /* |CWEBINPUTS| */
  }
  if (l>0) {
    if (k+l+2>=cur_file_name_end) too_long();
@.Include file name ...@>
    for (; k>=cur_file_name; k--) *(k+l+1)=*k;
    strcpy(cur_file_name,temp_file_name);
    cur_file_name[l]='/'; /* \UNIX/ pathname separator */
    if ((cur_file=fopen(cur_file_name,"r"))!=NULL) {
      cur_line=0; print_where=true;
      goto restart; /* success */
    }
  }
  include_depth--; err_print("! Cannot open include file"); goto restart;
@y
  include_depth--; err_print(_("! Cannot open include file")); goto restart;
@z

@x
    err_print("! Change file ended without @@z");
@y
    err_print(_("! Change file ended without @@z"));
@z

@x
        err_print("! Where is the matching @@z?");
@y
        err_print(_("! Where is the matching @@z?"));
@z

@x
    err_print("! Change file entry did not match");
@y
    err_print(_("! Change file entry did not match"));
@z

@x
@d hash_size 353 /* should be prime */
@y
@d hash_size 8501 /* should be prime */
@z

@x
  if (byte_ptr+l>byte_mem_end) overflow("byte memory");
  if (name_ptr>=name_dir_end) overflow("name");
@y
  if (byte_ptr+l>byte_mem_end) overflow(_("byte memory"));
  if (name_ptr>=name_dir_end) overflow(_("name"));
@z

@x
  if (s+name_len>byte_mem_end) overflow("byte memory");
  if (name_ptr+1>=name_dir_end) overflow("name");
@y
  if (s+name_len>byte_mem_end) overflow(_("byte memory"));
  if (name_ptr+1>=name_dir_end) overflow(_("name"));
@z

@x
  if (name_ptr>=name_dir_end) overflow("name");
@y
  if (name_ptr>=name_dir_end) overflow(_("name"));
@z

@x
  if (s+name_len>byte_mem_end) overflow("byte memory");
@y
  if (s+name_len>byte_mem_end) overflow(_("byte memory"));
@z

@x
      fputs("\n! Ambiguous prefix: matches <",stdout);
@y
      fputs(_("\n! Ambiguous prefix: matches <"),stdout);
@z

@x
      fputs(">\n and <",stdout);
@y
      fputs(_(">\n and <"),stdout);
@z

@x
      fputs("\n! New name is a prefix of <",stdout);
@y
      fputs(_("\n! New name is a prefix of <"),stdout);
@z

@x
      fputs("\n! New name extends <",stdout);
@y
      fputs(_("\n! New name extends <"),stdout);
@z

@x
    fputs("\n! Section name incompatible with <",stdout);
@y
    fputs(_("\n! Section name incompatible with <"),stdout);
@z

@x
    fputs(">,\n which abbreviates <",stdout);
@y
    fputs(_(">,\n which abbreviates <"),stdout);
@z

@x
  printf(". (l. %d of change file)\n", change_line);
else if (include_depth==0) printf(". (l. %d)\n", cur_line);
  else printf(". (l. %d of include file %s)\n", cur_line, cur_file_name);
@y
  printf(_(". (l. %d of change file)\n"), change_line);
else if (include_depth==0) printf(_(". (l. %d)\n"), cur_line);
  else printf(_(". (l. %d of include file %s)\n"), cur_line, cur_file_name);
@z

@x
Some implementations may wish to pass the |history| value to the
operating system so that it can be used to govern whether or not other
programs are started. Here, for instance, we pass the operating system
a status of |EXIT_SUCCESS| if and only if only harmless messages were printed.
@^system dependencies@>
@y
On multi-tasking systems like the {\mc AMIGA} it is very convenient to
know a little bit more about the reasons why a program failed.  The four
levels of return indicated by the |history| value are very suitable for
this purpose.  Here, for instance, we pass the operating system a status
of~0 if and only if the run was a complete success.  Any warning or error
message will result in a higher return value, so that {\mc AREXX} scripts
can be made sensitive to these conditions.
@^system dependencies@>

@d RETURN_OK     0 /* No problems, success */
@d RETURN_WARN   5 /* A warning only */
@d RETURN_ERROR 10 /* Something wrong */
@d RETURN_FAIL  20 /* Complete or severe failure */
@z

@x
  @<Print the job |history|@>@;
@y
  @<Print the job |history|@>@;
  @<Remove the temporary file if not already done@>@;
@z

@x
  if (history > harmless_message) return EXIT_FAILURE;
  else return EXIT_SUCCESS;
@y
  switch(history) {
  case spotless: return RETURN_OK;
  case harmless_message: return RETURN_WARN;
  case error_message: return RETURN_ERROR;
  case fatal_message: default: return RETURN_FAIL;
  }
@z

@x
case spotless:
  if (show_happiness) puts("(No errors were found.)"); break;
case harmless_message:
  puts("(Did you see the warning message above?)"); break;
case error_message:
  puts("(Pardon me, but I think I spotted something wrong.)"); break;
case fatal_message: default:
  puts("(That was a fatal error, my friend.)");
@y
case spotless:
  if (show_happiness) puts(_("(No errors were found.)")); break;
case harmless_message:
  puts(_("(Did you see the warning message above?)")); break;
case error_message:
  puts(_("(Pardon me, but I think I spotted something wrong.)")); break;
case fatal_message: default:
  puts(_("(That was a fatal error, my friend.)"));
@z

@x
  printf("\n! Sorry, %s capacity exceeded",t); fatal("","");
@y
  printf(_("\n! Sorry, %s capacity exceeded"),t); fatal("","");
@z

@x
or flags to be turned on (beginning with |"+"|).
@y
or flags to be turned on (beginning with |"+"|).
\TeX~Live's \.{CWEB} executables accept several ``long options'' as well;
see section |@<Handle flag arg...@>| for details.
@z

@x
char scn_file_name[max_file_name_length]; /* name of |scn_file| */
@y
char scn_file_name[max_file_name_length]; /* name of |scn_file| */
char check_file_name[max_file_name_length]; /* name of |check_file| */
@z

@x
show_banner=show_happiness=show_progress=make_xrefs=true;
@y
make_xrefs=true;
@z

@x
file.  It may have an extension, or it may omit the extension to get |".w"| or
|".web"| added.  The \TEX/ output file name is formed by replacing the \.{CWEB}
@y
file.  It may have an extension, or it may omit the extension to get |".w"|
added.  The \TEX/ output file name is formed by replacing the \.{CWEB}
@z

@x
An omitted change file argument means that |"/dev/null"| should be used,
when no changes are desired.
@y
An omitted change file argument means that |"/dev/null"| or---on non-\UNIX/
systems the contents of the compile-time variable |DEV_NULL| (\TeX~Live) or
|_DEV_NULL| (Amiga)---should be used, when no changes are desired.
@z

@x
  strcpy(change_file_name,"/dev/null");
@y
  strcpy(change_file_name,"/dev/null");
#if defined DEV_NULL
  strncpy(change_file_name,DEV_NULL,max_file_name_length-2);
  change_file_name[max_file_name_length-2]='\0';
#elif defined _DEV_NULL
  strncpy(change_file_name,_DEV_NULL,max_file_name_length-2);
  change_file_name[max_file_name_length-2]='\0';
#endif
@^system dependencies@>
@z

@x
      while (*s)
        if (*s=='.') dot_pos=s++;
        else if (*s=='/') dot_pos=NULL,name_pos=++s;
        else s++;
@y
      while (*s)
        if (*s=='.') dot_pos=s++;
        else if (*s==DIR_SEPARATOR || *s==DEVICE_SEPARATOR || *s=='/')
          dot_pos=NULL,name_pos=++s;
        else s++;
@^system dependencies@>
@z

@x
@ We use all of |*argv| for the |web_file_name| if there is a |'.'| in it,
otherwise we add |".w"|. If this file can't be opened, we prepare an
|alt_web_file_name| by adding |"web"| after the dot.
@y
@ We use all of |*argv| for the |web_file_name| if there is a |'.'| in it,
otherwise we add |".w"|.
@z

@x
  sprintf(alt_web_file_name,"%s.web",*argv);
@y
@z

@x
for(dot_pos=*argv+1;*dot_pos>'\0';dot_pos++)
  flags[(eight_bits)*dot_pos]=flag_change;
@y
{
  if (strcmp("-help",*argv)==0 || strcmp("--help",*argv)==0)
@.--help@>
    @<Display help message and |exit|@>@;
  if (strcmp("-version",*argv)==0 || strcmp("--version",*argv)==0)
@.--version@>
    @<Display version information and |exit|@>@;
  if (strcmp("-verbose",*argv)==0 || strcmp("--verbose",*argv)==0)
@.--verbose@>
    strcpy(*argv,"-v");
  if (strcmp("-quiet",*argv)==0 || strcmp("--quiet",*argv)==0)
@.--quiet@>
      strcpy(*argv,"-q");
  for(dot_pos=*argv+1;*dot_pos>'\0';dot_pos++) {
    switch (*dot_pos) {
    case 'v': show_banner=show_progress=show_happiness=true; continue;
    case 'q': show_banner=show_progress=show_happiness=false; continue;
    case 'd':
      if (sscanf(++dot_pos,"%u",&kpathsea_debug)!=1) @<Print usage error...@>@;
      while (isdigit(*dot_pos)) dot_pos++; /* skip numeric part */
      dot_pos--; /* reset to final digit */
      continue;
    case 'l': use_language=++dot_pos; break; /* from |switch| */
    default: flags[(eight_bits)*dot_pos]=flag_change; continue;
    }
    break; /* from |for| loop */
  }
}
@z

@x
@ @<Print usage error message and quit@>=
{
if (program==ctangle)
  fatal(
"! Usage: ctangle [options] webfile[.w] [{changefile[.ch]|-} [outfile[.c]]]\n"
   ,"");
@.Usage:@>
else fatal(
"! Usage: cweave [options] webfile[.w] [{changefile[.ch]|-} [outfile[.tex]]]\n"
   ,"");
}
@y
@ @<Print usage error message and quit@>=
cb_usage(program==ctangle ? "ctangle" : program==cweave ? "cweave" : "ctwill");
@.Usage:@>
@z

@x
@ @<Complain about arg...@>= fatal("! Filename too long\n", *argv);
@y
@ @<Complain about arg...@>= fatal(_("! Filename too long\n"), *argv);
@z

@x
FILE *scn_file; /* where list of sections from \.{CWEAVE} goes */
@y
FILE *scn_file; /* where list of sections from \.{CWEAVE} goes */
FILE *check_file; /* temporary output file */
@z

@x
FILE *active_file; /* currently active file for \.{CWEAVE} output */
@y
FILE *active_file; /* currently active file for \.{CWEAVE} output */
char *found_filename; /* filename found by |kpse_find_file| */
@z

@x
@ @<Scan arguments and open output files@>=
scan_args();
if (program==ctangle) {
  if ((C_file=fopen(C_file_name,"wb"))==NULL)
    fatal("! Cannot open output file ", C_file_name);
@.Cannot open output file@>
}
else {
  if ((tex_file=fopen(tex_file_name,"wb"))==NULL)
    fatal("! Cannot open output file ", tex_file_name);
}
@y
@ @<Scan arguments and open output files@>=
scan_args();
if (program==ctangle) {
  if (check_for_change) @<Open intermediate \CEE/ output file@>@;
  else if ((C_file=fopen(C_file_name,"wb"))==NULL)
    fatal(_("! Cannot open output file "), C_file_name);
@.Cannot open output file@>
}
else {
  if (check_for_change) @<Open intermediate \TEX/ output file@>@;
  else if ((tex_file=fopen(tex_file_name,"wb"))==NULL)
    fatal(_("! Cannot open output file "), tex_file_name);
}
@z

@x
@** Index.
@y
@** Extensions to {\tentex CWEB}.  The following sections introduce new or
improved features that have been created by numerous contributors over the
course of a quarter century.

Care has been taken to keep the original section numbering intact, so this new
material should nicely integrate with the original ``\&{85.~Index}.''

@* Language setting.  This global variable is set by the argument of the
`\.{+l}' (or `\.{-l}') command-line option.

@<Global var...@>=
const char *use_language=""; /* prefix of \.{cwebmac.tex} in \TEX/ output */


@* User communication.  The |scan_args| and |cb_show_banner| routines and the
|bindtextdomain| argument string need a few extra variables.

@d max_banner 50

@d PATH_SEPARATOR   separators[0]
@d DIR_SEPARATOR    separators[1]
@d DEVICE_SEPARATOR separators[2]

@<Global var...@>=
char cb_banner[max_banner];@/
string texmf_locale;@/
#ifndef SEPARATORS
#define SEPARATORS "://"
#endif
char separators[]=SEPARATORS;

@* Temporary file output. Most \CEE/ projects are controlled by a \.{Makefile}
that automatically takes care of the temporal dependecies between the different
source modules. It may be convenient that \.{CWEB} doesn't create new output
for all existing files, when there are only changes to some of them. Thus the
\.{make} process will only recompile those modules where necessary. You can
activate this feature with the `\.{+c}' command-line option. The idea and basic
implementation of this mechanism can be found in the program \.{NUWEB} by
Preston Briggs, to whom credit is due.

@<Open intermediate \CEE/ output file@>= {
  if ((C_file=fopen(C_file_name,"a"))==NULL)
    fatal(_("! Cannot open output file "), C_file_name);
@.Cannot open output file@>
  else fclose(C_file); /* Test accessability */
  strcpy(check_file_name,C_file_name);
  if(check_file_name[0]!='\0') {
    char *dot_pos=strrchr(check_file_name,'.');
    if(dot_pos==NULL) strcat(check_file_name,".ttp");
    else strcpy(dot_pos,".ttp");
  }
  if ((C_file=fopen(check_file_name,"wb"))==NULL)
    fatal(_("! Cannot open output file "), check_file_name);
}

@ @<Open intermediate \TEX/ output file@>= {
  if ((tex_file=fopen(tex_file_name,"a"))==NULL)
    fatal(_("! Cannot open output file "), tex_file_name);
@.Cannot open output file@>
  else fclose(tex_file); /* Test accessability */
  strcpy(check_file_name,tex_file_name);
  if(check_file_name[0]!='\0') {
    char *dot_pos=strrchr(check_file_name,'.');
    if(dot_pos==NULL) strcat(check_file_name,".wtp");
    else strcpy(dot_pos,".wtp");
  }
  if ((tex_file=fopen(check_file_name,"wb"))==NULL)
    fatal(_("! Cannot open output file "), check_file_name);
}

@ Before we leave the program we have to make
sure that the output files are correctly written.

@<Remove the temporary file...@>=
if(C_file) fclose(C_file);
if(tex_file) fclose(tex_file);
if(check_file) fclose(check_file);
if(strlen(check_file_name)) /* Delete the temporary file in case of a break */
   remove(check_file_name);

@* Internationalization.  If translation catalogs for your personal
\.{LANGUAGE} are installed at the appropriate place, \.{CTANGLE} and \.{CWEAVE}
will talk to you in your favorite language.  Catalog \.{cweb} contains all
strings from ``plain \.{CWEB},'' catalog \.{cweb-tl} contains a few extra
strings specific to the \TeX~Live interface, and catalog \.{web2c-help}
contains the ``\.{--help}'' texts for \.{CTANGLE} and \.{CWEAVE}.
@.cweb.mo@>
@.cweb-tl.mo@>
@.web2c-help.mo@>
@.--help@>

If such translation files are not available, you may want to improve this
system by checking out the sources and translating the strings in files
\.{cweb.pot}, \.{cweb-tl.pot}, and \.{web2c-help.pot}, and submitting the
resulting \.{*.po} files to the maintainers at \.{tex-k@@tug.org}.

\medskip \noindent \&{Note to maintainers:} \.{CWEB} in \TeX~Live generally
does \\{not} set |HAVE_GETTEXT| at build-time, so \.{i18n} is ``off'' by
default.  If you want to create \.{CWEB} executables with NLS support, you
have to recompile the \TeX~Live sources with a positive value for
|HAVE_GETTEXT| in \.{comm-w2c.h}.  Also you have to ``compile'' the NLS
catalogs provided for \.{CWEB} in the source tree with \.{msgfmt} and store the
resulting \.{.mo} files at an appropriate place in the file system.

Plans for \TeX~Live are to store NLS catalogs inside the ``\TeX\ Directory
Structure'' (TDS) and look them up with the help of the configuration variable
``|TEXMFLOCALEDIR|,'' which should contain a single absolute path definition.
Below we use the \Kpathsea/ function |kpse_var_expand| to evaluate this
variable from various origins and redirect the ``GNU~gettext utilities''
to a possibly different location than the canonical \.{/usr/share/locale}.

There are several ways to set |TEXMFLOCALEDIR|:
\smallskip
{\parindent5em
\item{(a)} a user-set environment variable \.{TEXMFLOCALEDIR}\hfil\break
    (overridden by \.{TEXMFLOCALEDIR\_cweb});
\item{(b)} a line in \Kpathsea/ configuration file \.{texmf.cnf},\hfil\break
    e.g., \.{TEXMFLOCALEDIR=\$TEXMFMAIN/locale}\hfil\break
    or \.{TEXMFLOCALEDIR.cweb=\$TEXMFMAIN/locale}.\par}

@<Include files@>=
#if HAVE_GETTEXT
#include <locale.h> /* |@!LC_MESSAGES|, |@!LC_CTYPE| */
#else
#define setlocale(a,b) ""
#define bindtextdomain(a,b) ""
#define textdomain(a) ""
#endif

@ @<Set locale...@>=
setlocale(LC_MESSAGES, setlocale(LC_CTYPE, ""));
texmf_locale = kpse_var_expand ("${TEXMFLOCALEDIR}");

bindtextdomain("cweb",
  bindtextdomain("cweb-tl",
    bindtextdomain("web2c-help", @|
      strcmp(texmf_locale, "") ?
        texmf_locale : "/usr/share/locale")));

free(texmf_locale);
textdomain("cweb"); /* the majority of |"strings"| come from ``plain \.{CWEB}'' */
@.cweb.mo@>

@* File lookup with \Kpathsea/.  The \.{CTANGLE} and \.{CWEAVE} programs from
the original \.{CWEB} package use the compile-time default directory or the
value of the environment variable \.{CWEBINPUTS} as an alternative place to be
searched for files, if they could not be found in the current directory.

This version uses the \Kpathsea/ mechanism for searching files.
The directories to be searched for come from three sources:
\smallskip
{\parindent5em
\item{(a)} a user-set environment variable \.{CWEBINPUTS}
    (overridden by \.{CWEBINPUTS\_cweb});
\item{(b)} a line in \Kpathsea/ configuration file \.{texmf.cnf},\hfil\break
    e.g., \.{CWEBINPUTS=\$TEXMFDOTDIR:\$TEXMF/texmf/cweb//}\hfil\break
    or \.{CWEBINPUTS.cweb=\$TEXMFDOTDIR:\$TEXMF/texmf/cweb//};
\item{(c)} compile-time default directories (specified in
    \.{texmf.in}),\hfil\break
    i.e., \.{\$TEXMFDOTDIR:\$TEXMF/texmf/cweb//}.\par}
@.CWEBINPUTS@>

@s const_string int
@s string int

@d kpse_find_cweb(name) kpse_find_file(name,kpse_cweb_format,true)

@<Include files@>=
#include <kpathsea/kpathsea.h> /* include every \Kpathsea/ header;
  |@!kpathsea_debug|, |@!const_string|, |@!string| */
#include <w2c/config.h> /* \&{integer} */
#include <lib/lib.h> /* |@!versionstring| */
#if defined(MIKTEX)
#include <cstdlib>
#undef exit
#define exit(exitCode) throw(exitCode)
#endif

@ We set |kpse_program_name| to `\.{cweb}'.  This means if the variable
\.{CWEBINPUTS.cweb} is present in \.{texmf.cnf} (or \.{CWEBINPUTS\_cweb}
in the environment) its value will be used as the search path for filenames.
This allows different flavors of \.{CWEB} to have different search paths.
@.CWEBINPUTS@>

@<Set up |PROGNAME| feature and initialize the search path mechanism@>=
kpse_set_program_name(argv[0], "cweb");

@ When the files you expect are not found, the thing to do is to enable
\Kpathsea/ runtime debugging by assigning to the |kpathsea_debug| variable a
small number via the `\.{-d}' option. The meaning of this number is shown
below. To set more than one debugging option, simply sum the corresponding
numbers.
\medskip
\halign{\hskip5em\tt\hfil#&&\qquad\rm#\hfil\cr
 1&report `\.{stat}' calls\cr
 2&report lookups in all hash tables\cr
 4&report file openings and closings\cr
 8&report path information\cr
16&report directory list\cr
32&report on each file search\cr
64&report values of variables being looked up\cr}
\medskip
Debugging output is always written to |stderr|, and begins with the string
`\.{kdebug:}'.

@* System dependent changes. The most volatile stuff comes at the very end.

Modules for dealing with help messages and version info.

@<Include files@>=
#define CWEB
#include "help.h" /* |@!CTANGLEHELP|, |@!CWEAVEHELP|, |@!CTWILLHELP| */

@ @<Display help message and |exit|@>=
cb_usagehelp(program==ctangle ? CTANGLEHELP :
  program==cweave ? CWEAVEHELP : CTWILLHELP);
@.--help@>

@ Special variants from Web2c's `\.{lib/usage.c}', adapted for
\.{i18n}/\.{t10n}.  We simply filter the strings through the catalogs
(if available).

@<Predecl...@>=
static void cb_usage (const_string str);@/
static void cb_usagehelp (const_string *message);@/

@ @c
static void cb_usage (const_string str)
{
  textdomain("cweb-tl");
@.cweb-tl.mo@>
  fprintf(stderr, _("%s: Need one to three file arguments.\n"), str);
  fprintf(stderr, _("Try `%s --help' for more information.\n"), str);
@.--help@>
  textdomain("cweb");
@.cweb.mo@>
  history=fatal_message; exit(wrap_up());
}

static void cb_usagehelp (const_string *message)
{
  textdomain("web2c-help");
@.web2c-help.mo@>
  while (*message) {
    /* empty string \.{""} has special meaning for |gettext| */
    printf("%s\n", strcmp("", *message) ? _(*message) : *message);
    ++message;
  }
  textdomain("cweb-tl");
@.cweb-tl.mo@>
  printf(_("\nPackage home page: %s.\n"), "https://ctan.org/pkg/cweb");
  textdomain("cweb");
@.cweb.mo@>
  history=spotless; exit(wrap_up());
}

@ The version information will not be translated, it uses a generic text
template in English.

@<Display version information and |exit|@>=
printversionandexit(cb_banner,
  program == ctwill ? "Donald E. Knuth" : "Silvio Levy and Donald E. Knuth",
  NULL, "Contemporary development on https://github.com/ascherer/cweb.\n");
@.--version@>

@ But the ``banner'' is, at least the first part.

@c
void cb_show_banner (void)
{
  textdomain("cweb-tl");
@.cweb-tl.mo@>
  printf("%s%s\n", _(cb_banner), versionstring);
  textdomain("cweb");
@.cweb.mo@>
}

@** Index.
@z
