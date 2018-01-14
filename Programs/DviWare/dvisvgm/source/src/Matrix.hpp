/*************************************************************************
** Matrix.hpp                                                           **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2018 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <istream>
#include <string>
#include <vector>
#include "MessageException.hpp"
#include "Pair.hpp"


struct ParserException : public MessageException
{
	ParserException (const std::string &msg) : MessageException(msg) {}
};

class Calculator;

class Matrix
{
	friend double det (const Matrix &m);
	friend double det (const Matrix &m, int row, int col);

	public:
		Matrix (const std::string &cmds, Calculator &calc);
		Matrix (double d=0);
		Matrix (double v[], unsigned size=9);
		Matrix (const std::vector<double> &v, int start=0);
		Matrix& set (double d);
		Matrix& set (double v[], unsigned size);
		Matrix& set (const std::vector<double> &v, int start=0);
		Matrix& set (const std::string &cmds, Calculator &calc);
		double get (int row, int col) const {return _values[row][col];}
		Matrix& transpose ();
		Matrix& invert ();
		Matrix& parse (std::istream &is, Calculator &c);
		Matrix& parse (const std::string &cmds, Calculator &c);
		Matrix& lmultiply (const Matrix &tm);
		Matrix& rmultiply (const Matrix &tm);
		Matrix& translate (double tx, double ty);
		Matrix& translate (const DPair &p)   {return translate(p.x(), p.y());}
		Matrix& scale (double sx, double sy);
		Matrix& scale (const DPair &p)       {return scale(p.x(), p.y());}
		Matrix& rotate (double deg);
		Matrix& xskewByAngle (double deg);
		Matrix& yskewByAngle (double deg);
		Matrix& xskewByRatio (double xyratio);
		Matrix& yskewByRatio (double xyratio);
		Matrix& flip (bool h, double a);
		Matrix& operator *= (double c);
		DPair operator * (const DPair &p) const;
		bool operator == (const Matrix &m) const;
		bool operator != (const Matrix &m) const;
		bool isIdentity() const;
		bool isTranslation (double &tx, double &ty) const;
		std::string getSVG () const;
		std::ostream& write (std::ostream &os) const;

	private:
		double _values[3][3];  // row x col
};


struct TranslationMatrix : public Matrix
{
	TranslationMatrix (double tx, double ty);
};


struct ScalingMatrix : public Matrix
{
	ScalingMatrix (double sx, double sy);
};


struct RotationMatrix : public Matrix
{
	RotationMatrix (double deg);
};


inline std::ostream& operator << (std::ostream &os, const Matrix &m) {
	return m.write(os);
}


double det (const Matrix &m);
double det (const Matrix &m, int row, int col);


#endif
