#define STRICT
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Client.h"
#include "Server.h"
#include "Util.h"

#pragma comment(lib, "WS2_32.Lib")

#define IDM_HELP		101
#define IDM_EXIT		102
#define IDM_CLIENT		103
#define IDM_SERVER		104
#define IDM_TRANS		105
#define IDM_SAVE		106

#define IDD_TRANSDIA	107
#define IDD_SERVDIA		108
#define IDC_HOSTLABEL	109
#define IDC_PORTLABEL	110
#define IDC_PSIZELABEL	111
#define IDC_REPLABEL	112

#define IDC_FILERADIO	113
#define IDC_RANDRADIO	114
#define IDC_HOSTEDIT	115
#define IDC_PORTEDIT	116
#define IDC_PSIZEEDIT	117
#define IDC_REPEDIT	118
#define IDC_TCPRADIO	119
#define IDC_UDPRADIO	120
#define IDC_FILEEDIT	121
#define IDOPENFILE		122
#define IDC_UDPPORTEDIT	123
#define IDC_UDPPORTLABEL	124
#define IDC_TCPPORTEDIT	125
#define IDC_TCPPORTLABEL	126
#define IDC_SAVEFILELABEL	127
#define IDC_SAVEFILEEDIT	128
#define IDOPENSAVEFILE	129

#define UDPSERVPORT 7000
#define TCPSERVPORT 8000
#define PACKETSIZE	1024
#define NUMOFPACKETS 10

void writeToScreen(LPCSTR);
