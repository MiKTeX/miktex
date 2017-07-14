[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.tex]

	!clear ${MIKTEX_CONFIG_VALUE_PATHS}
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/platex//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/latex//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/generic//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex//

	!clear ${MIKTEX_CONFIG_VALUE_EXTENSIONS}
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .tex
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .ltx

;; DEPRECATED
[ft.tex]

extensions=
extensions;=.tex
extensions;=.ltx

path=.
path;=%R/tex/platex//
path;=%R/tex/latex//
path;=%R/tex/generic//
path;=%R/tex//
