/*

Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
Copyright 2006-2013 Taco Hoekwater <taco@luatex.org>

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
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"
#include "lua/luatex-api.h"

/*tex

    Some macros for processing character packets.

*/

#define packet_number(fw) {    \
    fw = *(vfp++);             \
    fw = fw * 256 + *(vfp++);  \
    fw = fw * 256 + *(vfp++);  \
    fw = fw * 256 + *(vfp++);  \
}

#define packet_scaled(a, fs) {  \
    int fw;                     \
    fw = *(vfp++);              \
    if (fw > 127)               \
        fw = fw - 256;          \
    fw = fw * 256 + *(vfp++);   \
    fw = fw * 256 + *(vfp++);   \
    fw = fw * 256 + *(vfp++);   \
    a = store_scaled_f(fw, fs); \
}

vf_struct *new_vfstruct(void)
{
    vf_struct *vp = (vf_struct *) xmalloc(sizeof(vf_struct));
    vp->packet_stack_level = vp->packet_stack_minlevel = 0;
    vp->packet_stack = (packet_stack_record *) xmalloc(packet_stack_size * sizeof(packet_stack_record));
    vp->lf = 0;
    vp->fs_f = 0;
    vp->packet_cur_s = 0;
    vp->refpos = NULL;
    vp->vflua = false;
    return vp;
}

/*tex

     Count the number of bytes in a command packet.
*/

int vf_packet_bytes(charinfo * co)
{
    eight_bits *vf_packets, *vfp;
    unsigned k;
    int cmd;
    vfp = vf_packets = get_charinfo_packets(co);
    if (vf_packets == NULL) {
        return 0;
    }
    while ((cmd = *(vfp++)) != packet_end_code) {
        switch (cmd) {
        case packet_nop_code:
        case packet_pop_code:
        case packet_push_code:
            break;
        case packet_char_code:
        case packet_down_code:
        case packet_font_code:
        case packet_image_code:
        case packet_node_code:
        case packet_right_code:
            vfp += 4;
            break;
        case packet_rule_code:
            vfp += 8;
            break;
        case packet_pdf_mode:
            vfp += 4;
            break;
        case packet_pdf_code:
            vfp += 4;
            /*tex Plus a string so we fall through: */
        case packet_special_code:
            /*tex |+4| */
            packet_number(k);
            vfp += (int) k;
            break;
        default:
            normal_error("vf", "invalid DVI command (1)");
        }
    };
    return (vfp - vf_packets);
}

/*tex

    Typeset the \.{DVI} commands in the character packet for character |c| in
    current font |f|.
*/

const char *packet_command_names[] = {
    /*tex |slot| maps to |char| and |font| */
    "char",
    "font",
    "pop",
    "push",
    "special",
    "image",
    "right",
    "down",
    "rule",
    "node",
    "nop",
    "end",
    /*tex the next one is not (yet) supported */
    "scale",
    "lua",
    "pdf",
    NULL
};

static float packet_float(eight_bits ** vfpp)
{
    unsigned int i;
    union U {
        float a;
        eight_bits b[sizeof(float)];
    } u;
    eight_bits *vfp = *vfpp;
    for (i = 0; i < sizeof(float); i++)
        u.b[i] = *(vfp++);
    *vfpp = vfp;
    return u.a;
}

/*tex

    The |do_vf_packet| procedure is called in order to interpret the character
    packet for a virtual character. Such a packet may contain the instruction to
    typeset a character from the same or an other virtual font; in such cases
    |do_vf_packet| calls itself recursively. The recursion level, i.e., the
    number of times this has happened, is kept in the global variable
    |packet_cur_s| and should not exceed |packet_max_recursion|.
*/

