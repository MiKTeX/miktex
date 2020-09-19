/*************************************************************************
** GraphicsPath.hpp                                                     **
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

#pragma once

#include <array>
#include <cctype>
#include <cmath>
#include <deque>
#include <ostream>
#include <type_traits>
#include <mpark/variant.hpp>
#include "BoundingBox.hpp"
#include "EllipticalArc.hpp"
#include "Matrix.hpp"
#include "Pair.hpp"
#include "utility.hpp"
#include "XMLString.hpp"

template <typename T>
class GraphicsPath;

namespace gp {

/// Base class for all path data commands, like moveto, lineto, curveto, etc.
struct CommandBase {};

/** Base class for all path data commands with NUM_POINTS point parameters
 * @tparam NUM_POINTS number of parameter pairs representing points, e.g. 1 for moveto and lineto */
template <typename T, int NUM_POINTS>
class Command : public CommandBase {
	friend class GraphicsPath<T>;
	public:
		int numPoints () const {return NUM_POINTS;}
		Pair<T>& point (int n) {return points[n];}
		const Pair<T>& point (int n) const {return points[n];}

		/** Transforms the command by a given transformation matrix.
 		 *  @params[in] matrix describes the affine transformation to apply
		 *  @params[in] currentPoint the untransformed end point of the preceding command */
		void transform (const Matrix &matrix, const Pair<T> &currentPoint) {
			for (Pair<T> &p : points)
				p = matrix * p;
		}

		/** Returns true if all points are identical to those of another command. */
		bool pointsEqual (const Command &cmd) const {
			for (int i=0; i < NUM_POINTS; i++)
				if (points[i] != cmd.points[i])
					return false;
			return true;
		}

	protected:
		explicit Command () =default;
		explicit Command (std::array<Pair<T>, NUM_POINTS> &&pts) : points(std::move(pts)) {}

	protected:
		std::array<Pair<T>, NUM_POINTS> points;
};

template <typename T>
struct MoveTo : public Command<T, 1> {
	explicit MoveTo (const Pair<T> &p) : Command<T, 1>({p}) {}
};

template <typename T>
struct LineTo : public Command<T, 1> {
	explicit LineTo (const Pair<T> &p) : Command<T, 1>({p}) {}
};

template <typename T>
struct CubicTo : public Command<T, 3> {
	explicit CubicTo (const Pair<T> &p1, const Pair<T> &p2, const Pair<T> &p3) : Command<T, 3>({p1, p2, p3}) {}
};

template <typename T>
struct QuadTo : public Command<T, 2> {
	explicit QuadTo (const Pair<T> &p1, const Pair<T> &p2) : Command<T, 2>({p1, p2}) {}
};

template <typename T>
struct ClosePath : public Command<T, 0> {
	ClosePath () : Command<T, 0>() {}
};

template <typename T>
struct ArcTo : Command<T, 1> {
	ArcTo (T rxx, T ryy, double xrot, bool laf, bool sf, const Pair<T> &pp)
		: Command<T, 1>({pp}), rx(rxx < 0 ? -rxx : rxx), ry(ryy < 0 ? -ryy : ryy),
		  xrotation(xrot), largeArcFlag(laf), sweepFlag(sf) {}

	bool operator == (const ArcTo &arc) const {
		return rx == arc.rx
			&& ry == arc.ry
			&& xrotation == arc.xrotation
			&& largeArcFlag == arc.largeArcFlag
			&& sweepFlag == arc.sweepFlag
			&& this->points[0] == arc.points[0];
	}

	void transform (const Matrix &matrix, const Pair<T> &currentPoint);

	bool operator != (const ArcTo &arc) const {return !(*this == arc);}

	T rx, ry;          ///< length of the semi-major and semi-minor axes
	double xrotation;  ///< rotation of the semi-major axis in degrees
	bool largeArcFlag; ///< if true, the longer arc from start to end point is chosen, else the shorter one
	bool sweepFlag;    ///< if true, arc is drawn in direction of positive angles, else the opposite direction
};

/** Applies an affine transformation described by a given matrix to the arc segment.
 *  @params[in] matrix describes the affine transformation to apply
 *  @params[in] currentPoint the untransformed end point of the preceding command */
