#ifndef __SERVER_POC_CLASS_H__
#define __SERVER_POC_CLASS_H__

#pragma once

#include <Windows.h>
#include <iostream>
#include <tchar.h>
#include <strsafe.h>
#include <string>

#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 
#define MAX_INSTANCES 4 
#define PIPE_TIMEOUT 5000
#define BUFSIZE 1024

typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInstance;
	TCHAR chRequest[BUFSIZE];
	DWORD cbRead;
	TCHAR chReply[BUFSIZE];
	DWORD cbToWrite;
	DWORD dwState;
	BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;

class ServerPocClass
{
private:
	short int noInstances;
	DWORD dwWait;
	DWORD cbRet;
	DWORD dwErr;
	DWORD dwThreadId;
	bool isSuccess;
	HANDLE hPipe;
	HANDLE hThread;
	std::string pipeNameStr;
	BOOL fConnected;

protected:

	// create instances of a named pipe
	int createNamedPipeInstance();

	// create Several Named Pipe Instances:
	int createSeveralNamedPipeInstance();

	// wait the client to connect
	int waitClientToConnect();

	// treat client request when it connects to the server
	int treatClient();


public:

	ServerPocClass(std::string pipeName, short int noInstances);
	virtual ~ServerPocClass();

	// create the main Server Thread:
	void createMainServerThread();

};
#endif /* __SERVER_POC_CLASS_H__ */

