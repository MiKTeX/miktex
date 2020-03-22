/*************************************************************************
** Matrix.cpp                                                           **
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

#include <algorithm>
#include <limits>
#include <sstream>
#include "Calculator.hpp"
#include "Matrix.hpp"
#include "utility.hpp"
#include "XMLString.hpp"

using namespace std;
using math::deg2rad;


/** Computes the determinant of a given matrix */
double det (const Matrix &m) {
	double sum=0;
	for (int i=0; i < 3; ++i) {
		sum += m._values[0][i] * m._values[1][(i+1)%3] * m._values[2][(i+2)%3]
			  - m._values[0][2-i] * m._values[1][(4-i)%3] * m._values[2][(3-i)%3];
	}
	return sum;
}


/** Computes the determinant of the 2x2 submatrix of m where a given
 *  row and column were removed.
 *  @param[in] m base matrix
 *  @param[in] row row to remove
 *  @param[in] col column to remove */
double det (const Matrix &m, int row, int col) {
	int c1 = (col+1)%3, c2 = (col+2)%3;
	int r1 = (row+1)%3, r2 = (row+2)%3;
	if (c1 > c2)
		swap(c1, c2);
	if (r1 > r2)
		swap(r1, r2);
	return m._values[r1][c1] * m._values[r2][c2]
		  - m._values[r1][c2] * m._values[r2][c1];
}


/** Creates a diagonal matrix ((d,0,0),(0,d,0),(0,0,d)).
 *  @param[in] d value of diagonal elements */
Matrix::Matrix (double d) {
	set(d);
}


/** Creates the matrix ((v0,v1,v2),(v3,v4,v5),(v6,v7,v8)).
 *  Expects that array v consists of 'size' elements. If size is less than 9, the
 *  remaining matrix components will be set to those of the identity matrix.
 *  @param[in] v array containing the matrix components
 *  @param[in] size size of array v */
Matrix::Matrix (const double *v, unsigned size) {
	set(v, size);
}


/** Creates the matrix ((v0,v1,v2),(v3,v4,v5),(v6,v7,v8)).
 *  If vector v has less than 9 elements, the remaining matrix components will be set to
 *  those of the identity matrix.
 *  @param[in] v array containing the matrix components
 *  @param[in] start use vector components start,...,start+8 */
Matrix::Matrix (const std::vector<double> &v, int start) {
	set(v, start);
}


Matrix::Matrix (const string &cmds, Calculator &calc) {
	*this = parse(cmds, calc);
}


Matrix::Matrix (initializer_list<double> initlist) {
	int count=0;
	for (auto it=initlist.begin(); it != initlist.end() && count < 9; ++it) {
		_values[count/3][count%3] = *it;
		count++;
	}
	for (; count < 9; count++)
		_values[count/3][count%3] = (count%4 ? 0 : 1);
}


Matrix& Matrix::set (double d) {
	for (int i=0; i < 3; i++)
		for (int j=0; j < 3; j++)
			_values[i][j] = (i==j ? d : 0);
	return *this;
}


Matrix& Matrix::set (const double *v, unsigned size) {
	size = min(size, 9u);
	for (unsigned i=0; i < size; i++)
		_values[i/3][i%3] = v[i];
	for (unsigned i=size; i < 9; i++)
		_values[i/3][i%3] = (i%4 ? 0 : 1);
	return *this;
}


/** Set matrix values ((v0,v1,v2),(v3,v4,v5),(v6,v7,v8)).
 *  If vector v has less than 9 elements, the remaining matrix components will be set to
 *  those of the identity matrix.
 *  @param[in] v array containing the matrix components
 *  @param[in] start use vector components start,...,start+8 */
Matrix& Matrix::set (const vector<double> &v, int start) {
	unsigned size = min((unsigned)v.size()-start, 9u);
	for (unsigned i=0; i < size; i++)
		_values[i/3][i%3] = v[i+start];
	for (unsigned i=size; i < 9; i++)
		_values[i/3][i%3] = (i%4 ? 0 : 1);
	return *this;
}


Matrix& Matrix::set (const string &cmds, Calculator &calc) {
	*this = parse(cmds, calc);
	return *this;
}


Matrix& Matrix::translate (double tx, double ty) {
	if (tx != 0 || ty != 0) {
		TranslationMatrix t(tx, ty);
		lmultiply(t);
	}
	return *this;
}


Matrix& Matrix::scale (double sx, double sy) {
	if (sx != 1 || sy != 1) {
		ScalingMatrix s(sx, sy);
		lmultiply(s);
	}
	return *this;
}


/** Multiplies this matrix by ((cos d, -sin d, 0), (sin d, cos d, 0), (0,0,1)) that
 *  describes an anti-clockwise rotation by d degrees.
 *  @param[in] deg rotation angle in degrees */
