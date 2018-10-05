/* texmfmp.c: Hand-coded routines for TeX or Metafont in C.  Originally
   written by Tim Morgan, drawing from other Unix ports of TeX.  This is
   a collection of miscellany, everything that's easier (or only
   possible) to do in C.
   
   This file is public domain.  */

#if defined(pdfTeX)
#include "ptexlib.h"
#endif

#include <sys/stat.h>

#include "texmfmp.h"

#define check_nprintf(size_get, size_want) \
    if ((unsigned)(size_get) >= (unsigned)(size_want)) \
        pdftex_fail ("snprintf failed: file %s, line %d", __FILE__, __LINE__);

#if !defined(XeTeX)
static boolean start_time_set = false;
static time_t start_time = 0;
#define TIME_STR_SIZE 30
char start_time_str[TIME_STR_SIZE];
static char time_str[TIME_STR_SIZE];
    /* minimum size for time_str is 24: "D:YYYYmmddHHMMSS+HH'MM'" */

static void makepdftime(time_t t, char *time_str, boolean utc)
{

    struct tm lt, gmt;
    size_t size;
    int i, off, off_hours, off_mins;

    /* get the time */
    if (utc) {
        lt = *gmtime(&t);
    }
    else {
        lt = *localtime(&t);
    }
    size = strftime(time_str, TIME_STR_SIZE, "D:%Y%m%d%H%M%S", &lt);
    /* expected format: "YYYYmmddHHMMSS" */
    if (size == 0) {
        /* unexpected, contents of time_str is undefined */
        time_str[0] = '\0';
        return;
    }

    /* correction for seconds: %S can be in range 00..61,
       the PDF reference expects 00..59,
       therefore we map "60" and "61" to "59" */
    if (time_str[14] == '6') {
        time_str[14] = '5';
        time_str[15] = '9';
        time_str[16] = '\0';    /* for safety */
    }

    /* get the time zone offset */
    gmt = *gmtime(&t);

    /* this calculation method was found in exim's tod.c */
    off = 60 * (lt.tm_hour - gmt.tm_hour) + lt.tm_min - gmt.tm_min;
    if (lt.tm_year != gmt.tm_year) {
        off += (lt.tm_year > gmt.tm_year) ? 1440 : -1440;
    } else if (lt.tm_yday != gmt.tm_yday) {
        off += (lt.tm_yday > gmt.tm_yday) ? 1440 : -1440;
    }

    if (off == 0) {
        time_str[size++] = 'Z';
        time_str[size] = 0;
    } else {
        off_hours = off / 60;
        off_mins = abs(off - off_hours * 60);
        i = snprintf(&time_str[size], 9, "%+03d'%02d'", off_hours, off_mins);
        check_nprintf(i, 9);
    }
}

#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif

void initstarttime(void)
{
    char *source_date_epoch;
    int64_t epoch;
    char *endptr;
    if (!start_time_set) {
        start_time_set = true;
        source_date_epoch = getenv("SOURCE_DATE_EPOCH");
        if (source_date_epoch) {
            errno = 0;
            epoch = strtoll(source_date_epoch, &endptr, 10);
            if (epoch < 0 || *endptr != '\0' || errno != 0) {
                fprintf(stderr, "Environment variable $SOURCE_DATE_EPOCH: invalid value: %s\n", source_date_epoch);
                uexit(EXIT_FAILURE);
            }
            start_time = epoch;
            makepdftime(start_time, start_time_str, /* utc= */true);
        }
        else {
            start_time = time((time_t *) NULL);
            makepdftime(start_time, start_time_str, /* utc= */false);
        }
    }
}

