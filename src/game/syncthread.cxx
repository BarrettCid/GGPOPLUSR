#include <cstdio>
#include <stdlib.h>
#include <stdio.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <shlwapi.h>
#include <strsafe.h>

#include <detours.h>
#include <d3d9.h>
#include <ggponet.h>
#include <sstream>

#include "./game.h"


typedef struct SynchronizeServerThreadData {
	GameState* lpGameState;
	unsigned short nSyncPort;
	unsigned short nGGPOPort;
	int nOurCharacter;
	StageSelection* pOurStage;
	RandomNumberGenerator RNG1;
	RandomNumberGenerator RNG2;
	RandomNumberGenerator RNG3;
} SynchronizeServerThreadData;

typedef struct SynchronizeClientThreadData {
	GameState* lpGameState;
	char szHostIP[32];
	unsigned short nSyncPort;
	unsigned short nGGPOPort;
	int nOurCharacter;
	int nCharacterColor;
	StageSelection* pOurStage;
} SynchronizeClientThreadData;

DWORD WINAPI SynchronizeClientThreadProc(LPVOID td);
DWORD WINAPI SynchronizeServerThreadProc(LPVOID td);

HANDLE CreateSynchronizeServerThread(
	GameState* lpGameState,
	unsigned short nSyncPort,
	unsigned short nGGPOPort,
	int nOurCharacter,
	StageSelection* pOurStage
) {
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;

	SynchronizeServerThreadData* std = (SynchronizeServerThreadData*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SynchronizeServerThreadData));
	std->lpGameState = lpGameState;
	std->nSyncPort = nSyncPort;
	std->nGGPOPort = nGGPOPort;
	std->nOurCharacter = nOurCharacter;
	std->pOurStage = pOurStage;
	memcpy(&std->RNG1, std->lpGameState->lpRNG1, sizeof(RandomNumberGenerator));
	memcpy(&std->RNG2, std->lpGameState->lpRNG2, sizeof(RandomNumberGenerator));
	memcpy(&std->RNG3, std->lpGameState->lpRNG3, sizeof(RandomNumberGenerator));

	return CreateThread(
		&sa,
		0,
		SynchronizeServerThreadProc,
		std,
		0,
		NULL
	);
}

DWORD WINAPI SynchronizeServerThreadProc(LPVOID td) {
	SynchronizeServerThreadData* std = (SynchronizeServerThreadData*)td;

	struct addrinfo* result = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;
	int iResult;
	int iSendResult;
	char szPort[6];
	char opponentCharacter;
	unsigned short nOpponentGGPOPort;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN addr;
	int addrlen = sizeof(addr);

	ClientSynchronizationRequest request;
	ServerSynchronizationResponse response;
	char szMessageBuf[1024] = { 0 };

	response.nPort = std->nGGPOPort;
	response.nSelectedCharacter = std->nOurCharacter;
	response.pSelectedStage = std->pOurStage;
	memcpy(&response.RNG1, &std->RNG1, sizeof(RandomNumberGenerator));
	memcpy(&response.RNG2, &std->RNG2, sizeof(RandomNumberGenerator));
	memcpy(&response.RNG3, &std->RNG3, sizeof(RandomNumberGenerator));

	sprintf_s(szPort, sizeof(char) * 6, "%d", std->nSyncPort);
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, szPort, &hints, &result);

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		MessageBoxA(NULL, "bind failed with error", NULL, MB_OK);
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		MessageBoxA(NULL, "Listen failed with error", NULL, MB_OK);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, (SOCKADDR*)&addr, &addrlen);
	if (ClientSocket == INVALID_SOCKET) {
		MessageBoxA(NULL, "accept failed", NULL, MB_OK);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	iResult = recv(ClientSocket, (char*)&request, sizeof(ClientSynchronizationRequest), 0);
	if (iResult > 0)
		printf("Bytes received: %d\n", iResult);
	else if (iResult == 0)
		printf("Connection closed\n");
	else
		MessageBoxA(NULL, "recv failed with error", NULL, MB_OK);

	EnterCriticalSection(&std->lpGameState->sessionInitState.criticalSection);
	std->lpGameState->sessionInitState.bHasRequest = 1;
	memcpy(&std->lpGameState->sessionInitState.request, &request, sizeof(ClientSynchronizationRequest));
	strcpy(std->lpGameState->sessionInitState.szOpponentIP, inet_ntoa(addr.sin_addr));
	LeaveCriticalSection(&std->lpGameState->sessionInitState.criticalSection);

	iSendResult = send(ClientSocket, (char*)&response, sizeof(ServerSynchronizationResponse), 0);
	if (iSendResult == SOCKET_ERROR) {
		MessageBoxA(NULL, "Could not send synchronization data", NULL, MB_OK);
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// shutdown the send half of the connection since no more data will be sent
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		MessageBoxA(NULL, "Shutdown failed", NULL, MB_OK);
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	EnterCriticalSection(&std->lpGameState->sessionInitState.criticalSection);
	std->lpGameState->sessionInitState.bHasResponse = 1;
	memcpy(&std->lpGameState->sessionInitState.response, &response, sizeof(ServerSynchronizationResponse));
	std->lpGameState->sessionInitState.bIsHost = 1;
	LeaveCriticalSection(&std->lpGameState->sessionInitState.criticalSection);
	HeapFree(GetProcessHeap(), 0, std);
	return 0;
}

HANDLE CreateSynchronizeClientThread(
	GameState* lpGameState,
	char* szHostIP,
	unsigned short nSyncPort,
	unsigned short nGGPOPort,
	int nOurCharacter,
	StageSelection* pOurStage
) {
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;
	SynchronizeClientThreadData* ctd = (SynchronizeClientThreadData*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SynchronizeClientThreadData));
	ctd->lpGameState = lpGameState;
	strcpy(ctd->szHostIP, szHostIP);
	ctd->nSyncPort = nSyncPort;
	ctd->nGGPOPort = nGGPOPort;
	ctd->nOurCharacter = nOurCharacter;
	ctd->pOurStage = pOurStage;
	return CreateThread(
		&sa,
		0,
		SynchronizeClientThreadProc,
		ctd,
		0,
		NULL
	);
}

