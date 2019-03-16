/*
 * Gregorio is a program that translates gabc files to GregorioTeX
 * This file implements the command line interface of Gregorio.
 *
 * Copyright (C) 2006-2019 The Gregorio Project (see CONTRIBUTORS.md)
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

static const char *copyright =
"Copyright (C) 2006-2019 Gregorio Project authors (see CONTRIBUTORS.md)";

#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef USE_KPSE
#include <kpathsea/kpathsea.h>
#else
#include <getopt.h>
#endif
#include <string.h> /* for strcmp */
#include <locale.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include "struct.h"
#include "plugins.h"
#include "messages.h"
#include "characters.h"
#include "support.h"
#include "gabc/gabc.h"
#include "vowel/vowel.h"

#ifndef MODULE_PATH_ENV
#define MODULE_PATH_ENV        "MODULE_PATH"
#endif

typedef enum gregorio_file_format {
    FORMAT_UNSET = 0,
    GABC,
    GTEX,
    DUMP
} gregorio_file_format;

#define GABC_STR "gabc"
#define GTEX_STR "gtex"
#define DUMP_STR "dump"

#define DEFAULT_INPUT_FORMAT    GABC
#define DEFAULT_OUTPUT_FORMAT   GTEX

/* realpath is not in mingw32 */
#ifdef _WIN32
/* _MAX_PATH is being passed for the maxLength (third) argument of _fullpath,
 * but we are always passing NULL for the absPath (first) argument, so it will
 * be ignored per the MSDN documentation */
#define gregorio_realpath(path,resolved_path) _fullpath(resolved_path,path,_MAX_PATH)
#else
#ifdef FUNC_REALPATH_WORKS
#define gregorio_realpath(path,resolved_path) realpath(path,resolved_path)
#else
/* When realpath doesn't work (on an older system), we are forced to use
 * PATH_MAX to allocate a buffer */
#define gregorio_realpath(path,resolved_path) realpath(path,gregorio_malloc(PATH_MAX))
#endif /* FUNC_REALPATH_WORKS */
#endif /* _WIN32 */

/* define_path attempts to canonicalize the pathname of a given string */
static char *define_path(char *current_directory, char *string)
{
    int length;
    char *file_name;
    char *temp_name;
    char *base_name;
#ifdef _WIN32
    char *last_backslash;
#endif

    temp_name = gregorio_strdup(string);
    base_name = strrchr(temp_name, '/');
#ifdef _WIN32
    last_backslash = strrchr(temp_name, '\\');
    if (last_backslash > base_name) {
        base_name = last_backslash;
    }
#endif
    if (base_name) {
        /* some path was supplied */

        *base_name = '\0';
        base_name++;

        /* try to resolve it */
        file_name = gregorio_realpath(temp_name, NULL);
        if (!file_name) {
            /* it's not reasonable to cover this failure in testing */
            /* LCOV_EXCL_START */
            fprintf(stderr, "the directory %s for %s does not exist\n",
                    temp_name, base_name);
            gregorio_exit(1);
            /* LCOV_EXCL_STOP */
        }
    } else {
        /* no path was supplied */
        base_name = string;
        file_name = gregorio_malloc(
                strlen(current_directory) + strlen(base_name) + 2);
        strcpy(file_name, current_directory);
    }

    /* build the file name */
    length = strlen(file_name);
    file_name = gregorio_realloc(file_name, length + strlen(base_name) + 2);
    file_name[length] = '/';
    strcpy(file_name + length + 1, base_name);

    free(temp_name);
    return file_name;
}

/* function that returns the filename without the extension */
static char *get_base_filename(char *fbasename)
{
    char *p;
    int l;
    char *ret;
    p = strrchr(fbasename, '.');
    if (!p || strchr(p, '/') || strchr(p, '\\')) {
        return gregorio_strdup(fbasename);
    }
    l = strlen(fbasename) - strlen(p);
    ret = (char *) gregorio_malloc(l + 1);
    gregorio_snprintf(ret, l + 1, "%s", fbasename);
    ret[l] = '\0';
    return ret;
}

