/* 
 Copyright (c) 2008-2017 jerome DOT laurens AT u-bourgogne DOT fr
 
 This file is part of the SyncTeX package.
 
 Version 1
 Latest Revision: Tue Apr 25 14:46:32 UTC 2017
 
 License:
 --------
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE
 
 Except as contained in this notice, the name of the copyright holder  
 shall not be used in advertising or otherwise to promote the sale,  
 use or other dealings in this Software without prior written  
 authorization from the copyright holder.
 
 Important notice:
 -----------------
 This file is named "synctex.c", it may or may not have a header counterpart
 depending on its use.  It aims to provide basic components useful for the
 input/output synchronization technology for TeX.
 The purpose of the implementation is threefold
 - firstly, it defines a new input/output synchronization technology named
 "synchronize texnology", "SyncTeX" or "synctex"
 - secondly, it defines the naming convention and format of the auxiliary file
 used by this technology
 - thirdly, it defines the API of a controller and a controller, used in
 particular by the pdfTeX and XeTeX programs to prepare synchronization.
 
 All these are up to a great extent de facto definitions, which means that they
 are partly defined by the implementation itself.
 
 This technology was first designed for pdfTeX, an extension of TeX managing the
 pdf output file format, but it can certainly be adapted to other programs built
 from TeX as long as the extensions do not break too much the core design.
 Moreover, the synchronize texnology only relies on code concept and not
 implementation details, so it can be ported to other TeX systems.  In order to
 support SyncTeX, one can start reading the dedicated section in synctex.ch,
 sync-pdftex.ch and sync-xetex.ch. Actually, support is provided for TeX, e-TeX,
 pdfTeX and XeTeX.
 
 Other existing public synchronization technologies are defined by srcltx.sty -
 also used by source specials - and pdfsync.sty.  Like them, the synchronize
 texnology is meant to be shared by various text editors, viewers and TeX
 engines.  A centralized reference and source of information is available in TeX-Live.
 
 Versioning:
 -----------
 As synctex is embedded into different TeX implementation, there is an independent
 versionning system.
 For TeX implementations, the actual version is: 3
 For .synctex file format, the actual version is SYNCTEX_VERSION below
 
 Please, do not remove these explanations.
 
 Acknowledgments:
 ----------------
 The author received useful remarks from the pdfTeX developers, especially Hahn The Thanh,
 and significant help from XeTeX developer Jonathan Kew
 
 Nota Bene:
 ----------
 If you include or use a significant part of the synctex package into a software,
 I would appreciate to be listed as contributor and see "SyncTeX" highlighted.
 
 History:
 --------
 Version 1.14
 Fri Apr 15 19:10:57 UTC 2011
 - taking output_directory into account
 - Replaced FOPEN_WBIN_MODE by FOPEN_W_MODE when opening the text version of the .synctex file.
 - Merging with LuaTeX's version of synctex.c
 
 Version 3
 - very minor design change to take luatex into account
 - typo fixed
 - some size_t replaced by int
 - very minor code design change to remove wrong xetex specific warnings
 
 Version 2
 Fri Sep 19 14:55:31 UTC 2008
 - support for file names containing spaces.
 This is one thing that xetex and pdftex do not manage the same way.
 When the input file name contains a space character ' ',
 pdftex will automatically enclose this name between two quote characters '"',
 making programs believe that these quotes are really part of the name.
 xetex does nothing special.
 For that reason, running the command line
 xetex --synctex=-1 "my file.tex"
 is producing the expected file named <my file.synctex>, (the '<' and '>' are not part of the name)
 whereas running the command line
 pdftex --synctex=-1 "my file.tex"
 was producing the unexpected file named <"my file".synctex> where the two '"' characters were part of the name.
 Of course, that was breaking the typesetting mechanism when pdftex was involved.
 To solve this problem, we prefer to rely on the output_file_name instead of the jobname.
 In the case when no output_file_name is available, we use jobname and test if the file name
 starts and ends with a quote character. Every synctex output file is removed because we consider
 TeX encontered a problem.
 There is some conditional coding.
 
 Version 1
 Latest Revision: Sat Apr 22 09:10:30 UTC 2017
 
 */

#   define SYNCTEX_VERSION 1

#   define SYNCTEX_DEBUG 0

/*  Debugging: define the next macro to "return;" in order to disable the synctex code
 *  only suplemental function calls will be used. The compiler may optimize them. */
#   define SYNCTEX_RETURN_IF_DISABLED ;

#   define SYNCTEX_NOERR 0

#   define EXTERN extern

#   ifdef xfree
#       define SYNCTEX_FREE xfree
#   else
#       define SYNCTEX_FREE(x) free(x)
#   endif

/*  The header file SYNCTEX_ENGINE_H below is "synctex-tex.h" for TeX, ...
 *  Some macros are defined and additional headers will be imported.
 *  The macros below can be defined there, prior to their default definition given afterwards. */
#   include SYNCTEX_ENGINE_H

/*  the macros defined below do the same job than their almost eponym
 *  counterparts of *tex.web, the memory access is sometimes more direct
 *  because *tex.web won't share its own constants the main purpose is to
 *  maintain very few hook points into *tex.web in order both to ensure
 *  portability and not modifying to much the original code.  see texmfmem.h
 *  and *tex.web for details, the synctex_ prefix prevents name conflicts, it
 *  is some kind of namespace
 */
/*  synctexoption is a global integer variable defined in *tex.web
 *  it is set to 1 by texmfmp.c if the command line has the '-synctex=1'
 *  option.  */
#   if !defined(synctex_options)
#       define synctex_options synctexoption
#   endif
#   if !defined(SYNCTEX_NO_OPTION)
#       define SYNCTEX_NO_OPTION INT_MAX
#   endif
/*  if synctex_options is set to SYNCTEX_NO_OPTION, no command line option was provided.  */

/*  glue code: really define the main memory,
 *  this is exactly the same "mem" as in *tex.web.  */
#   if !defined(mem)
#       define mem zmem
#   endif
/*  glue code: synctexoffset is a global integer variable defined in *tex.web
 *  it is set to the offset where the primitive \synctex reads and writes its
 *  value.  */
#   if !defined(SYNCTEX_VALUE)
#if defined(MIKTEX)
#       define SYNCTEX_VALUE eqtb[synctexoffset].c4p_P2.c4p_int
#else
#       define SYNCTEX_VALUE zeqtb[synctexoffset].cint
#endif
#   endif
/*  if there were a mean to share the value of synctex_code between *tex.web
 *  and this file, it would be great.  */

/*  WARNING:
 The 9 definitions below must be in sync with their eponym declarations in
 the proper synctex-*.ch* file or equivalent.
 Since version 1.14, the definitions are moved after the include directive above
 and we adopted a conservative policy. The forthcoming definitions apply only if
 when the macros are not already defined in SYNCTEX_ENGINE_H.
 If the default values below do not fit with your requirements,
 you can define them in the above mentionned header file.
 */
#   if !defined(synchronization_field_size)
#       define synchronization_field_size 2
#   endif
/*  The default value is 2, it is suitable for original TeX and alike,
 *  but it is too big for XeTeX.
 *  The tag and the line are just the two last words of the node.  This is a
 *  very handy design but this is not strictly required by the concept.  If
 *  really necessary, one can define other storage rules.
 *  XeTeX already defined synchronization_field_size,
 *  SYNCTEX_TAG_MODEL and SYNCTEX_LINE_MODEL
 *  All the default values are targeted to TeX or e-TeX.  */
