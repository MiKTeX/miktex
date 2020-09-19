/* automatic.{cc,hh} -- code for automatic mode and interfacing with kpathsea
 *
 * Copyright (c) 2003-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#include <config.h>
#include "automatic.hh"
#include "kpseinterface.h"
#include "util.hh"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#if HAVE_FCNTL_H
# include <fcntl.h>
#endif
#include <algorithm>

#ifdef WIN32
# define mkdir(dir, access) mkdir(dir)
# define COPY_CMD "copy"
# define CMD_SEP "&"
#else
# define COPY_CMD "cp"
# define CMD_SEP ";"
#endif

/* kpathsea may already have defined this */
#ifndef DEV_NULL
# ifdef WIN32
#  define DEV_NULL "NUL"
# else
#  define DEV_NULL "/dev/null"
# endif
#endif

#if HAVE_AUTO_T1DOTLESSJ
enum { T1DOTLESSJ_EXIT_J_NODOT = 2 };
#endif

static String odir[NUMODIR];
static String typeface;
static String vendor;
static String map_file;
#define DEFAULT_VENDOR "lcdftools"
#define DEFAULT_TYPEFACE "unknown"

static const struct {
    const char *name;
    const char *envvar;
    const char *texdir;
} odir_info[] = {
    { "encoding", "ENCODINGDESTDIR", "#fonts/enc/dvips/@#dvips/@" },
    { "TFM", "TFMDESTDIR", "fonts/tfm/%" },
    { "PL", "PLDESTDIR", "fonts/pl/%" },
    { "VF", "VFDESTDIR", "fonts/vf/%" },
    { "VPL", "VPLDESTDIR", "fonts/vpl/%" },
    { "Type 1", "T1DESTDIR", "fonts/type1/%" },
    { "DVIPS map", "DVIPS directory", "#fonts/map/dvips/@#dvips/@" },
    { "DVIPS updmap", "DVIPS directory", "dvips" },
    { "TrueType", "TTFDESTDIR", "fonts/truetype/%" },
    { "OpenType", "OPENTYPEDESTDIR", "fonts/opentype/%" },
    { "Type 42", "T42DESTDIR", "fonts/type42/%" }
};

#if HAVE_KPATHSEA
static String odir_kpathsea[NUMODIR];

static bool writable_texdir_tried = false;
static String writable_texdir;  // always ends with directory separator
static int tds_1_1 = -1;

static bool mktexupd_tried = false;
static String mktexupd;

static String
kpsei_string(char* x)
{
    String s(x);
    free((void*)x);
    return s;
}

static void
look_for_writable_texdir(const char *path_variable, bool create)
{
    String path = kpsei_string(kpsei_path_expand(path_variable));
    while (path && !writable_texdir) {
        const char* colon = std::find(path.begin(), path.end(), kpsei_env_sep_char);
        String texdir = path.substring(path.begin(), colon);
        path = path.substring(colon + 1, path.end());
        if (access(texdir.c_str(), W_OK) >= 0)
            writable_texdir = texdir;
        else if (create && errno != EACCES && mkdir(texdir.c_str(), 0777) >= 0)
            // create file if it doesn't exist already
            writable_texdir = texdir;
    }
    if (writable_texdir && writable_texdir.back() != '/')
        writable_texdir += "/";
}

static void
find_writable_texdir(ErrorHandler *errh, const char *)
{
    look_for_writable_texdir("$TEXMFVAR", true);
    if (!writable_texdir)
        look_for_writable_texdir("$VARTEXMF", false);
    if (!writable_texdir)
        look_for_writable_texdir("$TEXMF", false);
    if (!writable_texdir) {
        errh->warning("no writable directory found in $TEXMFVAR or $TEXMF");
        errh->message("(You probably need to set your TEXMF environment variable; see\n\
the manual for more information. The current TEXMF path is\n\
%<%s%>.)", kpsei_string(kpsei_path_expand("$TEXMF")).c_str());
    }
    writable_texdir_tried = true;
}

