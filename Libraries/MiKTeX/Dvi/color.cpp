/* color.cpp: color specials

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the MiKTeX DVI Library; if not, write to the
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
   USA.  */

#include "StdAfx.h"

#include "internal.h"

namespace {
  const struct
  {
    const char *        lpszName;
    CmykColor           color;
  } colorNames[] =
  {                             // borrowed from color.lpro (Dvips)
    "Apricot", 0, 0.32, 0.52, 0,
    "Aquamarine", 0.82, 0, 0.30, 0,
    "Bittersweet", 0, 0.75, 1, 0.24,
    "Black", 0, 0, 0, 1,
    "Blue", 1, 1, 0, 0,
    "BlueGreen", 0.85, 0, 0.33, 0,
    "BlueViolet", 0.86, 0.91, 0, 0.04,
    "BrickRed", 0, 0.89, 0.94, 0.28,
    "Brown", 0, 0.81, 1, 0.60,
    "BurntOrange", 0, 0.51, 1, 0,
    "CadetBlue", 0.62, 0.57, 0.23, 0,
    "CarnationPink", 0, 0.63, 0, 0,
    "Cerulean", 0.94, 0.11, 0, 0,
    "CornflowerBlue", 0.65, 0.13, 0, 0,
    "Cyan", 1, 0, 0, 0,
    "Dandelion", 0, 0.29, 0.84, 0,
    "DarkOrchid", 0.40, 0.80, 0.20, 0,
    "Emerald", 1, 0, 0.50, 0,
    "ForestGreen", 0.91, 0, 0.88, 0.12,
    "Fuchsia", 0.47, 0.91, 0, 0.08,
    "Goldenrod", 0, 0.10, 0.84, 0,
    "Gray", 0, 0, 0, 0.50,
    "Green", 1, 0, 1, 0,
    "GreenYellow", 0.15, 0, 0.69, 0,
    "JungleGreen", 0.99, 0, 0.52, 0,
    "Lavender", 0, 0.48, 0, 0,
    "LimeGreen", 0.50, 0, 1, 0,
    "Magenta", 0, 1, 0, 0,
    "Mahogany", 0, 0.85, 0.87, 0.35,
    "Maroon", 0, 0.87, 0.68, 0.32,
    "Melon", 0, 0.46, 0.50, 0,
    "MidnightBlue", 0.98, 0.13, 0, 0.43,
    "Mulberry", 0.34, 0.90, 0, 0.02,
    "NavyBlue", 0.94, 0.54, 0, 0,
    "OliveGreen", 0.64, 0, 0.95, 0.40,
    "Orange", 0, 0.61, 0.87, 0,
    "OrangeRed", 0, 1, 0.50, 0,
    "Orchid", 0.32, 0.64, 0, 0,
    "Peach", 0, 0.50, 0.70, 0,
    "Periwinkle", 0.57, 0.55, 0, 0,
    "PineGreen", 0.92, 0, 0.59, 0.25,
    "Plum", 0.50, 1, 0, 0,
    "ProcessBlue", 0.96, 0, 0, 0,
    "Purple", 0.45, 0.86, 0, 0,
    "RawSienna", 0, 0.72, 1, 0.45,
    "Red", 0, 1, 1, 0,
    "RedOrange", 0, 0.77, 0.87, 0,
    "RedViolet", 0.07, 0.90, 0, 0.34,
    "Rhodamine", 0, 0.82, 0, 0,
    "RoyalBlue", 1, 0.50, 0, 0,
    "RoyalPurple", 0.75, 0.90, 0, 0,
    "RubineRed", 0, 1, 0.13, 0,
    "Salmon", 0, 0.53, 0.38, 0,
    "SeaGreen", 0.69, 0, 0.50, 0,
    "Sepia", 0, 0.83, 1, 0.70,
    "SkyBlue", 0.62, 0, 0.12, 0,
    "SpringGreen", 0.26, 0, 0.76, 0,
    "Tan", 0.14, 0.42, 0.56, 0,
    "TealBlue", 0.86, 0, 0.34, 0.02,
    "Thistle", 0.12, 0.59, 0, 0,
    "Turquoise", 0.85, 0, 0.20, 0,
    "Violet", 0.79, 0.88, 0, 0,
    "VioletRed", 0, 0.81, 0, 0,
    "White", 0, 0, 0, 0,
    "WildStrawberry", 0, 0.96, 0.39, 0,
    "Yellow", 0, 0, 1, 0,
    "YellowGreen", 0.44, 0, 0.74, 0,
    "YellowOrange", 0, 0.42, 1, 0,
  };
}