/* function that adds the good extension to a basename (without extension) */
static char *get_output_filename(char *fbasename, const char *extension)
{
    char *output_filename = NULL;
    output_filename =
        (char *) gregorio_malloc((strlen(extension) + strlen(fbasename) + 2));
    output_filename = strcpy(output_filename, fbasename);
    output_filename = strcat(output_filename, ".");
    output_filename = strcat(output_filename, extension);
    return output_filename;
}

static void print_licence(void)
{
    printf("Gregorio: Gregorian chant score engraving.\n\
%s\n\
\n\
This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n", copyright);
    printf("This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\
");
}

static void print_usage(char *name)
{
    printf(_("Usage: %s [OPTION]... [-s | INPUT_FILE]\n\
\nEngrave Gregorian chant scores, convert a gabc file to GregorioTeX.\n\n\
Options:\n\
  -o, --output-file FILE    write output to FILE,\n\
                            default is basename(INPUT_FILE).FORMAT\n\
  -S, --stdout              write output to stdout\n\
  -s, --stdin               read input from stdin\n\
  -l, --messages-file FILE  output messages to FILE (default: stderr)\n\
  -F, --output-format FORMAT\n\
                            specify output format (default: gtex)\n"), name);
printf(_("  -f, --input-format FORMAT\n\
                            specify input format (default: gabc)\n\
  -p, --point-and-click     generate Lilypond point and click information\n\
  -h, --help                print this help message\n\
  -V, --version             print version and exit\n"));
    printf(_("\
  -L, --license             print licence\n\
  -v, --verbose             verbose mode\n\
  -W, --all-warnings        output warnings\n\
  -D, --deprecation-errors  treat deprecation warnings as errors\n\
  -d, --debug               output debug information\n\
\n\
Formats:\n\
  gabc      gabc\n\
  gtex      GregorioTeX\n\
  dump      plain text dump (for debugging purpose)\n\
\n\
See <" PACKAGE_URL "> for general documentation,\n\
GregorioRef.pdf and GregorioNabcRef.pdf for full documentation.\
\n"));
}

static void print_short_usage(char *name)
{
    fprintf(stderr, "Usage: %s [OPTION]... [-s | INPUT_FILE]\n\
Try '%s --help' for more information.\n", name, name);
}

static void check_input_clobber(char *input_file_name, char *output_file_name)
{
    if (input_file_name && output_file_name) {
        char *absolute_input_file_name;
        char *absolute_output_file_name;
        char *current_directory;
        int file_cmp;
        size_t bufsize = 128;
        char *buf = gregorio_malloc(bufsize);
        while ((current_directory = getcwd(buf, bufsize)) == NULL
                && errno == ERANGE && bufsize < MAX_BUF_GROWTH) {
            free(buf);
            bufsize <<= 1;
            buf = gregorio_malloc(bufsize);
        }
        if (current_directory == NULL) {
            fprintf(stderr, _("can't determine current directory"));
            free(buf);
            gregorio_exit(1);
        }
        absolute_input_file_name = define_path(current_directory, input_file_name);
        absolute_output_file_name = define_path(current_directory, output_file_name);
        file_cmp = strcmp(absolute_input_file_name, absolute_output_file_name);
        if (file_cmp == 0) {
            fprintf(stderr, "error: refusing to overwrite the input file\n");
        }
        free(buf);
        free(absolute_input_file_name);
        free(absolute_output_file_name);
        if (file_cmp == 0) {
            gregorio_exit(1);
        }
    }
}