char *makecstring(integer s)
{
    static char *cstrbuf = NULL;
    char *p;
    static int allocsize;
    int allocgrow, i, l = strstart[s + 1] - strstart[s];
    check_buf(l + 1, MAX_CSTRING_LEN);
    if (cstrbuf == NULL) {
        allocsize = l + 1;
        cstrbuf = xmallocarray(char, allocsize);
    } else if (l + 1 > allocsize) {
        allocgrow = allocsize * 0.2;
        if (l + 1 - allocgrow > allocsize)
            allocsize = l + 1;
        else if (allocsize < MAX_CSTRING_LEN - allocgrow)
            allocsize += allocgrow;
        else
            allocsize = MAX_CSTRING_LEN;
        cstrbuf = xreallocarray(cstrbuf, char, allocsize);
    }
    p = cstrbuf;
    for (i = 0; i < l; i++)
        *p++ = strpool[i + strstart[s]];
    *p = 0;
    return cstrbuf;
}

/* makecfilename
  input/ouput same as makecstring:
    input: string number
    output: C string with quotes removed.
    That means, file names that are legal on some operation systems
    cannot any more be used since pdfTeX version 1.30.4.
*/
char *makecfilename(integer s)
{
    char *name = makecstring(s);
    char *p = name;
    char *q = name;

    while (*p) {
        if (*p != '"')
            *q++ = *p;
        p++;
    }
    *q = '\0';
    return name;
}

void getcreationdate(void)
{
    size_t len;
    initstarttime();
    /* put creation date on top of string pool and update poolptr */
    len = strlen(start_time_str);

    /* In e-pTeX, "init len => call initstarttime()" (as pdftexdir/utils.c)
       yields  unintentional output. */

    if ((unsigned) (poolptr + len) >= (unsigned) (poolsize)) {
        poolptr = poolsize;
        /* error by str_toks that calls str_room(1) */
        return;
    }

    memcpy(&strpool[poolptr], start_time_str, len);
    poolptr += len;
}

void getfilemoddate(integer s)
{
    struct stat file_data;

    char *file_name = kpse_find_tex(makecfilename(s));
    if (file_name == NULL) {
        return;                 /* empty string */
    }

    recorder_record_input(file_name);
    /* get file status */
    if (stat(file_name, &file_data) == 0) {
        size_t len;

        makepdftime(file_data.st_mtime, time_str, /* utc= */false);
        len = strlen(time_str);
        if ((unsigned) (poolptr + len) >= (unsigned) (poolsize)) {
            poolptr = poolsize;
            /* error by str_toks that calls str_room(1) */
        } else {
            memcpy(&strpool[poolptr], time_str, len);
            poolptr += len;
        }
    }
    /* else { errno contains error code } */

    xfree(file_name);
}

void getfilesize(integer s)
{
    struct stat file_data;
    int i;

    char *file_name = kpse_find_tex(makecfilename(s));
    if (file_name == NULL) {
        return;                 /* empty string */
    }

    recorder_record_input(file_name);
    /* get file status */
    if (stat(file_name, &file_data) == 0) {
        size_t len;
        char buf[20];

        /* st_size has type off_t */
        i = snprintf(buf, sizeof(buf),
                     "%lu", (long unsigned int) file_data.st_size);
        check_nprintf(i, sizeof(buf));
        len = strlen(buf);
        if ((unsigned) (poolptr + len) >= (unsigned) (poolsize)) {
            poolptr = poolsize;
            /* error by str_toks that calls str_room(1) */
        } else {
            memcpy(&strpool[poolptr], buf, len);
            poolptr += len;
        }
    }
    /* else { errno contains error code } */

    xfree(file_name);
}

