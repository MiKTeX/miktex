[${MIKTEX_CONFIG_SECTION_CORE}]

	;; Shell command mode.
	;;   Forbidden: don't allow shell commands
	;;   Restricted: allow the commands listed in ${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS}
	;;   Unrestricted: allow all shell commands
	${MIKTEX_CONFIG_VALUE_SHELLCOMMANDMODE} = Restricted

	;; The programs listed here are as safe as any we know: they
	;; either do not write any output files, respect openout_any,
	;; or have hard-coded restrictions similar to or higher than
	;; openout_any=p.  They also have no features to invoke
	;; arbitrary other programs, and no known exploitable bugs.
	;; All to the best of our knowledge.  They also have practical
	;; use for being called from TeX.
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = ${MIKTEX_PREFIX}bibtex
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = ${MIKTEX_PREFIX}bibtex8
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = ${MIKTEX_PREFIX}epstopdf
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = ${MIKTEX_PREFIX}gregorio
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = ${MIKTEX_PREFIX}kpsewhich
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = ${MIKTEX_PREFIX}makeindex
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = bibtex
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = bibtex8
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = epstopdf
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = extractbb
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = findtexmf
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = gregorio
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = kpsewhich
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = makeindex
	${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS} = texosquery-jre8

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.tex]

	;; Search path.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/{$progname,generic,}

	;; Environment variables to be used for searching.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXINPUTS

	;; File name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .tex

[${MIKTEX_CONFIG_SECTION_MPM}]

	;; Install packages for all users.
	${MIKTEX_CONFIG_VALUE_AUTOADMIN} = ${MPM_AutoAdmin}

	;; Install missing packages automatically (on-the-fly).
	${MIKTEX_CONFIG_VALUE_AUTOINSTALL} = ${MPM_AutoInstall}

[${MIKTEX_CONFIG_SECTION_TEXANDFRIENDS}]

	;; Create the output directory if '--output-directory=DIR'
	;; refers to a non-existing directory.
	${MIKTEX_CONFIG_VALUE_CREATEOUTPUTDIRECTORY} = t
