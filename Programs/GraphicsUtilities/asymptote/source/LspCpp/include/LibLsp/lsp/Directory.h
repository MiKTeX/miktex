#pragma once
#include <string>

struct AbsolutePath;

struct Directory
{
    explicit Directory(AbsolutePath const& path);

    bool operator==(Directory const& rhs) const;
    bool operator!=(Directory const& rhs) const;

    std::string path;
};
