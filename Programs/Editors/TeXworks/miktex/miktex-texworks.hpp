/* miktex-texworks.hpp:

   Copyright (C) 2015-2020 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#include <string>
#include <vector>

#include <miktex/Trace/TraceStream>
#include <miktex/Trace/TraceCallback>

#define MIKTEX_FATAL(s) MiKTeX::TeXworks::Wrapper::GetInstance()->GetTraceStream()->WriteLine("texworks", MiKTeX::Trace::TraceLevel::Fatal, s)
#define MIKTEX_ERROR(s) MiKTeX::TeXworks::Wrapper::GetInstance()->GetTraceStream()->WriteLine("texworks", MiKTeX::Trace::TraceLevel::Error, s)
#define MIKTEX_WARNING(s) MiKTeX::TeXworks::Wrapper::GetInstance()->GetTraceStream()->WriteLine("texworks", MiKTeX::Trace::TraceLevel::Warning, s)
#define MIKTEX_INFO(s) MiKTeX::TeXworks::Wrapper::GetInstance()->GetTraceStream()->WriteLine("texworks", MiKTeX::Trace::TraceLevel::Info, s)
#define MIKTEX_TRACE(s) MiKTeX::TeXworks::Wrapper::GetInstance()->GetTraceStream()->WriteLine("texworks", MiKTeX::Trace::TraceLevel::Trace, s)
#define MIKTEX_DEBUG(s) MiKTeX::TeXworks::Wrapper::GetInstance()->GetTraceStream()->WriteLine("texworks", MiKTeX::Trace::TraceLevel::Debug, s)

namespace MiKTeX { namespace TeXworks {

class Wrapper :
  public MiKTeX::Trace::TraceCallback
{
public:
  Wrapper();

public:
  Wrapper(const Wrapper& other) = delete;

public:
  Wrapper& operator=(const Wrapper& other) = delete;

public:
  Wrapper(Wrapper&& other) = delete;

public:
  Wrapper& operator=(Wrapper&& other) = delete;

public:
  ~Wrapper() = default;

public:
  int Run(int(*Main)(int argc, char* argv[]), int argc, char* argv[]);
  
private:
  bool isLog4cxxConfigured = false;

private:
  std::vector<MiKTeX::Trace::TraceCallback::TraceMessage> pendingTraceMessages;

public:
  bool MIKTEXTHISCALL Trace(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage) override;

private:
  void FlushPendingTraceMessages();

private:
  void TraceInternal(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage);

private:
  void Sorry()
  {
    return Sorry("");
  }

private:
  void Sorry(std::string reason);

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;

public:
  MiKTeX::Trace::TraceStream* GetTraceStream() const
  {
    return traceStream.get();
  }

private:
  static Wrapper* instance;

public:
  static Wrapper* GetInstance()
  {
    return instance;
  }
};

}}