#   if !defined(SYNCTEX_TAG_MODEL)
#     if defined(MIKTEX)
#       define SYNCTEX_TAG_MODEL(NODE,TYPE)\
mem[NODE+TYPE##_node_size-synchronization_field_size].c4p_P2.c4p_int
#     else
#       define SYNCTEX_TAG_MODEL(NODE,TYPE)\
mem[NODE+TYPE##_node_size-synchronization_field_size].cint
#     endif
#   endif
#   if !defined(SYNCTEX_LINE_MODEL)
#     if defined(MIKTEX)
#       define SYNCTEX_LINE_MODEL(NODE,TYPE)\
mem[NODE+TYPE##_node_size-synchronization_field_size+1].c4p_P2.c4p_int
#     else
#       define SYNCTEX_LINE_MODEL(NODE,TYPE)\
mem[NODE+TYPE##_node_size-synchronization_field_size+1].cint
#     endif
#   endif
/*  SYNCTEX_TAG_MODEL and SYNCTEX_LINE_MODEL are used to define
 *  SYNCTEX_TAG and SYNCTEX_LINE in a model independant way
 *  Both are tag and line accessors.
 *  TYPE takes one of the prefixes in the ???_node_size definition below. */
/*  see: @d box_node_size=...  
 *  There should be an automatic process here because these definitions
 *  are redundant. However, this process would certainly be overcomplicated
 *  (building then parsing the *tex.web file would be a pain) */
#   if !defined(box_node_size)
#       define box_node_size (7+synchronization_field_size)
#   endif
/*  glue code: node sizes  */
#   if !defined(small_node_size)
#       define small_node_size 2
#   endif
/*  see: @d small_node_size=2 {number of words to allocate for most node types}  */
#   if !defined(medium_node_size)
#       define medium_node_size (small_node_size+synchronization_field_size)
#   endif
/*  see: @d rule_node_size=4  */
#   if !defined(rule_node_size)
#       define rule_node_size (4+synchronization_field_size)
#   endif
/*  see: luatex  */
#   if !defined(glue_node_size)
#       define glue_node_size medium_node_size
#   endif
#   if !defined(kern_node_size)
#       define kern_node_size medium_node_size
#   endif
#   if !defined(math_node_size)
#       define math_node_size medium_node_size
#   endif
#   if !defined(width_offset)
#       define width_offset 1
#   endif
/*  see: @d width_offset=...  */
#   if !defined( depth_offset)
#       define depth_offset 2
#   endif
/*  see: @d depth_offset=...  */
#   if !defined(height_offset)
#       define height_offset 3
#   endif
/*  see: @d height_offset=...  */

/*  Now define the local version of width(##), height(##) and depth(##) macros
 These only depend on the 3 macros above.  */
#   if !defined(SYNCTEX_TYPE)
#     if defined(MIKTEX)
#       define SYNCTEX_TYPE(NODE) mem[NODE].c4p_P2.hh.c4p_P1.c4p_P0.b0
#     else
#       define SYNCTEX_TYPE(NODE) mem[NODE].hh.b0
#     endif
#   endif
#   if !defined(SYNCTEX_SUBTYPE)
#     if defined(MIKTEX)
#       define SYNCTEX_SUBTYPE(NODE) mem[NODE].c4p_P2.hh.c4p_P1.c4p_P0.b1
#     else
#       define SYNCTEX_SUBTYPE(NODE) mem[NODE].hh.b1
#     endif
#   endif
#   if !defined(SYNCTEX_WIDTH)
#     if defined(MIKTEX)
#       define SYNCTEX_WIDTH(NODE) mem[NODE+width_offset].c4p_P2.c4p_int
#     else
#       define SYNCTEX_WIDTH(NODE) mem[NODE+width_offset].cint
#     endif
#   endif
#   if !defined(SYNCTEX_DEPTH)
#     if defined(MIKTEX)
#       define SYNCTEX_DEPTH(NODE) mem[NODE+depth_offset].c4p_P2.c4p_int
#     else
#       define SYNCTEX_DEPTH(NODE) mem[NODE+depth_offset].cint
#     endif
#   endif
#   if !defined(SYNCTEX_HEIGHT)
#     if defined(MIKTEX)
#       define SYNCTEX_HEIGHT(NODE) mem[NODE+height_offset].c4p_P2.c4p_int
#     else
#       define SYNCTEX_HEIGHT(NODE) mem[NODE+height_offset].cint
#     endif
#   endif
#   if !defined(rule_node)
#       define rule_node 2
#   endif
#   if !defined(glue_node)
#       define glue_node 10
#   endif
#   if !defined(kern_node)
#       define kern_node 11
#   endif

/*  Some parts of the code may differ depending on the ouput mode,
 *  dvi or xdv vs pdf, in particular the management of magnification.
 *  The default is dvi mode.
 *  Also, if pdftex is used, the origin of the coordinates is at 0, not at 1in
 *  Default values are suitable for TeX  */
#   if !defined(SYNCTEX_OUTPUT)
#       define SYNCTEX_OUTPUT "dvi"
#   endif
#   if !defined(SYNCTEX_OFFSET_IS_PDF)
#       define SYNCTEX_OFFSET_IS_PDF 0
#   endif

#if defined(MIKTEX)
#if defined(__cplusplus)
#include <miktex/Core/File>
#include <miktex/Core/PathName>
#endif
#endif
#if !defined(MIKTEX)
#if defined(_WIN32) && (defined(pdfTeX) || defined(upTeX) || defined(eupTeX) || defined(XeTeX))
#define W32UPTEXSYNCTEX 1
#include <wchar.h>
static char *chgto_oem(char *src);
static int fsyscp_remove(char *name);
#define remove fsyscp_remove
#endif
#endif

/*  This macro layer was added to take luatex into account as suggested by T. Hoekwater. */
#   if !defined(SYNCTEX_GET_JOB_NAME)
#       define SYNCTEX_GET_JOB_NAME() (gettexstring(jobname))
#   endif
#   if !defined(SYNCTEX_GET_LOG_NAME)
#       define SYNCTEX_GET_LOG_NAME() (gettexstring(texmflogname))
#   endif
#   if !defined(SYNCTEX_CURRENT_TAG)
#       define SYNCTEX_CURRENT_TAG (curinput.synctextagfield)
#   endif
#   if !defined(SYNCTEX_GET_CURRENT_NAME)
#       define SYNCTEX_GET_CURRENT_NAME() generic_synctex_get_current_name()
#   endif
#   if !defined(SYNCTEX_GET_TOTAL_PAGES)
#       define SYNCTEX_GET_TOTAL_PAGES() (totalpages)
#   endif
#   if !defined(SYNCTEX_CURH)
#       define SYNCTEX_CURH curh
#   endif
#   if !defined(SYNCTEX_CURV)
#       define SYNCTEX_CURV curv
#   endif
#   if !defined(SYNCTEX_RULE_WD)
#       define SYNCTEX_RULE_WD rulewd
#   endif
#   if !defined(SYNCTEX_RULE_HT)
#       define SYNCTEX_RULE_HT ruleht
#   endif
#   if !defined(SYNCTEX_RULE_DP)
#       define SYNCTEX_RULE_DP ruledp
#   endif

/*  For non-GCC compilation.  */
#   if !defined(__GNUC__) || (__GNUC__ < 2)
#       define __attribute__(A)
#   endif

#   include "synctex.h"

#   if !defined(SYNCTEX_SUPPORT_PDF_FORM)
#       define SYNCTEX_SUPPORT_PDF_FORM 0
#   endif

#   define SYNCTEX_YES (1)
#   define SYNCTEX_NO  (0)
#   define SYNCTEX_NO_ERROR  (0)

#   define SYNCTEX_UNIT_FACTOR 1
#   define UNIT / synctex_ctxt.unit
/*  UNIT is the scale. TeX coordinates are very accurate and client won't need
 *  that, at leat in a first step.  1.0 <-> 2^16 = 65536. 
 *  The TeX unit is sp (scaled point) or pt/65536 which means that the scale
 *  factor to retrieve a bp unit (a postscript) is 72/72.27/65536 =
 *  1/4096/16.06 = 1/8192/8.03
 *  Here we use 1/SYNCTEX_UNIT_FACTOR as scale factor, then we can limit ourselves to
 *  integers. This default value assumes that TeX magnification factor is 1000.
 *  The real TeX magnification factor is used to fine tune the synctex context
 *  scale in the synctex_dot_open function.
 *  IMPORTANT: We can say that the natural unit of .synctex files is SYNCTEX_UNIT_FACTOR sp.
 *  To retrieve the proper bp unit, we'll have to divide by 8.03.  To reduce
 *  rounding errors, we'll certainly have to add 0.5 for non negative integers
 *  and ±0.5 for negative integers.  This trick is mainly to gain speed and
 *  size. A binary file would be more appropriate in that respect, but I guess
 *  that some clients like auctex would not like it very much.  we cannot use
 *  "<<13" instead of "/SYNCTEX_UNIT_FACTOR" because the integers are signed and we do not
 *  want the sign bit to be propagated.  The origin of the coordinates is at
 *  the top left corner of the page.  For pdf mode, it is straightforward, but
 *  for dvi mode, we'll have to record the 1in offset in both directions,
 *  eventually modified by the magnification.
 */

#   if defined(__SyncTeX__)

#   include <stdio.h>
#   include <stdarg.h>
#   include "zlib.h"

#if defined(MIKTEX)
#  include <miktex/W2C/Emulation.h> /* output_directory */
#endif
typedef void (*synctex_recorder_t) (halfword);  /* recorders know how to record a node */
typedef int (*synctex_fprintf_t) (void *, const char *, ...);   /* print formatted to either FILE * or gzFile */

#   define SYNCTEX_BITS_PER_BYTE 8

/*  Here are all the local variables gathered in one "synchronization context"  */
static struct {
    void *file;                 /*  the foo.synctex or foo.synctex.gz I/O identifier  */
    synctex_fprintf_t fprintf;  /*  either fprintf or gzprintf */
    char *busy_name;            /*  the real "foo.synctex(busy)" or "foo.synctex.gz(busy)" name, with output_directory  */
    char *root_name;            /*  in general jobname.tex  */
    integer count;              /*  The number of interesting records in "foo.synctex"  */
    /*  next concern the last sync record encountered  */
    halfword node;              /*  the last synchronized node, must be set 
                                 *  before the recorder */
    synctex_recorder_t recorder;/*  the recorder of the node above, the
                                 *  routine that knows how to record the 
                                 *  node to the .synctex file */
    integer tag, line;          /*  current tag and line  */
    integer curh, curv;         /*  current point  */
    integer magnification;      /*  The magnification as given by \mag */
    integer unit;               /*  The unit, defaults to 1, use 8192 to produce shorter but less accurate info */
    integer total_length;       /*  The total length of the bytes written since    the last check point  */
    integer options;            /* unsigned options */
    integer lastv;              /* compression trick if
                                 |synctex_options&4|>0.  */
    integer form_depth;        /*  pdf forms are an example of nested sheets */
    struct _flags {
        unsigned int option_read:1; /*  Command line option read (in case of problem or at the end) */
        unsigned int content_ready:1; /*  Command line option read (in case of problem or at the end) */
        unsigned int off:1;         /*  Definitely turn off synctex, corresponds to cli option -synctex=0 */
        unsigned int no_gz:1;       /*  Whether zlib is used or not */
        unsigned int not_void:1;    /*  Whether it really contains synchronization material */
        unsigned int warn:1;        /*  One shot warning flag */
        unsigned int quoted:1;      /*  Whether the input file name was quoted by tex or not, for example "\"my input file.tex\"", unused by XeTeX */
        unsigned int output_p:1;    /*  Whether the output_directory is used */
        unsigned int reserved:SYNCTEX_BITS_PER_BYTE*sizeof(int)-8; /* Align */
    } flags;
} synctex_ctxt = {
    NULL, NULL, NULL, NULL, 0, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, {0,0,0,0,0,0,0,0,0}}; /* last_v_recorded is initialized to -1. */

#   define SYNCTEX_FILE synctex_ctxt.file
#   define SYNCTEX_CONTENT_READY (synctex_ctxt.flags.content_ready)
#   define SYNCTEX_IS_OFF (synctex_ctxt.flags.off)
#   define SYNCTEX_NO_GZ (synctex_ctxt.flags.no_gz)
#   define SYNCTEX_NOT_VOID (synctex_ctxt.flags.not_void)
#   define SYNCTEX_WARNING_DISABLE (synctex_ctxt.flags.warn)
#   define SYNCTEX_fprintf (*synctex_ctxt.fprintf)

#   define SYNCTEX_IS_READY (synctex_ctxt.flags.ready)

#   define SYNCTEX_NO_GZ_AUX_NAME\
    (SYNCTEX_NO_GZ||((synctex_ctxt.options)&2)!=0)
#   define SYNCTEX_WITH_FORMS (((synctex_ctxt.options)&4)!=0)
#   define SYNCTEX_H_COMPRESS (((synctex_ctxt.options)&8)!=0)

static inline void _synctex_read_command_line_option(void) {
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: _synctex_read_command_line_option\n");
#   endif
    /*  This is a one shot function, any subsequent call is void */
    if (synctex_ctxt.flags.option_read) {
#   if SYNCTEX_DEBUG
        printf("\nSynchronize DEBUG: skipped\n");
#   endif
        return;
    }
    if (SYNCTEX_NO_OPTION == synctex_options) {
        /*  No option given from the command line  */
        SYNCTEX_VALUE = 0;
    } else if (synctex_options == 0) {
        /*  -synctex=0 was given: SyncTeX must be definitely disabled,
         *  any subsequent \synctex=N will have no effect at all */
        SYNCTEX_IS_OFF = SYNCTEX_YES;
        SYNCTEX_VALUE = 0;
    } else {
        /*  the command line options are not ignored  */
        if (synctex_options < 0) {
            SYNCTEX_NO_GZ = SYNCTEX_YES;
            synctex_ctxt.options = -synctex_options;
        } else {
            SYNCTEX_NO_GZ = SYNCTEX_NO;
            synctex_ctxt.options =  synctex_options;
        }
        synctex_options|=1; /* Do it when positive */
        /*  Initialize the content of the \synctex primitive */
        SYNCTEX_VALUE = synctex_options;
    }
    synctex_ctxt.flags.option_read = SYNCTEX_YES;
    return;
}
/*  Initialize the options, synchronize the variables.
 *  This is sent by *tex.web before any TeX macro is used.
 *  */
void synctexinitcommand(void)
{
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexinitcommand\n");
#   endif
    _synctex_read_command_line_option();
    return;
}

/*  Free all memory used, close and remove the file if any,
 *  It is sent locally when there is a problem with synctex output.
 *  It is sent by pdftex when a fatal error occurred in pdftex.web. */
void synctexabort(boolean log_opened __attribute__ ((unused)))
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctex_abort\n");
#   endif
    if (SYNCTEX_FILE) {
        if (SYNCTEX_NO_GZ) {
            xfclose((FILE *) SYNCTEX_FILE, synctex_ctxt.busy_name);
        } else {
            gzclose((gzFile) SYNCTEX_FILE);
        }
        SYNCTEX_FILE = NULL;
        remove(synctex_ctxt.busy_name);
        SYNCTEX_FREE(synctex_ctxt.busy_name);
        synctex_ctxt.busy_name = NULL;
    }
    if (NULL != synctex_ctxt.root_name) {
        SYNCTEX_FREE(synctex_ctxt.root_name);
        synctex_ctxt.root_name = NULL;
    }
    SYNCTEX_IS_OFF = SYNCTEX_YES;      /* disable synctex */
}

static inline int synctex_record_preamble(void);
static inline int synctex_record_input(integer tag, char *name);

static const char *synctex_suffix = ".synctex";
static const char *synctex_suffix_gz = ".gz";
static const char *synctex_suffix_busy = "(busy)";

/*  for DIR_SEP_STRING */
#   include <kpathsea/c-pathch.h>
/*  for kpse_absolute_p */
#   include <kpathsea/absolute.h>

#ifdef W32UPTEXSYNCTEX
static char *chgto_oem(char *src)
{
  wchar_t *sw = NULL;
  char    *dst = NULL;
  static int f_codepage = 0;

  if(f_codepage == 0) {
    f_codepage = AreFileApisANSI() ? GetACP() : GetOEMCP();
  }

  if(f_codepage == file_system_codepage) {
    dst = xstrdup(src);
    return dst;
  }

  sw = get_wstring_from_mbstring(file_system_codepage, src, sw);
  dst = get_mbstring_from_wstring(f_codepage, sw, dst);
  if(sw) free(sw);
  return dst;
}

static gzFile fsyscp_gzopen(const char *path, const char *mode)
{
  gzFile  gzf;
  wchar_t *pathw = NULL;
  pathw = get_wstring_from_fsyscp(path, pathw);
  gzf = gzopen_w(pathw, mode);
  free(pathw);
  return gzf;
}

static int fsyscp_remove(char *s)
{
  wchar_t *sw = NULL;
  int ret;
  sw = get_wstring_from_fsyscp(s, sw);
  ret = _wremove(sw);
  if(sw) free(sw);
  return ret;
}

static int fsyscp_rename(char *s1, char *s2)
{
  wchar_t *sw1 = NULL, *sw2 = NULL;
  int ret;

  sw1 = get_wstring_from_fsyscp(s1, sw1);
  sw2 = get_wstring_from_fsyscp(s2, sw2);
  ret = _wrename(sw1, sw2);
  if(sw1) free(sw1);
  if(sw2) free(sw2);
  return ret;
}

#undef fopen
#define fopen fsyscp_fopen
#define gzopen fsyscp_gzopen
#define rename fsyscp_rename
#endif

/*  synctex_dot_open ensures that the foo.synctex file is open.
 *  In case of problem, it definitely disables synchronization.
 *  Now all the output synchronization info is gathered in only one file.
 *  It is possible to split this info into as many different output files as sheets
 *  plus 1 for the control but the overall benefits are not so clear.
 *  For example foo-i.synctex would contain input synchronization
 *  information for page i alone.
 *  Options given through \synctex=Number are managed here too.
 */
static void *synctex_dot_open(void)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nwarning: Synchronize DEBUG: synctex_dot_open\n");
    printf("\nwarning: SYNCTEX_VALUE=%0X\n", SYNCTEX_VALUE);
    printf("\nwarning: synctex_options=%0X\n", synctex_ctxt.options);
#   endif
    if (SYNCTEX_IS_OFF || !SYNCTEX_VALUE) {
        return NULL;            /*  synchronization is disabled: do nothing  */
    }
    if (SYNCTEX_FILE) {
        return SYNCTEX_FILE;    /*  synchronization is already enabled  */
    }
    _synctex_read_command_line_option();
#   if SYNCTEX_DEBUG
    printf("\nwarning: Synchronize DEBUG: synctex_dot_open 1\n");
#   endif
    /*  this is the first time we are asked to open the file
     this part of code is executed only once:
     either SYNCTEX_FILE is nonnegative or synchronization is
     definitely disabled. */
    {
        char *tmp = SYNCTEX_GET_JOB_NAME();
        size_t len = strlen(tmp);
        if (len>0) {
            /*  jobname was set by the \jobname command on the *TeX side  */
#if defined(MIKTEX)
            /* C++: typecast needed */
            char *the_busy_name = (char*)xmalloc((size_t)
                ( len
                 + strlen(synctex_suffix)
                 + strlen(synctex_suffix_busy)
                 + 1
                 + (output_directory?strlen(output_directory) + strlen(DIR_SEP_STRING):0)));
#else
            char *the_busy_name = xmalloc((size_t)
                ( len
                 + strlen(synctex_suffix)
                 + strlen(synctex_suffix_busy)
                 + 1
                 + (output_directory?strlen(output_directory) + strlen(DIR_SEP_STRING):0)));
#endif
            if (!the_busy_name) {
                SYNCTEX_FREE(tmp);
                tmp = NULL;
                synctexabort(0);
                return NULL;
            }
            /* Initialize the_busy_name to the void string */
            the_busy_name[0] = (char)0;
            /* If an output directory was specified, use it instead of cwd.  */
            if (output_directory && !kpse_absolute_p(tmp, false)) {
                synctex_ctxt.flags.output_p = 1;
                strcat(the_busy_name, output_directory);
                strcat(the_busy_name, DIR_SEP_STRING);
            }
#  if defined(XeTeX)
            synctex_ctxt.flags.quoted = 0;
            strcat(the_busy_name, tmp);
#  else
            if (tmp[0] == '"' && tmp[len - 1] == '"') {
                /*  We are certainly on a pdftex like engine and the input file name did contain spaces inside.
                 Quotes where added around that file name. We prefer to remove the quotes to have a human readable name.
                 As of Fri Sep 19 14:00:01 UTC 2008, the file names containing quotes are not supported by pdfTeX
                 nor SyncTeX. */
                synctex_ctxt.flags.quoted = 1;      /* we will have to add quotes around the file name in the log file. */
                tmp[len - 1] = (char)0;             /* Remove the trailing " in order not to copy it */
                strcat(the_busy_name, tmp + 1);     /* only copy what follows the leading " character */
            } else {
                synctex_ctxt.flags.quoted = 0;
                strcat(the_busy_name, tmp);
            }
#  endif
            SYNCTEX_FREE(tmp);
            tmp = NULL;
            strcat(the_busy_name, synctex_suffix);
            /*  Initialize SYNCTEX_NO_GZ with the content of \synctex to let the user choose the format. */
            strcat(the_busy_name, synctex_suffix_busy);
            if (SYNCTEX_NO_GZ) {
                SYNCTEX_FILE = fopen(the_busy_name, FOPEN_W_MODE);
                synctex_ctxt.fprintf = (synctex_fprintf_t) (&fprintf);
            } else {
                SYNCTEX_FILE = gzopen(the_busy_name, FOPEN_WBIN_MODE);
                synctex_ctxt.fprintf = (synctex_fprintf_t) (&gzprintf);
            }
#   if SYNCTEX_DEBUG
            printf("\nwarning: Synchronize DEBUG: synctex_dot_open 2\n");
#   endif
            if (SYNCTEX_FILE) {
                if (SYNCTEX_NO_ERROR == synctex_record_preamble()) {
                    /*  Initialization of the context */
                    if (synctex_ctxt.magnification == 0)
                        synctex_ctxt.magnification = 1000;
                    synctex_ctxt.unit = SYNCTEX_UNIT_FACTOR;
                    /*  synctex_ctxt.busy_name was NULL before, it now owns the_busy_name */
                    synctex_ctxt.busy_name = the_busy_name;
                    the_busy_name = NULL;
                    /*  print the preamble, this is quite an UTF8 file  */
                    if (NULL != synctex_ctxt.root_name) {
                        synctex_record_input(1, synctex_ctxt.root_name);
                        SYNCTEX_FREE(synctex_ctxt.root_name);
                        synctex_ctxt.root_name = NULL;
                    }
                    synctex_ctxt.count = 0;
#   if SYNCTEX_DEBUG
                    fprintf(stdout,
                            "\nwarning: Synchronize DEBUG: synctex_dot_open SYNCTEX AVAILABLE\n");
#   endif
                    SYNCTEX_FREE(the_busy_name);
                    the_busy_name = NULL;
                    return SYNCTEX_FILE;
                } else {
                    printf("\nSyncTeX warning: no synchronization, problem with %s\n",
                           the_busy_name);
                }
            }
            SYNCTEX_FREE(the_busy_name);
            the_busy_name = NULL;
        } else {
            printf("\nSyncTeX information: no synchronization with keyboard input\n");
        }
        /*  no .synctex file available, so disable synchronization  */
        SYNCTEX_FREE(tmp);
        tmp = NULL;
        synctexabort(0);
        return NULL;
#   if SYNCTEX_DEBUG
        fprintf(stdout,
                "\nwarning: Synchronize DEBUG: synctex_dot_open SYNCTEX DISABLED\n");
#   endif
    }
    return SYNCTEX_FILE;
}

static inline int synctex_record_content(void);
static inline int synctex_record_settings(void);

/**
 *  synctex_record_settings must be called very late,
 *  only once there is an opportunity to know whether
 *  in pdf or dvi mode.
 */
static void * synctex_prepare_content(void)
{
#   if SYNCTEX_DEBUG
    printf("\nwarning: Synchronize DEBUG: synctex_prepare_content\n");
    printf("\nwarning: SYNCTEX_VALUE=%0X\n", SYNCTEX_VALUE);
    printf("\nwarning: synctex_options=%0X\n", synctex_ctxt.options);
#   endif
    if (SYNCTEX_CONTENT_READY) {
        return SYNCTEX_FILE;
    }
    if ((NULL != synctex_dot_open())
        && (SYNCTEX_NO_ERROR == synctex_record_settings())
        && (SYNCTEX_NO_ERROR == synctex_record_content())) {
        SYNCTEX_CONTENT_READY = SYNCTEX_YES;
#       if SYNCTEX_DEBUG
            printf("\nwarning: DONE");
#       endif
        return SYNCTEX_FILE;
    }
    synctexabort(0);
    return NULL;
}

/*  Each time TeX opens a file, it sends a synctexstartinput message and enters
 *  this function.  Here, a new synchronization tag is created and stored in
 *  the synctex_tag_field of the TeX current input context.  Each synchronized
 *  TeX node will record this tag instead of the file name.  synctexstartinput
 *  writes the mapping synctag <-> file name to the .synctex (or .synctex.gz) file.  A client
 *  will read the .synctex file and retrieve this mapping, it will be able to
 *  open the correct file just knowing its tag.  If the same file is read
 *  multiple times, it might be associated to different tags.  Synchronization
 *  controllers, either in viewers, editors or standalone should be prepared to
 *  handle this situation and take the appropriate action if they want to
 *  optimize memory.  No two different files will have the same positive tag.
 *  It is not advisable to definitely store the file names here.  If the file
 *  names ever have to be stored, it should definitely be done at the TeX level
 *  just like src-specials do, such that other components of the program can use
 *  it.  This function does not make any difference between the files, it
 *  treats the same way .tex, .aux, .sty ... files, even if many of them do not
 *  contain any material meant to be typeset.
 */
void synctexstartinput(void)
{
    static unsigned int synctex_tag_counter = 0;
    
    _synctex_read_command_line_option();

    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nwarning: Synchronize DEBUG: synctexstartinput %i",
           synctex_tag_counter);
    printf("\nwarning: SYNCTEX_VALUE=%i", SYNCTEX_VALUE);
    printf("\nwarning: synctex_options=%0X\n", synctex_ctxt.options);
#   endif

    if (SYNCTEX_IS_OFF) {
        return;
    }
    /*  synctex_tag_counter is a counter uniquely identifying the file actually
     *  open.  Each time tex opens a new file, synctexstartinput will increment this
     *  counter  */
    if (~synctex_tag_counter > 0) {
        ++synctex_tag_counter;
    } else {
        /*  we have reached the limit, subsequent files will be softly ignored
         *  this makes a lot of files... even in 32 bits
         *  Maybe we will limit this to 16bits and
         *  use the 16 other bits to store the column number */
        SYNCTEX_CURRENT_TAG = 0;
        return;
    }
    SYNCTEX_CURRENT_TAG = (int) synctex_tag_counter;     /*  -> *TeX.web  */
    if (synctex_tag_counter == 1) {
        /*  this is the first file TeX ever opens, in general \jobname.tex we
         *  do not know yet if synchronization will ever be enabled so we have
         *  to store the file name, because we will need it later.
         *  This is necessary because \jobname can be different */
#ifdef W32UPTEXSYNCTEX
        char *tmpa = SYNCTEX_GET_CURRENT_NAME();
        synctex_ctxt.root_name = chgto_oem(tmpa);
        free(tmpa);
#else
        synctex_ctxt.root_name = SYNCTEX_GET_CURRENT_NAME();
#endif
        if (!strlen(synctex_ctxt.root_name)) {
#if defined(MIKTEX)
            /* C++: typecast needed */
            synctex_ctxt.root_name = (char*)xrealloc(synctex_ctxt.root_name, strlen("texput") + 1);
#else
            synctex_ctxt.root_name = xrealloc(synctex_ctxt.root_name, strlen("texput") + 1);
#endif
            strcpy(synctex_ctxt.root_name, "texput");
        }
#   if SYNCTEX_DEBUG
        printf("\nwarning: Synchronize DEBUG: synctexstartinput first END\n");
#   endif
        return;
    }
    if (SYNCTEX_FILE
        || (NULL != synctex_dot_open())) {
#ifdef W32UPTEXSYNCTEX
        char *tmpb = SYNCTEX_GET_CURRENT_NAME();
        char *tmp = chgto_oem(tmpb);
        free(tmpb);
#else
        char *tmp = SYNCTEX_GET_CURRENT_NAME();
#endif
        /*  Always record the input, even if SYNCTEX_VALUE is 0.
         *  It may happen when \synctex=0 was given temporarily
         *  in the source file.
         */
        synctex_record_input(SYNCTEX_CURRENT_TAG,tmp);
        SYNCTEX_FREE(tmp);
    }
#   if SYNCTEX_DEBUG
    printf("\nwarning: Synchronize DEBUG: synctexstartinput END\n");
#   endif
    return;
}

/*  All the synctex... functions below have the smallest set of parameters.  It
 *  appears to be either the address of a node, or nothing at all.  Using zmem,
 *  which is the place where all the nodes are stored, one can retrieve every
 *  information about a node.  The other information is obtained through the
 *  global context variable.
 */

static inline int synctex_record_postamble(void);


/*  Free all memory used and close the file,
 *  sent by close_files_and_terminate in tex.web.
 *  synctexterminate() is called when the TeX run terminates.
 *  If synchronization was active, the working synctex file is moved to
 *  the final synctex file name.
 *  If synchronization was not active of if there is no output,
 *  the synctex file is removed if any.
 *  That way we can be sure that any synctex file is in sync with a tex run.
 *  However, it does not mean that it will be in sync with the pdf, especially
 *  when the output is dvi or xdv and the dvi (or xdv) to pdf driver has not been applied.
 */
void synctexterminate(boolean log_opened)
{
    char *tmp = NULL;
    char *the_real_syncname = NULL;
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexterminate\n");
#   endif
    if (log_opened && (tmp = SYNCTEX_GET_LOG_NAME())) {
        /* In version 1, the jobname was used but it caused problems regarding spaces in file names. */
#if defined(MIKTEX) && (defined(MIKTEX_PDFTEX) || defined(MIKTEX_XETEX))
      MiKTeX::Core::PathName path = MiKTeX::TeXAndFriends::WebAppInputLine::GetWebAppInputLine()->GetOutputDirectory();
      path /= MiKTeX::Core::PathName(tmp).RemoveDirectorySpec();
      tmp = (char*)xrealloc(tmp, path.GetLength() + 1);
      strcpy(tmp, path.GetData());
#endif
#if defined(MIKTEX) && defined(__cplusplus)
        /* C++: typecast needed */
        the_real_syncname = (char*)
        xmalloc((unsigned)(strlen(tmp)
                           + strlen(synctex_suffix)
                           + strlen(synctex_suffix_gz)
                           + 1));
#else
        the_real_syncname =
        xmalloc((unsigned)(strlen(tmp)
                           + strlen(synctex_suffix)
                           + strlen(synctex_suffix_gz)
                           + 1));
#endif
        if (!the_real_syncname) {
            SYNCTEX_FREE(tmp);
            synctexabort(0);
            return;
        }
        strcpy(the_real_syncname, tmp);
        SYNCTEX_FREE(tmp);
        tmp = NULL;
        /* now remove the last path extension which is in general log */
        tmp = the_real_syncname + strlen(the_real_syncname);
        while (tmp > the_real_syncname) {
            --tmp;
            if (*tmp == '.') {
                *tmp = (char)0;    /* end the string here */
                break;
            }
        }
        strcat(the_real_syncname, synctex_suffix);
        if (!SYNCTEX_NO_GZ) {
            /*  Remove any uncompressed synctex file, from a previous build. */
            remove(the_real_syncname);
            if (!SYNCTEX_NO_GZ_AUX_NAME) {
                strcat(the_real_syncname, synctex_suffix_gz);
            }
        }
        /* allways remove the synctex output file before renaming it, windows requires it. */
        if (0 != remove(the_real_syncname) && errno == EACCES) {
            fprintf(stderr,
                    "SyncTeX: Can't remove %s (file is open or read only)\n",
                    the_real_syncname);
        }
        if (SYNCTEX_FILE) {
            if (SYNCTEX_NOT_VOID) {
                synctex_record_postamble();
                /* close the synctex file */
                if (SYNCTEX_NO_GZ) {
                    xfclose((FILE *) SYNCTEX_FILE, synctex_ctxt.busy_name);
                } else {
                    gzclose((gzFile) SYNCTEX_FILE);
                }
                SYNCTEX_FILE = NULL;
#if defined(MIKTEX)
#  if defined(_MSC_VER)
		/* MSVC rename() requires that the new name is not the
		   name of an existing file */
#if defined(__cplusplus)
		if(MiKTeX::Core::File::Exists(MiKTeX::Core::PathName(the_real_syncname)))
		  {
		    MiKTeX::Core::File::Delete(MiKTeX::Core::PathName(the_real_syncname));
		  }
#else
		if(_access(the_real_syncname, 0) == 0)
		{
		  _unlink (the_real_syncname);
		}
#endif
#  endif
#endif
                /*  renaming the working synctex file */
                if (0 == rename(synctex_ctxt.busy_name, the_real_syncname)) {
                    if (log_opened) {
                        tmp = the_real_syncname;
#                       if SYNCTEX_DO_NOT_LOG_OUTPUT_DIRECTORY
                        if (synctex_ctxt.flags.output_p) {
                            tmp += strlen(output_directory) + strlen(DIR_SEP_STRING);
                        }
#                       endif
#ifndef SYNCTEX_interaction
#   define SYNCTEX_interaction 1
#endif
                        if (SYNCTEX_interaction>0) {
#ifdef W32UPTEXSYNCTEX
                        {
                        char *stmp = chgto_oem(tmp);
                        printf((synctex_ctxt.flags.quoted ? "SyncTeX written on \"%s\"\n" : "\nSyncTeX written on %s.\n"),
                               stmp);
                        free(stmp);
                        }
#else
#if defined(MIKTEX)
                        if (termoffset > 0)
                        {
                          printf("\n");
                        }
                        printf((synctex_ctxt.flags.quoted ? "SyncTeX written on \"%s\"\n" : "SyncTeX written on %s.\n"),
                          tmp);
                        termoffset = 0;
#else
#ifndef SYNCTEX_PRE_NL
#   define SYNCTEX_PRE_NL "\n"
#endif
#ifndef SYNCTEX_POST_NL
#   define SYNCTEX_POST_NL ""
#endif
                        printf((synctex_ctxt.flags.quoted ?
                                SYNCTEX_PRE_NL
                                "SyncTeX written on \"%s\""
                                SYNCTEX_POST_NL :
                                SYNCTEX_PRE_NL
                                "SyncTeX written on %s."
                                SYNCTEX_POST_NL
                                ),
                               tmp);
#endif
#endif
                        }
                        tmp = NULL;
                    }
                } else {
                    fprintf(stderr, "SyncTeX: Can't rename %s to %s\n",
                            synctex_ctxt.busy_name, the_real_syncname);
                    remove(synctex_ctxt.busy_name);
                }
            } else {
                /* close and remove the synctex file because there are no pages of output */
                if (SYNCTEX_NO_GZ) {
                    xfclose((FILE *) SYNCTEX_FILE, synctex_ctxt.busy_name);
                } else {
                    gzclose((gzFile) SYNCTEX_FILE);
                }
                SYNCTEX_FILE = NULL;
                remove(synctex_ctxt.busy_name);
            }
        }
        if (SYNCTEX_NO_GZ || SYNCTEX_NO_GZ_AUX_NAME) {
            /*  Remove any compressed synctex file, from a previous build. */
            strcat(the_real_syncname, synctex_suffix_gz);
            remove(the_real_syncname);
        }
    } else if ((tmp = SYNCTEX_GET_JOB_NAME())) {
        size_t len = strlen(tmp);
        /*  There was a problem with the output.
         We just try to remove existing synctex output files
         including the busy one. */
#if defined(MIKTEX)
        /* C++: typecast needed */
        the_real_syncname = (char*)xmalloc((size_t)
                                    (len
                                     + strlen(synctex_suffix)
                                     + strlen(synctex_suffix_gz)
                                     + 1));
#else
        the_real_syncname = xmalloc((size_t)
                                    (len
                                     + strlen(synctex_suffix)
                                     + strlen(synctex_suffix_gz)
                                     + 1));
#endif
        if (!the_real_syncname) {
            SYNCTEX_FREE(tmp);
            synctexabort(0);
            return;
        }
#   if defined(XeTeX)
        strcpy(the_real_syncname, tmp);
#   else
        if (len > 0 && tmp[0] == '"' && tmp[len - 1] == '"') {
            /*  See above a similar situation. */
            strcpy(the_real_syncname, tmp + 1); /* only copy what follows the leading " character */
            len = strlen(the_real_syncname);
            if ((len > 0) && (the_real_syncname[len - 1] == '"')) {
                the_real_syncname[len - 1] = '\0';
            }
        } else {
            strcpy(the_real_syncname, tmp);
        }
#   endif
        SYNCTEX_FREE(tmp);
        tmp = NULL;
        strcat(the_real_syncname, synctex_suffix);
        remove(the_real_syncname);
        strcat(the_real_syncname, synctex_suffix_gz);
        remove(the_real_syncname);
        if (SYNCTEX_FILE) {
            /* close the synctex file */
            if (SYNCTEX_NO_GZ) {
                xfclose((FILE *) SYNCTEX_FILE, synctex_ctxt.busy_name);
            } else {
                gzclose((gzFile) SYNCTEX_FILE);
            }
            SYNCTEX_FILE = NULL;
            /*  removing the working synctex file */
            remove(synctex_ctxt.busy_name);
        }
    }
    SYNCTEX_FREE(synctex_ctxt.busy_name);
    synctex_ctxt.busy_name = NULL;
    SYNCTEX_FREE(the_real_syncname);
    the_real_syncname = NULL;
    synctexabort(0);
}

static inline int synctex_record_sheet(integer sheet);

/*  Recording the "{..." line.  In *tex.web, use synctex_sheet(pdf_output) at
 *  the very beginning of the ship_out procedure.
 */
void synctexsheet(integer mag)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexsheet %i\n", mag);
#   endif
    if (SYNCTEX_IS_OFF) {
        if (SYNCTEX_VALUE && !SYNCTEX_WARNING_DISABLE) {
            SYNCTEX_WARNING_DISABLE = SYNCTEX_YES;
            printf
            ("\nSyncTeX warning: Synchronization was disabled from\nthe command line with -synctex=0\nChanging the value of \\synctex has no effect.");
        }
        return;
    }
    if (SYNCTEX_GET_TOTAL_PAGES() == 0) {
        /*  Now it is time to properly set up the scale factor. */
        if (mag > 0) {
            synctex_ctxt.magnification = mag;
        }
    }
    if (NULL != synctex_prepare_content()) {
        /*  First possibility: the .synctex file is already open because SyncTeX was activated on the CLI
         *  or it was activated with the \synctex macro and the first page is already shipped out.
         *  Second possibility: tries to open the .synctex, useful if synchronization was enabled
         *  from the source file and not from the CLI. */
        synctex_record_sheet(SYNCTEX_GET_TOTAL_PAGES()+1);
    }
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexsheet END\n");
#   endif
    return;
}

