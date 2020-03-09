/**
 * Copyright (C) 2013-2019  Stefan Löffler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */
#include <PDFTransitions.h>

// DEBUG
#include <QDebug>
#include <QPixmap>
// DEBUG

#include <cstdlib>
#include <ctime>

namespace QtPDF {

namespace Transition {

void AbstractTransition::start(const QImage & imgStart, const QImage & imgEnd)
{
  setImages(imgStart, imgEnd);
  _started = true;
  _finished = false;
  _timer.start();
}

void AbstractTransition::setImages(const QImage & imgStart, const QImage & imgEnd)
{
  // Set up our internal copies of the page images
  // Because imgStart and imgSize might have different sizes (unlikely, but
  // theoretically possible), we may need to pad them to ensure that our
  // internal copies _imgStart and _imgEnd have the same size.
  // NOTE: Don't use QRect::center() here to align the respective centers as
  // round-off errors can introduce a shift of +-1px.
  QSize size = imgStart.size().expandedTo(imgEnd.size());

  if (imgStart.size() == size)
    _imgStart = imgStart;
  else {
    QRect r(QPoint((imgStart.width() - size.width()) / 2, (imgStart.height() - size.height()) / 2), size);
    _imgStart = imgStart.copy(r);
  }
  if (imgEnd.size() == size)
    _imgEnd = imgEnd;
  else {
    QRect r(QPoint((imgEnd.width() - size.width()) / 2, (imgEnd.height() - size.height()) / 2), size);
    _imgEnd = imgEnd.copy(r);
  }
}

double AbstractTransition::getFracTime()
{
  double t;
  if (!_started)
    return 0.0;
  if (_finished)
    return 1.0;
  t = static_cast<double>(_timer.elapsed()) / 1000.0 / _duration;
  if (t >= 1.0)
    _finished = true;
  if (t < 0.0)
    t = 0.0;
  else if (t > 1.0)
    t = 1.0;
  return t;
}

//static
AbstractTransition * AbstractTransition::newTransition(const Type type)
{
  switch (type) {
  case Type_Split:
    return new Split();
  case Type_Blinds:
  case Type_Box:
  case Type_Wipe:
  case Type_Dissolve:
  case Type_Glitter:
    break;
  case Type_Replace:
    return new Replace();
  case Type_Fly:
  case Type_Push:
  case Type_Cover:
  case Type_Uncover:
  case Type_Fade:
    break;
  }
  return nullptr;
}

void AbstractInPlaceTransition::start(const QImage & imgStart, const QImage & imgEnd)
{
  setImages(imgStart, imgEnd);
  // NOTE: the mask must be initialized in start() for two reasons: (i) only
  // here all properties must be set properly; (ii) if the mask is based on
  // some random data set, it must be recreated each time the transition is
  // started to avoid repetitive animations.
  initMask();
  _started = true;
  _finished = false;
  _timer.start();
}

QImage AbstractInPlaceTransition::getImage()
{
  Q_ASSERT(_imgStart.size() == _imgEnd.size() && _imgStart.size() == _mask.size());
  Q_ASSERT(_imgStart.format() == QImage::Format_ARGB32);
  Q_ASSERT(_imgEnd.format() == QImage::Format_ARGB32);
  Q_ASSERT(_mask.format() == QImage::Format_Indexed8);

  QImage retVal(_imgStart.size(), QImage::Format_ARGB32);
  
  // map: 0 -> -_spread, 1 -> 1+_spread
  // this ensures that even with a contrast spread, 0 corresponds to img1, and
  // 1 corresponds to img2
  double t = (1 + 2 * _spread) * getFracTime() - _spread;
  int i, j;
  
  // Contrast mapping. Every pixel <= c1 corresponds entirely to img1, every
  // pixel >= c2 corresponds entirely to img2, and everything in-between is
  // interpolated linearly
  int c1 = static_cast<int>(255 * (t - _spread));
  int c2 = static_cast<int>(255 * (t + _spread));
  float f;

  // NOTE: Using bits() instead of scanLine() here led to some unpredictable
  // crashes on Linux/Ubuntu when using zoom (probably due to some data
  // alignment issues).
  for (j = 0; j < _mask.height(); ++j) {
    const uchar * img1 = _imgStart.constScanLine(j);
    const uchar * img2 = _imgEnd.constScanLine(j);
    const uchar * mask = _mask.constScanLine(j);
    uchar * img = retVal.scanLine(j);
    for (i = 0; i < _mask.width(); ++i) {
      if (mask[i] <= c1)
        f = 1.0f;
      else if (mask[i] >= c2)
        f = 0.0f;
      else
        // c1 != c2 is guaranteed here; if c1 == c2, then c2 <= mask[i] <= c1
        // reduces to c1 <= mask[i] <= c1 and always holds.
        f = static_cast<float>(c2 - mask[i]) / static_cast<float>(c2 - c1);
      img[4 * i + 0] = static_cast<uchar>(img1[4 * i + 0] * (1.0f - f) + img2[4 * i + 0] * f);
      img[4 * i + 1] = static_cast<uchar>(img1[4 * i + 1] * (1.0f - f) + img2[4 * i + 1] * f);
      img[4 * i + 2] = static_cast<uchar>(img1[4 * i + 2] * (1.0f - f) + img2[4 * i + 2] * f);
      img[4 * i + 3] = static_cast<uchar>(img1[4 * i + 3] * (1.0f - f) + img2[4 * i + 3] * f);
    }
  }
  
  return retVal;
}

QImage Replace::getImage()
{
  if (!_started)
    return _imgStart;
  _finished = true;
  return _imgEnd;
}

void Split::initMask()
{
  _mask = QImage(_imgStart.size(), QImage::Format_Indexed8);
  int i, j;
  
  switch (_motion) {
  case Motion_Inward:
    if (_direction == 0) {
      for (j = 0; j < _mask.height() / 2; ++j) {
        uchar * data = _mask.scanLine(j);
        for (i = 0; i < _mask.width(); ++i)
          data[i] = static_cast<uchar>((2 * 255 * j) / (_mask.height() - 2));
      }
      for (; j < _mask.height(); ++j) {
        uchar * data = _mask.scanLine(j);
        for (i = 0; i < _mask.width(); ++i)
          data[i] = static_cast<uchar>((2 * 255 * (_mask.height() - 1 - j)) / (_mask.height() - 2));
      }
    }
    else if (_direction == 90) {
      for (j = 0; j < _mask.height(); ++j) {
        uchar * data = _mask.scanLine(j);
        for (i = 0; i < _mask.width() / 2; ++i)
          data[i] = static_cast<uchar>((2 * 255 * i) / (_mask.width() - 2));
        for (; i < _mask.width(); ++i)
          data[i] = static_cast<uchar>((2 * 255 * (_mask.width() - 1 - i)) / (_mask.width() - 2));
      }
    }
    break;
  case Motion_Outward:
    if (_direction == 0) {
      for (j = 0; j < _mask.height() / 2; ++j) {
        uchar * data = _mask.scanLine(j);
        for (i = 0; i < _mask.width(); ++i)
          data[i] = static_cast<uchar>((2 * 255 * (_mask.height() / 2 - 1 - j)) / (_mask.height() - 2));
      }
      for (; j < _mask.height(); ++j) {
        uchar * data = _mask.scanLine(j);
        for (i = 0; i < _mask.width(); ++i)
          data[i] = static_cast<uchar>((2 * 255 * (j - _mask.height() / 2)) / (_mask.height() - 2));
      }
    }
    else if (_direction == 90) {
      for (j = 0; j < _mask.height(); ++j) {
        uchar * data = _mask.scanLine(j);
        for (i = 0; i < _mask.width() / 2; ++i)
          data[i] = static_cast<uchar>((2 * 255 * (_mask.width() / 2 - 1 - i)) / (_mask.width() - 2));
        for (; i < _mask.width(); ++i)
          data[i] = static_cast<uchar>((2 * 255 * (i - _mask.width() / 2)) / (_mask.width() - 2));
      }
    }
    break;
  }
}

void Blinds::initMask()
{
  _mask = QImage(_imgStart.size(), QImage::Format_Indexed8);
  int i, j;
  if (_direction == 0) {
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        data[i] = static_cast<uchar>((255 * static_cast<int>(_numBlinds) * j / (_mask.height() - 1)) % 256);
      }
    }
  }
  else if (_direction == 90) {
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        data[i] = static_cast<uchar>((255 * static_cast<int>(_numBlinds) * i / (_mask.width()) - 1) % 256);
      }
    }
  }
}

