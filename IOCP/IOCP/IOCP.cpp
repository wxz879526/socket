// IOCP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "InitSocket.h"

#define BUFFER_SIZE 1024

typedef struct _PER_HANDLE_DATA
{
	SOCKET s;
	sockaddr_in addr;
} PER_HANDLE_DATA, *PPER_HANDLE_DATA;

typedef struct _PER_IO_DATA
{
	OVERLAPPED ol;
	char buffer[BUFFER_SIZE];
	int nOperationType;
#define OP_READ 1
#define OP_WRITE 2
#define OP_ACCEPT 3
} PER_IO_DATA, *PPER_IO_DATA;

DWORD WINAPI ServerThread(LPVOID lParam);

int main()
{
    std::cout << "Hello World!\n"; 

	CInitSocket setup;

	int port = 4567;
	HANDLE hCompletePort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
		NULL, 0, 0);

	CreateThread(NULL, 0, ServerThread, (LPVOID)hCompletePort, 0, 0);

	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(s, (sockaddr*)&addr, sizeof(addr));
	listen(s, 5);

	while (true)
	{
		sockaddr_in sRemote;
		int nRemote = sizeof(sRemote);

		SOCKET sClient = accept(s, (sockaddr*)&sRemote, &nRemote);
		PPER_HANDLE_DATA pPerHandle = new PER_HANDLE_DATA;
		pPerHandle->s = sClient;
		memcpy(&pPerHandle->addr, (LPVOID)&sRemote, nRemote);

		CreateIoCompletionPort((HANDLE)pPerHandle->s, hCompletePort, (DWORD)pPerHandle, 0);

		PPER_IO_DATA pPerIO = new PER_IO_DATA();
		pPerIO->nOperationType = OP_READ;
		WSABUF buf;
		buf.buf = pPerIO->buffer;
		buf.len = BUFFER_SIZE;
		DWORD dwRecv;
		DWORD dwFlags = 0;
		WSARecv(pPerHandle->s, &buf, 1, &dwRecv, &dwFlags, &pPerIO->ol, NULL);
	}

}

DWORD WINAPI ServerThread(LPVOID lParam)
{
	HANDLE hCompletePort = (HANDLE)lParam;

	DWORD dwTrans;
	PPER_HANDLE_DATA pPerHandle;
	PPER_IO_DATA pPerIO;

	while (true)
	{
		BOOL bOK = GetQueuedCompletionStatus(hCompletePort, &dwTrans, (LPDWORD)&pPerHandle,
			(LPOVERLAPPED*)&pPerIO, WSA_INFINITE);

		if (!bOK)
		{
			closesocket(pPerHandle->s);
			delete pPerHandle;
			delete pPerIO;
			continue;
		}

		if (0 == dwTrans &&(pPerIO->nOperationType == OP_READ || pPerIO->nOperationType == OP_WRITE))
		{
			closesocket(pPerHandle->s);
			delete pPerHandle;
			delete pPerIO;
			continue;
		}

		switch (pPerIO->nOperationType)
		{
		case OP_READ:
		{
			pPerIO->buffer[dwTrans] = '\0';

			WSABUF buf;
			buf.buf = pPerIO->buffer;
			buf.len = BUFFER_SIZE;
			pPerIO->nOperationType = OP_READ;

			DWORD nFlags = 0;
			WSARecv(pPerHandle->s, &buf, 1, &dwTrans, &nFlags, &pPerIO->ol, NULL);
		}
			break;
		default:
			break;
		}
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
