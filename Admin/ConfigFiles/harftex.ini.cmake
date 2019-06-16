!include texmfapp.ini
!include texapp.ini

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.tex]

	!clear ${MIKTEX_CONFIG_VALUE_PATHS}
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/harftex//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/luatex//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/plain//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/generic//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex//