template <typename T>
void ArcTo<T>::transform (const Matrix &matrix, const Pair<T> &currentPoint) {
	EllipticalArc arc(currentPoint, rx, ry, math::deg2rad(xrotation), largeArcFlag, sweepFlag, this->points[0]);
	arc.transform(matrix);
	rx = arc.rx();
	ry = arc.ry();
	xrotation = math::rad2deg(arc.rotationAngle());
	largeArcFlag = arc.largeArc();
	sweepFlag = arc.sweepPositive();
	this->points[0] = Pair<T>(arc.endPoint());
}

/** Returns true if two path command objects are identical (same command and same parameters). */
template <typename Cmd1, typename Cmd2>
inline typename std::enable_if<std::is_base_of<CommandBase, Cmd1>::value, bool>::type
operator == (const Cmd1 &cmd1, const Cmd2 &cmd2) {
	if (std::is_convertible<Cmd1, Cmd2>::value && std::is_convertible<Cmd2, Cmd1>::value)
		return cmd1.pointsEqual(cmd2);
	return false;
}

/** Returns true if two path command objects differ (different commands or different parameters). */
template <typename Cmd1, typename Cmd2>
inline typename std::enable_if<std::is_base_of<CommandBase, Cmd1>::value, bool>::type
operator != (const Cmd1 &cmd1, const Cmd2 &cmd2) {
	if (std::is_convertible<Cmd1, Cmd2>::value && std::is_convertible<Cmd2, Cmd1>::value)
		return !cmd1.pointsEqual(cmd2);
	return true;
}

}  // namespace gp


template <typename T>
class GraphicsPath {
	friend class PathClipper;
	public:
		enum class WindingRule {EVEN_ODD, NON_ZERO};
		using Point = Pair<T>;

	protected:

		static XMLString to_param_str (double v, double s, double d, bool leadingSpace) {
			XMLString str(v*s + d);
			if (leadingSpace && (str[0] != '-'))
				str.insert(0, " ");
			return str;
		}

		static XMLString to_param_str (double val, double prev, double s, double d, bool leadingSpace) {
			XMLString str((val-prev)*s + d);
			if (leadingSpace && (str[0] != '-'))
				str.insert(0, " ");
			return str;
		}

		static std::string to_param_str (const Point &p, double sx, double sy, double dx, double dy, bool leadingSpace) {
			return to_param_str(p.x(), sx, dx, leadingSpace) + to_param_str(p.y(), sy, dy, true);
		}

		static std::string to_param_str (const Point &p, const Point &prev, double sx, double sy, double dx, double dy, bool leadingSpace) {
			return to_param_str(p.x()-prev.x(), sx, dx, leadingSpace) + to_param_str(p.y()-prev.y(), sy, dy, true);
		}

		using MoveTo = gp::MoveTo<T>;
		using LineTo = gp::LineTo<T>;
		using CubicTo = gp::CubicTo<T>;
		using QuadTo = gp::QuadTo<T>;
		using ArcTo = gp::ArcTo<T>;
		using ClosePath = gp::ClosePath<T>;

		/// Variant representing a single path command
		using CommandVariant = mpark::variant<MoveTo, LineTo, CubicTo, QuadTo, ArcTo, ClosePath>;

		class IterationVisitor;

	public:
		/** Base class providing several template methods being called when executing
		 *  GraphicsPath::iterate(). */
		class IterationActions {
			friend class IterationVisitor;
			public:
				virtual ~IterationActions () =default;
				virtual void moveto (const Point &p) {}
				virtual void lineto (const Point &p) {}
				virtual void hlineto (const T &x) {}
				virtual void vlineto (const T &y) {}
				virtual void quadto (const Point &p) {}
				virtual void quadto (const Point &p1, const Point &p2) {}
				virtual void cubicto (const Point &p1, const Point &p2) {}
				virtual void cubicto (const Point &p1, const Point &p2, const Point &p3) {}
				virtual void arcto (T rx, T ry, double angle, bool largeArcFlag, bool sweepFlag, const Point &p) {}
				virtual void closepath () {}
				virtual bool quit () {return false;}
				virtual void finished () {}
				const Point& startPoint () const {return _startPoint;}
				const Point& currentPoint () const {return _currentPoint;}

