#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <LibLsp/lsp/asio.h>

template<typename Duration = std::chrono::milliseconds>
class SimpleTimer
{
public:
    SimpleTimer(unsigned int duration, std::function<void()> const& _call_back)
        : is_running_(true), call_back(_call_back), _timer(_ios, Duration(duration))
    {
        _timer.async_wait(
            [&](std::error_code const& e)
            {
                if (e.value() == asio::error::operation_aborted)
                {
                    return;
                }
                if (is_running_.load(std::memory_order_relaxed))
                {
                    call_back();
                }
            }
        );
        _thread = std::thread([this] { _ios.run(); });
    }
    ~SimpleTimer()
    {
        Stop();
    }
    void Stop()
    {
        is_running_.store(false, std::memory_order_relaxed);
        _ios.stop();
        if (_thread.joinable())
        {
            _thread.join();
        }
    }

private:
    std::atomic_bool is_running_;
    std::function<void()> call_back;
    asio::io_context _ios;
    asio::steady_timer       _timer;
    std::thread _thread;
};
