/*************************************************************************
** Color.cpp                                                            **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <array>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>
#include "Color.hpp"
#include "utility.hpp"

using namespace std;

bool Color::SUPPRESS_COLOR_NAMES = true;

const Color Color::BLACK(uint32_t(0));
const Color Color::WHITE(uint8_t(255), uint8_t(255), uint8_t(255));


inline uint8_t double_to_byte (double v) {
	v = max(0.0, min(1.0, v));
	return uint8_t(round(255.0*v));
}


/** Reads n double values from input stream is. The values may be
 *  separated by whitespace and/or commas. */
static valarray<double> read_doubles (istream &is, int n) {
	valarray<double> values(n);
	for (double &val : values) {
		is >> ws;
		if (!util::read_double(is, val))
			val = 0;
		is >> ws;
		if (is.peek() == ',') {
			is.get();
			is >> ws;
		}
	}
	return values;
}


/** Creates a color object from a string specifying the color. It can
 *  either be a PS color name or one of the color functions rgb(r,g,b),
 *  cmyk(c,m,y,k) where the parameters are numbers in the interval
 *  from 0 to 1. */
Color::Color (const string &colorstr) {
	if (colorstr.substr(0, 4) == "rgb(") {
		istringstream iss(colorstr.substr(4));
		setRGB(read_doubles(iss, 3));
	}
	else if (colorstr.substr(0, 5) == "cmyk(") {
		istringstream iss(colorstr.substr(5));
		setCMYK(read_doubles(iss, 4));
	}
	else if (!setPSName(colorstr, false))
		setGray(uint8_t(0));
}


Color::Color (const valarray<double> &values, ColorSpace cs) noexcept : _cspace(cs) {
	int n = numComponents(cs);
	int i=0;
	_value = 0;
	for (int shift=(n-1)*8; shift >= 0; shift-=8)
		_value |= double_to_byte(values[i++]) << shift;
}


void Color::setRGB (uint8_t r, uint8_t g, uint8_t b)  {
	_value = (r << 16) | (g << 8) | b;
	_cspace = ColorSpace::RGB;
}


void Color::setRGB (double r, double g, double b) {
	setRGB(double_to_byte(r), double_to_byte(g), double_to_byte(b));
}


/** Sets the color value according to a given hex RGB string of the
 *  form "#123456" or "#123" where the latter is expanded to "#112233".
 *  The leading '#' character is optional.
 *  @param[in] hexString the RGB hex string
 *  @return true if the color value was assigned successfully */
bool Color::setRGBHexString (string hexString) {
	if (!hexString.empty()) {
		if (hexString[0] == '#')
			hexString = hexString.substr(1);
		if (hexString.length() == 3) {
			// expand short form "123" to "112233"
			hexString.resize(6);
			hexString[5] = hexString[4] = hexString[2];
			hexString[3] = hexString[2] = hexString[1];
			hexString[1] = hexString[0];
		}
		if (hexString.length() == 6) {
			try {
				_value = stoi(hexString, nullptr, 16);
				_cspace = ColorSpace::RGB;
				return true;
			}
			catch (...) {
			}
		}
	}
	return false;
}


/** Expects a PostScript color name and sets the color accordingly.
 *  @param[in] name PS color name
 *  @param[in] case_sensitive if true, upper/lower case spelling is significant
 *  @return true if color name could be applied properly */
bool Color::setPSName (string name, bool case_sensitive) {
	_cspace = ColorSpace::RGB;
	if (name[0] == '#') {
		char *p=nullptr;
		_value = uint32_t(strtol(name.c_str()+1, &p, 16));
		while (isspace(*p))
			p++;
		return (*p == 0 && _value <= 0xFFFFFF);
	}

	struct ColorConstant {
		const char *name;
		const uint32_t rgb;
	};
	// converted color constants from color.pro
	static const array<ColorConstant, 68> constants {{
		{"Apricot",        0xFFAD7A},
		{"Aquamarine",     0x2DFFB2},
		{"Bittersweet",    0xC10200},
		{"Black",          0x000000},
		{"Blue",           0x0000FF},
		{"BlueGreen",      0x26FFAA},
		{"BlueViolet",     0x190CF4},
		{"BrickRed",       0xB70000},
		{"Brown",          0x660000},
		{"BurntOrange",    0xFF7C00},
		{"CadetBlue",      0x606DC4},
		{"CarnationPink",  0xFF5EFF},
		{"Cerulean",       0x0FE2FF},
		{"CornflowerBlue", 0x59DDFF},
		{"Cyan",           0x00FFFF},
		{"Dandelion",      0xFFB528},
		{"DarkOrchid",     0x9932CC},
		{"Emerald",        0x00FF7F},
		{"ForestGreen",    0x00E000},
		{"Fuchsia",        0x7202EA},
		{"Goldenrod",      0xFFE528},
		{"Gray",           0x7F7F7F},
		{"Green",          0x00FF00},
		{"GreenYellow",    0xD8FF4F},
		{"JungleGreen",    0x02FF7A},
		{"Lavender",       0xFF84FF},
		{"LimeGreen",      0x7FFF00},
		{"Magenta",        0xFF00FF},
		{"Mahogany",       0xA50000},
		{"Maroon",         0xAD0000},
		{"Melon",          0xFF897F},
		{"MidnightBlue",   0x007091},
		{"Mulberry",       0xA314F9},
		{"NavyBlue",       0x0F75FF},
		{"OliveGreen",     0x009900},
		{"Orange",         0xFF6321},
		{"OrangeRed",      0xFF007F},
		{"Orchid",         0xAD5BFF},
		{"Peach",          0xFF7F4C},
		{"Periwinkle",     0x6D72FF},
		{"PineGreen",      0x00BF28},
		{"Plum",           0x7F00FF},
		{"ProcessBlue",    0x0AFFFF},
		{"Purple",         0x8C23FF},
		{"RawSienna",      0x8C0000},
		{"Red",            0xFF0000},
		{"RedOrange",      0xFF3A21},
		{"RedViolet",      0x9600A8},
		{"Rhodamine",      0xFF2DFF},
		{"RoyalBlue",      0x007FFF},
		{"RoyalPurple",    0x3F19FF},
		{"RubineRed",      0xFF00DD},
		{"Salmon",         0xFF779E},
		{"SeaGreen",       0x4FFF7F},
		{"Sepia",          0x4C0000},
		{"SkyBlue",        0x60FFE0},
		{"SpringGreen",    0xBCFF3D},
		{"Tan",            0xDB9370},
		{"TealBlue",       0x1EF9A3},
		{"Thistle",        0xE068FF},
		{"Turquoise",      0x26FFCC},
		{"Violet",         0x351EFF},
		{"VioletRed",      0xFF30FF},
		{"White",          0xFFFFFF},
		{"WildStrawberry", 0xFF0A9B},
		{"Yellow",         0xFFFF00},
		{"YellowGreen",    0x8EFF42},
		{"YellowOrange",   0xFF9300},
	}};
	if (case_sensitive) {
		const ColorConstant cmppair = {name.c_str(), 0};
		auto it = lower_bound(constants.begin(), constants.end(), cmppair,
			[](const ColorConstant &c1, const ColorConstant &c2) {
				return strcmp(c1.name, c2.name) < 0;
			}
		);
		if (it != constants.end() && it->name == name) {
			_value = it->rgb;
			return true;
		}
	}
	else {
		name = util::tolower(name);
		auto it = find_if(constants.begin(), constants.end(), [&](const ColorConstant &cc) {
			return name == util::tolower(cc.name);
		});
		if (it != constants.end()) {
			_value = it->rgb;
			return true;
		}
	}
	return false;
}


