/*************************************************************************
** SVGCharHandler.hpp                                                   **
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

#ifndef SVGCHARHANDLER_HPP
#define SVGCHARHANDLER_HPP

#include <memory>
#include <stack>
#include "Color.hpp"
#include "Font.hpp"
#include "Matrix.hpp"


template <typename T>
class CharProperty {
	public:
		CharProperty (const T &v) : _value(v) {}

		void set (const T &v) {
			if (v != _value) {
				_value = v;
				_changed = true;
			}
		}

		const T& get () const      {return _value;}
		operator const T& () const {return _value;}
		bool changed () const      {return _changed;}
		void changed (bool c)      {_changed = c;}

	private:
		T _value;
		bool _changed=false;
};


class XMLElement;


/** Base class for all character handlers. These handlers create SVG representations
 *  for the added characters and append them to the SVG tree. */
class SVGCharHandler {
	public:
		SVGCharHandler () : _matrix(1) {}
		virtual ~SVGCharHandler() =default;
		virtual void setInitialContextNode (XMLElement *node);
		virtual void appendChar (uint32_t c, double x, double y) =0;
		virtual void notifyXAdjusted () {}
		virtual void notifyYAdjusted () {}
		void setColor (const Color &color)      {_color.set(color);}
		void setFont (const Font &font, int id) {_font.set(&font); _fontnum = id;}
		void setMatrix (const Matrix &matrix)   {_matrix.set(matrix);}
		void setVertical (bool vertical)        {_vertical.set(vertical);}
		Color getColor () const                 {return _color.get();}
		const Font* getFont () const            {return _font.get();}
		const Matrix& getMatrix () const        {return _matrix.get();}

	protected:
		virtual void resetContextNode ();
		XMLElement* pushContextNode (std::unique_ptr<XMLElement> node);
		void popContextNode ();

		XMLElement* contextNode () const {
			return _contextNodeStack.empty() ? _initialContextNode : _contextNodeStack.top();
		}

		CharProperty<Color> _color=Color::BLACK;   ///< current color
		CharProperty<const Font*> _font=0;         ///< current font
		int _fontnum=0;                            ///< current font ID
		CharProperty<Matrix> _matrix;              ///< current transformation
		CharProperty<bool> _vertical=false;        ///< current writing mode

	private:
		XMLElement *_initialContextNode= nullptr;  ///< SVG element the generated character nodes are attached to
		std::stack<XMLElement*> _contextNodeStack;
};


/** Base class for all character handlers that create SVG <text> elements. */
class SVGCharTextHandler : public SVGCharHandler {
	public:
		explicit SVGCharTextHandler (bool selectFontByClass) : _selectFontByClass(selectFontByClass) {}

	protected:
		std::unique_ptr<XMLElement> createTextNode (double x, double y) const;

	private:
		bool _selectFontByClass;
};

#endif
