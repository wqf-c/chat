#include<iostream>
#include<string>
#include<sstream>

using namespace std;

void maintest() {
	stringstream ss{ "0|123|456" };
	//char s[] = "0|123|456";
	char buf[1024];
	ss.getline(buf, 1024, '|');
	cout << buf << endl;
	ss.getline(buf, 1024, '|');
	cout << buf << endl;

}