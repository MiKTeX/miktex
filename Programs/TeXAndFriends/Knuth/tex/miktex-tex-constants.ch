%%% miktex-tex-constants.ch:
%%%
%%% Derived from:
%%% tex.ch for C compilation with web2c, derived from various other
%%% change files.  By Tim Morgan, UC Irvine ICS Department, and many
%%% others.

% _____________________________________________________________________________
%
% [1.11]
% _____________________________________________________________________________

@x
@<Constants...@>=
@!mem_max=30000; {greatest index in \TeX's internal |mem| array;
  must be strictly less than |max_halfword|;
  must be equal to |mem_top| in \.{INITEX}, otherwise |>=mem_top|}
@!mem_min=0; {smallest index in \TeX's internal |mem| array;
  must be |min_halfword| or more;
  must be equal to |mem_bot| in \.{INITEX}, otherwise |<=mem_bot|}
@!buf_size=500; {maximum number of characters simultaneously present in
  current lines of open files and in control sequences between
  \.{\\csname} and \.{\\endcsname}; must not exceed |max_halfword|}
@!error_line=72; {width of context lines on terminal error messages}
@!half_error_line=42; {width of first lines of contexts in terminal
  error messages; should be between 30 and |error_line-15|}
@!max_print_line=79; {width of longest text lines output; should be at least 60}
@!stack_size=200; {maximum number of simultaneous input sources}
@!max_in_open=6; {maximum number of input files and error insertions that
  can be going on simultaneously}
@!font_max=75; {maximum internal font number; must not exceed |max_quarterword|
  and must be at most |font_base+256|}
@!font_mem_size=20000; {number of words of |font_info| for all fonts}
@!param_size=60; {maximum number of simultaneous macro parameters}
@!nest_size=40; {maximum number of semantic levels simultaneously active}
@!max_strings=3000; {maximum number of strings; must not exceed |max_halfword|}
@!string_vacancies=8000; {the minimum number of characters that should be
  available for the user's control sequences and font names,
  after \TeX's own error messages are stored}
@!pool_size=32000; {maximum number of characters in strings, including all
  error messages and help texts, and the names of all fonts and
  control sequences; must exceed |string_vacancies| by the total
  length of \TeX's own strings, which is currently about 23000}
@!save_size=600; {space for saving values outside of current group; must be
  at most |max_halfword|}
@!trie_size=8000; {space for hyphenation patterns; should be larger for
  \.{INITEX} than it is in production versions of \TeX}
@!trie_op_size=500; {space for ``opcodes'' in the hyphenation patterns}
@!dvi_buf_size=800; {size of the output buffer; must be a multiple of 8}
@!file_name_size=40; {file names shouldn't be longer than this}
@!pool_name='TeXformats:TEX.POOL                     ';
  {string of length |file_name_size|; tells where the string pool appears}
@y
@d file_name_size == maxint
@d ssup_error_line = 255
@d ssup_max_strings == 2097151
{Larger values than 65536 cause the arrays to consume much more memory.}
@d ssup_trie_opcode == 65535
@d ssup_trie_size == @"3FFFFF

@d ssup_hyph_size == 65535 {Changing this requires changing (un)dumping!}
@d iinf_hyphen_size == 610 {Must be not less than |hyph_prime|!}

@d max_font_max=9000 {maximum number of internal fonts; this can be
                      increased, but |hash_size+max_font_max|
                      should not exceed 29000.}
@d font_base=0 {smallest internal font number; must be
                |>= min_quarterword|; do not change this without
                modifying the dynamic definition of the font arrays.}


@<Constants...@>=
@!hash_offset=514; {smallest index in hash array, i.e., |hash_base| }
  {Use |hash_offset=0| for compilers which cannot decrement pointers.}
@!trie_op_size=35111; {space for ``opcodes'' in the hyphenation patterns;
  best if relatively prime to 313, 361, and 1009.}
@!neg_trie_op_size=-35111; {for lower |trie_op_hash| array bound;
  must be equal to |-trie_op_size|.}
@!min_trie_op=0; {first possible trie op code for any language}
@!max_trie_op=ssup_trie_opcode; {largest possible trie opcode for any language}
@!pool_name=TEXMF_POOL_NAME; {this is configurable, for the sake of ML-\TeX}
  {string of length |file_name_size|; tells where the string pool appears}
@!engine_name=TEXMF_ENGINE_NAME; {the name of this engine}
@#
@!inf_mem_bot = 0;
@!sup_mem_bot = 1;

@!inf_main_memory = 3000;
@!sup_main_memory = 256000000;

@!inf_trie_size = 8000;
@!sup_trie_size = ssup_trie_size;

@!inf_max_strings = 3000;
@!sup_max_strings = ssup_max_strings;
@!inf_strings_free = 100;
@!sup_strings_free = sup_max_strings;

@!inf_buf_size = 500;
@!sup_buf_size = 30000000;

@!inf_nest_size = 40;
@!sup_nest_size = 4000;

@!inf_max_in_open = 6;
@!sup_max_in_open = 127;

@!inf_param_size = 60;
@!sup_param_size = 32767;

@!inf_save_size = 600;
@!sup_save_size = 30000000;

@!inf_stack_size = 200;
@!sup_stack_size = 30000;

@!inf_dvi_buf_size = 800;
@!sup_dvi_buf_size = 65536;
@!dvi_buf_size = 8192; {TODO: eliminate}

@!inf_font_mem_size = 20000;
@!sup_font_mem_size = 147483647; {|integer|-limited, so 2 could be prepended?}

@!sup_font_max = max_font_max;
@!inf_font_max = 50; {could be smaller, but why?}

@!inf_pool_size = 32000;
@!sup_pool_size = 40000000;
@!inf_pool_free = 1000;
@!sup_pool_free = sup_pool_size;
@!inf_string_vacancies = 8000;
@!sup_string_vacancies = 39977000; {|sup_pool_size - 23000|}

@!sup_hash_extra = sup_max_strings;
@!inf_hash_extra = 0;

@!sup_hyph_size = ssup_hyph_size;
@!inf_hyph_size = iinf_hyphen_size; {Must be not less than |hyph_prime|!}

@!inf_expand_depth = 10;
@!sup_expand_depth = 10000000;
@z

% _____________________________________________________________________________
%
% [1.12]
% _____________________________________________________________________________

@x
@d mem_bot=0 {smallest index in the |mem| array dumped by \.{INITEX};
  must not be less than |mem_min|}
@d mem_top==30000 {largest index in the |mem| array dumped by \.{INITEX};
  must be substantially larger than |mem_bot|
  and not greater than |mem_max|}
@d font_base=0 {smallest internal font number; must not be less
  than |min_quarterword|}
@d hash_size=2100 {maximum number of control sequences; it should be at most
  about |(mem_max-mem_min)/10|}
@d hash_prime=1777 {a prime number equal to about 85\pct! of |hash_size|}
@d hyph_size=307 {another prime; the number of \.{\\hyphenation} exceptions}
@y
@d hash_size=15000 {maximum number of control sequences; it should be at most
  about |(mem_max-mem_min)/10|; see also |font_max|}
@d hash_prime=8501 {a prime number equal to about 85\pct! of |hash_size|}
@d hyph_prime=607 {another prime for hashing \.{\\hyphenation} exceptions;
                if you change this, you should also change |iinf_hyphen_size|.}
@z