DWORD WINAPI SynchronizeClientThreadProc(LPVOID td) {
	SynchronizeClientThreadData* ctd = (SynchronizeClientThreadData*)td;
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;
	char szPort[6];
	char opponentCharacter;
	int iResult;

	ClientSynchronizationRequest request;
	ServerSynchronizationResponse response;
	char szMessageBuf[1024] = { 0 };

	request.nPort = ctd->nGGPOPort;
	request.nSelectedCharacter = ctd->nOurCharacter;

	sprintf_s(szPort, sizeof(char) * 6, "%d", ctd->nSyncPort);

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
	iResult = getaddrinfo(ctd->szHostIP, szPort, &hints, &result);
	if (iResult != 0) {
		MessageBoxA(NULL, "getaddrinfo failed with error", NULL, MB_OK);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			MessageBoxA(NULL, "socket failed with error", NULL, MB_OK);
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			MessageBoxA(NULL, "socket error on result", NULL, MB_OK);
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		MessageBoxA(NULL, "Unable to connect to server", NULL, MB_OK);
		WSACleanup();
		return 1;
	}

	iResult = send(ConnectSocket, (char*)&request, sizeof(ClientSynchronizationRequest), 0);
	if (iResult == SOCKET_ERROR) {
		MessageBoxA(NULL, "Could not send synchronization data", NULL, MB_OK);
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	EnterCriticalSection(&ctd->lpGameState->sessionInitState.criticalSection);
	ctd->lpGameState->sessionInitState.bHasRequest = 1;
	ctd->lpGameState->sessionInitState.bIsHost = 0;
	memcpy(&ctd->lpGameState->sessionInitState.request, &request, sizeof(ClientSynchronizationRequest));
	LeaveCriticalSection(&ctd->lpGameState->sessionInitState.criticalSection);

	iResult = recv(ConnectSocket, (char*)&response, sizeof(ServerSynchronizationResponse), 0);
	if (iResult > 0)
		printf("Bytes received: %d\n", iResult);
	else if (iResult == 0)
		printf("Connection closed\n");
	else
		MessageBoxA(NULL, "recv failed with error", NULL, MB_OK);

	EnterCriticalSection(&ctd->lpGameState->sessionInitState.criticalSection);
	ctd->lpGameState->sessionInitState.bHasResponse = 1;
	memcpy(&ctd->lpGameState->sessionInitState.response, &response, sizeof(ServerSynchronizationResponse));
	strcpy(ctd->lpGameState->sessionInitState.szOpponentIP, ctd->szHostIP);
	LeaveCriticalSection(&ctd->lpGameState->sessionInitState.criticalSection);

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		MessageBoxA(NULL, "shutdown failed with error", NULL, MB_OK);
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ConnectSocket);
	HeapFree(GetProcessHeap(), 0, ctd);
	return 0;
}