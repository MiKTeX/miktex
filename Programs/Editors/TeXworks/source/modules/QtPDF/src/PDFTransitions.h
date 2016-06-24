/**
 * Copyright (C) 2012-2013  Stefan Löffler
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
#ifndef PDFTransitions_H
#define PDFTransitions_H

#include <QImage>
#include <QTime>

namespace QtPDF {

namespace Transition {

class AbstractTransition
{
public:
  enum Type { Type_Split, Type_Blinds, Type_Box, Type_Wipe, Type_Dissolve,
              Type_Glitter, Type_Replace, Type_Fly, Type_Push, Type_Cover,
              Type_Uncover, Type_Fade };
  enum Motion { Motion_Inward, Motion_Outward };

  AbstractTransition();
  virtual ~AbstractTransition() { }

  bool isRunning() const { return (_started && !_finished); }
  bool isFinished() const { return _finished; }
  float duration() const { return _duration; }
  int direction() const { return _direction; }
  Motion motion() const { return _motion; }
  void setDuration(const float duration) { _duration = duration; }
  // for valid values, see pdf specs (use -1 for None)
  void setDirection(const int direction) { _direction = direction; }
  void setMotion(const Motion motion) { _motion = motion; }
  
  virtual void start(const QImage & imgStart, const QImage & imgEnd);
  virtual void reset() { _started = _finished = false; }
  virtual QImage getImage() = 0;
  
  static AbstractTransition * newTransition(const Type type);
  
protected:
  float getFracTime();
  virtual void setImages(const QImage & imgStart, const QImage & imgEnd);
  
  float _duration;
  int _direction;
  Motion _motion;
  bool _started;
  bool _finished;
  QTime _timer;
  QImage _imgStart;
  QImage _imgEnd;
  // TODO: /SS and /B properties
};

class AbstractInPlaceTransition : public AbstractTransition
{
public:
  AbstractInPlaceTransition() : _spread(0.05f) { }
  virtual ~AbstractInPlaceTransition() { }
  
  virtual void start(const QImage & imgStart, const QImage & imgEnd);
  virtual QImage getImage();
protected:
  virtual void initMask() = 0;
  
  QImage _mask;
  float _spread;
};

class Replace : public AbstractTransition
{
public:
  Replace() { }
  virtual QImage getImage();
};

class Split : public AbstractInPlaceTransition
{
public:
  Split() { _spread = 0; }
protected:
  virtual void initMask();
};

class Blinds : public AbstractInPlaceTransition
{
public:
  Blinds() : _numBlinds(6) { }
protected:
  virtual void initMask();
  unsigned int _numBlinds;
};

class Box : public AbstractInPlaceTransition
{
public:
  Box() { _spread = 0; }
protected:
  virtual void initMask();
};

class Wipe : public AbstractInPlaceTransition
{
public:
  Wipe() { _spread = 0; }
protected:
  virtual void initMask();
};

class Dissolve : public AbstractInPlaceTransition
{
public:
  Dissolve() { }
protected:
  virtual void initMask();
};

class Glitter : public AbstractInPlaceTransition
{
public:
  Glitter() { _spread = .1; }
protected:
  virtual void initMask();
};

class Fly : public AbstractTransition
{
public:
  Fly() { }
  virtual void start(const QImage & imgStart, const QImage & imgEnd);
  virtual QImage getImage();
protected:
  QImage _mask;
};

class Push : public AbstractTransition
{
public:
  Push() { }
  virtual QImage getImage();
};

class Cover : public AbstractTransition
{
public:
  Cover() { }
  virtual QImage getImage();
};

class Uncover : public AbstractTransition
{
public:
  Uncover() { }
  virtual QImage getImage();
};

class Fade : public AbstractTransition
{
public:
  Fade() { }
  virtual QImage getImage();
};

} // namespace(Transition)

} // namespace QtPDF

#endif // End header guard
// vim: set sw=2 ts=2 et

