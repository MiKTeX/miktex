/* to fill */

#ifndef BACKEND_H
#  define BACKEND_H

#include "ptexlib.h"

extern scaled max_v;
extern scaled max_h;
extern boolean doing_leaders;
extern int cur_s;

# define MAX_CONTROL_TYPE 7
# define BACKEND_INDEX(a) backend__##a##__index



/* node */
#define backend__rule_node__index				0
#define BACKEND__MAX_NODE_RULE__INDEX   backend__rule_node__index             
#define backend__glyph_node__index			        0
#define BACKEND__MAX_NODE_GLYPH__INDEX  backend__glyph_node__index             
/* whatsit */
#define backend__special_node__index				0
#define backend__late_special_node__index			1
#define backend__late_lua_node__index				2
#define backend__pdf_literal_node__index			3
#define backend__pdf_late_literal_node__index			4
#define backend__pdf_refobj_node__index				5
#define backend__pdf_end_link_node__index			6
#define backend__pdf_end_thread_node__index			7
#define backend__pdf_colorstack_node__index			8
#define backend__pdf_setmatrix_node__index			9
#define backend__pdf_save_node__index				10
#define backend__pdf_restore_node__index			11
#define backend__pdf_link_state_node__index			12
#define BACKEND__MAX_WHATSIT__INDEX  backend__pdf_link_state_node__index             
#define backend__pdf_annot_node__index				0
#define backend__pdf_start_link_node__index			1
#define backend__pdf_dest_node__index				2
#define backend__pdf_thread_node__index				3 
#define BACKEND__MAX_WHATSIT_DO__INDEX backend__pdf_thread_node__index
/* control */
#define backend__backend_control_begin_page__index		0
#define backend__backend_control_end_page__index		1
#define backend__backend_control_open_file__index		2
#define backend__backend_control_write_header__index		3
#define BACKEND__MAX_CONTROL__INDEX  backend__backend_control_write_header__index
#define backend__backend_control_push_list__index		0
#define backend__backend_control_pop_list__index		1
#define BACKEND__MAX_CONTROL_LIST__INDEX  backend__backend_control_pop_list__index
#define backend__backend_control_finish_file__index		0
#define BACKEND__MAX_CONTROL_ERR__INDEX backend__backend_control_finish_file__index
#define backend__backend_control_set_reference_point__index	0
#define BACKEND__MAX_CONTROL_REF__INDEX backend__backend_control_set_reference_point__index


/* inline unsigned int BACKEND_WHATSIT_INDEX(int a){ */
/*   switch (a) { */
/*   case special_node:		backend__special_node__index            ; break; */
/*   case late_special_node:	backend__late_special_node__index       ; break; */
/*   case late_lua_node:		backend__late_lua_node__index           ; break; */
/*   case pdf_literal_node:	backend__pdf_literal_node__index        ; break; */
/*   case pdf_late_literal_node:	backend__pdf_late_literal_node__index   ; break; */
/*   case pdf_refobj_node:		backend__pdf_refobj_node__index         ; break; */
/*   case pdf_end_link_node:	backend__pdf_end_link_node__index       ; break; */
/*   case pdf_end_thread_node:	backend__pdf_end_thread_node__index     ; break; */
/*   case pdf_colorstack_node:	backend__pdf_colorstack_node__index     ; break; */
/*   case pdf_setmatrix_node:	backend__pdf_setmatrix_node__index      ; break; */
/*   case pdf_save_node:		backend__pdf_save_node__index           ; break; */
/*   case pdf_restore_node:	backend__pdf_restore_node__index        ; break; */
/*   case pdf_link_state_node:	backend__pdf_link_state_node__index     ; break; */
/*   default:			BACKEND__MAX_WHATSIT__INDEX+1; */
/*   } */
/* } */


# define BACKEND_WHATSIT_INDEX(a) \
( (a)==special_node		?       backend__special_node__index            \
: (a)==late_special_node	?	backend__late_special_node__index       \
: (a)==late_lua_node		?	backend__late_lua_node__index           \
: (a)==pdf_literal_node		?	backend__pdf_literal_node__index        \
: (a)==pdf_late_literal_node	?	backend__pdf_late_literal_node__index   \
: (a)==pdf_refobj_node		?	backend__pdf_refobj_node__index         \
: (a)==pdf_end_link_node	?	backend__pdf_end_link_node__index       \
: (a)==pdf_end_thread_node	?	backend__pdf_end_thread_node__index     \
: (a)==pdf_colorstack_node	?	backend__pdf_colorstack_node__index     \
: (a)==pdf_setmatrix_node	?	backend__pdf_setmatrix_node__index      \
: (a)==pdf_save_node		?	backend__pdf_save_node__index           \
: (a)==pdf_restore_node		?	backend__pdf_restore_node__index        \
: (a)==pdf_link_state_node	?	backend__pdf_link_state_node__index     \
: BACKEND__MAX_WHATSIT__INDEX+1)