static String
get_vendor()
{
    return (vendor ? vendor : String(DEFAULT_VENDOR));
}

static String
get_typeface()
{
    return (typeface ? typeface : String(DEFAULT_TYPEFACE));
}
#endif

bool
set_vendor(const String &s)
{
    bool had = (bool) vendor;
    vendor = s;
    return !had;
}

bool
set_typeface(const String &s, bool override)
{
    bool had = (bool) typeface;
    if (!had || override)
        typeface = s;
    return !had;
}

String
getodir(int o, ErrorHandler *errh)
{
    assert(o >= 0 && o < NUMODIR);

    if (!odir[o] && automatic && odir_info[o].envvar)
        odir[o] = getenv(odir_info[o].envvar);

#if HAVE_KPATHSEA
    if (!odir[o] && automatic && !writable_texdir_tried)
        find_writable_texdir(errh, odir_info[o].name);

    if (!odir[o] && automatic && writable_texdir) {
        String suffix = odir_info[o].texdir;

        // May need to behave differently on TDS 1.1 rather than TDS 1.0.
        if (suffix[0] == '#') {
            // check type of TDS
            if (tds_1_1 < 0) {
                // using a procedure suggested by Olaf Weber
                String encfonts = kpsei_string(kpsei_path_expand("$TEXMFMAIN/fonts/enc"));
                if (!encfonts)
                    encfonts = kpsei_string(kpsei_path_expand("$TEXMFDIST/fonts/enc"));
                tds_1_1 = (encfonts != String());
            }
            if (tds_1_1 == 0)
                suffix = suffix.substring(std::find(suffix.begin() + 1, suffix.end(), '#') + 1, suffix.end());
            else
                suffix = suffix.substring(suffix.begin() + 1, std::find(suffix.begin() + 1, suffix.end(), '#'));
        }

        String dir = writable_texdir + suffix;

        if (dir.back() == '%')
            dir = dir.substring(0, -1) + get_vendor() + "/" + get_typeface();
        else if (dir.back() == '@')
            dir = dir.substring(0, -1) + get_vendor();

        // create parent directories as appropriate
        int slash = writable_texdir.length() - 1;
        while (access(dir.c_str(), F_OK) < 0 && slash < dir.length()) {
            if ((slash = dir.find_left('/', slash + 1)) < 0)
                slash = dir.length();
            String subdir = dir.substring(0, slash);
            if (access(subdir.c_str(), F_OK) < 0
                && !no_create
                && mkdir(subdir.c_str(), 0777) < 0)
                goto kpathsea_done;
        }

        // that's our answer
        odir[o] = dir;
        odir_kpathsea[o] = dir;
    }
  kpathsea_done:
#endif

    if (!odir[o]) {
        if (automatic) {
            errh->warning("%s not specified, placing %s files in %<.%>", odir_info[o].envvar, odir_info[o].name);
#if !HAVE_KPATHSEA
            static int kpathsea_warning = 0;
            if (++kpathsea_warning == 1)
                errh->message("(This version of otftotfm lacks $TEXMF directory support.)");
#endif
        }
        odir[o] = ".";
    }

    while (odir[o].length() && odir[o].back() == '/')
        odir[o] = odir[o].substring(0, -1);

    if (verbose)
        errh->message("placing %s files in %<%s%>", odir_info[o].name, odir[o].c_str());
    return odir[o];
}

void
setodir(int o, const String &value)
{
    assert(o >= 0 && o < NUMODIR);
    odir[o] = value;
}

const char *
odirname(int o)
{
    if (o == NUMODIR) {
        return "default";
    } else {
        assert(o >= 0 && o < NUMODIR);
        return odir_info[o].name;
    }
}

#if HAVE_KPATHSEA
static bool
file_in_kpathsea_odir(int o, const String &file)
{
    return odir_kpathsea[o]
        && file.length() > odir[o].length()
        && memcmp(file.data(), odir[o].data(), odir[o].length()) == 0
        && file[odir[o].length()] == '/';
}
#endif