#if defined(MIKTEX_DEBUG)
STATICFUNC(void)
DebugCheckColorNames()
{
  static bool done = false;
  if (done)
  {
    return;
  }
  for (size_t i = 1; i < sizeof(colorNames) / sizeof(colorNames[0]); ++i)
  {
    MIKTEX_ASSERT(StringCompare(colorNames[i].lpszName,
      colorNames[i - 1].lpszName,
      true)
  > 0);
  }
  done = true;
}
#endif

STATICFUNC(bool)
LookupColorName(/*[in]*/ const char *  lpszName,
  /*[in]*/ CmykColor &   color)
{
#if defined(MIKTEX_DEBUG)
  DebugCheckColorNames();
#endif
  // <fixme>use binary search for efficiency</fixme>
  for (size_t idx = 0;
  idx < sizeof(colorNames) / sizeof(colorNames[0]);
    ++idx)
  {
    if (Utils::EqualsIgnoreCase(colorNames[idx].lpszName, lpszName))
    {
      color = colorNames[idx].color;
      return true;
    }
  }
  return false;
}

namespace GhostscriptApi {
#pragma warning (push, 1)
  typedef double floatp;
  typedef unsigned long ulong;

  /*
   * Represent a fraction in [0.0..1.0].
   * Note that the 1.0 endpoint is included.
   * Since undercolor removal requires a signed frac,
   * we limit fracs to 15 bits rather than 16.
   */
  typedef short frac;
  typedef short signed_frac;
  /*
   * Compute the quotient Q = floor(P / frac_1),
   * where P is the (ulong) product of a uint or ushort V and a frac F.
   * See gxarith.h for the underlying algorithm.
   */
#define frac_1_quo(p)\
  ( (((p) >> frac_1_0bits) + ((p) >> frac_bits) + 1) >> (frac_bits - frac_1_0bits) )

#define arch_log2_sizeof_frac arch_log2_sizeof_short
#define arch_sizeof_frac arch_sizeof_short
#define frac_bits 15
#define frac_0 ((frac)0)
   /* The following definition of frac_1 allows exact representation of */
   /* almost all common fractions (e.g., N/360 for 0<=N<=360). */
#define frac_1_0bits 3
#define frac_1 ((frac)0x7ff8)
#define frac_1_long ((long)frac_1)
#define frac_1_float ((float)frac_1)
/* Conversion between fracs and floats. */
#define frac2float(fr) ((fr) / frac_1_float)
#define float2frac(fl) ((frac)(((fl) + 0.5 / frac_1_float) * frac_1_float))

/* Convert HSB to RGB. */
  STATICFUNC(void)
    color_hsb_to_rgb(floatp hue, floatp saturation, floatp brightness, float rgb[3])
  {
    if (saturation == 0) {
      rgb[0] = rgb[1] = rgb[2] = brightness;
    }
    else {                    /* Convert hsb to rgb. */
     /* We rely on the fact that the product of two */
     /* fracs fits into an unsigned long. */
      floatp h6 = hue * 6;
      ulong V = float2frac(brightness);       /* force arithmetic to long */
      frac S = float2frac(saturation);
      int I = (int)h6;
      ulong F = float2frac(h6 - I);   /* ditto */

      /* M = V*(1-S), N = V*(1-S*F), K = V*(1-S*(1-F)) = M-N+V */
      frac M = V * (frac_1_long - S) / frac_1_long;
      frac N = V * (frac_1_long - S * F / frac_1_long) / frac_1_long;
      frac K = M - N + V;
      frac R, G, B;

      switch (I) {
      default:
        R = V;
        G = K;
        B = M;
        break;
      case 1:
        R = N;
        G = V;
        B = M;
        break;
      case 2:
        R = M;
        G = V;
        B = K;
        break;
      case 3:
        R = M;
        G = N;
        B = V;
        break;
      case 4:
        R = K;
        G = M;
        B = V;
        break;
      case 5:
        R = V;
        G = M;
        B = N;
        break;
      }
      rgb[0] = frac2float(R);
      rgb[1] = frac2float(G);
      rgb[2] = frac2float(B);
#if ! defined(MIKTEX)
#ifdef DEBUG
      if (gs_debug_c('c')) {
        dlprintf7("[c]hsb(%g,%g,%g)->VSFI(%ld,%d,%ld,%d)->\n",
          hue, saturation, brightness, V, S, F, I);
        dlprintf6("   RGB(%d,%d,%d)->rgb(%g,%g,%g)\n",
          R, G, B, rgb[0], rgb[1], rgb[2]);
      }
#endif
#endif // MIKTEX
    }
  }

