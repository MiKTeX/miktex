// -*- related-file-name: "../include/efont/afm.hh" -*-

/* afm.{cc,hh} -- Adobe Font Metrics files
 *
 * Copyright (c) 1998-2019 Eddie Kohler
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
#include <efont/afm.hh>
#include <efont/afmparse.hh>
#include <efont/t1cs.hh>        /* for UNKDOUBLE */
#include <lcdf/error.hh>
#include <ctype.h>
#include <assert.h>
namespace Efont {

AfmReader::AfmReader(AfmParser &parser, Metrics *afm, AfmMetricsXt *afm_xt,
                     ErrorHandler *errh)
    : _afm(afm), _afm_xt(afm_xt), _l(parser),
      _composite_warned(false), _metrics_sets_warned(false), _y_width_warned(0)
{
    _errh = errh ? errh : ErrorHandler::silent_handler();
}

Metrics *
AfmReader::read(Slurper &slurp, ErrorHandler *errh)
{
    AfmParser p(slurp);
    if (!p.ok())
        return 0;

    Metrics *afm = new Metrics;
    AfmMetricsXt *afm_xt = new AfmMetricsXt;
    afm->add_xt(afm_xt);
    AfmReader reader(p, afm, afm_xt, errh);

    if (!reader.read()) {
        delete afm;
        return 0;
    } else
        return afm;
}

Metrics *
AfmReader::read(const Filename &fn, ErrorHandler *errh)
{
    Slurper slurpy(fn);
    return read(slurpy, errh);
}


void
AfmReader::lwarning(const char *format, ...) const
{
    va_list val;
    va_start(val, format);
    _errh->xmessage(_l.landmark(), ErrorHandler::e_warning, format, val);
    va_end(val);
}

void
AfmReader::lerror(const char *format, ...) const
{
    va_list val;
    va_start(val, format);
    _errh->xmessage(_l.landmark(), ErrorHandler::e_error, format, val);
    va_end(val);
}

GlyphIndex
AfmReader::find_err(PermString name, const char *) const
{
    GlyphIndex gi = _afm->find(name);
    if (gi < 0)
        lerror("character `%s' doesn't exist", name.c_str());
    return gi;
}

void
AfmReader::composite_warning() const
{
    if (!_composite_warned)
        lwarning("composite fonts not supported");
    _composite_warned = 1;
}

void
AfmReader::metrics_sets_warning() const
{
    if (!_metrics_sets_warned)
        lwarning("only metrics set 0 is supported");
    _metrics_sets_warned = 1;
}

void
AfmReader::y_width_warning() const
{
    if (_y_width_warned < 40) {
        lwarning("character has a nonzero Y width");
        _y_width_warned++;
        if (_y_width_warned == 40)
            lwarning("I won't warn you again.");
    }
}

void
AfmReader::no_match_warning(const char *context) const
{
    // keyword() will fail (and a warning won't get printed) only if the string
    // is all whitespace, which the spec allows
    PermString keyword = _l.keyword();
    if (!keyword) return;
    if (_l.key_matched()) {
        lwarning(context ? "bad `%s' command in %s:"
                 : "bad `%s' command:", keyword.c_str(), context);
        lwarning("field %d %s", _l.fail_field(), _l.message().c_str());
    } else
        lwarning(context ? "unknown command `%s' in %s"
                 : "unknown command `%s'", keyword.c_str(), context);
    _l.clear_message();
}


bool
AfmReader::read()
{
    AfmParser &l = _l;
    assert(_afm && _afm_xt);

    // First, read all opening comments into an array so we can print them out
    // later.
    PermString comment;
    while (l.next_line()) {
        if (l.isall("Comment %+s", &comment))
            _afm_xt->opening_comments.push_back(comment);
        else if (l.isall("StartFontMetrics %g", (double *)0))
            ;
        else {
            l.save_line();
            break;
        }
    }

    _afm->set_scale(1000);
    unsigned invalid_lines = 0;
    PermString s;
    bool isbasefont;
    int metrics_sets;
    int direction;

    while (l.next_line())
        switch (l.first()) {

          case 'A':
            if (l.isall("Ascender %g", &fd( fdAscender )))
                break;
            goto invalid;

          case 'C':
            if (l.isall("Characters %d", (int *)0))
                break;
            if (l.isall("CapHeight %g", &fd( fdCapHeight )))
                break;
            if (l.isall("CharacterSet %+s", (PermString *) 0))
                break;
            if (l.isall("CharWidth %g %g", (double *)0, (double *)0))
                break;
            if (l.isall("Comment %+s", (PermString *) 0))
                break;
            goto invalid;

          case 'D':
            if (l.isall("Descender %g", &fd( fdDescender )))
                break;
            goto invalid;

          case 'E':
            if (l.isall("EncodingScheme %+s", &_afm_xt->encoding_scheme))
                break;
            if (l.isall("EndDirection"))
                break;
            if (l.isall("EndFontMetrics"))
                goto done;
            if (l.isall("EscChar %d", (int *)0)) {
                composite_warning();
                break;
            }
            goto invalid;

          case 'F':
            if (l.isall("FontName %+s", &s)) {
                _afm->set_font_name(s);
                break;
            }
            if (l.isall("FullName %+s", &s)) {
                _afm->set_full_name(s);
                break;
            }
            if (l.isall("FamilyName %+s", &s)) {
                _afm->set_family(s);
                break;
            }
            if (l.isall("FontBBox %g %g %g %g",
                        &fd( fdFontBBllx ), &fd( fdFontBBlly ),
                        &fd( fdFontBBurx ), &fd( fdFontBBury )))
                break;
            goto invalid;

          case 'I':
            if (l.isall("ItalicAngle %g", &fd( fdItalicAngle )))
                break;
            if (l.isall("IsBaseFont %b", &isbasefont)) {
                if (isbasefont == 0)
                    composite_warning();
                break;
            }
            if (l.isall("IsFixedV %b", (bool *)0)) {
                metrics_sets_warning();
                break;
            }
            if (l.isall("IsFixedPitch %b", (bool *)0))
                break;
            goto invalid;

          case 'M':
            if (l.isall("MappingScheme %d", (int *)0)) {
                composite_warning();
                break;
            }
            if (l.isall("MetricsSets %d", &metrics_sets)) {
                if (metrics_sets != 0)
                    metrics_sets_warning();
                break;
            }
            goto invalid;

          case 'N':
            if (l.isall("Notice %+s", &_afm_xt->notice))
                break;
            goto invalid;

          case 'S':
            if (l.isall("StartDirection %d", &direction)) {
                if (direction != 0)
                    metrics_sets_warning();
                break;
            }
            if (l.isall("StartCharMetrics %d", (int *)0)) {
                read_char_metrics();
                break;
            }
            if (l.isall("StartKernData")) {
                read_kerns();
                break;
            }
            if (l.isall("StartComposites %d", (int *)0)) {
                read_composites();
                break;
            }
            if (l.isall("StdHW %g", &fd( fdStdHW )))
                break;
            if (l.isall("StdVW %g", &fd( fdStdVW )))
                break;
            if (l.isall("StartFontMetrics %g", (double *)0))
                break;
            goto invalid;

          case 'U':
            if (l.isall("UnderlinePosition %g", &fd( fdUnderlinePosition )))
                break;
            if (l.isall("UnderlineThickness %g", &fd( fdUnderlineThickness )))
                break;
            goto invalid;

          case 'V':
            if (l.isall("Version %+s", &s)) {
                _afm->set_version(s);
                break;
            }
            if (l.isall("VVector %g %g", (double *)0, (double *)0)) {
                metrics_sets_warning();
                break;
            }
            goto invalid;

          case 'W':
            if (l.isall("Weight %+s", &s)) {
                _afm->set_weight(s);
                break;
            }
            goto invalid;

          case 'X':
            if (l.isall("XHeight %g", &fd( fdXHeight )))
                break;
            goto invalid;

          default:
          invalid:
            invalid_lines++;
            no_match_warning();

        }

  done:
    if (invalid_lines >= l.lineno() - 10)
        return false;
    else
        return true;
}


static Vector<PermString> ligature_left;
static Vector<PermString> ligature_right;
static Vector<PermString> ligature_result;

void
AfmReader::read_char_metric_data() const
{
    int c = -1;
    double wx = UNKDOUBLE;
    double bllx = UNKDOUBLE, blly = 0, burx = 0, bury = 0;
    PermString n;
    PermString ligright, ligresult;

    AfmParser &l = _l;

    l.is("C %d ; WX %g ; N %/s ; B %g %g %g %g ;",
         &c, &wx, &n, &bllx, &blly, &burx, &bury);

    while (l.left()) {

        switch (l.first()) {

          case 'B':
            if (l.is("B %g %g %g %g", &bllx, &blly, &burx, &bury))
                break;
            goto invalid;

          case 'C':
            if (l.is("C %d", &c))
                break;
            if (l.is("CH <%x>", &c))
                break;
            goto invalid;

          case 'E':
            if (l.isall("EndCharMetrics"))
                return;
            goto invalid;

          case 'L':
            if (l.is("L %/s %/s", &ligright, &ligresult)) {
                if (!n)
                    lerror("ligature given, but character has no name");
                else {
                    ligature_left.push_back(n);
                    ligature_right.push_back(ligright);
                    ligature_result.push_back(ligresult);
                }
                break;
            }
            goto invalid;

          case 'N':
            if (l.is("N %/s", &n))
                break;
            goto invalid;

          case 'W':
            if (l.is("WX %g", &wx) ||
                l.is("W0X %g", &wx))
                break;
            if (l.is("W %g %g", &wx, (double *)0) ||
                l.is("W0 %g %g", &wx, (double *)0) ||
                l.is("W0Y %g", (double *)0)) {
                y_width_warning();
                break;
            }
            if (l.is("W1X %g", (double *)0) ||
                l.is("W1Y %g", (double *)0) ||
                l.is("W1 %g %g", (double *)0, (double *)0)) {
                metrics_sets_warning();
                break;
            }
            goto invalid;

          default:
          invalid:
            // always warn about unknown directives here!
            no_match_warning("character metrics");
            l.skip_until(';');
            break;

        }

        l.is(";"); // get rid of any possible semicolon
    }

    // create the character
    if (!n)
        lwarning("character without a name ignored");
    else {
        if (_afm->find(n) != -1)
            lwarning("character %s defined twice", n.c_str());

        GlyphIndex gi = _afm->add_glyph(n);

        _afm->wd(gi) = wx;
        _afm->lf(gi) = bllx;
        _afm->rt(gi) = burx;
        _afm->tp(gi) = bury;
        _afm->bt(gi) = blly;

        if (c != -1)
            _afm->set_code(gi, c);
    }
}


void
AfmReader::read_char_metrics() const
{
    assert(!ligature_left.size());

    while (_l.next_line())
        // Grok the whole line. Are we on a character metric data line?
        switch (_l.first()) {

          case 'C':
            if (isspace(_l[1]) || (_l[1] == 'H' && isspace(_l[2]))) {
                read_char_metric_data();
                break;
            }
            if (_l.is("Comment"))
                break;
            goto invalid;

          case 'E':
            if (_l.isall("EndCharMetrics"))
                goto end_char_metrics;
            goto invalid;

          default:
          invalid:
            no_match_warning();

        }

  end_char_metrics:

    for (int i = 0; i < ligature_left.size(); i++) {
        GlyphIndex leftgi = find_err(ligature_left[i], "ligature");
        GlyphIndex rightgi = find_err(ligature_right[i], "ligature");
        GlyphIndex resultgi = find_err(ligature_result[i], "ligature");
        if (leftgi >= 0 && rightgi >= 0 && resultgi >= 0)
            if (_afm->add_lig(leftgi, rightgi, resultgi))
                lwarning("duplicate ligature; first ignored");
    }
    ligature_left.clear();
    ligature_right.clear();
    ligature_result.clear();
}


void
AfmReader::read_kerns() const
{
    double kx;
    PermString left, right;
    GlyphIndex leftgi, rightgi;

    AfmParser &l = _l;
    // AFM files have reversed pair programs when read.
    _afm->pair_program()->set_reversed(true);

    while (l.next_line())
        switch (l.first()) {

          case 'C':
            if (l.is("Comment"))
                break;
            goto invalid;

          case 'E':
            if (l.isall("EndKernPairs"))
                break;
            if (l.isall("EndKernData"))
                return;
            if (l.isall("EndTrackKern"))
                break;
            goto invalid;

          case 'K':
            if (l.isall("KPX %/s %/s %g", &left, &right, &kx)) {
                goto validkern;
            }
            if (l.isall("KP %/s %/s %g %g", &left, &right, &kx, (double *)0)) {
                y_width_warning();
                goto validkern;
            }
            if (l.isall("KPY %/s %/s %g", &left, &right, (double *)0)) {
                y_width_warning();
                break;
            }
            if (l.isall("KPH <%x> <%x> %g %g", (int *)0, (int *)0,
                        (double *)0, (double *)0)) {
                lwarning("KPH not supported");
                break;
            }
            goto invalid;

          validkern:
            leftgi = find_err(left, "kern");
            rightgi = find_err(right, "kern");
            if (leftgi >= 0 && rightgi >= 0)
                // A kern with 0 amount is NOT useless!
                // (Because of multiple masters.)
                if (_afm->add_kern(leftgi, rightgi, _afm->add_kv(kx)))
                    lwarning("duplicate kern; first pair ignored");
            break;

          case 'S':
            if (l.isall("StartKernPairs %d", (int *)0) ||
                l.isall("StartKernPairs0 %d", (int *)0))
                break;
            if (l.isall("StartKernPairs1 %d", (int *)0)) {
                metrics_sets_warning();
                break;
            }
            if (l.isall("StartTrackKern %d", (int *)0))
                break;
            goto invalid;

          case 'T':
            if (l.isall("TrackKern %g %g %g %g %g", (double *)0, (double *)0,
                        (double *)0, (double *)0, (double *)0))
                break; // FIXME: implement TrackKern
            goto invalid;

          default:
          invalid:
            no_match_warning();
            break;

        }
}


void
AfmReader::read_composites() const
{
    while (_l.next_line())
        switch (_l.first()) {

          case 'C':
            if (_l.is("Comment"))
                break;
            if (_l.is("CC"))
                break;
            goto invalid;

          case 'E':
            if (_l.isall("EndComposites"))
                return;
            goto invalid;

          default:
          invalid:
            no_match_warning();
            break;

        }
}

}
