/*---------------------------------------------------------------------------------
--	SOURCE FILE:	Client.cpp -
--
--	PROGRAM:		Transport Layer Protocol Analyser
--
--	FUNCTIONS:
--					void sendViaUDP(char * hostname, int port, int packetSize, int repetition, HANDLE file, HANDLE logFile)
--					void sendViaTCP(char * hostname, int port, int packetSize, int repetition, HANDLE file, HANDLE logFile)
--
--	DATE:			Feb 14, 2016
--
--	REVISIONS:		Feb 14, 2016
--
--	DESIGNER:		Gabriella Cheung
--
--	PROGRAMMER:		Gabriella Cheung
--
--	NOTES:
--	This file contains the code for the client part of the application. When the user
--  starts a data transfer to a server, DialogProc (from Main.cpp) will call either
--  sendViaUDP or sendViaTCP to send data to a server.
--
---------------------------------------------------------------------------------*/
#include "resource.h"

/*---------------------------------------------------------------------------------
--	FUNCTION: sendViaUDP
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void sendViaUDP(char * hostname, int port, int packetSize, int repetition, HANDLE file, HANDLE logFile)
--
--	PARAMETERS:	char *hostname - hostname of server
--				int port - port of server
--				int packetSize - size of packet to send
--				int repetition - number of packets to send
--				HANDLE file - handle for file for data to read from
--				HANDLE logFile - handle for client log file.
--
--	RETURNS:	void
--
--	NOTES:
--	This function handles the process of sending packets to the server using UDP.
--  First it creates a UDP socket, then in a loop it gets the data from the getData 
--  method and sends the packet using the WSASendTo method until all the packets 
--  has been sent. Finally it prints out the details of the data transfer to the
--	 screen before closing the socket.
--
---------------------------------------------------------------------------------*/
void sendViaUDP(char * hostname, int port, int packetSize, int repetition, HANDLE file, HANDLE logFile)
{
	int err, server_len;
	SOCKET sd = INVALID_SOCKET;
	struct hostent	*hp;
	struct sockaddr_in server;
	SYSTEMTIME stStartTime, stEndTime;
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	char *sbuf;
	HANDLE hFile = NULL, hLogFile;
	char message[256];

	int sentCount = 0;
	hFile = file;
	hLogFile = logFile;

	sprintf(message, "Sending %d byte packets %d times to %s port %d using UDP",
		packetSize,
		repetition,
		hostname,
		port);
	writeToScreen(message);
	strcat(message, "\r\n");
	writeToFile(hLogFile, message);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) //No usable DLL
	{
		writeToScreen("DLL not found!");
		return;
	}

	// Create the socket
	if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		writeToScreen("Cannot create socket");
		return;
	}

	sbuf = (char*)malloc(packetSize);

	// Store server's information
	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if ((hp = gethostbyname(hostname)) == NULL) //async?
	{
		writeToScreen("Can't get server's IP address");
		return;
	}

	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	// transmit data
	server_len = sizeof(server);
	GetSystemTime(&stStartTime);
	for (int sent = 0; sent < repetition; sent++)
	{
		//get data
		getData(hFile, sbuf, packetSize);
		int length = strlen(sbuf);
	    length = length > packetSize ? packetSize : length;
		if (sendto(sd, sbuf, length, 0, (struct sockaddr *)&server, server_len) == -1)
		{
			sprintf(message, "error: %d", WSAGetLastError());
			writeToScreen(message);
		}
		sentCount++;
	}
	//close file
	if (hFile != NULL)
	{
		closeFile(hFile);
	}
	sprintf(message, "%d %d byte datagrams were sent to server", sentCount, packetSize);
	writeToScreen(message);
	strcat(message, "\r\n");
	writeToFile(hLogFile, message);
	GetSystemTime(&stEndTime);
	sprintf(message, "Start time: %d-%02d-%02d %02d:%02d:%02d:%03d",
		stStartTime.wYear,
		stStartTime.wMonth,
		stStartTime.wDay,
		stStartTime.wHour,
		stStartTime.wMinute,
		stStartTime.wSecond,
		stStartTime.wMilliseconds);
	writeToScreen(message);
	strcat(message, "\r\n");
	writeToFile(hLogFile, message);
	sprintf(message, "End time: %d-%02d-%02d %02d:%02d:%02d:%03d",
		stEndTime.wYear,
		stEndTime.wMonth,
		stEndTime.wDay,
		stEndTime.wHour,
		stEndTime.wMinute,
		stEndTime.wSecond,
		stEndTime.wMilliseconds);
	writeToScreen(message);
	strcat(message, "\r\n\r\n");
	writeToFile(hLogFile, message);
	closesocket(sd);
	WSACleanup();
}

