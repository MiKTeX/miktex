// -*- related-file-name: "../../libefont/t1bounds.cc" -*-
#ifndef EFONT_T1BOUNDS_HH
#define EFONT_T1BOUNDS_HH
#include <efont/t1interp.hh>
#include <lcdf/transform.hh>
namespace Efont {

class CharstringBounds : public CharstringInterp { public:

    CharstringBounds();
    CharstringBounds(const Transform&);
    CharstringBounds(const Transform&, const Vector<double>& weight_vec);
    ~CharstringBounds()                         { }

    const Point& width() const                  { return _width; }
    double x_width() const                      { return _width.x; }
    bool bb_known() const                       { return KNOWN(_lb.x); }
    double bb_left() const                      { return _lb.x; }
    double bb_top() const                       { return _rt.y; }
    double bb_right() const                     { return _rt.x; }
    double bb_bottom() const                    { return _lb.y; }
    const Point& bb_bottom_left() const         { return _lb; }
    const Point& bb_top_right() const           { return _rt; }

    void act_width(int, const Point&);
    void act_line(int, const Point&, const Point&);
    void act_curve(int, const Point&, const Point&, const Point&, const Point&);
    inline void mark(const Point&);

    void clear();
    bool char_bounds(const CharstringContext&, bool shift = true);
    void translate(double dx, double dy);
    inline Point transform(const Point& p) const;

    // output: [left, bottom, right, top]
    bool output(double bb[4], double& width, bool use_cur_width = false) const;
    static bool bounds(const CharstringContext&,
                       double bounds[4], double& width);
    static bool bounds(const Transform&, const CharstringContext&,
                       double bounds[4], double& width);

  private:

    Point _lb;
    Point _rt;
    Point _width;
    Transform _xf;
    Transform _nonfont_xf;
    const CharstringProgram* _last_xf_program;

    void set_xf(const CharstringProgram*);

    inline void xf_mark(const Point&);
    void xf_mark(const Bezier&);

    inline bool xf_inside(const Point&) const;
    inline bool xf_controls_inside(const Bezier&) const;

};

inline void CharstringBounds::xf_mark(const Point& p)
{
    if (!KNOWN(_lb.x))
        _lb = _rt = p;
    if (p.x < _lb.x)
        _lb.x = p.x;
    else if (p.x > _rt.x)
        _rt.x = p.x;
    if (p.y < _lb.y)
        _lb.y = p.y;
    else if (p.y > _rt.y)
        _rt.y = p.y;
}

inline void CharstringBounds::mark(const Point& p)
{
    xf_mark(p * _xf);
}

inline bool CharstringBounds::xf_inside(const Point& p) const
{
    return p.x >= _lb.x && p.x <= _rt.x && p.y >= _lb.y && p.y <= _rt.y;
}

inline bool CharstringBounds::xf_controls_inside(const Bezier& b) const
{
    return xf_inside(b.point(1)) && xf_inside(b.point(2));
}

inline Point CharstringBounds::transform(const Point& p) const
{
    return p * _xf;
}

}
#endif