void do_vf_packet(PDF pdf, internal_font_number vf_f, int c, int ex_glyph)
{
    eight_bits *vfp;
    posstructure *save_posstruct, localpos;
    vf_struct *save_vfstruct, localvfstruct, *vp;
    int cmd, w, mode;
    unsigned k;
    scaledpos size;
    scaled i;
    str_number s;
    float f;
    packet_stack_record *mat_p;
    vfp = get_charinfo_packets(get_charinfo(vf_f, c));
    save_posstruct = pdf->posstruct;
    /*tex use local structure for recursion */
    pdf->posstruct = &localpos;
    localpos.pos = save_posstruct->pos;
    /*tex invariably for vf */
    localpos.dir = dir_TLT;
    save_vfstruct = pdf->vfstruct;
    vp = pdf->vfstruct = &localvfstruct;
    localvfstruct = *save_vfstruct;
    vp->packet_stack_minlevel = ++(vp->packet_stack_level);
    vp->lf = 0;
    vp->fs_f = font_size(vf_f);
    vp->ex_glyph = ex_glyph;
    vp->packet_cur_s++;
    if (vp->packet_cur_s == packet_max_recursion)
        overflow("max level recursion of virtual fonts", packet_max_recursion);
    vp->refpos = save_posstruct;
    vp->vflua = false;
    mat_p = &(vp->packet_stack[vp->packet_stack_level]);
    mat_p->c0 = 1.0;
    mat_p->c1 = 0.0;
    mat_p->c2 = 0.0;
    mat_p->c3 = 1.0;
    mat_p->pos.h = 0;
    mat_p->pos.v = 0;
    while ((cmd = *(vfp++)) != packet_end_code) {
        switch (cmd) {
            case packet_font_code:
                packet_number(vp->lf);
                break;
            case packet_push_code:
                vp->packet_stack_level++;
                if (vp->packet_stack_level == packet_stack_size)
                    normal_error("vf", "packet_stack_level overflow");
                vp->packet_stack[vp->packet_stack_level] = *mat_p;
                mat_p = &(vp->packet_stack[vp->packet_stack_level]);
                break;
            case packet_pop_code:
                if (vp->packet_stack_level == vp->packet_stack_minlevel)
                    normal_error("vf", "packet_stack_level underflow");
                vp->packet_stack_level--;
                mat_p = &(vp->packet_stack[vp->packet_stack_level]);
                break;
            case packet_char_code:
                packet_number(k);
                /*tex We also check if |c == k| and |font(c) == font(k)| */
                if (!char_exists(vp->lf, (int) k)) {
                    char_warning(vp->lf, (int) k);
                } else if (! ((c == k && vp->lf == vf_f)) && (has_packet(vp->lf, (int) k))) {
                    do_vf_packet(pdf, vp->lf, (int) k, ex_glyph);
                } else {
                    backend_out[glyph_node] (pdf, vp->lf, (int) k, ex_glyph);
                }
                w = char_width(vp->lf, (int) k);
                if (ex_glyph != 0 && w != 0)
                    w = round_xn_over_d(w, 1000 + ex_glyph, 1000);
                mat_p->pos.h += w;
                break;
            case packet_rule_code:
                packet_scaled(size.v, vp->fs_f);
                packet_scaled(size.h, vp->fs_f);
                if (ex_glyph != 0 && size.h > 0)
                    size.h = round_xn_over_d(size.h, 1000 + ex_glyph, 1000);
                if (size.h > 0 && size.v > 0)
                    backend_out[rule_node](pdf, 0, size);
                mat_p->pos.h += size.h;
                break;
            case packet_right_code:
                packet_scaled(i, vp->fs_f);
                if (ex_glyph != 0 && i != 0)
                    i = round_xn_over_d(i, 1000 + ex_glyph, 1000);
                mat_p->pos.h += i;
                break;
            case packet_down_code:
                packet_scaled(i, vp->fs_f);
                mat_p->pos.v += i;
                break;
            case packet_pdf_code:
                packet_number(mode);
                packet_number(k);
                str_room(k);
                while (k > 0) {
                    k--;
                    append_char(*(vfp++));
                }
                s = make_string();
                pdf_literal(pdf, s, mode, false);
                flush_str(s);
                break;
            case packet_pdf_mode:
                packet_number(mode);
                pdf_literal_set_mode(pdf, mode);
                break;
            case packet_special_code:
                packet_number(k);
                str_room(k);
                while (k > 0) {
                    k--;
                    append_char(*(vfp++));
                }
                s = make_string();
                pdf_literal(pdf, s, scan_special, false);
                flush_str(s);
                break;
            case packet_lua_code:
                packet_number(k);
                vp->vflua = true;
                luacall_vf(k, vf_f, c);
                /*tex

                    We don't release as we (can ) flush multiple times, so no:

                    \starttyping
                    luaL_unref(Luas, LUA_REGISTRYINDEX, k);
                    \stoptyping

                    here!

                */
                vp->vflua = false;
                break;
            case packet_image_code:
                packet_number(k);
                vf_out_image(pdf, k);
                break;
            case packet_node_code:
                packet_number(k);
                hlist_out(pdf, (halfword) k, 0);
                break;
            case packet_nop_code:
                break;
            case packet_scale_code:
                /*tex This is not yet supported in the backend. */
                f = packet_float(&vfp);
                mat_p->c0 = mat_p->c0 * f;
                mat_p->c3 = mat_p->c3 * f;
                /* pdf->pstruct->scale = f; */
                pdf->pstruct->need_tm = true;
                pdf->pstruct->need_tf = true;
                break;
            default:
                normal_error("vf", "invalid DVI command (2)");
        }
        /*tex The trivial case, always |TLT|. */
        synch_pos_with_cur(&localpos, save_posstruct, mat_p->pos);
    }
    pdf->posstruct = save_posstruct;
    pdf->vfstruct = save_vfstruct;
}

