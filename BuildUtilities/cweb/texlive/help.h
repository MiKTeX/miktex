/* help.h: help messages for web2c programs.

   This is included by everyone, from cpascal.h.  This is better than
   putting the help messages directly in the change files because (1)
   multiline strings aren't supported by tangle, and it would be a pain
   to make up a new syntax for them in web2c, and (2) when a help msg
   changes, we need only recompile, not retangle or reconvert.  The
   downside is that everything gets recompiled when any msg changes, but
   that's better than having umpteen separate tiny files.  (For one
   thing, the messages have a lot in common, so it's nice to have them
   in one place.)

Copyright 1995, 1996, 2009, 2011-2022 Karl Berry.
Copyright 2008 Taco Hoekwater.
Copyright 2001, 2003, 2004 Olaf Weber.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.  */

/* Help messages for TeX & MF family are in texmfmp-help.h, to
   prevent multiple-definition errors. */

/* Stripped down for MiKTeX. */

#pragma

#ifdef CWEB
const_string CTANGLEHELP[] = {
    "Usage: ctangle [OPTIONS] WEBFILE[.w] [{CHANGEFILE[.ch]|-} [OUTFILE[.c]]]",
    "  Tangle WEBFILE with CHANGEFILE into a C/C++ program.",
    "  Default CHANGEFILE is " DEV_NULL ";",
    "  C output goes to the basename of WEBFILE extended with `.c'",
    "  unless otherwise specified by OUTFILE; in this case, '-' specifies",
    "  a null CHANGEFILE.",
    "",
    "+b          print banner line on terminal",
    "+h          print success message on completion",
    "+p          print progress report messages",
    "+/-q        shortcut for '-bhp'; also '--quiet' (default)",
    "+/-v        shortcut for '+bhp'; also '--verbose'",
    "+c          check temporary output for changes",
    "-dN         set 'kpathsea_debug' to N (0..127)",
    "+k          keep separators in numeric literals in the output",
    "+s          print usage statistics",
    "--help      display this help and exit",
    "--version   output version information and exit",
    NULL
};

const_string CWEAVEHELP[] = {
    "Usage: cweave [OPTIONS] WEBFILE[.w] [{CHANGEFILE[.ch]|-} [OUTFILE[.tex]]]",
    "  Weave WEBFILE with CHANGEFILE into a TeX document.",
    "  Default CHANGEFILE is " DEV_NULL ";",
    "  TeX output goes to the basename of WEBFILE extended with `.tex'",
    "  unless otherwise specified by OUTFILE; in this case, '-' specifies",
    "  a null CHANGEFILE.",
    "",
    "+b          print banner line on terminal",
    "+h          print success message on completion",
    "+p          print progress report messages",
    "+/-q        shortcut for '-bhp'; also '--quiet' (default)",
    "+/-v        shortcut for '+bhp'; also '--verbose'",
    "+c          check temporary output for changes",
    "-dN         set 'kpathsea_debug' to N (0..127)",
    "-e          do not enclose C material in \\PB{...}",
    "-f          do not force a newline after every C statement in output",
    "-i          suppress indentation of parameter declarations",
    "-o          suppress separation of declarations and statements",
    "-x          omit indices, section names, table of contents",
    "+lX         use macros for language X as of Xcwebmac.tex",
    "+s          print usage statistics",
    "+t          treat 'typename' in a template like 'typedef'",
    "--help      display this help and exit",
    "--version   output version information and exit",
    NULL
};

const_string CTWILLHELP[] = {
    "Usage: ctwill [OPTIONS] WEBFILE[.w] [{CHANGEFILE[.ch]|-} [OUTFILE[.tex]]]",
    "  Weave WEBFILE with CHANGEFILE into a TeX document with mini-indexes.",
    "  Default CHANGEFILE is " DEV_NULL ";",
    "  TeX output goes to the basename of WEBFILE extended with `.tex'",
    "  unless otherwise specified by OUTFILE; in this case, '-' specifies",
    "  a null CHANGEFILE.",
    "",
    "+b          print banner line on terminal",
    "+h          print success message on completion",
    "+p          print progress report messages",
    "+/-q        shortcut for '-bhp'; also '--quiet' (default)",
    "+/-v        shortcut for '+bhp'; also '--verbose'",
    "+c          check temporary output for changes",
    "-dN         set 'kpathsea_debug' to N (0..127)",
    "-e          do not enclose C material in \\PB{...}",
    "-f          do not force a newline after every C statement in output",
    "-i          suppress indentation of parameter declarations",
    "-o          suppress separation of declarations and statements",
    "-x          omit indices, section names, table of contents",
    "+P          \\input ctproofmac.tex instead of ctwimac.tex",
    "+/-lX       use macros for language X as of Xct{wi|proof}mac.tex",
    "+s          print usage statistics",
    "+t          treat 'typename' in a template like 'typedef'",
    "--help      display this help and exit",
    "--version   output version information and exit",
    NULL
};
#endif /* CWEB */
