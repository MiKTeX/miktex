/* This is extractbb, a bounding box extraction program.
    Copyright (C) 2008-2020 by Jin-Hwan Cho and Matthias Franz
    and the dvipdfmx project team.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "dpxconf.h"
#include "dpxutil.h"

#include "numbers.h"
#include "system.h"
#include "mem.h"
#include "error.h"
#include "mfileio.h"
#include "pdfobj.h"
#include "pdfdoc.h"
#include "pdfparse.h"
#include "pdfdraw.h"

#include "bmpimage.h"
#include "jpegimage.h"
#include "jp2image.h"
#include "pngimage.h"

#include "dvipdfmx.h"
#include "pdflimits.h"
#if defined(MIKTEX_WINDOWS)
#include <miktex/unxemu.h>
#include <getopt.h>
#endif

static enum pdf_page_boundary PageBox = pdf_page_boundary__auto;

static int Include_Page = 1;

static void show_version(void)
{
  fprintf (stdout, "\nThis is %s Version " VERSION "\n", my_name);
  fprintf (stdout, "\nCopyright (C) 2008-2020 by Jin-Hwan Cho and Matthias Franz\n");
  fprintf (stdout, "\nThis is free software; you can redistribute it and/or modify\n");
  fprintf (stdout, "it under the terms of the GNU General Public License as published by\n");
  fprintf (stdout, "the Free Software Foundation; either version 2 of the License, or\n");
  fprintf (stdout, "(at your option) any later version.\n");
}

static void show_usage(void)
{
  printf ("\nUsage: %s [-B pagebox] [-p page] [-q|-v] [-O] [-m|-x] FILE...\n", my_name);
  printf ("       %s --help|--version\n", my_name);
  printf ("Extract bounding box from PDF, PNG, JPEG, JP2, or BMP file; default output below.\n");
  printf ("\nOptions:\n");
  printf ("  -B pagebox\tSpecify a PDF pagebox for bounding box\n");
  printf ("            \tpagebox=cropbox, mediabox, artbox, trimbox, bleedbox\n");
  printf ("  -h | --help\tShow this help message and exit\n");
  printf ("  --version\tOutput version information and exit\n");
  printf ("  -p page\tSpecify a PDF page to extract bounding box\n");
  printf ("  -q\t\tBe quiet\n");
  printf ("  -v\t\tBe verbose\n");
  printf ("  -O\t\tWrite output to stdout\n");
  printf ("  -m\t\tOutput .bb  file used in DVIPDFM%s\n", my_name[1] == 'b' ? " (default)" : "");
  printf ("  -x\t\tOutput .xbb file used in DVIPDFMx%s\n", my_name[1] == 'b' ? "" : " (default)");
}

static void usage(void)
{
  fprintf(stdout, "\nTry \"%s --help\" for more information.\n", my_name);
  exit(1);
}

static void do_time(FILE *file)
{
  time_t current_time;
  struct tm *bd_time;

  current_time = dpx_util_get_unique_time_if_given();
  if (current_time == INVALID_EPOCH_VALUE) {
    time(&current_time);
    bd_time = localtime(&current_time);
  } else {
    bd_time = gmtime(&current_time);
  }
  fprintf(file, "%%%%CreationDate: %s\n", asctime(bd_time));
}

const char *extensions[] = {
  ".ai", ".AI", ".bmp", ".BMP", ".jpeg", ".JPEG", ".jpg", ".JPG",
  ".jp2", ".JP2", ".jpf", ".JPF", ".pdf", ".PDF", ".png", ".PNG"
};

static int xbb_to_file = 1;

static char *make_xbb_filename(const char *name)
{
  char *result;
  int   i;

  for (i = 0; i < sizeof(extensions) / sizeof(extensions[0]); i++) {
    if (strlen(extensions[i]) < strlen(name) &&
        strncmp(name+strlen(name)-strlen(extensions[i]), extensions[i], strlen(extensions[i])) == 0)
      break;
  }
  if (i == sizeof(extensions) / sizeof(extensions[0])) {
    WARN("%s: Filename does not end in a recognizable extension.\n", name);
    result = NEW(strlen(name)+strlen(".xbb")+1, char);
    strcpy(result, name);
  } else { /* Remove extension */
    size_t len;
    len    = strlen(name) - strlen(extensions[i]);
    result = NEW(len+strlen(".xbb")+1, char);
    strncpy(result, name, len);
    result[len] = 0;
  }
  strcat(result, ((dpx_conf.compat_mode == dpx_mode_compat_mode) ? ".bb" : ".xbb"));
  return result;
}

