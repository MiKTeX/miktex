/* cfg.cpp: configuration files

   Copyright (C) 1996-2016 Christian Schenk

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

#include "miktex/Core/CSVList.h"
#include "miktex/Core/Cfg.h"
#include "miktex/Core/Registry.h"
#include "miktex/Core/StreamReader.h"
#include "miktex/Core/StreamWriter.h"

#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;

#define FATAL_CFG_ERROR(message) \
  MIKTEX_FATAL_ERROR_2(T_("A MiKTeX configuration file could not be loaded."), "file", currentFile.ToString(), "line", std::to_string(lineno), "error", message)

const char COMMENT_CHAR = ';';
const char * const COMMENT_CHAR_STR = ";";

const char * const EMSA_ = "EMSA1(SHA-256)";

MIKTEXSTATICFUNC(bool) EndsWith(const string & s, const string & suffix)
{
  return s.length() >= suffix.length() &&
    s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
}

MIKTEXSTATICFUNC(string &) Trim(string & str)
{
  const char * whitespace = " \t\r\n";
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
  string documentation;

public:
  bool commentedOut = false;

public:
  string name;

public:
  string lookupName;

public:
  vector<string> value;

public:
  CfgValue()
  {
  }

public:
  CfgValue(const string & name, const string & lookupName, const string & value, const string & documentation, bool isCommentedOut) :
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

inline bool operator< (const CfgValue & lhs, const CfgValue & rhs)
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
  CfgKey(const string & name, const string & lookupName) :
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
  shared_ptr<Cfg::Value> MIKTEXTHISCALL GetValue(const string & valueName) const override
  {
    ValueMap::const_iterator it = valueMap.find(Utils::MakeLower(valueName));
    if (it == valueMap.end())
    {
      return nullptr;
    }
    return it->second;
  }

public:
  vector<shared_ptr<Cfg::Value>> MIKTEXTHISCALL GetValues() const override
  {
    vector<shared_ptr<Cfg::Value>> values;
    for (const auto & p : valueMap)
    {
      values.push_back(p.second);
    }
    return values;
  }

private:
  ValueMap::iterator iter = valueMap.end();

public:
  void WriteValues(StreamWriter & writer);
};

inline bool operator< (const CfgKey & lhs, const CfgKey & rhs)
{
  return lhs.lookupName < rhs.lookupName;
}

static const char * const knownSearchPathValues[] = {
  "path",
  "extensions",
  MIKTEX_REGVAL_COMMON_ROOTS,
  MIKTEX_REGVAL_USER_ROOTS,
};

bool IsSearchPathValue(const string & valueName)
{
  for (int idx = 0; idx != sizeof(knownSearchPathValues) / sizeof(knownSearchPathValues[0]); ++idx)
  {
    if (Utils::EqualsIgnoreCase(valueName, knownSearchPathValues[idx]))
    {
      return true;
    }
  }
  return false;
}

void CfgKey::WriteValues(StreamWriter & writer)
{
  bool isKeyWritten = false;
  for (ValueMap::iterator it = valueMap.begin(); it != valueMap.end(); ++it)
  {
    if (!isKeyWritten)
    {
      writer.WriteLine();
      writer.WriteFormattedLine("[%s]", name.c_str());
      isKeyWritten = true;
    }
    if (!it->second->documentation.empty())
    {
      writer.WriteLine();
      bool start = true;
      for (string::const_iterator it2 = it->second->documentation.begin(); it2 != it->second->documentation.end(); ++it2)
      {
        if (start)
        {
          writer.WriteFormatted("%c%c ", COMMENT_CHAR, COMMENT_CHAR);
        }
        writer.Write(*it2);
        start = (*it2 == '\n');
      }
      if (!start)
      {
        writer.WriteLine();
      }
    }
    if (it->second->value.empty())
    {
      writer.WriteFormattedLine("%s%s=",
        it->second->commentedOut ? COMMENT_CHAR_STR : "",
        it->second->name.c_str());
    }
    else if (it->second->IsMultiValue())
    {
      for (const string & val : it->second->value)
      {
        writer.WriteFormattedLine("%s%s=%s",
          it->second->commentedOut ? COMMENT_CHAR_STR : "",
          it->second->name.c_str(),
          val.c_str());
      }
    }
    else if (IsSearchPathValue(it->second->name) && it->second->value.front().find_first_of(PATH_DELIMITER) != string::npos)
    {
      writer.WriteFormattedLine("%s%s=",
        it->second->commentedOut ? COMMENT_CHAR_STR : "",
        it->second->name.c_str());
      for (CSVList root(it->second->value.front(), PATH_DELIMITER); root.GetCurrent() != nullptr; ++root)
      {
        writer.WriteFormattedLine("%s%s;=%s",
          it->second->commentedOut ? COMMENT_CHAR_STR : "",
          it->second->name.c_str(),
          root.GetCurrent());
      }
    }
    else
    {
      writer.WriteFormattedLine("%s%s=%s",
        it->second->commentedOut ? COMMENT_CHAR_STR : "",
        it->second->name.c_str(),
        it->second->value.front().c_str());
    }
  }
}

typedef unordered_map<string, shared_ptr<CfgKey>> KeyMap;

class CfgImpl : public Cfg
{
public:
  bool MIKTEXTHISCALL Empty() const override;

public:
  MD5 MIKTEXTHISCALL GetDigest() const override;

public:
  string MIKTEXTHISCALL GetValue(const string & keyName, const string & valueName) const override;

public:
  bool MIKTEXTHISCALL TryGetValue(const string & keyName, const string & valueName, string & value) const override;

public:
  bool MIKTEXTHISCALL TryGetValue(const string & keyName, const string & valueName, PathName & path) const override;

public:
  bool MIKTEXTHISCALL TryGetValue(const string & keyName, const string & valueName, vector<string> & value) const override;

public:
  void MIKTEXTHISCALL SetModified(bool b) override;

public:
  bool MIKTEXTHISCALL IsModified() const override;

public:
  void MIKTEXTHISCALL PutValue(const string & keyName, const string & valueName, const string & value) override;

public:
  void MIKTEXTHISCALL PutValue(const string & keyName, const string & valueName, const string & value, const string & documentation, bool commentedOut) override;

public:
  void MIKTEXTHISCALL Read(const PathName & path) override
  {
    Read(path, false);
  }

public:
  void MIKTEXTHISCALL Write(const PathName & path) override
  {
    Write(path, T_("DO NOT EDIT THIS FILE!"));
  }

public:
  void MIKTEXTHISCALL Write(const PathName & path, const string & header) override
  {
    Write(path, header, nullptr);
  }

public:
  vector<shared_ptr<Key>> GetKeys() override
  {
    vector<shared_ptr<Cfg::Key>> keys;
    for (const auto & p : keyMap)
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
  void MIKTEXTHISCALL DeleteKey(const string & keyName) override;

public:
  void MIKTEXTHISCALL DeleteValue(const string & keyName, const string & valueName) override;

public:
  void MIKTEXTHISCALL Read(const PathName & path, bool mustBeSigned) override;

public:
  bool MIKTEXTHISCALL IsSigned() const override
  {
    return signature.size() > 0;
  }

public:
  void MIKTEXTHISCALL Write(const PathName & path, const string & header, IPrivateKeyProvider * pPrivateKeyProvider) override;

private:
  void Read(const PathName & path, const string & defaultKeyName, int level, bool mustBeSigned);

private:
  enum PutMode {
    None,
    Append,
    SearchPathAppend
  };

private:
  bool ParseValueDefinition(const string & line, string & valueName, string & value, PutMode & putMode);

private:
  void Walk(Botan::Pipe & pipe) const;

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
  Botan::SecureVector<Botan::byte> signature;

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
  shared_ptr<CfgKey> FindKey(const string & keyName) const;

private:
  void WriteKeys(StreamWriter & writer);

private:
  void PutValue(const string & keyName, const string & valueName, const string & value, PutMode putMode, const string & documentation, bool commentedOut);

private:
  bool ClearValue(const string & keyName, const string & valueName);
  
private:
  int lineno = 0;

private:
  PathName currentFile;

private:
  friend class Cfg;
};

Cfg::~Cfg()
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
  catch (const exception &)
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

shared_ptr<CfgKey> CfgImpl::FindKey(const string & keyName) const
{
  KeyMap::const_iterator it = keyMap.find(Utils::MakeLower(keyName.empty() ? GetDefaultKeyName() : keyName));
  if (it == keyMap.end())
  {
    return nullptr;
  }
  return it->second;
}

void CfgImpl::WriteKeys(StreamWriter & writer)
{
  for (auto & p : keyMap)
  {
    p.second->WriteValues(writer);
  }
  if (tracking)
  {
    snapshotDigest = GetDigest();
  }
}

void CfgImpl::DeleteKey(const string & keyName)
{
  KeyMap::iterator it = keyMap.find(Utils::MakeLower(keyName));
  if (it == keyMap.end())
  {
    INVALID_ARGUMENT("keyName", keyName);
  }
  keyMap.erase(it);
}

void CfgImpl::Walk(Botan::Pipe & pipe) const
{
  vector<CfgKey> keys;
  keys.reserve(keyMap.size());
  for (const auto & p : keyMap)
  {
    keys.push_back(*p.second);
  }
  sort(keys.begin(), keys.end());
  for (const CfgKey & key : keys)
  {
    pipe.write("[");
    pipe.write(key.lookupName);
    pipe.write("]\n");
    vector<CfgValue> values;
    values.reserve(key.valueMap.size());
    for (const auto & p : key.valueMap)
    {
      values.push_back(*p.second);
    }
    sort(values.begin(), values.end());
    for (const CfgValue & val : values)
    {
      if (val.value.empty())
      {
        pipe.write(val.lookupName);
        pipe.write("=");
        pipe.write("\n");
      }
      else if (val.IsMultiValue())
      {
        for (const string & v : val.value)
        {
          pipe.write(val.lookupName);
          pipe.write("=");
          pipe.write(v);
          pipe.write("\n");
        }
      }
      else
      {
        pipe.write(val.lookupName);
        pipe.write("=");
        pipe.write(val.value.front());
        pipe.write("\n");
      }
    }
  }
}

MD5 CfgImpl::GetDigest() const
{
  Botan::Pipe pipe(new Botan::Hash_Filter("MD5"));
  pipe.start_msg();
  Walk(pipe);
  pipe.end_msg();
  Botan::SecureVector<Botan::byte> md5 = pipe.read_all(0);
  MiKTeX::Core::MD5 result;
  MIKTEX_ASSERT(sizeof(result) == 16);
  MIKTEX_ASSERT(md5.size() == sizeof(result));
  memcpy(&result[0], &md5[0], md5.size());
  return result;
}

unique_ptr<Cfg> Cfg::Create()
{
  return make_unique<CfgImpl>();
}

string CfgImpl::GetValue(const string & keyName, const string & valueName) const
{
  string result;
  if (!TryGetValue(keyName, valueName, result))
  {
    MIKTEX_FATAL_ERROR_2(T_("The configuration value does not exist."), "valueName", valueName);
  }
  return result;
}

bool CfgImpl::TryGetValue(const string & keyName, const string & valueName, string & outValue) const
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

bool CfgImpl::TryGetValue(const string & keyName, const string & valueName, PathName & path) const
{
  string value;
  if (!TryGetValue(keyName, valueName, value))
  {
    return false;
  }
  path = value;
  return true;
}

bool CfgImpl::TryGetValue(const string & keyName, const string & valueName, vector<string> & outValue) const
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

void CfgImpl::PutValue(const string & keyName_, const string & valueName, const string & value, CfgImpl::PutMode putMode, const string & documentation, bool commentedOut)
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

bool CfgImpl::ClearValue(const string & keyName_, const string & valueName)
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

void CfgImpl::PutValue(const string & keyName, const string & valueName, const string & value)
{
  return PutValue(keyName, valueName, value, None, "", false);
}

void CfgImpl::PutValue(const string & keyName, const string & valueName, const string & value, const string & documentation, bool commentedOut)
{
  return PutValue(keyName, valueName, value, None, value, commentedOut);
}

void CfgImpl::Read(const PathName & path, bool mustBeSigned)
{
  Read(path, path.GetFileNameWithoutExtension().ToString(), 0, mustBeSigned);
  this->path = path;
}

void CfgImpl::Read(const PathName & path, const string & defaultKeyName, int level, bool mustBeSigned)
{
  MIKTEX_ASSERT(!(level > 0 && mustBeSigned));

  traceStream->WriteFormattedLine("core", T_("parsing: %s..."), path.Get());

  if (mustBeSigned)
  {
    traceStream->WriteFormattedLine("core", T_("signature required..."));
  }

  bool wasEmpty = Empty();

  StreamReader reader(path);

  string line;
  line.reserve(128);

  AutoRestore<int> autoRestore1(lineno);
  AutoRestore<PathName> autoRestore(currentFile);

  string keyName = defaultKeyName;

  lineno = 0;
  currentFile = path;

  string documentation;

  while (reader.ReadLine(line))
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
      Tokenizer tok(line.c_str() + 1, " \t");
      const char * lpsz = tok.GetCurrent();
      if (lpsz == 0)
      {
        FATAL_CFG_ERROR(T_("invalid cfg directive"));
      }
      if (StringCompare(lpsz, "include") == 0)
      {
        ++tok;
        lpsz = tok.GetCurrent();
        if (lpsz == nullptr)
        {
          FATAL_CFG_ERROR(T_("missing file name argument"));
        }
        PathName path2(path);
        path2.MakeAbsolute();
        path2.RemoveFileSpec();
        path2 /= lpsz;
        Read(path2, keyName, level + 1, false);
      }
      else if (StringCompare(lpsz, "clear") == 0)
      {
        ++tok;
        lpsz = tok.GetCurrent();
        if (lpsz == nullptr)
        {
          FATAL_CFG_ERROR(T_("missing value name argument"));
        }
        ClearValue(keyName, lpsz);
      }
      else
      {
        FATAL_CFG_ERROR(T_("unknown cfg directive"));
      }
    }
    else if (line[0] == '[')
    {
      documentation = "";
      Tokenizer tok(line.c_str() + 1, "]");
      const char * lpsz = tok.GetCurrent();
      if (lpsz == nullptr)
      {
        FATAL_CFG_ERROR(T_("incomplete secion name"));
      }
      keyName = lpsz;
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
      Tokenizer tok(line.c_str() + 4, " \t");
      const char * lpsz = tok.GetCurrent();
      if (lpsz != nullptr)
      {
        if (StringCompare(lpsz, "signature/miktex:") == 0)
        {
          ++tok;
          lpsz = tok.GetCurrent();
          if (lpsz != nullptr && wasEmpty && level == 0)
          {
            Botan::Pipe pipe(new Botan::Base64_Decoder(Botan::Decoder_Checking::FULL_CHECK));
            pipe.process_msg(lpsz);
            signature = pipe.read_all();
          }
        }
      }
    }
  }

  reader.Close();

  if (mustBeSigned && signature.size() == 0)
  {
    FATAL_CFG_ERROR(T_("the configuration file is not signed"));
  }

  if (wasEmpty && level == 0 && signature.size() > 0)
  {
    unique_ptr<Botan::Public_Key> pPublicKey(LoadPublicKey());
    Botan::RSA_PublicKey * pRsaKey = dynamic_cast<Botan::RSA_PublicKey*>(pPublicKey.get());
    if (pRsaKey == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
#if BOTAN_VERSION_CODE >= BOTAN_VERSION_CODE_FOR(1, 10, 0)
    Botan::Pipe pipe(new Botan::PK_Verifier_Filter(new Botan::PK_Verifier(*pRsaKey, EMSA_), signature));
#else
    Botan::Pipe pipe(new Botan::PK_Verifier_Filter(Botan::get_pk_verifier(*pRsaKey, EMSA_), signature));
#endif
    pipe.start_msg();
    Walk(pipe);
    pipe.end_msg();
    Botan::byte ok;
    if (pipe.read_byte(ok) != 1)
    {
      MIKTEX_UNEXPECTED();
    }
    MIKTEX_ASSERT(ok == 1 || ok == 0);
    if (ok != 1)
    {
      FATAL_CFG_ERROR(T_("the file has been tampered with"));
    }
  }
}

bool CfgImpl::ParseValueDefinition(const string & line, string & valueName, string & value, CfgImpl::PutMode & putMode)
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

class BotanUI : public Botan::User_Interface
{
public:
  BotanUI(IPrivateKeyProvider * pPrivateKeyProvider) :
    pPrivateKeyProvider(pPrivateKeyProvider)
  {
  }
public:
  string get_passphrase(const string & what, const string & source, UI_Result & result) const override
  {
    string passphrase;
    result = (pPrivateKeyProvider->GetPassphrase(passphrase) ? UI_Result::OK : UI_Result::CANCEL_ACTION);
    return passphrase;
  }
private:
  IPrivateKeyProvider * pPrivateKeyProvider = nullptr;
};

void CfgImpl::Write(const PathName & path, const string & header, IPrivateKeyProvider * pPrivateKeyProvider)
{
  time_t t = time(0);
  StreamWriter writer(path);
  if (!header.empty())
  {
    writer.WriteFormattedLine("%c%c%c %s", COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR, header.c_str());
    writer.WriteLine();
  }
  WriteKeys(writer);
  if (pPrivateKeyProvider != nullptr)
  {
    unique_ptr<Botan::PKCS8_PrivateKey> privateKey;
    Botan::AutoSeeded_RNG rng;
    BotanUI ui(pPrivateKeyProvider);
    unique_ptr<Botan::Pipe> pPipe;
    privateKey.reset(Botan::PKCS8::load_key(
      pPrivateKeyProvider->GetPrivateKeyFile().ToString(),
      rng,
      ui));
    Botan::RSA_PrivateKey * pRsaKey = dynamic_cast<Botan::RSA_PrivateKey *>(privateKey.get());
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
    Walk(*pPipe);
    pPipe->end_msg();
    writer.WriteLine();
    writer.WriteFormattedLine(
      T_("%c%c%c%c This configuration file is signed by a MiKTeX maintainer. The signature follows."),
      COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR);
    writer.WriteFormattedLine(
      T_("%c%c%c%c-----BEGIN MIKTEX SIGNATURE-----"),
      COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR);
    writer.WriteFormattedLine("%c%c%c%c signature/miktex: %s",
      COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR, COMMENT_CHAR,
      pPipe->read_all_as_string().c_str());
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

void CfgImpl::DeleteValue(const string & keyName, const string & valueName)
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
