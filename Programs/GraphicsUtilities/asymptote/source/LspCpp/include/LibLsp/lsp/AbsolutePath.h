#pragma once

#include <LibLsp/JsonRpc/serializer.h>
#include <string>

struct AbsolutePath {
	static AbsolutePath BuildDoNotUse(const std::string& path);

	// Try not to use this.
	AbsolutePath();

	// Provide implicit conversions to std::string for the time being.
	AbsolutePath(const std::string& path, bool validate = true);
	operator std::string() const;

	bool operator==(const AbsolutePath& rhs) const;
	bool operator!=(const AbsolutePath& rhs) const;
	bool operator<(const AbsolutePath& rhs) const;
	bool operator>(const AbsolutePath& rhs) const;
	std::string path;
	bool qualify = true;
};


void Reflect(Reader& visitor, AbsolutePath& value);
void Reflect(Writer& visitor, AbsolutePath& value);

