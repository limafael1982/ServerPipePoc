#include "stdafx.h"
#include "ServerPocClass.h"


// declarinng useful function for thread instatiation
DWORD WINAPI instanceThread(LPVOID lpvParam);

ServerPocClass::ServerPocClass(std::string pipeName, short int noInstances)
{
	this->isSuccess = false;
	this->dwWait = -1;
	this->cbRet = -1;
	this->dwErr;
	this->hPipe = INVALID_HANDLE_VALUE;
	this->pipeNameStr = pipeName;
	this->fConnected = FALSE;
	this->dwThreadId = 0;

	if (noInstances <= 0)
	{
		this->noInstances = 1;
		std::cout << "[ServerPocClass] Minimum number of instances is ONE" << std::endl;
	}
	else
	{
		this->noInstances = noInstances;
		if (noInstances >= MAX_INSTANCES)
		{
			std::cout << "[ServerPocClass] Maximum number of instances is  " << MAX_INSTANCES << std::endl;
			this->noInstances = MAX_INSTANCES;
		}
	}


}

ServerPocClass::~ServerPocClass()
{
	std::cout << "[ServerPocClass] Destructing ServerPocClass object" << std::endl;
}

void ServerPocClass::createMainServerThread()
{
	std::cout << "[ServerPocClass] Main thread created. Waiting for client connections."
		<< std::endl;
	while (true)
	{
		auto retCreateResult = this->createSeveralNamedPipeInstance();
		if (retCreateResult == -1)
		{
			break;
		}
		retCreateResult = this->waitClientToConnect();
		if (retCreateResult == FALSE)
		{
			std::cout << "[ServerPocClass] Sorry, but the client could not connect. Closing the pipe..." << std::endl;
			CloseHandle(this->hPipe);
			std::cout << "[ServerPocClass] pipe closed" << std::endl;
			break;
		}
		retCreateResult = this->treatClient();
		if (retCreateResult == -1)
		{
			std::cout << "[ServerPocClass] Sorry, I could not treat the client. Closing the pipe..." << std::endl;
			break;
		}


	}

}

int ServerPocClass::createSeveralNamedPipeInstance()
{
	int retValue = 0;
	this->hPipe = CreateNamedPipe(
		TEXT("\\\\.\\pipe\\namedPipeExample"),
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE |
		PIPE_READMODE_MESSAGE |
		PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		BUFSIZE,
		BUFSIZE,
		0,
		NULL);

	if (this->hPipe == INVALID_HANDLE_VALUE)
	{
		std::cout << "[ServerPocClass]: ERROR! CreateNamedPipe failed!" << std::endl;
		std::cout << GetLastError();
		retValue = -1;
		return retValue;
	}

	return 0;

}

