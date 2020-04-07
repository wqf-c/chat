#pragma once
#include<iostream>
#include<sstream>
#include<string>
#include<memory>
#include <list>   
#include<mutex>
#include<condition_variable>
#include<thread>
#include"constant.h"
using std::string;
using std::stringstream;
using std::shared_ptr;
using std::list;
using std::make_shared;
using std::mutex;
using std::condition_variable;
using std::unique_lock;
using std::thread;
using std::mem_fn;

enum  class EventType { Login, Chat, Logout };

struct Event {
	EventType type;
	Event(const EventType etype) : type(etype) {};
	virtual string toMsg() = 0;
	virtual string getEventInfo() = 0;
};

struct LoginEvent : Event {
	string username;
	string password;

	LoginEvent(string uname, string pwd) : Event(EventType::Login), username(uname), password(pwd) {};

	string toMsg() override {
		stringstream ss;
		ss << static_cast<int>(EventType::Login) << "|";
		ss << username << "|";
		ss << password << "|";
		return ss.str();
	}

	string getEventInfo() override {
		string s = "[login, username=";
		s += username;
		s += "]";
		return s;
	}

	static shared_ptr<LoginEvent> create(string &data) {
		shared_ptr<LoginEvent> evt{};
		bool parseFail = false;
		if (data[0] == '0') {
			try {
				stringstream ss{ data };
				char buf[BUFFER_SIZE];
				ss.getline(buf, BUFFER_SIZE, '|');
				ss.getline(buf, BUFFER_SIZE, '|');
				string uname{ buf };
				ss.getline(buf, BUFFER_SIZE, '|');
				string pwd{ buf };
				evt.reset(new LoginEvent(uname, pwd));
			}
			catch (...) {
				parseFail = true;
			}
		}
		else {
			parseFail = true;
		}
		if (!parseFail) {
			return evt;
		}
		else {
			string failInfo = "[ERROR INFO: LoginEvent parse fail:[" + data + "] fail";
			throw failInfo;
		}
	}
};

struct ChatEvent : Event {
	string fromUser;
	string toUsr;
	string content;

	ChatEvent(string _fromUser, string _toUser, string _content) :
		Event(EventType::Chat), fromUser(_fromUser), toUsr(_toUser), content(_content) {};

	string toMsg() override {
		stringstream ss;
		ss << static_cast<int>(EventType::Chat) << "|";
		ss << fromUser << "|";
		ss << toUsr << "|";
		ss << content << '|';
		return ss.str();
	}

	string getEventInfo() override {
		string s = "[chat, fromUser=";
		s += fromUser;
		s += ", toUser=";
		s += toUsr;
		s += ", content=";
		s += content;
		s += "]";
		return s;
	}

	static shared_ptr<ChatEvent> create(string &data) {
		shared_ptr<ChatEvent> evt{};
		bool parseFail = false;
		if (data[0] == '1') {
			try {
				stringstream ss{ data };
				char buf[BUFFER_SIZE];
				ss.getline(buf, BUFFER_SIZE, '|');
				ss.getline(buf, BUFFER_SIZE, '|');
				string _fromUser{ buf };
				ss.getline(buf, BUFFER_SIZE, '|');
				string _toUser{ buf };
				ss.getline(buf, BUFFER_SIZE, '|');
				string _content{ buf };
				evt.reset(new ChatEvent(_fromUser, _toUser, _content));
			}
			catch (...) {
				parseFail = true;
			}
		}
		else {
			parseFail = true;
		}

		if (parseFail) {
			string failInfo = "[ERROR INFO: ChatEvent parse fail:[" + data + "] fail";
			throw failInfo;
		}
		else {
			return evt;
		}

	}
};

struct LogoutEvent : Event {
	string username;

	LogoutEvent() : Event(EventType::Logout) {};

	string toMsg() override {
		stringstream ss;
		ss << static_cast<int>(EventType::Logout) << "|";
		ss << username << "|";
		return ss.str();
	}

	string getEventInfo() override {
		string s = "[logout, username=";
		s += username;
		s += "]";
		return s;
	}

	shared_ptr<LogoutEvent> create(string &data) {
		shared_ptr<LogoutEvent> evt{};
		bool parseFail = false;
		if (data[0] == '2') {
			try {
				stringstream ss{ data };
				char buf[BUFFER_SIZE];
				ss.getline(buf, BUFFER_SIZE, '|');
				ss.getline(buf, BUFFER_SIZE, '|');
				string uname{ buf };
				evt = make_shared<LogoutEvent>();
			}
			catch (...) {
				parseFail = true;
			}
		}
		else {
			parseFail = true;
		}
		if (parseFail) {
			string failInfo = "[ERROR INFO: LogoutEvent parse fail:[" + data + "] fail]";
			throw failInfo;
		}
		else {
			return evt;
		}
	}
};



class EventProcessor{

public:
	void push(shared_ptr<Event> evt) {
		unique_lock<mutex> locker(mu);
		eventList.push_back(evt);
		locker.unlock();
		cv.notify_one();
	}

	void handleEvent() {
		list<shared_ptr<Event>> targetList;
		while (true)
		{
			if (!targetList.empty()) {
				shared_ptr<Event> event = targetList.front();
				targetList.pop_front();
				processEvent(event);
			}
			else {
				unique_lock<mutex> locker(mu);
				cv.wait(locker, [this]() {return !eventList.empty(); });
				eventList.swap(targetList);
				locker.unlock();
			}
		}
	}

	void startEventHandler() {
		thread t(mem_fn(&EventProcessor::handleEvent), this);
		t.join();
	}

	EventProcessor() {
		//thread t(std::mem_fn(&EventProcessor::startHandler), std::ref(*this));
		thread t(&EventProcessor::handleEvent, this);
		t.detach();
	};

private:
	list<shared_ptr<Event>> eventList;
	mutex mu;
	condition_variable cv;

protected:
	virtual void processEvent(shared_ptr<Event>  event) = 0;
};