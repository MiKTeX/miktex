// -*- related-file-name: "../../libefont/t1rw.cc" -*-
#ifndef EFONT_T1RW_HH
#define EFONT_T1RW_HH
#include <stdio.h>
#include <lcdf/string.hh>
#include <lcdf/straccum.hh>
namespace Efont {

class Type1Reader { public:

    Type1Reader();
    virtual ~Type1Reader();

    int get_data(unsigned char *, int);
    virtual int more_data(unsigned char *, int) = 0;
    virtual bool preserve_whitespace() const { return false; }

    void switch_eexec(bool, unsigned char *, int);
    virtual void set_charstring_definer(PermString);

    bool next_line(StringAccum &);
    bool was_charstring() const         { return _charstring_len > 0; }
    int charstring_start() const        { return _charstring_start; }
    int charstring_length() const       { return _charstring_len; }

  private:

    enum { DATA_SIZE = 1024 };

    unsigned char *_data;
    int _len;
    int _pos;

    PermString _charstring_definer;
    int _charstring_start;
    int _charstring_len;

    int _ungot;
    int _crlf;

    bool _eexec;
    bool _binary_eexec;
    int _r;

    Type1Reader(const Type1Reader &);
    Type1Reader &operator=(const Type1Reader &);

    int more_data();

    inline int eexec(int);
    int ascii_eexec_get();
    inline int get_base();
    inline int get();

    void start_eexec(int ascii_chars);

    bool test_charstring(StringAccum &);

    static unsigned char xvalue_store[];
    static unsigned char *xvalue;
    static void static_initialize();

};


class Type1PFAReader : public Type1Reader {

    FILE *_f;

  public:

    Type1PFAReader(FILE *);

    int more_data(unsigned char *, int);

};

class Type1PFBReader : public Type1Reader {

    FILE *_f;
    bool _binary;
    int _left;

  public:

    Type1PFBReader(FILE *);

    int more_data(unsigned char *, int);
    bool preserve_whitespace() const;

};

class Type1SubsetReader : public Type1Reader {

    Type1Reader *_reader;
    int _left;

  public:

    Type1SubsetReader(Type1Reader *, int);

    int more_data(unsigned char *, int);
    bool preserve_whitespace() const;

};


/*****
 * Writers
 **/

class Type1Writer { public:

    Type1Writer();
    virtual ~Type1Writer();

    bool eexecing() const                               { return _eexec; }

    inline void print(int);
    void print(const char *, int);

    inline Type1Writer &operator<<(char);
    inline Type1Writer &operator<<(unsigned char);
    Type1Writer &operator<<(int);
    Type1Writer &operator<<(double);

    virtual void flush();
    virtual void switch_eexec(bool);
    virtual void print0(const unsigned char *, int) = 0;

    PermString charstring_start() const         { return _charstring_start; }
    int lenIV() const                           { return _lenIV; }
    void set_charstring_start(PermString p)     { _charstring_start = p; }
    void set_lenIV(int l)                       { _lenIV = l; }

  private:

    enum { BufSize = 1024 };

    unsigned char *_buf;
    int _pos;

    bool _eexec;
    int _eexec_start;
    int _eexec_end;
    int _r;

    PermString _charstring_start;
    int _lenIV;

    void local_flush();
    inline unsigned char eexec(int);

    Type1Writer(const Type1Writer &);
    Type1Writer &operator=(const Type1Writer &);

};


class Type1PFAWriter: public Type1Writer {

    FILE *_f;
    int _hex_line;

  public:

    Type1PFAWriter(FILE *);
    ~Type1PFAWriter();

    void switch_eexec(bool);
    void print0(const unsigned char *, int);

};


class Type1PFBWriter: public Type1Writer {

    StringAccum _save;
    FILE *_f;
    bool _binary;

  public:

    Type1PFBWriter(FILE *);
    ~Type1PFBWriter();

    void flush();
    void switch_eexec(bool);
    void print0(const unsigned char *, int);

};


inline void Type1Writer::print(int c)
{
    if (_pos >= BufSize)
        local_flush();
    _buf[_pos++] = c;
}

inline Type1Writer &operator<<(Type1Writer &w, const char *cc)
{
    w.print(cc, strlen(cc));
    return w;
}

inline Type1Writer &operator<<(Type1Writer &w, PermString p)
{
    w.print(p.c_str(), p.length());
    return w;
}

inline Type1Writer &operator<<(Type1Writer &w, const String &s)
{
    w.print(s.data(), s.length());
    return w;
}

inline Type1Writer &Type1Writer::operator<<(char c)
{
    print((unsigned char)c);
    return *this;
}

inline Type1Writer &Type1Writer::operator<<(unsigned char c)
{
    print(c);
    return *this;
}

}
#endif