int ServerPocClass::waitClientToConnect()
{
	// this->fConnected will have a non-zero value if the connection is successful.
	this->fConnected = ConnectNamedPipe(this->hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	int retVal = static_cast<int> (this->fConnected);
	return retVal;
}

int ServerPocClass::treatClient()
{
	if (this->fConnected == TRUE)
	{
		std::cout << "[ServerPocClass] Client was successfully connected. Now creating thread..." << std::endl;
		this->hThread = CreateThread(
			NULL,
			0,
			instanceThread,
			(LPVOID)hPipe,
			0,
			&this->dwThreadId
		);

		if (this->hThread == NULL)
		{
			std::cout << "[ServerPocClass] Failed creating thread" << std::endl;
			return -1;
		}
		else
		{
			CloseHandle(this->hThread);
		}
	}
	else
	{
		std::cout << "[ServerPocClass] Client could not connect. Closing the pipe..." << std::endl;
		CloseHandle(this->hPipe);
		std::cout << "[ServerPocClass] pipe handle closed." << std::endl;
		return -1;
	}
	return 0;
}


VOID getAnswerToRequest(LPTSTR pchRequest, LPTSTR pchReply, LPDWORD pchBytes)
{
	_tprintf(TEXT("Client Request String:\"%s\"\n"), pchRequest);	

	// we will get a string which represents a number from the client
	// we must convert to a numerical type and double it.
	int numberToDouble = _tstoi(pchRequest);
	numberToDouble *= 2;
	std::wstring numStr = std::to_wstring(numberToDouble);
	TCHAR buf[BUFSIZE];

	swprintf_s(buf, BUFSIZE, TEXT("server : %d"), numberToDouble);
	if (FAILED(StringCchCopy(pchReply, BUFSIZE, buf)))
	{
		*pchBytes = 0;
		pchReply[0] = 0;
		std::cout << "StringCchCopy failed, no outgoing message" << std::endl;
		return;
	}
	*pchBytes = (lstrlen(pchReply) + 1) * sizeof(TCHAR);
}

DWORD WINAPI instanceThread(LPVOID lpvParam)
{
	HANDLE hHeap = GetProcessHeap();
	TCHAR * pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));
	TCHAR * pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = NULL;

	if (lpvParam == NULL)
	{
		std::cout << "[ServerPocClass] Error -> Pipe Server Failure" << std::endl;
		std::cout << "[ServerPocClass] InstanceThread got an unexpected NULL value in lpvParam" << std::endl;
		std::cout << "[ServerPocClass::instanceThread] Error. Exiting" << std::endl;
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return static_cast<DWORD>(-1);
	}

	if (pchRequest == NULL)
	{
		std::cout << "[ServerPocClass] Error -> Pipe Server Failure" << std::endl;
		std::cout << "[ServerPocClass] InstanceThread got an unexpected NULL value in pchRequest" << std::endl;
		std::cout << "[ServerPocClass::instanceThread] Error. Exiting" << std::endl;
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		return static_cast<DWORD>(-1);
	}

	if (pchReply == NULL)
	{
		std::cout << "[ServerPocClass] Error -> Pipe Server Failure" << std::endl;
		std::cout << "[ServerPocClass] InstanceThread got an unexpected NULL value in pchReply" << std::endl;
		std::cout << "[ServerPocClass::instanceThread] Error. Exiting" << std::endl;
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return static_cast<DWORD>(-1);
	}

	std::cout << "[ServerPocClass] instanceThread created. Ready to receive and process messages" << std::endl;

	hPipe = static_cast<HANDLE>(lpvParam);

	// loop for reading:
	while (true)
	{
		// reading client requests...
		fSuccess = ReadFile(hPipe, pchRequest, BUFSIZE * sizeof(TCHAR), &cbBytesRead, NULL);

		if ((fSuccess != TRUE) || (cbBytesRead == 0))
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				auto error = GetLastError();
				auto errorInt = static_cast<int>(error);
				std::cout << "[ServerPocClass] GetLastError() = " << errorInt << std::endl;
				std::cout << "[ServerPocClass] Error -> client disconnected" << std::endl;
			}
			else
			{
				auto error = GetLastError();
				auto errorInt = static_cast<int>(error);
				std::cout << "[ServerPocClass] GetLastError() = " << errorInt << std::endl;
				std::cout << "[ServerPocClass] Error -> ReadFile failed" << std::endl;
			}
			break;
		}

		// get the incoming message
		getAnswerToRequest(pchRequest, pchReply, &cbReplyBytes);

		// write the reply to the pipe
		fSuccess = WriteFile(hPipe, pchReply, cbReplyBytes, &cbWritten, NULL);
		if ((fSuccess != TRUE) || (cbReplyBytes != cbWritten))
		{
			auto error = GetLastError();
			auto errorInt = static_cast<int>(error);
			std::cout << "[ServerPocClass] GetLastError() = " << errorInt << std::endl;
			std::cout << "[ServerPocClass] Error -> WriteFile failed" << std::endl;
			break;
		}
	}

	// Flush the pipe...
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	HeapFree(hHeap, 0, pchRequest);
	HeapFree(hHeap, 0, pchReply);

	return static_cast<DWORD>(1);

}