static inline int synctex_record_anchor(void);
static inline int synctex_record_teehs(integer sheet);

/*  Recording the "}..." line.  In *tex.web, use synctex_teehs at
 *  the very end of the ship_out procedure.
 */
void synctexteehs(void)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexteehs\n");
#   endif
    if (SYNCTEX_IS_OFF || !SYNCTEX_FILE) {
        return;
    }
    synctex_record_teehs(SYNCTEX_GET_TOTAL_PAGES());/* not SYNCTEX_GET_TOTAL_PAGES()+1*/
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexteehs END\n");
#   endif
    return;
}

/*  The SYNCTEX_IGNORE macro is used to detect unproperly initialized nodes.  See
 *  details in the implementation of the functions below.  */
#   define SYNCTEX_IGNORE(NODE) SYNCTEX_IS_OFF || !SYNCTEX_VALUE || !SYNCTEX_FILE || (synctex_ctxt.form_depth>0 && !SYNCTEX_WITH_FORMS)
#define SYNCTEX_RECORD_LEN_OR_RETURN_ERR do {\
    if (len > 0) {\
        synctex_ctxt.total_length += len;\
        ++synctex_ctxt.count;\
    } else {\
        return -1;\
    } } while(false)
#define SYNCTEX_RECORD_LEN_AND_RETURN_NOERR do {\
    if (len > 0) {\
        synctex_ctxt.total_length += len;\
        ++synctex_ctxt.count;\
        return SYNCTEX_NOERR;\
    } } while(false)

