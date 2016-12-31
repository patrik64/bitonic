#pragma once

#include <boost/any.hpp>

class MessageQueue {
private:
	struct Node {
		boost::shared_ptr<const boost::any> data;
		Node* next;
		Node(): next(NULL) {}
	};

	boost::mutex head_mutex;
	Node* head;
	boost::mutex tail_mutex;
	Node* tail;
	boost::condition_variable data_cond;

	Node* get_tail() {
		boost::lock_guard<boost::mutex> tail_lock(tail_mutex);
		return tail;
	}

public:
	explicit MessageQueue() : head(new Node), tail(head) {}
	~MessageQueue()	{
		while(head)	{
			Node* const old_head = head;
			head = old_head->next;
			delete old_head;
		}
	}

	void wait_and_dequeue(boost::any& value) {
		boost::unique_lock<boost::mutex> head_lock(head_mutex);
		data_cond.wait(head_lock, [=]{return head != this->get_tail();});
		value = *(head->data);
		Node* const old_head = head;
		head = old_head->next;
		delete old_head;
	}

	void enqueue(const boost::any& new_value) {
		boost::lock_guard<boost::mutex> tail_lock(tail_mutex);
		boost::shared_ptr<boost::any> new_data(new boost::any(new_value));
		std::unique_ptr<Node> p (new Node);
		tail->data = new_data;
		tail->next = p.get();
		tail = p.release();
		
		data_cond.notify_one();
	}

	bool empty() {
		boost::lock_guard<boost::mutex> head_lock(head_mutex);
		return (head == get_tail());
	}
};