void Box::initMask()
{
  _mask = QImage(_imgStart.size(), QImage::Format_Indexed8);
  int i, j, x, y, d;

  // Here, we use the fact that the set of all points in a 2D space with a
  // constant manhattan distance from a center point is a square that is rotated
  // by 45° (i.e., each corner of the square lies on one of the axis).
  // Hence, we use the diagonals as (non-orthogonal) axis and compute the
  // manhattan distance from the center in that coordinate system.
  int max = _mask.width() * _mask.height();

  switch (_motion) {
  case Motion_Inward:
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        x = _mask.width() * _mask.height() - _mask.height() * i - _mask.width() * j;
        y = _mask.height() * i - _mask.width() * j;
        d = qAbs(x) + qAbs(y);
        data[i] = static_cast<uchar>(255 * (max - d) / max);
      }
    }
    break;
  case Motion_Outward:
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        x = _mask.width() * _mask.height() - _mask.height() * i - _mask.width() * j;
        y = _mask.height() * i - _mask.width() * j;
        d = qAbs(x) + qAbs(y);
        data[i] = static_cast<uchar>(255 * d / max);
      }
    }
    break;
  }
}

void Wipe::initMask()
{
  _mask = QImage(_imgStart.size(), QImage::Format_Indexed8);
  int i, j;
  
  if (_direction == 0) {
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        data[i] = static_cast<uchar>(255 * i / (_mask.width() - 1));
      }
    }
  }
  else if (_direction == 180) {
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        data[i] = static_cast<uchar>(255 - 255 * i / (_mask.width() - 1));
      }
    }
  }
  else if (_direction == 270) {
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        data[i] = static_cast<uchar>(255 * j / (_mask.height() - 1));
      }
    }
  }
  else if (_direction == 90) {
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        data[i] = static_cast<uchar>(255 - 255 * j / (_mask.height() - 1));
      }
    }
  }
}

