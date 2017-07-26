[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.enc]

	!clear ${MIKTEX_CONFIG_VALUE_PATHS}
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/ttf2tfm//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/enc//
	;${MIKTEX_CONFIG_VALUE_PATHS} = %R/dvips//

;; DEPRECATED
[ft.enc]

path=.
path;=%R/ttf2tfm//
path;=%R/fonts/enc//

;; <legacy>
path;=%R/dvips//
;; </legacy>

