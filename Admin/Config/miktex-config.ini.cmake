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

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.afm]

	;; Search path for Adobe font metric (AFM) files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/afm//

	;; Environment variables to be used for searching AFM files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = AFMFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; AFM file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .afm

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.base]

	;; Search path for METAFONT memory dump files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %r/${MIKTEX_REL_MIKTEX_BASE_DIR}

	;; METAFONT memory dump file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .base

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.bib]

	;; Search path for BibTeX database files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/bibtex/bib//

	;; Environment variables to be used for searching BibTeX
	;; databsae files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = BIBINPUTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXBIB

	;; BibTeX database file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .bib

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.bst]

	;; Search path for BibTeX style files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/bibtex/{bst,csf}//

	;; Environment variables to be used for searching BibTeX
	;; style files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = BSTINPUTS

	;; BibTeX style file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .bst

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.cid maps]

	;; Search path for CID map files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/cid//

	;; Environment variables to be used for searching CID map
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = FONTCIDMAPS

	;; CID map file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .cid
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .cidmap

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.clua]

	;; Search path for dynamic libraries for Lua.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/scripts/{$$progname,$$engine,}/lua//

	;; Environment variables to be used for searching dynamic
	;; libraries for Lua.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = CLUAINPUTS

	;; File name extensions for Lua dynamic libraries.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .dll
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .so
	