/*  Recording a "}..." or a ">" line  */
static inline int synctex_record_teehs(integer sheet)
{
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_teehs\n");
#   endif
    if (SYNCTEX_NOERR == synctex_record_anchor()) {
        int len = SYNCTEX_fprintf(SYNCTEX_FILE, "}%i\n", sheet);
        SYNCTEX_RECORD_LEN_AND_RETURN_NOERR;
    }
    synctexabort(0);
    return -1;
}

#   define SYNCTEX_CTXT_CURH \
(synctex_ctxt.curh)
#   define SYNCTEX_CTXT_CURV \
(synctex_ctxt.curv)
#   define SYNCTEX_SHOULD_COMPRESS_V \
SYNCTEX_H_COMPRESS && (synctex_ctxt.lastv == SYNCTEX_CTXT_CURV)

#if SYNCTEX_SUPPORT_PDF_FORM

static inline int synctex_record_pdfxform(halfword form);

/*  glue code, this message is sent whenever a pdf form will ship out
 See pdftex.web: procedure out_form(p: pointer);  */
void synctexpdfxform(halfword p)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexpdfxform\n");
#   endif
    if (SYNCTEX_IS_OFF) {
        if (SYNCTEX_VALUE && !SYNCTEX_WARNING_DISABLE) {
            SYNCTEX_WARNING_DISABLE = SYNCTEX_YES;
            printf
            ("\nSyncTeX warning: Synchronization was disabled from\nthe command line with -synctex=0\nChanging the value of \\synctex has no effect.");
        }
        return;
    }
    if (NULL != synctex_prepare_content()) {
        synctex_record_pdfxform(p);
    }
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexpdfxform END\n");
#   endif
    return;
}

