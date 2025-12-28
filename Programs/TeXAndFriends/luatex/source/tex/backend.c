/* to fill */

#include "ptexlib.h"

scaled max_v = 0;               /* maximum height-plus-depth of pages shipped so far */
scaled max_h = 0;               /* maximum width of pages shipped so far */
boolean doing_leaders = false;  /* are we inside a leader box? */
int cur_s = -1;                 /* current depth of output box nesting, initially $-1$ */

static backend_struct *backend = NULL;
/* backend_function *backend_out, *backend_out_whatsit, *backend_out_control; */

backend_function_node_rule	 *backend_out_node_rule;
backend_function_node_glyph	 *backend_out_node_glyph;
backend_function_whatsit	 *backend_out_whatsit;
backend_function_whatsit_do	 *backend_out_whatsit_do;
backend_function_control	 *backend_out_control;
backend_function_control_list	 *backend_out_control_list;
backend_function_control_err	 *backend_out_control_err;
backend_function_control_ref	 *backend_out_control_ref;


static void whatsit__missing_backend_function(PDF pdf, halfword p)
{
  const char *n = get_node_name(type(p), subtype(p));
  if (type(p) == whatsit_node)
    formatted_error("pdf backend","no output function for whatsit %s",n);
  else
    formatted_error("pdf backend","no output function for node %s",n);
}

static void whatsit_do__missing_backend_function(PDF pdf, halfword p, halfword parent_box, scaledpos cur)
{
  (void)parent_box;
  (void)cur;
  whatsit__missing_backend_function(pdf,p);
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
    p->node_rule_fu	[BACKEND_INDEX(rule_node)]				= &pdf_place_rule;
    p->node_glyph_fu	[BACKEND_INDEX(glyph_node)]				= &pdf_place_glyph;

    p->whatsit_fu	[BACKEND_INDEX(special_node)]				= &pdf_special;
    p->whatsit_fu	[BACKEND_INDEX(late_special_node)]			= &pdf_special;
    p->whatsit_fu	[BACKEND_INDEX(late_lua_node)]				= &late_lua;
    p->whatsit_fu	[BACKEND_INDEX(pdf_literal_node)]			= &pdf_out_literal;
    p->whatsit_fu	[BACKEND_INDEX(pdf_late_literal_node)]			= &pdf_out_literal;
    p->whatsit_fu	[BACKEND_INDEX(pdf_refobj_node)]			= &pdf_ref_obj;
    p->whatsit_do_fu	[BACKEND_INDEX(pdf_annot_node)]				= &do_annot; /*0*/
    p->whatsit_do_fu	[BACKEND_INDEX(pdf_start_link_node)]			= &do_link; /*1*/
    p->whatsit_fu	[BACKEND_INDEX(pdf_end_link_node)]			= &end_link;
    p->whatsit_do_fu	[BACKEND_INDEX(pdf_dest_node)]				= &do_dest; /*2*/
    p->whatsit_do_fu	[BACKEND_INDEX(pdf_thread_node)]			= &do_thread;/*3*/
    p->whatsit_fu	[BACKEND_INDEX(pdf_end_thread_node)]			= &end_thread;
    p->whatsit_fu	[BACKEND_INDEX(pdf_colorstack_node)]			= &pdf_out_colorstack;
    p->whatsit_fu	[BACKEND_INDEX(pdf_setmatrix_node)]			= &pdf_out_setmatrix;
    p->whatsit_fu	[BACKEND_INDEX(pdf_save_node)]				= &pdf_out_save;
    p->whatsit_fu	[BACKEND_INDEX(pdf_restore_node)]			= &pdf_out_restore;
    p->whatsit_fu	[BACKEND_INDEX(pdf_link_state_node)]			= &pdf_out_link_state;

    p->control_list_fu	[BACKEND_INDEX(backend_control_push_list)]		= &pdf_push_list;/*0*/
    p->control_list_fu	[BACKEND_INDEX(backend_control_pop_list)]		= &pdf_pop_list;/*1*/
    p->control_fu	[BACKEND_INDEX(backend_control_begin_page)]		= &pdf_begin_page;
    p->control_fu	[BACKEND_INDEX(backend_control_end_page)]		= &pdf_end_page;
    p->control_fu	[BACKEND_INDEX(backend_control_open_file)]		= &pdf_open_file;
    p->control_fu	[BACKEND_INDEX(backend_control_write_header)]		= &pdf_write_header;
    p->control_err_fu	[BACKEND_INDEX(backend_control_finish_file)]		= &pdf_finish_file;/*0*/
    p->control_ref_fu	[BACKEND_INDEX(backend_control_set_reference_point)]	= &pdf_set_reference_point;/*0*/
}

