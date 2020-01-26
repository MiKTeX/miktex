/* main.cc -- driver for mmafm program
 *
 * Copyright (c) 1997-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <efont/findmet.hh>
#include <efont/psres.hh>
#include <lcdf/slurper.hh>
#include <efont/afm.hh>
#include <efont/afmw.hh>
#include <efont/amfm.hh>
#include <lcdf/error.hh>
#include <lcdf/clp.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#ifdef HAVE_CTIME
# include <time.h>
#endif

#define WEIGHT_OPT	300
#define WIDTH_OPT	301
#define OPSIZE_OPT	302
#define STYLE_OPT	303
#define N1_OPT		304
#define N2_OPT		305
#define N3_OPT		306
#define N4_OPT		307
#define VERSION_OPT	308
#define HELP_OPT	309
#define OUTPUT_OPT	310
#define PRECISION_OPT	311
#define KERN_PREC_OPT	312

const Clp_Option options[] = {
  { "1", '1', N1_OPT, Clp_ValDouble, 0 },
  { "2", '2', N2_OPT, Clp_ValDouble, 0 },
  { "3", '3', N3_OPT, Clp_ValDouble, 0 },
  { "4", '4', N4_OPT, Clp_ValDouble, 0 },
  { "weight", 'w', WEIGHT_OPT, Clp_ValDouble, 0 },
  { "width", 'W', WIDTH_OPT, Clp_ValDouble, 0 },
  { "optical-size", 'O', OPSIZE_OPT, Clp_ValDouble, 0 },
  { "style", 0, STYLE_OPT, Clp_ValDouble, 0 },
  { "wt", 0, WEIGHT_OPT, Clp_ValDouble, 0 },
  { "wd", 0, WIDTH_OPT, Clp_ValDouble, 0 },
  { "min-kern", 'k', KERN_PREC_OPT, Clp_ValDouble, 0 },
  { "minimum-kern", 'k', KERN_PREC_OPT, Clp_ValDouble, 0 },
  { "kern-precision", 'k', KERN_PREC_OPT, Clp_ValDouble, 0 },
  { "output", 'o', OUTPUT_OPT, Clp_ValString, 0 },
  { "precision", 'p', PRECISION_OPT, Clp_ValInt, 0 },
  { "version", 'v', VERSION_OPT, 0, 0 },
  { "help", 'h', HELP_OPT, 0, 0 },
};

using namespace Efont;


static const char *program_name;
static ErrorHandler *errh;
static AmfmMetrics *amfm;

static Vector<PermString> ax_names;
static Vector<int> ax_nums;
static Vector<double> values;

static void
set_design(PermString a, double v)
{
  ax_names.push_back(a);
  ax_nums.push_back(-1);
  values.push_back(v);
}

static void
set_design(int a, double v)
{
  ax_names.push_back(PermString());
  ax_nums.push_back(a);
  values.push_back(v);
}

static void
set_amfm(AmfmMetrics *a)
{
  if (a) {
    if (amfm) errh->fatal("already read one AMFM file");
    amfm = a;
  }
}


// apply precision
static inline void
pround(double &v, double multiplier, double divider)
{
  if (KNOWN(v))
    v = floor(v * multiplier + 0.5) * divider;
}

static void
apply_precision(Metrics *m, int precision)
{
  if (precision < 0)
    return;

  double multiplier = 1, divider = 1;
  for (int i = 0; i < precision; i++)
    multiplier *= 10, divider /= 10;

  for (int i = 0; i < m->nfd(); i++)
    pround(m->fd(i), multiplier, divider);

  for (int i = 0; i < m->nglyphs(); i++) {
    pround(m->wd(i), multiplier, divider);
    pround(m->lf(i), multiplier, divider);
    pround(m->bt(i), multiplier, divider);
    pround(m->rt(i), multiplier, divider);
    pround(m->tp(i), multiplier, divider);
  }

  for (int i = 0; i < m->nkv(); i++)
    pround(m->kv(i), multiplier, divider);
}

static void
apply_kern_precision(Metrics *m, double kern_precision)
{
  if (kern_precision <= 0)
    return;

  for (int i = 0; i < m->nkv(); i++)
    if (fabs(m->kv(i)) < kern_precision)
      m->kv(i) = 0;
}


static void
read_file(const char *fn, MetricsFinder *finder)
{
  Filename filename;
  FILE *file;

  if (strcmp(fn, "-") == 0) {
    filename = Filename("<stdin>");
    file = stdin;
  } else {
    filename = Filename(fn);
    file = filename.open_read();
  }
  int save_errno = errno;

  if (!file) {
    // look for a font by name
    AmfmMetrics *new_amfm = finder->find_amfm(fn, errh);
    if (new_amfm) {
      set_amfm(new_amfm);
      return;
    }
    if (finder->find_metrics(fn, errh))
      return;

    // check for instance name. don't use InstanceMetricsFinder.
    const char *underscore = strchr(fn, '_');
    if (underscore)
      new_amfm = finder->find_amfm(PermString(fn, underscore - fn), errh);
    if (!new_amfm)
      errh->fatal("%s: %s", fn, strerror(save_errno));
    set_amfm(new_amfm);

    int i = 0;
    while (underscore[0] == '_' && underscore[1]) {
      double x = strtod(underscore + 1, const_cast<char **>(&underscore));
      set_design(i, x);
      i++;
    }
    return;
  }

  Slurper slurper(filename, file);
  bool is_afm = false;
  if (file != stdin) {
    char *first_line = slurper.peek_line();
    if (first_line)
      is_afm = strncmp(first_line, "StartFontMetrics", 16) == 0;
  }

  if (is_afm) {
    Metrics *afm = AfmReader::read(slurper, errh);
    if (afm)
      finder->record(afm);
  } else
    set_amfm(AmfmReader::read(slurper, finder, errh));
}


static void
usage_error(const char *error_message, ...)
{
  va_list val;
  va_start(val, error_message);
  if (!error_message)
    errh->message("Usage: %s [OPTION | FONT]...", program_name);
  else
    errh->xmessage(ErrorHandler::e_error, error_message, val);
  errh->message("Type %s --help for more information.", program_name);
  exit(1);
}

static void
usage()
{
    FileErrorHandler uerrh(stdout);
    uerrh.message("\
%<Mmafm%> creates an AFM font metrics file for a multiple master font by\n\
interpolating at a point you specify and writes it to the standard output.\n\
\n\
Usage: %s [OPTION | FONT]...\n\
\n\
Each FONT is either an AFM or AMFM file name, or the font name of a multiple\n\
master font. In the second case, mmafm will find the actual AMFM file using\n\
the PSRESOURCEPATH environment variable.\n\
\n\
General options:\n\
  -o, --output=FILE             Write output to FILE.\n\
  -h, --help                    Print this message and exit.\n\
  -v, --version                 Print version number and warranty and exit.\n\
\n\
Interpolation settings:\n\
  -w, --weight=N                Set weight to N.\n\
  -W, --width=N                 Set width to N.\n\
  -O, --optical-size=N          Set optical size to N.\n\
      --style=N                 Set style axis to N.\n\
  --1=N, --2=N, --3=N, --4=N    Set first (second, third, fourth) axis to N.\n\
  -p, --precision=N             Allow N digits of fraction (default 3).\n\
  -k, --min-kern=N              Remove kerns smaller than N (default 2).\n\
\n\
Report bugs to <ekohler@gmail.com>.\n", program_name);
}


int
main(int argc, char *argv[])
{
  MetricsFinder *finder = new CacheMetricsFinder;

  PsresDatabase *psres = new PsresDatabase;
  psres->add_psres_path(getenv("PSRESOURCEPATH"), 0, false);
  PsresMetricsFinder *psres_finder = new PsresMetricsFinder(psres);
  finder->add_finder(psres_finder);

  Clp_Parser *clp =
    Clp_NewParser(argc, (const char * const *)argv, sizeof(options) / sizeof(options[0]), options);
  program_name = Clp_ProgramName(clp);

  errh = ErrorHandler::static_initialize(new FileErrorHandler(stderr, String(program_name) + ": "));

  FILE *output_file = 0;
  int precision = 3;
  double kern_precision = 2.0;
  while (1) {
    int opt = Clp_Next(clp);
    switch (opt) {

     case WEIGHT_OPT:
      set_design("Weight", clp->val.d);
      break;

     case WIDTH_OPT:
      set_design("Width", clp->val.d);
      break;

     case OPSIZE_OPT:
      set_design("OpticalSize", clp->val.d);
      break;

     case STYLE_OPT:
      set_design("Style", clp->val.d);
      break;

     case N1_OPT:
     case N2_OPT:
     case N3_OPT:
     case N4_OPT:
      set_design(opt - N1_OPT, clp->val.d);
      break;

     case PRECISION_OPT:
      precision = clp->val.i;
      break;

     case KERN_PREC_OPT:
      kern_precision = clp->val.d;
      break;

     case OUTPUT_OPT:
      if (output_file) errh->fatal("output file already specified");
      if (strcmp(clp->vstr, "-") == 0)
	output_file = stdout;
      else {
	output_file = fopen(clp->vstr, "wb");
	if (!output_file)
	    errh->fatal("%s: %s", clp->vstr, strerror(errno));
      }
      break;

     case HELP_OPT:
      usage();
      exit(0);
      break;

     case VERSION_OPT:
      printf("mmafm (LCDF typetools) %s\n", VERSION);
      printf("Copyright (C) 1997-2019 Eddie Kohler\n\
This is free software; see the source for copying conditions.\n\
There is NO warranty, not even for merchantability or fitness for a\n\
particular purpose.\n");
      exit(0);
      break;

     case Clp_NotOption:
      read_file(clp->vstr, finder);
      break;

     case Clp_Done:
      goto done;

     case Clp_BadOption:
      usage_error(0);
      break;

    }
  }

 done:
  if (!amfm) usage_error("missing font argument");

  MultipleMasterSpace *mmspace = amfm->mmspace();
#if MMAFM_RUN_MMPFB
  if (!mmspace->check_intermediate()) {
    char *buf = new char[amfm->font_name().length() + 30];
    sprintf(buf, "mmpfb -q --amcp-info '%s'", amfm->font_name().c_str());

    FILE *f = popen(buf, "r");
    if (f) {
      Filename fake("<mmpfb output>");
      Slurper slurpy(fake, f);
      AmfmReader::add_amcp_file(slurpy, amfm, errh);
      pclose(f);
    }

    delete[] buf;
  }
#endif

  Vector<double> design = mmspace->default_design_vector();
  for (int i = 0; i < values.size(); i++)
    if (ax_names[i])
      mmspace->set_design(design, ax_names[i], values[i], errh);
    else
      mmspace->set_design(design, ax_nums[i], values[i], errh);

  Vector<double> weight;
  if (!mmspace->design_to_weight(design, weight, errh)) {
    if (!mmspace->check_intermediate()) {
      errh->message("(I can%,t interpolate font programs with intermediate masters on my own.");
#if MMAFM_RUN_MMPFB
      errh->message("I tried to run %<mmpfb --amcp-info %s%>, but it didn't work.", amfm->font_name().c_str());
      errh->message("Maybe your PSRESOURCEPATH environment variable is not set?");
#endif
      errh->fatal("See the manual page for more information.)");
    } else
      errh->fatal("can%,t create weight vector");
  }

  // Need to check for case when all design coordinates are unspecified. The
  // AMFM file contains a default WeightVector, but often NOT a default
  // DesignVector; we don't want to generate a file with a FontName like
  // `MyriadMM_-9.79797979e97_-9.79797979e97_' because the DesignVector
  // components are unknown.
  if (!KNOWN(design[0]))
    errh->fatal("must specify %s%,s %s coordinate", amfm->font_name().c_str(),
		mmspace->axis_type(0).c_str());

  Metrics *m = amfm->interpolate(design, weight, errh);
  if (m) {

    // Add a comment identifying this as interpolated by mmafm
    if (MetricsXt *xt = m->find_xt("AFM")) {
      AfmMetricsXt *afm_xt = (AfmMetricsXt *)xt;

#if HAVE_CTIME
      time_t cur_time = time(0);
      char *time_str = ctime(&cur_time);
      int time_len = strlen(time_str) - 1;
      char *buf = new char[strlen(VERSION) + time_len + 100];
      sprintf(buf, "Interpolated by mmafm-%s on %.*s.", VERSION,
	      time_len, time_str);
#else
      char *buf = new char[strlen(VERSION) + 100];
      sprintf(buf, "Interpolated by mmafm-%s.", VERSION);
#endif

      afm_xt->opening_comments.push_back(buf);
      afm_xt->opening_comments.push_back("Mmafm is free software.  See <http://www.lcdf.org/type/>.");
      delete[] buf;
    }

    // round numbers if necessary
    if (precision >= 0)
      apply_precision(m, precision);
    if (kern_precision > 0)
      apply_kern_precision(m, kern_precision);

    // write the output file
    if (!output_file)
      output_file = stdout;
    AfmWriter::write(m, output_file);

    return 0;
  } else
    return 1;
}
