// -*- related-file-name: "../../liblcdf/landmark.cc" -*-
#ifndef LCDF_LANDMARK_HH
#define LCDF_LANDMARK_HH
#include <lcdf/string.hh>

class Landmark { public:

    Landmark()				: _file(), _line(~0U) { }
    explicit Landmark(const String &f)	: _file(f), _line(~0U) { }
    Landmark(const String &f, unsigned l) : _file(f), _line(l) { }

    operator bool() const		{ return _file; }
    bool operator!() const		{ return !_file; }
    bool has_line() const		{ return _line != ~0U; }

    const String &file() const		{ return _file; }
    unsigned line() const		{ return _line; }

    Landmark next_line() const;
    Landmark whole_file() const		{ return Landmark(_file); }

    operator String() const;

  private:

    String _file;
    unsigned _line;

};

Landmark operator+(const Landmark &, int);

inline Landmark
Landmark::next_line() const
{
    return *this + 1;
}

#endif
