/* extensions.h

   Copyright 2009 Taco Hoekwater <taco@luatex.org>

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


#ifndef EXTENSIONS_H
#  define EXTENSIONS_H

extern alpha_file write_file[last_file_selector+1];
extern halfword write_file_mode[last_file_selector+1];
extern halfword write_file_translation[last_file_selector+1];
extern boolean write_open[last_file_selector+1];

# define valid_write_file(n)    ((n>=0) && (n<=last_file_selector))
# define file_can_be_written(n) (valid_write_file(n) && write_open[n])

extern void expand_macros_in_tokenlist(halfword p);
extern void write_out(halfword p);
extern void finalize_write_files(void);
extern void initialize_write_files(void);
extern void close_write_file(int id);
extern boolean open_write_file(int id, char *fn);

extern scaled neg_wd;
extern scaled pos_wd;
extern scaled neg_ht;

extern halfword write_loc;

extern void do_extension(int immediate);

/*
User defined whatsits can be inserted into node lists to pass data
along from one lua call to another without interference from the
typesetting engine itself. Each has an id, a type, and a value. The
type of the value depends on the |user_node_type| field.
*/

extern void new_whatsit(int s);

extern halfword concat_tokens(halfword q, halfword r);

/*
The \.{\\pagediscards} and \.{\\splitdiscards} commands share the
command code |un_vbox| with \.{\\unvbox} and \.{\\unvcopy}, they are
distinguished by their |chr_code| values |last_box_code| and
|vsplit_code|.  These |chr_code| values are larger than |box_code| and
|copy_code|.
*/

extern boolean *eof_seen;       /* has eof been seen? */
extern void print_group(boolean e);
extern void group_trace(boolean e);
extern save_pointer *grp_stack; /* initial |cur_boundary| */
extern halfword *if_stack;      /* initial |cond_ptr| */
extern void group_warning(void);
extern void if_warning(void);
extern void file_warning(void);

extern halfword last_line_fill; /* the |par_fill_skip| glue node of the new paragraph */

#  define get_tex_dimen_register(j) dimen(j)
#  define get_tex_skip_register(j) skip(j)
#  define get_tex_mu_skip_register(j) mu_skip(j)
#  define get_tex_count_register(j) count(j)
#  define get_tex_attribute_register(j) attribute(j)
#  define get_tex_box_register(j) box(j)

extern int  get_tex_extension_count_register(int i);
extern void set_tex_extension_count_register(int i, int d);
extern int  get_tex_extension_dimen_register(int i);
extern void set_tex_extension_dimen_register(int i, int d);
extern int  get_tex_extension_toks_register (int i);

extern int set_tex_dimen_register(int j, scaled v);
extern int set_tex_skip_register(int j, halfword v);
extern int set_tex_mu_skip_register(int j, halfword v);
extern int set_tex_count_register(int j, scaled v);
extern int set_tex_box_register(int j, scaled v);
extern int set_tex_attribute_register(int j, scaled v);
extern int get_tex_toks_register(int l);
extern int set_tex_toks_register(int j, lstring s);
extern int scan_tex_toks_register(int j, int c, lstring s);
extern scaled get_tex_box_width(int j);
extern int set_tex_box_width(int j, scaled v);
extern scaled get_tex_box_height(int j);
extern int set_tex_box_height(int j, scaled v);
extern scaled get_tex_box_depth(int j);
extern int set_tex_box_depth(int j, scaled v);

/* Synctex variables */

extern int synctexoption;
extern int synctexoffset;

/* Here are extra variables for Web2c. */

extern pool_pointer edit_name_start;
extern int edit_name_length, edit_line;
extern int ipcon;
extern boolean stop_at_space;
extern int shellenabledp;
extern int restrictedshell;
extern char *output_comment;
extern boolean debug_format_file;

extern int last_saved_box_index ;
extern int last_saved_image_index ;
extern int last_saved_image_pages ;
extern scaledpos last_position ;

typedef enum {
    /* traditional extensions */
    open_code = 0,
    write_code,
    close_code,
    reserved_extension_code, // 3: we moved special below immediate //
    reserved_immediate_code, // 4: same number as main codes, expected value //
    /* backend specific implementations */
    special_code,
    save_box_resource_code,
    use_box_resource_code,
    save_image_resource_code,
    use_image_resource_code,
    /* backend */
    dvi_extension_code,
    pdf_extension_code,
} extension_codes ;

#endif
