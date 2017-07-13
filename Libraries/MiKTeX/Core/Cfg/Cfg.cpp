/* cfg.cpp: configuration files

   Copyright (C) 1996-2017 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/CsvList.h"
#include "miktex/Core/Cfg.h"
#include "miktex/Core/Registry.h"
#include "miktex/Core/StreamWriter.h"

#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;

#define FATAL_CFG_ERROR(message) \
  MIKTEX_FATAL_ERROR_2(T_("A MiKTeX configuration file could not be loaded."), "file", currentFile.ToString(), "line", std::to_string(lineno), "error", message)

const char COMMENT_CHAR = ';';
const char* const COMMENT_CHAR_STR = ";";

#if defined(ENABLE_BOTAN)
const char* const EMSA_ = "EMSA3(SHA-256)";
#endif

MIKTEXSTATICFUNC(bool) EndsWith(const string& s, const string& suffix)
{
  return s.length() >= suffix.length() &&
    s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
}

MIKTEXSTATICFUNC(string&) Trim(string& str)
{
  const char* whitespace = " \t\r\n";
  size_t pos = str.find_last_not_of(whitespace);
  if (pos != string::npos)
  {
    str.erase(pos + 1);
  }
  pos = str.find_first_not_of(whitespace);
  if (pos == string::npos)
  {
    str.erase();
  }
  else
  {
    str.erase(0, pos);
  }
  return str;
}

class CfgValue : public Cfg::Value
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
  string MIKTEXTHISCALL GetValue() const override
  {
    if (IsMultiValue())
    {
      MIKTEX_UNEXPECTED();
    }
    return value.empty() ? "" : value.front();
  }

public:
  vector<string> MIKTEXTHISCALL GetMultiValue() const override
  {
    if (!IsMultiValue())
    {
      MIKTEX_UNEXPECTED();
    }
    return value;
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

inline bool operator< (const CfgValue& lhs, const CfgValue& rhs)
{
  return lhs.lookupName < rhs.lookupName;
}

typedef unordered_map<string, shared_ptr<CfgValue>> ValueMap;

class CfgKey : public Cfg::Key
{
public:
  ValueMap valueMap;

public:
  CfgKey()
  {
  }

public:
  CfgKey(const string& name, const string& lookupName) :
    name(name), lookupName(lookupName)
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
  vector<shared_ptr<Cfg::Value>> MIKTEXTHISCALL GetValues() const override
  {
    vector<shared_ptr<Cfg::Value>> values;
    for (const auto& p : valueMap)
    {
      values.push_back(p.second);
    }
    return values;
  }

private:
  ValueMap::iterator iter = valueMap.end();

public:
  void WriteValues(StreamWriter& writer) const;
};

inline bool operator< (const CfgKey& lhs, const CfgKey& rhs)
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

void CfgKey::WriteValues(StreamWriter& writer) const
{
  bool isKeyWritten = false;
  for (const CfgValue& v : GetCfgValues(true))
  {
    if (!isKeyWritten)
    {
      writer.WriteLine();
      writer.WriteFormattedLine("[%s]", name.c_str());
      isKeyWritten = true;
    }
    if (!v.documentation.empty())
    {
      writer.WriteLine();
      bool start = true;
      for (const char& ch : v.documentation)
      {
        if (start)
        {
          writer.WriteFormatted("%c%c ", COMMENT_CHAR, COMMENT_CHAR);
        }
        writer.Write(ch);
        start = (ch == '\n');
      }
      if (!start)
      {
        writer.WriteLine();
      }
    }
    if (v.value.empty())
    {
      writer.WriteFormattedLine("%s%s=",
        v.commentedOut ? COMMENT_CHAR_STR : "",
        v.name.c_str());
    }
    else if (v.IsMultiValue())
    {
      for (const string& val : v.value)
      {
        writer.WriteFormattedLine("%s%s=%s",
          v.commentedOut ? COMMENT_CHAR_STR : "",
          v.name.c_str(),
          val.c_str());
      }
    }
    else if (IsSearchPathValue(v.name) && v.value.front().find_first_of(PATH_DELIMITER) != string::npos)
    {
      writer.WriteFormattedLine("%s%s=",
        v.commentedOut ? COMMENT_CHAR_STR : "",
        v.name.c_str());
      for (CsvList root(v.value.front(), PATH_DELIMITER); root; ++root)
      {
        writer.WriteFormattedLine("%s%s;=%s",
          v.commentedOut ? COMMENT_CHAR_STR : "",
          v.name.c_str(),
          (*root).c_str());
      }
    }
    else
    {
      writer.WriteFormattedLine("%s%s=%s",
        v.commentedOut ? COMMENT_CHAR_STR : "",
        v.name.c_str(),
        v.value.front().c_str());
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

class MD5WalkCallback : public WalkCallback
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

#if defined(ENABLE_BOTAN)
class BotanWalkCallback : public WalkCallback
{
public:
  BotanWalkCallback(Botan::Pipe& pipe) :
    pipe(pipe)
  {
  }

private:
  Botan::Pipe& pipe;
  
public:
  void addData(const string& data) override
  {
    pipe.write(data);
  }
};
#endif

#if defined(ENABLE_OPENSSL)
class OpenSSLWalkCallback : public WalkCallback
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

class CfgImpl : public Cfg
{
public:
  bool MIKTEXTHISCALL Empty() const override;

public:
  MD5 MIKTEXTHISCALL GetDigest() const override;

public:
  string MIKTEXTHISCALL GetValue(const string& keyName, const string& valueName) const override;

public:
  bool MIKTEXTHISCALL TryGetValue(const string& keyName, const string& valueName, string& value) const override;

public:
  bool MIKTEXTHISCALL TryGetValue(const string& keyName, const string& valueName, PathName& path) const override;

public:
  bool MIKTEXTHISCALL TryGetValue(const string& keyName, const string& valueName, vector<string>& value) const override;

public:
  void MIKTEXTHISCALL SetModified(bool b) override;

public:
  bool MIKTEXTHISCALL IsModified() const override;

public:
  void MIKTEXTHISCALL PutValue(const string& keyName, const string& valueName, const string& value) override;

public:
  void MIKTEXTHISCALL PutValue(const string& keyName, const string& valueName, const string& value, const string& documentation, bool commentedOut) override;

private:
  void Read(std::istream& reader, const string& defaultKeyName, int level, bool mustBeSigned, const PathName& publicKeyFile);

private:
  void Read(const PathName& path, const string& defaultKeyName, int level, bool mustBeSigned, const PathName& publicKeyFile);

public:
  void MIKTEXTHISCALL Read(const PathName& path) override
  {
    this->path = path;
    Read(path, false);
  }

public:
  void MIKTEXTHISCALL Read(const PathName& path, bool mustBeSigned) override
  {
    this->path = path;
    Read(path, path.GetFileNameWithoutExtension().ToString(), 0, mustBeSigned, PathName());
  }

public:
  void MIKTEXTHISCALL Read(const PathName& path, const PathName& publicKeyFile) override
  {
    this->path = path;
    Read(path, path.GetFileNameWithoutExtension().ToString(), 0, true, publicKeyFile);
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
  vector<shared_ptr<Key>> GetKeys() override
  {
    vector<shared_ptr<Cfg::Key>> keys;
    for (const auto& p : keyMap)
    {
      keys.push_back(p.second);
    }
    return keys;
  }

public:
  shared_ptr<Key> MIKTEXTHISCALL FirstKey() override;

public:
  shared_ptr<Key> MIKTEXTHISCALL NextKey() override;

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
  KeyMap::const_iterator iter = keyMap.end();

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

public:
  CfgImpl();

public:
  virtual ~CfgImpl() override;

private:
  string GetDefaultKeyName() const;

private:
  shared_ptr<CfgKey> FindKey(const string& keyName) const;

private:
  void WriteKeys(StreamWriter& writer);

private:
  void PutValue(const string& keyName, const string& valueName, const string& value, PutMode putMode, const string& documentation, bool commentedOut);

private:
  bool ClearValue(const string& keyName, const string& valueName);
  
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
  traceError(TraceStream::Open(MIKTEX_TRACE_ERROR))
{
}

CfgImpl::~CfgImpl()
{
  try
  {
    traceError->Close();
    traceStream->Close();
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

void CfgImpl::WriteKeys(StreamWriter& writer)
{
  for (const CfgKey& k : GetCfgKeys(true))
  {
    k.WriteValues(writer);
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

string CfgImpl::GetValue(const string& keyName, const string& valueName) const
{
  string result;
  if (!TryGetValue(keyName, valueName, result))
  {
    MIKTEX_FATAL_ERROR_2(T_("The configuration value does not exist."), "valueName", valueName);
  }
  return result;
}

bool CfgImpl::TryGetValue(const string& keyName, const string& valueName, string& outValue) const
{
  shared_ptr<CfgKey> key = FindKey(keyName);

  if (key == nullptr)
  {
    return false;
  }

  shared_ptr<Cfg::Value> value = key->GetValue(valueName);

  if (value == nullptr || value->IsCommentedOut())
  {
    return false;
  }

  outValue = value->GetValue();

  return true;
}

bool CfgImpl::TryGetValue(const string& keyName, const string& valueName, PathName& path) const
{
  string value;
  if (!TryGetValue(keyName, valueName, value))
  {
    return false;
  }
  path = value;
  return true;
}

bool CfgImpl::TryGetValue(const string& keyName, const string& valueName, vector<string>& outValue) const
{
  shared_ptr<CfgKey> key = FindKey(keyName);

  if (key == nullptr)
  {
    return false;
  }

  shared_ptr<Cfg::Value> value = key->GetValue(valueName);

  if (value == nullptr || value->IsCommentedOut())
  {
    return false;
  }

  outValue = value->GetMultiValue();

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
          itVal->second->value.front() += PATH_DELIMITER;
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

void CfgImpl::Read(std::istream& reader, const string& defaultKeyName, int level, bool mustBeSigned, const PathName& publicKeyFile)
{
  MIKTEX_ASSERT(!(level > 0 && mustBeSigned));

  if (mustBeSigned)
  {
    traceStream->WriteFormattedLine("core", T_("signature required..."));
  }

  bool wasEmpty = Empty();

  AutoRestore<int> autoRestore1(lineno);
  AutoRestore<PathName> autoRestore(currentFile);

  string keyName = defaultKeyName;

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
      string valueName;
      string value;
      PutMode putMode;
      if (!ParseValueDefinition(line[0] == COMMENT_CHAR ? &line[1] : &line[0], valueName, value, putMode))
      {
        FATAL_CFG_ERROR(T_("invalid value definition"));
      }
      PutValue(keyName, valueName, value, putMode, documentation, line[0] == COMMENT_CHAR);
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

  if (mustBeSigned && signature.empty())
  {
    FATAL_CFG_ERROR(T_("the configuration file is not signed"));
  }

  if (wasEmpty && level == 0 && !signature.empty())
  {
#if defined(ENABLE_BOTAN)
    if (GetCryptoLib() == CryptoLib::Botan)
    {
      Botan::Pipe sigPipe(new Botan::Base64_Decoder(Botan::Decoder_Checking::FULL_CHECK));
      sigPipe.process_msg(signature);
      unique_ptr<Botan::Public_Key> pPublicKey(LoadPublicKey_Botan(publicKeyFile));
      Botan::RSA_PublicKey* pRsaKey = dynamic_cast<Botan::RSA_PublicKey*>(pPublicKey.get());
      if (pRsaKey == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
#if BOTAN_VERSION_CODE >= BOTAN_VERSION_CODE_FOR(1, 10, 0)
      Botan::Pipe pipe(new Botan::PK_Verifier_Filter(new Botan::PK_Verifier(*pRsaKey, EMSA_), sigPipe.read_all()));
#else
      Botan::Pipe pipe(new Botan::PK_Verifier_Filter(Botan::get_pk_verifier(*pRsaKey, EMSA_), sigPipe.read_all()));
#endif
      BotanWalkCallback callback(pipe);
      pipe.start_msg();
      Walk(&callback);
      pipe.end_msg();
      Botan::byte ok;
      if (pipe.read_byte(ok) != 1)
      {
        MIKTEX_UNEXPECTED();
      }
      MIKTEX_ASSERT(ok == 1 || ok == 0);
      if (ok != 1)
      {
#if !defined(DISABLE_SIGNATURE_CHECK)
        FATAL_CFG_ERROR(T_("the file has been tampered with"));
#endif
      }
    }
#endif
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

void CfgImpl::Read(const PathName& path, const string& defaultKeyName, int level, bool mustBeSigned, const PathName& publicKeyFile)
{
  traceStream->WriteFormattedLine("core", T_("parsing: %s..."), path.GetData());
  std::ifstream reader(path.ToString());
  Read(reader, defaultKeyName, level, mustBeSigned, publicKeyFile);
  reader.close();
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

#if defined(ENABLE_BOTAN)
class BotanUI : public Botan::User_Interface
{
public:
  BotanUI(IPrivateKeyProvider* pPrivateKeyProvider) :
    pPrivateKeyProvider(pPrivateKeyProvider)
  {
  }
public:
  string get_passphrase(const string& what, const string& source, UI_Result& result) const override
  {
    string passphrase;
    result = (pPrivateKeyProvider->GetPassphrase(passphrase) ? UI_Result::OK : UI_Result::CANCEL_ACTION);
    return passphrase;
  }
private:
  IPrivateKeyProvider* pPrivateKeyProvider = nullptr;
};
#endif

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

void CfgImpl::Write(const PathName& path, const string& header, IPrivateKeyProvider* pPrivateKeyProvider)
{
  time_t t = time(nullptr);
  StreamWriter writer(path);
  if (!header.empty())
  {
    writer.WriteFormattedLine("%c%c%c %s", COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR, header.c_str());
    writer.WriteLine();
  }
  WriteKeys(writer);
  if (pPrivateKeyProvider != nullptr)
  {
    string sig;
#if defined(ENABLE_BOTAN)
    if (GetCryptoLib() == CryptoLib::Botan)
    {
      unique_ptr<Botan::PKCS8_PrivateKey> privateKey;
      Botan::AutoSeeded_RNG rng;
      BotanUI ui(pPrivateKeyProvider);
      unique_ptr<Botan::Pipe> pPipe;
      privateKey.reset(Botan::PKCS8::load_key(pPrivateKeyProvider->GetPrivateKeyFile().ToString(), rng, ui));
      Botan::RSA_PrivateKey* pRsaKey = dynamic_cast<Botan::RSA_PrivateKey*>(privateKey.get());
      if (pRsaKey == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
      pPipe.reset(new Botan::Pipe(
#if BOTAN_VERSION_CODE >= BOTAN_VERSION_CODE_FOR(1, 10, 0)
                                  new Botan::PK_Signer_Filter(new Botan::PK_Signer(*pRsaKey, EMSA_), rng),
#else
                                  new Botan::PK_Signer_Filter(new Botan::PK_Signer(*pRsaKey, Botan::get_emsa(EMSA_)), rng),
#endif
                                  new Botan::Base64_Encoder()));
      pPipe->start_msg();
      BotanWalkCallback callback(*pPipe);
      Walk(&callback);
      pPipe->end_msg();
      sig = pPipe->read_all_as_string();
    }
#endif
#if defined(ENABLE_OPENSSL)
    if (GetCryptoLib() == CryptoLib::OpenSSL)
    {
      FileStream stream(File::Open(pPrivateKeyProvider->GetPrivateKeyFile(), FileMode::Open, FileAccess::Read));
      RSA_ptr rsa (PEM_read_RSAPrivateKey(stream.Get(), nullptr, OpenSSLPasswordCallback, pPrivateKeyProvider), RSA_free);
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
    writer.WriteLine();
    writer.WriteFormattedLine(
      T_("%c%c%c%c This configuration file is signed by a MiKTeX maintainer. The signature follows."),
      COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR);
    writer.WriteFormattedLine(
      T_("%c%c%c%c-----BEGIN MIKTEX SIGNATURE-----"),
      COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR);
    writer.WriteFormattedLine("%c%c%c%c signature/miktex: %s",
      COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR,
      sig.c_str());
    writer.WriteFormattedLine(
      T_("%c%c%c%c-----END MIKTEX SIGNATURE-----"),
      COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR);
  }
  writer.Close();
  File::SetTimes(path, t, t, t);
}

shared_ptr<Cfg::Key> CfgImpl::FirstKey()
{
  iter = keyMap.begin();
  if (iter == keyMap.end())
  {
    return nullptr;
  }
  return iter->second;
}

shared_ptr<Cfg::Key> CfgImpl::NextKey()
{
  if (iter == keyMap.end())
  {
    MIKTEX_UNEXPECTED();
  }
  ++iter;
  if (iter == keyMap.end())
  {
    return nullptr;
  }
  return iter->second;
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
