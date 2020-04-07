#pragma once
#include<iostream>
#include"SocketClient.h"
#include"EventProcessor.h"

using std::string;

enum class UserStatue {waitName, waitPwd, Logining, Logined};
class UserAuth : public EventProcessor{
private:
	SocketClient client;
	UserStatue curState = UserStatue::waitName;
	string username;
	string password;
	string toUser = "";

public:
	void start();
	void startInput();
	virtual void processEvent(shared_ptr<Event> event);
};