			private:
				Point _startPoint;   ///< first point of the current sub-path
				Point _currentPoint; ///< point reached by preceding path command, or (0,0) otherwise
		};

	protected:
		class ModificationActions : public IterationActions {
			friend class GraphicsPath;
			public:
				explicit ModificationActions (GraphicsPath &path) : _path(path) {}

			protected:
				GraphicsPath& path () {return _path;}
				int commandPos () const {return _commandPos;}

			private:
				GraphicsPath &_path;
				int _commandPos=0;  ///< number of command in path being processed
		};

		class WriteActions : public IterationActions {
			public:
				WriteActions (std::ostream &os, bool rel, double sx, double sy, double dx, double dy)
					: _os(os), _relative(rel), _sx(sx), _sy(sy), _dx(dx), _dy(dy) {}

				void moveto (const Point &p) override  {write('M', {p});}
				void lineto (const Point &p) override  {write('L', {p});}
				void hlineto (const T &x) override     {write('H', x, this->currentPoint().x(), _sx, _dx);}
				void vlineto (const T &y) override     {write('V', y, this->currentPoint().y(), _sy, _dy);}
				void quadto (const Point &p) override {write('T', {p});}
				void quadto (const Point &p1, const Point &p2) override {write('Q', {p1, p2});}
				void cubicto (const Point &p1, const Point &p2) override {write('S', {p1, p2});}
				void cubicto (const Point &p1, const Point &p2, const Point &p3) override {write('C', {p1, p2, p3});}
				void closepath () override {_os << (_relative ? 'z' : 'Z');}

				void arcto (T rx, T ry, double angle, bool largeArcFlag, bool sweepFlag, const Point &p) override {
					Point diff = p-this->currentPoint();
					if (std::abs(diff.x()) < 1e-7 && std::abs(diff.y()) < 1e-7)
						return;
					if (std::abs(rx) < 1e-7 && std::abs(ry) < 1e-7)
						lineto(p);
					else {
						if (std::abs(std::abs(_sx) - std::abs(_sy)) < 1e-7)  {  // symmetric scaling?
							angle *= math::sgn(_sx) * math::sgn(_sy);
							rx *= std::abs(_sx);
							ry *= std::abs(_sx);
						}
						else {  // asymmetric scaling => compute new shape parameters
							EllipticalArc arc(this->currentPoint(), double(rx), double(ry), math::deg2rad(angle), largeArcFlag, sweepFlag, p);
							arc.transform(ScalingMatrix(_sx, _sy));
							angle = math::rad2deg(arc.rotationAngle());
							rx = arc.rx();
							ry = arc.ry();
						}
						_os << (_relative ? 'a' : 'A')
							 << to_param_str(rx, 1.0, 0, false)
							 << to_param_str(ry, 1.0, 0, true)
							 << to_param_str(angle, 1.0, 0, true)
							 << ' ' << (largeArcFlag ? 1 : 0)
							 << ' ' << (sweepFlag ? 1 : 0);
						if (_relative)
							_os << to_param_str(p, this->currentPoint(), _sx, _sy, _dx, _dy, true);
						else
							_os << to_param_str(p, _sx, _sy, _dx, _dy, true);
					}
				}

			protected:
				void write (char cmdchar, std::initializer_list<Point> points) const {
					int count=0;
					if (_relative) {
						_os << char(tolower(cmdchar));
						for (const Point &p : points)
							_os << to_param_str(p, this->currentPoint(), _sx, _sy, _dx, _dy, count++ > 0);
					}
					else {
						_os << cmdchar;
						for (const Point &p : points)
							_os << to_param_str(p, _sx, _sy, _dx, _dy, count++ > 0);
					}
				}

				void write (char cmdchar, T val, T relval, double s, double d) const {
					if (_relative)
						_os << char(tolower(cmdchar)) << to_param_str(val, relval, s, d, false);
					else
						_os << cmdchar << to_param_str(val, s, d, false);
				}

