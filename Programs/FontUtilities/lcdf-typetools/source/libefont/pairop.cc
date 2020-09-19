// -*- related-file-name: "../include/efont/pairop.hh" -*-

/* pairop.{cc,hh} -- ligature/kern font metrics
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
#include <efont/pairop.hh>
#include <stdio.h>
namespace Efont {

PairProgram::PairProgram(const PairProgram &o)
  : _reversed(o._reversed),
    _left_map(o._left_map),
    _op(o._op)
{
}


void
PairProgram::reserve_glyphs(int e)
{
  if (e <= _left_map.size()) return;
  _left_map.resize(e, -1);
}


PairOpIndex
PairProgram::find(GlyphIndex leftgi, GlyphIndex rightgi) const
{
  PairOpIndex opi = find_left(leftgi);
  while (opi >= 0) {
    if (op(opi).right() == rightgi)
      return opi;
    opi = op(opi).next_left();
  }
  return -1;
}


bool
PairProgram::add_kern(GlyphIndex left, GlyphIndex right, int ki)
{
  PairOp newop(left, right, ki, _left_map[left]);
  int newopi = _op.size();
  _op.push_back(newop);
  _left_map[left] = newopi;

  //PairOpIndex duplicate = map[newop];
  //map.add(newop, newopi);

  return false;
}


bool
PairProgram::add_lig(GlyphIndex left, GlyphIndex right, GlyphIndex result,
                     int kind)
{
  PairOp newop(left, right, result, kind, _left_map[left]);
  int newopi = _op.size();
  _op.push_back(newop);
  _left_map[left] = newopi;

  //PairOpIndex duplicate = map[newop];
  //map.add(newop, newopi);

  return false;
}


void
PairProgram::unreverse()
{
  if (!_reversed) return;

  _left_map.assign(_left_map.size(), -1);

  for (PairOpIndex opi = _op.size() - 1; opi >= 0; opi--) {
    PairOp &o = _op[opi];
    PairOpIndex l = o.left();
    o.set_next(_left_map[l]);
    _left_map[l] = opi;
  }

  _reversed = false;
}


void
PairProgram::optimize()
{
  /*  PairOpIndex opi;

      // Get rid of 0-valued kerns.
      for (opi = 0; opi < opcount(); opi++) {
      PairOp &o = op(opi);
      if (o.is_kern() && o.value() == 0)
      o.noopify();
      }*/
}


inline const char *
PairProgram::print_name(GlyphIndex) const
{
#if 0
  if (gi == opAnychar) return "*";
  else return glyph(gi).name();
#endif
  return 0;
}


void
PairProgram::print() const
{
#if 0
  for (GlyphIndex gi = 0; gi < glyphblock.size(); gi++)
    if (glyphblock[gi] != -1)
      printf("%s->B%d ", glyph(gi).name().c_str(), glyphblock[gi]);
  printf("\n");

  for (int i = 0; i < blocks.size(); i++) {
    printf("B%-2d:  ", i);
    PairOpBlock &opb = *blocks[i];
    for (int j = 0; j < opb.size(); j++)
      if (opb[j].is_lig())
        printf("%s->%s ", printname(opb[j].right()),
               printname(opb[j].result()));
      else if (opb[j].is_kern())
        printf("%s[%g] ", printname(opb[j].right()), kern(opb[j].value()));
      else if (opb[j].is_noop())
        printf(". ");
    if (blocks[i]->nextblock != -1)
      printf("  :B%d", blocks[i]->nextblock);
    printf("\n");
  }
#endif
}

}