void
update_odir(int o, String file, ErrorHandler *errh)
{
    assert(o >= 0 && o < NUMODIR);
#if HAVE_KPATHSEA
    if (file.find_left('/') < 0)
        file = odir[o] + "/" + file;

    // exit if this directory was not found via kpathsea, or the file is not
    // in the kpathsea directory
    if (!file_in_kpathsea_odir(o, file))
        return;

    assert(writable_texdir && writable_texdir.length() <= odir[o].length()
           && memcmp(file.data(), writable_texdir.data(), writable_texdir.length()) == 0);

    // divide the filename into portions
    // file == writable_texdir + directory + file
    file = file.substring(writable_texdir.length());
    while (file && file[0] == '/')
        file = file.substring(1);
    int last_slash = file.find_right('/');
    String directory = (last_slash >= 0 ? file.substring(0, last_slash) : String());
    file = file.substring(last_slash >= 0 ? last_slash + 1 : 0);
    if (!file)                  // no filename to update
        return;

    // return if nocreate
    if (no_create) {
        errh->message("would update %sls-R for %s/%s", writable_texdir.c_str(), directory.c_str(), file.c_str());
        return;
    } else if (verbose)
        errh->message("updating %sls-R for %s/%s", writable_texdir.c_str(), directory.c_str(), file.c_str());

    // try to update ls-R ourselves, rather than running mktexupd --
    // mktexupd's runtime is painful: a half second to update a file
    String ls_r = writable_texdir + "ls-R";
    bool success = false;
    if (access(ls_r.c_str(), R_OK) >= 0) // make sure it already exists
        if (FILE *f = fopen(ls_r.c_str(), "a")) {
            fprintf(f, "./%s:\n%s\n", directory.c_str(), file.c_str());
            success = true;
            fclose(f);
        }

    // otherwise, run mktexupd
    if (!success && writable_texdir.find_left('\'') < 0 && directory.find_left('\'') < 0 && file.find_left('\'') < 0) {
        // look for mktexupd script
        if (!mktexupd_tried) {
            mktexupd = kpsei_string(kpsei_find_file("mktexupd", KPSEI_FMT_WEB2C));
            mktexupd_tried = true;
        }

        // run script
        if (mktexupd) {
            String command = mktexupd + " " + shell_quote(writable_texdir + directory) + " " + shell_quote(file);
            int retval = system(command.c_str());
            if (retval == 127)
                errh->error("could not run %<%s%>", command.c_str());
            else if (retval < 0)
                errh->error("could not run %<%s%>: %s", command.c_str(), strerror(errno));
            else if (retval != 0)
                errh->error("%<%s%> failed", command.c_str());
        }
    }
#else
    (void) file, (void) errh;
#endif
}

bool
set_map_file(const String &s)
{
    bool had = (bool) map_file;
    map_file = s;
    return !had;
}

