/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header shares definitions between the vowel parser and lexer.
 *
 * Copyright (C) 2015-2019 The Gregorio Project (see CONTRIBUTORS.md)
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

#define YYSTYPE char *
#define YYSTYPE_IS_DECLARED 1

int gregorio_vowel_rulefile_lex(void);
#define YY_DECL int gregorio_vowel_rulefile_lex(void)

