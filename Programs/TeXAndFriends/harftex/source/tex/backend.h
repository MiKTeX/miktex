/* to fill */

#ifndef BACKEND_H
#  define BACKEND_H

#include "ptexlib.h"

extern scaled max_v;
extern scaled max_h;
extern boolean doing_leaders;
extern int cur_s;

# define MAX_CONTROL_TYPE 7

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

typedef void (*backend_function) (); /* variadic arguments  */

typedef struct {
    char *name;                    /* name of the backend */
    backend_function *node_fu;     /* array of node output functions */
    backend_function *whatsit_fu;  /* array of whatsit output functions */
    backend_function *control_fu;  /* array of whatsit output functions */
} backend_struct;

/* extern pos_info_structure pos_info; */

extern backend_function *backend_out;
extern backend_function *backend_out_whatsit;
extern backend_function *backend_out_control;

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