void Dissolve::initMask()
{
  _mask = QImage(_imgStart.size(), QImage::Format_Indexed8);
  int i, j;
  
  srand(static_cast<unsigned int>(time(nullptr)));
  for (j = 0; j < _mask.height(); ++j) {
    uchar * data = _mask.scanLine(j);
    for (i = 0; i < _mask.width(); ++i) {
      data[i] = static_cast<uchar>(rand() % 256);
    }
  }
}

void Glitter::initMask()
{
  _mask = QImage(_imgStart.size(), QImage::Format_Indexed8);
  int i, j;
  int randomRange = static_cast<int>(255 * _spread);

  srand(static_cast<unsigned int>(time(nullptr)));
  
  if (_direction == 0) {
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        data[i] = static_cast<uchar>(rand() % randomRange + i * (256 - randomRange) / (_mask.width() - 1));
      }
    }
  }
  else if (_direction == 270) {
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        data[i] = static_cast<uchar>(rand() % randomRange + j * (256 - randomRange) / (_mask.height() - 1));
      }
    }
  }
  else if (_direction == 315) {
    for (j = 0; j < _mask.height(); ++j) {
      uchar * data = _mask.scanLine(j);
      for (i = 0; i < _mask.width(); ++i) {
        data[i] = static_cast<uchar>(rand() % randomRange + (i + j) * (256 - randomRange) / (_mask.width() + _mask.height() - 2));
      }
    }
  }
}

void Fly::start(const QImage & imgStart, const QImage & imgEnd)
{
  int i, j;

  setImages(imgStart, imgEnd);

  // Using QImage::Format_Mono would be more efficient, but would complicate the
  // code (introducing lots of bit operations and alignment checks)
  _mask = QImage(_imgStart.size(), QImage::Format_Indexed8);
  for (j = 0; j < _mask.height(); ++j) {
    const QRgb * img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j));
    const QRgb * img2 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j));
    uchar * mask = _mask.scanLine(j);
    for (i = 0; i < _mask.width(); ++i)
      mask[i] = (img1[i] == img2[i] ? 0 : 255);
  }
  
  _started = true;
  _finished = false;
  _timer.start();
}