[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.cmap files]

	;; Search path for character map files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/cmap//

	;; Environment variables to be used for character map files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = CMAPFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.cweb]

	;; Search path for CWeb input files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/cweb//

	;; Environment variables to be used for searching Cweb input
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = CWEBINPUTS

	;; CWeb file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .w

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.dvi]

	;; Search path for DVI files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/doc//

	;; DVI file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .dvi

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.dvips config]

	;; Search path for Dvips configuration files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/dvips//

	;; Environment variables to be used for searching Dvips
	;; configuration files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXCONFIG

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.enc]

	;; Search path for encoding vector files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/enc//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/miktex/config//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/dvips//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/pdftex//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/dvipdfm//

	;; Environment variables to be used for searching encoding
	;; vector files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = ENCFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; Encoding vector file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .enc

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.font feature files]

	;; Search path for font feature files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/fea//

	;; Environment variables to be used for searching font feature
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = FONTFEATURES

	;; Font feature file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .fea

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.fmt]

	;; Search path for TeX memory dump files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %r/${MIKTEX_REL_MIKTEX_FMT_DIR}/{$engine,}

	;; TeX memory dump file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .fmt

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.gf]

	;; Search path for generic font bitmap files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts//

	;; Environment variables to be used for searching generic font
	;; bitmap files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = GFFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = GLYPHFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; Generic font bitmap file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .gf

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.bitmap font]

	;; Search path for bitmap font files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts//

	;; Environment variables to be used for searching bitmap font
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = GLYPHFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.graphic/figure]

	;; Search path for figure files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/dvips//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/pdftex//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex//

	;; Environment variables to be used for searching figure
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXPICTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXINPUTS

	;; Figure file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .eps
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .epsi
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .png

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.hbf]

	;; Search path for HBF files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/misc/hbf//

	;; HBF file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .hbf

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.ist]

	;; Search path for MakeIndex style files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/makeindex//

	;; Environment variables to be used for searching MakeIndex
	;; style files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXINDEXSTYLE
	${MIKTEX_CONFIG_VALUE_ENVVARS} = INDEXSTYLE

	;; MakeIndex style file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .ist

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.lig files]

	;; Search path for ligature definition files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/lig//

	;; Environment variables to be used for searching ligature
	;; definition files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; Ligature definition file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .lig

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.ls-R]

	;; Search path for Web2c file name database files.
	${MIKTEX_CONFIG_VALUE_PATHS} = %R

	;; Environment variables to be used for searching Web2C file
	;; name database files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXMFDBS

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.lua]

	;; Search path for Lua files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/scripts/{$$progname,$$engine,}/{lua,}//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/{$$progname,generic,}//

	;; Environment variables to be used for searching Lua files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = LUAINPUTS

	;; File name extensions for Lua files.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .lua
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .luatex
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .luc
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .luctex
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .texlua
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .texluc
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .tlu

	;; Environment variables to be used for searching Lua files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = LUAINPUTS

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.map]

	;; Search path for font map files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/map/{$$progname,pdftex,dvips,}//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/miktex/config//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/dvips//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/pdftex//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/dvipdfm//

	;; Environment variables to be used for searching font map
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTMAPS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; Font map file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .map

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.mem]

	;; Search path for MetaPost memory dump files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .

	;; MetaPost memory dump file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .mem

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.mf]

	;; Search path for METAFONT input files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/metafont//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/source//

	;; Environment variables to be used for searching METAFONT
	;; input files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = MFINPUTS

	;; METAFONT file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .mf

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.mfpool]

	;; Search path for METAFONT program string files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .

	;; Environment variables to be used for searching METAFONT
	;; program string files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = MFPOOL
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXMFINI

	;; METAFONT program string file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .pool

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.mft]

	;; Search path for MFT style files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/mft//

	;; Environment variables to be used for searching MFT style
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = MFTINPUTS

	;; MFT style file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .mft

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.misc fonts]

	;; Search path for font related files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/misc//

	;; Environment variables to be used for font related
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = MISCFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.mlbib]

	;; Search path for MlBibTeX database files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/bibtex/bib/{mlbib,}//

	;; Environment variables to be used for searching MlBibTeX
	;; databsae files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = MLBIBINPUTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = BIBINPUTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXBIB

	;; MlBibTeX database file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .mlbib
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .bib

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.mlbst]

	;; Search path for MlBibTeX style files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/bibtex/{mlbst,bst}//

	;; Environment variables to be used for searching MlBibTeX
	;; style files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = MLBSTINPUTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = BSTINPUTS

	;; MlBibTeX style file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .bst

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.mp]

	;; Search path for MetaPost input files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/metapost//

	;; Environment variables to be used for searching MetaPost
	;; input files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = MPINPUTS

	;; MetaPost file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .mp

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.mppool]

	;; Search path for MetaPost program string files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .

	;; Environment variables to be used for searching MetaPost
	;; program string files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = MPPOOL
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXMFINI

	;; MetaPost program string file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .pool

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.MetaPost support]

	;; Search path for MetaPost support files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/metapost/support//

	;; Environment variables to be used for searching MetaPost
	;; support files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = MPSUPPORT

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.ocp]

	;; Search path for Omega compiled process files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/omega/ocp//

	;; Environment variables to be used for searching Omega
	;; compiled process files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = OCPINPUTS

	;; Omega compiled process file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .ocp

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.ofm]

	;; Search path for Omega font metric files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/ofm//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/tfm//

	;; Environment variables to be used for searching Omega
	;; font metric files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = OFMFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; Omega font metric file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .ofm
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .tfm

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.opl]

	;; Search path for Omega property list files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/opl//

	;; Environment variables to be used for searching Omega
	;; property list files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = OPLFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; Omega property list file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .opl

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.otp]

	;; Search path for Omega translation process files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/otp//

	;; Environment variables to be used for searching Omega
	;; translation process files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = OTPINPUTS

	;; Omega translation process file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .otp

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.opentype fonts]

	;; Search path for OpenType font files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/opentype//

	;; Environment variables to be used for searching OpenType
	;; font files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = OPENTYPEFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; OpenType font file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .otf

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.ovf]

	;; Search path for Omega virtual font files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/ovf//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/vf//

	;; Environment variables to be used for searching Omega
	;; virtual font files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = OVFFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; Omega virtual font file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .ovf

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.ovp]

	;; Search path for Omega virtual property list files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/ovp//

	;; Environment variables to be used for searching Omega
	;; virtual property list files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = OVPFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; Omega virtual property list file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .ovp

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.pdftex config]

	;; Search path for pdfTeX configuration files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/pdftex/{$$progname,}//

	;; Environment variables to be used for searching pdfTeX
	;; configuration files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = PDFTEXCONFIG

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.pk]

	;; Search path for packed bitmap font files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts//

	;; Packed bitmap font file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .pk

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.other binary files]

	;; Search path for program binary files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/$$progname//

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.other text files]

	;; Search path for program text files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/$$progname//

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.PostScript header]

	;; Search path for downloadable PostScript files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/miktex/config//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/dvips//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/pdftex//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/dvipdfm//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/enc//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/type1//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/type42//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/type3//
	${MIKTEX_CONFIG_VALUE_PATHS} = $psfontdirs

	;; Environment variables to be used for searching downloadable
	;; PostScript files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXPSHEADERS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = PSHEADERS

	;; Downloadable PostScript file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .pro
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .enc

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.texmfscripts]

	;; Search path for architecture-independent executables.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/scripts/{$$progname,$$engine,}//

	;; Environment variables to be used for searching
	;; architecture-independent executables.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXMFSCRIPTS

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.subfont definition files]

	;; Search path for subfont definition files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/sfd//

	;; Environment variables to be used for searching subfont
	;; definition files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = SFDFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; Subfont definition file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .sfd

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.tcx]

	;; Search path for TCX files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/miktex/config
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/miktex/web2c

	;; TCX file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .tcx

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.tex]

	;; Search path for TeX input files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/tex/{$$progname,generic,}//

	;; Environment variables to be used for searching TeX input
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXINPUTS

	;; TeX input file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .tex

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.texpool]

	;; Search path for TeX program string files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .

	;; Environment variables to be used for searching METAFONT
	;; program string files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXPOOL
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXMFINI

	;; TeX program string file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .pool

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.TeX system sources]

	;; Search path for source files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/source//

	;; Environment variables to be used for searching source
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXSOURCES

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.TeX system documentation]

	;; Search path for documentation files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/doc/miktex//
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/doc//

	;; Environment variables to be used for searching
	;; documentation files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXDOCS

	;; Documentation file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .pdf
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .html
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .md
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .txt
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .ps
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .dvi

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.tfm]

	;; Search path for TeX font metric files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/tfm//

	;; Environment variables to be used for searching TeX font
	;; metric files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TFMFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; TeX font metric file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .tfm

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.troff fonts]

	;; Environment variables to be used for searching Troff font
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TRFONTS

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.truetype fonts]

	;; Search path for TrueType font files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/truetype//

	;; Environment variables to be used for searching TrueType
	;; font files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TTFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; TrueType font file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .ttf
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .ttc

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.type1 fonts]

	;; Search path for Type1 font files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/type1//

	;; Environment variables to be used for searching Type1 font
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = T1FONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = T1INPUTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXPSHEADERS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = PSHEADERS

	;; Type1 font file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .pfb
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .pfa

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.type42 fonts]

	;; Search path for Type42 font files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/type42//

	;; Environment variables to be used for searching Type42 font
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = T42FONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; Type42 font file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .t42
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .T42

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.vf]

	;; Search path for TeX virtual font files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/fonts/vf//

	;; Environment variables to be used for searching TeX virtual
	;; font files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = VFFONTS
	${MIKTEX_CONFIG_VALUE_ENVVARS} = TEXFONTS

	;; TeX virtual font file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .vf

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.web2c files]

	;; Search path for Web2c files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/web2c//

