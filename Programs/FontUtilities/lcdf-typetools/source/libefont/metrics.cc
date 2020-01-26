// -*- related-file-name: "../include/efont/metrics.hh" -*-

/* metrics.{cc,hh} -- generic font metrics
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
#include <efont/metrics.hh>
#include <efont/t1cs.hh>        /* for UNKDOUBLE */
namespace Efont {

Metrics::Metrics()
  : _name_map(-1),
    _scale(1), _fdv(fdLast, UNKDOUBLE),
    _xt_map(0),
    _uses(0)
{
  _xt.push_back((MetricsXt *)0);
}


Metrics::Metrics(PermString font_name, PermString full_name, const Metrics &m)
  : _font_name(font_name),
    _family(m._family), _full_name(full_name), _version(m._version),
    _name_map(m._name_map), _names(m._names), _encoding(m._encoding),
    _scale(1), _fdv(fdLast, UNKDOUBLE),
    _pairp(m._pairp),
    _xt_map(0),
    _uses(0)
{
  reserve_glyphs(m._wdv.size());
  _kernv.resize(m._kernv.size(), UNKDOUBLE);
  _xt.push_back((MetricsXt *)0);
}


Metrics::~Metrics()
{
  assert(_uses == 0);
  for (int i = 1; i < _xt.size(); i++)
    delete _xt[i];
}


void
Metrics::set_font_name(PermString n)
{
  assert(!_font_name);
  _font_name = n;
}


void
Metrics::reserve_glyphs(int amt)
{
  if (amt <= _wdv.size()) return;
  _wdv.resize(amt, UNKDOUBLE);
  _lfv.resize(amt, UNKDOUBLE);
  _rtv.resize(amt, UNKDOUBLE);
  _tpv.resize(amt, UNKDOUBLE);
  _btv.resize(amt, UNKDOUBLE);
  _encoding.reserve_glyphs(amt);
  _pairp.reserve_glyphs(amt);
  for (int i = 1; i < _xt.size(); i++)
    _xt[i]->reserve_glyphs(amt);
}


GlyphIndex
Metrics::add_glyph(PermString n)
{
  if (nglyphs() >= _wdv.size())
    reserve_glyphs(nglyphs() ? nglyphs() * 2 : 64);
  GlyphIndex gi = _names.size();
  _names.push_back(n);
  _name_map.insert(n, gi);
  return gi;
}


static void
set_dimen(Vector<double> &dest, const Vector<double> &src, double scale,
          bool increment)
{
  int c = src.size();
  if (increment)
    for (int i = 0; i < c; i++)
      dest.at_u(i) += src.at_u(i) * scale;
  else if (scale < 0.9999 || scale > 1.0001)
    for (int i = 0; i < c; i++)
      dest.at_u(i) = src.at_u(i) * scale;
  else
    dest = src;
}

void
Metrics::interpolate_dimens(const Metrics &m, double scale, bool increment)
{
  set_dimen(_fdv, m._fdv, scale, increment);
  set_dimen(_wdv, m._wdv, scale, increment);
  set_dimen(_lfv, m._lfv, scale, increment);
  set_dimen(_rtv, m._rtv, scale, increment);
  set_dimen(_tpv, m._tpv, scale, increment);
  set_dimen(_btv, m._btv, scale, increment);
  set_dimen(_kernv, m._kernv, scale, increment);
}


void
Metrics::add_xt(MetricsXt *mxt)
{
  int n = _xt.size();
  _xt.push_back(mxt);
  _xt_map.insert(mxt->kind(), n);
  if (_wdv.size() > 0)
    mxt->reserve_glyphs(_wdv.size());
}

}
