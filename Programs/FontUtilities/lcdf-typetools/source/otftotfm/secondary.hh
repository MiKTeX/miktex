#ifndef OTFTOTFM_SECONDARY_HH
#define OTFTOTFM_SECONDARY_HH
#include <efont/otfcmap.hh>
#include <efont/cff.hh>
#include "setting.hh"
class Metrics;
class Secondary;
class Transform;
namespace Efont { class TrueTypeBoundsCharstringProgram; }

struct FontInfo {

    const Efont::OpenType::Font *otf;
    const Efont::OpenType::Cmap *cmap;

    Efont::Cff *cff_file;
    const Efont::Cff::Font *cff;

    const Efont::OpenType::Post *post;
    const Efont::OpenType::Name *name;

    FontInfo(const Efont::OpenType::Font *otf, ErrorHandler *);
    ~FontInfo();

    bool ok() const;

    int nglyphs() const                 { return _nglyphs; }
    bool glyph_names(Vector<PermString> &) const;
    int glyphid(PermString) const;
    const Efont::CharstringProgram *program() const;
    int units_per_em() const {
        return program()->units_per_em();
    }

    bool is_fixed_pitch() const;
    double italic_angle() const;
    double x_height(const Transform& font_xform) const;

    void set_is_fixed_pitch(bool is_fixed_pitch) {
        _override_is_fixed_pitch = true;
        _is_fixed_pitch = is_fixed_pitch;
    }
    void set_italic_angle(double italic_angle) {
        _override_italic_angle = true;
        _italic_angle = italic_angle;
    }
    enum { x_height_auto = 0, x_height_explicit,
           x_height_os2, x_height_x };
    void set_x_height(int source, double x_height) {
        _override_x_height = source;
        _x_height = x_height;
    }

    String family_name() const;
    String postscript_name() const;

  private:

    int _nglyphs;
    mutable Vector<PermString> _glyph_names;
    mutable bool _got_glyph_names;
    mutable Vector<uint32_t> _unicodes;
    mutable Efont::TrueTypeBoundsCharstringProgram *_ttb_program;
    bool _override_is_fixed_pitch;
    bool _override_italic_angle;
    bool _is_fixed_pitch;
    uint8_t _override_x_height;
    double _italic_angle;
    double _x_height;

};

class SettingSet {
public:
    inline SettingSet(Secondary* s, Metrics& m);
    inline SettingSet& push_back(Setting s);
    inline SettingSet& push_back(int op, int x = 0, int y = 0);
    inline SettingSet& move(int x, int y = 0);
    SettingSet& show(int uni);
    inline SettingSet& kernx(bool is_kernx);
    inline bool check();
    inline void checkpoint();
    inline Metrics& metrics() const;
    inline const Vector<Setting>& settings() const;
private:
    Secondary* s_;
    Vector<Setting> v_;
    int original_size_;
    Metrics& metrics_;
    int kern_type_;
    bool ok_;
    typedef Efont::OpenType::Glyph Glyph;
};

class Secondary { public:
    Secondary(const FontInfo& finfo)    : _finfo(finfo), _next(0) { }
    virtual ~Secondary();
    void set_next(Secondary *s)         { _next = s; }
    typedef Efont::OpenType::Glyph Glyph;
    bool encode_uni(int code, PermString name, const uint32_t* uni_first, const uint32_t* uni_last, Metrics &metrics, ErrorHandler *errh);
    inline bool encode_uni(int code, PermString name, uint32_t uni, Metrics& m, ErrorHandler* errh);
    virtual int setting(uint32_t uni, SettingSet&, ErrorHandler *);
  protected:
    const FontInfo& _finfo;

    friend class SettingSet;
  private:
    Secondary *_next;
};

class T1Secondary : public Secondary { public:
    T1Secondary(const FontInfo &, const String &font_name, const String &otf_file_name);
    int setting(uint32_t uni, SettingSet&, ErrorHandler *);
  private:
    String _font_name;
    String _otf_file_name;
    int _units_per_em;
    int _xheight;
    int _spacewidth;
    enum { J_NODOT = -1031892 /* unlikely value */ };
    int dotlessj_font(Metrics &, ErrorHandler *, Glyph &dj_glyph);
};

bool char_bounds(double bounds[4], double& width, const FontInfo &,
                 const Transform &, uint32_t uni);

double char_one_bound(const FontInfo &, const Transform &,
                      int dimen, bool max, double best, int uni, ...);

inline SettingSet::SettingSet(Secondary* s, Metrics& m)
    : s_(s), original_size_(0), metrics_(m),
      kern_type_(Setting::KERN), ok_(true) {
}

inline SettingSet& SettingSet::kernx(bool is_kernx) {
    kern_type_ = is_kernx ? Setting::KERNX : Setting::KERN;
    return *this;
}

inline SettingSet& SettingSet::push_back(Setting s) {
    if (ok_)
        v_.push_back(s);
    return *this;
}

inline SettingSet& SettingSet::push_back(int op, int x, int y) {
    return push_back(Setting(op, x, y));
}

inline SettingSet& SettingSet::move(int x, int y) {
    return push_back(Setting(Setting::MOVE, x, y));
}

inline bool SettingSet::check() {
    bool ok = ok_;
    ok_ = true;
    return ok;
}

inline void SettingSet::checkpoint() {
    assert(ok_);
    original_size_ = v_.size();
}

inline Metrics& SettingSet::metrics() const {
    return metrics_;
}

inline const Vector<Setting>& SettingSet::settings() const {
    return v_;
}

inline bool Secondary::encode_uni(int code, PermString name, uint32_t uni, Metrics& m, ErrorHandler* errh) {
    return encode_uni(code, name, &uni, &uni + 1, m, errh);
}

#endif
