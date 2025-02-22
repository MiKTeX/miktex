#pragma once
#include <algorithm>
#include <codecvt>
#include <functional>
#include <iterator>
#include <locale>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <LibLsp/lsp/AbsolutePath.h>

#include "lsPosition.h"

namespace lsp
{

// Returns true if |value| starts/ends with |start| or |ending|.
bool StartsWith(std::string value, std::string start);
bool EndsWith(std::string value, std::string ending);
bool AnyStartsWith(std::vector<std::string> const& values, std::string const& start);
bool StartsWithAny(std::string const& value, std::vector<std::string> const& startings);
bool EndsWithAny(std::string const& value, std::vector<std::string> const& endings);
bool FindAnyPartial(std::string const& value, std::vector<std::string> const& values);
// Returns the dirname of |path|, i.e. "foo/bar.cc" => "foo/", "foo" => "./",
// "/foo" => "/". The result always ends in '/'.
std::string GetDirName(std::string path);
// Returns the basename of |path|, ie, "foo/bar.cc" => "bar.cc".
std::string GetBaseName(std::string const& path);
// Returns |path| without the filetype, ie, "foo/bar.cc" => "foo/bar".
std::string StripFileType(std::string const& path);

std::string ReplaceAll(std::string const& source, std::string const& from, std::string const& to);

std::vector<std::string> SplitString(std::string const& str, std::string const& delimiter);

template<typename TValues, typename TMap>
std::string StringJoinMap(TValues const& values, TMap const& map, std::string const& sep = ", ")
{
    std::string result;
    bool first = true;
    for (auto& entry : values)
    {
        if (!first)
        {
            result += sep;
        }
        first = false;
        result += map(entry);
    }
    return result;
}

template<typename TValues>
std::string StringJoin(TValues const& values, std::string const& sep = ", ")
{
    return StringJoinMap(values, [](std::string const& entry) { return entry; }, sep);
}

template<typename TCollection, typename TValue>
bool ContainsValue(TCollection const& collection, TValue const& value)
{
    return std::find(std::begin(collection), std::end(collection), value) != std::end(collection);
}

// Ensures that |path| ends in a slash.
void EnsureEndsInSlash(std::string& path);

// Converts a file path to one that can be used as filename.
// e.g. foo/bar.c => foo_bar.c
std::string EscapeFileName(std::string path);

// FIXME: Move ReadContent into ICacheManager?
bool FileExists(std::string const& filename);
optional<std::string> ReadContent(AbsolutePath const& filename);
std::vector<std::string> ReadLinesWithEnding(AbsolutePath const& filename);

bool WriteToFile(std::string const& filename, std::string const& content);

template<typename T, typename Fn>
void RemoveIf(std::vector<T>* vec, Fn predicate)
{
    vec->erase(std::remove_if(vec->begin(), vec->end(), predicate), vec->end());
}

std::string FormatMicroseconds(long long microseconds);

// Makes sure all newlines in |output| are in \r\n format.
std::string UpdateToRnNewlines(std::string output);

// Utility methods to check if |path| is absolute.
bool IsAbsolutePath(std::string const& path);
bool IsUnixAbsolutePath(std::string const& path);
bool IsWindowsAbsolutePath(std::string const& path);

bool IsDirectory(std::string const& path);

// string <-> wstring conversion (UTF-16), e.g. for use with Window's wide APIs.
std::string ws2s(std::wstring const& wstr);
std::wstring s2ws(std::string const& str);

AbsolutePath NormalizePath(std::string const& path, bool ensure_exists = true, bool force_lower_on_windows = true);

int GetOffsetForPosition(lsPosition position, std::string const& content);

// Finds the position for an |offset| in |content|.
lsPosition GetPositionForOffset(int offset, std::string const& content);

// Utility method to find a position for the given character.
lsPosition CharPos(std::string const& search, char character, int character_offset = 0);

void scanDirsNoRecursive(std::wstring const& rootPath, std::vector<std::wstring>& ret);

void scanFilesUseRecursive(std::wstring const& rootPath, std::vector<std::wstring>& ret, std::wstring strSuf = L"");

void scanFileNamesUseRecursive(std::wstring const& rootPath, std::vector<std::wstring>& ret, std::wstring strSuf = L"");
void scanFileNamesUseRecursive(std::string const& rootPath, std::vector<std::string>& ret, std::string strSuf = "");

void scanFilesUseRecursive(std::string const& rootPath, std::vector<std::string>& ret, std::string strSuf = "");

void scanDirsUseRecursive(std::wstring const& rootPath, std::vector<std::wstring>& ret);

} // namespace lsp
