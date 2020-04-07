#pragma once
#include <winsock2.h>
#include<iostream>
#include"EventProcessor.h"
using std::string;
#pragma comment(lib, "ws2_32.lib")

class SocketClient {
public:

	int connectServer(string ip, int port);

	string recvMsg(bool &flag);

	int sendMsg(string &data);

	void selecting(EventProcessor &processor);

	void exit();

private:
	SOCKET client = INVALID_SOCKET;
};