static char *encode_point_and_click_filename(char *input_file_name)
{
    /* percent-encoding favors capital hex digits */
    static const char *const hex = "0123456789ABCDEF";
    char *filename, *result = NULL, *r = NULL, *p;

    filename = gregorio_realpath(input_file_name, NULL);
    if (!filename) {
        /* it's not reasonable to generate the system error that would cause
         * this to fail */
        /* LCOV_EXCL_START */
        fprintf(stderr, "error: unable to resolve %s\n", input_file_name);
        gregorio_exit(1);
        /* LCOV_EXCL_STOP */
    }

    /* 2 extra characters for a possible leading slash and final NUL */
    r = result = gregorio_malloc(strlen(filename) * 4 + 2);

#ifdef _WIN32
    *(r++) = '/';
#endif

    for (p = filename; *p; ++p) {
#ifdef _WIN32
        if (*p == '\\') {
            *p = '/';
        }
#endif

        /* note that -, _ and ~ are conspicuously missing from this list
         * because they cause trouble in TeX; we will percent-encode them */
        if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p < 'z')
                || (*p >= '0' && *p <= '9') || *p == '.' || *p == '/'
#ifdef _WIN32
                || *p == ':'
#endif
                ) {
            *(r++) = *p;
        }
        else {
            /* percent-encode anything else */
            *(r++) = '\\'; /* must escape it because it's TeX */
            *(r++) = '%';
            *(r++) = hex[(*p >> 4) & 0x0FU];
            *(r++) = hex[*p & 0x0FU];
        }
    }

    *r = '\0';

    free(filename);
    return result;
}