static inline int synctex_record_mrofxfdp(void);

/*  glue code, this message is sent whenever a pdf form did ship out  */
void synctexmrofxfdp(void)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexmrofxfdp\n");
#   endif
    if (SYNCTEX_FILE) {
        synctex_record_mrofxfdp();
    }
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexmrofxfdp END\n");
#   endif
    return;
}

static inline int synctex_record_node_pdfrefxform(int formout);

/*  glue code, this message is sent whenever a pdf form ref will ship out
 See pdftex.web: procedure out_form(p: pointer);  */
void synctexpdfrefxform(int objnum)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexpdfrefxform\n");
#   endif
    if (SYNCTEX_FILE) {
        synctex_record_node_pdfrefxform(objnum);
    }
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexpdfrefxform END\n");
#   endif
    return;
}

/*  Recording a "<..." line  */
static inline int synctex_record_pdfxform(halfword form)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctex_record_pdfxform\n");
#   endif
    if (SYNCTEX_IGNORE(nothing)) {
        return SYNCTEX_NO_ERROR;
    } else {
        ++synctex_ctxt.form_depth;
        if (SYNCTEX_WITH_FORMS) {
            int len = SYNCTEX_fprintf(SYNCTEX_FILE, "<%i\n",
                                  SYNCTEX_PDF_CUR_FORM);
            SYNCTEX_RECORD_LEN_AND_RETURN_NOERR;
        } else {
            return SYNCTEX_NO_ERROR;
        }
    }
    synctexabort(0);
    return -1;
}

