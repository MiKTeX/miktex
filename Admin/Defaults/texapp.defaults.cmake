;; Maximum number of input files and error insertions that can be going
;; on simultaneously.
max_in_open=50

;; Maximum number of semantic levels simultaneously active.
nest_size=500

;; Space for saving values outside current group.
save_size=50000

;; Space for hyphenation patterns.
trie_size=700000

;; Total number of fonts.
font_max=9000

;; Words of font info for TeX (total size of all TFM files, approximately).
font_mem_size=3000000

;; Extra space for the hash table of control sequences (which allows 10K
;; names as distributed).
hash_extra=200000

;; Prime number of hyphenation exceptions.
hyph_size=8191

;; Size of the output buffer; must be a multiple of 8.
dvi_buf_size=8192

;; Enable system commands via \write18{...}.  When enabled fully (set to
;; t), obviously insecure.  When enabled partially (set to p), only the
;; commands listed in AllowedShellCommands are allowed.
EnableWrite18=p

;; The programs listed here are as safe as any we know: they either do
;; not write any output files, respect openout_any, or have hard-coded
;; restrictions similar to or higher than openout_any=p.  They also
;; have no features to invoke arbitrary other programs, and no known
;; exploitable bugs.  All to the best of our knowledge.  They also
;; have practical use for being called from TeX.
AllowedShellCommands=
AllowedShellCommands;=${MIKTEX_PREFIX}-bibtex
AllowedShellCommands;=${MIKTEX_PREFIX}-bibtex8
AllowedShellCommands;=${MIKTEX_PREFIX}-epstopdf
AllowedShellCommands;=${MIKTEX_PREFIX}-gregorio
AllowedShellCommands;=${MIKTEX_PREFIX}-kpsewhich
AllowedShellCommands;=${MIKTEX_PREFIX}-makeindex
AllowedShellCommands;=bibtex
AllowedShellCommands;=bibtex8
AllowedShellCommands;=epstopdf
AllowedShellCommands;=extractbb
AllowedShellCommands;=findtexmf
AllowedShellCommands;=gregorio
AllowedShellCommands;=kpsewhich
AllowedShellCommands;=makeindex
AllowedShellCommands;=texosquery-jre8
