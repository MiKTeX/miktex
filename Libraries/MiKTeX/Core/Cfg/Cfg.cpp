/* cfg.cpp: configuration files

   Copyright (C) 1996-2018 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <fstream>

#include "miktex/Core/FileStream.h"
#include <miktex/Trace/StopWatch>
#include <miktex/Trace/Trace>
#include <miktex/Util/Tokenizer>

#include "internal.h"

#include "miktex/Core/Cfg.h"
#include "miktex/Core/Registry.h"

#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;

#define FATAL_CFG_ERROR(message) \
  MIKTEX_FATAL_ERROR_2(T_("A MiKTeX configuration file could not be loaded."), "file", currentFile.ToString(), "line", std::to_string(lineno), "error", message)

constexpr const char COMMENT_CHAR = ';';
constexpr const char* COMMENT1 = ";";
constexpr const char* COMMENT2 = ";;";
constexpr const char* COMMENT3 = ";;;";
constexpr const char* COMMENT4 = ";;;;";

MIKTEXSTATICFUNC(bool) EndsWith(const string& s, const string& suffix)
{
  return s.length() >= suffix.length() &&
    s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
}

MIKTEXSTATICFUNC(string&) Trim(string& str)
{
  constexpr const char* WHITESPACE = " \t\r\n";
  size_t pos = str.find_last_not_of(WHITESPACE);
  if (pos != string::npos)
  {
    str.erase(pos + 1);
  }
  pos = str.find_first_not_of(WHITESPACE);
  if (pos == string::npos)
  {
    str.erase();
  }
  else if (pos != 0)
  {
    str.erase(0, pos);
  }
  return str;
}

class CfgValue :
  public Cfg::Value
{
public:
  string name;

public:
  string lookupName;

public:
  vector<string> value;

public:
  string documentation;

public:
  bool commentedOut = false;

public:
  CfgValue()
  {
  }

public:
  CfgValue(const string& name, const string& lookupName, const string& value, const string& documentation, bool isCommentedOut) :
    name(name), lookupName(lookupName), value{ value }, documentation(documentation), commentedOut(isCommentedOut)
  {
  }

public:
  string MIKTEXTHISCALL GetName() const override
  {
    return name;
  }

public:
  bool IsMultiValue() const
  {
    return EndsWith(name, "[]");
  }

public:
  string MIKTEXTHISCALL AsString() const override
  {
    if (IsMultiValue())
    {
      return StringUtil::Flatten(value, PathName::PathNameDelimiter);
    }
    else
    {
      return value.empty() ? "" : value.front();
    }
  }

public:
  vector<string> MIKTEXTHISCALL AsStringVector() const override
  {
    if (IsMultiValue())
    {
      return value;
    }
    else if (value.empty())
    {
      return vector<string>();
    }
    else
    {
      return StringUtil::Split(value.front(), PathName::PathNameDelimiter);
    }
  }

public:
  string MIKTEXTHISCALL GetDocumentation() const override
  {
    return documentation;
  }

public:
  bool IsCommentedOut() const override
  {
    return commentedOut;
  }
};

inline bool operator<(const CfgValue& lhs, const CfgValue& rhs)
{
  return lhs.lookupName < rhs.lookupName;
}

typedef unordered_map<string, shared_ptr<CfgValue>> ValueMap;

class CfgKey :
  public Cfg::Key
{
public:
  ValueMap valueMap;

public:
  CfgKey()
  {
  }

public:
  CfgKey(const string& name, const string& lookupName) :
    name(name),
    lookupName(lookupName)
  {
  }

private:
  string name;

public:
  string lookupName;

public:
  string MIKTEXTHISCALL GetName() const override
  {
    return name;
  }

public:
  shared_ptr<Cfg::Value> MIKTEXTHISCALL GetValue(const string& valueName) const override
  {
    ValueMap::const_iterator it = valueMap.find(Utils::MakeLower(valueName));
    if (it == valueMap.end())
    {
      return nullptr;
    }
    return it->second;
  }

public:
  vector<CfgValue> GetCfgValues(bool sorted) const
  {
    vector<CfgValue> values;
    values.reserve(valueMap.size());
    for (const auto& p : valueMap)
    {
      values.push_back(*p.second);
    }
    if (sorted)
    {
      sort(values.begin(), values.end());
    }
    return values;
  }

public:
  void WriteValues(ostream& stream) const;

public:
  Cfg::ValueIterator begin() override;

public:
  Cfg::ValueIterator end() override;
};

inline bool operator<(const CfgKey& lhs, const CfgKey& rhs)
{
  return lhs.lookupName < rhs.lookupName;
}

static const char* const knownSearchPathValues[] = {
  "path",
  "extensions",
  MIKTEX_REGVAL_COMMON_ROOTS,
  MIKTEX_REGVAL_USER_ROOTS,
};

bool IsSearchPathValue(const string& valueName)
{
  for (const char* path : knownSearchPathValues)
  {
    if (Utils::EqualsIgnoreCase(valueName, path))
    {
      return true;
    }
  }
  return false;
}

void CfgKey::WriteValues(ostream& stream) const
{
  bool isKeyWritten = false;
  for (const CfgValue& v : GetCfgValues(true))
  {
    if (!isKeyWritten)
    {
      stream
        << "\n"
        << "[" << name << "]" << "\n";
      isKeyWritten = true;
    }
    if (!v.documentation.empty())
    {
      stream << "\n";
      bool start = true;
      for (const char& ch : v.documentation)
      {
        if (start)
        {
          stream << COMMENT2 << " ";
        }
        stream << ch;
        start = (ch == '\n');
      }
      if (!start)
      {
        stream << "\n";
      }
    }
    if (v.value.empty())
    {
      stream << (v.commentedOut ? COMMENT1 : "") << v.name << "=" << "\n";
    }
    else if (v.IsMultiValue())
    {
      for (const string& val : v.value)
      {
        stream << (v.commentedOut ? COMMENT1 : "") << v.name << "=" << val << "\n";
      }
    }
    else if (IsSearchPathValue(v.name) && v.value.front().find_first_of(PathName::PathNameDelimiter) != string::npos)
    {
      stream << (v.commentedOut ? COMMENT1 : "") << v.name << "=" << "\n";
      for (const string& root: StringUtil::Split(v.value.front(), PathName::PathNameDelimiter))
      {
        stream << (v.commentedOut ? COMMENT1 : "") << v.name << ";=" << root << "\n";
      }
    }
    else
    {
      stream << (v.commentedOut ? COMMENT1 : "") << v.name << "=" << v.value.front() << "\n";
    }
  }
}

typedef unordered_map<string, shared_ptr<CfgKey>> KeyMap;

class WalkCallback
{
public:
  virtual ~WalkCallback() {}
  virtual void addData(const string& data) = 0;
};

class MD5WalkCallback :
  public WalkCallback
{
public:
  void addData(const string& data) override
  {
    md5Builder.Update(data.c_str(), data.length());
  }

public:
  MD5 GetFinalMD5()
  {
    return md5Builder.Final();
  }
  
private:
  MD5Builder md5Builder;
};

#if defined(ENABLE_OPENSSL)
class OpenSSLWalkCallback :
  public WalkCallback
{
public:
  OpenSSLWalkCallback(EVP_PKEY* pkey, bool verify) :
    mdctx(EVP_MD_CTX_create(),
#if defined(LIBRESSL_VERSION_NUMBER) || OPENSSL_VERSION_NUMBER < 0x10100000L
          EVP_MD_CTX_destroy
#else
          EVP_MD_CTX_free
#endif
          )
  {
    this->isVerifying = verify;
    this->pkey = pkey;
    if (mdctx == nullptr)
    {
      FatalOpenSSLError();
    }
    const EVP_MD* md = EVP_get_digestbyname("SHA256");
    if (md == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    if (isVerifying)
    {
      if (EVP_DigestVerifyInit(mdctx.get(), nullptr, md, nullptr, pkey) != 1)
      {
        FatalOpenSSLError();
      }
    }
    else
    {
      if (EVP_DigestSignInit(mdctx.get(), nullptr, md, nullptr, pkey) != 1)
      {
        FatalOpenSSLError();
      }
    }
  }

public:
  void addData(const string& data) override
  {
    if (isVerifying)
    {
      if (EVP_DigestVerifyUpdate(mdctx.get(), data.c_str(), data.length()) != 1)
      {
        FatalOpenSSLError();
      }
    }
    else
    {
      if (EVP_DigestSignUpdate(mdctx.get(), data.c_str(), data.length()) != 1)
      {
        FatalOpenSSLError();
      }
    }
  }

public:
  bool Verify(const vector<unsigned char>& sig)
  {
    if (!isVerifying)
    {
      MIKTEX_UNEXPECTED();
    }
    vector<unsigned char> modifiableSig = sig;
    bool ok = EVP_DigestVerifyFinal(mdctx.get(), &modifiableSig[0], modifiableSig.size()) == 1;
    if (!ok)
    {
#if 0
      FatalOpenSSLError();
#endif
    }
    return ok;
  }

public:
  vector<unsigned char> Sign()
  {
    if (isVerifying)
    {
      MIKTEX_UNEXPECTED();
    }
    vector<unsigned char> sig;
    size_t sigLen = 0;
    if (EVP_DigestSignFinal(mdctx.get(), nullptr, &sigLen) != 1)
    {
      FatalOpenSSLError();
    }
    sig.resize(sigLen);
    if (EVP_DigestSignFinal(mdctx.get(), &sig[0], &sigLen) != 1)
    {
      FatalOpenSSLError();
    }
    return sig;
  }
  
private:
  bool isVerifying;

private:
  EVP_MD_CTX_ptr mdctx;

private:
  EVP_PKEY* pkey;
};
#endif

class CfgImpl :
  public Cfg
{
public:
  Options GetOptions() const override
  {
    return options;
  }

public:
  void SetOptions(Options options) override
  {
    this->options = options;
  }

public:
  bool MIKTEXTHISCALL Empty() const override;

public:
  MD5 MIKTEXTHISCALL GetDigest() const override;

public:
  shared_ptr<Value> MIKTEXTHISCALL GetValue(const string& keyName, const string& valueName) const override;

public:
  bool MIKTEXTHISCALL TryGetValueAsString(const string& keyName, const string& valueName, string& value) const override;

public:
  bool MIKTEXTHISCALL TryGetValueAsStringVector(const string& keyName, const string& valueName, vector<string>& value) const override;

public:
  void MIKTEXTHISCALL SetModified(bool b) override;

public:
  bool MIKTEXTHISCALL IsModified() const override;

public:
  void MIKTEXTHISCALL PutValue(const string& keyName, const string& valueName, const string& value) override;

public:
  void MIKTEXTHISCALL PutValue(const string& keyName, const string& valueName, const string& value, const string& documentation, bool commentedOut) override;

private:
  void Read(const PathName& path, const string& defaultKeyName, int level, bool mustBeSigned, const PathName& publicKeyFile);

private:
  void Read(std::istream& reader, const string& defaultKeyName, int level, bool mustBeSigned, const PathName& publicKeyFile);

public:
  void MIKTEXTHISCALL Read(const PathName& path) override
  {
    this->path = path;
    Read(path, false);
  }

public:
  void MIKTEXTHISCALL Read(std::istream& reader) override
  {
    Read(reader, false);
  }

public:
  void MIKTEXTHISCALL Read(const PathName& path, bool mustBeSigned) override
  {
    this->path = path;
    Read(path, path.GetFileNameWithoutExtension().ToString(), 0, mustBeSigned, PathName());
  }

public:
  void MIKTEXTHISCALL Read(std::istream& reader, bool mustBeSigned) override
  {
    Read(reader, "", 0, mustBeSigned, PathName());
  }

public:
  void MIKTEXTHISCALL Read(const PathName& path, const PathName& publicKeyFile) override
  {
    this->path = path;
    Read(path, path.GetFileNameWithoutExtension().ToString(), 0, true, publicKeyFile);
  }
  
public:
  void MIKTEXTHISCALL Read(std::istream& reader, const PathName& publicKeyFile) override
  {
    Read(reader, "", 0, true, publicKeyFile);
  }

public:
  void MIKTEXTHISCALL Write(const PathName& path) override
  {
    Write(path, T_("DO NOT EDIT THIS FILE!"));
  }

public:
  void MIKTEXTHISCALL Write(const PathName& path, const string& header) override
  {
    Write(path, header, nullptr);
  }

public:
  vector<CfgKey> GetCfgKeys(bool sorted) const
  {
    vector<CfgKey> keys;
    keys.reserve(keyMap.size());
    for (const auto& p : keyMap)
    {
      keys.push_back(*p.second);
    }
    if (sorted)
    {
      sort(keys.begin(), keys.end());
    }
    return keys;
  }

public:
  virtual shared_ptr<Key> GetKey(const string& keyName) const override
  {
    KeyMap::const_iterator it = keyMap.find(Utils::MakeLower(keyName));
    if (it == keyMap.end())
    {
      return nullptr;
    }
    return it->second;
  }

public:
  KeyIterator begin() override;

public:
  KeyIterator end() override;

public:
  void MIKTEXTHISCALL DeleteKey(const string& keyName) override;

public:
  void MIKTEXTHISCALL DeleteValue(const string& keyName, const string& valueName) override;

public:
  bool MIKTEXTHISCALL IsSigned() const override
  {
    return !signature.empty();
  }

public:
  void MIKTEXTHISCALL Write(const PathName& path, const string& header, IPrivateKeyProvider* pPrivateKeyProvider) override;

private:
  enum PutMode {
    None,
    Append,
    SearchPathAppend
  };

private:
  bool ParseValueDefinition(const string& line, string& valueName, string& value, PutMode& putMode);

private:
  void Walk(WalkCallback* callback) const;

private:
  PathName path;

private:
  KeyMap keyMap;

private:
  bool tracking = false;

private:
  MD5 snapshotDigest;

private:
  string signature;

private:
  unique_ptr<TraceStream> traceStream;

private:
  unique_ptr<TraceStream> traceError;

private:
  unique_ptr<TraceStream> traceStopWatch;

public:
  CfgImpl();

public:
  virtual ~CfgImpl() override;

private:
  string GetDefaultKeyName() const;

private:
  shared_ptr<CfgKey> FindKey(const string& keyName) const;

private:
  void WriteKeys(ostream& stream);

private:
  void PutValue(const string& keyName, const string& valueName, const string& value, PutMode putMode, const string& documentation, bool commentedOut);

private:
  bool ClearValue(const string& keyName, const string& valueName);
  
private:
  Options options;

private:
  int lineno = 0;

private:
  PathName currentFile;

private:
  friend class Cfg;
};

Cfg::~Cfg() noexcept
{
}

CfgImpl::CfgImpl() :
  traceStream(TraceStream::Open(MIKTEX_TRACE_CONFIG)),
  traceError(TraceStream::Open(MIKTEX_TRACE_ERROR)),
  traceStopWatch(TraceStream::Open(MIKTEX_TRACE_STOPWATCH))
{
}

CfgImpl::~CfgImpl()
{
  try
  {
    traceError->Close();
    traceStream->Close();
    traceStopWatch->Close();
  }
  catch (const exception&)
  {
  }
}

string CfgImpl::GetDefaultKeyName() const
{
  if (path.Empty())
  {
    return "";
  }
  return path.GetFileNameWithoutExtension().ToString();
}

shared_ptr<CfgKey> CfgImpl::FindKey(const string& keyName) const
{
  KeyMap::const_iterator it = keyMap.find(Utils::MakeLower(keyName.empty() ? GetDefaultKeyName() : keyName));
  if (it == keyMap.end())
  {
    return nullptr;
  }
  return it->second;
}

void CfgImpl::WriteKeys(ostream& stream)
{
  for (const CfgKey& k : GetCfgKeys(true))
  {
    k.WriteValues(stream);
  }
  if (tracking)
  {
    snapshotDigest = GetDigest();
  }
}

void CfgImpl::DeleteKey(const string& keyName)
{
  KeyMap::iterator it = keyMap.find(Utils::MakeLower(keyName));
  if (it == keyMap.end())
  {
    INVALID_ARGUMENT("keyName", keyName);
  }
  keyMap.erase(it);
}

void CfgImpl::Walk(WalkCallback* callback) const
{
  for (const CfgKey& key : GetCfgKeys(true))
  {
    callback->addData("[");
    callback->addData(key.lookupName);
    callback->addData("]\n");
    for (const CfgValue& val : key.GetCfgValues(true))
    {
      if (val.value.empty())
      {
        callback->addData(val.lookupName);
        callback->addData("=");
        callback->addData("\n");
      }
      else if (val.IsMultiValue())
      {
        for (const string& v : val.value)
        {
          callback->addData(val.lookupName);
          callback->addData("=");
          callback->addData(v);
          callback->addData("\n");
        }
      }
      else
      {
        callback->addData(val.lookupName);
        callback->addData("=");
        callback->addData(val.value.front());
        callback->addData("\n");
      }
    }
  }
}

MD5 CfgImpl::GetDigest() const
{
  MD5WalkCallback callback;
  Walk(&callback);
  return callback.GetFinalMD5();
}

unique_ptr<Cfg> Cfg::Create()
{
  return make_unique<CfgImpl>();
}

shared_ptr<Cfg::Value> CfgImpl::GetValue(const string& keyName, const string& valueName) const
{
  shared_ptr<CfgKey> key = FindKey(keyName);
  if (key == nullptr)
  {
    return nullptr;
  }
  shared_ptr<Cfg::Value> value = key->GetValue(valueName);
  return value == nullptr || value->IsCommentedOut() ? nullptr : value;
}

bool CfgImpl::TryGetValueAsString(const string& keyName, const string& valueName, string& outValue) const
{
  shared_ptr<Value> value = GetValue(keyName, valueName);
  if (value == nullptr)
  {
    return false;
  }
  outValue = value->AsString();
  return true;
}

bool CfgImpl::TryGetValueAsStringVector(const string& keyName, const string& valueName, vector<string>& outValue) const
{
  shared_ptr<Value> value = GetValue(keyName, valueName);
  if (value == nullptr)
  {
    return false;
  }
  outValue = value->AsStringVector();
  return true;
}

void CfgImpl::PutValue(const string& keyName_, const string& valueName, const string& value, CfgImpl::PutMode putMode, const string& documentation, bool commentedOut)
{
  string keyName = keyName_.empty() ? GetDefaultKeyName() : keyName_;
  if (keyName.empty())
  {
    MIKTEX_UNEXPECTED();
  }
  string lookupKeyName = Utils::MakeLower(keyName);
  pair<KeyMap::iterator, bool> pair1 = keyMap.insert(make_pair(lookupKeyName, make_shared<CfgKey>(keyName, lookupKeyName)));

  KeyMap::iterator itKey = pair1.first;
  MIKTEX_ASSERT(itKey != keyMap.end());

  string lookupValueName = Utils::MakeLower(valueName);
  if (options[Option::NoOverwriteValues] && itKey->second->valueMap.find(lookupValueName) != itKey->second->valueMap.end())
  {
    return;
  }
  pair<ValueMap::iterator, bool> pair2 = itKey->second->valueMap.insert(make_pair(lookupValueName, make_shared<CfgValue>(valueName, lookupValueName, value, documentation, commentedOut)));

  if (!pair2.second)
  {
    // modify existing value
    ValueMap::iterator itVal = pair2.first;
    if (itVal->second->IsMultiValue() && putMode != None)
    {
      MIKTEX_UNEXPECTED();
    }
    itVal->second->documentation = documentation;
    itVal->second->commentedOut = commentedOut;
    if (putMode == Append)
    {
      if (itVal->second->value.empty())
      {
        itVal->second->value.push_back(value);
      }
      else
      {
        itVal->second->value.front() += value.front();
      }
    }
    else if (putMode == SearchPathAppend)
    {
      if (itVal->second->value.empty())
      {
        itVal->second->value.push_back(value);
      }
      else
      {
        if (!itVal->second->value.front().empty())
        {
          itVal->second->value.front() += PathName::PathNameDelimiter;
        }
        itVal->second->value.front() += value;
      }
    }
    else if (itVal->second->IsMultiValue())
    {
      itVal->second->value.push_back(value);
    }
    else
    {
      itVal->second->value.clear();
      itVal->second->value.push_back(value);
    }
  }
}

bool CfgImpl::ClearValue(const string& keyName_, const string& valueName)
{
  string keyName = keyName_.empty() ? GetDefaultKeyName() : keyName_;
  if (keyName.empty())
  {
    MIKTEX_UNEXPECTED();
  }
  string lookupKeyName = Utils::MakeLower(keyName);
  KeyMap::iterator itKey = keyMap.find(lookupKeyName);
  if (itKey == keyMap.end())
  {
    return false;
  }
  string lookupValueName = Utils::MakeLower(valueName);
  ValueMap::iterator itValue = itKey->second->valueMap.find(lookupValueName);
  if (itValue == itKey->second->valueMap.end())
  {
    return false;
  }
  itValue->second->value.clear();
  return true;
}

void CfgImpl::PutValue(const string& keyName, const string& valueName, const string& value)
{
  return PutValue(keyName, valueName, value, None, "", false);
}

void CfgImpl::PutValue(const string& keyName, const string& valueName, const string& value, const string& documentation, bool commentedOut)
{
  return PutValue(keyName, valueName, value, None, value, commentedOut);
}

void CfgImpl::Read(const PathName& path, const string& defaultKeyName, int level, bool mustBeSigned, const PathName& publicKeyFile)
{
  unique_ptr<StopWatch> stopWatch = StopWatch::Start(traceStopWatch.get(), "core", path.ToString());
  traceStream->WriteFormattedLine("core", T_("parsing: %s..."), path.GetData());
  AutoRestore<int> autoRestore1(lineno);
  AutoRestore<PathName> autoRestore(currentFile);
  std::ifstream reader = File::CreateInputStream(path);
  Read(reader, defaultKeyName, level, mustBeSigned, publicKeyFile);
  reader.close();
}

void CfgImpl::Read(std::istream& reader, const string& defaultKeyName, int level, bool mustBeSigned, const PathName& publicKeyFile)
{
  MIKTEX_ASSERT(!(level > 0 && mustBeSigned));

  if (mustBeSigned)
  {
    traceStream->WriteFormattedLine("core", T_("signature required..."));
  }

  bool wasEmpty = Empty();

  string keyName = defaultKeyName;
  string lookupKeyName = Utils::MakeLower(keyName);
  bool ignoreKey = false;

  lineno = 0;
  currentFile = path;

  string documentation;

  for (string line; std::getline(reader, line); )
  {
    ++lineno;
    Trim(line);
    if (line.empty())
    {
      documentation = "";
    }
    else if (line[0] == '!')
    {
      documentation = "";
      Tokenizer tok(line.substr(1), " \t");
      if (!tok)
      {
        FATAL_CFG_ERROR(T_("invalid cfg directive"));
      }
      if (*tok == "include")
      {
        ++tok;
        if (!tok)
        {
          FATAL_CFG_ERROR(T_("missing file name argument"));
        }
        PathName path2(path);
        path2.MakeAbsolute();
        path2.RemoveFileSpec();
        path2 /= *tok;
        Read(path2, keyName, level + 1, false, PathName());
      }
      else if (*tok == "clear")
      {
        ++tok;
        if (!tok)
        {
          FATAL_CFG_ERROR(T_("missing value name argument"));
        }
        ClearValue(keyName, *tok);
      }
      else
      {
        FATAL_CFG_ERROR(T_("unknown cfg directive"));
      }
    }
    else if (line[0] == '[')
    {
      documentation = "";
      Tokenizer tok(line.substr(1), "]");
      if (!tok)
      {
        FATAL_CFG_ERROR(T_("incomplete secion name"));
      }
      keyName = *tok;
      lookupKeyName = Utils::MakeLower(keyName);
      ignoreKey = options[Option::NoOverwriteKeys] && keyMap.find(lookupKeyName) != keyMap.end();
    }
    else if (line.length() >= 3 && line[0] == COMMENT_CHAR && line[1] == COMMENT_CHAR && line[2] == ' ')
    {
      if (!documentation.empty())
      {
        documentation += '\n';
      }
      documentation += &line[3];
    }
    else if ((line.length() >= 2 && line[0] == COMMENT_CHAR && (IsAlNum(line[1]) || line[1] == '.')) || IsAlNum(line[0]) || line[0] == '.')
    {
      if (!ignoreKey)
      {
        string valueName;
        string value;
        PutMode putMode;
        if (!ParseValueDefinition(line[0] == COMMENT_CHAR ? &line[1] : &line[0], valueName, value, putMode))
        {
          FATAL_CFG_ERROR(T_("invalid value definition"));
        }
        PutValue(keyName, valueName, value, putMode, documentation, line[0] == COMMENT_CHAR);
      }
    }
    else if (line.length() >= 4 && line[0] == COMMENT_CHAR && line[1] == COMMENT_CHAR && line[2] == COMMENT_CHAR && line[3] == COMMENT_CHAR)
    {
      documentation = "";
      Tokenizer tok(line.substr(4), " \t");
      if (tok)
      {
        if (*tok == "signature/miktex:")
        {
          ++tok;
          if (tok && wasEmpty && level == 0)
          {
            signature = *tok;
          }
        }
      }
    }
  }

  if (reader.bad() || reader.fail() && !reader.eof())
  {
    FATAL_CFG_ERROR(T_("error reading the configuration file"));
  }

  if (mustBeSigned && signature.empty())
  {
    FATAL_CFG_ERROR(T_("the configuration file is not signed"));
  }

  if (wasEmpty && level == 0 && !signature.empty())
  {
#if defined(ENABLE_OPENSSL)
    if (GetCryptoLib() == CryptoLib::OpenSSL)
    {
      BIO_ptr b64(BIO_new(BIO_f_base64()), BIO_free);
      if (b64 == nullptr)
      {
        FatalOpenSSLError();
      }
      BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
      vector<char> modifiableSignature;
      for (const char& ch : signature)
      {
        modifiableSignature.push_back(ch);
      }
      BIO_ptr mem(BIO_new_mem_buf(&modifiableSignature[0], modifiableSignature.size()), BIO_free);
      if (mem == nullptr)
      {
        FatalOpenSSLError();
      }
      BIO* bio = BIO_push(b64.get(), mem.get());
      unsigned char buf[1000];
      vector<unsigned char> sig;
      int n = 0;
      while ((n = BIO_read(bio, buf, 1000)) > 0)
      {
        sig.insert(sig.end(), buf, buf + n);
      }
      if (n < -1)
      {
        FatalOpenSSLError();
      }
      RSA_ptr rsa = LoadPublicKey_OpenSSL(publicKeyFile);
      EVP_PKEY_ptr pkey(EVP_PKEY_new(), EVP_PKEY_free);
      if (pkey == nullptr)
      {
        FatalOpenSSLError();
      }
      if (EVP_PKEY_set1_RSA(pkey.get(), rsa.get()) != 1)
      {
        FatalOpenSSLError();
      }
      OpenSSLWalkCallback callback(pkey.get(), true);
      Walk(&callback);
      if (!callback.Verify(sig))
      {
#if !defined(DISABLE_SIGNATURE_CHECK)
        FATAL_CFG_ERROR(T_("the file has been tampered with"));
#endif
      }
    }
#endif
  }
}

bool CfgImpl::ParseValueDefinition(const string& line, string& valueName, string& value, CfgImpl::PutMode& putMode)
{
  MIKTEX_ASSERT(!line.empty() && (IsAlNum(line[0]) || line[0] == '.'));

  size_t posEqual = line.find('=');

  putMode = None;

  if (posEqual == string::npos || posEqual == 0)
  {
    return false;
  }

  value = line.substr(posEqual + 1);
  Trim(value);

  if (line[posEqual - 1] == '+')
  {
    putMode = Append;
    posEqual -= 1;
  }
  else if (line[posEqual - 1] == ';')
  {
    putMode = SearchPathAppend;
    posEqual -= 1;
  }

  valueName = line.substr(0, posEqual);
  Trim(valueName);

  return true;
}

#if defined(ENABLE_OPENSSL)
extern "C" int OpenSSLPasswordCallback(char* buf, int size, int rwflag, void* userdata)
{
  IPrivateKeyProvider* privKey = (IPrivateKeyProvider*)userdata;
  string passphrase;
  if (!privKey->GetPassphrase(passphrase))
  {
    return 0;
  }
  if (passphrase.length() >= size)
  {
    MIKTEX_UNEXPECTED();
  }
  strcpy(buf, passphrase.c_str());
  return passphrase.length();
}
#endif

string ToBase64(const vector<unsigned char>& bytes)
{
#if defined(ENABLE_OPENSSL)
  BIO_ptr b64 (BIO_new(BIO_f_base64()), BIO_free);
  if (b64 == nullptr)
  {
    FatalOpenSSLError();
  }
  BIO_ptr mem (BIO_new(BIO_s_mem()), BIO_free);
  if (mem == nullptr)
  {
    FatalOpenSSLError();
  }
  BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
  BIO_push(b64.get(), mem.get());
  if (BIO_write(b64.get(), &bytes[0], bytes.size()) != bytes.size())
  {
    FatalOpenSSLError();
  }
  if (BIO_flush(b64.get()) != 1)
  {
    FatalOpenSSLError();
  }
  char buf[1024];
  int n;
  string s;
  while ((n = BIO_read(mem.get(), buf, sizeof(buf))) > 0)
  {
    s.insert(s.end(), buf, buf + n);
  }
  if (n < -1)
  {
    FatalOpenSSLError();
  }
  return s;
#else
  UNIMPLEMENTED();
#endif
}

void CfgImpl::Write(const PathName& path, const string& header, IPrivateKeyProvider* privateKeyProvider)
{
  time_t t = time(nullptr);
  ofstream stream = File::CreateOutputStream(path);
  if (!header.empty())
  {
    stream
      << COMMENT3 << " " << header << "\n"
      << "\n";
  }
  WriteKeys(stream);
  if (privateKeyProvider != nullptr)
  {
    string sig;
#if defined(ENABLE_OPENSSL)
    if (GetCryptoLib() == CryptoLib::OpenSSL)
    {
      FileStream stream(File::Open(privateKeyProvider->GetPrivateKeyFile(), FileMode::Open, FileAccess::Read));
      RSA_ptr rsa (PEM_read_RSAPrivateKey(stream.GetFile(), nullptr, OpenSSLPasswordCallback, privateKeyProvider), RSA_free);
      stream.Close();
      if (rsa == nullptr)
      {
        FatalOpenSSLError();
      }
      EVP_PKEY_ptr pkey (EVP_PKEY_new(), EVP_PKEY_free);
      if (pkey == nullptr)
      {
        FatalOpenSSLError();
      }
      if (EVP_PKEY_set1_RSA(pkey.get(), rsa.get()) != 1)
      {
        FatalOpenSSLError();
      }
      OpenSSLWalkCallback callback(pkey.get(), false);
      Walk(&callback);
      sig = ToBase64(callback.Sign());
    }
#endif
    stream
      << "\n"
      << COMMENT4 << T_("This configuration file is signed by a MiKTeX maintainer. The signature follows.") << "\n"
      << COMMENT4 << "-----BEGIN MIKTEX SIGNATURE-----" << "\n"
      << COMMENT4 << " " << "signature/miktex:" << " " << sig << "\n"
      << COMMENT4 << "-----END MIKTEX SIGNATURE-----" << "\n";
  }
  stream.close();
  File::SetTimes(path, t, t, t);
}

class Cfg::KeyIterator::impl
{
public:
  KeyMap::iterator it;
};

Cfg::KeyIterator::KeyIterator() :
  pimpl(make_unique<Cfg::KeyIterator::impl>())
{
}

Cfg::KeyIterator::KeyIterator(Cfg::KeyIterator&& other) :
  pimpl(std::move(other.pimpl))
{
}

Cfg::KeyIterator& Cfg::KeyIterator::operator=(Cfg::KeyIterator&& other)
{
  pimpl = std::move(other.pimpl);
  return *this;
}

Cfg::KeyIterator::~KeyIterator()
{
}

shared_ptr<Cfg::Key> Cfg::KeyIterator::operator*() const
{
  return pimpl->it->second;
}

Cfg::KeyIterator& Cfg::KeyIterator::operator++()
{
  ++pimpl->it;
  return *this;
}

bool Cfg::KeyIterator::operator==(const Cfg::KeyIterator& other)
{
  return pimpl->it == other.pimpl->it;
}

bool Cfg::KeyIterator::operator!=(const Cfg::KeyIterator& other)
{
  return pimpl->it != other.pimpl->it;
}

Cfg::KeyIterator CfgImpl::begin()
{
  Cfg::KeyIterator keyIterator;
  keyIterator.GetImpl().it = keyMap.begin();
  return keyIterator;
}

Cfg::KeyIterator CfgImpl::end()
{
  Cfg::KeyIterator keyIterator;
  keyIterator.GetImpl().it = keyMap.end();
  return keyIterator;
}

class Cfg::ValueIterator::impl
{
public:
  ValueMap::iterator it;
  ValueMap::iterator end;
};

Cfg::ValueIterator::ValueIterator() :
  pimpl(make_unique<Cfg::ValueIterator::impl>())
{
}

Cfg::ValueIterator::ValueIterator(Cfg::ValueIterator&& other) :
  pimpl(std::move(other.pimpl))
{
}

Cfg::ValueIterator& Cfg::ValueIterator::operator=(Cfg::ValueIterator&& other)
{
  pimpl = std::move(other.pimpl);
  return *this;
}

Cfg::ValueIterator::~ValueIterator()
{
}

shared_ptr<Cfg::Value> Cfg::ValueIterator::operator*() const
{
  return pimpl->it->second;
}

Cfg::ValueIterator& Cfg::ValueIterator::operator++()
{
  do
  {
    ++pimpl->it;
  } while (pimpl->it != pimpl->end && pimpl->it->second->IsCommentedOut());
  return *this;
}

bool Cfg::ValueIterator::operator==(const Cfg::ValueIterator& other)
{
  return pimpl->it == other.pimpl->it;
}

bool Cfg::ValueIterator::operator!=(const Cfg::ValueIterator& other)
{
  return pimpl->it != other.pimpl->it;
}

Cfg::ValueIterator CfgKey::begin()
{
  ValueMap::iterator it;
  for (it = valueMap.begin(); it != valueMap.end() && it->second->IsCommentedOut(); ++it)
  {
  }
  Cfg::ValueIterator valueIterator;
  valueIterator.GetImpl().it = it;
  valueIterator.GetImpl().end = valueMap.end();
  return valueIterator;
}

Cfg::ValueIterator CfgKey::end()
{
  Cfg::ValueIterator valueIterator;
  valueIterator.GetImpl().it = valueMap.end();
  valueIterator.GetImpl().end = valueMap.end();
  return valueIterator;
}

void CfgImpl::DeleteValue(const string& keyName, const string& valueName)
{
  KeyMap::iterator it = keyMap.find(Utils::MakeLower(keyName));
  if (it == keyMap.end())
  {
    INVALID_ARGUMENT("keyName", keyName);
  }
  ValueMap::iterator it2 = it->second->valueMap.find(Utils::MakeLower(valueName));
  if (it2 == it->second->valueMap.end())
  {
    INVALID_ARGUMENT("valueName", valueName);
  }
  it->second->valueMap.erase(it2);
}

void CfgImpl::SetModified(bool b)
{
  tracking = true;
  snapshotDigest = GetDigest();
  if (b)
  {
    snapshotDigest[0] = ~snapshotDigest[0];
  }
}

bool CfgImpl::IsModified() const
{
  MIKTEX_ASSERT(tracking);
  MD5 digest = GetDigest();
  return !(digest == snapshotDigest);
}

bool CfgImpl::Empty() const
{
  return keyMap.empty();
}
