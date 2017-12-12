/* fcmiktex.h:                                          -*- C++ -*-

   Copyright (C) 2007-2017 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

const char* miktex_fontconfig_path();

const char* miktex_fc_cachedir();

const char* miktex_fc_default_fonts();

const char* miktex_fontconfig_file();

int miktex_get_fontconfig_config_dirs(char** pPaths, int nPaths);

void miktex_close_cache_file(int fd, const char* directory);

void miktex_report_crt_error(const char* message, ...);

void miktex_report_problem(const char* message, ...);

int miktex_file_delete(const char* path);

#define FONTCONFIG_PATH miktex_fontconfig_path()

#define FC_CACHEDIR miktex_fc_cachedir()

#define FC_DEFAULT_FONTS miktex_fc_default_fonts()

#define FONTCONFIG_FILE miktex_fontconfig_file()
