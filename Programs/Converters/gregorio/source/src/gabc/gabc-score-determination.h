/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header shares definitions between the score parser and lexer.
 *
 * Gregorio score determination from gabc.
 * Copyright (C) 2006-2019 The Gregorio Project (see CONTRIBUTORS.md)
 *
 * This file is part of Gregorio.
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with 
 * this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#ifndef GABC_SCORE_DETERMINATION_H
#define GABC_SCORE_DETERMINATION_H

#include "bool.h"
#include "struct.h"
#include "gabc.h"

/* The bits in this enum are named to correspond with the _BEGIN/_END tokens */
typedef enum {
    SB_I = 0x01,
    SB_B = 0x02,
    SB_TT = 0x04,
    SB_SC = 0x08,
    SB_UL = 0x10,
    SB_C = 0x20,
    SB_ELISION = 0x40
} gabc_style_bits;

typedef union gabc_score_determination_lval_t {
    char *text;
    char character;
} gabc_score_determination_lval_t;

#define YYSTYPE gabc_score_determination_lval_t
#define YYSTYPE_IS_DECLARED 1

#define YY_DECL \
    int gabc_score_determination_lex(gabc_style_bits *const styles)
YY_DECL;

#define YYLTYPE gregorio_scanner_location

void gabc_suppress_extra_custos_at_linebreak(gregorio_score *score);
void gabc_fix_custos_pitches(gregorio_score *score_to_check);
bool gabc_check_score_integrity(gregorio_score *score_to_check);
bool gabc_check_infos_integrity(gregorio_score *score_to_check);
void gabc_determine_oriscus_orientation(const gregorio_score *score);
void gabc_determine_punctum_inclinatum_orientation(const gregorio_score *score);
void gabc_determine_ledger_lines(const gregorio_score *const score);

#endif
