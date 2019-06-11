#pragma once
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32")

class CInitSocket final
{
public:
	CInitSocket(BYTE minor = 2, BYTE major = 2)
	{
		WSADATA wsaData;
		WORD ver = MAKEWORD(minor, major);
		if (WSAStartup(ver, &wsaData) != 0)
		{
			exit(-1);
		}
	}

	~CInitSocket()
	{
		WSACleanup();
	}
};