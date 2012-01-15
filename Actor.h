#pragma once

#include <boost/thread.hpp>
#include <boost/any.hpp>
#include "MessageQueue.h"

class Actor {
public:
	void start() {
		m_thread = boost::thread([this]{return this->act();});
		m_act_running = true;
	}

	void end() {
		if(m_thread.joinable())
			m_thread.join();
	}

	void send(const boost::any& msg) {
		m_mq.enqueue(msg);
	}

	bool is_mailbox_empty()	{
		return m_mq.empty();
	}

	void exit()	{
		m_act_running = false;
	}

	void wait_and_dequeue(boost::any& value) {
		m_mq.wait_and_dequeue(value);
	}
			
	virtual void act() = 0;

protected:
	MessageQueue m_mq;

private:
	boost::thread m_thread;
	bool m_act_running;
};

