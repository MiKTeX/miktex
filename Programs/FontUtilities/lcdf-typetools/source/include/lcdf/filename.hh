// -*- related-file-name: "../../liblcdf/filename.cc" -*-
#ifndef LCDF_FILENAME_HH
#define LCDF_FILENAME_HH
#include <lcdf/string.hh>
#include <stdio.h>

class Filename { public:

    Filename() : _dir("."), _actual(0) { }
    Filename(const String &);
    Filename(const String &dir, const String &name);
    Filename(FILE *, const String &fake_name);

    bool fake() const			{ return _actual != 0; }

    const String &directory() const	{ return _dir; }
    const String &name() const		{ return _name; }
    const String &path() const		{ return _path; }
    String base() const;
    String extension() const;

    operator bool() const		{ return _name; }
    bool operator!() const		{ return !_name; }

    FILE *open_read(bool binary = false) const;
    bool readable() const;

    FILE *open_write(bool binary = false) const;

    Filename from_directory(const String &n) const { return Filename(_dir, n);}

  private:

    mutable String _dir;	// mutable for c_str()
    mutable String _name;
    mutable String _path;
    FILE *_actual;

};

#endif
