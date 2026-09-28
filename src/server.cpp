#include "entrypoint.h"

#include <stdio.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#elif __linux__
#endif

#define DEFAULT_PORT "7900"

struct addrinfo *result = NULL, *ptr = NULL, hints;
SOCKET ListenSocket = INVALID_SOCKET;


int init_server(void){

    int iResult;

    #ifdef _WIN32
    WSADATA wsaData;
    //Initializing Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    #endif
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP; //Connection de type TCP (et non UDP ou autres)
    hints.ai_flags = AI_PASSIVE;

// Resolve the local address and port to be used by the server
    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result); //127 ou localhost
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return -1;
    }
    // Create a SOCKET for the server to listen for client connections

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }
    freeaddrinfo(result);
    if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
        printf( "Listen failed with error: %ld\n", WSAGetLastError() );
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }
}

#define DEFAULT_BUFLEN 512

char recvbuf[DEFAULT_BUFLEN] = {'S','T','A','R','T', '\0'};
int iResult, iSendResult;

//Creer votre class Engin ici et appeler une fonction start que vous définisser à la classe dans la fonction raylib_start plus bas.
void raylib_start(void){
    if(init_server() < 0 ){
        exit(1);
    }

    printf("Server had started");
    SOCKET ClientSocket;
    ClientSocket = INVALID_SOCKET;

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    int iResult = 6;
    while(iResult > 0){
        if (iResult > 0) {
        printf("Bytes received: %d\n", iResult);
        printf("Message: %d\n", recvbuf);

        // Echo the buffer back to the sender
        iSendResult = send(ClientSocket, recvbuf, iResult, 0);
        if (iSendResult == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            exit(1);
        }
        printf("Bytes sent: %d\n", iSendResult);
        } else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            exit(1);
        }
        iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
    }
}