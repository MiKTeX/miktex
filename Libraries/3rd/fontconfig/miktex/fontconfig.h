/**
 * @file miktex/fontconfig.h
 * @author Christian Schenk
 * @brief MiKTeX fontconfig utilities
 *
 * @copyright Copyright © 2007-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

const char* miktex_fontconfig_path();

const char* miktex_fc_cachedir();

const char* miktex_fc_default_fonts();

const char* miktex_fontconfig_file();

int miktex_get_fontconfig_config_dirs(char** pPaths, int nPaths);

#if defined(MIKTEX_WINDOWS)
void miktex_close_cache_file(int fd, const char* directory);
#endif

void miktex_report_crt_error(const char* message, ...);

void miktex_report_problem(const char* message, ...);

int miktex_file_delete(const char* path);

#define FONTCONFIG_PATH miktex_fontconfig_path()

#define FC_CACHEDIR miktex_fc_cachedir()

#define FC_DEFAULT_FONTS miktex_fc_default_fonts()

#define FONTCONFIG_FILE miktex_fontconfig_file()
