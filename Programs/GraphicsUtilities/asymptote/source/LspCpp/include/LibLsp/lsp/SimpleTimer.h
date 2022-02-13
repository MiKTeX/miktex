#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <boost/asio.hpp>

template<typename Duration = boost::posix_time::milliseconds>
class SimpleTimer
{
public:
    SimpleTimer(unsigned int duration,const std::function<void()>& _call_back)
	:is_running_(true), call_back(_call_back), _deadline_timer(_ios, Duration(duration))
    {
        _deadline_timer.async_wait([&](const boost::system::error_code& e)
        {
            if (e.value() == boost::asio::error::operation_aborted)
            {
                return;
            }
            if(is_running_.load(std::memory_order_relaxed))
            {
                call_back();
            }

        });
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
    boost::asio::io_service _ios;
    boost::asio::deadline_timer _deadline_timer;
    std::thread _thread;


};