static void write_xbb(char *fname,
                      double bbllx_f, double bblly_f,
                      double bburx_f, double bbury_f,
                      int pdf_version, int pagecount)
{
  char *outname = NULL;
  FILE *fp = NULL;

  int bbllx = ROUND(bbllx_f, 1.0), bblly = ROUND(bblly_f, 1.0);
  int bburx = ROUND(bburx_f, 1.0), bbury = ROUND(bbury_f, 1.0);

  if (xbb_to_file) {
    outname = make_xbb_filename(fname);
    if (!kpse_out_name_ok(outname) || !(fp = MFOPEN(outname, FOPEN_W_MODE))) {
      ERROR("Unable to open output file: %s\n", outname);
    }
  } else {
    fp = stdout;
#ifdef WIN32
    setmode(fileno(fp), _O_BINARY);
#endif
  }

  if (dpx_conf.verbose_level > 0) {
    MESG("Writing to %s: ", xbb_to_file ? outname : "stdout");
    MESG("Bounding box: %d %d %d %d\n", bbllx, bblly, bburx, bbury);
  }

  fprintf(fp, "%%%%Title: %s\n", fname);
  fprintf(fp, "%%%%Creator: extractbb %s\n", VERSION);
  fprintf(fp, "%%%%BoundingBox: %d %d %d %d\n", bbllx, bblly, bburx, bbury);

  if (dpx_conf.compat_mode != dpx_mode_compat_mode) {
    /* Note:
     * According to Adobe Technical Note #5644, the arguments to
     * "%%HiResBoundingBox:" must be of type real. And according
     * to the PostScript Language Reference, a real number must
     * be written with a decimal point (or an exponent). Hence
     * it seems illegal to replace "0.0" by "0".
     */
    fprintf(fp, "%%%%HiResBoundingBox: %f %f %f %f\n",
            bbllx_f, bblly_f, bburx_f, bbury_f);
    if (pdf_version >= 0) {
      fprintf(fp, "%%%%PDFVersion: %d.%d\n", pdf_version/10, pdf_version%10);
      fprintf(fp, "%%%%Pages: %d\n", pagecount);
    }
  }

  do_time(fp);

  if (xbb_to_file) {
    RELEASE(outname);
    MFCLOSE(fp);
  }
}

static void do_bmp (FILE *fp, char *filename)
{
  int    width, height;
  double xdensity, ydensity;

  if (bmp_get_bbox(fp, &width, &height, &xdensity, &ydensity) < 0) {
    WARN("%s does not look like a BMP file...\n", filename);
    return;
  }

  write_xbb(filename, 0, 0, xdensity*width, ydensity*height, -1, -1);
  return;
}

static void do_jpeg (FILE *fp, char *filename)
{
  int    width, height;
  double xdensity, ydensity;

  if (jpeg_get_bbox(fp, &width, &height, &xdensity, &ydensity) < 0) {
    WARN("%s does not look like a JPEG file...\n", filename);
    return;
  }

  write_xbb(filename, 0, 0, xdensity*width, ydensity*height, -1, -1);
  return;
}

static void do_jp2 (FILE *fp, char *filename)
{
  int    width, height;
  double xdensity, ydensity;

  if (jp2_get_bbox(fp, &width, &height, &xdensity, &ydensity) < 0) {
    WARN("%s does not look like a JP2/JPX file...\n", filename);
    return;
  }

  write_xbb(filename, 0, 0, xdensity*width, ydensity*height, -1, -1);
  return;
}

#ifdef HAVE_LIBPNG
static void do_png (FILE *fp, char *filename)
{
  uint32_t width, height;
  double xdensity, ydensity;

  if (png_get_bbox(fp, &width, &height, &xdensity, &ydensity) < 0) {
    WARN("%s does not look like a PNG file...\n", filename);
    return;
  }

  write_xbb(filename, 0, 0, xdensity*width, ydensity*height, -1, -1);
  return;
}
#endif /* HAVE_LIBPNG */

static void do_pdf (FILE *fp, char *filename)
{
  pdf_obj *page;
  pdf_file *pf;
  int page_no = Include_Page;
  int count;
  pdf_rect bbox;
  pdf_tmatrix matrix;
  pdf_coord   p1, p2, p3, p4;

  pf = pdf_open(filename, fp);
  if (!pf) {
    WARN("%s does not look like a PDF file...\n", filename);
    return;
  }
  count = pdf_doc_get_page_count(pf);
  page  = pdf_doc_get_page(pf, page_no, PageBox, &bbox, &matrix, NULL);

  pdf_close(pf);

  if (!page)
    return;

  pdf_release_obj(page);

  /* Image's attribute "bbox" here is affected by /Rotate entry of included
   * PDF page.
   */
  p1.x = bbox.llx; p1.y = bbox.lly;
  pdf_dev_transform(&p1, &matrix);
  p2.x = bbox.urx; p2.y = bbox.lly;
  pdf_dev_transform(&p2, &matrix);
  p3.x = bbox.urx; p3.y = bbox.ury;
  pdf_dev_transform(&p3, &matrix);
  p4.x = bbox.llx; p4.y = bbox.ury;
  pdf_dev_transform(&p4, &matrix);

  bbox.llx = min4(p1.x, p2.x, p3.x, p4.x);
  bbox.lly = min4(p1.y, p2.y, p3.y, p4.y);
  bbox.urx = max4(p1.x, p2.x, p3.x, p4.x);
  bbox.ury = max4(p1.y, p2.y, p3.y, p4.y);

  write_xbb(filename, bbox.llx, bbox.lly, bbox.urx, bbox.ury,
            pdf_file_get_version(pf), count);
}

