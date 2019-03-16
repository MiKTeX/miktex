/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header file contains UTF-8 encoded strings used by Gregorio
 *
 * Copyright (C) 2015-2019 The Gregorio Project (see CONTRIBUTORS.md)
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

#ifndef UTF8STRINGS_H
#define UTF8STRINGS_H

#define ACCENTED_AE "'\303\246"
#define ACCENTED_OE "'\305\223"

/* This is a superset of Latin vowels which includes some French, Vietnamese,
 * Slavic, Hungarian, and Norwegian vowels which don't interfere with Latin
 * itself.  For something more accurate, the user should consider the use of
 * custom centering rules. */
#define DEFAULT_VOWELS "a\303\240\303\241\303\242\304\203\304\205\303\245A\303\200\303\201\303\202\304\202\304\204\303\205e\303\250\303\251\303\252\303\253\304\233\304\231E\303\210\303\211\303\212\303\213\304\232\304\230i\303\254\303\255\303\256I\303\214\303\215\303\216" \
    "o\303\262\303\263\303\264\306\241\305\221\303\270O\303\222\303\223\303\224\306\240\305\220\303\230u\303\271\303\272\303\273\306\260\305\257\305\261U\303\231\303\232\303\233\306\257\305\256\305\260y\341\273\263\303\275Y\341\273\262\303\235\303\246\307\275\303\206\307\274\305\223\305\222"

#endif
