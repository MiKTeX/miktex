/*
  This is part of TeXworks, an environment for working with TeX documents
  Copyright (C) 2022  Stefan Löffler

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

  For links to further information, or to contact the authors,
  see <http://www.tug.org/texworks/>.
*/
#ifndef PHYSICALUNITS_H
#define PHYSICALUNITS_H

namespace QtPDF {

namespace Physical {

class Length
{
  // Internally, we store values in SI base units, i.e., in meters
  double m_val{0.};
public:
  enum Unit { Centimeters, Inches, Bigpoints };

  static constexpr double conversionFactor(const Unit unit) {
  // TODO: Replace by a switch once we switch to C++14
  return (unit == Centimeters ? 1e-2 :
       (unit == Inches ? 25.4e-3 :
       (unit == Bigpoints ? 25.4e-3 / 72. :
      1)));
  }
  static constexpr const char* unitSymbol(const Unit unit) {
  // TODO: Replace by a switch once we switch to C++14
  return (unit == Centimeters ? "cm" :
         (unit == Inches ? "in" :
         (unit == Bigpoints ? "bp" :
          "")));
  }
  Length(const double val, const Unit unit) { setVal(val, unit); }
  void setVal(const double val, const Unit unit) { m_val = val * conversionFactor(unit); }
  double val(const Unit unit) const { return m_val / conversionFactor(unit); }

  static constexpr double convert(const double val, const Unit fromUnit, const Unit toUnit) {
    return val * conversionFactor(fromUnit) / conversionFactor(toUnit);
  }
};

} // namespace Physical

} // namespace QtPDF

#endif // PHYSICALUNITS_H