/*---------------------------------------------------------------------------------
--	FUNCTION: sendViaTCP
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void sendViaTCP(char * hostname, int port, int packetSize, int repetition, HANDLE file, HANDLE logFile)
--
--	PARAMETERS:	char *hostname - hostname of server
--				int port - port of server
--				int packetSize - size of packet to send
--				int repetition - number of packets to send
--				HANDLE file - handle for file for data to read from
--				HANDLE logFile - handle for client log file.
--
--	RETURNS:	void
--
--	NOTES:
--	This function handles the process of sending packets to the server using TCP.
--  First it creates a TCP socket, then it tries to establish a connection with
--  the server. If the connection was established successfully, it goes in a loop
--  where it gets the data from the getData method and sends the packet using the
--  WSASend method until all the packets to be sent has been sent. Finally it prints
--  out the details of the data transfer to the screen before closing the socket.
--
---------------------------------------------------------------------------------*/
void sendViaTCP(char * hostname, int port, int packetSize, int repetition, HANDLE file, HANDLE logFile)
{
	int err, server_len;
	SOCKET sd = INVALID_SOCKET;
	struct hostent	*hp;
	struct sockaddr_in server;
	SYSTEMTIME stStartTime = {0}, stEndTime = { 0 };
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	char *sbuf;
	HANDLE hFile = NULL, hLogFile;
	char message[256];

	hFile = file;
	hLogFile = logFile;
	
	sprintf(message, "Sending %d byte packets %d times to %s port %d using TCP",
		packetSize,
		repetition,
		hostname,
		port);
	writeToScreen(message);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) //No usable DLL
	{
		writeToScreen("DLL not found!");
		return;
	}

	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		writeToScreen("Cannot create socket");
		return;
	}

	sbuf = (char*)malloc(packetSize);

	// Store server's information
	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if ((hp = gethostbyname(hostname)) == NULL) //async?
	{
		writeToScreen("Can't get server's IP address");
		return;
	}

	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		writeToScreen("Can't connect to server");
		return;
	}

	// transmit data
	server_len = sizeof(server);
	GetSystemTime(&stStartTime);
	int sent;
	for (sent = 0; sent < repetition; sent++)
	{
		//get data
		if (stStartTime.wYear == 0)
		{
			GetSystemTime(&stStartTime);
		}
		getData(hFile, sbuf, packetSize);
		if (send(sd, sbuf, strlen(sbuf), 0) == -1)
		{
			sprintf(message, "error: %d", WSAGetLastError());
			writeToScreen(message);
		}
	}
	GetSystemTime(&stEndTime);
	sprintf(message, "%d %d byte datagrams were sent to server", sent, packetSize);
	writeToScreen(message);
	strcat(message, "\r\n");
	writeToFile(hLogFile, message);
	sprintf(message, "Start time: %d-%02d-%02d %02d:%02d:%02d:%03d",
		stStartTime.wYear,
		stStartTime.wMonth,
		stStartTime.wDay,
		stStartTime.wHour,
		stStartTime.wMinute,
		stStartTime.wSecond,
		stStartTime.wMilliseconds);
	writeToScreen(message);
	strcat(message, "\r\n");
	writeToFile(hLogFile, message);
	sprintf(message, "End time: %d-%02d-%02d %02d:%02d:%02d:%03d",
		stEndTime.wYear,
		stEndTime.wMonth,
		stEndTime.wDay,
		stEndTime.wHour,
		stEndTime.wMinute,
		stEndTime.wSecond,
		stEndTime.wMilliseconds);
	writeToScreen(message);
	strcat(message, "\r\n\r\n");
	writeToFile(hLogFile, message);
	//close file
	if (hFile != NULL)
	{
		closeFile(hFile);
	}
	closesocket(sd);
	WSACleanup();
}