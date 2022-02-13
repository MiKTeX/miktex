#pragma once
#include <string>

struct AbsolutePath;

struct Directory {
	explicit Directory(const AbsolutePath& path);

	bool operator==(const Directory& rhs) const;
	bool operator!=(const Directory& rhs) const;

	std::string path;
};