int *packet_local_fonts(internal_font_number f, int *num)
{
    int c, cmd, lf, k, l, i;
    int localfonts[256] = { 0 };
    int *lfs;
    charinfo *co;
    eight_bits *vf_packets, *vfp;
    k = 0;
    for (c = font_bc(f); c <= font_ec(f); c++) {
        if (quick_char_exists(f, c)) {
            co = get_charinfo(f, c);
            vfp = vf_packets = get_charinfo_packets(co);
            if (vf_packets == NULL)
                continue;
            while ((cmd = *(vfp++)) != packet_end_code) {
                switch (cmd) {
                    case packet_font_code:
                        packet_number(lf);
                        for (l = 0; l < k; l++) {
                            if (localfonts[l] == lf) {
                                break;
                            }
                        }
                        if (l == k) {
                            localfonts[k++] = lf;
                        }
                        break;
                    case packet_nop_code:
                    case packet_pop_code:
                    case packet_push_code:
                        break;
                    case packet_char_code:
                    case packet_down_code:
                    case packet_image_code:
                    case packet_node_code:
                    case packet_right_code:
                        vfp += 4;
                        break;
                    case packet_rule_code:
                        vfp += 8;
                        break;
                    case packet_special_code:
                        packet_number(i);
                        vfp += i;
                        break;
                    default:
                        normal_error("vf", "invalid DVI command (3)");
                }
            }
        }
    }
    *num = k;
    if (k > 0) {
        lfs = xmalloc((unsigned) ((unsigned) k * sizeof(int)));
        memcpy(lfs, localfonts, (size_t) ((unsigned) k * sizeof(int)));
        return lfs;
    }
    return NULL;
}

void replace_packet_fonts(internal_font_number f, int *old_fontid, int *new_fontid, int count)
{
    int c, cmd, lf, k, l;
    charinfo *co;
    eight_bits *vf_packets, *vfp;
    for (c = font_bc(f); c <= font_ec(f); c++) {
        if (quick_char_exists(f, c)) {
            co = get_charinfo(f, c);
            vfp = vf_packets = get_charinfo_packets(co);
            if (vf_packets == NULL)
                continue;
            while ((cmd = *(vfp++)) != packet_end_code) {
                switch (cmd) {
                    case packet_font_code:
                        packet_number(lf);
                        for (l = 0; l < count; l++) {
                            if (old_fontid[l] == lf) {
                                break;
                            }
                        }
                        if (l < count) {
                            k = new_fontid[l];
                            *(vfp - 4) = (eight_bits)
                                ((k & 0xFF000000) >> 24);
                            *(vfp - 3) = (eight_bits)
                                ((k & 0x00FF0000) >> 16);
                            *(vfp - 2) = (eight_bits)
                                ((k & 0x0000FF00) >> 8);
                            *(vfp - 1) = (eight_bits) (k & 0x000000FF);
                        }
                        break;
                    case packet_nop_code:
                    case packet_pop_code:
                    case packet_push_code:
                        break;
                    case packet_char_code:
                    case packet_down_code:
                    case packet_image_code:
                    case packet_node_code:
                    case packet_right_code:
                    case packet_rule_code:
                        vfp += 8;
                        break;
                    case packet_pdf_mode:
                        vfp += 4;
                        break;
                    case packet_pdf_code:
                        vfp += 4;
                        /*tex Plus a string so we fall through. */
                    case packet_special_code:
                        packet_number(k);
                        vfp += k;
                        break;
                    default:
                        normal_error("vf", "invalid DVI command (4)");
                }
            }
        }
    }
}