			private:
				std::ostream &_os;  ///< write output to this stream
				bool _relative;     ///< if true, use relative coordinates in path commands
				double _sx, _sy;    ///< horizontal and vertical scaling factors
				double _dx, _dy;    ///< horizontal and vertical translation values
		};

		///////////////////////////////////////////////////////////////////////////////

		/** Calls the corresponding template method of an Action object for the current path command.
		 *  If parameter 'useShortCmds' is true, the visitor operators check whether a command
		 *  can be shortened due to special cases, e.g. horizontal or vertical lines, smooth
		 *  curve connections etc. Otherwise, the full command templates are triggered. */
		class IterationVisitor {
			public:
				IterationVisitor (IterationActions &actions, bool useShortCmds, double eps=1e-7)
					: _actions(actions), _shortCommandsActive(useShortCmds), _eps(eps) {}

				void setPrevCommand (const CommandVariant &prevCommand) {
					_prevCommand = &prevCommand;
				}

				void operator () (const MoveTo &cmd) {
					_actions.moveto(cmd.points[0]);
					_actions._startPoint = _actions._currentPoint = cmd.points[0];
				}

				void operator () (const LineTo &cmd) {
					Point diff = abs(_actions._currentPoint-cmd.points[0]);
					if (diff.x() >= _eps || diff.y() >= _eps) {
						if (!_shortCommandsActive)
							_actions.lineto(cmd.points[0]);
						else {
							if (diff.x() < _eps)
								_actions.vlineto(cmd.points[0].y());
							else if (diff.y() < _eps)
								_actions.hlineto(cmd.points[0].x());
							else
								_actions.lineto(cmd.points[0]);
						}
					}
					_actions._currentPoint = cmd.points[0];
				}

				void operator () (const CubicTo &cmd) {
					bool smooth=false;
					if (_shortCommandsActive) {
						if (auto *prevCubic = mpark::get_if<CubicTo>(_prevCommand)) {
							Point diff = abs(cmd.points[0] - prevCubic->points[2]*T(2) + prevCubic->points[1]);
							if ((smooth = (diff.x() < _eps && diff.y() < _eps)))
								_actions.cubicto(cmd.points[1], cmd.points[2]);
						}
					}
					if (!smooth)
						_actions.cubicto(cmd.points[0], cmd.points[1], cmd.points[2]);
					_actions._currentPoint = cmd.points[2];
				}

				void operator () (const QuadTo &cmd) {
					bool smooth=false;
					if (_shortCommandsActive) {
						if (auto *prevQuad = mpark::get_if<QuadTo>(_prevCommand)) {
							Point diff = abs(cmd.points[0] - prevQuad->points[1] * T(2) + prevQuad->points[0]);
							if ((smooth = (diff.x() < _eps && diff.y() < _eps)))  // is reflection?
								_actions.quadto(cmd.points[1]);
						}
					}
					if (!smooth)
						_actions.quadto(cmd.points[0], cmd.points[1]);
					_actions._currentPoint = cmd.points[1];
				}

				void operator () (const ClosePath &cmd) {
					_actions.closepath();
					_actions._currentPoint = _actions._startPoint;
				}

				void operator () (const ArcTo &cmd) {
					_actions.arcto(cmd.rx, cmd.ry, cmd.xrotation, cmd.largeArcFlag, cmd.sweepFlag, cmd.points[0]);
					_actions._currentPoint = cmd.points[0];
				}

			private:
				IterationActions &_actions;
				bool _shortCommandsActive=false;
				double _eps=1e-7;
				const CommandVariant *_prevCommand=nullptr;
		};

		///////////////////////////////////////////////////////////////////////////////

		/** Transforms all Point parameters of a path command. */
		class TransformVisior {
			public:
				explicit TransformVisior (const Matrix &m) : matrix(m) {}

				template <typename Cmd>	void operator () (Cmd &cmd) {
					Point cp = cmd.point(cmd.numPoints()-1);
					cmd.transform(matrix, _currentPoint);
					_currentPoint = cp;
				}

				void operator () (MoveTo &cmd) {
					Point cp = cmd.point(0);
					cmd.transform(matrix, _currentPoint);
					_startPoint = _currentPoint = cp;
				}

