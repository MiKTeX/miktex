/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header prototypes the "main" entry points for reading and writing data.
 *
 * Copyright (C) 2008-2019 The Gregorio Project (see CONTRIBUTORS.md)
 * 
 * This file is part of Gregorio.
 *
 * Gregorio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gregorio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gregorio.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLUGINS_H
#define PLUGINS_H

#include "bool.h"

void dump_write_score(FILE *f, gregorio_score *score);

void dump_write_characters(FILE *const f,
        const gregorio_character *current_character);

gregorio_score *gabc_read_score(FILE *f_in, bool point_and_click);

void gabc_write_score(FILE *f, gregorio_score *score);

void gregoriotex_write_score(FILE *f, gregorio_score *score,
        const char *point_and_click_filename);

#endif
