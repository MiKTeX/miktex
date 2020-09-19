// -*- related-file-name: "../../libefont/t1mm.cc" -*-
#ifndef EFONT_T1MM_HH
#define EFONT_T1MM_HH
#include <efont/t1cs.hh>
class ErrorHandler;
namespace Efont {
class Type1Font;

class MultipleMasterSpace : public CharstringProgram { public:

    MultipleMasterSpace(PermString, int naxes, int nmasters);
    // default destructor

    typedef Vector<double> NumVector;

    PermString font_name() const        { return _font_name; }
    int naxes() const                   { return _naxes; }
    int nmasters() const                { return _nmasters; }

    int axis(PermString) const;
    double axis_low(int) const;
    double axis_high(int) const;

    PermString axis_type(int a) const   { return _axis_types[a]; }
    PermString axis_label(int a) const  { return _axis_labels[a]; }
    inline PermString axis_abbreviation(int a) const;
    static PermString axis_abbreviation(PermString);

    const Type1Charstring &ndv() const  { return _ndv; }
    const Type1Charstring &cdv() const  { return _cdv; }

    void set_master_positions(const Vector<NumVector> &);
    void set_normalize(const Vector<NumVector> &, const Vector<NumVector> &);
    void set_axis_type(int, PermString);
    void set_axis_label(int, PermString);
    void set_ndv(const Type1Charstring &cs) { _ndv = cs; }
    void set_cdv(const Type1Charstring &cs) { _cdv = cs; }
    void set_design_vector(const NumVector &);
    void set_weight_vector(const NumVector &);

    bool check(ErrorHandler * = 0);
    bool check_intermediate(ErrorHandler * = 0);

    NumVector empty_design_vector() const;
    inline const NumVector &default_design_vector() const;
    bool set_design(NumVector &, int, double, ErrorHandler * = 0) const;
    bool set_design(NumVector &, PermString, double, ErrorHandler * = 0) const;

    inline const NumVector &default_weight_vector() const;

    bool design_to_norm_design(const NumVector &, NumVector &, ErrorHandler * = 0) const;
    bool design_to_weight(const NumVector &, NumVector &, ErrorHandler * = 0) const;

    NumVector *mm_vector(VectorType, bool writable) const;

  private:

    mutable bool _ok;

    PermString _font_name;
    int _naxes;
    int _nmasters;

    Vector<NumVector> _master_positions;
    Vector<NumVector> _normalize_in;
    Vector<NumVector> _normalize_out;

    Vector<PermString> _axis_types;
    Vector<PermString> _axis_labels;

    Type1Charstring _ndv;
    Type1Charstring _cdv;

    NumVector _default_design_vector;
    NumVector _default_weight_vector;

    mutable NumVector *_design_vector;
    mutable NumVector *_norm_design_vector;
    mutable NumVector *_weight_vector;

    bool error(ErrorHandler *, const char *, ...) const;

    MultipleMasterSpace(const MultipleMasterSpace &);
    MultipleMasterSpace &operator=(const MultipleMasterSpace &);

    bool normalize_vector(ErrorHandler *) const;
    bool convert_vector(ErrorHandler *) const;

};


inline const Vector<double> &MultipleMasterSpace::default_design_vector() const
{
    return _default_design_vector;
}

inline const Vector<double> &MultipleMasterSpace::default_weight_vector() const
{
    return _default_weight_vector;
}

inline PermString MultipleMasterSpace::axis_abbreviation(int a) const
{
    return axis_abbreviation(_axis_types[a]);
}

}
#endif
