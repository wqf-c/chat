#include"UserAuth.h"
#include"SocketServerh.h"
#include<iostream>
using namespace std;

int main(int argc, char *argv[]) {
	cout << "argc :" << argc << endl;
	if (argc != 2) {
		cout << "input error" << endl;
		return -1;
	}
	if (strcmp(argv[1], "0") == 0) {
		SocketServer server;
		server.start();
	}
	else if (strcmp(argv[1], "1") == 0) {
		UserAuth user;
		user.start();
	}
	else {
		cout << "input error" << endl;
		return -1;
	}
	return 0;
}