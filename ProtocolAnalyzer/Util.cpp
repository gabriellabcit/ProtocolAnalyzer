/*---------------------------------------------------------------------------------
--	SOURCE FILE:	Util.cpp -
--
--	PROGRAM:		Transport Layer Protocol Analyser
--
--	FUNCTIONS:
--					HANDLE openFile(char* fileName, BOOL readOnly)
--					BOOL closeFile(HANDLE file)
--					void getData(HANDLE hFile, char * buffer, int size)
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
--	This file contains utility methods used by both the server and the client part 
--  of the application.
--
---------------------------------------------------------------------------------*/
#include "resource.h"

/*---------------------------------------------------------------------------------
--	FUNCTION: openFile
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	HANDLE openFile(char* fileName, BOOL readOnly)
--
--	PARAMETERS:	char *fileName - name of file to be opened
--				BOOL readOnly - whether or not file should be opened in read-only mode
--
--	RETURNS:	handle to file that was opened
--
--	NOTES:
--	This function opens a file in either read-only mode (for sending data) or 
--  in write mode (for saving data and writing log files).
--
---------------------------------------------------------------------------------*/
HANDLE openFile(char* fileName, BOOL readOnly)
{
	HANDLE hFile = NULL;
	if (fileName[0] != '\0') // if user wanted to open file
	{
		if (readOnly)
		{
			hFile = CreateFile(fileName, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		}
		else {
			hFile = CreateFile(fileName, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		}

		if (hFile == INVALID_HANDLE_VALUE)
		{
			writeToScreen("Unable to open file");
			hFile = NULL;
		}
	}
	return hFile;
}

/*---------------------------------------------------------------------------------
--	FUNCTION: closeFile
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	BOOL closeFile(HANDLE file)
--
--	PARAMETERS:	HANDLE file - handle of file to be closed
--
--	RETURNS:	true if file closed successfully, false otherwise
--
--	NOTES:
--	A wrapper function for the CloseHandle method that is used to close a file.
--
---------------------------------------------------------------------------------*/
BOOL closeFile(HANDLE file)
{
	return CloseHandle(file);
}

/*---------------------------------------------------------------------------------
--	FUNCTION: writeToFile
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	BOOL writeToFile(HANDLE file, char * data)
--
--	PARAMETERS:	HANDLE file - handle of file to write to
--				char * data - data to write to file
--
--	RETURNS:	true if data was written successfully, false otherwise
--
--	NOTES:
--	A wrapper function for the WriteFile method.
--
---------------------------------------------------------------------------------*/
BOOL writeToFile(HANDLE file, char * data)
{
	DWORD charsWritten;
	if (FALSE == WriteFile(file, data, strlen(data), &charsWritten, NULL))
	{
		writeToScreen("Unable to write to file");
		return false;
	}
	return true;
}

/*---------------------------------------------------------------------------------
--	FUNCTION: getData
--
--	DATE:		Feb 14, 2016
--
--	REVISIONS:	Feb 14, 2016
--
--	DESIGNER:	Gabriella Cheung
--
--	PROGRAMMER:	Gabriella Cheung
--
--	INTERFACE:	void getData(HANDLE hFile, char * buffer, int size)
--
--	PARAMETERS:	HANDLE hFile - handle of file to read data from
--				char * buffer - buffer to save data to
--				int size - the number of chars to save to buffer
--
--	RETURNS:	none
--
--	NOTES:
--	This function fills the buffer with characters, either read from a file (if
--  file handle is not NULL) or randomly generated.
--
---------------------------------------------------------------------------------*/
void getData(HANDLE hFile, char * buffer, int size)
{
	DWORD charsRead;
	if (hFile != NULL)
	{
		//read from file
		if (FALSE == ReadFile(hFile, buffer, size, &charsRead, NULL))
		{
			writeToScreen("ReadFile failed!");
		}
	}
	else {
		time_t t;
		srand((unsigned)time(&t));
		//randomly generate
		char temp;
		for (int i = 0; i < size; i++)
		{
			temp = (rand() % 93) + 33;
			buffer[i] = temp;
		}
	}
	buffer[size] = '\0';
}