				void operator () (ClosePath &cmd) {
					_currentPoint = _startPoint;
				}

			private:
				const Matrix &matrix;
				Point _startPoint, _currentPoint;  ///< untransformed start end current point
		};

	public:
		explicit GraphicsPath (WindingRule wr=WindingRule::NON_ZERO) : _windingRule(wr) {}

		void setWindingRule (WindingRule wr) {_windingRule = wr;}
		WindingRule windingRule () const     {return _windingRule;}

		void clear () {
			_commands.clear();
		}

		/// Returns true if the path is empty, i.e. there is nothing to draw
		bool empty () const {
			return _commands.empty();
		}

		/// Returns the number of path commands used to describe the path.
		size_t size () const {
			return _commands.size();
		}

		const Point& startPoint () const {return _startPoint;}
		const Point& finalPoint () const {return _finalPoint;}

		/// Insert another path at the beginning of this one.
		void prepend (const GraphicsPath &path) {
			_commands.insert(_commands.begin(), path._commands.begin(), path._commands.end());
		}

		void moveto (const T &x, const T &y) {
			moveto(Point(x, y));
		}

		void moveto (const Point &p) {
			// avoid sequences of several MOVETOs; always use latest
			if (_commands.empty() || !mpark::get_if<MoveTo>(&_commands.back()))
				_commands.emplace_back(MoveTo{p});
			else
				mpark::get<MoveTo>(_commands.back()).points[0] = p;
			_startPoint = _finalPoint = p;
		}

		void lineto (const T &x, const T &y) {
			lineto(Point(x, y));
		}

		void lineto (const Point &p) {
			_commands.emplace_back(LineTo{p});
			_finalPoint = p;
		}

		void quadto (const T &x1, const T &y1, const T &x2, const T &y2) {
			quadto(Point(x1, y1), Point(x2, y2));
		}

		/** Creates a quadratic Bézier segment. */
		void quadto (const Point &p1, const Point &p2) {
			_commands.emplace_back(QuadTo{p1, p2});
			_finalPoint = p2;
		}

		/** Creates a quadratic Bézier segment smoothly extending a preceding one, i.e. the gradients
		 *  of the two curves are identical at the connection point. The control point of the second
		 *  curve is computed as the reflection of the preceding curve's control point at the connection
		 *  point. */
		void quadto (const Point &p2) {
			Point p1;
			if (!_commands.empty()) {
				if (auto qto = mpark::get_if<QuadTo>(&_commands.back()))
					p1 = _finalPoint*T(2) - qto->point(0);  // reflect previous control point at current point
				else                  // previous command isn't a quadto?
					p1 = _finalPoint;  // => use current point as control point
			}
			quadto(p1, p2);
		}

		void cubicto (const T &x1, const T &y1, const T &x2, const T &y2, const T &x3, const T &y3) {
			cubicto(Point(x1, y1), Point(x2, y2), Point(x3, y3));
		}

		/** Creates a cubic Bézier segment. */
		void cubicto (const Point &p1, const Point &p2, const Point &p3) {
			_commands.emplace_back(CubicTo{p1, p2, p3});
			_finalPoint = p3;
		}

		/** Creates a cubic Bézier segment smoothly extending a preceding one, i.e. the gradients
		 *  of the two curves are identical at the connection point. The first control point of
		 *  the second curve is computed as the reflection of the preceding curve's second control
		 *  point at the connection point. */
		void cubicto (const Point &p2, const Point &p3) {
			Point p1;
			if (!_commands.empty()) {
				if (auto cto = mpark::get_if<CubicTo>(&_commands.back()))
					p1 = _finalPoint*T(2) - cto->point(1);  // reflect previous control point at current point
				else                  // previous command isn't a cubicto?
					p1 = _finalPoint;  // => use current point as control point
			}
			cubicto(p1, p2, p3);
		}

		void closepath () {
			if (!_commands.empty() && !mpark::get_if<ClosePath>(&_commands.back())) {
				_commands.emplace_back(ClosePath{});
				_finalPoint = _startPoint;
			}
		}