String
installed_type1(const String &otf_filename, const String &ps_fontname, bool allow_generate, ErrorHandler *errh)
{
    (void) otf_filename, (void) allow_generate, (void) errh;

    if (!ps_fontname)
        return String();

#if HAVE_KPATHSEA
# if HAVE_AUTO_CFFTOT1
    if (!(force && allow_generate && otf_filename && otf_filename != "-" && getodir(O_TYPE1, errh))) {
# endif
        // look for .pfb and .pfa
        String file, path;
        if ((file = ps_fontname + ".pfb", path = kpsei_string(kpsei_find_file(file.c_str(), KPSEI_FMT_TYPE1)))
            || (file = ps_fontname + ".pfa", path = kpsei_string(kpsei_find_file(file.c_str(), KPSEI_FMT_TYPE1)))) {
            if (path == "./" + file || path == file) {
                if (verbose)
                    errh->message("ignoring Type 1 file %s found with kpathsea in %<.%>", path.c_str());
            } else {
                if (verbose)
                    errh->message("Type 1 file %s found with kpathsea at %s", file.c_str(), path.c_str());
                return path;
            }
        }
# if HAVE_AUTO_CFFTOT1
    }
# endif
#endif

#if HAVE_AUTO_CFFTOT1
    // if not found, and can generate on the fly, run cfftot1
    if (allow_generate && otf_filename && otf_filename != "-" && getodir(O_TYPE1, errh)) {
        String pfb_filename = odir[O_TYPE1] + "/" + ps_fontname + ".pfb";
        if (pfb_filename.find_left('\'') >= 0 || otf_filename.find_left('\'') >= 0)
            return String();
        String command = "cfftot1 " + shell_quote(otf_filename) + " -n " + shell_quote(ps_fontname) + " " + shell_quote(pfb_filename);
        int retval = mysystem(command.c_str(), errh);
        if (retval == 127)
            errh->error("could not run %<%s%>", command.c_str());
        else if (retval < 0)
            errh->error("could not run %<%s%>: %s", command.c_str(), strerror(errno));
        else if (retval != 0)
            errh->error("%<%s%> failed", command.c_str());
        if (retval == 0) {
            update_odir(O_TYPE1, pfb_filename, errh);
            return pfb_filename;
        }
    }
#endif

    return String();
}

String
installed_type1_dotlessj(const String &otf_filename, const String &ps_fontname, bool allow_generate, ErrorHandler *errh)
{
    (void) otf_filename, (void) allow_generate, (void) errh;

    if (!ps_fontname)
        return String();
    if (verbose)
        errh->message("searching for dotless-j font for %s", ps_fontname.c_str());

    String j_ps_fontname = ps_fontname + "LCDFJ";

#if HAVE_KPATHSEA
# if HAVE_AUTO_T1DOTLESSJ
    if (!(force && allow_generate && getodir(O_TYPE1, errh))) {
# endif
        // look for existing .pfb or .pfa
        String file, path;
        if ((file = j_ps_fontname + ".pfb", path = kpsei_string(kpsei_find_file(file.c_str(), KPSEI_FMT_TYPE1)))
            || (file = j_ps_fontname + ".pfa", path = kpsei_string(kpsei_find_file(file.c_str(), KPSEI_FMT_TYPE1)))) {
            // ignore versions in the current directory
            if (path == "./" + file || path == file) {
                if (verbose)
                    errh->message("ignoring Type 1 file %s found with kpathsea in %<.%>", path.c_str());
            } else {
                if (verbose)
                    errh->message("Type 1 file %s found with kpathsea at %s", file.c_str(), path.c_str());
                return path;
            }
        }
# if HAVE_AUTO_T1DOTLESSJ
    }
# endif
#endif

#if HAVE_AUTO_T1DOTLESSJ
    // if not found, and can generate on the fly, try running t1dotlessj
    if (allow_generate && getodir(O_TYPE1, errh)) {
        if (String base_filename = installed_type1(otf_filename, ps_fontname, allow_generate, errh)) {
            String pfb_filename = odir[O_TYPE1] + "/" + j_ps_fontname + ".pfb";
            if (pfb_filename.find_left('\'') >= 0 || base_filename.find_left('\'') >= 0)
                return String();
            String command = "t1dotlessj " + shell_quote(base_filename) + " -n " + shell_quote(j_ps_fontname) + " " + shell_quote(pfb_filename);
            int retval = mysystem(command.c_str(), errh);
            if (retval == 127)
                errh->warning("could not run %<%s%>", command.c_str());
            else if (retval < 0)
                errh->warning("could not run %<%s%>: %s", command.c_str(), strerror(errno));
            else if (WEXITSTATUS(retval) == T1DOTLESSJ_EXIT_J_NODOT)
                return String("\0", 1);
            else if (retval != 0)
                errh->warning("%<%s%> failed (%d)", command.c_str(), retval);
            if (retval == 0) {
                update_odir(O_TYPE1, pfb_filename, errh);
                return pfb_filename;
            } else
                errh->warning("output font will not contain a dotless-j character");
        }
    }
#endif

    return String();
}

