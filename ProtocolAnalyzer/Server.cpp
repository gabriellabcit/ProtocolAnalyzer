/*---------------------------------------------------------------------------------
--	SOURCE FILE:	Server.cpp -
--
--	PROGRAM:		Transport Layer Protocol Analyser
--
--	FUNCTIONS:
--					DWORD WINAPI startUDPServer(LPVOID)
--					void CALLBACK udpRoutine(DWORD, DWORD, LPOVERLAPPED, DWORD)
--					DWORD WINAPI udpThread(LPVOID)
--					DWORD WINAPI startTCPServer(LPVOID)
--					void CALLBACK tcpRoutine(DWORD, DWORD, LPOVERLAPPED, DWORD)
--					DWORD WINAPI tcpThread(LPVOID)
--					long delay(SYSTEMTIME, SYSTEMTIME)
--					void displayStats(TRANSFER_STATS *)
--					void startServer(int udpPort, int tcpPort, HANDLE hFile)
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
--	This file contains the code for the server part of the application. When the user
--  selects to run the application in server mode, the startServer method is called.
--  Two threads are created - one to run the UDP server and the other to run the
--  TCP server. Those two threads continue to run until a signal is received to
--  stop the server.
--
--  While the server is running, it will continue to display statistics obtained
--  from the data transfers onto the screen.
--
---------------------------------------------------------------------------------*/
#include "resource.h"

DWORD WINAPI startUDPServer(LPVOID);
void CALLBACK udpRoutine(DWORD, DWORD, LPOVERLAPPED, DWORD);
DWORD WINAPI udpThread(LPVOID);
DWORD WINAPI startTCPServer(LPVOID);
void CALLBACK tcpRoutine(DWORD, DWORD, LPOVERLAPPED, DWORD);
DWORD WINAPI tcpThread(LPVOID);
long delay(SYSTEMTIME, SYSTEMTIME);
void displayStats(TRANSFER_STATS *);

SOCKET udpSocket, tcpSocket, tcpAcceptSocket;
TRANSFER_STATS *tcpStats, *udpStats;
BOOL serverRunning = false;
int uPort, tPort;
HANDLE hWriteFile, hServerLogFile;
WSAEVENT udpEvent, tcpEvent;

/*---------------------------------------------------------------------------------
--	FUNCTION: startServer
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void startServer(int udpPort, int tcpPort, HANDLE hFile)
--
--	PARAMETERS:	int udpPort - port of UDP server as specified by user
--				int tcpPort - port of TCP server as specified by user
--				HANDLE hFile - handle for file to read data from
--
--	RETURNS:	void
--
--	NOTES:
--	This function starts the server. First it initializes the Winsock 2.2 DLL, then
--  it creates two threads, one for UDP and one for TCP. The rest of the work is
--  done by the two methods: startUDPServer and startTCPServer.
--
---------------------------------------------------------------------------------*/
void startServer(int udpPort, int tcpPort, HANDLE hFile)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	int error;
	HANDLE udpThreadHandle, tcpThreadHandle;
	DWORD udpThreadId, tcpThreadId;
	char message[256];

	hWriteFile = hFile;

	hServerLogFile = openFile("ServerLog.txt", false);

	// Initialize the DLL with version Winsock 2.2
	error = WSAStartup(wVersionRequested, &wsaData);
	if (error != 0) //No usable DLL
	{
		writeToScreen("DLL not found!");
		return;
	}
	serverRunning = true;
	sprintf(message, "Starting UDP Server using port %d", udpPort);
	writeToScreen(message);
	sprintf(message, "Starting TCP Server using port %d", tcpPort);
	writeToScreen(message);

	uPort = udpPort;
	tPort = tcpPort;
	if ((tcpThreadHandle = CreateThread(NULL, 0, startTCPServer, (LPVOID)0, 0, &tcpThreadId)) == NULL)
	{
		writeToScreen("TCP server initialization failed");
		return;
	}
	if ((udpThreadHandle = CreateThread(NULL, 0, startUDPServer, (LPVOID)0, 0, &udpThreadId)) == NULL)
	{
		writeToScreen("UDP server initialization failed");
		return;
	}
}