Matrix& Matrix::rotate (double deg) {
	RotationMatrix r(deg);
	lmultiply(r);
	return *this;
}


Matrix& Matrix::xskewByAngle (double deg) {
	if (fmod(abs(deg)-90, 180) != 0)
		return xskewByRatio(tan(deg2rad(deg)));
	return *this;
}


Matrix& Matrix::xskewByRatio (double xyratio) {
	if (xyratio != 0) {
		double v[] = {1, xyratio};
		Matrix t(v, 2);
		lmultiply(t);
	}
	return *this;
}


Matrix& Matrix::yskewByAngle (double deg) {
	if (fmod(abs(deg)-90, 180) != 0)
		return yskewByRatio(tan(deg2rad(deg)));
	return *this;
}


Matrix& Matrix::yskewByRatio (double xyratio) {
	if (xyratio != 0) {
		double v[] = {1, 0, 0, xyratio};
		Matrix t(v, 4);
		lmultiply(t);
	}
	return *this;
}


Matrix& Matrix::flip (bool haxis, double a) {
	double s = 1;
	if (haxis) // mirror at horizontal axis?
		s = -1;
	double v[] = {-s, 0, (haxis ? 0 : 2*a), 0, s, (haxis ? 2*a : 0), 0, 0, 1};
	Matrix t(v);
	lmultiply(t);
	return *this;
}


/** Swaps rows and columns of the matrix. */
Matrix& Matrix::transpose () {
	for (int i=0; i < 3; i++)
		for (int j=i+1; j < 3; j++)
			swap(_values[i][j], _values[j][i]);
	return *this;
}


/** Multiplies this matrix M with matrix tm (tm is the factor on the right side): M := M * tm */
Matrix& Matrix::rmultiply (const Matrix &tm) {
	Matrix ret;
	for (int i=0; i < 3; i++)
		for (int j=0; j < 3; j++)
			for (int k=0; k < 3; k++)
				ret._values[i][j] += _values[i][k] * tm._values[k][j];
	return *this = ret;
}


/** Multiplies this matrix M with matrix tm (tm is the factor on the left side): M := tm * M */
Matrix& Matrix::lmultiply (const Matrix &tm) {
	Matrix ret;
	for (int i=0; i < 3; i++)
		for (int j=0; j < 3; j++)
			for (int k=0; k < 3; k++)
				ret._values[i][j] += tm._values[i][k] * _values[k][j];
	return *this = ret;
}


Matrix& Matrix::invert () {
	Matrix ret;
	double denom = det(*this);
	if (abs(denom) < 1e-12)
		throw exception();
	for (int i=0; i < 3; ++i) {
		for (int j=0; j < 3; ++j) {
			ret._values[j][i] = det(*this, i, j)/denom;
			if ((i+j)%2 != 0)
				ret._values[j][i] *= -1;
		}
	}
	return *this = ret;
}


Matrix& Matrix::operator *= (double c) {
	for (int i=0; i < 3; i++)
		for (int j=0; j < 3; j++)
			_values[i][j] *= c;
	return *this;
}


DPair Matrix::operator * (const DPair &p) const {
	double pp[] = {p.x(), p.y(), 1};
	double ret[]= {0, 0};
	for (int i=0; i < 2; i++)
		for (int j=0; j < 3; j++)
			ret[i] += _values[i][j] * pp[j];
	return DPair(ret[0], ret[1]);
}


/** Returns true if this matrix equals. Checks equality by comparing the matrix components. */
bool Matrix::operator == (const Matrix &m) const {
	for (int i=0; i < 2; i++)
		for (int j=0; j < 3; j++)
			if (std::abs(_values[i][j]-m._values[i][j]) >= numeric_limits<double>::epsilon())
				return false;
	return true;
}


/** Returns true if this matrix doesn't equal m. Checks inequality by comparing the matrix components. */
bool Matrix::operator != (const Matrix &m) const {
	for (int i=0; i < 2; i++)
		for (int j=0; j < 3; j++)
			if (std::abs(_values[i][j]-m._values[i][j]) >= numeric_limits<double>::epsilon())
				return true;
	return false;
}


/** Returns true if this matrix is the identity matrix ((1,0,0),(0,1,0),(0,0,1)). */
bool Matrix::isIdentity() const {
	for (int i=0; i < 2; i++)
		for (int j=0; j < 3; j++) {
			const double &v = _values[i][j];
			if ((i == j && v != 1) || (i != j && v != 0))
				return false;
		}
	return true;
}


/** Checks whether this matrix describes a plain translation (without any other transformations).
 *  If so, the parameters tx and ty are filled with the translation components.
 *  @param[out] tx horizontal translation
 *  @param[out] ty vertical translation
 *  @return true if matrix describes a pure translation */
