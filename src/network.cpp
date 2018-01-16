#include "network.h"

static bool _networkIsInitialized=false;

bool networkInit()
{
    WSADATA wsaData;
    int iResult  = ! WSAStartup(MAKEWORD(2,2), &wsaData);
    if (!iResult)
    {
        printf("WSA Startup Failed with error: %d\n", iResult);
    }

    return (_networkIsInitialized=iResult);
}
bool networkRelease()
{
    // Clean up and quit.
    WSACleanup();
    _networkIsInitialized=false;
    return true;
}

 UDPsocket::UDPsocket()
{

}
bool UDPsocket::initSender(int portAddress, std::string serverName)
{
    if (!_networkIsInitialized)
    {
        return 0;
    }

    if (serverName.length()==0)
    {
        char* hostname=new char[50];
        memset(hostname,0,50);
        gethostname(hostname,50);
        serverName=hostname;
        delete[] hostname;
    }

    hostent* host =gethostbyname(serverName.c_str());
    if (host)
    {
        in_addr * address = (in_addr * )host->h_addr;
        serverName = inet_ntoa(* address);
        //std::cout<<serverName<<std::endl;
    }
    else
    {
        //printf("failed to find host ip address with error %d\n", WSAGetLastError());
        return 0;
    }

    //---------------------------------------------
    // Create a socket for sending data
    _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_socket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 0;
    }
    //---------------------------------------------
    // Set up the RecvAddr structure with the IP address of
    // the receiver (in this example case "192.168.1.1")
    // and the specified port number.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(portAddress);
    RecvAddr.sin_addr.s_addr = inet_addr(serverName.c_str());    // make Socket perform non blocking calls

    ioctlsocket(_socket,FIONBIO,&_nonBlocking);
#ifdef SHOW_CONSOLE_MESSAGES
    printf("Started UDP Sender Successfully\n");
#endif // SHOW_CONSOLE_MESSAGES
    return 1;
}
bool UDPsocket::initReceiver(int portAddress)
{
    if (!_networkIsInitialized)
    {
        return 0;
    }

    _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_socket == INVALID_SOCKET) {
        printf("socket failed with error %d\n", WSAGetLastError());
        return 0;
    }
    //-----------------------------------------------
    // Bind the socket to any address and the specified port.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(portAddress);
    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    iResult = bind(_socket, (SOCKADDR *) & RecvAddr, sizeof (RecvAddr));
    if (iResult != 0) {
        printf("bind failed with error %d\n", WSAGetLastError());
        return 0;
    }
    //-----------------------------------------------
    // Call the recvfrom function to receive datagrams
    // on the bound socket.
    ioctlsocket(_socket,FIONBIO,&_nonBlocking);

#ifdef SHOW_CONSOLE_MESSAGES
    printf("Started UDP Receiver Successfully\n");
#endif // SHOW_CONSOLE_MESSAGES
    return 1;
}
bool UDPsocket::transmit(const char* data, int size)
{
    iResult = sendto(_socket, data, size, 0, (SOCKADDR *) & RecvAddr, sizeof (RecvAddr));
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
    }
    return iResult != SOCKET_ERROR;
}
bool UDPsocket::receive(char* data, int &size)
{
    iResult = recvfrom(_socket,data, recvbuflen, 0, (SOCKADDR *) & SenderAddr, &SenderAddrSize);

    if (iResult > 0)
    {
        size=iResult;
    }

    return iResult>0;
}
bool UDPsocket::release()
{
    // When the application is finished sending, close the socket.
    iResult = closesocket(_socket);
    if (iResult == SOCKET_ERROR)
    {
        printf("close socket failed with error: %d\n", WSAGetLastError());
    }
    return iResult != SOCKET_ERROR;
}

