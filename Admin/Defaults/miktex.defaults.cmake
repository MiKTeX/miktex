;; The programs listed here are as safe as any we know: they either do
;; not write any output files, respect openout_any, or have hard-coded
;; restrictions similar to or higher than openout_any=p.  They also
;; have no features to invoke arbitrary other programs, and no known
;; exploitable bugs.  All to the best of our knowledge.  They also
;; have practical use for being called from TeX.
AllowedShellCommands=
AllowedShellCommands;=${MIKTEX_PREFIX}bibtex
AllowedShellCommands;=${MIKTEX_PREFIX}bibtex8
AllowedShellCommands;=${MIKTEX_PREFIX}epstopdf
AllowedShellCommands;=${MIKTEX_PREFIX}gregorio
AllowedShellCommands;=${MIKTEX_PREFIX}kpsewhich
AllowedShellCommands;=${MIKTEX_PREFIX}makeindex
AllowedShellCommands;=bibtex
AllowedShellCommands;=bibtex8
AllowedShellCommands;=epstopdf
AllowedShellCommands;=extractbb
AllowedShellCommands;=findtexmf
AllowedShellCommands;=gregorio
AllowedShellCommands;=kpsewhich
AllowedShellCommands;=makeindex
AllowedShellCommands;=texosquery-jre8