  //#define USE_ADOBE_CMYK_RGB

/* Convert CMYK to RGB. */
  STATICFUNC(void)
    color_cmyk_to_rgb(frac c, frac m, frac y, frac k, // const gs_imager_state * pis,
      frac rgb[3])
  {
    switch (k) {
    case frac_0:
      rgb[0] = frac_1 - c;
      rgb[1] = frac_1 - m;
      rgb[2] = frac_1 - y;
      break;
    case frac_1:
      rgb[0] = rgb[1] = rgb[2] = frac_0;
      break;
    default:
    {
#ifdef USE_ADOBE_CMYK_RGB
      /* R = 1.0 - min(1.0, C + K), etc. */
      frac not_k = frac_1 - k;

      rgb[0] = (c > not_k ? frac_0 : not_k - c);
      rgb[1] = (m > not_k ? frac_0 : not_k - m);
      rgb[2] = (y > not_k ? frac_0 : not_k - y);
#else
      /* R = (1.0 - C) * (1.0 - K), etc. */
      ulong not_k = frac_1 - k;

      /* Compute not_k * (frac_1 - v) / frac_1 efficiently. */
      ulong prod;

#define deduct_black(v)\
  (prod = (frac_1 - (v)) * not_k, frac_1_quo(prod))
      rgb[0] = deduct_black(c);
      rgb[1] = deduct_black(m);
      rgb[2] = deduct_black(y);
#undef deduct_black
#endif
    }
    }
#if ! defined(MIKTEX)
    if_debug7('c', "[c]CMYK 0x%x,0x%x,0x%x,0x%x -> RGB 0x%x,0x%x,0x%x\n",
      c, m, y, k, rgb[0], rgb[1], rgb[2]);
#endif // MIKTEX
  }
#pragma warning (pop)
} // namespace GhostscriptApi

using namespace GhostscriptApi;

RgbColor::operator unsigned long()
{
  unsigned char r = static_cast<unsigned char>(red * 255.0);
  unsigned char g = static_cast<unsigned char>(green * 255.0);
  unsigned char b = static_cast<unsigned char>(blue * 255.0);
  return RGB(r, g, b);
}

CmykColor::operator unsigned long()
{
  GhostscriptApi::frac rgb[3];
  GhostscriptApi::color_cmyk_to_rgb(float2frac(cyan),
    float2frac(magenta),
    float2frac(yellow),
    float2frac(black),
    rgb);
  RgbColor rgbcol;
  rgbcol.red = frac2float(rgb[0]);
  rgbcol.green = frac2float(rgb[1]);
  rgbcol.blue = frac2float(rgb[2]);
  return rgbcol;
}

HsbColor::operator unsigned long()
{
  float rgb[3];
  GhostscriptApi::color_hsb_to_rgb(hue, saturation, brightness, rgb);
  RgbColor rgbcol;
  rgbcol.red = rgb[0];
  rgbcol.green = rgb[1];
  rgbcol.blue = rgb[2];
  return rgbcol;
}

