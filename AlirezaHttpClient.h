#ifndef _ALIREZA_HTTP_CLIENT_H_
#define _ALIREZA_HTTP_CLIENT_H_

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <functional>

#define  _WINSOCK_DEPRECATED_NO_WARNINGS 1
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include "conio.h"

using namespace std;

#pragma comment(lib,"ws2_32.lib")

class AlirezaHttpClient{
private:
	unordered_map<string, string> params;
	string hostName;
	string requestPath;
	int method = 2;
	thread *requestThread;
public:
	static const int POST = 1, GET = 2;
	void sendAsync(string host, function<void(string,string)> onResponse) {
		requestThread = new thread(&AlirezaHttpClient::_send,this,host,onResponse);
	}
	void sendSync(string host,function<void(string,string)> onResponse) {
		return _send(host, onResponse);
	}
	void waitForResult() {
		requestThread->join();
	}
	void setParam(string key, string val) {
		params[key] = val;
	}
	void setMethod(int method) {
		this->method = method;
	}
	void setPath(string path) {
		this->requestPath = path;
	}
	void _send(string host,function<void(string,string)> onResponse) {
		hostName = host;
		WSADATA wsa;
		if (WSAStartup(514, &wsa) != 0) {
			throw std::runtime_error("Failed. Error Code : %d" + WSAGetLastError());
		}
		struct addrinfo *result = NULL, *ptr = NULL, hints;
		SOCKET ConnectSocket = INVALID_SOCKET;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		if (getaddrinfo(host.c_str(), "80", &hints, &result) != 0) {
			throw std::runtime_error("failed to resolve host!");
			return;
		}
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				throw std::runtime_error("creating socket failed with error: " + WSAGetLastError());
				WSACleanup();
				return;
			}
			if (connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			else {
				string request;
				if (method == AlirezaHttpClient::GET) {
					request += "GET ";
					request += requestPath;
					request += '?';
					for (auto r : params) {
						request += r.first;
						request += "=";
						request += r.second;
					}
					request += " HTTP/1.0\r\n";
					request += "Host: ";
					request += hostName;
					request += "\r\n\r\n";
				}
				else if (method == AlirezaHttpClient::POST) {
					request += "POST ";
					request += requestPath;
					request += " HTTP/1.0\r\n";
					string paramsString;
					for (auto r : params) {
						if (paramsString.size()) paramsString += '&';
						paramsString += r.first;
						paramsString += "=";
						paramsString += r.second;
					}
					request += "Host: ";
					request += hostName;
					request += "\r\n";
					request += "Content-Length: ";
					request += to_string(paramsString.size());
					request += "\r\n";
					request += paramsString;
					request += "\r\n";
				}
				else {
					throw runtime_error("bad http method.");
				}
				::send(ConnectSocket, request.c_str(), request.size(), 0);
				char *header = new char[4096];
				int headerLen = ::recv(ConnectSocket, header, 4096, 0);
				header[headerLen] = 0;
				string headerString(header);
				unsigned contentLenPos = headerString.find("Content-Length:");
				if (contentLenPos == string::npos) {
					throw runtime_error("response error");
					return;
				}
				unsigned contentLenEndPos = headerString.find('\n', contentLenPos);
				string ln = headerString.substr(contentLenPos + 15, contentLenEndPos - contentLenPos - 15);
				int blen;
				if (!(ln[0] == '"' || (ln[0] < 58 && ln[0]>47))) blen = 4096; else blen = stoi(ln);
				char *body = new char[blen+1];
				int bodyLen = ::recv(ConnectSocket, body, blen, 0);
				body[bodyLen] = 0;
				onResponse(header,body);
				closesocket(ConnectSocket);
				break;
			}
		}
	}
};

#endif