/*************************************************************************
** SpecialActions.hpp                                                   **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2019 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef SPECIALACTIONS_HPP
#define SPECIALACTIONS_HPP

#include <string>
#include <memory>
#include "BoundingBox.hpp"
#include "Color.hpp"
#include "Matrix.hpp"

class XMLNode;
class XMLElementNode;

class SpecialActions {
	public:
		virtual ~SpecialActions () =default;
		virtual double getX () const =0;
		virtual double getY () const =0;
		virtual void setX (double x) =0;
		virtual void setY (double y) =0;
		virtual void finishLine () =0;
		virtual void setColor (const Color &color) =0;
		virtual Color getColor () const =0;
		virtual void setMatrix (const Matrix &m) =0;
		virtual const Matrix& getMatrix () const =0;
		virtual Matrix getPageTransformation () const {return Matrix(1);}
		virtual void setBgColor (const Color &color) =0;
		virtual void appendToPage(std::unique_ptr<XMLNode> &&node) =0;
		virtual void appendToDefs(std::unique_ptr<XMLNode> &&node) =0;
		virtual void prependToPage(std::unique_ptr<XMLNode> &&node) =0;
		virtual void pushContextElement (std::unique_ptr<XMLElementNode> &&node) =0;
		virtual void popContextElement () =0;
		virtual BoundingBox& bbox () =0;
		virtual BoundingBox& bbox (const std::string &name, bool reset=false) =0;
		virtual void embed (const BoundingBox &bbox) =0;
		virtual void embed (const DPair &p, double r=0) =0;
		virtual unsigned getCurrentPageNumber () const =0;
		virtual std::string getSVGFilename (unsigned pageno) const =0;
		virtual std::string getBBoxFormatString () const =0;
		virtual void progress (const char *id) {}
		virtual int getDVIStackDepth () const  {return 0;}

		static double PROGRESSBAR_DELAY; ///< progress bar doesn't appear before this time has elapsed (in sec)
};


class EmptySpecialActions : public SpecialActions {
	public:
		double getX () const override {return 0;}
		double getY () const override {return 0;}
		void setX (double x) override {}
		void setY (double y) override {}
		void finishLine ()  override {}
		void setColor (const Color &color) override {}
		void setBgColor (const Color &color) override {}
		Color getColor () const override {return Color::BLACK;}
		void setMatrix (const Matrix &m) override {}
		const Matrix& getMatrix () const override {return _matrix;}
		void appendToPage(std::unique_ptr<XMLNode> &&node) override {}
		void appendToDefs(std::unique_ptr<XMLNode> &&node) override {}
		void prependToPage(std::unique_ptr<XMLNode> &&node) override {}
		void pushContextElement (std::unique_ptr<XMLElementNode> &&node) override {}
		void popContextElement () override {}
		BoundingBox& bbox () override {return _bbox;}
		BoundingBox& bbox (const std::string &name, bool reset=false) override {return _bbox;}
		void embed (const BoundingBox &bbox) override {}
		void embed (const DPair &p, double r=0) override {}
		unsigned getCurrentPageNumber() const override {return 0;}
		std::string getSVGFilename (unsigned pageno) const override {return "";}
		std::string getBBoxFormatString () const override {return "";}

	private:
		BoundingBox _bbox;
		Matrix _matrix;
};

#endif
