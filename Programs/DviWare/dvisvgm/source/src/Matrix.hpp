/*************************************************************************
** Matrix.hpp                                                           **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2020 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <initializer_list>
#include <istream>
#include <string>
#include <vector>
#include "MessageException.hpp"
#include "Pair.hpp"


struct ParserException : public MessageException {
	explicit ParserException (const std::string &msg) : MessageException(msg) {}
};

class Calculator;

class Matrix {
	friend double det (const Matrix &m);
	friend double det (const Matrix &m, int row, int col);

	public:
		Matrix (const std::string &cmds, Calculator &calc);
		Matrix (double d=0);
		explicit Matrix (const double *v, unsigned size=9);
		explicit Matrix (const std::vector<double> &v, int start=0);
		Matrix (std::initializer_list<double> initlist);
		Matrix& set (double d);
		Matrix& set (const double *v, unsigned size);
		Matrix& set (const std::vector<double> &v, int start=0);
		Matrix& set (const std::string &cmds, Calculator &calc);
		double get (int row, int col) const {return _values[row][col];}
		Matrix& transpose ();
		Matrix& invert ();
		Matrix& rmultiply (const Matrix &tm);
		Matrix& lmultiply (const Matrix &tm);
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
		std::string toSVG () const;
		std::ostream& write (std::ostream &os) const;

		static Matrix parse (std::istream &is, Calculator &c);
		static Matrix parse (const std::string &cmds, Calculator &c);
		static Matrix parseSVGTransform (const std::string &transform);

	private:
		double _values[3][3];  // row x col
};


struct TranslationMatrix : public Matrix {
	TranslationMatrix (double tx, double ty);
};


struct ScalingMatrix : public Matrix {
	ScalingMatrix (double sx, double sy);
};


struct RotationMatrix : public Matrix {
	explicit RotationMatrix (double deg);
};


struct XSkewingMatrix : public Matrix {
	explicit XSkewingMatrix (double deg);
};


struct YSkewingMatrix : public Matrix {
	explicit YSkewingMatrix (double deg);
};


inline std::ostream& operator << (std::ostream &os, const Matrix &m) {
	return m.write(os);
}


double det (const Matrix &m);
double det (const Matrix &m, int row, int col);


#endif