QImage Fly::getImage()
{
  Q_ASSERT(_imgStart.size() == _imgEnd.size());
  Q_ASSERT(_imgStart.format() == QImage::Format_ARGB32);
  Q_ASSERT(_imgEnd.format() == QImage::Format_ARGB32);

  QImage retVal = QImage(_imgEnd.size(), QImage::Format_ARGB32);
  int i, j;
  int offset;
  const QRgb * img1, * img2;
  const uchar * mask;
  QRgb * img;
  
  switch (_motion) {
  case Motion_Inward:
    if (_direction == 0) {
      offset = static_cast<int>(getFracTime() * static_cast<float>(_imgEnd.width()));
      for (j = 0; j < _imgEnd.height(); ++j) {
        img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j));
        img2 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j));
        mask = _mask.constScanLine(j);
        img = reinterpret_cast<QRgb*>(retVal.scanLine(j));
        for (i = 0; i < offset; ++i)
          img[i] = (mask[i + _imgEnd.width() - offset] == 0 ? img1[i] : img2[i + _imgEnd.width() - offset]);
        for (; i < _imgEnd.width(); ++i)
          img[i] = img1[i];
      }
    }
    else if (_direction == 270) {
      offset = static_cast<int>(getFracTime() * static_cast<float>(_imgEnd.height()));
      for (j = 0; j < _imgEnd.height(); ++j) {
        img = reinterpret_cast<QRgb*>(retVal.scanLine(j));
        if (j < offset) {
          img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j));
          img2 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j + _imgEnd.height() - offset));
          mask = _mask.constScanLine(j + _imgEnd.height() - offset);
          for (i = 0; i < _imgEnd.width(); ++i)
            img[i] = (mask[i] == 0 ? img1[i] : img2[i] );
        }
        else {
          img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j));
          for (i = 0; i < _imgEnd.width(); ++i)
            img[i] = img1[i];
        }
      }
    }
    break;
  case Motion_Outward:
    if (_direction == 0) {
      offset = static_cast<int>(getFracTime() * static_cast<float>(_imgEnd.width()));
      for (j = 0; j < _imgEnd.height(); ++j) {
        img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j));
        img2 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j));
        mask = _mask.constScanLine(j);
        img = reinterpret_cast<QRgb*>(retVal.scanLine(j));
        for (i = 0; i < offset; ++i)
          img[i] = img2[i];
        for (; i < _imgEnd.width(); ++i)
          img[i] = (mask[i - offset] == 0 ? img2[i] : img1[i - offset]);
      }
    }
    else if (_direction == 270) {
      offset = static_cast<int>(getFracTime() * static_cast<float>(_imgEnd.height()));
      for (j = 0; j < _imgEnd.height(); ++j) {
        img = reinterpret_cast<QRgb*>(retVal.scanLine(j));
        if (j < offset) {
          img2 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j));
          for (i = 0; i < _imgEnd.width(); ++i)
            img[i] = img2[i];
        }
        else {
          img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j - offset));
          img2 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j));
          mask = _mask.constScanLine(j - offset);
          for (i = 0; i < _imgEnd.width(); ++i)
            img[i] = (mask[i] == 0 ? img2[i] : img1[i] );
        }
      }
    }
    break;
  }
  return retVal;
}


QImage Push::getImage()
{
  Q_ASSERT(_imgStart.size() == _imgEnd.size());
  Q_ASSERT(_imgStart.format() == QImage::Format_ARGB32);
  Q_ASSERT(_imgEnd.format() == QImage::Format_ARGB32);

  QImage retVal = QImage(_imgEnd.size(), QImage::Format_ARGB32);
  int i, j;
  int edge;
  const QRgb * img1, * img2;
  
  if (_direction == 0) {
    edge = static_cast<int>(getFracTime() * static_cast<float>(_imgEnd.width()));
    for (j = 0; j < _imgEnd.height(); ++j) {
      img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j));
      img2 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j));
      QRgb * img = reinterpret_cast<QRgb*>(retVal.scanLine(j));
      for (i = 0; i < edge; ++i)
        img[i] = img2[i + _imgEnd.width() - edge];
      for (; i < _imgEnd.width(); ++i)
        img[i] = img1[i - edge];
    }
  }
  else if (_direction == 270) {
    edge = static_cast<int>(getFracTime() * static_cast<float>(_imgEnd.height()));
    for (j = 0; j < _imgEnd.height(); ++j) {
      if (j < edge)
        img1 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j + _imgEnd.height() - edge));
      else
        img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j - edge));
      QRgb * img = reinterpret_cast<QRgb*>(retVal.scanLine(j));

      for (i = 0; i < retVal.width(); ++i)
        img[i] = img1[i];
    }
  }
  return retVal;
}

