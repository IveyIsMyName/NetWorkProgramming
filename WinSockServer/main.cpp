#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>
#include<iostream>
using namespace std;

#pragma comment(lib, "WS2_32.lib")

#define DEFAULT_PORT			"27015"
#define DEFAULT_BUFFER_LENGTH	1500 
#define SZ_SORRY	"Sorry, but all is busy"

void main()
{
	setlocale(LC_ALL, "");
	//1) Инициалищация WinSock:
	WSAData wsaData;

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
				
	{
		cout << "Error: WSAstartup failed: " << iResult << endl;
		return;
	}

	//2) Получаем IP-адреса, на которых можно запустить socket:
	//2) Проверяем, не занят ли порт, на котором мы хотим запустить свой Сервер
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //TCP/IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	addrinfo* result = NULL;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		WSACleanup();
		cout << "ERROR: getaddressinfo failed: " << iResult << endl;
		return;
	}
	cout << hints.ai_addr << endl;


	//3) Создаем socket, который будет прослушивать Сервер:
	SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET)
	{
		cout << "ERROR: Socket creation failed: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//4) Связываем socket с сетевой картой, которую он будет прослушивать:
	//strcpy_s(result->ai_addr->sa_data ,"127.0.0.1");
	iResult = bind(ListenSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Error: binding failed with code: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	//5) Запускаем Socket:
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		cout << "ERROR: listening failed with code: " << WSAGetLastError() << endl;
		closesocket(ListenSocket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	VOID WINAPI HandleClient(SOCKET ClientSocket);
	CONST INT MAX_CLIENTS = 3;
	SOCKET clients[MAX_CLIENTS] = {};
	DWORD dwThreadIDs[MAX_CLIENTS] = {};
	HANDLE hThreads[MAX_CLIENTS] = {};

	INT i = 0;

	while (true)
	{
		SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
		if (i < MAX_CLIENTS)
		{
			//HandleClient(ClientSocket);
			clients[i] = ClientSocket;
			hThreads[i] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)HandleClient,
				(LPVOID)clients[i],
				0,
				&dwThreadIDs[i]
			);
			i++;
		}
		else
		{
			CHAR recieve_buffer[DEFAULT_BUFFER_LENGTH] = {};
			INT iResult = recv(ClientSocket, recieve_buffer, DEFAULT_BUFFER_LENGTH, 0);
			if (iResult > 0)
			{
				cout << "Bytes receieved: " << iResult << endl;
				cout << "Message: " << recieve_buffer << endl;
				//CONST CHAR SZ_SORRY[] = "Sorry, but all is busy";
				INT iSendResult = send(ClientSocket, SZ_SORRY, strlen(SZ_SORRY), 0);
				closesocket(ClientSocket);
			}
		}
	}
	WaitForMultipleObjects(MAX_CLIENTS, hThreads, TRUE, INFINITE);
	closesocket(ListenSocket);
	freeaddrinfo(result);
	WSACleanup();
}

VOID WINAPI HandleClient(SOCKET ClientSocket)
{
	//6) Зацикливаем Socket на получение соединений от клиентов:
	INT iResult = 0;
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH] = {};
	int recv_buffer_lenght = DEFAULT_BUFFER_LENGTH;
	do
	{
		ZeroMemory(recvbuffer, size(recvbuffer));
		iResult = recv(ClientSocket, recvbuffer, recv_buffer_lenght, 0);
		if (iResult > 0)
		{
			cout << "Bytes received: " << iResult << endl;
			CHAR sz_response[] = "Hello, I am Server! Nice to meet you!";
			cout << "Message: " << recvbuffer << endl;
			//INT iSendResult = send(ClientSocket, sz_response,sizeof(sz_response), 0);
			INT iSendResult = send(ClientSocket, recvbuffer, strlen(recvbuffer), 0);
			if (iSendResult == SOCKET_ERROR)
			{
				cout << "Error: Send failed with code: " << WSAGetLastError() << endl;
				//closesocket(ClientSocket);
				//closesocket(ListenSocket);
				//freeaddrinfo(result);
				//WSACleanup();
				//return;
				break; // Выходим из цикла, но не закрываем сразу
			}
			cout << "Bytes sent: " << iSendResult << endl;
		}
		else if (iResult == 0)
		{
			cout << "Connection closing" << endl;
			closesocket(ClientSocket);
		}
		else
		{
			cout << "ERROR: recv() failed with code: " << WSAGetLastError() << endl;
			//closesocket(ClientSocket);
			//closesocket(ListenSocket);
			//freeaddrinfo(result);
			//WSACleanup();
			//return;
			break;
		}
	} while (iResult > 0);
}