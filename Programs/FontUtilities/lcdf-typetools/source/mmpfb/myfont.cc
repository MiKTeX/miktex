/* myfont.cc -- general multiple- to single-master conversion
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
#include "myfont.hh"
#include <efont/t1item.hh>
#include <efont/t1interp.hh>
#include "t1rewrit.hh"
#include <efont/t1mm.hh>
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <string.h>
#include <stdio.h>
#include <math.h>
using namespace Efont;

MyFont::MyFont(Type1Reader &reader)
  : Type1Font(reader)
{
}

MyFont::~MyFont()
{
}

void
MyFont::kill_def(Type1Definition *t1d, int whichd)
{
  if (!t1d)
    return;

  if (whichd < 0)
    for (whichd = dFont; whichd < dLast; whichd = (Dict)(whichd + 1))
      if (dict(whichd, t1d->name()) == t1d)
	break;
  if (whichd < 0 || whichd >= dLast || dict(whichd, t1d->name()) != t1d)
    return;

  int icount = nitems();
  for (int i = first_dict_item(whichd); i < icount; i++)
    if (item(i) == t1d) {
      StringAccum sa;
      sa << '%';
      t1d->gen(sa);
      PermString name = t1d->name();
      Type1CopyItem *t1ci = new Type1CopyItem(sa.take_string());
      set_item(i, t1ci);
      set_dict(whichd, name, 0);
      return;
    }

  assert(0);
}

bool
MyFont::set_design_vector(MultipleMasterSpace *mmspace, const Vector<double> &design, ErrorHandler *errh)
{
    Type1Definition *t1d = dict("DesignVector");
    if (t1d) {
	t1d->set_numvec(design);
	kill_def(t1d, dFont);
    }

    t1d = dict("NormDesignVector");
    if (t1d) {
	NumVector norm_design;
	if (mmspace->design_to_norm_design(design, norm_design))
	    t1d->set_numvec(norm_design);
	kill_def(t1d, dFont);
    }

    if (!mmspace->design_to_weight(design, _weight_vector, errh))
	return false;

    // Need to check for case when all design coordinates are unspecified. The
    // font file contains a default WeightVector, but possibly NOT a default
    // DesignVector; we don't want to generate a FontName like
    // `MyriadMM_-9.79797979e97_-9.79797979e97_' because the DesignVector
    // components are unknown.
    if (!KNOWN(design[0])) {
	errh->error("must specify %s%,s %s coordinate", font_name().c_str(),
		    mmspace->axis_type(0).c_str());
	return false;
    }

    t1d = dict("WeightVector");
    if (t1d) {
	t1d->set_numvec(_weight_vector);
	kill_def(t1d, dFont);
    }

    int naxes = design.size();
    _nmasters = _weight_vector.size();

    PermString name;
    t1d = dict("FontName");
    if (t1d && t1d->value_name(name)) {
	StringAccum sa(name);
	for (int a = 0; a < naxes; a++)
	    sa << '_' << design[a];
	// Multiple masters require an underscore AFTER the font name
	sa << '_';
	t1d->set_name(sa.c_str());
	uncache_defs();		// remove cached font name
    }

    // add a FullName too
    String full_name;
    t1d = fi_dict("FullName");
    if (t1d && t1d->value_string(full_name)) {
	StringAccum sa(full_name);
	for (int a = 0; a < naxes; a++) {
	    sa << (a ? ' ' : '_') << design[a];
	    PermString label = mmspace->axis_abbreviation(a);
	    if (label)
		sa << ' ' << label;
	}
	t1d->set_string(sa.c_str());
    }

    // save UniqueID, then kill its definition
    int uniqueid;
    t1d = dict("UniqueID");
    bool have_uniqueid = (t1d && t1d->value_int(uniqueid));
    kill_def(t1d, dFont);

    // prepare XUID
    t1d = dict("XUID");
    NumVector xuid;
    if (!t1d || !t1d->value_numvec(xuid)) {
	if (have_uniqueid) {
	    t1d = ensure(dFont, "XUID");
	    xuid.clear();
	    xuid.push_back(1);
	    xuid.push_back(uniqueid);
	} else if (t1d) {
	    kill_def(t1d, dFont);
	    t1d = 0;
	}
    }
    if (t1d) {
	// Append design vector values to the XUID to prevent cache pollution.
	for (int a = 0; a < naxes; a++)
	    xuid.push_back((int)(design[a] * 100));
	t1d->set_numvec(xuid);
    }

    return true;
}

void
MyFont::interpolate_dict_int(PermString name, Dict the_dict, ErrorHandler *errh)
{
    Type1Definition *def = dict(the_dict, name);
    Type1Definition *blend_def = dict(the_dict + dBlend, name);
    NumVector blend;

    if (def && blend_def && blend_def->value_numvec(blend)) {
	int n = _nmasters;
	double val = 0;
	for (int m = 0; m < n; m++)
	    val += blend[m] * _weight_vector[m];
	int ival = (int)floor(val + 0.50001);
	if (fabs(val - ival) >= 0.001)
	    errh->warning("interpolated %s should be an integer (it is %g)", name.c_str(), val);
	def->set_num(ival);
	kill_def(blend_def, the_dict + dBlend);
    }
}

void
MyFont::interpolate_dict_num(PermString name, Dict the_dict, bool force_integer)
{
    Type1Definition *def = dict(the_dict, name);
    Type1Definition *blend_def = dict(the_dict + dBlend, name);
    NumVector blend;

    if (def && blend_def && blend_def->value_numvec(blend)) {
	int n = _nmasters;
	double val = 0;
	for (int m = 0; m < n; m++)
	    val += blend[m] * _weight_vector[m];
	if (force_integer)
	    val = floor(val + 0.50001);
	def->set_num(val);
	kill_def(blend_def, the_dict + dBlend);
    } else if (def && !blend_def && force_integer) {
	double val;
	if (def->value_num(val))
	    def->set_num(floor(val + 0.50001));
    }
}

void
MyFont::interpolate_dict_numvec(PermString name, Dict the_dict, int round_mode, bool executable)
{
    Type1Definition *def = dict(the_dict, name);
    Type1Definition *blend_def = dict(the_dict + dBlend, name);
    Vector<NumVector> blend;

    if (def && blend_def && blend_def->value_numvec_vec(blend)) {
	int n = blend.size();
	NumVector val;
	for (int i = 0; i < n; i++) {
	    double d = 0;
	    for (int m = 0; m < _nmasters; m++)
		d += blend[i][m] * _weight_vector[m];
	    if (round_mode == 2 && i < 2)
		d = floor(d - 0.50001);
	    else if (round_mode)
		d = floor(d + 0.50001);
	    val.push_back(d);
	}
	def->set_numvec(val, executable);
	kill_def(blend_def, the_dict + dBlend);
    }
}

void
MyFont::interpolate_dicts(bool force_integer, ErrorHandler *errh)
{
  // Unfortunately, some programs (acroread) expect the FontBBox to consist
  // of integers. Round its elements away from zero (this is what the
  // Acrobat distiller seems to do).
  interpolate_dict_numvec("FontBBox", dFont, 2, true);

  interpolate_dict_numvec("BlueValues", dPrivate, force_integer);
  interpolate_dict_numvec("OtherBlues", dPrivate, force_integer);
  interpolate_dict_numvec("FamilyBlues", dPrivate, force_integer);
  interpolate_dict_numvec("FamilyOtherBlues", dPrivate, force_integer);
  interpolate_dict_numvec("StdHW", dPrivate);
  interpolate_dict_numvec("StdVW", dPrivate);
  interpolate_dict_numvec("StemSnapH", dPrivate);
  interpolate_dict_numvec("StemSnapV", dPrivate);

  interpolate_dict_num("BlueScale", dPrivate);
  interpolate_dict_num("BlueShift", dPrivate, force_integer);
  interpolate_dict_int("BlueFuzz", dPrivate, errh);

  {
    Type1Definition *def = p_dict("ForceBold");
    Type1Definition *blend_def = bp_dict("ForceBold");
    Type1Definition *thresh = p_dict("ForceBoldThreshold");
    Vector<PermString> namevec;
    double thresh_val;
    if (def && blend_def && thresh && blend_def->value_namevec(namevec)
	&& thresh->value_num(thresh_val) && namevec.size() == _nmasters) {
      double v = 0;
      for (int m = 0; m < _nmasters; m++)
	if (namevec[m] == "true")
	  v += _weight_vector[m];
      def->set_code(v >= thresh_val ? "true" : "false");
      kill_def(blend_def, dBlendPrivate);
    }
  }

  interpolate_dict_num("UnderlinePosition", dFontInfo);
  interpolate_dict_num("UnderlineThickness", dFontInfo);
  interpolate_dict_num("ItalicAngle", dFontInfo);

  if (Type1Definition *def = bp_dict("BuildCharArray"))
    kill_def(def, dBlendPrivate);

  for (DictHashMap::const_iterator i = dict_begin(dBlend); i; i++) {
      PermString name = i.key();
      if (i.value() && name != "Private" && name != "FontInfo"
	  && name != "ConvertDesignVector" && name != "NormalizeDesignVector")
	  errh->warning("didn%,t interpolate %s in Blend", name.c_str());
  }

  for (DictHashMap::const_iterator i = dict_begin(dBlendPrivate); i; i++)
      if (i.value() && i.key() != "Erode")
	  errh->warning("didn%,t interpolate %s in BlendPrivate", i.key().c_str());

  kill_def(p_dict("NDV"), dPrivate);
  kill_def(p_dict("CDV"), dPrivate);
  kill_def(p_dict("UniqueID"), dPrivate);
  kill_def(fi_dict("BlendDesignPositions"), dFontInfo);
  kill_def(fi_dict("BlendDesignMap"), dFontInfo);
  kill_def(fi_dict("BlendAxisTypes"), dFontInfo);
}

void
MyFont::interpolate_charstrings(int precision, ErrorHandler *errh)
{
    Type1MMRemover remover(this, _weight_vector, precision, errh);
    remover.run();
}