/*  Recording a ">" line  */
static inline int synctex_record_mrofxfdp(void)
{
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_mrofxfpd\n");
#   endif
    if (SYNCTEX_NOERR == synctex_record_anchor()) {
        --synctex_ctxt.form_depth;
        if (SYNCTEX_WITH_FORMS) {
            int len = SYNCTEX_fprintf(SYNCTEX_FILE, ">\n",
                                  SYNCTEX_PDF_CUR_FORM);
            SYNCTEX_RECORD_LEN_AND_RETURN_NOERR;
        } else {
            return SYNCTEX_NO_ERROR;
        }
    }
    synctexabort(0);
    return -1;
}

/*  Recording a "f..." line  */
static inline int synctex_record_node_pdfrefxform(int objnum)//UNUSED form JL
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctex_record_node_pdfrefxform\n");
#   endif
    synctex_ctxt.curh = SYNCTEX_CURH;
    synctex_ctxt.curv = SYNCTEX_CURV;
    if (SYNCTEX_IGNORE(nothing)) {
        return SYNCTEX_NO_ERROR;
    } else if (synctex_ctxt.form_depth>0 && !SYNCTEX_WITH_FORMS) {
        return SYNCTEX_NO_ERROR;
    } else {
        int len = 0;
        if (SYNCTEX_SHOULD_COMPRESS_V) {
            len = SYNCTEX_fprintf(SYNCTEX_FILE, "f%i:%i,=\n",
                                  objnum,
                                  SYNCTEX_CURH UNIT);
        } else {
            len = SYNCTEX_fprintf(SYNCTEX_FILE, "f%i:%i,%i\n",
                                  objnum,
                                  SYNCTEX_CURH UNIT,
                                  SYNCTEX_CURV UNIT);
            synctex_ctxt.lastv = SYNCTEX_CURV;
        }
        SYNCTEX_RECORD_LEN_AND_RETURN_NOERR;
    }
    synctexabort(0);
    return -1;
}
#else
/*  DO NOTHING functions.
    Any engine will have these functions, even when not using pdf xforms.
    One shoud be able to replace these void implementaions with an alternation in the header file.
 */
void synctexpdfxform(halfword p)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: DO NOTHING synctexpdfxform\n");
#   endif
    return;
}
void synctexmrofxfdp(void)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: DO NOTHING synctexmrofxfdp END\n");
#   endif
    return;
}
void synctexpdfrefxform(int objnum)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: DO NOTHING synctexpdfrefxform\n");
#   endif
    return;
}
#endif
/*#if SYNCTEX_SUPPORT_PDF_FORM*/

static inline void synctex_record_node_vlist(halfword p);

/*  When an hlist ships out, it can contain many different kern/glue nodes with
 *  exactly the same sync tag and line.  To reduce the size of the .synctex
 *  file, we only display a kern node sync info when either the sync tag or the
 *  line changes.  Also, we try ro reduce the distance between the chosen nodes
 *  in order to improve accuracy.  It means that we display information for
 *  consecutive nodes, as far as possible.  This tricky part uses a "recorder",
 *  which is the address of the routine that knows how to write the
 *  synchronization info to the .synctex file.  It also uses criteria to detect
 *  a change in the context, this is the macro SYNCTEX_???_CONTEXT_DID_CHANGE.*/

/*  This message is sent when a vlist will be shipped out, more precisely at
 *  the beginning of the vlist_out procedure in *TeX.web.  It will be balanced
 *  by a synctex_tsilv, sent at the end of the vlist_out procedure.  p is the
 *  address of the vlist. We assume that p is really a vlist node! */
void synctexvlist(halfword this_box)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexhlist\n");
#   endif
    if (SYNCTEX_IGNORE(this_box)) {
        return;
    }
    synctex_ctxt.node = this_box;   /*  0 to reset  */
    synctex_ctxt.recorder = NULL;   /*  reset  */
    synctex_ctxt.tag = SYNCTEX_TAG_MODEL(this_box,box);
    synctex_ctxt.line = SYNCTEX_LINE_MODEL(this_box,box);
    synctex_ctxt.curh = SYNCTEX_CURH;
    synctex_ctxt.curv = SYNCTEX_CURV;
    synctex_record_node_vlist(this_box);
}

static inline void synctex_record_node_tsilv(halfword p);

/*  Recording a "f" line ending a vbox: this message is sent whenever a vlist
 *  has been shipped out. It is used to close the vlist nesting level. It is
 *  sent at the end of the vlist_out procedure in *TeX.web to balance a former
 *  synctex_vlist sent at the beginning of that procedure.    */
void synctextsilv(halfword this_box)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctextsilv\n");
#   endif
    if (SYNCTEX_IGNORE(this_box)) {
        return;
    }
    /*  Ignoring any pending info to be recorded  */
    synctex_ctxt.node = this_box; /*  0 to reset  */
    synctex_ctxt.tag = SYNCTEX_TAG_MODEL(this_box,box);
    synctex_ctxt.line = SYNCTEX_LINE_MODEL(this_box,box);
    synctex_ctxt.curh = SYNCTEX_CURH;
    synctex_ctxt.curv = SYNCTEX_CURV;
    synctex_ctxt.recorder = NULL;
    synctex_record_node_tsilv(this_box);
}

static inline void synctex_record_node_void_vlist(halfword p);

/*  This message is sent when a void vlist will be shipped out.
 *  There is no need to balance a void vlist.  */
void synctexvoidvlist(halfword p, halfword this_box __attribute__ ((unused)))
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexvoidvlist\n");
#   endif
    if (SYNCTEX_IGNORE(p)) {
        return;
    }
    synctex_ctxt.node = p;          /*  reset  */
    synctex_ctxt.tag = SYNCTEX_TAG_MODEL(p,box);
    synctex_ctxt.line = SYNCTEX_LINE_MODEL(p,box);
    synctex_ctxt.curh = SYNCTEX_CURH;
    synctex_ctxt.curv = SYNCTEX_CURV;
    synctex_ctxt.recorder = NULL;   /*  reset  */
    synctex_record_node_void_vlist(p);
}

static inline void synctex_record_hlist(halfword p);

/*  This message is sent when an hlist will be shipped out, more precisely at
 *  the beginning of the hlist_out procedure in *TeX.web.  It will be balanced
 *  by a synctex_tsilh, sent at the end of the hlist_out procedure.  p is the
 *  address of the hlist We assume that p is really an hlist node! */
void synctexhlist(halfword this_box)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexhlist\n");
#   endif
    if (SYNCTEX_IGNORE(this_box)) {
        return;
    }
    synctex_ctxt.node = this_box;   /*  0 to reset  */
    synctex_ctxt.tag = SYNCTEX_TAG_MODEL(this_box,box);
    synctex_ctxt.line = SYNCTEX_LINE_MODEL(this_box,box);
    synctex_ctxt.curh = SYNCTEX_CURH;
    synctex_ctxt.curv = SYNCTEX_CURV;
    synctex_ctxt.recorder = NULL;   /*  reset  */
    synctex_record_hlist(this_box);
}

static inline void synctex_record_node_tsilh(halfword p);

/*  Recording a ")" line ending an hbox this message is sent whenever an hlist
 *  has been shipped out it is used to close the hlist nesting level. It is
 *  sent at the end of the hlist_out procedure in *TeX.web to balance a former
 *  synctex_hlist sent at the beginning of that procedure.    */
void synctextsilh(halfword this_box)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctextsilh\n");
#   endif
    if (SYNCTEX_IGNORE(this_box)) {
        return;
    }
    /*  Ignoring any pending info to be recorded  */
    synctex_ctxt.node = this_box;     /*  0 to force next node to be recorded!  */
    synctex_ctxt.tag = SYNCTEX_TAG_MODEL(this_box,box);
    synctex_ctxt.line = SYNCTEX_LINE_MODEL(this_box,box);
    synctex_ctxt.curh = SYNCTEX_CURH;
    synctex_ctxt.curv = SYNCTEX_CURV;
    synctex_ctxt.recorder = NULL;   /*  reset  */
    synctex_record_node_tsilh(this_box);
}

static inline void synctex_record_node_void_hlist(halfword p);

/*  This message is sent when a void hlist will be shipped out.
 *  There is no need to balance a void hlist.  */
void synctexvoidhlist(halfword p, halfword this_box __attribute__ ((unused)))
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexvoidhlist\n");
#   endif
    if (SYNCTEX_IGNORE(p)) {
        return;
    }
    /*  the sync context has changed  */
    if (synctex_ctxt.recorder != NULL) {
        /*  but was not yet recorded  */
        (*synctex_ctxt.recorder) (synctex_ctxt.node);
    }
    synctex_ctxt.node = p;          /*  0 to reset  */
    synctex_ctxt.tag = SYNCTEX_TAG_MODEL(p,box);
    synctex_ctxt.line = SYNCTEX_LINE_MODEL(p,box);
    synctex_ctxt.curh = SYNCTEX_CURH;
    synctex_ctxt.curv = SYNCTEX_CURV;
    synctex_ctxt.recorder = NULL;   /*  reset  */
    synctex_record_node_void_hlist(p);
}

/* With LuaTeX we have to consider other node sizes than medium ones */
#   define SYNCTEX_IGNORE_NODE(NODE,TYPE) SYNCTEX_IS_OFF || !SYNCTEX_VALUE \
|| (synctex_ctxt.form_depth>0 && !SYNCTEX_WITH_FORMS) \
|| (0 >= SYNCTEX_TAG_MODEL(NODE,TYPE)) \
|| (0 >= SYNCTEX_LINE_MODEL(NODE,TYPE))
/*  This macro will detect a change in the synchronization context.  As long as
 *  the synchronization context remains the same, there is no need to write
 *  synchronization info: it would not help more.  The synchronization context
 *  has changed when either the line number or the file tag has changed.  */
#   define SYNCTEX_CONTEXT_DID_CHANGE(NODE,TYPE) ((0 == synctex_ctxt.node)\
|| (SYNCTEX_TAG_MODEL(NODE,TYPE) != synctex_ctxt.tag)\
|| (SYNCTEX_LINE_MODEL(NODE,TYPE) != synctex_ctxt.line))

void synctex_record_node_math(halfword p);

/*  glue code, this message is sent whenever an inline math node will ship out
 See: @ @<Output the non-|char_node| |p| for...  */