void getfiledump(integer s, int offset, int length)
{
    FILE *f;
    int read, i;
    poolpointer data_ptr;
    poolpointer data_end;
    char *file_name;

    if (length == 0) {
        /* empty result string */
        return;
    }

    if (poolptr + 2 * length + 1 >= poolsize) {
        /* no place for result */
        poolptr = poolsize;
        /* error by str_toks that calls str_room(1) */
        return;
    }

    file_name = kpse_find_tex(makecfilename(s));
    if (file_name == NULL) {
        return;                 /* empty string */
    }

    /* read file data */
    f = fopen(file_name, FOPEN_RBIN_MODE);
    if (f == NULL) {
        xfree(file_name);
        return;
    }
    recorder_record_input(file_name);
    if (fseek(f, offset, SEEK_SET) != 0) {
        xfree(file_name);
        return;
    }
    /* there is enough space in the string pool, the read
       data are put in the upper half of the result, thus
       the conversion to hex can be done without overwriting
       unconverted bytes. */
    data_ptr = poolptr + length;
    read = fread(&strpool[data_ptr], sizeof(char), length, f);
    fclose(f);

    /* convert to hex */
    data_end = data_ptr + read;
    for (; data_ptr < data_end; data_ptr++) {
        i = snprintf((char *) &strpool[poolptr], 3,
                     "%.2X", (unsigned int) strpool[data_ptr]);
        check_nprintf(i, 3);
        poolptr += i;
    }
    xfree(file_name);
}
#endif /* not XeTeX */

/* Converts any given string in into an allowed PDF string which is
 * hexadecimal encoded;
 * sizeof(out) should be at least lin*2+1.
 */
void convertStringToHexString(const char *in, char *out, int lin)
{
    int i, j, k;
    char buf[3];
    j = 0;
    for (i = 0; i < lin; i++) {
        k = snprintf(buf, sizeof(buf),
                     "%02X", (unsigned int) (unsigned char) in[i]);
        check_nprintf(k, sizeof(buf));
        out[j++] = buf[0];
        out[j++] = buf[1];
    }
    out[j] = '\0';
}

#define DIGEST_SIZE 16
#define FILE_BUF_SIZE 1024

void getmd5sum(strnumber s, boolean file)
{
    md5_state_t state;
    md5_byte_t digest[DIGEST_SIZE];
    char outbuf[2 * DIGEST_SIZE + 1];
    int len = 2 * DIGEST_SIZE;
#if defined(XeTeX)
    char *xname;
    int i;
#endif

    if (file) {
        char file_buf[FILE_BUF_SIZE];
        int read = 0;
        FILE *f;
        char *file_name;

#if defined(XeTeX)
        xname = gettexstring (s);
        file_name = kpse_find_tex (xname);
        xfree (xname);
#else
        file_name = kpse_find_tex(makecfilename(s));
#endif
        if (file_name == NULL) {
            return;             /* empty string */
        }
        /* in case of error the empty string is returned,
           no need for xfopen that aborts on error.
         */
        f = fopen(file_name, FOPEN_RBIN_MODE);
        if (f == NULL) {
            xfree(file_name);
            return;
        }
        recorder_record_input(file_name);
        md5_init(&state);
        while ((read = fread(&file_buf, sizeof(char), FILE_BUF_SIZE, f)) > 0) {
            md5_append(&state, (const md5_byte_t *) file_buf, read);
        }
        md5_finish(&state, digest);
        fclose(f);

        xfree(file_name);
    } else {
        /* s contains the data */
        md5_init(&state);
#if defined(XeTeX)
        xname = gettexstring (s);
        md5_append(&state,
                   (md5_byte_t *) xname,
                   strlen (xname));
        xfree (xname);
#else
        md5_append(&state,
                   (md5_byte_t *) &strpool[strstart[s]],
                   strstart[s + 1] - strstart[s]);
#endif
        md5_finish(&state, digest);
    }

    if (poolptr + len >= poolsize) {
        /* error by str_toks that calls str_room(1) */
        return;
    }
    convertStringToHexString((char *) digest, outbuf, DIGEST_SIZE);
#if defined(XeTeX)
    for (i = 0; i < 2 * DIGEST_SIZE; i++)
        strpool[poolptr++] = (uint16_t)outbuf[i];
#else
    memcpy(&strpool[poolptr], outbuf, len);
    poolptr += len;
#endif
}
