#pragma once
#include <condition_variable>
template <class T>
class Condition
{
public:

	std::mutex m_mutex;
	std::condition_variable   m_condition;
	~Condition() {
		m_condition.notify_all();
	}
	void notify(std::unique_ptr<T> data) noexcept
	{
		{
			std::lock_guard<std::mutex> eventLock(m_mutex);
			any.swap(data);
		}
		// wake up one waiter
		m_condition.notify_one();
	};

	
	std::unique_ptr<T> wait(unsigned timeout=0)
	{
		std::unique_lock<std::mutex> ul(m_mutex);
		if (!timeout) {
			m_condition.wait(ul,[&]() {
					if (!any)
						return false;
					return true;
			});
		}
		else{
			if(!any){
				std::cv_status status = m_condition.wait_for(ul, std::chrono::milliseconds(timeout));
				if (status == std::cv_status::timeout)
				{
					return {};
				}
			}
		}
		return std::unique_ptr<T>(any.release());
		
	}
private:
	std::unique_ptr<T> any;
};