void synctexmath(halfword p, halfword this_box __attribute__ ((unused)))
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexmath\n");
#   endif
    if (SYNCTEX_IGNORE(p)) {
        return;
    }
    if ((synctex_ctxt.recorder != NULL) && SYNCTEX_CONTEXT_DID_CHANGE(p,math)) {
        /*  the sync context did change  */
        (*synctex_ctxt.recorder) (synctex_ctxt.node);
    }
    synctex_ctxt.node = p;
    synctex_ctxt.tag = SYNCTEX_TAG_MODEL(p,math);
    synctex_ctxt.line = SYNCTEX_LINE_MODEL(p,math);
    synctex_ctxt.curh = SYNCTEX_CURH;
    synctex_ctxt.curv = SYNCTEX_CURV;
    synctex_ctxt.recorder = NULL;/*  no need to record once more  */
    synctex_record_node_math(p);/*  always record synchronously  */
}

static inline void synctex_record_node_glue(halfword p);
static inline void synctex_record_node_rule(halfword p);
static inline void synctex_record_node_kern(halfword p);

/*  this message is sent whenever an horizontal glue node or rule node ships out
 See: move_past:...    */
#   undef SYNCTEX_IGNORE
#   define SYNCTEX_IGNORE(NODE,TYPE) SYNCTEX_IS_OFF || !SYNCTEX_VALUE \
|| (0 >= SYNCTEX_TAG_MODEL(NODE,TYPE)) \
|| (0 >= SYNCTEX_LINE_MODEL(NODE,TYPE))
void synctexhorizontalruleorglue(halfword p, halfword this_box
                                 __attribute__ ((unused)))
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexhorizontalruleorglue\n");
#   endif
    switch (SYNCTEX_TYPE(p)) {
        case rule_node:
            if (SYNCTEX_IGNORE(p,rule)) {
                return;
            }
            break;
        case glue_node:
            if (SYNCTEX_IGNORE(p,glue)) {
                return;
            }
            break;
        case kern_node:
            if (SYNCTEX_IGNORE(p,kern)) {
                return;
            }
            break;
        default:
            printf("\nSynchronize ERROR: unknown node type %i\n", SYNCTEX_TYPE(p));
    }
    synctex_ctxt.node = p;
    synctex_ctxt.curh = SYNCTEX_CURH;
    synctex_ctxt.curv = SYNCTEX_CURV;
    synctex_ctxt.recorder = NULL;
    switch (SYNCTEX_TYPE(p)) {
        case rule_node:
            synctex_ctxt.tag = SYNCTEX_TAG_MODEL(p,rule);
            synctex_ctxt.line = SYNCTEX_LINE_MODEL(p,rule);
            synctex_record_node_rule(p); /*  always record synchronously: maybe some text is outside the box  */
            break;
        case glue_node:
            synctex_ctxt.tag = SYNCTEX_TAG_MODEL(p,glue);
            synctex_ctxt.line = SYNCTEX_LINE_MODEL(p,glue);
            synctex_record_node_glue(p); /*  always record synchronously: maybe some text is outside the box  */
            break;
        case kern_node:
            synctex_ctxt.tag = SYNCTEX_TAG_MODEL(p,kern);
            synctex_ctxt.line = SYNCTEX_LINE_MODEL(p,kern);
            synctex_record_node_kern(p); /*  always record synchronously: maybe some text is outside the box  */
            break;
        default:
            printf("\nSynchronize ERROR: unknown node type %i\n", SYNCTEX_TYPE(p));
    }
}

/*  this message is sent whenever a kern node ships out
 See: @ @<Output the non-|char_node| |p| for...    */
void synctexkern(halfword p, halfword this_box)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexkern\n");
#   endif
    if (SYNCTEX_IGNORE(p,kern)) {
        return;
    }
    if (SYNCTEX_CONTEXT_DID_CHANGE(p,kern)) {
        /*  the sync context has changed  */
        if (synctex_ctxt.recorder != NULL) {
            /*  but was not yet recorded  */
            (*synctex_ctxt.recorder) (synctex_ctxt.node);
        }
        if (synctex_ctxt.node == this_box) {
            /* first node in the list */
            synctex_ctxt.node = p;
            synctex_ctxt.tag = SYNCTEX_TAG_MODEL(p,kern);
            synctex_ctxt.line = SYNCTEX_LINE_MODEL(p,kern);
            synctex_ctxt.recorder = &synctex_record_node_kern;
        } else {
            synctex_ctxt.node = p;
            synctex_ctxt.tag = SYNCTEX_TAG_MODEL(p,kern);
            synctex_ctxt.line = SYNCTEX_LINE_MODEL(p,kern);
            synctex_ctxt.recorder = NULL;
            /*  always record when the context has just changed
             *  and when not the first node  */
            synctex_record_node_kern(p);
        }
    } else {
        /*  just update the geometry and type (for future improvements)  */
        synctex_ctxt.node = p;
        synctex_ctxt.tag = SYNCTEX_TAG_MODEL(p,kern);
        synctex_ctxt.line = SYNCTEX_LINE_MODEL(p,kern);
        synctex_ctxt.recorder = &synctex_record_node_kern;
    }
}

/*  This last part is used as a tool to infer TeX behaviour,
 *  but not for direct synchronization. */
#   undef SYNCTEX_IGNORE
#   define SYNCTEX_IGNORE(NODE) SYNCTEX_IS_OFF || !SYNCTEX_VALUE || !SYNCTEX_FILE \
|| (synctex_ctxt.count>2000)

void synctex_record_node_char(halfword p);

/*  this message is sent whenever a char node ships out    */
void synctexchar(halfword p, halfword this_box __attribute__ ((unused)))
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexchar\n");
#   endif
    if (SYNCTEX_IGNORE(p)) {
        return;
    }
    if (synctex_ctxt.recorder != NULL) {
        /*  but was not yet recorded  */
        (*synctex_ctxt.recorder) (synctex_ctxt.node);
    }
    synctex_ctxt.node = p;
    synctex_ctxt.tag = 0;
    synctex_ctxt.line = 0;
    synctex_ctxt.recorder = NULL;
    /*  always record when the context has just changed  */
    synctex_record_node_char(p);
}

void synctex_record_node_unknown(halfword p);

#   undef SYNCTEX_IGNORE
#   define SYNCTEX_IGNORE(NODE) (SYNCTEX_IS_OFF || !SYNCTEX_VALUE || !SYNCTEX_FILE)

/*  this message should be sent to record information
 for a node of an unknown type    */
void synctexnode(halfword p, halfword this_box __attribute__ ((unused)))
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexnode\n");
#   endif
    if (SYNCTEX_IGNORE(p)) {
        return;
    }
    /*  always record, not very usefull yet  */
    synctex_record_node_unknown(p);
}

/*  this message should be sent to record information
 synchronously for the current location    */
void synctexcurrent(void)
{
    SYNCTEX_RETURN_IF_DISABLED;
#   if SYNCTEX_DEBUG
    printf("\nSynchronize DEBUG: synctexcurrent\n");
#   endif
    if (SYNCTEX_IGNORE(nothing)) {
        return;
    } else {
        int len = 0;
        if (SYNCTEX_SHOULD_COMPRESS_V) {
            len = SYNCTEX_fprintf(SYNCTEX_FILE, "x%i,%i:%i,=\n",
                                  synctex_ctxt.tag,synctex_ctxt.line,
                                  SYNCTEX_CURH UNIT);
        } else {
            len = SYNCTEX_fprintf(SYNCTEX_FILE, "x%i,%i:%i,%i\n",
                                  synctex_ctxt.tag,synctex_ctxt.line,
                                  SYNCTEX_CURH UNIT,
                                  SYNCTEX_CURV UNIT);
            synctex_ctxt.lastv = SYNCTEX_CURV;
        }
        if (len > 0) {
            synctex_ctxt.total_length += len;
            return;
        }
    }
    synctexabort(0);
    return;
}

/*  Recording the settings  */
static inline int synctex_record_settings(void)
{
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_settings\n");
#   endif
    if (NULL == SYNCTEX_FILE) {
        return SYNCTEX_NOERR;
    }
    if (SYNCTEX_FILE) {
        int len = SYNCTEX_fprintf(SYNCTEX_FILE, "Output:%s\nMagnification:%i\nUnit:%i\nX Offset:%i\nY Offset:%i\n",
                                  SYNCTEX_OUTPUT,synctex_ctxt.magnification,synctex_ctxt.unit,
                                  ((SYNCTEX_OFFSET_IS_PDF != 0) ? 0 : 4736287 UNIT),
                                  ((SYNCTEX_OFFSET_IS_PDF != 0) ? 0 : 4736287 UNIT));
        if (len > 0) {
            synctex_ctxt.total_length += len;
            return SYNCTEX_NOERR;
        }
    }
    synctexabort(0);
    return -1;
}

/*  Recording a "SyncTeX..." line  */
static inline int synctex_record_preamble(void)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_preamble\n");
#   endif
    len =
    SYNCTEX_fprintf(SYNCTEX_FILE, "SyncTeX Version:%i\n",
                    synctex_ctxt.options>SYNCTEX_VERSION?
                    synctex_ctxt.options:SYNCTEX_VERSION);
    if (len > 0) {
        synctex_ctxt.total_length = len;
        return SYNCTEX_NOERR;
    }
    synctexabort(0);
    return -1;
}

/*  Recording a "Input:..." line  */
static inline int synctex_record_input(integer tag, char *name)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_input\n");
#   endif
    len = SYNCTEX_fprintf(SYNCTEX_FILE, "Input:%i:%s\n", tag, name);
    if (len > 0) {
        synctex_ctxt.total_length += len;
        return SYNCTEX_NOERR;
    }
    synctexabort(0);
    return -1;
}

/*  Recording a "!..." line  */
static inline int synctex_record_anchor(void)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_anchor\n");
    printf("\nSYNCTEX_FILE:%p\n",SYNCTEX_FILE);
    printf("\ntotal_length:%i\n",synctex_ctxt.total_length);
#   endif
    len = SYNCTEX_fprintf(SYNCTEX_FILE, "!%i\n", synctex_ctxt.total_length);
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_anchor 1\n");
#   endif
    if (len > 0) {
        synctex_ctxt.total_length = len;
        ++synctex_ctxt.count;
        return SYNCTEX_NOERR;
    }
    synctexabort(0);
    return -1;
}

/*  Recording a "Content" line  */
static inline int synctex_record_content(void)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_content\n");
#   endif
    len = SYNCTEX_fprintf(SYNCTEX_FILE, "Content:\n");
    if (len > 0) {
        synctex_ctxt.total_length += len;
        return SYNCTEX_NOERR;
    }
    synctexabort(0);
    return -1;
}

