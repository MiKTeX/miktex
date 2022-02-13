#pragma once
#include "LibLsp/JsonRpc/serializer.h"
#include <string>
struct lsDocumentUri {
	static lsDocumentUri FromPath(const AbsolutePath& path);

	lsDocumentUri();
	
	lsDocumentUri(const AbsolutePath& path);
	lsDocumentUri(const lsDocumentUri& other);;
	bool operator==(const lsDocumentUri& other) const;
	bool operator==(const std::string& other) const;
	void SetPath(const AbsolutePath& path);
	std::string GetRawPath() const;
	AbsolutePath GetAbsolutePath() const;

	
	std::string raw_uri_;
	void swap(lsDocumentUri& arg) noexcept
	{
		raw_uri_.swap(arg.raw_uri_);
	}
};
extern void Reflect(Writer& visitor, lsDocumentUri& value);
extern void Reflect(Reader& visitor, lsDocumentUri& value);
extern   std::string  make_file_scheme_uri(const std::string& absolute_path);
