const_string CWEAVEHELP[] = {
    "Usage: cweave [OPTIONS] WEBFILE[.w] [{CHANGEFILE[.ch]|-} [OUTFILE[.tex]]]",
    "  Weave WEBFILE with CHANGEFILE into a TeX document.",
    "  Default CHANGEFILE is /dev/null;",
    "  TeX output goes to the basename of WEBFILE extended with `.tex'",
    "  unless otherwise specified by OUTFILE; in this case, '-' specifies",
    "  a null CHANGEFILE.",
    "",
    "-b          suppress banner line on terminal",
    "-f          do not force a newline after every C statement in output",
    "-h          suppress success message on completion",
    "-p          suppress progress report messages",
    "-x          omit indices and table of contents",
    "+e          enclose C material in \\PB{...}",
    "+s          print usage statistics",
    "--help      display this help and exit",
    "--version   output version information and exit",
    NULL
};

const_string CTANGLEHELP[] = {
    "Usage: ctangle [OPTIONS] WEBFILE[.w] [{CHANGEFILE[.ch]|-} [OUTFILE[.c]]]",
    "  Tangle WEBFILE with CHANGEFILE into a C/C++ program.",
    "  Default CHANGEFILE is /dev/null;",
    "  C output goes to the basename of WEBFILE extended with `.c'",
    "  unless otherwise specified by OUTFILE; in this case, '-' specifies",
    "  a null CHANGEFILE.",
    "",
    "-b          suppress banner line on terminal",
    "-h          suppress success message on completion",
    "-p          suppress progress report messages",
    "+s          print usage statistics",
    "--help      display this help and exit",
    "--version   output version information and exit",
    NULL
};