		void arcto (T rx, T ry, double angle, bool laf, bool sweep, const Point &p) {
			_commands.emplace_back(ArcTo{rx, ry, angle, laf, sweep, p});
			_finalPoint = p;
		}

		/** Detects all open subpaths and closes them by adding a closePath command.
		 *	 Most font formats only support closed outline paths so there are no explicit closePath statements
		 *	 in the glyph's outline description. All open paths are automatically closed by the renderer.
		 *	 This method detects all open paths and adds the missing closePath statement. */
		void closeOpenSubPaths () {
			CommandVariant *prevCmd = nullptr;
			for (auto it=_commands.begin(); it != _commands.end(); ++it) {
				if (mpark::get_if<MoveTo>(&*it) && prevCmd && !mpark::get_if<ClosePath>(prevCmd)) {
					prevCmd = &*it;
					it = _commands.insert(it, ClosePath{})+1;
				}
				else
					prevCmd = &*it;
			}
			if (!_commands.empty() && !mpark::get_if<ClosePath>(&_commands.back()))
				closepath();
		}

		/** Removes redundant path commands commands. Currently, only removes movetos. */
		void removeRedundantCommands () {
			// remove trailing moveto commands
			while (!_commands.empty() && mpark::get_if<MoveTo>(&_commands.back()))
				_commands.pop_back();
			// resolve intermediate sequences of moveto commands
			auto it=_commands.begin();
			if (it == _commands.end())
				return;
			auto prev = it++;
			while (it != _commands.end()) {
				if (!mpark::get_if<MoveTo>(&*prev) || !mpark::get_if<MoveTo>(&*it))
					prev = it++;
				else {
					prev = _commands.erase(prev);  // remove leading MOVETO and advance 'prev' to 'it'
					++it;
				}
			}
		}

		/** Writes the path data as SVG path drawing command to a given output stream.
		 *  @param[in] os output stream used to write the SVG commands to
		 *  @param[in] relative if true, create relative rather than absolute coordinate values
		 *  @param[in] sx horizontal scale factor
		 *  @param[in] sy vertical scale factor
		 *  @param[in] dx horizontal translation in PS point units
		 *  @param[in] dy vertical translation in PS point units */
		void writeSVG (std::ostream &os, bool relative, double sx=1.0, double sy=1.0, double dx=0.0, double dy=0.0) const {
			WriteActions actions(os, relative, sx, sy, dx, dy);
			iterate(actions, true);
		}

		/** Computes the bounding box of the current path.
		 *  @param[out] bbox the computed bounding box */
		BoundingBox computeBBox () const {
			BoundingBox bbox;
			struct BBoxActions : IterationActions {
				explicit BBoxActions (BoundingBox &bb) : bbox(bb) {}
				void moveto (const Point &p) override {bbox.embed(p);}
				void lineto (const Point &p) override {bbox.embed(p);}
				void quadto (const Point &p1, const Point &p2) override {bbox.embed(p1); bbox.embed(p2);}
				void cubicto (const Point &p1, const Point &p2, const Point &p3) override {bbox.embed(p1); bbox.embed(p2); bbox.embed(p3);}
				void arcto (T rx, T ry, double angle, bool laf, bool sweep, const Point &p) override {
					bbox.embed(EllipticalArc(this->currentPoint(), double(rx), double(ry), angle, laf, sweep, p).getBBox());
				}
				BoundingBox &bbox;
			} actions(bbox);
			iterate(actions, false);
			return bbox;
		}

		/** Checks whether the current path describes a dot/point only (with no extent).
		 *  @param[out] p coordinates of the point if path describes a dot
		 *  @return true if path is a dot/point */
		bool isDot (Point &p) const {
			struct DotActions : IterationActions {
				DotActions () : differs(false) {}
				void moveto (const Point &p) override {point = p;}
				void lineto (const Point &p) override {differs = (p != point);}
				void quadto (const Point &p1, const Point &p2) override { differs = (point != p1 || point != p2);}
				void cubicto (const Point &p1, const Point &p2, const Point &p3) override {differs = (point != p1 || point != p2 || point != p3);}
				void arcto (T rx, T ry, double angle, bool largeArcFlag, bool sweepFlag, const Point &p) override { differs = (point != p);}
				bool quit () override {return differs;}
				Point point;
				bool differs;
			} actions;
			iterate(actions, false);
			p = actions.point;
			return !actions.differs;
		}