int main(int argc, char **argv)
{
    int c;

    char *input_file_name = NULL;
    char *output_file_name = NULL;
    char *output_basename = NULL;
    char *error_file_name = NULL;
    FILE *input_file = NULL;
    FILE *output_file = NULL;
    FILE *error_file = NULL;
    gregorio_file_format input_format = FORMAT_UNSET;
    gregorio_file_format output_format = FORMAT_UNSET;
    gregorio_verbosity verb_mode = 0;
    bool deprecation_errors = false;
    bool point_and_click = false;
    char *point_and_click_filename = NULL;
    bool debug = false;
    bool must_print_short_usage = false;
    int option_index = 0;
    static struct option long_options[] = {
        {"output-file", 1, 0, 'o'},
        {"stdout", 0, 0, 'S'},
        {"output-format", 1, 0, 'F'},
        {"messages-file", 1, 0, 'l'},
        {"input-format", 1, 0, 'f'},
        {"stdin", 0, 0, 's'},
        {"help", 0, 0, 'h'},
        {"version", 0, 0, 'V'},
        {"licence", 0, 0, 'L'},
        {"verbose", 0, 0, 'v'},
        {"all-warnings", 0, 0, 'W'},
        {"deprecation-errors", 0, 0, 'D'},
        {"point-and-click", 0, 0, 'p'},
        {"debug", 0, 0, 'd'},
    };
    gregorio_score *score = NULL;

    gregorio_support_init("gregorio", argv[0]);

    if (argc == 1) {
        fprintf(stderr, "%s: missing file operand.\n", argv[0]);
        print_short_usage(argv[0]);
        gregorio_exit(0);
    }
    setlocale(LC_CTYPE, "C");

    while (1) {
        c = getopt_long(argc, argv, "o:SF:l:f:shOLVvWDpd",
                        long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
        case 'o':
            if (output_file_name) {
                fprintf(stderr,
                        "warning: several output files declared, %s taken\n",
                        output_file_name);
                must_print_short_usage = true;
                break;
            }
            if (output_file) {  /* means that stdout is defined */
                fprintf(stderr,
                        "warning: can't write to file and stdout, writing on stdout\n");
                must_print_short_usage = true;
                break;
            }
            output_file_name = optarg;
            break;
        case 'S':
            if (output_file_name) {
                fprintf(stderr,
                        "warning: can't write to file and stdout, writing on %s\n",
                        output_file_name);
                must_print_short_usage = true;
                break;
            }
            if (output_file) {  /* means that stdout is defined */
                fprintf(stderr, "warning: option used several times: %c\n", c);
                must_print_short_usage = true;
                break;
            }
            output_file = stdout;
            break;
        case 'F':
            if (output_format) {
                fprintf(stderr,
                        "warning: several output formats declared, first taken\n");
                must_print_short_usage = true;
                break;
            }
            if (!strcmp(optarg, GABC_STR)) {
                output_format = GABC;
                break;
            }
            if (!strcmp(optarg, GTEX_STR)) {
                output_format = GTEX;
                break;
            }
            if (!strcmp(optarg, DUMP_STR)) {
                output_format = DUMP;
                break;
            } else {
                fprintf(stderr, "error: unknown output format: %s\n", optarg);
                print_short_usage(argv[0]);
                gregorio_exit(1);
            }
            break;
        case 'l':
            if (error_file_name) {
                fprintf(stderr,
                        "warning: several error files declared, %s taken\n",
                        error_file_name);
                must_print_short_usage = true;
                break;
            }
            error_file_name = optarg;
            break;
        case 'f':
            if (input_format) {
                gregorio_set_error_out(error_file);
                fprintf(stderr,
                        "warning: several output formats declared, first taken\n");
                must_print_short_usage = true;
                break;
            }
            if (!strcmp(optarg, GABC_STR)) {
                input_format = GABC;
                break;
            } else {
                fprintf(stderr, "error: unknown input format: %s\n", optarg);
                print_short_usage(argv[0]);
                gregorio_exit(1);
            }
            break;
        case 's':
            /* input_file_name will be null here because of the way
             * we use getopt_long */
            assert(!input_file_name);
            if (input_file) { /* means that stdin is defined */
                fprintf(stderr, "warning: option used several times: %c\n", c);
                must_print_short_usage = true;
                break;
            }
            input_file = stdin;
            break;
        case 'h':
            print_usage(argv[0]);
            gregorio_exit(0);
            break;
        case 'V':
            gregorio_print_version(copyright);
            gregorio_exit(0);
            break;
        case 'v':
            if (verb_mode && verb_mode != VERBOSITY_WARNING) {
                fprintf(stderr, "warning: verbose option passed several times\n");
                must_print_short_usage = true;
                break;
            }
            verb_mode = VERBOSITY_INFO;
            break;
        case 'W':
            if (verb_mode == VERBOSITY_WARNING) {
                fprintf(stderr,
                        "warning: all-warnings option passed several times\n");
                must_print_short_usage = true;
                break;
            }
            if (verb_mode != VERBOSITY_INFO) {
                verb_mode = VERBOSITY_WARNING;
            }
            break;
        case 'D':
            if (deprecation_errors) {
                fprintf(stderr, "warning: deprecation-errors option passed "
                        "several times\n");
                must_print_short_usage = true;
                break;
            }
            deprecation_errors = true;
            break;
        case 'L':
            print_licence();
            gregorio_exit(0);
            break;
        case 'p':
            if (point_and_click) {
                fprintf(stderr,
                        "warning: point-and-click option passed several times\n");
                must_print_short_usage = true;
                break;
            }
            point_and_click = true;
            break;
        case 'd':
            if (debug) {
                fprintf(stderr,
                        "warning: debug option passed several times\n");
                must_print_short_usage = true;
                break;
            }
            debug = true;
            break;
        case '?':
            must_print_short_usage = true;
            break;
        default:
            /* not reachable unless there's a programming error */
            /* LCOV_EXCL_START */
            gregorio_fail2(main, "unknown option: %c", c);
            print_short_usage(argv[0]);
            gregorio_exit(1);
            break;
            /* LCOV_EXCL_STOP */
        }
    } /* end of while */
    if (optind == argc) {
        if (!input_file) { /* input not undefined (could be stdin) */
            fprintf(stderr, "%s: missing file operand.\n", argv[0]);
            print_short_usage(argv[0]);
            gregorio_exit(1);
        }
    } else {
        input_file_name = argv[optind++];
        output_basename = get_base_filename(input_file_name);
        if (input_file) {
            fprintf(stderr,
                    "warning: can't read from both stdin and a file, reading from %s\n",
                    input_file_name);
            input_file = NULL;
            must_print_short_usage = true;
        }
    }
    if (optind < argc) {
        must_print_short_usage = true;
        fprintf(stderr, "ignored arguments:");
        for (; optind < argc; ++optind) {
            fprintf(stderr, " %s", argv[optind]);
        }
        fprintf(stderr, "\n");
    }

    if (must_print_short_usage) {
        print_short_usage(argv[0]);
        fprintf(stderr, "Proceeding anyway...\n");
    }

    gregorio_set_debug_messages(debug);
    gregorio_set_deprecation_errors(deprecation_errors);

    if (!input_format) {
        input_format = DEFAULT_INPUT_FORMAT;
    }

    if (!output_format) {
        output_format = DEFAULT_OUTPUT_FORMAT;
    }

    /* then we act... */

    if (!output_file_name && !output_file) {
        if (!output_basename) {
            output_file = stdout;
        } else {
            switch (output_format) {
            case GABC:
                output_file_name =
                    get_output_filename(output_basename, "gabc");
                break;
            case GTEX:
                output_file_name =
                    get_output_filename(output_basename, "gtex");
                break;
            case DUMP:
                output_file_name =
                    get_output_filename(output_basename, "dump");
                break;
            default:
                /* not reachable unless there's a programming error */
                /* LCOV_EXCL_START */
                fprintf(stderr, "error: unsupported format");
                print_short_usage(argv[0]);
                gregorio_exit(1);
                /* LCOV_EXCL_STOP */
            }
        }
    }

    if (output_basename) {
        free(output_basename);
    }

    if (!output_file) {
        if (!input_file) {
            check_input_clobber(input_file_name, output_file_name);
        }
        gregorio_check_file_access(write, output_file_name, ERROR,
                gregorio_exit(1));
        output_file = fopen(output_file_name, "wb");
        if (!output_file) {
            fprintf(stderr, "error: can't write in file %s", output_file_name);
            gregorio_exit(1);
        }
    }

    /* we always have input_file or input_file_name */
    if (input_file) {
        if (point_and_click) {
            fprintf(stderr,
                    "warning: disabling point-and-click since reading from stdin\n");
        }
    } else {
        gregorio_check_file_access(read, input_file_name, ERROR,
                gregorio_exit(1));
        input_file = fopen(input_file_name, "r");
        if (!input_file) {
            fprintf(stderr, "error: can't open file %s for reading\n",
                    input_file_name);
            gregorio_exit(1);
        }
        if (point_and_click) {
            point_and_click_filename = encode_point_and_click_filename(
                    input_file_name);
        }
    }

    if (!error_file_name) {
        error_file = stderr;
        gregorio_set_error_out(error_file);
    } else {
        gregorio_check_file_access(write, error_file_name, ERROR,
                gregorio_exit(1));
        error_file = fopen(error_file_name, "wb");
        if (!error_file) {
            fprintf(stderr, "error: can't open file %s for writing\n",
                    error_file_name);
            gregorio_exit(1);
        }
        gregorio_set_error_out(error_file);
    }

    if (!verb_mode) {
        verb_mode = VERBOSITY_DEPRECATION;
    }

    gregorio_set_verbosity_mode(verb_mode);

    switch (input_format) {
    case GABC:
        score = gabc_read_score(input_file, point_and_click);
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        fprintf(stderr, "error : invalid input format\n");
        fclose(input_file);
        fclose(output_file);
        gregorio_exit(1);
        break;
        /* LCOV_EXCL_STOP */
    }

    fclose(input_file);
    if (score == NULL) {
        /* score should never be NULL on return from gabc_read_score */
        /* LCOV_EXCL_START */
        fclose(output_file);
        fprintf(stderr, "error in file parsing\n");
        gregorio_exit(1);
        /* LCOV_EXCL_STOP */
    }

    switch (output_format) {
    case GABC:
        gabc_write_score(output_file, score);
        break;
    case GTEX:
        gregoriotex_write_score(output_file, score, point_and_click_filename);
        break;
    case DUMP:
        dump_write_score(output_file, score);
        break;
    default:
        /* not reachable unless there's a programming error */
        /* LCOV_EXCL_START */
        fprintf(stderr, "error : invalid output format\n");
        gregorio_free_score(score);
        fclose(output_file);
        gregorio_exit(1);
        break;
        /* LCOV_EXCL_STOP */
    }
    fclose(output_file);
    if (point_and_click_filename) {
        free(point_and_click_filename);
    }
    gregorio_free_score(score);
    gregorio_vowel_tables_free();
    gabc_score_determination_lex_destroy();
    gabc_notes_determination_lex_destroy();
    gregorio_vowel_rulefile_lex_destroy();
    if (error_file_name) {
        fclose(error_file);
    }

    gregorio_exit(gregorio_get_return_value());
} /* due to exit on prior line, this will never be reached; LCOV_EXCL_LINE */
