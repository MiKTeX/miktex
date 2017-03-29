/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This header wraps the generated config_.h to provide version macros.
 *
 * Gregorio configuration headers.
 *
 * Copyright (C) 2015 The Gregorio Project (see CONTRIBUTORS.md)
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

#ifndef CONFIG_H
#define CONFIG_H

#include "config_.h"

#ifdef BRANCH_VERSION
#define GREGORIO_VERSION VERSION "-" BRANCH_VERSION
#else
#define GREGORIO_VERSION VERSION
#endif

#endif
