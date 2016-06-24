/* synctex-luatex.h

   Copyright (c) 2010, 2011 Taco Hoekwater <taco@luatex.org>

   This file is part of LuaTeX.

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

*/

/* $Id: synctex-luatex.h 4064 2011-01-12 22:12:22Z hhenkel $ */

#       define synchronization_field_size 1

#define SYNCTEX_TAG_MODEL(NODE,TYPE)\
                    vinfo(NODE+TYPE##_node_size-synchronization_field_size)
#define SYNCTEX_LINE_MODEL(NODE,TYPE)\
                    vlink(NODE+TYPE##_node_size-synchronization_field_size)

#define SYNCTEX_TYPE(NODE) type(NODE)
#define SYNCTEX_SUBTYPE(NODE) subtype(NODE)
#define SYNCTEX_WIDTH(NODE) width(NODE)
#define SYNCTEX_DEPTH(NODE) depth(NODE)
#define SYNCTEX_HEIGHT(NODE) height(NODE)
#define SYNCTEX_VALUE int_par(synctex_code)

#define SYNCTEX_CURVV (dimen_par(page_height_code)-static_pdf->posstruct->pos.v)
#define SYNCTEX_CURHH static_pdf->posstruct->pos.h

#define SYNCTEX_CURV (static_pdf->o_mode==OMODE_PDF?SYNCTEX_CURVV:SYNCTEX_CURVV-4736287)
#define SYNCTEX_CURH (static_pdf->o_mode==OMODE_PDF?SYNCTEX_CURHH:SYNCTEX_CURHH-4736287)

#define SYNCTEX_GET_JOB_NAME() makecstring(job_name)
#define SYNCTEX_GET_LOG_NAME() get_full_log_name()

#define SYNCTEX_RULE_WD width(p)
#define SYNCTEX_RULE_HT height(p)
#define SYNCTEX_RULE_DP depth(p)

#define SYNCTEX_CURRENT_TAG (cur_input.synctex_tag_field)
#define SYNCTEX_GET_CURRENT_NAME() luatex_synctex_get_current_name()
#define SYNCTEX_GET_TOTAL_PAGES() (total_pages)

#include "ptexlib.h"

/*   We observe static_pdf->o_mode in order to determine whether output mode is
 *   pdf or dvi.
 *   We will assume that pdf->o_mode is set before entering
 *   the synctex_sheet function below.  */
#define SYNCTEX_OFFSET_IS_PDF (static_pdf->o_mode==OMODE_PDF)
#define SYNCTEX_OUTPUT ((static_pdf->o_mode==OMODE_PDF)?"pdf":"dvi")

#define __SyncTeX__ 1
#define SYNCTEX_DO_NOT_LOG_OUTPUT_DIRECTORY 1

/*   New LuaTeX (0.87.0) or later */
#define glue_node 12
#define kern_node 13
