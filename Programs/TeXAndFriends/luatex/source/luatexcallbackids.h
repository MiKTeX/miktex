/* luatexcallbackids.h

   Copyright 2012 Taco Hoekwater <taco@luatex.org>

   This file is part of LuaTeX.

   LuaTeX is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   LuaTeX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#ifndef LUATEXCALLBACKIDS_H
#define LUATEXCALLBACKIDS_H

typedef enum {
    find_write_file_callback = 1,
    find_output_file_callback,
    find_image_file_callback,
    find_format_file_callback,
    find_read_file_callback, open_read_file_callback,
    find_vf_file_callback, read_vf_file_callback,
    find_data_file_callback, read_data_file_callback,
    find_font_file_callback, read_font_file_callback,
    find_map_file_callback, read_map_file_callback,
    find_enc_file_callback, read_enc_file_callback,
    find_type1_file_callback, read_type1_file_callback,
    find_truetype_file_callback, read_truetype_file_callback,
    find_opentype_file_callback, read_opentype_file_callback,
    find_cidmap_file_callback, read_cidmap_file_callback,
    find_pk_file_callback, read_pk_file_callback,
    show_error_hook_callback,
    process_input_buffer_callback, process_output_buffer_callback,
    process_jobname_callback,
    start_page_number_callback, stop_page_number_callback,
    start_run_callback, stop_run_callback,
    define_font_callback,
    pre_output_filter_callback,
    buildpage_filter_callback,
    hpack_filter_callback, vpack_filter_callback,
    glyph_not_found_callback,
    hyphenate_callback,
    ligaturing_callback,
    kerning_callback,
    pre_linebreak_filter_callback,
    linebreak_filter_callback,
    post_linebreak_filter_callback,
    append_to_vlist_filter_callback,
    mlist_to_hlist_callback,
    finish_pdffile_callback,
    finish_pdfpage_callback,
    pre_dump_callback,
    start_file_callback, stop_file_callback,
    show_error_message_callback, show_lua_error_hook_callback,
    show_warning_message_callback,
    hpack_quality_callback, vpack_quality_callback,
    process_rule_callback,
    insert_local_par_callback,
    contribute_filter_callback,
    call_edit_callback,
    build_page_insert_callback,
    glyph_stream_provider_callback,
    font_descriptor_objnum_provider_callback,
    finish_synctex_callback,
    wrapup_run_callback,
    new_graf_callback,
    page_objnum_provider_callback,
    make_extensible_callback,
    process_pdf_image_content_callback,
    total_callbacks,
} callback_callback_types;

/* lcallbacklib.c */

extern int callback_set[];

#  define callback_defined(a) callback_set[a]
/* #  define callback_defined(a) debug_callback_defined(a) */

extern int lua_active;

extern int debug_callback_defined(int i);

extern int run_callback(int i, const char *values, ...);
extern int run_saved_callback(int i, const char *name, const char *values, ...);
extern int run_and_save_callback(int i, const char *values, ...);
extern void destroy_saved_callback(int i);

extern void get_saved_lua_boolean(int i, const char *name, boolean * target);
extern void get_saved_lua_number(int i, const char *name, int *target);
extern void get_saved_lua_string(int i, const char *name, char **target);

extern void get_lua_boolean(const char *table, const char *name, boolean * target);
extern void get_lua_number(const char *table, const char *name, int *target);
extern void get_lua_string(const char *table, const char *name, char **target);


extern char *get_lua_name(int i);

/* texfileio.c */
extern char *luatex_find_file(const char *s, int callback_index);
extern int readbinfile(FILE * f, unsigned char **b, int *s);

#define filetype_unknown 0
#define filetype_tex     1
#define filetype_map     2
#define filetype_image   3
#define filetype_subset  4
#define filetype_font    5

static const char *const filetypes_left[]  = { "?", "(", "{", "<", "<", "<<" } ;
static const char *const filetypes_right[] = { "?", ")", "}", ">", ">", ">>" } ;

#define report_start_file(left,name) do { \
    if (tracefilenames) { \
        int report_id = callback_defined(start_file_callback); \
        if (report_id == 0) { \
            if (left == 1) { \
                /* we only do this for traditional name reporting, normally name is iname */ \
                if (term_offset + strlen(name) > max_print_line - 2) \
                    print_ln(); \
                else if ((term_offset > 0) || (file_offset > 0)) \
                    print_char(' '); \
                tex_printf("%s", filetypes_left[left]); \
                tprint_file_name(NULL, (unsigned char *) name, NULL); \
            } else { \
                tex_printf("%s", filetypes_left[left]); \
                tex_printf("%s", (unsigned char *) name); \
            } \
        } else { \
          /*  (void) run_callback(report_id, "dS->",left,(unsigned char *) fullnameoffile); */ \
            (void) run_callback(report_id, "dS->",left,name); \
        } \
    } \
} while (0)

#define report_stop_file(right) do { \
    if (tracefilenames) { \
        int report_id = callback_defined(stop_file_callback); \
        if (report_id == 0) { \
            tex_printf("%s", filetypes_right[right]); \
        } else { \
            (void) run_callback(report_id, "d->",right); \
        } \
    } \
} while (0)

#endif

#define normal_page_filter(A) lua_node_filter_s(buildpage_filter_callback,lua_key_index(A))
#define checked_page_filter(A) if (!output_active) lua_node_filter_s(buildpage_filter_callback,lua_key_index(A))
#define checked_break_filter(A) if (!output_active) lua_node_filter_s(contribute_filter_callback,lua_key_index(A))