static void init_dvi_backend_functions(void)
{
    backend_struct *p = &backend[OMODE_DVI];
    p->name = strdup("DVI");
    p->node_rule_fu	[BACKEND_INDEX(rule_node)]				= &dvi_place_rule;
    p->node_glyph_fu	[BACKEND_INDEX(glyph_node)]				= &dvi_place_glyph;
    p->whatsit_fu	[BACKEND_INDEX(special_node)]				= &dvi_special;
    p->whatsit_fu	[BACKEND_INDEX(late_special_node)]			= &dvi_special;
    p->whatsit_fu	[BACKEND_INDEX(late_lua_node)]				= &late_lua;
    p->control_list_fu	[BACKEND_INDEX(backend_control_push_list)]		= &dvi_push_list;
    p->control_list_fu	[BACKEND_INDEX(backend_control_pop_list)]		= &dvi_pop_list;
    p->control_fu	[BACKEND_INDEX(backend_control_begin_page)]		= &dvi_begin_page;
    p->control_fu	[BACKEND_INDEX(backend_control_end_page)]		= &dvi_end_page;
    p->control_fu	[BACKEND_INDEX(backend_control_open_file)]		= &dvi_open_file;
    p->control_fu	[BACKEND_INDEX(backend_control_write_header)]		= &dvi_write_header;
    p->control_err_fu	[BACKEND_INDEX(backend_control_finish_file)]		= &dvi_finish_file;
    p->control_ref_fu	[BACKEND_INDEX(backend_control_set_reference_point)]	= &dvi_set_reference_point;
}


void init_backend_functionpointers(output_mode o_mode)
{
    int i, j;
    if (backend == NULL) {
        backend = xmalloc((MAX_OMODE + 1) * sizeof(backend_struct));
        for (i = 0; i <= MAX_OMODE; i++) {

	  backend[i].node_rule_fu	= xmalloc((BACKEND__MAX_NODE_RULE__INDEX + 1)	* sizeof(backend_function_node_rule));
	  backend[i].node_glyph_fu	= xmalloc((BACKEND__MAX_NODE_GLYPH__INDEX + 1)	* sizeof(backend_function_node_glyph));
	  backend[i].whatsit_fu	        = xmalloc((BACKEND__MAX_WHATSIT__INDEX + 2)	* sizeof(backend_function_whatsit));
	  backend[i].whatsit_do_fu	= xmalloc((BACKEND__MAX_WHATSIT_DO__INDEX + 2)	* sizeof(backend_function_whatsit_do));
	  backend[i].control_fu	        = xmalloc((BACKEND__MAX_CONTROL__INDEX + 1)	* sizeof(backend_function_control));
	  backend[i].control_list_fu	= xmalloc((BACKEND__MAX_CONTROL_LIST__INDEX + 1)* sizeof(backend_function_control_list));
	  backend[i].control_err_fu	= xmalloc((BACKEND__MAX_CONTROL_ERR__INDEX + 1)	* sizeof(backend_function_control_err));
	  backend[i].control_ref_fu	= xmalloc((BACKEND__MAX_CONTROL_REF__INDEX + 1)	* sizeof(backend_function_control_ref));

	  backend[i].whatsit_fu[(BACKEND__MAX_WHATSIT__INDEX + 1)]		= &whatsit__missing_backend_function;
	  backend[i].whatsit_do_fu[(BACKEND__MAX_WHATSIT_DO__INDEX + 1)]	= &whatsit_do__missing_backend_function;

	  /* for (j = 0; j < MAX_NODE_TYPE + 1; j++) */
	  /*   backend[i].node_fu[j] = &missing_backend_function; */
	  /* for (j = 0; j < MAX_WHATSIT_TYPE + 1; j++) */
	  /*   backend[i].whatsit_fu[j] = &missing_backend_function; */
	  /* for (j = 0; j < MAX_CONTROL_TYPE + 1; j++) */
	  /*   backend[i].control_fu[j] = &missing_backend_function; */
        }
        init_none_backend_functions();
        init_dvi_backend_functions();
        init_pdf_backend_functions();
    }
    backend_out_node_rule	= backend[o_mode].node_rule_fu;
    backend_out_node_glyph	= backend[o_mode].node_glyph_fu;
    backend_out_whatsit		= backend[o_mode].whatsit_fu;
    backend_out_whatsit_do	= backend[o_mode].whatsit_do_fu;
    backend_out_control		= backend[o_mode].control_fu;
    backend_out_control_list	= backend[o_mode].control_list_fu;
    backend_out_control_err	= backend[o_mode].control_err_fu;
    backend_out_control_ref	= backend[o_mode].control_ref_fu;
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



