#pragma once
HANDLE openFile(char*, BOOL);
BOOL closeFile(HANDLE);
BOOL writeToFile(HANDLE, char *);
void getData(HANDLE, char *, int);