bool Matrix::isTranslation (double &tx, double &ty) const {
	tx = _values[0][2];
	ty = _values[1][2];
	for (int i=0; i < 3; i++)
		for (int j=0; j < 2; j++) {
			const double &v = _values[i][j];
			if ((i == j && v != 1) || (i != j && v != 0))
				return false;
		}
	return _values[2][2] == 1;
}


/** Gets a parameter for the transformation command.
 *  @param[in] is parameter chars are read from this stream
 *  @param[in] calc parameters can be arithmetic expressions, so we need a calculator to evaluate them
 *  @param[in] def default value if parameter is optional
 *  @param[in] optional true if parameter is optional
 *  @param[in] leadingComma true if first non-blank must be a comma
 *  @return value of argument */
static double getArgument (istream &is, Calculator &calc, double def, bool optional, bool leadingComma) {
	is >> ws;
	if (!optional && leadingComma && is.peek() != ',')
		throw ParserException("',' expected");
	if (is.peek() == ',') {
		is.get();         // skip comma
		optional = false; // now we expect a parameter
	}
	string expr;
	while (!isupper(is.peek()) && is.peek() != ',' && is)
		expr += (char)is.get();
	if (expr.length() == 0) {
		if (optional)
			return def;
		else
			throw ParserException("parameter expected");
	}
	return calc.eval(expr);
}


Matrix Matrix::parse (istream &is, Calculator &calc) {
	Matrix ret(1);
	while (is) {
		is >> ws;
		int cmd = is.get();
		switch (cmd) {
			case 'T': {
				double tx = getArgument(is, calc, 0, false, false);
				double ty = getArgument(is, calc, 0, true, true);
				ret.translate(tx, ty);
				break;
			}
			case 'S': {
				double sx = getArgument(is, calc, 1, false, false);
				double sy = getArgument(is, calc, sx, true, true );
				ret.scale(sx, sy);
				break;
			}
			case 'R': {
				double a = getArgument(is, calc, 0, false, false);
				double x = getArgument(is, calc, calc.getVariable("ux")+calc.getVariable("w")/2, true, true);
				double y = getArgument(is, calc, calc.getVariable("uy")+calc.getVariable("h")/2, true, true);
				ret.translate(-x, -y);
				ret.rotate(a);
				ret.translate(x, y);
				break;
			}
			case 'F': {
				int c = is.get();
				if (c != 'H' && c != 'V')
					throw ParserException("'H' or 'V' expected");
				double a = getArgument(is, calc, 0, false, false);
				ret.flip(c == 'H', a);
				break;
			}
			case 'K': {
				int c = is.get();
				if (c != 'X' && c != 'Y')
					throw ParserException("transformation command 'K' must be followed by 'X' or 'Y'");
				double a = getArgument(is, calc, 0, false, false);
				if (std::abs(cos(deg2rad(a))) < numeric_limits<double>::epsilon())
					throw ParserException("illegal skewing angle: " + util::to_string(a) + " degrees");
				if (c == 'X')
					ret.xskewByAngle(a);
				else
					ret.yskewByAngle(a);
				break;
			}
			case 'M': {
				double v[9];
				for (int i=0; i < 6; i++)
					v[i] = getArgument(is, calc, i%4 ? 0 : 1, i!=0, i!=0);
				// third row (0, 0, 1)
				v[6] = v[7] = 0;
				v[8] = 1;
				Matrix tm(v);
				ret.lmultiply(tm);
				break;
			}
			default:
				throw ParserException("transformation command expected (found '" + string(1, cmd) + "' instead)");
		}
	}
	return ret;
}


Matrix Matrix::parse (const string &cmds, Calculator &calc) {
	istringstream iss;
	iss.str(cmds);
	return parse(iss, calc);
}


/** Returns an SVG matrix expression that can be used in transform attributes.
 *  ((a,b,c),(d,e,f),(0,0,1)) => matrix(a d b e c f) */
string Matrix::toSVG () const {
	ostringstream oss;
	oss << "matrix(";
	for (int i=0; i < 3; i++) {
		for (int j=0; j < 2; j++)
			oss << XMLString(_values[j][i]) << ' ';
	}
	oss.seekp(-1, ios::cur) << ')';  // overwrite trailing space character
	return oss.str();
}


ostream& Matrix::write (ostream &os) const {
	os << '(';
	for (int i=0; i < 3; i++) {
		os << '(' << _values[i][0];
		for (int j=1; j < 3; j++)
			os << ',' << _values[i][j];
		os << ')';
		if (i < 2)
			os << ',';
	}
	os << ')';
	return os;
}


static const char* ord_suffix (int n) {
	static const char *suffixes[] = {"th", "st", "nd", "rd"};
	if (abs(n) < 4)
		return suffixes[n];
	return suffixes[0];
}


