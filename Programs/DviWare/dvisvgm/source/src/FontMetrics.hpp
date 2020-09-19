/*************************************************************************
** FontMetrics.hpp                                                      **
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

#ifndef FONTMETRICS_HPP
#define FONTMETRICS_HPP

#include <istream>
#include <memory>
#include <string>
#include "MessageException.hpp"

struct FontMetrics {
	virtual ~FontMetrics () =default;
	virtual double getDesignSize () const =0;
	virtual double getCharWidth (int c) const =0;
	virtual double getCharHeight (int c) const =0;
	virtual double getCharDepth (int c) const =0;
	virtual double getItalicCorr (int c) const =0;
	virtual double getSpace () const =0;
	virtual double getSpaceStretch () const =0;
	virtual double getSpaceShrink () const =0;
	virtual double getQuad () const =0;
	virtual double getAscent () const =0;
	virtual double getDescent () const =0;
	virtual bool verticalLayout () const =0;
	virtual uint32_t getChecksum () const =0;
	virtual uint16_t firstChar () const =0;
	virtual uint16_t lastChar () const =0;
	static std::unique_ptr<FontMetrics> read (const std::string &fontname);
};


struct NullFontMetric : public FontMetrics {
	double getDesignSize () const override      {return 1;}
	double getCharWidth (int c) const override  {return 0;}
	double getCharHeight (int c) const override {return 0;}
	double getCharDepth (int c) const override  {return 0;}
	double getItalicCorr (int c) const override {return 0;}
	double getSpace () const override           {return 0;}
	double getSpaceStretch () const override    {return 0;}
	double getSpaceShrink () const override     {return 0;}
	double getQuad () const override            {return 0;}
	double getAscent () const override          {return 0;}
	double getDescent () const override         {return 0;}
	bool verticalLayout () const override       {return false;}
	uint32_t getChecksum () const override      {return 0;}
	uint16_t firstChar () const override        {return 0;}
	uint16_t lastChar () const override         {return 0;}
};


struct FontMetricException : public MessageException {
	explicit FontMetricException (const std::string &msg) : MessageException(msg) {}
};

#endif
