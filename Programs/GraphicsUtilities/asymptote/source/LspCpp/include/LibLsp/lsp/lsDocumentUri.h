#pragma once
#include "LibLsp/JsonRpc/serializer.h"
#include <string>
struct lsDocumentUri
{
    static lsDocumentUri FromPath(AbsolutePath const& path);

    lsDocumentUri();

    lsDocumentUri(AbsolutePath const& path);
    lsDocumentUri(lsDocumentUri const& other);
    ;
    bool operator==(lsDocumentUri const& other) const;
    bool operator==(std::string const& other) const;
    void SetPath(AbsolutePath const& path);
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
extern std::string make_file_scheme_uri(std::string const& absolute_path);