bool DviImpl::ParseColorSpec(const char * lpsz, unsigned long & rgb)
{
  while (isspace(*lpsz))
  {
    ++lpsz;
  }

  bool isRgb = false;
  bool isCmyk = false;
  bool isHsb = false;
  bool isGray = false;

  if (strncmp(lpsz, "rgb", 3) == 0)
  {
    isRgb = true;
    lpsz += 3;
  }
  else if (strncmp(lpsz, "hsb", 3) == 0)
  {
    isHsb = true;
    lpsz += 3;
  }
  else if (strncmp(lpsz, "cmyk", 4) == 0)
  {
    isCmyk = true;
    lpsz += 4;
  }
  else if (strncmp(lpsz, "gray", 4) == 0)
  {
    isGray = true;
    lpsz += 4;
  }

  if (isRgb || isHsb || isCmyk || isGray)
  {
    while (isspace(*lpsz))
    {
      ++lpsz;
    }

    if (*lpsz == 0)
    {
      return false;
    }

    if (isRgb || isHsb)
    {
      float frac1, frac2, frac3;
      if (sscanf_s(lpsz, "%f %f %f", &frac1, &frac2, &frac3) != 3
        || frac1 < 0.0 || frac1 > 1.0
        || frac2 < 0.0 || frac2 > 1.0
        || frac3 < 0.0 || frac3 > 1.0)
      {
        trace_error->WriteFormattedLine("libdvi", T_("invalid color triple: %s"), lpsz);
        return false;
      }
      if (isRgb)
      {
        RgbColor rgbcolor;
        rgbcolor.red = frac1;
        rgbcolor.green = frac2;
        rgbcolor.blue = frac3;
        rgb = rgbcolor;
      }
      else
      {
        HsbColor hsbcolor;
        hsbcolor.hue = frac1;
        hsbcolor.saturation = frac2;
        hsbcolor.brightness = frac3;
        rgb = hsbcolor;
      }
    }
    else if (isGray)
    {
      float frac1;
      if (sscanf_s(lpsz, "%f", &frac1) != 1
        || frac1 < 0.0 || frac1 > 1.0)
      {
        trace_error->WriteFormattedLine("libdvi", T_("invalid gray value: %s"), lpsz);
        return false;
      }
      CmykColor cmykcolor;
      cmykcolor.cyan = 0;
      cmykcolor.magenta = 0;
      cmykcolor.yellow = 0;
      cmykcolor.black = 1.0 - frac1;
      rgb = cmykcolor;
    }
    else if (isCmyk)
    {
      float frac1, frac2, frac3, frac4;
      if ((sscanf_s(lpsz, "%f %f %f %f", &frac1, &frac2, &frac3, &frac4) != 4)
        || frac1 < 0.0 || frac1 > 1.0
        || frac2 < 0.0 || frac2 > 1.0
        || frac3 < 0.0 || frac3 > 1.0
        || frac4 < 0.0 || frac4 > 1.0)

      {
        trace_error->WriteFormattedLine("libdvi", T_("invalid cmyk quadrupel: %s"), lpsz);
        return false;
      }
      CmykColor cmykcolor;
      cmykcolor.cyan = frac1;
      cmykcolor.magenta = frac2;
      cmykcolor.yellow = frac3;
      cmykcolor.black = frac4;
      rgb = cmykcolor;
    }
  }
  else
  {
    if (!isalpha(*lpsz))
    {
      trace_error->WriteFormattedLine("libdvi", T_("invalid color name: %s"), lpsz);
      return false;
    }
    string name;
    while (isalpha(*lpsz))
    {
      name += *lpsz++;
    }
    CmykColor cmykcolor;
    if (!LookupColorName(name.c_str(), cmykcolor))
    {
      trace_error->WriteFormattedLine("libdvi", T_("unknown color name: %s"), name.c_str());
      return false;
    }
    rgb = cmykcolor;
  }

  return true;
}

bool DviImpl::SetCurrentColor(const char * lpszColor)
{
  MIKTEX_ASSERT(strncmp(lpszColor, "color", 5) == 0);
  const char * lpsz = lpszColor + 5;

  while (isspace(*lpsz))
  {
    ++lpsz;
  }

  if (*lpsz == 0)
  {
    return false;
  }

  bool ret;

  if (strncmp(lpsz, "push", 4) == 0)
  {
    unsigned long rgb;
    ret = ParseColorSpec(lpsz + 4, rgb);
    if (ret)
    {
      PushColor(rgb);
    }
  }
  else if (strncmp(lpsz, "pop", 3) == 0)
  {
    ret = true;
    PopColor();
  }
  else
  {
    unsigned long rgb;
    ret = ParseColorSpec(lpsz, rgb);
    if (ret)
    {
      ResetCurrentColor();
      PushColor(rgb);
    }
  }

  return ret;
}

void DviImpl::PushColor(unsigned long rgb)
{
  trace_color->WriteFormattedLine("libdvi", T_("push color %x"), rgb);
  colorStack.push(currentColor);
  currentColor = rgb;
}

void DviImpl::PopColor()
{
  if (colorStack.empty())
  {
    trace_error->WriteLine("libdvi", T_("color pop: color stack is empty"));
    return;
  }
  currentColor = colorStack.top();
  colorStack.pop();
  trace_color->WriteFormattedLine
    ("libdvi", T_("pop color; currentcolor now %x"), currentColor);
}

void DviImpl::ResetCurrentColor()
{
  trace_color->WriteFormattedLine("libdvi", T_("reset color stack"));
  while (!colorStack.empty())
  {
    colorStack.pop();
  }
  currentColor = rgbDefaultColor;
}
