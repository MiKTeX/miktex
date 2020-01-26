// -*- related-file-name: "../../libefont/afmparse.cc" -*-
#ifndef EFONT_AFMPARSE_HH
#define EFONT_AFMPARSE_HH
#include <lcdf/slurper.hh>
#include <lcdf/permstr.hh>
#include <stdarg.h>
namespace Efont {

class AfmParser { public:

    AfmParser(Slurper &);

    bool ok() const                     { return _slurper.ok(); }
    operator Landmark() const           { return _slurper.landmark(); }
    Landmark landmark() const           { return _slurper.landmark(); }
    unsigned lineno() const             { return _slurper.lineno(); }
    const Filename &filename() const    { return _slurper.filename(); }

    bool key_matched() const            { return _fail_field >= 0; }
    int fail_field() const              { return _fail_field; }
    PermString message() const          { return _message; }
    void clear_message()                { _message = PermString(); }

    PermString keyword() const;
    bool is(const char *, ...);
    bool isall(const char *, ...);

    inline bool next_line();
    void save_line()                    { _slurper.save_line(); }
    void skip_until(unsigned char);

    unsigned char *cur_line() const     { return _pos; }
    unsigned char first() const         { return _pos[0]; }
    unsigned char operator[](int i) const { return _pos[i]; }
    bool left() const                   { return *_pos != 0; }

  private:

    Slurper &_slurper;
    bool _save_line;

    unsigned char *_line;
    unsigned char *_pos;
    int _length;

    PermString _message;
    int _fail_field;

    void static_initialize();
    void trim_end();
    unsigned char *vis(const char *, va_list);

};


inline bool AfmParser::next_line()
{
    _pos = _line = (unsigned char *)_slurper.next_line();
    _length = _slurper.cur_line_length();
    return _line != 0;
}

}
#endif
