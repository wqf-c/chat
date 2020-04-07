#include "SocketClient.h"
#include"constant.h"
#include <iostream>
#include<thread>
#include <WS2tcpip.h>
#include "EventProcessor.h"

using std::cout;
using std::endl;
using std::thread;


int SocketClient::connectServer(string ip, int port) {
	WSADATA  Ws;
	struct sockaddr_in ServerAddr;
	int ret = 0;
	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
	{
		cout << "Init Windows Socket Failed::" << GetLastError() << endl;
		return ERR;
	}
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		cout << "Create client socket Failed::" << GetLastError << endl;
		return ERR;
	}

	ServerAddr.sin_family = AF_INET;
	//ServerAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	inet_pton(AF_INET, ip.data(), (void*)&ServerAddr.sin_addr.s_addr);
	ServerAddr.sin_port = htons(port);
	memset(ServerAddr.sin_zero, 0x00, 8);
	ret = connect(client, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if (ret == SOCKET_ERROR)
	{
		cout << "Connect Error::" << GetLastError() << endl;
		return -1;
	}
	else
	{
		cout << "连接成功!" << endl;
	}
	return OK;
}

int SocketClient::sendMsg(string &data) {
	if (send(client, data.data(), data.size(), 0) == -1) {
		cout << "Send msg Error::" << GetLastError() << endl;
		return ERR;
	}
	
	return OK;
}

string SocketClient::recvMsg(bool &flag) {
	char buff[BUFFER_SIZE]{};
	memset(buff, 0, BUFFER_SIZE);
	if (recv(client, buff, BUFFER_SIZE - 1, 0) < 0) {
		flag = false;
		cout << "Recv msg Error::" << GetLastError() << endl;
		return "";
	}
	string result{ buff };
	return result;
}

void SocketClient::selecting(EventProcessor &processor) {
	//cout << "aaa";
	while (true)
	{
		//cout << "hello" << endl;
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(client, &fds);
		int ret;
		timeval timeout = timeval{ 1, 0 };
		ret = select(client + 1, &fds, nullptr, nullptr, &timeout);
		if (ret < 0) {
			cout << "select complete" << endl;
			break;
		}
		if (FD_ISSET(client, &fds)) {
			FD_CLR(client, &fds);
			//cout << "recv msg !!!" << endl;
			bool flag = true;
			string rst = recvMsg(flag);
			if (!flag) {
				break;
			}
			else {
				if (rst.empty()) continue;
				switch (rst[0])
				{
				case '1': {
					shared_ptr<ChatEvent> event = ChatEvent::create(rst);
					processor.push(event);
				}break;

				default:
					cout << "recv message:" << rst << endl;
					break;
				}
			}
		}
	}
	shared_ptr<LogoutEvent> logout = make_shared<LogoutEvent>();
	processor.push(logout);
}

void SocketClient::exit() {
	closesocket(client);
	WSACleanup();
}