[${MIKTEX_CONFIG_SECTION_CORE_FILETYPES}.web]

	;; Search path for WEB input files.
	${MIKTEX_CONFIG_VALUE_PATHS} = .
	${MIKTEX_CONFIG_VALUE_PATHS} = %R/web//

	;; Environment variables to be used for searching WEB input
	;; files.
	${MIKTEX_CONFIG_VALUE_ENVVARS} = WEBINPUTS

	;; CWeb file name extensions.
	${MIKTEX_CONFIG_VALUE_EXTENSIONS} = .web

[${MIKTEX_CONFIG_SECTION_MPM}]

	;; Install packages for all users.
	${MIKTEX_CONFIG_VALUE_AUTOADMIN} = ${MPM_AutoAdmin}

	;; Install missing packages automatically (on-the-fly).
	${MIKTEX_CONFIG_VALUE_AUTOINSTALL} = ${MPM_AutoInstall}

[${MIKTEX_CONFIG_SECTION_TEXANDFRIENDS}]

	;; Create auxiliary directory if '--aux-directory=DIR' refers
	;; a non-existing directory.
	${MIKTEX_CONFIG_VALUE_CREATEAUXDIRECTORY} = t

	;; Create the output directory if '--output-directory=DIR'
	;; refers to a non-existing directory.
	${MIKTEX_CONFIG_VALUE_CREATEOUTPUTDIRECTORY} = t

	;; Enable file:line:error style messages.
	${MIKTEX_CONFIG_VALUE_CSTYLEERRORS} = f
