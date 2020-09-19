// -*- related-file-name: "../../libefont/psres.cc" -*-
#ifndef EFONT_PSRES_HH
#define EFONT_PSRES_HH
#include <lcdf/vector.hh>
#include <lcdf/hashmap.hh>
#include <lcdf/string.hh>
#include <lcdf/filename.hh>
class Slurper;
namespace Efont {
class PsresDatabaseSection;

class PsresDatabase { public:

    PsresDatabase();
    ~PsresDatabase();

    void add_psres_path(const char* path, const char* defaults, bool override);
    bool add_psres_file(Filename&, bool override);
    void add_database(PsresDatabase*, bool override);

    inline PsresDatabaseSection* section(PermString section) const;
    const String& value(PermString section, PermString key) const;
    const String& unescaped_value(PermString section, PermString key) const;
    Filename filename_value(PermString section, PermString key) const;

  private:

    HashMap<PermString, int> _section_map;
    Vector<PsresDatabaseSection*> _sections;

    PsresDatabaseSection* force_section(PermString);
    bool add_one_psres_file(Slurper&, bool override);
    void add_psres_directory(PermString);

};

class PsresDatabaseSection { public:

    PsresDatabaseSection(PermString);

    PermString section_name() const             { return _section_name; }

    void add_psres_file_section(Slurper&, PermString, bool);
    void add_section(PsresDatabaseSection*, bool override);

    const String& value(PermString key)         { return value(_map[key]); }
    inline const String& unescaped_value(PermString key) const;
    Filename filename_value(PermString key);

  private:

    PermString _section_name;
    HashMap<PermString, int> _map;
    Vector<PermString> _directories;
    Vector<String> _values;
    Vector<int> _value_escaped;

    const String& value(int index);

};

inline PsresDatabaseSection* PsresDatabase::section(PermString n) const
{
    return _sections[_section_map[n]];
}

inline const String& PsresDatabaseSection::unescaped_value(PermString key) const
{
    assert(!_value_escaped[_map[key]]);
    return _values[_map[key]];
}

}
#endif
