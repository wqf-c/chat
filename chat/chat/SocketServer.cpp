#include"constant.h"
#include"SocketServerh.h"
#include"EventProcessor.h"
#include <iostream>
#include <WS2tcpip.h>
#include<sstream>
#include<thread>
#include<memory>
#include<string.h>

using std::cout;
using std::endl;
using std::thread;
using std::mem_fn;
using std::stringstream;
using std::shared_ptr;

int SocketServer::sendMsg(SOCKET client, string data) {
	if (send(client, data.data(), data.size(), 0) == -1) {
		cout << "Send msg Error::" << GetLastError() << endl;
		return ERR;
	}

	return OK;
}

string SocketServer::recvMsg(SOCKET client, bool &flag) {
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

void SocketServer::start() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "init fail" << endl;
		return;
	}


	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	sockaddr.sin_port = htons(SERVER_PORT);

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server == INVALID_SOCKET) {
		cout << "create socket fail " << endl;
		WSACleanup();
		return;
	}

	if (bind(server, (SOCKADDR*)& sockaddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "Ì×½Ó×Ö°ó¶¨Ê§°Ü£¡" << endl;
		WSACleanup();
	}
	else {
		cout << "Ì×½Ó×Ö°ó¶¨³É¹¦£¡" << endl;
	}
	//ÉèÖÃÌ×½Ó×ÖÎª¼àÌý×´Ì¬
	if (listen(server, SOMAXCONN) < 0) {
		cout << "ÉèÖÃ¼àÌý×´Ì¬Ê§°Ü£¡" << endl;
		WSACleanup();
	}
	else {
		cout << "ÉèÖÃ¼àÌý×´Ì¬³É¹¦£¡" << endl;
	}
	cout << "·þÎñ¶ËÕýÔÚ¼àÌýÁ¬½Ó" << endl;

	thread t(mem_fn(&SocketServer::selecting), this);
	t.join();
}

void SocketServer::selecting() {
	while (true)
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(server, &fds);
		int maxfd = server;
		map<SOCKET, string>::iterator iter;
		iter = sockUser.begin();
		while (iter != sockUser.end()) {
			FD_SET(iter->first, &fds);
			if (iter->first > maxfd) maxfd = iter->first;
			iter++;
		}
		timeval *timeout = new timeval{1, 0};
		int ret = select(maxfd + 1, &fds, nullptr, nullptr, timeout);
		if (ret < 0) {
			cout << "select error:" << GetLastError() << endl;
			break;
		}
		for (int i = 0; i < fds.fd_count; ++i) {
			SOCKET sock = fds.fd_array[i];
			if (sock == server) {
				SOCKADDR_IN accept_addr;
				int len = sizeof(SOCKADDR);;
				SOCKET client = accept(server, (sockaddr *)&accept_addr, &len);
				if (client == SOCKET_ERROR) {
					cout << "accept fail" << endl;
				}
				else {
					sockUser[client] = "";
				}
			}
			else {
				bool flag = true;
				string data = recvMsg(sock, flag);
				if (flag) {
					if (!data.empty() && data[0] == '0') {
						stringstream ss{ data };
						shared_ptr<LoginEvent> evt = LoginEvent::create(data);
						cout << evt->username << "  login" << endl;
						sockUser[sock] = evt->username;
						map<string, string>::iterator iter = database.find(evt->username);
						if (iter == database.end() || iter->second != evt->password) {
							sendMsg(sock, "login fail");
						}
						else {
							sendMsg(sock, "Ok");
						}
					}
					else if (!data.empty() && data[0] == '1') {
						bool findToFlag = false;
						shared_ptr<ChatEvent> evt =ChatEvent::create(data);
						map<SOCKET, string>::iterator iter;
						iter = sockUser.find(sock);
						if (iter == sockUser.end() || iter->second != evt->fromUser) {
							cout << evt->fromUser << "  have not login" << endl;
							sendMsg(sock, "service Info:" + evt->fromUser + "have not login");
						}
						else {
							
							iter = sockUser.begin();
							while (iter != sockUser.end()) {
								if (iter->second == evt->toUsr) {
									cout << "find toUser: " << evt->toUsr << endl;
									string data = evt->toMsg();
									sendMsg(iter->first, data);
									findToFlag = true;
									
									break;
								}
								iter++;
							}
							cout << findToFlag << endl;
							if (!findToFlag) {
								cout << "not find" << endl;
								sendMsg(sock, "service Info: " + evt->toUsr + " have not login");
							}
						}
						
					}
					else {
						cout << "recv unknown data: " << data;
					}
				}
			}
		}
	}
	closesocket(server);
	WSACleanup();
}