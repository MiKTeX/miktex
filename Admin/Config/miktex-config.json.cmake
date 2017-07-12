[
    {
	"path": "core",
	"desc": "General MiKTeX core configuration settings.",
	"values": [
	    {
		"name": "AllowedShellCommands",
		"default": [
		    "${MIKTEX_PREFIX}bibtex",
		    "${MIKTEX_PREFIX}bibtex8",
		    "${MIKTEX_PREFIX}epstopdf",
		    "${MIKTEX_PREFIX}gregorio",
		    "${MIKTEX_PREFIX}kpsewhich",
		    "${MIKTEX_PREFIX}makeindex",
		    "bibtex",
		    "bibtex8",
		    "epstopdf",
		    "extractbb",
		    "findtexmf",
		    "gregorio",
		    "kpsewhich",
		    "makeindex",
		    "texosquery-jre8"
		],
		"desc": "The programs listed here are as safe as any we know: they either do not write any output files, respect openout_any, or have hard-coded restrictions similar to or higher than openout_any=p.  They also have no features to invoke arbitrary other programs, and no known exploitable bugs.  All to the best of our knowledge.  They also have practical use for being called from TeX."
	    },
            {
		"name": "ShellCommandMode",
		"default": "Restricted",
		"desc": "Shell command mode.\n  Forbidden: don't allow shell commands\n  Restricted: allow the commands listed in AllowedShellCommands\n  Unrestricted: allow all shell commands"
	    }
	]
    },
    {
	"path": "core.ft.tex",
	"desc": "TeX file type.",
	"values": [
	    {
		"name": "name",
		"value": "tex"
	    },
	    {
		"name": "extensions",
		"value": [
		    ".tex"
		]
	    },
	    {
		"name": "path",
		"value": [
		    ".",
		    "%R/tex/{$progname,generic,}//"
		]
	    },
	    {
		"name": "env",
		"value": [
		    "TEXINPUTS"
		]
	    }
	]
    },
    {
	"path": "mpm",
	"desc": "MiKTeX package manager configuration settings.",
	"values": [
	    {
		"name": "AutoAdmin",
		"default": "${MPM_AutoAdmin}",
		"desc": "Install packages for all users."
	    },
	    {
		"name": "AutoInstall",
		"default": "${MPM_AutoInstall}",
		"desc": "Install missing packages automatically (on-the-fly)."
	    }
	]
    },
    {
	"path": "texmf",
	"desc": "Common configuration settings for TeX & Friends.",
	"values": [
	    {
		"name": "buf_size",
		"default": 200000,
		"desc": "TeX uses the buffer to contain input lines, but macro expansion works by writing material into the buffer and reparsing the line.  As a consequence, certain constructs require the buffer to be very large, even though most documents can be handled with a small value."
	    },
	    {
		"name": "error_line",
		"default": 79,
		"desc": "Width of context lines on terminal error messages."
	    },
	    {
		"name": "extra_mem_bot",
		"default": 0,
		"desc": "Extra low memory for boxes, glue, breakpoints, etc."
	    },
	    {
		"name": "extra_mem_top",
		"default": 0,
		"desc": "Extra high memory for chars, tokens, etc."
	    },
	    {
		"name": "half_error_line",
		"default": 50,
		"desc": "Width of first lines of contexts in terminal error messages; should be between 30 and (error_line - 15)."
	    },
	    {
		"name": "main_memory",
		"default": 3000000,
		"desc": "Words of inimemory available."
	    },
	    {
		"name": "max_print_line",
		"default": 79,
		"desc": "Width of longest text lines output; should be at least 60."
	    },
	    {
		"name": "max_strings",
		"default": 500000,
		"desc": "Maximum number of strings."
	    },
	    {
		"name": "param_size",
		"default": 10000,
		"desc": "Maximum number of simultaneous macro parameters."
	    },
	    {
		"name": "pool_free",
		"default": 47500,
		"desc": "Pool space free after format loaded."
	    },
	    {
		"name": "pool_size",
		"default": 3250000,
		"desc": "Max number of characters in all strings, including all error messages, help texts, font names, control sequences.  These values apply to TeX and MP."
	    },
	    {
		"name": "stack_size",
		"default": 5000,
		"desc": "Maximum number of simultaneous input sources."
	    },
	    {
		"name": "strings_free",
		"default": 100,
		"desc": "Strings available after format loaded."
	    },
	    {
		"name": "string_vacancies",
		"default": 90000,
		"desc": "Minimum pool space after TeX/MP's own strings; must be at least 25000 less than pool_size, but doesn't need to be nearly that large."
	    },
	    {
		"name": "CreateAuxDirectory",
		"default": true,
		"desc": "Create auxiliary directory if '--aux-directory=DIR' refers a non-existing directory."
	    },
	    {
		"name": "CreateOutputDirectory",
		"default": true,
		"desc": "Create output directory if '--output-directory=DIR' refers a non-existing directory."
	    }
	]
    },
    {
	"path": "texmf.mf",
	"desc": "Configuration settings for METAFONT.",
	"values": [
            {
		"name": "bistack_size",
		"default": 1500,
		"desc": "Size of stack for bisection algorithms; should probably be left at this value."
	    },
	    {
		"name": "lig_table_size",
		"default": 15000,
		"desc": "Maximum number of ligature/kern steps, must be at least 255 and at most 32510."
	    },
            {
		"name": "max_wiggle",
		"default": 1000,
		"desc": "Number of autorounded points per cycle."
	    },
            {
		"name": "move_size",
		"default": 20000,
		"desc": "Space for storing moves in a single octant."
	    },
	    {
		"name": "path_size",
		"default": 10000,
		"desc": "Maximum number of knots between breakpoints of a path."
	    }
	]
    },
    {
	"path": "texmf.omega",
	"desc": "Configuration settings for Omega.",
	"values": [
	    {
		"name": "ocp_buf_size",
		"default": 500000
	    },
	    {
		"name": "ocp_listinfo_size",
		"default": 1000
	    },
	    {
		"name": "ocp_list_list_size",
		"default": 1000
	    },
	    {
		"name": "ocp_lstack_size",
		"default": 1000
	    },
	    {
		"name": "ocp_stack_size",
		"default": 10000
	    },
	    {
		"name": "trie_op_size",
		"default": 35111
	    }
	]
    },
    {
	"path": "texmf.pdftex",
	"desc": "Configuration settings for pdfTeX.",
	"values": [
	    {
		"name": "pdf_mem_size",
		"default": 10000
	    },
	    {
		"name": "obj_tab_size",
		"default": 1000
	    },
	    {
		"name": "dest_names_size",
		"default": 131072
	    },
	    {
		"name": "pdf_os_buf_size",
		"default": 1
	    }
	]
    },
    {
	"path": "texmf.tex",
	"desc": "Common configuration settings for TeX engines.",
	"values": [
	    {
		"name": "max_in_open",
		"default": 50,
		"desc": "Maximum number of input files and error insertions that can be going on simultaneously."
	    },
	    {
		"name": "nest_size",
		"default": 500,
		"desc": "Maximum number of semantic levels simultaneously active."
	    },
	    {
		"name": "save_size",
		"default": 50000,
		"desc": "Space for saving values outside current group."
	    },
	    {
		"name": "trie_size",
		"default": 700000,
		"desc": "Space for hyphenation patterns."
	    },
	    {
		"name": "font_max",
		"default": 9000,
		"desc": "Total number of fonts."
	    },
	    {
		"name": "font_mem_size",
		"default": 3000000,
		"desc": "Words of font info for TeX (total size of all TFM files, approximately)."
	    },
	    {
		"name": "hash_extra",
		"default": 200000,
		"desc": "Extra space for the hash table of control sequences (which allows 10K names as distributed)."
	    },
	    {
		"name": "hyph_size",
		"default": 8191,
		"desc": "Prime number of hyphenation exceptions."
	    },
	    {
		"name": "dvi_buf_size",
		"default": 8192,
		"desc": "Size of the output buffer; must be a multiple of 8."
	    }
	]
    }
]
