/*---------------------------------------------------------------------------------
--	SOURCE FILE:	Main.cpp -
--
--	PROGRAM:		Transport Layer Protocol Analyser
--
--	FUNCTIONS:
--					int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
--					LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--					INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
--					void writeToScreen(LPCSTR);
--					void openFileDialog(HWND hDlg, char * fileName);
--
--	DATE:			Jan 16, 2016
--
--	REVISIONS:		Feb 13, 2016
--
--	DESIGNER:		Gabriella Cheung
--
--	PROGRAMMER:		Gabriella Cheung
--
--	NOTES:
--	This file contains the code for the GUI of the application, and the proc
--  functions to handle user input from dialogs. The application allows the user
--  to choose to run the application in client mode (where they can send data to 
--  a server) or server mode (where they can received data sent from a client).
--  Messages from the client and the server are displayed on the screen.
--
---------------------------------------------------------------------------------*/
#include "resource.h"

TCHAR Name[] = TEXT("Transport Layer Protocol Analyzer");
char help[1024] = "Choose to be in client or server mode.\nIn client mode, click on Transfer->Transfer Data to send data to a server.\nIn server mode, enter the ports for UDP and TCP.";
HWND hwnd, hwndList, hTransfer, hServerSetup;
HMENU hMenu;
BOOL clientMode = TRUE;
HANDLE clientLogFile;

// function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
void writeToScreen(LPCSTR);
void openFileDialog(HWND hDlg, char * fileName);

#pragma warning (disable: 4096)

/*---------------------------------------------------------------------------------
--	FUNCTION: winMain
--
--	DATE:		Oct 3, 2015
--
--	REVISIONS:	Feb 13, 2016
--
--	DESIGNER:	Microsoft
--
--	PROGRAMMER:	Microsoft
--
--	MODIFIED BY: Gabriella Cheung
--
--	INTERFACE:	int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--					LPSTR lspszCmdParam, int nCmdShow)
--
--  PARAMETERS: HINSTANCE hInst - A handle to the current instance of the application.
--				HINSTANCE hprevInstance - A handle to the previous instance of the application. 
--				LPSTR lspszCmdParam - The command line for the application,
					excluding the program name. 
--				int nCmdShow - Controls how the window is to be shown.
--
--	RETURNS:	void
--
--	NOTES:
--	Entry point of the application. It initializes the main window and sets up
--  WndProc as the method that processes messages in the message queue.
--
---------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = 0;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = TEXT("MYMENU"); // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		600, 600, NULL, NULL, hInst, NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	hMenu = GetMenu(hwnd);
	EnableMenuItem(hMenu, IDM_TRANS, MF_CHECKED);
	CheckMenuRadioItem(hMenu, IDM_CLIENT, IDM_SERVER, IDM_CLIENT, MF_CHECKED);
	clientLogFile = openFile("clientLog.txt", false);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

/*---------------------------------------------------------------------------------
--	FUNCTION: WndProc
--
--	DATE:		Oct 3, 2015
--
--	REVISIONS:	Feb 6, 2016 - added code to handle custom messages
--
--	DESIGNER:	Microsoft
--
--	PROGRAMMER:	Microsoft
--
--	MODIFIED BY: Gabriella Cheung
--
--	INTERFACE:	LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
--					WPARAM wParam, LPARAM lParam)
--
--	PARAMETERS:	HWND hwnd - A handle to the window.
--				UINT Message - The message.
--				WPARAM wParam - Additional message information.
--				LPARAM lParam - Additional message information.
--
--	RETURNS:	LRESULT - the result of the message processing and
--					depends on the message sent.
--
--	NOTES:
--	An application-defined function that processes messages sent to a window.
--
--	Modified so it will handle messages related to menu item selection.
--
--	When the user selects a menu item, a dialog with the appropriate textbox(es)
--  will appear. Messages sent from those dialogs are handled by a separate proc
--  (DialogProc).
--
---------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_CREATE:
		hwndList = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("listbox"), "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 20, 20, 540, 500, hwnd, NULL, NULL, NULL);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_CLIENT:
			cleanUpServer();
			CheckMenuRadioItem(hMenu, IDM_CLIENT, IDM_SERVER, IDM_CLIENT, MF_CHECKED);
			EnableMenuItem(hMenu, IDM_TRANS, MF_ENABLED);
			break;
		case IDM_SERVER:
			// run server code
			hServerSetup = CreateDialogParam((HINSTANCE)GetWindowLong(hwnd, 0), MAKEINTRESOURCE(IDD_SERVDIA), 0, DialogProc, 0);
			ShowWindow(hServerSetup, SW_SHOWDEFAULT);
			break;
		case IDM_TRANS:
			//open transfer data dialog
			hTransfer = CreateDialogParam((HINSTANCE)GetWindowLong(hwnd, 0), MAKEINTRESOURCE(IDD_TRANSDIA), 0, DialogProc, 0);
			ShowWindow(hTransfer, SW_SHOWDEFAULT);
			break;
		case IDM_SAVE:
			//open save to file dialog
			break;
		case IDM_HELP:
			MessageBox(hwnd, TEXT(help), TEXT("Help"), MB_OK);
			break;
		case IDM_EXIT:
			//Terminate program
			if (!clientMode)
			{
				cleanUpServer();
			}
			closeFile(clientLogFile);
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_DESTROY:	// Terminate program
		if (!clientMode)
		{
			cleanUpServer();
		}
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/*---------------------------------------------------------------------------------
--	FUNCTION: writeToScreen
--
--	DATE:		Oct 3, 2015
--
--	REVISIONS:	Feb 13, 2016 - modified so it works by adding string to listbox
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void writeToScreen(LPCSTR data)
--
--	PARAMETERS:	LPCSTR data
--
--	RETURNS:	void
--
--	NOTES:
--	Writes strings to screen by adding a string to the listbox.
--
---------------------------------------------------------------------------------*/
void writeToScreen(LPCSTR data)
{
	SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)data);
}

