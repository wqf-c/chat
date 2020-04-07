#include<iostream>
#include"UserAuth.h"
#include"constant.h"
#include<thread>
#include<string.h>
#include<memory>
#include"EasyWay.h"

using std::cout;
using std::endl;
using std::shared_ptr;
using std::reinterpret_pointer_cast;

void UserAuth::processEvent(shared_ptr<Event> event) {
	switch (event->type)
	{
	case EventType::Login: {
		shared_ptr<LoginEvent> login = reinterpret_pointer_cast<LoginEvent>(event);
		cout << login->username << " " << login << endl;
	}break;

	case EventType::Chat: {
		auto chat = reinterpret_pointer_cast<ChatEvent>(event);
		if (chat->fromUser == username) {
			string data = chat->toMsg();
		
			if (client.sendMsg(data) < 0) {
				//client.exit();
			}
		}
		if (chat->toUsr == username) {
			cout << chat->fromUser << ": " << chat->content << endl;
		}
	}break;

	case EventType::Logout: {
		curState = UserStatue::waitName;
		cout << username << " logout" << endl;
	}break;

	default:
		break;
	}
}

void UserAuth::startInput() {
	while (true)
	{
		switch (curState)
		{
		case UserStatue::waitName: {
			cout << "输入用户名：";
			std::getline(std::cin, username);
			EasyWay::trim(username);
			curState = UserStatue::waitPwd;
		}break;

		case UserStatue::waitPwd: {
			cout << "输入密码：";
			std::getline(std::cin, password);
			EasyWay::trim(password);
			curState = UserStatue::Logining;
		}break;

		case UserStatue::Logining: {
			LoginEvent evt(username, password);
			string data = evt.toMsg();
			int ret = client.sendMsg(data);
			if (ret < 0) {
				return;
			}

			bool flag = true;
			string response = client.recvMsg(flag);
			if (!flag) {
				return;
			}
			
			if (strcmp(response.c_str(), "Ok") != 0) {
				cout << "login fail:" <<  response << endl;
				curState = UserStatue::waitName;
			}
			else {
				cout << "login success" << endl;
				curState = UserStatue::Logined;
			}
			
		}break;

		case UserStatue::Logined: {
			string data;
			std::getline(std::cin, data);
			if (data[0] == ':') {
				toUser = data.substr(1);
			}
			else {
				if (toUser == "") {
					cout << "toUser:" << toUser << endl;
				}
				else {
					//cout << "kkk" << endl;
					shared_ptr<ChatEvent> evt = make_shared<ChatEvent>(username, toUser, data);
					push(evt);
				}
			}
		}break;

		default:
			break;
		}
	}
}

void UserAuth::start() {
	int ret = client.connectServer(SERVER_IP, SERVER_PORT);
	if (ret == ERR) {
		return;
	}
	thread sendThread(mem_fn(&UserAuth::startInput), std::ref(*this));
	client.selecting(*this);
	sendThread.join();
	
}