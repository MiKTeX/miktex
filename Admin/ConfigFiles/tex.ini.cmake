!include texmfapp.ini
!include texapp.ini

[${MIKTEX_CONFIG_SECTION_CORE}]

	!clear ${MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS}

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.tex]

	!clear ${MIKTEX_CONFIG_VALUE_PATHS}
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/plain//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/generic//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex//

;; DEPRECATED
AllowShellCommands=none

[ft.tex]

path=.
path;=%R/tex/plain//
path;=%R/tex/generic//
path;=%R/tex//