static void skip_comma_wsp (istream &is) {
	is >> ws;
	if (is.peek() == ',') is.get();
	is >> ws;
}


static size_t parse_transform_cmd (istream &is, string cmd, size_t minparams, size_t maxparams, vector<double> &params) {
	for (int i=0; i < int(cmd.length()); i++) {
		if (is.get() != cmd[i]) {
			is.seekg(-i-1, ios::cur);
			return 0;
		}
	}
	params.clear();
	is >> ws;
	if (is.get() != '(')
		throw ParserException("missing '(' after command '"+cmd+"'");
	for (size_t i=1; i <= maxparams; i++) {
		is >> ws;
		double val;
		if (is.fail())
			throw ParserException(to_string(i)+ord_suffix(i)+" parameter of '"+cmd+"' must be a number");
		is >> val;
		params.push_back(val);
		is >> ws;
		if (i == minparams && is.peek() == ')') {
			is.get();
			return i;
		}
		if (i == maxparams) {
			if (is.peek() != ')')
				throw ParserException("missing ')' at end of command '"+cmd+"'");
			is.get();
		}
		skip_comma_wsp(is);
	}
	return maxparams;
}


static bool ne (double x, double y) {return abs(x-y) >= 1e-6;}
static bool ne_angle (double x, double y) {return abs(x-y) >= 1e-3;}


Matrix Matrix::parseSVGTransform (const string &transform) {
	istringstream iss(transform);
	Matrix matrix(1);
	iss >> ws;
	while (iss) {
		vector<double> params;
		if (parse_transform_cmd(iss, "matrix", 6, 6, params)) {
			if (ne(params[0], 1) || ne(params[1], 0) || ne(params[2], 0) || ne(params[3], 1) || ne(params[4], 0) || ne(params[5], 0))
				matrix.rmultiply({params[0], params[2], params[4], params[1], params[3], params[5]});
		}
		else if (parse_transform_cmd(iss, "rotate", 1, 3, params)) {
			if (params.size() == 1) {
				params.push_back(0);
				params.push_back(0);
			}
			if (ne_angle(fmod(params[0], 360), 0)) {
				bool translate = ne(params[1], 0) || ne(params[2], 0);
				if (translate)
					matrix.rmultiply(TranslationMatrix(params[1], params[2]));
				matrix.rmultiply(RotationMatrix(params[0]));
				if (translate)
					matrix.rmultiply(TranslationMatrix(-params[1], -params[2]));
			}
		}
		else if (parse_transform_cmd(iss, "scale", 1, 2, params)) {
			if (params.size() == 1)
				params.push_back(params[0]);
			if (ne(params[0], 1) || ne(params[1], 1))
				matrix.rmultiply(ScalingMatrix(params[0], params[1]));
		}
		else if (parse_transform_cmd(iss, "skewX", 1, 1, params)) {
			if (ne_angle(fmod(abs(params[0])-90, 180), 0))
				matrix.rmultiply(XSkewingMatrix(params[0]));
		}
		else if (parse_transform_cmd(iss, "skewY", 1, 1, params)) {
			if (ne_angle(fmod(abs(params[0])-90, 180), 0))
				matrix.rmultiply(YSkewingMatrix(params[0]));
		}
		else if (parse_transform_cmd(iss, "translate", 1, 2, params)) {
			if (params.size() == 1)
				params.push_back(0);
			if (ne(params[0], 0) || ne(params[1], 0))
				matrix.rmultiply(TranslationMatrix(params[0], params[1]));
		}
		else {  // invalid command
			string cmd;
			while (isalpha(iss.peek()))
				cmd += char(iss.get());
			if (cmd.empty())
				throw ParserException("unexpected character in transform attribute: "+to_string(char(iss.get())));
			throw ParserException("invalid command in transform attribute: "+cmd);
		}
		skip_comma_wsp(iss);
	}
	return matrix;
}

//////////////////////////////////////////////////////////////////

TranslationMatrix::TranslationMatrix (double tx, double ty) {
	double v[] = {1, 0, tx, 0, 1, ty};
	set(v, 6);
}


ScalingMatrix::ScalingMatrix (double sx, double sy) {
	double v[] = {sx, 0, 0, 0, sy};
	set(v, 5);
}


RotationMatrix::RotationMatrix (double deg) {
	double rad = deg2rad(deg);
	double c = cos(rad);
	double s = sin(rad);
	double v[] = {c, -s, 0, s, c};
	set(v, 5);
}


XSkewingMatrix::XSkewingMatrix (double deg) {
	double xyratio = tan(deg2rad(deg));
	lmultiply(Matrix({1, xyratio}));
}


YSkewingMatrix::YSkewingMatrix (double deg) {
	double xyratio = tan(deg2rad(deg));
	lmultiply(Matrix({1, 0, 0, xyratio}));
}