/*---------------------------------------------------------------------------------
--	FUNCTION: DialogProc
--
--	DATE:		Jan 16, 2016
--
--	REVISIONS:	Feb 6, 2016 - modified to work with client and server dialogs
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMSG, WPARAM wParam, LPARAM lParam)
--
--	PARAMETERS:	HWND hDlg - A handle to the dialog box.
--				UINT Message - The message.
--				WPARAM wParam - Additional message-specific information.
--				LPARAM lParam - Additional message-specific information.
--
--	RETURNS:	INT_PTR - TRUE if it processed the message, and FALSE if it did not. 
--
--	NOTES:
--	This function handles messages from the client ("Transfer Data") and server
--	("Server Setup") dialogs.
--  If the dialog is Transfer Data, it checks all the data entered first before
--  calling the sendViaUDP or sendViaTCP method in Client.cpp.
--  If the dialog is Server Setup, it checks the data entered before calling the
--  startServer method in Server.cpp.
--
---------------------------------------------------------------------------------*/
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMSG, WPARAM wParam, LPARAM lParam)
{
	char sizeStr[16] = { 0 };
	char repStr[16] = { 0 };
	char portStr[16] = { 0 };
	BOOL tcp = false;
	char message[256] = { 0 };
	HANDLE hReadFile = NULL, hWriteFile = NULL;

	switch (uMSG)
	{
	case WM_INITDIALOG:
		sprintf(sizeStr, "%d", PACKETSIZE);
		SetDlgItemText(hDlg, IDC_PSIZEEDIT, sizeStr);
		sprintf(repStr, "%d", NUMOFPACKETS);
		SetDlgItemText(hDlg, IDC_REPEDIT, repStr);
		sprintf(portStr, "%d", UDPSERVPORT);
		SetDlgItemText(hDlg, IDC_UDPPORTEDIT, portStr);
		sprintf(portStr, "%d", TCPSERVPORT);
		SetDlgItemText(hDlg, IDC_TCPPORTEDIT, portStr);
		break;
	case WM_CLOSE:
		DestroyWindow(hDlg);
	case WM_DESTROY:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			if (hDlg == hTransfer)
			{
				char hostname[256] = { 0 };
				char port[16] = { 0 };
				char size[16] = { 0 };
				char rep[16] = { 0 };
				char file[256] = { 0 };

				//get server ip
				GetDlgItemText(hDlg, IDC_HOSTEDIT, hostname, 256);
				if (hostname[0] == NULL || !isdigit(*hostname)) //if string is empty or if first char is not a digit
				{
					MessageBox(hDlg, TEXT("Please enter IP address in X.X.X.X format"), TEXT("Error"), MB_OK);
					break;
				}
				//get port number
				GetDlgItemText(hDlg, IDC_PORTEDIT, port, 256);
				if (port[0] == NULL || !isdigit(*port))
				{
					MessageBox(hDlg, TEXT("Please enter port number"), TEXT("Error"), MB_OK);
					break;
				}
				//get packet size and repetition
				GetDlgItemText(hDlg, IDC_PSIZEEDIT, size, 16);
				if (size[0] == NULL || !isdigit(*size))
				{
					MessageBox(hDlg, TEXT("Please enter packet size"), TEXT("Error"), MB_OK);
					break;
				}
				else if (atoi(size) > MAXLEN)
				{
					MessageBox(hDlg, TEXT("Packet size entered exceeds limit of 65000 bytes"), TEXT("Error"), MB_OK);
					break;
				}
				GetDlgItemText(hDlg, IDC_REPEDIT, rep, 16);
				if (rep[0] == NULL || !isdigit(*rep))
				{
					MessageBox(hDlg, TEXT("Please enter number of packets to send"), TEXT("Error"), MB_OK);
					break;
				}
				//get protocol
				if (IsDlgButtonChecked(hDlg, IDC_TCPRADIO) == BST_CHECKED)
				{
					tcp = true;
				}

				//get source
				if (!IsDlgButtonChecked(hDlg, IDC_RANDRADIO) == BST_CHECKED)
				{
					//have to check file selected
					GetDlgItemText(hDlg, IDC_FILEEDIT, file, 256);
					if (file[0] == NULL) //if string is empty
					{
						MessageBox(hDlg, TEXT("Please enter file name"), TEXT("Error"), MB_OK);
						break;
					}
					else {
						hReadFile = openFile(file, true);
					}
				}
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				//call client function that takes in hostname, port, packet size, repetition, source
				if (tcp)
				{
					sendViaTCP(hostname, atoi(port), atoi(size), atoi(rep), hReadFile, clientLogFile);
				}
				else {
					sendViaUDP(hostname, atoi(port), atoi(size), atoi(rep), hReadFile, clientLogFile);
				}
			}
			else if (hDlg == hServerSetup)
			{
				char udp[64] = { 0 };
				char tcp[64] = { 0 };
				int uPort = 7000;
				int tPort = 8000;
				char file[256] = { 0 };
				GetDlgItemText(hDlg, IDC_UDPPORTEDIT, udp, 64);
				if (udp[0] != NULL || isdigit(*udp))
				{
					uPort = atoi(udp);
				}
				GetDlgItemText(hDlg, IDC_TCPPORTEDIT, tcp, 64);
				if (tcp[0] != NULL || isdigit(*tcp))
				{
					tPort = atoi(tcp);
				}
				if (uPort == tPort)
				{
					MessageBox(hDlg, TEXT("UDP and TCP Ports cannot be the same"), TEXT("Error"), MB_OK);
					break;
				}
				GetDlgItemText(hDlg, IDC_SAVEFILEEDIT, file, 256);
				if (file[0] != NULL) // if file name is entered
				{
					hWriteFile = openFile(file, false);
				}
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				cleanUpServer();
				startServer(uPort,tPort, hWriteFile);
				CheckMenuRadioItem(hMenu, IDM_CLIENT, IDM_SERVER, IDM_SERVER, MF_CHECKED);
				EnableMenuItem(hMenu, IDM_TRANS, MF_GRAYED);
				clientMode = FALSE;
			}
			return TRUE;
		}
		break;
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return TRUE;
		case IDOPENFILE:
		{
			char fileName[256];
			openFileDialog(hDlg, fileName);
			SetDlgItemText(hDlg, IDC_FILEEDIT, (LPCSTR) fileName);
			break;
		}
		case IDOPENSAVEFILE:
		{
			char fileName[256];
			openFileDialog(hDlg, fileName);
			SetDlgItemText(hDlg, IDC_SAVEFILEEDIT, (LPCSTR)fileName);
			break;
		}
		break;
		}
	}
	return FALSE;
}

/*---------------------------------------------------------------------------------
--	FUNCTION: openFileDialog
--
--	DATE:		Feb 6, 2016
--
--	REVISIONS:	Feb 6, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void openFileDialog(HWND hDlg, char * fileName)
--
--	PARAMETERS:	HWND hDlg - A handle to the dialog box.
--				char * fileName - a buffer to contain the name of file.
--
--	RETURNS:	none
--
--	NOTES:
--	This function opens the Open File Dialog and saves the name of the file
--	selected by the user into the buffer that was passed in.
--
---------------------------------------------------------------------------------*/
void openFileDialog(HWND hDlg, char * fileName)
{
	char name[256];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFile = name;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(name);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST ;

	if (GetOpenFileName(&ofn) == TRUE) {
		strcpy(fileName, name);
	}
	return;
}