//This code was apadptded from the msdn guide on win sockets
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "27015"

//prints out a char bufffer till it hits a new line character
void printNewLineString(char charbuf[])
{
	int i = 0;
	while (charbuf[i] != '\n')
	{
		printf("%c", charbuf[i]);
		i++;
	}
	printf("\n");
}

//prints out a char buffer from start till lenght
void printFixedLength(char charbuf[], int length)
{
	int i = 0;
	for (i = 0; i < length; i++)
	{
		printf("%c", charbuf[i]);
	}
}

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char sendbuf[DEFAULT_BUFLEN];
	char sendbuf2[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	USHORT numID;
	char welcomeMessage[8] = "Welcome\n";
	char successMessage[8] = "Success\n";
	char failureMessage[8] = "Failure\n";
	char tempBuf[DEFAULT_BUFLEN];
	int tempI = 0;
	int tempI2 = 0;
	UINT16 networdByteOrder;
	//Client starts with the command line arguments of server name and port number

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	//Client connects to server
	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	//Client waits for Welcome message from Server

	//Get Server Welcome Message
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (iResult == 0)
	{
		printf("recv error");
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	//Checks Welcome message against expected.
	//If it does not match the client closes connection, prints error and exits
	if (memcmp(welcomeMessage, recvbuf, 8) == 0)
	{
		printf("got Welcome Message\n");
	}
	else
	{
		printf("not welcome message");
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	//Client prompts user for ID number and name
	printf("Please enter your ID number and name\n");

	scanf("%s %s", sendbuf, sendbuf2);

	tempI = strlen(sendbuf);

	sendbuf[tempI] = '\n';

	tempI2 = strlen(sendbuf2);

	sendbuf2[tempI2] = '\n';

	//Client sends ID and name as two new line terminated messages
	//send null terminated string for id
	iResult = send(ConnectSocket, sendbuf, tempI + 1, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	//send null terminated string for name
	iResult = send(ConnectSocket, sendbuf2, tempI2  +1 , 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	
	//Client waits for server response.If client gets message “Failure”
	//the client closes the connection and starts from connect to server
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (iResult == 0)
	{
		printf("Recv error");
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	
	if (memcmp(successMessage, recvbuf, 8) != 0)
	{
		if (memcmp(failureMessage, recvbuf, 8) == 0)
		{
			printf("recvied failure message\n");
		}
		else
		{
			printf("unknown message received");
		}
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	//If client gets “Success” will prompt user for a password

	printf("Please enter your password:\n");
	scanf("%s", sendbuf, sendbuf2);

	tempI = strlen(sendbuf) -1;

	networdByteOrder = htons(tempI);

	//Client then sends server the length of the password as a two byte binary number in network byte order.The client will then send the password.
	
	//send password length
	iResult = send(ConnectSocket, &networdByteOrder, sizeof(UINT16), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	//send password
	iResult = send(ConnectSocket, sendbuf, tempI, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	//The client gets revc end message
	//Client first get lenght of message
	iResult = recv(ConnectSocket, &networdByteOrder, sizeof(UINT16), 0);
	if (iResult == 0)
	{
		printf("Recv error");
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	networdByteOrder = ntohs(networdByteOrder);
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (iResult == 0)
	{
		printf("Recv error");
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	printFixedLength(recvbuf, networdByteOrder);
	closesocket(ConnectSocket);
	WSACleanup();	
	return 0;
}
