#ifndef NETWORK_H_INC
#define NETWORK_H_INC

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define DEFAULT_BUFLEN 1024

#define SOCKET_OK           00001 // not an MSDN code
#define SOCKET_CLEAN        10035 // MSDN error code for empty sockets when trying to read
#define SOCKET_DISCONNECTED 10054 // MSDN error code for disconnected sockets when trying to read or write

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <vector>

bool networkInit();
bool networkRelease();

class UDPsocket{
public:
    UDPsocket();
    bool initReceiver(int portAddress);
    bool initSender  (int portAddress, std::string serverName="");
    bool release();
    bool transmit(const char* data, int size);
    bool receive (char* data, int &size);

private:
    SOCKET _socket = INVALID_SOCKET;

    sockaddr_in RecvAddr;
    sockaddr_in SenderAddr;

    struct addrinfo hints;
    struct addrinfo *result = NULL;

    int SenderAddrSize = sizeof (SenderAddr);

    int iResult;

    int recvbuflen = DEFAULT_BUFLEN;

    u_long _nonBlocking=1;
};

#endif // NETWORK_H_INC
