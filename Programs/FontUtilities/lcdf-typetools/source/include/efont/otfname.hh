// -*- related-file-name: "../../libefont/otfname.cc" -*-
#ifndef EFONT_OTFNAME_HH
#define EFONT_OTFNAME_HH
#include <efont/otf.hh>
#include <efont/otfdata.hh>     // for ntohl()
#include <lcdf/error.hh>
namespace Efont { namespace OpenType {

class Name { public:

    Name(const String &, ErrorHandler * = 0);
    // default destructor

    bool ok() const                     { return _error >= 0; }
    int error() const                   { return _error; }

    enum NameID { N_COPYRIGHT = 0, N_FAMILY = 1, N_SUBFAMILY = 2,
                  N_UNIQUEID = 3, N_FULLNAME = 4, N_VERSION = 5,
                  N_POSTSCRIPT = 6, N_TRADEMARK = 7, N_MANUFACTURER = 8,
                  N_DESIGNER = 9, N_DESCRIPTION = 10, N_VENDOR_URL = 11,
                  N_DESIGNER_URL = 12, N_LICENSE_DESCRIPTION = 13,
                  N_LICENSE_URL = 14, N_PREF_FAMILY = 16,
                  N_PREF_SUBFAMILY = 17, N_MAC_COMPAT_FULLNAME = 18,
                  N_SAMPLE_TEXT = 19, N_POSTSCRIPT_CID = 20 };
    enum Platform { P_UNICODE = 0, P_MACINTOSH = 1, P_MICROSOFT = 3,
                    E_MS_UNICODE_BMP = 1, E_MAC_ROMAN = 0,
                    L_MS_ENGLISH_AMERICAN = 0x409 };
    enum { HEADER_SIZE = 6, NAMEREC_SIZE = 12 };

    typedef uint8_t namerecord_t[NAMEREC_SIZE];
    typedef const namerecord_t *const_iterator;

    inline static int nameid(const namerecord_t &);
    inline static int platform(const namerecord_t &);
    inline static int encoding(const namerecord_t &);
    inline static int language(const namerecord_t &);

    inline const_iterator begin() const;
    inline const_iterator end() const;
    String name(const_iterator) const;
    String utf8_name(const_iterator) const;
    String english_name(int nameid) const;

    // check version string for backwards compatibility
    bool version_chaincontext_reverse_backtrack() const;

    struct PlatformPred {
        inline PlatformPred(int nameid, int platform = -1, int encoding = -1, int language = -1);
        inline bool operator()(const namerecord_t &) const;
      private:
        int _nameid, _platform, _encoding, _language;
    };

    struct EnglishPlatformPred {
        EnglishPlatformPred(int nameid) : _nameid(nameid) { }
        inline bool operator()(const namerecord_t &) const;
      private:
        int _nameid;
    };

  private:

    String _str;
    int _error;

    int parse_header(ErrorHandler *);

};


inline int Name::nameid(const namerecord_t &nr) {
    return Data::u16_aligned(reinterpret_cast<const unsigned char*>(&nr) + 6);
}

inline int Name::platform(const namerecord_t &nr) {
    return Data::u16_aligned(reinterpret_cast<const unsigned char*>(&nr));
}

inline int Name::encoding(const namerecord_t &nr) {
    return Data::u16_aligned(reinterpret_cast<const unsigned char*>(&nr) + 2);
}

inline int Name::language(const namerecord_t &nr) {
    return Data::u16_aligned(reinterpret_cast<const unsigned char*>(&nr) + 4);
}

inline Name::const_iterator Name::begin() const {
    return reinterpret_cast<const_iterator>(_str.udata() + HEADER_SIZE);
}

inline Name::const_iterator Name::end() const {
    if (_error >= 0) {
        int count = Data::u16_aligned(_str.udata() + 2);
        return reinterpret_cast<const_iterator>(_str.udata() + HEADER_SIZE + NAMEREC_SIZE * count);
    } else
        return reinterpret_cast<const_iterator>(_str.udata() + HEADER_SIZE);
}


inline Name::PlatformPred::PlatformPred(int nid, int p, int e, int l)
    : _nameid(nid), _platform(p), _encoding(e), _language(l) {
}

inline bool Name::PlatformPred::operator()(const namerecord_t &i) const {
    return (_nameid == nameid(i))
        && (_platform < 0 || _platform == platform(i))
        && (_encoding < 0 || _encoding == encoding(i))
        && (_language < 0 || _language == language(i));
}

inline bool Name::EnglishPlatformPred::operator()(const namerecord_t &i) const {
    if (_nameid == nameid(i)) {
        int p = platform(i), e = encoding(i), l = language(i);
        return (p == P_MACINTOSH && e == E_MAC_ROMAN && l == 0)
            || (p == P_MICROSOFT && e == E_MS_UNICODE_BMP && l == L_MS_ENGLISH_AMERICAN);
    } else
        return false;
}

} // namespace Efont::OpenType
} // namespace Efont
#endif