String
installed_truetype(const String &ttf_filename, bool allow_generate, ErrorHandler *errh)
{
    String file = pathname_filename(ttf_filename);

#if HAVE_KPATHSEA
    if (!(force && allow_generate && ttf_filename && ttf_filename != "-" && getodir(O_TRUETYPE, errh))) {
        if (String path = kpsei_string(kpsei_find_file(file.c_str(), KPSEI_FMT_TRUETYPE))) {
            if (path == "./" + file || path == file) {
                if (verbose)
                    errh->message("ignoring TrueType file %s found with kpathsea in %<.%>", path.c_str());
            } else {
                if (verbose)
                    errh->message("TrueType file %s found with kpathsea at %s", file.c_str(), path.c_str());
                return path;
            }
        }
    }
#endif

    // perhaps generate type 42 in the future, for now just copy
    if (allow_generate && ttf_filename && ttf_filename != "-" && getodir(O_TRUETYPE, errh)) {
        String installed_ttf_filename = odir[O_TRUETYPE] + "/" + file;
        if (installed_ttf_filename.find_left('\'') >= 0 || installed_ttf_filename.find_left('\"') >= 0)
            return String();

        int retval;
        if (!same_filename(ttf_filename, installed_ttf_filename)) {
            String command = COPY_CMD " " + shell_quote(ttf_filename) + " " + shell_quote(installed_ttf_filename);
            retval = mysystem(command.c_str(), errh);
            if (retval == 127)
                errh->error("could not run %<%s%>", command.c_str());
            else if (retval < 0)
                errh->error("could not run %<%s%>: %s", command.c_str(), strerror(errno));
            else if (retval != 0)
                errh->error("%<%s%> failed", command.c_str());
        } else {
            if (verbose)
                errh->message("TrueType file %s already located in output directory", installed_ttf_filename.c_str());
            retval = 0;
        }

        if (retval == 0) {
            update_odir(O_TRUETYPE, installed_ttf_filename, errh);
            return installed_ttf_filename;
        }
    }

    return String();
}

String
installed_type42(const String &ttf_filename, const String &ps_fontname, bool allow_generate, ErrorHandler *errh)
{
    (void) allow_generate, (void) ttf_filename, (void) errh;

    if (!ps_fontname)
        return String();

#if HAVE_KPATHSEA
# if HAVE_AUTO_TTFTOTYPE42
    if (!(force && allow_generate && ttf_filename && ttf_filename != "-" && getodir(O_TYPE42, errh))) {
# endif
        // look for .pfb and .pfa
        String file, path;
        if ((file = ps_fontname + ".t42", path = kpsei_string(kpsei_find_file(file.c_str(), KPSEI_FMT_TYPE42)))) {
            if (path == "./" + file || path == file) {
                if (verbose)
                    errh->message("ignoring Type 42 file %s found with kpathsea in %<.%>", path.c_str());
            } else {
                if (verbose)
                    errh->message("Type 42 file %s found with kpathsea at %s", file.c_str(), path.c_str());
                return path;
            }
        }
# if HAVE_AUTO_TTFTOTYPE42
    }
# endif
#endif

#if HAVE_AUTO_TTFTOTYPE42
    // if not found, and can generate on the fly, run ttftotype42
    if (allow_generate && ttf_filename && ttf_filename != "-" && getodir(O_TYPE42, errh)) {
        String t42_filename = odir[O_TYPE42] + "/" + ps_fontname + ".t42";
        if (t42_filename.find_left('\'') >= 0 || ttf_filename.find_left('\'') >= 0)
            return String();
        String command = "ttftotype42 " + shell_quote(ttf_filename) + " " + shell_quote(t42_filename);
        int retval = mysystem(command.c_str(), errh);
        if (retval == 127)
            errh->error("could not run %<%s%>", command.c_str());
        else if (retval < 0)
            errh->error("could not run %<%s%>: %s", command.c_str(), strerror(errno));
        else if (retval != 0)
            errh->error("%<%s%> failed", command.c_str());
        if (retval == 0) {
            update_odir(O_TYPE42, t42_filename, errh);
            return t42_filename;
        }
    }
#endif

    return String();
}

