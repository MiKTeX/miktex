#pragma once
#include <algorithm>
#include <codecvt>
#include <functional>
#include <iterator>
#include <locale>
#include <memory>
#include <string>
#include <vector>
#include <boost/optional/optional.hpp>
#include <LibLsp/lsp/AbsolutePath.h>

#include "lsPosition.h"


namespace lsp
{
	

// Returns true if |value| starts/ends with |start| or |ending|.
bool StartsWith(std::string value, std::string start);
bool EndsWith(std::string value, std::string ending);
bool AnyStartsWith(const std::vector<std::string>& values,
                   const std::string& start);
bool StartsWithAny(const std::string& value,
                   const std::vector<std::string>& startings);
bool EndsWithAny(const std::string& value,
                 const std::vector<std::string>& endings);
bool FindAnyPartial(const std::string& value,
                    const std::vector<std::string>& values);
// Returns the dirname of |path|, i.e. "foo/bar.cc" => "foo/", "foo" => "./",
// "/foo" => "/". The result always ends in '/'.
std::string GetDirName(std::string path);
// Returns the basename of |path|, ie, "foo/bar.cc" => "bar.cc".
std::string GetBaseName(const std::string& path);
// Returns |path| without the filetype, ie, "foo/bar.cc" => "foo/bar".
std::string StripFileType(const std::string& path);

std::string ReplaceAll(const std::string& source,
                       const std::string& from,
                       const std::string& to);

std::vector<std::string> SplitString(const std::string& str,
                                     const std::string& delimiter);

template <typename TValues, typename TMap>
std::string StringJoinMap(const TValues& values,
                          const TMap& map,
                          const std::string& sep = ", ") {
  std::string result;
  bool first = true;
  for (auto& entry : values) {
    if (!first)
      result += sep;
    first = false;
    result += map(entry);
  }
  return result;
}

template <typename TValues>
std::string StringJoin(const TValues& values, const std::string& sep = ", ") {
  return StringJoinMap(values, [](const std::string& entry) { return entry; },
                       sep);
}

template <typename TCollection, typename TValue>
bool ContainsValue(const TCollection& collection, const TValue& value) {
  return std::find(std::begin(collection), std::end(collection), value) !=
         std::end(collection);
}


// Ensures that |path| ends in a slash.
void EnsureEndsInSlash(std::string& path);

// Converts a file path to one that can be used as filename.
// e.g. foo/bar.c => foo_bar.c
std::string EscapeFileName(std::string path);

// FIXME: Move ReadContent into ICacheManager?
bool FileExists(const std::string& filename);
boost::optional<std::string> ReadContent(const AbsolutePath& filename);
std::vector<std::string> ReadLinesWithEnding(const AbsolutePath& filename);

bool WriteToFile(const std::string& filename, const std::string& content);


template <typename T, typename Fn>
void RemoveIf(std::vector<T>* vec, Fn predicate) {
  vec->erase(std::remove_if(vec->begin(), vec->end(), predicate), vec->end());
}

std::string FormatMicroseconds(long long microseconds);

// Makes sure all newlines in |output| are in \r\n format.
std::string UpdateToRnNewlines(std::string output);

// Utility methods to check if |path| is absolute.
bool IsAbsolutePath(const std::string& path);
bool IsUnixAbsolutePath(const std::string& path);
bool IsWindowsAbsolutePath(const std::string& path);

bool IsDirectory(const std::string& path);

// string <-> wstring conversion (UTF-16), e.g. for use with Window's wide APIs.
 std::string ws2s(std::wstring const& wstr);
 std::wstring s2ws(std::string const& str);

AbsolutePath NormalizePath(const std::string& path,
                            bool ensure_exists = true,
                            bool force_lower_on_windows = true);


int GetOffsetForPosition(lsPosition position, const std::string& content);

// Finds the position for an |offset| in |content|.
lsPosition GetPositionForOffset(int offset, const std::string& content);

// Utility method to find a position for the given character.
lsPosition CharPos(const std::string& search,
    char character,
    int character_offset = 0);


 void scanDirsNoRecursive(const std::wstring& rootPath, std::vector<std::wstring>& ret);

 void scanFilesUseRecursive(const std::wstring& rootPath, std::vector<std::wstring>& ret,
    std::wstring strSuf = L"");

 void scanFileNamesUseRecursive(const std::wstring& rootPath, std::vector<std::wstring>& ret,
    std::wstring strSuf = L"");
 void scanFileNamesUseRecursive(const std::string& rootPath, std::vector<std::string>& ret,
    std::string strSuf = "");

 void scanFilesUseRecursive(const std::string& rootPath, std::vector<std::string>& ret,
    std::string strSuf = "");

 void scanDirsUseRecursive(const std::wstring& rootPath, std::vector<std::wstring>& ret);

}
