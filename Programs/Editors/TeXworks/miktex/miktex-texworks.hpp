/**
 * @file miktex/miktex-texworks.h:
 * @author Christian Schenk
 * @brief Version number
 *
 * @copyright Copyright © 2015-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

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

namespace MiKTeX {
    namespace TeXworks {

        class Wrapper :
            public MiKTeX::Trace::TraceCallback
        {

        public:
            Wrapper();
            Wrapper(const Wrapper& other) = delete;
            Wrapper& operator=(const Wrapper& other) = delete;
            Wrapper(Wrapper&& other) = delete;
            Wrapper& operator=(Wrapper&& other) = delete;
            ~Wrapper() = default;

            int Run(int(*Main)(int argc, char* argv[]), int argc, char* argv[]);

            bool MIKTEXTHISCALL Trace(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage) override;

            MiKTeX::Trace::TraceStream* GetTraceStream() const
            {
                return traceStream.get();
            }

            static Wrapper* GetInstance()
            {
                return instance;
            }

        private:

            void FlushPendingTraceMessages();
            void TraceInternal(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage);
            void Sorry(std::string reason);
            void Sorry()
            {
                return Sorry("");
            }

            bool isLog4cxxConfigured = false;
            std::vector<MiKTeX::Trace::TraceCallback::TraceMessage> pendingTraceMessages;
            std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;

            static Wrapper* instance;
        };
    }
}