static const char *optstrig = ":hB:p:qvObmx";

static struct option long_options[] = {
  {"help", 0, 0, 'h'},
  {"version", 0, 0, 130},
  {0, 0, 0, 0}
};

int extractbb (int argc, char *argv[])
{
  int c;

  pdf_files_init();

  pdf_set_version(PDF_VERSION_MAX);

  opterr = 0;
  
  while ((c = getopt_long(argc, argv, optstrig, long_options, NULL)) != -1) {
    switch(c) {
    case 'h':
      show_usage();
      exit(0);

    case 130:
      show_version();
      exit(0);

    case 'B':
      if (strcasecmp (optarg, "cropbox") == 0) PageBox = pdf_page_boundary_cropbox;
      else if (strcasecmp (optarg, "mediabox") == 0) PageBox = pdf_page_boundary_mediabox;
      else if (strcasecmp (optarg, "artbox") == 0) PageBox = pdf_page_boundary_artbox; 
      else if (strcasecmp (optarg, "trimbox") == 0) PageBox = pdf_page_boundary_trimbox;
      else if (strcasecmp (optarg, "bleedbox") == 0) PageBox = pdf_page_boundary_bleedbox;
      else {
        fprintf(stderr, "%s: Invalid argument \"-B %s\"", my_name, optarg);
        usage();
      }
      break;

    case 'p':
      Include_Page = atol(optarg);
      if (Include_Page == 0)
        Include_Page = 1;
      break;

    case 'q':
      dpx_conf.verbose_level = 0;
      break;
    case 'v':
      dpx_conf.verbose_level++;
      break;

    case 'O':
      xbb_to_file = 0;
    case 'b':  /* Ignored for backward compatibility */
      break;

    case 'm':
      dpx_conf.compat_mode = dpx_mode_compat_mode;
      break;
    case 'x':
      dpx_conf.compat_mode = dpx_mode_normal_mode;
      break;

    default:
      fprintf(stderr, "%s: %s \"-%c\"", my_name,
              c == ':' ? "Missing argument for" : "Unknown option",
              optopt); 
      usage();
    }
  }

  if (optind >= argc) {
    fprintf (stderr, "%s: Missing filename argument", my_name);
    usage();
  }

  for (; optind < argc; optind++) {
    FILE *infile = NULL;
    char *kpse_file_name = NULL;

    if (kpse_in_name_ok(argv[optind])) {
      infile = MFOPEN(argv[optind], FOPEN_RBIN_MODE);
      if (infile) {
        kpse_file_name = xstrdup(argv[optind]);
      } else {
        kpse_file_name = kpse_find_pict(argv[optind]);
        if (kpse_file_name && kpse_in_name_ok(kpse_file_name))
          infile = MFOPEN(kpse_file_name, FOPEN_RBIN_MODE);
      }
    }
    if (infile == NULL) {
      WARN("Can't find file (%s), or it is forbidden to read ...skipping\n", argv[optind]);
      goto cont;
    }
    if (check_for_bmp(infile)) {
      do_bmp(infile, kpse_file_name);
      goto cont;
    }
    if (check_for_jpeg(infile)) {
      do_jpeg(infile, kpse_file_name);
      goto cont;
    }
    if (check_for_jp2(infile)) {
      do_jp2(infile, kpse_file_name);
      goto cont;
    }
    if (check_for_pdf(infile)) {
      do_pdf(infile, kpse_file_name);
      goto cont;
    }
#ifdef HAVE_LIBPNG
    if (check_for_png(infile)) {
      do_png(infile, kpse_file_name);
      goto cont;
    }
#endif /* HAVE_LIBPNG */
    WARN("Can't handle file type for file named %s\n", argv[optind]);
  cont:
    if (kpse_file_name)
      RELEASE(kpse_file_name);
    if (infile)
      MFCLOSE(infile);
  }

  pdf_files_close();

  return 0;
}