/* inline unsigned int BACKEND_WHATSIT_DO_INDEX(int a){ */
/*   switch (a) { */
/*   case pdf_annot_node:		backend__pdf_annot_node__index;		break;           */
/*   case pdf_start_link_node:	backend__pdf_start_link_node__index;	break;      */
/*   case pdf_dest_node:		backend__pdf_dest_node__index;		break;            */
/*   case pdf_thread_node:		backend__pdf_thread_node__index;	break;          */
/*   default: BACKEND__MAX_WHATSIT_DO__INDEX+1; */
/*   } */
/* } */

# define BACKEND_WHATSIT_DO_INDEX(a) \
( (a)==pdf_annot_node		?	backend__pdf_annot_node__index      \
: (a)==pdf_start_link_node	?	backend__pdf_start_link_node__index \
: (a)==pdf_dest_node		?	backend__pdf_dest_node__index       \
: (a)==pdf_thread_node		?	backend__pdf_thread_node__index     \
: BACKEND__MAX_WHATSIT_DO__INDEX+1)


typedef enum {
    backend_control_push_list = 0,
    backend_control_pop_list,
    backend_control_begin_page,
    backend_control_end_page,
    backend_control_open_file,
    backend_control_write_header,
    backend_control_finish_file,
    backend_control_set_reference_point
} backend_control_types ;

//typedef void (*backend_function) (); /* variadic arguments */
typedef void (*backend_function_node_rule) (PDF pdf, halfword q, scaledpos size, int callback_id); 
typedef void (*backend_function_node_glyph) (PDF pdf, internal_font_number f, int c, int ex);
typedef void (*backend_function_whatsit) (PDF pdf, halfword p);
typedef void (*backend_function_whatsit_do) (PDF pdf, halfword p, halfword parent_box, scaledpos cur);
typedef void (*backend_function_control) (PDF pdf);
typedef void (*backend_function_control_list) (PDF pdf, scaledpos *saved_pos, int *saved_loc);
typedef void (*backend_function_control_err) (PDF pdf, int fatal_error);
typedef void (*backend_function_control_ref) (PDF pdf, posstructure *refpoint);




/* typedef struct { */
/*     char *name;                    /\* name of the backend *\/ */
/*     backend_function *node_fu;     /\* array of node output functions *\/ */
/*     backend_function *whatsit_fu;  /\* array of whatsit output functions *\/ */
/*     backend_function *control_fu;  /\* array of whatsit output functions *\/ */
/* } backend_struct; */

typedef struct {
  char *name;                    /* name of the backend */
  backend_function_node_rule *node_rule_fu;
  backend_function_node_glyph *node_glyph_fu;
  backend_function_whatsit *whatsit_fu;
  backend_function_whatsit_do *whatsit_do_fu;
  backend_function_control *control_fu;
  backend_function_control_list *control_list_fu;
  backend_function_control_err *control_err_fu;
  backend_function_control_ref *control_ref_fu;
} backend_struct;



/* extern pos_info_structure pos_info; */

extern backend_function_node_rule *backend_out_node_rule;
extern backend_function_node_glyph *backend_out_node_glyph;
extern backend_function_whatsit *backend_out_whatsit;
extern backend_function_whatsit_do *backend_out_whatsit_do;
extern backend_function_control *backend_out_control;
extern backend_function_control_list *backend_out_control_list;
extern backend_function_control_err *backend_out_control_err;
extern backend_function_control_ref *backend_out_control_ref;

/* get_o_mode translates from output_mode to output_mode_used */
/* fix_o_mode freezes output_mode as soon as anything goes through the backend */

/*
    extern void check_o_mode(PDF pdf, const char *s, int o_mode, boolean errorflag);
    extern void ensure_output_file_open(PDF pdf, const char *ext);
*/

extern void fix_o_mode(void);
extern output_mode get_o_mode(void);

extern void init_backend_functionpointers(output_mode o_mode);

#endif
