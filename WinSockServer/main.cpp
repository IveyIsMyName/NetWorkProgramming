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
		SOCKADDR_IN client_addr;
		int addr_len = sizeof(client_addr);
		SOCKET ClientSocket = accept(ListenSocket, (sockaddr*)&client_addr,&addr_len);
		if (i < MAX_CLIENTS)
		{
			CHAR client_ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
			cout << "New connection from: " << client_ip << ":" << ntohs(client_addr.sin_port) << endl;
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
			}
				closesocket(ClientSocket);
		}
	}
	WaitForMultipleObjects(MAX_CLIENTS, hThreads, TRUE, INFINITE);
	for (int j = 0; j < MAX_CLIENTS; j++)
	{
		if (hThreads[j] != NULL)
		{
			CloseHandle(hThreads[j]);
		}
	}
	closesocket(ListenSocket);
	freeaddrinfo(result);
	WSACleanup();
}

VOID WINAPI HandleClient(SOCKET ClientSocket)
{
	/*SOCKADDR peer;
	int namelen = 0l;
	getsockname(ClientSocket, &peer, &namelen);
	cout << "SAdata:\t" << peer.sa_data << endl;
	cout << "Family:\t" << peer.sa_family << endl;
	cout << "Length:\t" << namelen << endl;*/
	
	//ZeroMemory(&peer, sizeof(peer));
	//6) Зацикливаем Socket на получение соединений от клиентов:
	SOCKADDR_IN peer;
	INT address_length = sizeof(peer);
	getpeername(ClientSocket, (sockaddr*)&peer, &address_length);

	CHAR client_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &peer.sin_addr, client_ip, INET_ADDRSTRLEN);
	//USHORT client_port = ntohs(peer.sin_port);
	INT iResult = 0;
	cout << "CLIENT: " << client_ip << ":" << ntohs(peer.sin_port) << endl;
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH] = {};
	
	int recv_buffer_lenght = DEFAULT_BUFFER_LENGTH;
	do
	{
		ZeroMemory(recvbuffer, size(recvbuffer));
		iResult = recv(ClientSocket, recvbuffer, recv_buffer_lenght, 0);
	
		if (iResult > 0)
		{
			//inet_ntop(AF_INET, &peer.sin_addr, address, INET_ADDRSTRLEN);
			/*cout << "Peer: " << address
				<< (INT)peer.sin_addr.S_un.S_un_b.s_b1 << "."  
				<< (INT)peer.sin_addr.S_un.S_un_b.s_b2 << "."  
				<< (INT)peer.sin_addr.S_un.S_un_b.s_b3 << "."  
				<< (INT)peer.sin_addr.S_un.S_un_b.s_b4  
				<< endl;*/
			cout << "[" << client_ip << ":" << ntohs(peer.sin_port) << "] Bytes received: " << iResult << endl;
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