void Color::setHSB (double h, double s, double b) {
	valarray<double> hsb(3), rgb(3);
	hsb[0] = h;
	hsb[1] = s;
	hsb[2] = b;
	HSB2RGB(hsb, rgb);
	setRGB(rgb);
}


void Color::setCMYK (uint8_t c, uint8_t m, uint8_t y, uint8_t k) {
	_value = (c << 24) | (m << 16) | (y << 8) | k;
	_cspace = ColorSpace::CMYK;
}


void Color::setCMYK (double c, double m, double y, double k) {
	setCMYK(double_to_byte(c), double_to_byte(m), double_to_byte(y), double_to_byte(k));
}


void Color::setCMYK (const std::valarray<double> &cmyk) {
	setCMYK(cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
}


void Color::set (ColorSpace colorSpace, VectorIterator<double> &it) {
	switch (colorSpace) {
		case ColorSpace::GRAY: setGray(*it++); break;
		case ColorSpace::RGB : setRGB(*it, *(it+1), *(it+2)); it+=3; break;
		case ColorSpace::LAB : setLab(*it, *(it+1), *(it+2)); it+=3; break;
		case ColorSpace::CMYK: setCMYK(*it, *(it+1), *(it+2), *(it+3)); it+=4; break;
		default: ;
	}
}


Color Color::operator *= (double c) {
	if (abs(c) < 0.001)
		_value &= 0xff000000;
	else if (abs(c-trunc(c)) < 0.999) {
		uint32_t value=0;
		for (int i=0; i < 4; i++) {
			value |= lround((_value & 0xff)*c) << (8*i);
			_value >>= 8;
		}
		_value = value;
	}
	return *this;
}


/** Returns an RGB string representing the color. Depending on the
 *  color value, the string either consists of 3 or 6 hex digits
 *  plus a leading '#' character. */
string Color::rgbString () const {
	uint32_t rgb = getRGBUInt32();
	ostringstream oss;
	oss << '#';
	for (int i=2; i >= 0; i--) {
		oss << setbase(16) << setfill('0') << setw(2)
			 << (((rgb >> (8*i)) & 0xff));
	}
	// check if RGB string can be reduced to a three digit hex value
	// #RRGGBB => #RGB, e.g. #112233 => #123
	string hexstr = oss.str();
	if (hexstr[1] == hexstr[2] && hexstr[3] == hexstr[4] && hexstr[5] == hexstr[6]) {
		hexstr[2] = hexstr[3];
		hexstr[3] = hexstr[5];
		hexstr.resize(4);
	}
	return hexstr;
}


/** Returns an SVG color string representing the current color. The result can
 *  either be an SVG color name or an RGB string of the form #RRGGBB. The latter
 *  will be returned if parameter rgbonly is true or if the SVG standard doesn't
 *  define a name for the current color. */
string Color::svgColorString (bool rgbonly) const {
	if (!rgbonly) {
		struct ColorName {
			uint32_t rgb;
			const char *name;
		};
		static const array<ColorName, 138> colornames {{
			{0x000000, "black"},
			{0x000080, "navy"},
			{0x00008b, "darkblue"},
			{0x0000cd, "mediumblue"},
			{0x0000ff, "blue"},
			{0x006400, "darkgreen"},
			{0x008000, "green"},
			{0x008080, "teal"},
			{0x008b8b, "darkcyan"},
			{0x00bfff, "deepskyblue"},
			{0x00ced1, "darkturquoise"},
			{0x00fa9a, "mediumspringgreen"},
			{0x00ff00, "lime"},
			{0x00ff7f, "springgreen"},
			{0x00ffff, "cyan"},
			{0x191970, "midnightblue"},
			{0x1e90ff, "dodgerblue"},
			{0x20b2aa, "lightseagreen"},
			{0x228b22, "forestgreen"},
			{0x2e8b57, "seagreen"},
			{0x2f4f4f, "darkslategray"},
			{0x32cd32, "limegreen"},
			{0x3cb371, "mediumseagreen"},
			{0x40e0d0, "turquoise"},
			{0x4169e1, "royalblue"},
			{0x4682b4, "steelblue"},
			{0x483d8b, "darkslateblue"},
			{0x48d1cc, "mediumturquoise"},
			{0x4b0082, "indigo"},
			{0x556b2f, "darkolivegreen"},
			{0x5f9ea0, "cadetblue"},
			{0x6495ed, "cornflowerblue"},
			{0x66cdaa, "mediumaquamarine"},
			{0x696969, "dimgray"},
			{0x6a5acd, "slateblue"},
			{0x6b8e23, "olivedrab"},
			{0x708090, "slategray"},
			{0x778899, "lightslategray"},
			{0x7b68ee, "mediumslateblue"},
			{0x7cfc00, "lawngreen"},
			{0x7fff00, "chartreuse"},
			{0x7fffd4, "aquamarine"},
			{0x800000, "maroon"},
			{0x800080, "purple"},
			{0x808000, "olive"},
			{0x808080, "gray"},
			{0x87ceeb, "skyblue"},
			{0x87cefa, "lightskyblue"},
			{0x8a2be2, "blueviolet"},
			{0x8b0000, "darkred"},
			{0x8b008b, "darkmagenta"},
			{0x8b4513, "saddlebrown"},
			{0x8fbc8f, "darkseagreen"},
			{0x90ee90, "lightgreen"},
			{0x9370db, "mediumpurple"},
			{0x9400d3, "darkviolet"},
			{0x98fb98, "palegreen"},
			{0x9932cc, "darkorchid"},
			{0x9acd32, "yellowgreen"},
			{0xa0522d, "sienna"},
			{0xa52a2a, "brown"},
			{0xa9a9a9, "darkgray"},
			{0xadd8e6, "lightblue"},
			{0xadff2f, "greenyellow"},
			{0xafeeee, "paleturquoise"},
			{0xb0c4de, "lightsteelblue"},
			{0xb0e0e6, "powderblue"},
			{0xb22222, "firebrick"},
			{0xb8860b, "darkgoldenrod"},
			{0xba55d3, "mediumorchid"},
			{0xbc8f8f, "rosybrown"},
			{0xbdb76b, "darkkhaki"},
			{0xc0c0c0, "silver"},
			{0xc71585, "mediumvioletred"},
			{0xcd5c5c, "indianred"},
			{0xcd853f, "peru"},
			{0xd2691e, "chocolate"},
			{0xd2b48c, "tan"},
			{0xd3d3d3, "lightgray"},
			{0xd8bfd8, "thistle"},
			{0xda70d6, "orchid"},
			{0xdaa520, "goldenrod"},
			{0xdb7093, "palevioletred"},
			{0xdc143c, "crimson"},
			{0xdcdcdc, "gainsboro"},
			{0xdda0dd, "plum"},
			{0xdeb887, "burlywood"},
			{0xe0ffff, "lightcyan"},
			{0xe6e6fa, "lavender"},
			{0xe9967a, "darksalmon"},
			{0xee82ee, "violet"},
			{0xeee8aa, "palegoldenrod"},
			{0xf08080, "lightcoral"},
			{0xf0e68c, "khaki"},
			{0xf0f8ff, "aliceblue"},
			{0xf0fff0, "honeydew"},
			{0xf0ffff, "azure"},
			{0xf4a460, "sandybrown"},
			{0xf5deb3, "wheat"},
			{0xf5f5dc, "beige"},
			{0xf5f5f5, "whitesmoke"},
			{0xf5fffa, "mintcream"},
			{0xf8f8ff, "ghostwhite"},
			{0xfa8072, "salmon"},
			{0xfaebd7, "antiquewhite"},
			{0xfaf0e6, "linen"},
			{0xfafad2, "lightgoldenrodyellow"},
			{0xfdf5e6, "oldlace"},
			{0xff0000, "red"},
			{0xff00ff, "magenta"},
			{0xff1493, "deeppink"},
			{0xff4500, "orangered"},
			{0xff6347, "tomato"},
			{0xff69b4, "hotpink"},
			{0xff7f50, "coral"},
			{0xff8c00, "darkorange"},
			{0xffa07a, "lightsalmon"},
			{0xffa500, "orange"},
			{0xffb6c1, "lightpink"},
			{0xffc0cb, "pink"},
			{0xffd700, "gold"},
			{0xffdab9, "peachpuff"},
			{0xffdead, "navajowhite"},
			{0xffe4b5, "moccasin"},
			{0xffe4c4, "bisque"},
			{0xffe4e1, "mistyrose"},
			{0xffebcd, "blanchedalmond"},
			{0xffefd5, "papayawhip"},
			{0xfff0f5, "lavenderblush"},
			{0xfff5ee, "seashell"},
			{0xfff8dc, "cornsilk"},
			{0xfffacd, "lemonchiffon"},
			{0xfffaf0, "floralwhite"},
			{0xfffafa, "snow"},
			{0xffff00, "yellow"},
			{0xffffe0, "lightyellow"},
			{0xfffff0, "ivory"},
			{0xffffff, "white"}
		}};
		ColorName cmppair = {_value, nullptr};
		auto it = lower_bound(colornames.begin(), colornames.end(), cmppair, [](const ColorName &c1, const ColorName &c2) {
			return c1.rgb < c2.rgb;
		});
		if (it != colornames.end() && it->rgb == _value)
			return it->name;
	}
	return rgbString();
}


valarray<double> Color::getDoubleValues () const {
	int n = numComponents(_cspace);
	valarray<double> values(n);
	int i=0;
	for (int shift= (n-1)*8; shift >= 0; shift-=8)
		values[i++] = ((_value >> shift) & 0xff)/255.0;
	return values;
}


static uint32_t interpolate_cmyk2rgb (const valarray<double> &cmyk);

/** Approximates a CMYK color by an RGB triple. The component values
 *  are expected to be normalized, i.e. 0 <= cmyk[i],rgb[j] <= 1.
 *  @param[in]  cmyk color in CMYK space
 *  @param[out] rgb  RGB approximation */
void Color::CMYK2RGB (const valarray<double> &cmyk, valarray<double> &rgb) {
#if 0
	double cc = 1-cmyk[0];
	double mc = 1-cmyk[1];
	double yc = 1-cmyk[2];
	double kc = 1-cmyk[3];
	// https://graphicdesign.stackexchange.com/a/137902
	rgb[0] = 0.3137+0.5882*cc-0.3529*mc-0.1373*yc+0.47175*cc*mc+0.1173*yc*cc;
	rgb[1] = 0.2588-0.1961*cc+0.2745*mc-0.0627*yc+0.54825*cc*mc+0.0204*yc*cc+0.1581*yc*mc;
	rgb[2] = 0.3373-0.3255*cc-0.1569*mc+0.1647*yc+0.1173*cc*mc+0.31365*yc*cc+0.54825*yc*mc;
	for (int i=0; i < 3; i++)
		rgb[i] = min(1.0, max(0.0, rgb[i]*kc));
#else
	uint32_t rgb32 = interpolate_cmyk2rgb(cmyk);
	rgb[0] = (rgb32 >> 16) / 255.0;
	rgb[1] = ((rgb32 >> 8) & 0xff) / 255.0;
	rgb[2] = (rgb32 & 0xff) / 255.0;
#endif
}


/** Converts a color given in HSB coordinates to RGB.
 *  @param[in]  hsb color in HSB space
 *  @param[out] rgb color in RGB space */
void Color::HSB2RGB (const valarray<double> &hsb, valarray<double> &rgb) {
	if (hsb[1] == 0)
		rgb[0] = rgb[1] = rgb[2] = hsb[2];
	else {
		double h = hsb[0]-floor(hsb[0]);
		int i = int(6*h);
		double f = 6*h-i;
		double p = hsb[2]*(1-hsb[1]);
		double q = hsb[2]*(1-hsb[1]*f);
		double t = hsb[2]*(1-hsb[1]*(1-f));
		switch (i) {
			case 0 : rgb[0]=hsb[2]; rgb[1]=t; rgb[2]=p; break;
			case 1 : rgb[0]=q; rgb[1]=hsb[2]; rgb[2]=p; break;
			case 2 : rgb[0]=p; rgb[1]=hsb[2]; rgb[2]=t; break;
			case 3 : rgb[0]=p; rgb[1]=q; rgb[2]=hsb[2]; break;
			case 4 : rgb[0]=t; rgb[1]=p; rgb[2]=hsb[2]; break;
			case 5 : rgb[0]=hsb[2]; rgb[1]=p; rgb[2]=q; break;
			default: ;  // prevent compiler warning
		}
	}
}


double Color::getGray () const {
	double r, g, b;
	getRGB(r, g, b);
	return r*0.3 + g*0.59 + b*0.11; // gray value according to NTSC video standard
}


void Color::getGray (valarray<double> &gray) const {
	gray.resize(1);
	gray[0] = getGray();
}


valarray<double> Color::getRGBDouble () const {
	valarray<double> values = getDoubleValues();
	valarray<double> rgbValues;
	switch (_cspace) {
		case ColorSpace::RGB:
			rgbValues = std::move(values);
			break;
		case ColorSpace::CMYK:
			rgbValues.resize(3);
			CMYK2RGB(values, rgbValues);
			break;
		case ColorSpace::GRAY:
			rgbValues.resize(3);
			rgbValues[0] = rgbValues[1] = rgbValues[2] = values[0];
			break;
		case ColorSpace::LAB:
			rgbValues.resize(3);
			Lab2XYZ(values, rgbValues);
			XYZ2RGB(rgbValues, values);
			rgbValues = std::move(values);
			break;
		default: ;
	}
	return rgbValues;
}


uint32_t Color::getRGBUInt32 () const {
	switch (_cspace) {
		case ColorSpace::GRAY: {
			uint32_t gray = _value & 0xff;
			return (gray << 16) | (gray << 8) | gray;
		}
		case ColorSpace::RGB:
			return _value;
		default:
			valarray<double> rgb = getRGBDouble();
			return (double_to_byte(rgb[0]) << 16) | (double_to_byte(rgb[1]) << 8) | (double_to_byte(rgb[2]));
	}
}


void Color::getRGB (double &r, double &g, double &b) const {
	valarray<double> rgb = getRGBDouble();
	r = rgb[0];
	g = rgb[1];
	b = rgb[2];
}


void Color::getRGB (valarray<double> &rgb) const {
	rgb = getRGBDouble();
}


void Color::getXYZ (double &x, double &y, double &z) const {
	valarray<double> rgb(3), xyz(3);
	getRGB(rgb);
	RGB2XYZ(std::move(rgb), xyz);
	x = xyz[0];
	y = xyz[1];
	z = xyz[2];
}


void Color::setXYZ (double x, double y, double z) {
	valarray<double> xyz(3), rgb(3);
	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;
	XYZ2RGB(xyz, rgb);
	setRGB(rgb);
	_cspace = ColorSpace::RGB;
}


void Color::setXYZ (const valarray<double> &xyz) {
	valarray<double> rgb(3);
	XYZ2RGB(xyz, rgb);
	setRGB(rgb);
	_cspace = ColorSpace::RGB;
}


void Color::setLab (double l, double a, double b) {
	valarray<double> lab(3), xyz(3);
	lab[0] = l;
	lab[1] = a;
	lab[2] = b;
	Lab2XYZ(lab, xyz);
	setXYZ(xyz);
}


void Color::setLab (const valarray<double> &lab) {
	valarray<double> xyz(3);
	Lab2XYZ(lab, xyz);
	setXYZ(xyz);
}


/** Get the color in CIELAB color space using the sRGB working space and reference white D65. */
void Color::getLab (double &l, double &a, double &b) const {
	valarray<double> rgb(3), lab(3);
	getRGB(rgb);
	RGB2Lab(rgb, lab);
	l = lab[0];
	a = lab[1];
	b = lab[2];
}


void Color::getLab (std::valarray<double> &lab) const {
	lab.resize(3);
	valarray<double> rgb(3);
	getRGB(rgb);
	RGB2Lab(rgb, lab);
}


inline double cube (double x) {return x*x*x;}

void Color::Lab2XYZ (const valarray<double> &lab, valarray<double> &xyz) {
	xyz.resize(3);
	double wx=0.95047, wy=1.00, wz=1.08883;  // reference white D65
	double eps = 0.008856;
	double kappa = 903.3;
	double fy = (lab[0]+16)/116;
	double fx = lab[1]/500 + fy;
	double fz = fy - lab[2]/200;
	double xr = (cube(fx) > eps ? cube(fx) : (116*fx-16)/kappa);
	double yr = (lab[0] > kappa*eps ? cube((lab[0]+16)/116) : lab[0]/kappa);
	double zr = (cube(fz) > eps ? cube(fz) : (116*fz-16)/kappa);
	xyz[0] = xr*wx;
	xyz[1] = yr*wy;
	xyz[2] = zr*wz;
}


void Color::XYZ2RGB (const valarray<double> &xyz, valarray<double> &rgb) {
	rgb.resize(3);
	rgb[0] =  3.2404542*xyz[0] - 1.5371385*xyz[1] - 0.4985314*xyz[2];
	rgb[1] = -0.9692660*xyz[0] + 1.8760108*xyz[1] + 0.0415560*xyz[2];
	rgb[2] =  0.0556434*xyz[0] - 0.2040259*xyz[1] + 1.0572252*xyz[2];
	for (int i=0; i < 3; i++)
		rgb[i] = (rgb[i] <= 0.0031308 ? 12.92*rgb[i] : 1.055*pow(rgb[i], 1/2.4)-0.055);
}


void Color::RGB2XYZ (valarray<double> rgb, valarray<double> &xyz) {
	xyz.resize(3);
	for (int i=0; i < 3; i++)
		rgb[i] = (rgb[i] <= 0.04045 ? rgb[i]/12.92 : pow((rgb[i]+0.055)/1.055, 2.4));
	xyz[0] = 0.4124564*rgb[0] + 0.3575761*rgb[1] + 0.1804375*rgb[2];
	xyz[1] = 0.2126729*rgb[0] + 0.7151522*rgb[1] + 0.0721750*rgb[2];
	xyz[2] = 0.0193339*rgb[0] + 0.1191920*rgb[1] + 0.9503041*rgb[2];
}


void Color::RGB2Lab (const valarray<double> &rgb, valarray<double> &lab) {
	double wx=0.95047, wy=1.00, wz=1.08883;  // reference white D65
	double eps = 0.008856;
	double kappa = 903.3;
	valarray<double> xyz(3);
	RGB2XYZ(rgb, xyz);
	xyz[0] /= wx;
	xyz[1] /= wy;
	xyz[2] /= wz;
	double fx = (xyz[0] > eps ? pow(xyz[0], 1.0/3) : (kappa*xyz[0]+16)/116);
	double fy = (xyz[1] > eps ? pow(xyz[1], 1.0/3) : (kappa*xyz[1]+16)/116);
	double fz = (xyz[2] > eps ? pow(xyz[2], 1.0/3) : (kappa*xyz[2]+16)/116);
	lab[0] = 116*fy-16;
	lab[1] = 500*(fx-fy);
	lab[2] = 200*(fy-fz);
}


#if 0
/** Returns the Delta E difference (CIE 2000) between this and another color. */
double Color::deltaE (const Color &c) const {
	double l1, a1, b1;
	double l2, a2, b2;
	getLab(l1, a1, b1);
	c.getLab(l2, a2, b2);
	const double deltaL = l2-l1;
	const double lBar = (l1+l2)/2;
	const double c1 = hypot(a1, b1);
	const double c2 = hypot(a2, b2);
	const double cBar = (c1+c2)/2.0;
	const double g = (1.0-sqrt(pow(cBar, 7.0)/(pow(cBar, 7.0)+6103515625.0)))/2.0;
	const double aa1 = a1*(1.0+g);
	const double aa2 = a2*(1.0+g);
	const double cc1 = hypot(aa1, b1);
	const double cc2 = hypot(aa2, b2);
	const double ccBar = (cc1+cc2)/2.0;
	const double deltaCC = cc2-cc1;
	double hh1 = atan2(b1, aa1);
	if (hh1 < 0) hh1 += deg2rad(360);
	double hh2 = atan2(b2, aa2);
	if (hh2 < 0) hh2 += deg2rad(360);
	const double hhBar = (std::abs(hh1-hh2) > deg2rad(180) ? (hh1+hh2+deg2rad(360)) : (hh1+hh2))/2.0;
	const double t = 1.0
		- 0.17*cos(hhBar-deg2rad(30.0))
		+ 0.24*cos(2.0*hhBar)
		+ 0.32*cos(3.0*hhBar+deg2rad(6.0))
		- 0.20*cos(4.0*hhBar-deg2rad(63.0));
	double deltaHH = hh2-hh1;
	if (deltaHH < deg2rad(-180))
		deltaHH += deg2rad(360);
	else if (deltaHH > deg2rad(180))
		deltaHH -= deg2rad(360);
	const double deltaHHH = 2.0*sqrt(cc1*cc2)*sin(deltaHH/2.0);
	const double sl = 1.0 + 0.015*sqr(lBar-50.0)/sqrt(20.0+sqr(lBar-50.0));
	const double sc = 1.0 + 0.045*ccBar;
	const double sh = 1.0 + 0.015*ccBar*t;
	const double deltaTheta = deg2rad(30)*exp(-sqr((hhBar-deg2rad(275))/deg2rad(25)));
	const double rc = 2.0*sqrt(pow(ccBar, 7)/(pow(ccBar, 7)+6103515625.0));
	const double rt = -rc*sin(2.0*deltaTheta);
	const double deltaE = sqrt(sqr(deltaL/sl) + sqr(deltaCC/sc) + sqr(deltaHHH/sh) + rt*deltaCC/sc*deltaHHH/sh);
	return deltaE;
}
#endif


int Color::numComponents (ColorSpace colorSpace) {
	switch (colorSpace) {
		case ColorSpace::GRAY: return 1;
		case ColorSpace::LAB:
		case ColorSpace::RGB:  return 3;
		case ColorSpace::CMYK: return 4;
		default: ;
	}
	return 0;
}


struct Cmyk2RgbTable {
	using IntVec = vector<int>;
	using DoubleVec = vector<double>;

	static constexpr const int GRIDSIZE = 5;  // number of grid points per CMYK component
	static array<uint32_t, GRIDSIZE*GRIDSIZE*GRIDSIZE*GRIDSIZE> rgbvalues;
	static const array<int, 4> dimfactors;

	static double interpolate (const IntVec &gi0, const IntVec &gi1, const DoubleVec &t, int icmyk, int irgb, int idx);
};

// For grid indices ci, mi, yi, ki, the corresponding table entry is located
// at index ci + mi*GRIDSIZE + yi*GRIDSIZE^2 + ki*GRIDSIZE^3.
const array<int, 4> Cmyk2RgbTable::dimfactors {{
	1, GRIDSIZE, GRIDSIZE*GRIDSIZE, GRIDSIZE*GRIDSIZE*GRIDSIZE
}};


/** Compute RGB from CMYK by multivariate linear interpolation of sampled color values.
 *  @param[in] cmyk CMYK color components (all values between 0 and 1)
 *  @return 24-bit RGB color value (0x00RRGGBB) */
static uint32_t interpolate_cmyk2rgb (const valarray<double> &cmyk) {
	vector<int> gi0(4), gi1(4);  // upper and lower grid indices enclosing the CMYK components
	vector<double> t(4);   // fractional parts of the grid indices where the CMYK components are located
	for (int i=0; i < 4; i++) {
		double gridpos = math::clip(cmyk[i], 0, 1)*(Cmyk2RgbTable::GRIDSIZE-1);
		double gi;
		t[i] = modf(gridpos, &gi); // distance of i-th CMYK component to nearest lower grid index (in [0,1))
		gi0[i] = int(gi);          // index of nearest grid value <= i-th CMYK component
		gi1[i] = ceil(gridpos);    // index of nearest grid value >= i-th CMYK component
	}
	uint32_t rgb=0;
	for (int i=0; i < 3; i++) {
		double rgb_component = Cmyk2RgbTable::interpolate(gi0, gi1, t, 3, i, 0);
		rgb_component = round(math::clip(rgb_component, 0, 255));
		rgb = (rgb << 8) | int(rgb_component);
	}
	return rgb;
}


inline uint32_t get_byte (uint32_t value, int n) {
	return (value >> n*8) & 0xff;
}


/** Computes an RGB color component for a CMYK color by multi-linear interpolation
 *  using a 4-dimensional grid of pre-computed color values.
 *  @param[in] gi0 indices of nearest grid values <= corresponding CMYK components
 *  @param[in] gi1 indices of nearest grid values >= corresponding CMYK components
 *  @param[in] t fractional parts of the grid indices where the components of the CMYK color are located
 *  @param[in] icmyk index of CMYK component to process (0=C, 1=M, 2=Y, 3=K)
 *  @param[in] irgb index of RGB component to interpolate (0=R, 1=G, 2=B)
 *  @param[in] idx index of RGB value in lookup table (computed during recursion, final value reached when icmyk==0)
 *  @return The interpolated RGB color component in the range from 0 to 255. */
double Cmyk2RgbTable::interpolate (const IntVec &gi0, const IntVec &gi1, const DoubleVec &t, int icmyk, int irgb, int idx) {
	int idx0 = gi0[icmyk]*dimfactors[icmyk]+idx;
	int idx1 = gi1[icmyk]*dimfactors[icmyk]+idx;
	double a, b;
	if (icmyk == 0) {
		a = get_byte(rgbvalues[idx0], 2-irgb);
		b = get_byte(rgbvalues[idx1], 2-irgb);
	}
	else {
		a = interpolate(gi0, gi1, t, icmyk-1, irgb, idx0);
		b = interpolate(gi0, gi1, t, icmyk-1, irgb, idx1);
	}
	return a + (b-a)*t[icmyk];
}


/** RGB values for a 5x5x5x5 grid of CMYK values in [0,1]^4, i.e. the grid values of each dimension
 *  are 0, 0.25, 0.5, 0.75, and 1. For grid coordinates (c,m,y,k), the corresponding RGB values are
 *  located at index ci + 5*mi + 5*5*yi + 5*5*5*ki where ci=4*c, mi=4*m, yi=4*y, ki=4*k.
 *  The RGB values were computed with Little CMS utility 'transicc' using options -t1 and -b
 *  applied to Ghostscript's ICC profiles default_cmyk.icc and default_rgb.icc. */
array<uint32_t, 625> Cmyk2RgbTable::rgbvalues {{
	0xffffff,0xb9e5fa,0x6dcff6,0x00bdf2,0x00aeef,0xf9cbdf,0xbbb8dc,0x7da7d9,0x1c9ad6,0x008fd5,0xf49ac1,0xbc8cbf,0x8781bd,0x4978bc,0x0072bc,0xf067a6,
	0xbd60a5,0x8e5ba5,0x5b57a6,0x0054a6,0xec008c,0xbd1a8d,0x92278f,0x662c91,0x2e3092,0xfffbcc,0xc0e2ca,0x7accc8,0x00bac6,0x00abc5,0xfbc8b4,0xc1b6b3,
	0x86a6b2,0x3799b1,0x008eb0,0xf5989d,0xc08c9c,0x8d819c,0x51789c,0x00719c,0xf16687,0xc06188,0x915c89,0x5f588a,0x03558b,0xed0972,0xbf1e74,0x942977,
	0x682f79,0x32327b,0xfff799,0xc4df9b,0x82ca9c,0x07b89d,0x00a99d,0xfdc689,0xc3b48b,0x8aa48c,0x43978d,0x008c8d,0xf69679,0xc28b7b,0x8f807d,0x56777e,
	0x00707e,0xf1666a,0xc1616c,0x925c6e,0x625870,0x145571,0xed145a,0xbf235e,0x942c61,0x693163,0x343465,0xfff45f,0xc8dd69,0x88c86f,0x2bb673,0x00a775,
	0xfec35a,0xc6b261,0x8ea366,0x4b9669,0x008b6b,0xf79552,0xc48a57,0x927f5c,0x5b775f,0x006f61,0xf26649,0xc2614e,0x945c53,0x645855,0x1c5557,0xed1941,
	0xc02646,0x952e4a,0x6a324d,0x35354e,0xfff200,0xcbdb2a,0x8dc63f,0x39b54a,0x00a650,0xffc20d,0xc8b12f,0x91a23d,0x519546,0x008a4b,0xf7941d,0xc5892f,
	0x947f3a,0x5e7641,0x006f45,0xf26522,0xc3602e,0x955c37,0x66583c,0x22543f,0xed1c24,0xc1272d,0x962e34,0x6b3337,0x363639,0xc7c8ca,0x92b6c7,0x57a5c4,
	0x0097c1,0x008bbf,0xc3a0b2,0x9593b0,0x6486ae,0x177bac,0x0072aa,0xc0799a,0x967099,0x6c6798,0x3a6097,0x005a96,0xbd4f84,0x964b84,0x714784,0x484385,
	0x004185,0xba006f,0x960470,0x741372,0x511b74,0x232176,0xcac6a3,0x97b3a2,0x60a3a0,0x00959f,0x008a9e,0xc59f90,0x98918f,0x6a858e,0x2a7a8e,0x00728d,
	0xc1787d,0x986f7d,0x70677d,0x40607d,0x00597d,0xbd506c,0x984b6c,0x73486d,0x4b446e,0x00416f,0xbb005a,0x970e5c,0x75185e,0x531e60,0x262261,0xcbc37b,
	0x9ab07c,0x65a07d,0x00937d,0x00887e,0xc69c6d,0x9a8f6f,0x6d836f,0x337970,0x007071,0xc17760,0x996e61,0x716663,0x435f63,0x005964,0xbe4f53,0x984b55,
	0x744857,0x4c4458,0x0a4159,0xba0546,0x971549,0x751c4b,0x52214d,0x27244f,0xccc04d,0x9cae53,0x6a9f58,0x1c915c,0x00875e,0xc69a47,0x9b8e4c,0x6f824f,
	0x397853,0x006f55,0xc1753f,0x9a6d43,0x736547,0x465e4a,0x00584c,0xbe4f38,0x994b3b,0x74483f,0x4e4442,0x124144,0xba0e31,0x971934,0x761f37,0x53233a,
	0x28253c,0xccbe00,0x9dad20,0x6d9d30,0x28903a,0x008641,0xc7990b,0x9c8c21,0x71812d,0x3d7735,0x006f3b,0xc27514,0x9b6c21,0x74652a,0x485e30,0x005834,
	0xbe4f17,0x994b20,0x754827,0x4f442c,0x17412f,0xba1319,0x981b1e,0x762123,0x542427,0x2a2529,0x939598,0x6c8896,0x3e7c94,0x007192,0x006991,0x917786,
	0x6f6d84,0x496483,0x055b82,0x005581,0x8f5973,0x705273,0x504b72,0x274572,0x004071,0x8e3762,0x713363,0x543063,0x332d63,0x002b64,0x8c0052,0x710053,
	0x570054,0x3b0256,0x140858,0x95937a,0x6f8679,0x447a78,0x007078,0x006877,0x92766b,0x716c6b,0x4c636a,0x175b6a,0x00546a,0x90585d,0x71515d,0x524b5d,
	0x2b455d,0x00405d,0x8e384f,0x713450,0x553151,0x352e51,0x002c52,0x8c0041,0x710043,0x570144,0x3b0846,0x160d47,0x96915b,0x70845c,0x48785d,0x006e5e,
	0x00665e,0x937450,0x716a51,0x4e6252,0x1e5a53,0x005454,0x905745,0x715047,0x524b48,0x2d4549,0x00404a,0x8d383a,0x71353c,0x55323e,0x362f40,0x002c41,
	0x8b0030,0x700032,0x560835,0x3b0d37,0x161038,0x968f37,0x72823c,0x4a7740,0x006d43,0x006545,0x937231,0x726935,0x506039,0x23593b,0x00533e,0x90562a,
	0x71502e,0x534a31,0x2f4534,0x004036,0x8d3823,0x713527,0x55322a,0x362f2d,0x002d2f,0x8b001d,0x700520,0x560c23,0x3b1026,0x161228,0x968d00,0x728012,
	0x4c7520,0x0d6c28,0x00652e,0x937100,0x726812,0x51601c,0x265823,0x005228,0x905500,0x714f0f,0x534919,0x31441e,0x004022,0x8d3802,0x71350d,0x553215,
	0x37301a,0x012d1d,0x8b0204,0x70090a,0x560f10,0x3b1215,0x171417,0x636466,0x465a65,0x225264,0x004b63,0x004563,0x624e59,0x494758,0x2c4058,0x003a57,
	0x003557,0x61374c,0x4a324c,0x322d4c,0x0f284c,0x00244c,0x601b3f,0x4b1740,0x361441,0x1c1041,0x000d42,0x600032,0x4c0034,0x380035,0x220037,0x000038,
	0x636250,0x475950,0x265150,0x004a50,0x004550,0x624d45,0x494646,0x2e4046,0x003a46,0x003546,0x61363a,0x4a323b,0x332d3c,0x12293c,0x00253c,0x601c30,
	0x4b1931,0x361632,0x1c1333,0x001134,0x5f0024,0x4b0027,0x380029,0x22002a,0x00002b,0x64603a,0x48573b,0x28503c,0x00493d,0x00443d,0x624b31,0x4a4532,
	0x2f3f33,0x003a34,0x003535,0x603628,0x4a312a,0x332d2b,0x14292c,0x00252d,0x5f1d1f,0x4b1a21,0x351823,0x1c1525,0x001226,0x5e0015,0x4b0018,0x37001b,
	0x21001d,0x00001e,0x645f1e,0x495623,0x2a4f26,0x004928,0x00442a,0x624a18,0x4a441c,0x303e1f,0x043922,0x003524,0x603510,0x4a3115,0x332d18,0x15291b,
	0x00261d,0x5f1d07,0x4a1b0d,0x351911,0x1c1714,0x001416,0x5e0000,0x4a0004,0x370008,0x21000c,0x00000e,0x635e00,0x495500,0x2b4e08,0x004811,0x004416,
	0x624a00,0x4a4300,0x303e03,0x07390b,0x003510,0x603500,0x4a3100,0x332d00,0x162904,0x002608,0x5f1e00,0x4a1b00,0x351900,0x1c1700,0x001502,0x5e0000,
	0x4a0000,0x360000,0x210000,0x000000,0x231f20,0x0c1a21,0x001522,0x001123,0x000e23,0x230d15,0x100617,0x000019,0x00001a,0x00001b,0x230008,0x13000d,
	0x00000f,0x000011,0x000012,0x230000,0x150001,0x030005,0x000008,0x00000a,0x230000,0x160000,0x060000,0x000000,0x000001,0x201d12,0x081914,0x001515,
	0x001216,0x001017,0x210c04,0x0e0608,0x00020a,0x00000c,0x00000d,0x220000,0x120000,0x000000,0x000002,0x000003,0x230000,0x140000,0x010000,0x000000,
	0x000000,0x230000,0x150000,0x040000,0x000000,0x000000,0x1e1c00,0x061802,0x001504,0x001306,0x001107,0x200c00,0x0d0700,0x000300,0x000100,0x000000,
	0x210000,0x110000,0x000000,0x000000,0x000000,0x220000,0x130000,0x000000,0x000000,0x000000,0x230000,0x150000,0x030000,0x000000,0x000000,0x1c1b00,
	0x041800,0x001500,0x001300,0x001200,0x1f0b00,0x0b0700,0x000400,0x000200,0x000100,0x210000,0x100000,0x000000,0x000000,0x000000,0x220000,0x120000,
	0x000000,0x000000,0x000000,0x230000,0x140000,0x020000,0x000000,0x000000,0x1b1a00,0x021700,0x001500,0x001400,0x001200,0x1e0b00,0x0a0700,0x000500,
	0x000300,0x000200,0x200000,0x0f0000,0x000000,0x000000,0x000000,0x210000,0x120000,0x000000,0x000000,0x000000,0x220000,0x140000,0x010000,0x000000,
	0x000000
}};