/*---------------------------------------------------------------------------------
--	FUNCTION: startTCPServer
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	DWORD WINAPI startTCPServer(LPVOID n)
--
--	PARAMETERS:	LPVOID n
--
--	RETURNS:	DWORD
--
--	NOTES:
--	This function starts the TCP server. It creates a TCP socket, then binds to it.
--  Then it calls listen so the socket will be listening to any incoming connection
--  requests. It then creates a WSAEvent for the WSAWaitForMultipleEvents call in
--  the tcpThread method. It creates a thread to run the tcpThread method. Finally
--  it waits for and accepts incoming connection requests.
--
---------------------------------------------------------------------------------*/
DWORD WINAPI startTCPServer(LPVOID n)
{
	struct	sockaddr_in tcpServer;

	LPSOCKET_INFORMATION tcpSocketInfo;
	DWORD flags, threadId;
	int error = 0;
	char message[256];
	HANDLE threadHandle;

	// Create a stream socket
	if ((tcpSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		writeToScreen("Can't create a socket");
		///exit(1);
	}

	// Bind an address to the socket
	memset((char *)&tcpServer, 0, sizeof(tcpServer));
	tcpServer.sin_family = AF_INET;
	tcpServer.sin_port = htons(tPort);
	tcpServer.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(tcpSocket, (struct sockaddr *)&tcpServer, sizeof(tcpServer)) == SOCKET_ERROR)
	{
		writeToScreen("Can't bind name to socket");
	}

	// Create a socket information structure to associate with socket.
	if ((tcpSocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		sprintf(message,"GlobalAlloc() failed with error %d\n", GetLastError());
		writeToScreen(message);
		return FALSE;
	}

	if (listen(tcpSocket, 5) == SOCKET_ERROR)
	{
		writeToScreen("listen failed");
	}

	if ((tcpEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		writeToScreen("WSACreateEvent() failed");
	}

	if ((threadHandle = CreateThread(NULL, 0, tcpThread, (LPVOID)tcpEvent, 0, &threadId)) == NULL)
	{
		writeToScreen("CreateThread() failed");
	}

	//initialize stats struct
	tcpStats = (TRANSFER_STATS*)malloc(sizeof(TRANSFER_STATS));
	tcpStats->protocol = "TCP";
	tcpStats->startTime = { 0 };
	tcpStats->endTime = { 0 };
	tcpStats->packetCount = 0;
	tcpStats->packetSize = 0;
	tcpStats->totalSize = 0;

	while (serverRunning)
	{
		tcpAcceptSocket = accept(tcpSocket, NULL, NULL);

		if (WSASetEvent(tcpEvent) == FALSE)
		{
			sprintf(message,"WSASetEvent failed with error %d\n", WSAGetLastError());
			writeToScreen(message);
			ExitThread(0);
		}
	}
	ExitThread(0);
}

/*---------------------------------------------------------------------------------
--	FUNCTION: tcpThread
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	DWORD WINAPI tcpThread(LPVOID lpParameter)
--
--	PARAMETERS:	LPVOID lpParameter - WSAEvent from startTCPServer
--
--	RETURNS:	DWORD
--
--	NOTES:
--	This function calls WSAWaitForMultipleEvents. When it receives an event,
--  it creates a socketInfo data structure and calls WSARecv to read data from
--  the socket into socketInfo. When WSARecv has read data successfully, a
--  completion routine is called.
--
---------------------------------------------------------------------------------*/
DWORD WINAPI tcpThread(LPVOID lpParameter)
{
	LPSOCKET_INFORMATION socketInfo;
	WSAEVENT eventArray[1];
	DWORD index, flags;
	int error = 0;
	char message[256];
	SOCKADDR_IN	client;
	int clientSize = sizeof(SOCKADDR_IN);

	eventArray[0] = (WSAEVENT)lpParameter;
	while (true)
	{
		while (true)
		{
			index = WSAWaitForMultipleEvents(1, eventArray, FALSE, WSA_INFINITE, TRUE);

			if (index == WSA_WAIT_FAILED)
			{
				writeToScreen("WSAWaitForMultipleEvents failed");
				break;
			}
			if (index != WAIT_IO_COMPLETION)
			{
				break;
			}
		}

		WSAResetEvent(eventArray[index - WSA_WAIT_EVENT_0]);

		// Create a socket information structure to associate with socket.
		if ((socketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			sprintf(message,"GlobalAlloc() failed with error %d\n", GetLastError());
			writeToScreen(message);
			return FALSE;
		}

		// Fill in the details of our accepted socket.
		socketInfo->Socket = tcpAcceptSocket;
		ZeroMemory(&(socketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		socketInfo->BytesSEND = 0;
		socketInfo->BytesRECV = 0;
		socketInfo->DataBuf.len = DATA_BUFSIZE;
		socketInfo->DataBuf.buf = socketInfo->Buffer;
		socketInfo->Timeout = INFINITE;

		flags = 0;
		if (WSARecv(socketInfo->Socket, &(socketInfo->DataBuf), 1, NULL, &flags, &(socketInfo->Overlapped), tcpRoutine) == SOCKET_ERROR)
		{
			if ((error = WSAGetLastError()) != WSA_IO_PENDING)
			{
				sprintf(message, "WSARecv failed with error %d", error);
				writeToScreen(message);
			}
		}
	}
}

/*---------------------------------------------------------------------------------
--	FUNCTION: tcpRoutine
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void CALLBACK tcpRoutine(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped, DWORD flags)
--
--	PARAMETERS:	DWORD errorCode - errorCode from WSARecv call
--				DWORD bytesTransferred - the number of bytes read by WSARecv
--				LPOVERLAPPED overlapped - the overlapped structure that contains
--											the data read
--				DWORD flags - flags used for WSARecv call
--
--	RETURNS:	none
--
--	NOTES:
--	This function is called when WSARecv successfully reads data from the TCP
--	socket. It updates the statistics and writes the data read to file (if user
--  specified a file to save to). If there were no bytes transferred, the statistics
--  are printed to the screen and reset. It then calls WSARecv so the server will
--  be ready to read data when it arrives.
--
---------------------------------------------------------------------------------*/
void CALLBACK tcpRoutine(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped, DWORD flags)
{
	LPSOCKET_INFORMATION socketInfo = (LPSOCKET_INFORMATION)overlapped;
	DWORD newFlags;
	char data[MAXLEN] = { 0 };
	int error = 0;
	char message[256];

	if (errorCode != 0)
	{
		writeToScreen("TCP recv error");
	}

	if (bytesTransferred > 0)
	{
		if (tcpStats->startTime.wYear == 0) //start time was never set
		{
			GetSystemTime(&(tcpStats->startTime));
		}
		GetSystemTime(&(tcpStats->endTime));

		if (hWriteFile != NULL)
		{
			if (writeToFile(hWriteFile, socketInfo->DataBuf.buf))
			{
				//writeToScreen("Incoming data successfully saved", true);
			}
			else {
				writeToScreen("Saving incoming data failed");
			}
		}
		tcpStats->packetCount++;
		tcpStats->totalSize += bytesTransferred;
	}
	else {
		displayStats(tcpStats);
		//reset stats
		tcpStats->startTime = { 0 };
		tcpStats->endTime = { 0 };
		tcpStats->packetCount = 0;
		tcpStats->packetSize = 0;
		tcpStats->totalSize = 0;
		return;
	}
	newFlags = 0;
	if (WSARecv(socketInfo->Socket, &(socketInfo->DataBuf), 1, NULL, &newFlags, &(socketInfo->Overlapped), tcpRoutine) == SOCKET_ERROR)
	{
		if ((error = WSAGetLastError()) != WSA_IO_PENDING)
		{
			sprintf(message, "WSARecv failed with error %d", error);
			writeToScreen(message);
		}
	}
}

/*---------------------------------------------------------------------------------
--	FUNCTION: startUDPServer
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	DWORD WINAPI startUDPServer(LPVOID n)
--
--	PARAMETERS:	LPVOID n
--
--	RETURNS:	DWORD
--
--	NOTES:
--	This function starts the UDP server. It creates a UDP socket, then binds to it.
--  It then creates a WSAEvent for the WSAWaitForMultipleEvents call in
--  the udpThread method. It creates a thread to run the udpThread method. Finally
--  it calls the select method to set a very large timeout on the socket. When a 
--  datagram arrives at the socket, the timeout is set to 1 second as a way to 
--  signal when the datagrams stop arriving. When the socket times out, it prints
--	out the statistics collected and the timeout is reset to the original timeout.
--
---------------------------------------------------------------------------------*/
DWORD WINAPI startUDPServer(LPVOID n)
{
	struct	sockaddr_in udpServer;
	DWORD threadId;
	int error = 0, selectRet;
	char buf[MAXLEN];
	HANDLE threadHandle;
	fd_set fds;
	struct timeval tv;

	// Create a datagram socket
	if ((udpSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		writeToScreen("Can't create a socket");
	}

	// Bind an address to the socket
	memset((char *)&udpServer, 0, sizeof(udpServer));
	udpServer.sin_family = AF_INET;
	udpServer.sin_port = htons(uPort);
	udpServer.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(udpSocket, (struct sockaddr *)&udpServer, sizeof(udpServer)) == SOCKET_ERROR)
	{
		writeToScreen("Can't bind name to socket");
	}

	if ((udpEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		writeToScreen("WSACreateEvent() failed");
	}
	
	if ((threadHandle = CreateThread(NULL, 0, udpThread, (LPVOID)udpEvent, 0, &threadId)) == NULL)
	{

	}

	//initialize stats struct
	udpStats = (TRANSFER_STATS*)malloc(sizeof(TRANSFER_STATS));
	udpStats->protocol = "UDP";
	udpStats->startTime = { 0 };
	udpStats->endTime = { 0 };
	udpStats->packetCount = 0;
	udpStats->packetSize = 0;
	udpStats->totalSize = 0;

	char message[256];

	FD_ZERO(&fds);
	FD_SET(udpSocket, &fds);

	tv.tv_sec = 36000000;
	tv.tv_usec = 0;

	while (true)
	{
		selectRet = select(udpSocket, &fds, NULL, NULL, &tv);
		if (serverRunning)
		{
			if (selectRet == SOCKET_ERROR)
			{
				if ((error = WSAGetLastError()) != WSA_IO_PENDING)
				{
					sprintf(message, "select failed with error %d", error);
					writeToScreen(message);
					break;
				}
			}
			else if (selectRet == 0)
			{
				displayStats(udpStats);
				tv.tv_sec = 36000000;
				FD_ZERO(&fds);
				FD_SET(udpSocket, &fds);

				//reset stats
				udpStats->startTime = { 0 };
				udpStats->endTime = { 0 };
				udpStats->packetCount = 0;
				udpStats->packetSize = 0;
				udpStats->totalSize = 0;
				if (WSASetEvent(udpEvent) == FALSE)
				{
					writeToScreen("Resetting event failed");
				}
			}
			else {
				if (WSASetEvent(udpEvent) == FALSE)
				{
					writeToScreen("Resetting event failed");
				}
				tv.tv_sec = 1;
			}
		}
		else {
			break;
		}
		
	}
	//return 0;
	ExitThread(0);
}

/*---------------------------------------------------------------------------------
--	FUNCTION: udpThread
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	DWORD WINAPI udpThread(LPVOID lpParameter)
--
--	PARAMETERS:	LPVOID lpParameter - WSAEvent from startUDPServer
--
--	RETURNS:	DWORD
--
--	NOTES:
--	This function calls WSAWaitForMultipleEvents. When it receives an event,
--  it creates a socketInfo data structure and calls WSARecvFrom to read data from
--  the socket into socketInfo. When WSARecvFrom has read data successfully, a
--  completion routine is called.
--
---------------------------------------------------------------------------------*/
DWORD WINAPI udpThread(LPVOID lpParameter)
{
	LPSOCKET_INFORMATION socketInfo;
	WSAEVENT eventArray[1];
	DWORD index, flags;
	int error = 0;
	char message[256];
	SOCKADDR_IN	client;
	int clientSize = sizeof(SOCKADDR_IN);

	eventArray[0] = (WSAEVENT)lpParameter;
	while (true)
	{
		while (true)
		{
			index = WSAWaitForMultipleEvents(1, eventArray, FALSE, WSA_INFINITE, TRUE);

			if (index == WSA_WAIT_FAILED)
			{
				writeToScreen("WSAWaitForMultipleEvents failed");
				break;
			}
			if (index != WAIT_IO_COMPLETION)
			{
				break;
			}
		}

		WSAResetEvent(eventArray[index - WSA_WAIT_EVENT_0]);

		// Create a socket information structure to associate with socket.
		if ((socketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			sprintf(message, "GlobalAlloc() failed with error %d\n", GetLastError());
			writeToScreen(message);
			return FALSE;
		}

		// Fill in the details of our accepted socket.
		socketInfo->Socket = udpSocket;
		ZeroMemory(&(socketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		socketInfo->BytesSEND = 0;
		socketInfo->BytesRECV = 0;
		socketInfo->DataBuf.len = DATA_BUFSIZE;
		socketInfo->DataBuf.buf = socketInfo->Buffer;
		socketInfo->Timeout = INFINITE;

		flags = 0;
		if (WSARecvFrom(socketInfo->Socket, &(socketInfo->DataBuf), 1, NULL, &flags, (sockaddr *)&client, &clientSize, &(socketInfo->Overlapped), udpRoutine) == SOCKET_ERROR)
		{
			if ((error = WSAGetLastError()) != WSA_IO_PENDING)
			{
				sprintf(message, "WSARecvFrom failed with error %d", error);
				writeToScreen(message);
			}
		}
	}
}

/*---------------------------------------------------------------------------------
--	FUNCTION: udpRoutine
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void CALLBACK udpRoutine(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped, DWORD flags)
--
--	PARAMETERS:	DWORD errorCode - errorCode from WSARecvFrom call
--				DWORD bytesTransferred - the number of bytes read by WSARecvFrom
--				LPOVERLAPPED overlapped - the overlapped structure that contains
--											the data read
--				DWORD flags - flags used for WSARecvFrom call
--
--	RETURNS:	none
--
--	NOTES:
--	This function is called when WSARecvFrom successfully reads data from the UDP
--	socket. It updates the statistics and writes the data read to file (if user
--  specified a file to save to).
--
---------------------------------------------------------------------------------*/
void CALLBACK udpRoutine(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped, DWORD flags)
{
	LPSOCKET_INFORMATION socketInfo = (LPSOCKET_INFORMATION)overlapped;
	DWORD newFlags;
	SOCKADDR_IN	client;
	int clientSize = sizeof(client);
	int error = 0;
	char message[256];

	if (errorCode != 0)
	{
		writeToScreen("UDP recv error");
	}

	if (bytesTransferred > 0)
	{
		if (udpStats->startTime.wYear == 0) //start time was never set
		{
			GetSystemTime(&(udpStats->startTime));
		}	
		GetSystemTime(&(udpStats->endTime));

		if (hWriteFile != NULL)
		{
			if (writeToFile(hWriteFile, socketInfo->DataBuf.buf))
			{
				
			}
			else {
				writeToScreen("Saving incoming data failed");
			}
		}
		udpStats->packetCount++;
		udpStats->totalSize += bytesTransferred;
	}
}

/*---------------------------------------------------------------------------------
--	FUNCTION: cleanUpServer
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	VOID cleanUpServer()
--
--	PARAMETERS:	none
--
--	RETURNS:	none
--
--	NOTES:
--	This function is responsible for server clean up. This method is called when
--  the application exits or when the user switches from server mode to client
--  mode. It closes the sockets and files before calling WSACleanup.
--
---------------------------------------------------------------------------------*/
VOID cleanUpServer()
{
	if (serverRunning)
	{
		serverRunning = false;
		WSACloseEvent(tcpEvent);
		WSACloseEvent(udpEvent);
		shutdown(udpSocket, SD_BOTH);
		shutdown(tcpSocket, SD_BOTH);
		closesocket(udpSocket);
		closesocket(tcpSocket);
		closeFile(hWriteFile);
		closeFile(hServerLogFile);
		WSACleanup();
	}
}

/*---------------------------------------------------------------------------------
--	FUNCTION: cleanUpServer
--
--	DATE:		Jan 6, 2008
--
--	REVISIONS:	Jan 6, 2008
--
--	DESIGNER:	Aman Abdulla
--
--	PROGRAMMER:	Aman Abdulla
--
--	INTERFACE:	long delay(SYSTEMTIME t1, SYSTEMTIME t2)
--
--	PARAMETERS:	SYSTEMTIME t1, t2 - SYSTEMTIME data structures
--
--	RETURNS:	the difference in milliseconds between the two SYSTEMTIMEs.
--
--	NOTES:
--	This function calculates and returns the difference (in milliseconds) between
--	two SYSTEMTIME data structures.
--
---------------------------------------------------------------------------------*/
long delay(SYSTEMTIME t1, SYSTEMTIME t2)
{
	long d;

	d = (t2.wSecond - t1.wSecond) * 1000;
	d += (t2.wMilliseconds - t1.wMilliseconds);
	return(d);
}

/*---------------------------------------------------------------------------------
--	FUNCTION: displayStats
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void displayStats(TRANSFER_STATS *stats)
--
--	PARAMETERS:	TRANSFER_STATS * stats - data structure that contains transfer statistics
--
--	RETURNS:	none
--
--	NOTES:
--	This function is responsible for going through the transfer statistics data
--  structure and printing out the data to the screen. It also writes the same
--  data to the server log file.
--
---------------------------------------------------------------------------------*/
void displayStats(TRANSFER_STATS *stats)
{
	char data[256] = { 0 };
	sprintf(data, "Data received via %s", stats->protocol);
	writeToScreen(data);
	strcat(data, "\r\n");
	writeToFile(hServerLogFile, data);
	sprintf(data, "Start time: %d-%02d-%02d %02d:%02d:%02d:%03d",
		(stats->startTime.wYear),
		(stats->startTime.wMonth),
		(stats->startTime.wDay),
		(stats->startTime.wHour),
		(stats->startTime.wMinute),
		(stats->startTime.wSecond),
		(stats->startTime.wMilliseconds));
	writeToScreen(data);
	strcat(data, "\r\n");
	writeToFile(hServerLogFile, data);
	sprintf(data, "End time: %d-%02d-%02d %02d:%02d:%02d:%03d",
		(stats->endTime.wYear),
		(stats->endTime.wMonth),
		(stats->endTime.wDay),
		(stats->endTime.wHour),
		(stats->endTime.wMinute),
		(stats->endTime.wSecond),
		(stats->endTime.wMilliseconds));
	writeToScreen(data);
	strcat(data, "\r\n");
	writeToFile(hServerLogFile, data);
	sprintf(data, "Packets received: %d", stats->packetCount);
	writeToScreen(data);
	strcat(data, "\r\n");
	writeToFile(hServerLogFile, data);
	sprintf(data, "Total bytes received: %d Bytes", stats->totalSize);
	writeToScreen(data);
	strcat(data, "\r\n");
	writeToFile(hServerLogFile, data);
	sprintf(data, "Total transfer time: %ld milliseconds", delay(stats->startTime, stats->endTime));
	writeToScreen(data);
	strcat(data, "\r\n\r\n");
	writeToFile(hServerLogFile, data);
}