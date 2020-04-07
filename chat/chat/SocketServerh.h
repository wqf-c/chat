#pragma once
#include <winsock2.h>
#include<iostream>
#include<map>
#pragma comment(lib, "ws2_32.lib")
using std::map;

using std::string;

class SocketServer {
public:
	int sendMsg(SOCKET client, string data);
	string recvMsg(SOCKET client, bool &flag);
	void start();
	void selecting();

private:
	SOCKET server{ INVALID_SOCKET };
	map<string, string> database{ {"wqf", "123"}, {"wqf1", "456"} };
	map<SOCKET, string> sockUser;
};