/* to fill */

#include "ptexlib.h"

scaled max_v = 0;               /* maximum height-plus-depth of pages shipped so far */
scaled max_h = 0;               /* maximum width of pages shipped so far */
boolean doing_leaders = false;  /* are we inside a leader box? */
int cur_s = -1;                 /* current depth of output box nesting, initially $-1$ */

static backend_struct *backend = NULL;
backend_function *backend_out, *backend_out_whatsit, *backend_out_control;

static void missing_backend_function(PDF pdf, halfword p)
{
    const char *n = get_node_name(type(p), subtype(p));
    if (type(p) == whatsit_node)
        formatted_error("pdf backend","no output function for whatsit %s",n);
    else
        formatted_error("pdf backend","no output function for node %s",n);
}

static void init_none_backend_functions(void)
{
    backend_struct *p = &backend[OMODE_NONE];
    p->name = strdup("NONE");
}

static void init_pdf_backend_functions(void)
{
    backend_struct *p = &backend[OMODE_PDF];
    p->name = strdup("PDF");
    p->node_fu[rule_node] = &pdf_place_rule;
    p->node_fu[glyph_node] = &pdf_place_glyph;
    p->whatsit_fu[special_node] = &pdf_special;
    p->whatsit_fu[pdf_literal_node] = &pdf_out_literal;
    p->whatsit_fu[pdf_refobj_node] = &pdf_ref_obj;
    p->whatsit_fu[pdf_annot_node] = &do_annot;
    p->whatsit_fu[pdf_start_link_node] = &do_link;
    p->whatsit_fu[pdf_end_link_node] = &end_link;
    p->whatsit_fu[pdf_dest_node] = &do_dest;
    p->whatsit_fu[pdf_thread_node] = &do_thread;
    p->whatsit_fu[pdf_end_thread_node] = &end_thread;
    p->whatsit_fu[late_lua_node] = &late_lua;
    p->whatsit_fu[pdf_colorstack_node] = &pdf_out_colorstack;
    p->whatsit_fu[pdf_setmatrix_node] = &pdf_out_setmatrix;
    p->whatsit_fu[pdf_save_node] = &pdf_out_save;
    p->whatsit_fu[pdf_restore_node] = &pdf_out_restore;
    p->control_fu[backend_control_push_list] = &pdf_push_list;
    p->control_fu[backend_control_pop_list] = &pdf_pop_list;
    p->control_fu[backend_control_begin_page] = &pdf_begin_page;
    p->control_fu[backend_control_end_page] = &pdf_end_page;
    p->control_fu[backend_control_open_file] = &pdf_open_file;
    p->control_fu[backend_control_write_header] = &pdf_write_header;
    p->control_fu[backend_control_finish_file] = &pdf_finish_file;
    p->control_fu[backend_control_set_reference_point] = &pdf_set_reference_point;
}

static void init_dvi_backend_functions(void)
{
    backend_struct *p = &backend[OMODE_DVI];
    p->name = strdup("DVI");
    p->node_fu[rule_node] = &dvi_place_rule;
    p->node_fu[glyph_node] = &dvi_place_glyph;
    p->whatsit_fu[special_node] = &dvi_special;
    p->whatsit_fu[late_lua_node] = &late_lua;
    p->control_fu[backend_control_push_list] = &dvi_push_list;
    p->control_fu[backend_control_pop_list] = &dvi_pop_list;
    p->control_fu[backend_control_begin_page] = &dvi_begin_page;
    p->control_fu[backend_control_end_page] = &dvi_end_page;
    p->control_fu[backend_control_open_file] = &dvi_open_file;
    p->control_fu[backend_control_write_header] = &dvi_write_header;
    p->control_fu[backend_control_finish_file] = &dvi_finish_file;
    p->control_fu[backend_control_set_reference_point] = &dvi_set_reference_point;
}


void init_backend_functionpointers(output_mode o_mode)
{
    int i, j;
    if (backend == NULL) {
        backend = xmalloc((MAX_OMODE + 1) * sizeof(backend_struct));
        for (i = 0; i <= MAX_OMODE; i++) {
            backend[i].node_fu = xmalloc((MAX_NODE_TYPE + 1) * sizeof(backend_function));
            backend[i].whatsit_fu = xmalloc((MAX_WHATSIT_TYPE + 1) * sizeof(backend_function));
            backend[i].control_fu = xmalloc((MAX_CONTROL_TYPE + 1) * sizeof(backend_function));
            for (j = 0; j < MAX_NODE_TYPE + 1; j++)
                backend[i].node_fu[j] = &missing_backend_function;
            for (j = 0; j < MAX_WHATSIT_TYPE + 1; j++)
                backend[i].whatsit_fu[j] = &missing_backend_function;
            for (j = 0; j < MAX_CONTROL_TYPE + 1; j++)
                backend[i].control_fu[j] = &missing_backend_function;
        }
        init_none_backend_functions();
        init_dvi_backend_functions();
        init_pdf_backend_functions();
    }
    backend_out = backend[o_mode].node_fu;
    backend_out_whatsit = backend[o_mode].whatsit_fu;
    backend_out_control = backend[o_mode].control_fu;
}

output_mode get_o_mode(void)
{
    output_mode o_mode;
    if (output_mode_par > 0) {
        o_mode = OMODE_PDF;
    } else
        o_mode = OMODE_DVI;
    return o_mode;
}

void fix_o_mode(void)
{
    output_mode o_mode = get_o_mode();
    if (output_mode_used == OMODE_NONE) {
        output_mode_used = o_mode;
        /*tex Used by synctex, we need to use output_mode_used there: */
        static_pdf->o_mode = output_mode_used;
    } else if (output_mode_used != o_mode) {
        normal_error("pdf backend", "\\outputmode can only be changed before anything is written to the output");
    }
    init_backend_functionpointers(output_mode_used);
}