int
update_autofont_map(const String &fontname, String mapline, ErrorHandler *errh)
{
#if HAVE_KPATHSEA
    if (automatic && !map_file && getodir(O_MAP, errh))
        map_file = odir[O_MAP] + "/" + get_vendor() + ".map";
#endif

    if (map_file == "" || map_file == "-")
        fputs(mapline.c_str(), stdout);
    else {
        // report no_create/verbose
        if (no_create) {
            errh->message("would update %s for %s", map_file.c_str(), String(fontname).c_str());
            return 0;
        } else if (verbose)
            errh->message("updating %s for %s", map_file.c_str(), String(fontname).c_str());

        int fd = open(map_file.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd < 0)
            return errh->lerror(map_file, "%s", strerror(errno));
        FILE *f = fdopen(fd, "r+");
        // NB: also change encoding logic if you change this code

#if defined(F_SETLKW) && defined(HAVE_FTRUNCATE)
        {
            struct flock lock;
            lock.l_type = F_WRLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = 0;
            lock.l_len = 0;
            int result;
            while ((result = fcntl(fd, F_SETLKW, &lock)) < 0 && errno == EINTR)
                /* try again */;
            if (result < 0) {
                result = errno;
                fclose(f);
                return errh->error("locking %s: %s", map_file.c_str(), strerror(result));
            }
        }
#endif

        // read old data from map file
        StringAccum sa;
        int amt;
        do {
            if (char *x = sa.reserve(8192)) {
                amt = fread(x, 1, 8192, f);
                sa.adjust_length(amt);
            } else
                amt = 0;
        } while (amt != 0);
        if (!feof(f))
            return errh->error("%s: %s", map_file.c_str(), strerror(errno));
        String text = sa.take_string();

        // add comment if necessary
        bool created = (!text);
        if (created)
            text = "% Automatically maintained by otftotfm or other programs. Do not edit.\n\n";
        if (text.back() != '\n')
            text += "\n";

        // append old encodings
        int fl = 0;
        int nl = text.find_left('\n') + 1;
        bool changed = created;
        while (fl < text.length()) {
            if (fl + fontname.length() + 1 < nl
                && memcmp(text.data() + fl, fontname.data(), fontname.length()) == 0
                && text[fl + fontname.length()] == ' ') {
                // found the old name
                if (text.substring(fl, nl - fl) == mapline) {
                    // duplicate of old name, don't change it
                    fclose(f);
                    if (verbose)
                        errh->message("%s unchanged", map_file.c_str());
                    return 0;
                } else {
                    text = text.substring(0, fl) + text.substring(nl);
                    nl = fl;
                    changed = true;
                }
            }
            fl = nl;
            nl = text.find_left('\n', fl) + 1;
        }

        if (!mapline && !changed) {
            // special case: empty mapline, unchanged file
            if (verbose)
                errh->message("%s unchanged", map_file.c_str());
        } else {
            // add our text
            text += mapline;

            // rewind file
#if HAVE_FTRUNCATE
            rewind(f);
            if (ftruncate(fd, 0) < 0)
#endif
            {
                fclose(f);
                f = fopen(map_file.c_str(), "w");
                fd = fileno(f);
            }

            // write data
            ignore_result(fwrite(text.data(), 1, text.length(), f));
        }

        fclose(f);

        // inform about the new file if necessary
        if (created)
            update_odir(O_MAP, map_file, errh);

#if HAVE_KPATHSEA && !WIN32
        // run 'updmap' if present
        String updmap_prog = output_flags & G_UPDMAP_USER ? "updmap-user" : "updmap-sys";
        String updmap_dir, updmap_file;
        if (automatic && (output_flags & G_UPDMAP))
            updmap_dir = getodir(O_MAP_PARENT, errh);
        if (updmap_dir
            && (updmap_file = updmap_dir + "/" + updmap_prog)
            && access(updmap_file.c_str(), X_OK) >= 0) {
            // want to run `updmap` from its directory, can't use system()
            if (verbose)
                errh->message("running %s", updmap_file.c_str());

            pid_t child = fork();
            if (child < 0)
                errh->fatal("%s during fork", strerror(errno));
            else if (child == 0) {
                // change to updmap directory, run it
                if (chdir(updmap_dir.c_str()) < 0)
                    errh->fatal("%s: %s during chdir", updmap_dir.c_str(), strerror(errno));
                if (execl(output_flags & G_UPDMAP_USER ? "./updmap-user" : "./updmap-sys",
                          updmap_file.c_str(),
                          (const char*) 0) < 0)
                    errh->fatal("%s: %s during exec", updmap_file.c_str(), strerror(errno));
                exit(1);        // should never get here
            }

# if HAVE_WAITPID
            // wait for updmap to finish
            int status;
            while (1) {
                pid_t answer = waitpid(child, &status, 0);
                if (answer >= 0)
                    break;
                else if (errno != EINTR)
                    errh->fatal("%s during wait", strerror(errno));
            }
            if (!WIFEXITED(status))
                errh->warning("%s exited abnormally", updmap_file.c_str());
            else if (WEXITSTATUS(status) != 0)
                errh->warning("%s exited with status %d", updmap_file.c_str(), WEXITSTATUS(status));
# else
#  error "need waitpid() support: report this bug to the maintainer"
# endif
            goto ran_updmap;
        }

# if HAVE_AUTO_UPDMAP
        // run system updmap
        if (output_flags & G_UPDMAP) {
            String filename = map_file;
            int slash = filename.find_right('/');
            if (slash >= 0)
                filename = filename.substring(slash + 1);
            String redirect = verbose ? " 1>&2" : " >" DEV_NULL " 2>&1";
            String command = updmap_prog + " --nomkmap --enable Map " + shell_quote(filename) + redirect
                + CMD_SEP " " + updmap_prog + redirect;
            int retval = mysystem(command.c_str(), errh);
            if (retval == 127)
                errh->warning("could not run %<%s%>", command.c_str());
            else if (retval < 0)
                errh->warning("could not run %<%s%>: %s", command.c_str(), strerror(errno));
            else if (retval != 0)
                errh->warning("%<%s%> exited with status %d;\nrun it manually to check for errors", command.c_str(), WEXITSTATUS(retval));
            goto ran_updmap;
        }
# endif

        if (verbose)
            errh->message("not running updmap");

      ran_updmap: ;
#endif
    }

    return 0;
}

String
locate_encoding(String encfile, ErrorHandler *errh, bool literal)
{
    if (!encfile || encfile == "-")
        return encfile;

    if (!literal) {
        int slash = encfile.find_right('/');
        int dot = encfile.find_left('.', slash >= 0 ? slash : 0);
        if (dot < 0)
            if (String file = locate_encoding(encfile + ".enc", errh, true))
                return file;
    }

#if HAVE_KPATHSEA
    if (String file = kpsei_string(kpsei_find_file(encfile.c_str(), KPSEI_FMT_ENCODING))) {
        if (verbose)
            errh->message("encoding file %s found with kpathsea at %s", encfile.c_str(), file.c_str());
        return file;
    } else if (verbose)
        errh->message("encoding file %s not found with kpathsea", encfile.c_str());
#endif

    if (access(encfile.c_str(), R_OK) >= 0)
        return encfile;
    else
        return String();
}
