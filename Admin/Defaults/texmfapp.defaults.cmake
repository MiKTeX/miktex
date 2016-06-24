;; TeX uses the buffer to contain input lines, but macro
;; expansion works by writing material into the buffer and reparsing the
;; line.  As a consequence, certain constructs require the buffer to be
;; very large, even though most documents can be handled with a small value.
buf_size=200000

;; Width of context lines on terminal error messages.
error_line=79

;; Extra low memory for boxes, glue, breakpoints, etc.
extra_mem_bot=0

;; Extra high memory for chars, tokens, etc.
extra_mem_top=0

;; Width of first lines of contexts in terminal error messages;
;; should be between 30 and (error_line - 15).
half_error_line=50

;; Words of inimemory available.
main_memory=3000000

;; Width of longest text lines output; should be at least 60.
max_print_line=79

;; Maximum number of strings.
max_strings=500000

;; Maximum number of simultaneous macro parameters.
param_size=10000

;; Pool space free after format loaded.
pool_free=47500

;; Max number of characters in all strings, including all
;; error messages, help texts, font names, control sequences.
;; These values apply to TeX and MP.
pool_size=3250000

;; Maximum number of simultaneous input sources.
stack_size=5000

;; Strings available after format loaded.
strings_free=100

;; Minimum pool space after TeX/MP's own strings; must be at least
;; 25000 less than pool_size, but doesn't need to be nearly that large.
string_vacancies=90000

;; Create auxiliary directory if '--aux-directory=DIR' refers
;; a non-existing directory.
CreateAuxDirectory=t

;; Create output directory if '--output-directory=DIR' refers
;; a non-existing directory.
CreateOutputDirectory=t