QImage Cover::getImage()
{
  Q_ASSERT(_imgStart.size() == _imgEnd.size());
  Q_ASSERT(_imgStart.format() == QImage::Format_ARGB32);
  Q_ASSERT(_imgEnd.format() == QImage::Format_ARGB32);

  QImage retVal = QImage(_imgEnd.size(), QImage::Format_ARGB32);
  int i, j;
  int edge;
  const QRgb * img1, * img2;
  
  if (_direction == 0) {
    edge = static_cast<int>(getFracTime() * static_cast<float>(_imgEnd.width()));
    for (j = 0; j < _imgEnd.height(); ++j) {
      img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j));
      img2 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j));
      QRgb * img = reinterpret_cast<QRgb*>(retVal.scanLine(j));
      for (i = 0; i < edge; ++i)
        img[i] = img2[i + _imgEnd.width() - edge];
      for (; i < _imgEnd.width(); ++i)
        img[i] = img1[i];
    }
  }
  else if (_direction == 270) {
    edge = static_cast<int>(getFracTime() * static_cast<float>(_imgEnd.height()));
    for (j = 0; j < _imgEnd.height(); ++j) {
      if (j < edge)
        img1 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j + _imgEnd.height() - edge));
      else
        img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j));
      QRgb * img = reinterpret_cast<QRgb*>(retVal.scanLine(j));

      for (i = 0; i < retVal.width(); ++i)
        img[i] = img1[i];
    }
  }
  return retVal;
}

QImage Uncover::getImage()
{
  Q_ASSERT(_imgStart.size() == _imgEnd.size());
  Q_ASSERT(_imgStart.format() == QImage::Format_ARGB32);
  Q_ASSERT(_imgEnd.format() == QImage::Format_ARGB32);

  QImage retVal = QImage(_imgEnd.size(), QImage::Format_ARGB32);
  int i, j;
  int edge;
  const QRgb * img1, * img2;
  
  if (_direction == 0) {
    edge = static_cast<int>(getFracTime() * static_cast<float>(_imgEnd.width()));
    for (j = 0; j < _imgEnd.height(); ++j) {
      img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j));
      img2 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j));
      QRgb * img = reinterpret_cast<QRgb*>(retVal.scanLine(j));
      for (i = 0; i < edge; ++i)
        img[i] = img2[i];
      for (; i < _imgEnd.width(); ++i)
        img[i] = img1[i - edge];
    }
  }
  else if (_direction == 270) {
    edge = static_cast<int>(getFracTime() * static_cast<float>(_imgEnd.height()));
    for (j = 0; j < _imgEnd.height(); ++j) {
      if (j < edge)
        img1 = reinterpret_cast<const QRgb*>(_imgEnd.constScanLine(j));
      else
        img1 = reinterpret_cast<const QRgb*>(_imgStart.constScanLine(j - edge));
      QRgb * img = reinterpret_cast<QRgb*>(retVal.scanLine(j));

      for (i = 0; i < retVal.width(); ++i)
        img[i] = img1[i];
    }
  }
  return retVal;
}

QImage Fade::getImage()
{
  Q_ASSERT(_imgStart.size() == _imgEnd.size());
  Q_ASSERT(_imgStart.format() == QImage::Format_ARGB32);
  Q_ASSERT(_imgEnd.format() == QImage::Format_ARGB32);

  QImage retVal = QImage(_imgEnd.size(), QImage::Format_ARGB32);
  int i;
  int f = static_cast<int>(255 * getFracTime());

  const uchar * img1 = _imgStart.constBits();
  const uchar * img2 = _imgEnd.constBits();
  uchar * img = retVal.bits();

  for (i = 0; i < retVal.byteCount(); ++i) {
    img[i] = static_cast<uchar>(((255 - f) * img1[i] + f * img2[i]) / 255);
  }
  return retVal;
}

} // namespace Transition

} // namespace QtPDF

// vim: set sw=2 ts=2 et

