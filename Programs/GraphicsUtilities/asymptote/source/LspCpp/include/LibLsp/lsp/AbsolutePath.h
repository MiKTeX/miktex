#pragma once

#include <LibLsp/JsonRpc/serializer.h>
#include <string>

struct AbsolutePath
{
    static AbsolutePath BuildDoNotUse(std::string const& path);

    // Try not to use this.
    AbsolutePath();

    // Provide implicit conversions to std::string for the time being.
    AbsolutePath(std::string const& path, bool validate = true);
    operator std::string() const;

    bool operator==(AbsolutePath const& rhs) const;
    bool operator!=(AbsolutePath const& rhs) const;
    bool operator<(AbsolutePath const& rhs) const;
    bool operator>(AbsolutePath const& rhs) const;
    std::string path;
    bool qualify = true;
};

void Reflect(Reader& visitor, AbsolutePath& value);
void Reflect(Writer& visitor, AbsolutePath& value);