		/** Replaces all elliptic arcs with cubic Bézier curves. */
		void approximateArcs () {
			struct ArcActions : ModificationActions {
				explicit ArcActions (GraphicsPath &path) : ModificationActions(path) {}
				void arcto (T rx, T ry, double angle, bool largeArcFlag, bool sweepFlag, const Point &p) override {
					EllipticalArc arc(this->currentPoint(), rx, ry, angle, largeArcFlag, sweepFlag, p);
					std::vector<CommandVariant> cmds;
					for (const Bezier &bezier : arc.approximate())
						cmds.emplace_back(CubicTo{bezier.point(1), bezier.point(2), bezier.point(3)});
					this->path().replace(this->commandPos(), cmds);
				}
			} actions(*this);
			iterate(actions);
		}

		/** Transforms the path according to a given Matrix.
		 *  @param[in] matrix Matrix describing the affine transformation */
		void transform (const Matrix &matrix) {
			TransformVisior visior(matrix);
			for (CommandVariant &command : _commands)
				mpark::visit(visior, command);
		}

		/** Returns true if this path equals another one, i.e. it consists the same sequence
		 *  of commands and coordinates. */
		bool operator == (const GraphicsPath &path) const {
			if (size() != path.size())
				return false;
			auto it = _commands.begin();
			for (const auto &cmd : path._commands) {
				if (*it++ != cmd)
					return false;
			}
			return true;
		}

		/** Returns true if this path differs from another one (command-wise). */
		bool operator != (const GraphicsPath &path) const {
			if (size() != path.size())
				return true;
			auto it = _commands.begin();
			for (const auto &cmd : path._commands) {
				if (*it++ != cmd)
					return true;
			}
			return false;
		}

		/** Iterates over all commands defining this path and calls the corresponding template methods.
		 *  In the case of successive bezier curve sequences, control points or tangent slopes are often
		 *  identical so that the path description contains redundant information. SVG provides shorthand
		 *  curve commands that require less parameters. If 'optimize' is true, this method detects such
		 *  command sequences.
		 *  @param[in] actions template methods called by each iteration step
		 *  @param[in] optimize if true, shorthand drawing commands (hlineto, vlineto,...) are considered */
		void iterate (IterationActions &actions, bool optimize) const {
			double eps = XMLString::DECIMAL_PLACES > 0 ? std::pow(10, -XMLString::DECIMAL_PLACES) : 1e-7;
			IterationVisitor visitor(actions, optimize, eps);
			for (const CommandVariant &cmd : _commands) {
				if (actions.quit())
					break;
				mpark::visit(visitor, cmd);
				visitor.setPrevCommand(cmd);
			}
			actions.finished();
		}

	protected:
		/** Replaces a command by a sequence of other ones.
		 *  @param[in] pos position of command to replace (0-based)
		 *  @param[in] cmds commands to insert */
		void replace (int pos, const std::vector<CommandVariant> &cmds) {
			auto it = _commands.end();
			if (!_commands.empty()) {
				it = _commands.begin()+pos;
				it = _commands.erase(it);
			}
			_commands.insert(it, cmds.begin(), cmds.end());
		}

		/** Iterates over all commands of the path and calls the corresponding template methods.
		 *  In contrast to the public iterate() method, this one allows to modify the command sequence.
		 *  @param[in] actions template methods called by each iteration step */
		void iterate (ModificationActions &actions) {
			IterationVisitor visitor(actions, false);
			// no iterators here since they may be invalidated during path modifications
			for (size_t i=0; i < _commands.size(); i++) {
				if (actions.quit())
					break;
				actions._commandPos = i;
				mpark::visit(visitor, _commands[i]);
				visitor.setPrevCommand(_commands[i]);
			}
			actions.finished();
		}

	private:
		std::deque<CommandVariant> _commands; ///< sequence of path commands
		WindingRule _windingRule;
		Point _startPoint; ///< start point of final sub-path
		Point _finalPoint; ///< final point reached by last command in path
};
