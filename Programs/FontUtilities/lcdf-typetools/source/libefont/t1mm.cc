// -*- related-file-name: "../include/efont/t1mm.hh" -*-

/* t1mm.{cc,hh} -- Type 1 multiple master font information
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
#include <efont/t1mm.hh>
#include <efont/t1interp.hh>
#include <lcdf/error.hh>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
namespace Efont {

MultipleMasterSpace::MultipleMasterSpace(PermString fn, int na, int nm)
    : CharstringProgram(1000),
      _ok(false), _font_name(fn), _naxes(na), _nmasters(nm),
      _axis_types(na, PermString()), _axis_labels(na, PermString()),
      _design_vector(0), _norm_design_vector(0), _weight_vector(0)
{
}


void
MultipleMasterSpace::set_master_positions(const Vector<NumVector> &mp)
{
    _master_positions = mp;
}

void
MultipleMasterSpace::set_normalize(const Vector<NumVector> &nin,
                            const Vector<NumVector> &nout)
{
    _normalize_in = nin;
    _normalize_out = nout;
}

void
MultipleMasterSpace::set_axis_type(int ax, PermString t)
{
    _axis_types[ax] = t;
}

void
MultipleMasterSpace::set_axis_label(int ax, PermString t)
{
    _axis_labels[ax] = t;
}

void
MultipleMasterSpace::set_design_vector(const NumVector &v)
{
    _default_design_vector = v;
}

void
MultipleMasterSpace::set_weight_vector(const NumVector &v)
{
    _default_weight_vector = v;
}


PermString
MultipleMasterSpace::axis_abbreviation(PermString atype)
{
    if (atype == "Weight")
        return "wt";
    else if (atype == "Width")
        return "wd";
    else if (atype == "OpticalSize")
        return "op";
    else if (atype == "Style")
        return "st";
    else
        return atype;
}


bool
MultipleMasterSpace::error(ErrorHandler *errh, const char *s, ...) const
{
    if (errh) {
        char buf[1024];
        va_list val;
        va_start(val, s);
        assert(strlen(s) < 800);
        sprintf(buf, (s[0] == ' ' ? "%.200s%s" : "%.200s: %s"),
                _font_name.c_str(), s);
        errh->xmessage(ErrorHandler::e_error, buf, val);
        va_end(val);
    }
    return false;
}


bool
MultipleMasterSpace::check(ErrorHandler *errh)
{
    if (_ok)
        return true;

    if (_nmasters <= 0 || _nmasters > 16)
        return error(errh, "number of masters must be between 1 and 16");
    if (_naxes <= 0 || _naxes > 4)
        return error(errh, "number of axes must be between 1 and 4");

    if (_master_positions.size() != _nmasters)
        return error(errh, "bad BlendDesignPositions");
    for (int i = 0; i < _nmasters; i++)
        if (_master_positions[i].size() != _naxes)
            return error(errh, "inconsistent BlendDesignPositions");

    if (_normalize_in.size() != _naxes || _normalize_out.size() != _naxes)
        return error(errh, "bad BlendDesignMap");
    for (int i = 0; i < _naxes; i++)
        if (_normalize_in[i].size() != _normalize_out[i].size())
            return error(errh, "bad BlendDesignMap");

    if (!_axis_types.size())
        _axis_types.assign(_naxes, PermString());
    if (_axis_types.size() != _naxes)
        return error(errh, "bad BlendAxisTypes");

    if (!_axis_labels.size())
        _axis_labels.assign(_naxes, PermString());
    if (_axis_labels.size() != _naxes)
        return error(errh, "bad axis labels");

    if (!_default_design_vector.size())
        _default_design_vector.assign(_naxes, UNKDOUBLE);
    if (_default_design_vector.size() != _naxes)
        return error(errh, "inconsistent design vector");

    if (!_default_weight_vector.size())
        _default_weight_vector.assign(_nmasters, UNKDOUBLE);
    if (_default_weight_vector.size() != _nmasters)
        return error(errh, "inconsistent weight vector");

    _ok = true;
    return true;
}

bool
MultipleMasterSpace::check_intermediate(ErrorHandler *errh)
{
    if (!_ok || _cdv)
        return true;

    for (int a = 0; a < _naxes; a++)
        for (int m = 0; m < _nmasters; m++)
            if (_master_positions[m][a] != 0 && _master_positions[m][a] != 1) {
                if (errh)
                    errh->warning("%s requires intermediate master conversion programs",
                                  _font_name.c_str());
                return false;
            }

    return true;
}


int
MultipleMasterSpace::axis(PermString ax) const
{
    for (int a = 0; a < _naxes; a++)
        if (_axis_types[a] == ax || _axis_labels[a] == ax)
            return a;
    return -1;
}

double
MultipleMasterSpace::axis_low(int ax) const
{
    return _normalize_in[ax][0];
}

double
MultipleMasterSpace::axis_high(int ax) const
{
    return _normalize_in[ax].back();
}


Vector<double>
MultipleMasterSpace::empty_design_vector() const
{
    return Vector<double>(_naxes, UNKDOUBLE);
}

bool
MultipleMasterSpace::set_design(NumVector &design_vector, int ax, double value,
                          ErrorHandler *errh) const
{
    if (ax < 0 || ax >= _naxes)
        return error(errh, " has only %d axes", _naxes);

    if (value < axis_low(ax)) {
        value = axis_low(ax);
        if (errh)
            errh->warning("raising %s's %s to %g", _font_name.c_str(),
                          _axis_types[ax].c_str(), value);
    }
    if (value > axis_high(ax)) {
        value = axis_high(ax);
        if (errh)
            errh->warning("lowering %s's %s to %g", _font_name.c_str(),
                          _axis_types[ax].c_str(), value);
    }

    design_vector[ax] = value;
    return true;
}

bool
MultipleMasterSpace::set_design(NumVector &design_vector, PermString ax_name,
                          double val, ErrorHandler *errh) const
{
    int ax = axis(ax_name);
    if (ax < 0)
        return error(errh, " has no `%s' axis", ax_name.c_str());
    else
        return set_design(design_vector, ax, val, errh);
}


bool
MultipleMasterSpace::normalize_vector(ErrorHandler *errh) const
{
    NumVector &design = *_design_vector;
    NumVector &norm_design = *_norm_design_vector;

    for (int a = 0; a < _naxes; a++)
        if (!KNOWN(design[a])) {
            if (errh)
                errh->error("must specify %s's %s coordinate", _font_name.c_str(), _axis_types[a].c_str());
            return false;
        }

    // Move to normalized design coordinates.
    norm_design.assign(_naxes, UNKDOUBLE);

    if (_ndv) {
        CharstringInterp ai;
        if (!ai.interpret(this, &_ndv))
            return error(errh, "%s in NDV program", ai.error_string().c_str());

    } else
        for (int a = 0; a < _naxes; a++) {
            double d = design[a];
            double nd = UNKDOUBLE;
            const Vector<double> &norm_in = _normalize_in[a];
            const Vector<double> &norm_out = _normalize_out[a];

            if (d < norm_in[0])
                nd = norm_out[0];
            for (int i = 1; i < norm_in.size(); i++)
                if (d >= norm_in[i-1] && d < norm_in[i]) {
                    nd = norm_out[i-1]
                        + ((d - norm_in[i-1])
                           * (norm_out[i] - norm_out[i-1])
                           / (norm_in[i] - norm_in[i-1]));
                    goto done;
                }
            if (d >= norm_in.back())
                nd = norm_out.back();

          done:
            norm_design[a] = nd;
        }

    for (int a = 0; a < _naxes; a++)
        if (!KNOWN(norm_design[a]))
            return error(errh, "bad normalization");

    return true;
}


bool
MultipleMasterSpace::convert_vector(ErrorHandler *errh) const
{
    NumVector &norm_design = *_norm_design_vector;
    NumVector &weight = *_weight_vector;

    weight.assign(_nmasters, 1);

    if (_cdv) {
        CharstringInterp ai;
        if (!ai.interpret(this, &_cdv))
            return error(errh, "%s in CDV program", ai.error_string().c_str());

    } else
        for (int a = 0; a < _naxes; a++)
            for (int m = 0; m < _nmasters; m++) {
                if (_master_positions[m][a] == 0)
                    weight[m] *= 1 - norm_design[a];
                else if (_master_positions[m][a] == 1)
                    weight[m] *= norm_design[a];
                else
                    return error(errh, " requires intermediate master conversion programs");
            }

    return true;
}


bool
MultipleMasterSpace::design_to_norm_design(const NumVector &design_in,
                                           NumVector &norm_design,
                                           ErrorHandler *errh) const
{
    NumVector design(design_in);
    NumVector weight;

    _design_vector = &design;
    _norm_design_vector = &norm_design;
    _weight_vector = &weight;
    if (!normalize_vector(errh))
        return false;
    _design_vector = _norm_design_vector = _weight_vector = 0;

    return true;
}


bool
MultipleMasterSpace::design_to_weight(const NumVector &design_in, NumVector &weight, ErrorHandler *errh) const
{
    NumVector design(design_in);
    NumVector norm_design;

    bool dirty = false;
    for (int i = 0; i < _naxes; i++)
        if (design[i] != _default_design_vector[i])
            dirty = true;

    if (dirty) {
        _design_vector = &design;
        _norm_design_vector = &norm_design;
        _weight_vector = &weight;
        if (!normalize_vector(errh))
            return false;
        if (!convert_vector(errh))
            return false;
        _design_vector = _norm_design_vector = _weight_vector = 0;
    } else
        weight = _default_weight_vector;

    double sum = 0;
    for (int m = 0; m < _nmasters; m++)
        sum += weight[m];
    if (sum < 0.9999 || sum > 1.0001)
        return error(errh, "bad conversion: weight vector doesn't sum to 1");

    // adjust weight vector to max 4 decimal digits of precision, and make it
    // sum to exactly 1
    sum = 0;
    for (int m = 0; m < _nmasters - 1; m++) {
        weight[m] = floor(weight[m] * 10000. + 0.5) / 10000.;
        sum += weight[m];
    }
    weight[_nmasters - 1] = 1 - sum;

    return true;
}


Vector<double> *
MultipleMasterSpace::mm_vector(VectorType t, bool writable) const
{
    if (t == VEC_WEIGHT)
        return _weight_vector;
    else if (t == VEC_NORM_DESIGN)
        return _norm_design_vector;
    else if (t == VEC_DESIGN && !writable)
        return _design_vector;
    else
        return 0;
}

}