/*  Recording a "{..." line  */
static inline int synctex_record_sheet(integer sheet)
{
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_sheet\n");
#   endif
    if (SYNCTEX_NOERR == synctex_record_anchor()) {
        int len = SYNCTEX_fprintf(SYNCTEX_FILE, "{%i\n", sheet);
        SYNCTEX_RECORD_LEN_AND_RETURN_NOERR;
    }
    synctexabort(0);
    return -1;
}

/*  Recording a "v..." line  */
static inline void synctex_record_node_void_vlist(halfword p)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_void_vlist\n");
#   endif
    if (SYNCTEX_SHOULD_COMPRESS_V) {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "v%i,%i:%i,=:%i,%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,box),
                              SYNCTEX_LINE_MODEL(p,box),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_WIDTH(p) UNIT,
                              SYNCTEX_HEIGHT(p) UNIT,
                              SYNCTEX_DEPTH(p) UNIT);
    } else {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "v%i,%i:%i,%i:%i,%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,box),
                              SYNCTEX_LINE_MODEL(p,box),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_CTXT_CURV UNIT,
                              SYNCTEX_WIDTH(p) UNIT,
                              SYNCTEX_HEIGHT(p) UNIT,
                              SYNCTEX_DEPTH(p) UNIT);
        synctex_ctxt.lastv = SYNCTEX_CTXT_CURV;
   }
#   define SYNCTEX_RECORD_LEN_AND_RETURN do {\
    if (len > 0) {\
        synctex_ctxt.total_length += len;\
        ++synctex_ctxt.count;\
        return;\
    } } while(false)
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a "[..." line  */
static inline void synctex_record_node_vlist(halfword p)
{
    int len = 0;
    SYNCTEX_NOT_VOID = SYNCTEX_YES;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_vlist\n");
#   endif
    if (SYNCTEX_SHOULD_COMPRESS_V) {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "[%i,%i:%i,=:%i,%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,box),
                              SYNCTEX_LINE_MODEL(p,box),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_WIDTH(p) UNIT,
                              SYNCTEX_HEIGHT(p) UNIT,
                              SYNCTEX_DEPTH(p) UNIT);
    } else {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "[%i,%i:%i,%i:%i,%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,box),
                              SYNCTEX_LINE_MODEL(p,box),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_CTXT_CURV UNIT,
                              SYNCTEX_WIDTH(p) UNIT,
                              SYNCTEX_HEIGHT(p) UNIT,
                              SYNCTEX_DEPTH(p) UNIT);
       synctex_ctxt.lastv = SYNCTEX_CTXT_CURV;
    }
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a "]..." line  */
static inline void synctex_record_node_tsilv(halfword p __attribute__ ((unused)))
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_tsilv\n");
#   endif
    len = SYNCTEX_fprintf(SYNCTEX_FILE, "]\n");
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a "h..." line  */
static inline void synctex_record_node_void_hlist(halfword p)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_void_hlist\n");
#   endif
    if (SYNCTEX_SHOULD_COMPRESS_V) {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "h%i,%i:%i,=:%i,%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,box),
                              SYNCTEX_LINE_MODEL(p,box),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_WIDTH(p) UNIT,
                              SYNCTEX_HEIGHT(p) UNIT,
                              SYNCTEX_DEPTH(p) UNIT);
    } else {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "h%i,%i:%i,%i:%i,%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,box),
                              SYNCTEX_LINE_MODEL(p,box),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_CTXT_CURV UNIT,
                              SYNCTEX_WIDTH(p) UNIT,
                              SYNCTEX_HEIGHT(p) UNIT,
                              SYNCTEX_DEPTH(p) UNIT);
        synctex_ctxt.lastv = SYNCTEX_CTXT_CURV;
    }
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a "(..." line  */
static inline void synctex_record_hlist(halfword p)
{
    int len = 0;
    SYNCTEX_NOT_VOID = SYNCTEX_YES;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_hlist\n");
#   endif
    if (SYNCTEX_SHOULD_COMPRESS_V) {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "(%i,%i:%i,=:%i,%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,box),
                              SYNCTEX_LINE_MODEL(p,box),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_WIDTH(p) UNIT,
                              SYNCTEX_HEIGHT(p) UNIT,
                              SYNCTEX_DEPTH(p) UNIT);
    } else {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "(%i,%i:%i,%i:%i,%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,box),
                              SYNCTEX_LINE_MODEL(p,box),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_CTXT_CURV UNIT,
                              SYNCTEX_WIDTH(p) UNIT,
                              SYNCTEX_HEIGHT(p) UNIT,
                              SYNCTEX_DEPTH(p) UNIT);
        synctex_ctxt.lastv = SYNCTEX_CTXT_CURV;
    }
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a ")..." line  */
static inline void synctex_record_node_tsilh(halfword p __attribute__ ((unused)))
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_tsilh\n");
#   endif
    len = SYNCTEX_fprintf(SYNCTEX_FILE, ")\n");
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a "Count..." line  */
static inline int synctex_record_count(void)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_count\n");
#   endif
    len = SYNCTEX_fprintf(SYNCTEX_FILE, "Count:%i\n", synctex_ctxt.count);
    if (len > 0) {
        synctex_ctxt.total_length += len;
        return SYNCTEX_NOERR;
    }
    synctexabort(0);
    return -1;
}

/*  Recording a "Postamble" section  */
static inline int synctex_record_postamble(void)
{
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_postamble\n");
#   endif
    if (SYNCTEX_NOERR == synctex_record_anchor()) {
        int len = SYNCTEX_fprintf(SYNCTEX_FILE, "Postamble:\n");
        if (len > 0) {
            synctex_ctxt.total_length += len;
            if (synctex_record_count() || synctex_record_anchor()) {
            } else {
                len = SYNCTEX_fprintf(SYNCTEX_FILE, "Post scriptum:\n");
                if (len > 0) {
                    synctex_ctxt.total_length += len;
                    return SYNCTEX_NOERR;
                }
            }
        }
    }
    synctexabort(0);
    return -1;
}

/*  Recording a "g..." line  */
static inline void synctex_record_node_glue(halfword p)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_glue\n");
#   endif
    if (SYNCTEX_SHOULD_COMPRESS_V) {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "g%i,%i:%i,=\n",
                              SYNCTEX_TAG_MODEL(p,glue),
                              SYNCTEX_LINE_MODEL(p,glue),
                              SYNCTEX_CTXT_CURH UNIT);
    } else {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "g%i,%i:%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,glue),
                              SYNCTEX_LINE_MODEL(p,glue),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_CTXT_CURV UNIT);
        synctex_ctxt.lastv = SYNCTEX_CTXT_CURV;
    }
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a "k..." line  */
static inline void synctex_record_node_kern(halfword p)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_kern\n");
#   endif
    if (SYNCTEX_SHOULD_COMPRESS_V) {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "k%i,%i:%i,=:%i\n",
                              SYNCTEX_TAG_MODEL(p,glue),
                              SYNCTEX_LINE_MODEL(p,glue),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_WIDTH(p) UNIT);
    } else {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "k%i,%i:%i,%i:%i\n",
                              SYNCTEX_TAG_MODEL(p,glue),
                              SYNCTEX_LINE_MODEL(p,glue),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_CTXT_CURV UNIT,
                              SYNCTEX_WIDTH(p) UNIT);
        synctex_ctxt.lastv = SYNCTEX_CTXT_CURV;
    }
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a "r..." line  */
static inline void synctex_record_node_rule(halfword p)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_tsilh\n");
#   endif
    if (SYNCTEX_SHOULD_COMPRESS_V) {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "r%i,%i:%i,=:%i,%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,rule),
                              SYNCTEX_LINE_MODEL(p,rule),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_RULE_WD UNIT, SYNCTEX_RULE_HT UNIT, SYNCTEX_RULE_DP UNIT);
    } else {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "r%i,%i:%i,%i:%i,%i,%i\n",
                              SYNCTEX_TAG_MODEL(p,rule),
                              SYNCTEX_LINE_MODEL(p,rule),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_CTXT_CURV UNIT,
                              SYNCTEX_RULE_WD UNIT, SYNCTEX_RULE_HT UNIT, SYNCTEX_RULE_DP UNIT);
        synctex_ctxt.lastv = SYNCTEX_CTXT_CURV;
    }
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a "$..." line  */
void synctex_record_node_math(halfword p)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_math\n");
#   endif
    if (SYNCTEX_SHOULD_COMPRESS_V) {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "$%i,%i:%i,=\n",
                              SYNCTEX_TAG_MODEL(p, math),
                              SYNCTEX_LINE_MODEL(p, math),
                              SYNCTEX_CTXT_CURH UNIT);
    } else {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "$%i,%i:%i,%i\n",
                              SYNCTEX_TAG_MODEL(p, math),
                              SYNCTEX_LINE_MODEL(p, math),
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_CTXT_CURV UNIT);
        synctex_ctxt.lastv = SYNCTEX_CTXT_CURV;
    }
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a "c..." line  */
void synctex_record_node_char(halfword p __attribute__ ((unused)))
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_char\n");
#   endif
    if (SYNCTEX_SHOULD_COMPRESS_V) {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "c%i,=\n",
                              SYNCTEX_CTXT_CURH UNIT);
    } else {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "c%i,%i\n",
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_CTXT_CURV UNIT);
        synctex_ctxt.lastv = SYNCTEX_CTXT_CURV;
    }
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}

/*  Recording a "?..." line, type, subtype and position  */
void synctex_record_node_unknown(halfword p)
{
    int len = 0;
#   if SYNCTEX_DEBUG > 999
    printf("\nSynchronize DEBUG: synctex_record_node_unknown(0x%x)\n", p);
#   endif
    if (SYNCTEX_SHOULD_COMPRESS_V) {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "?%i,=:%i,%i\n",
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_TYPE(p), SYNCTEX_SUBTYPE(p));
    } else {
        len = SYNCTEX_fprintf(SYNCTEX_FILE, "?%i,%i:%i,%i\n",
                              SYNCTEX_CTXT_CURH UNIT,
                              SYNCTEX_CTXT_CURV UNIT,
                              SYNCTEX_TYPE(p), SYNCTEX_SUBTYPE(p));
        synctex_ctxt.lastv = SYNCTEX_CTXT_CURV;
    }
    SYNCTEX_RECORD_LEN_AND_RETURN;
    synctexabort(0);
    return;
}
#   endif
