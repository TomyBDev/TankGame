#include "LobbyServer.h"
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

// The IP address for the server
#define SERVERIP "127.0.0.9"

// The UDP port number for the server
#define SERVERPORT 4444

bool findAddress(const sockaddr_in& left, std::vector<sockaddr_in>& right);
bool operator==(const sockaddr_in& left, const sockaddr_in& right);

void LobbyServer::_register_methods()
{
    register_method("_setUpServer", &LobbyServer::_setUpServer);

    register_method("_getLocalIP", &LobbyServer::_getLocalIP);
    register_method("_getMsgPlayerReady", &LobbyServer::_getMsgPlayerReady);
    register_method("_getMsgKey", &LobbyServer::_getMsgKey);


    register_method("_listenForConn", &LobbyServer::_listenForConn);
    register_method("_recieveMsg", &LobbyServer::_recieveMsg);
    register_method("_sendMsg", &LobbyServer::_sendMsg);
    register_method("_closeSocket", &LobbyServer::_closeSocket);
}

void LobbyServer::_init()
{
    // Initialising Variables

    timeout.tv_sec = 0;
    timeout.tv_usec = 100;
    msg.key = 0;
    msg.playerID = -1;
    msg.playerReady = false;
    eventIndex = 0;
    clientsID[0] = -1;
    clientsID[1] = -1;
    clientsID[2] = -1;
    // --Set Up Server--
    // Setting up server socket and events

    // Initialise Winsock

    int error = WSAStartup(0x0202, &w);
    if (error != 0)
    {
        printf("WSAStartup failed\n");
    }
    if (w.wVersion != 0x0202)
    {
        printf("Wrong WinSock version\n");
    }

}

bool LobbyServer::_setUpServer()
{
    // Create a TCP socket used for listening for connections

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        printf("socket failed\n");
        return false;
    }

    // A structut to hold out socket address information
    // converting the port to big endian

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVERIP);
    serverAddr.sin_port = htons(SERVERPORT);

    // Bind the socket to that address.
    if (bind(sock, (const sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
    {
        printf("bind failed\n");
        return false;
    }

    printf("Server socket bound to address %s, port %d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    //Create Listen event
    ListenEvent = WSACreateEvent();
    if (ListenEvent == WSA_INVALID_EVENT) {
        printf("server event creation failed");
        return false;
    }
    //Assosciate this event with the socket types we're interested in
    //In this case, on the server, we're interested in Accepts and Closes
    WSAEventSelect(sock, ListenEvent, FD_ACCEPT | FD_CLOSE);

    // Make the socket listen for connections.
    if (listen(sock, 1) == SOCKET_ERROR)
    {
        printf("listen failed");
        return false;
    }

    printf("Server socket listening\n");
    return true;
}

String LobbyServer::_getLocalIP()
{
    String temp;

    for (int i = 0; i < sizeof(SERVERIP); i++)
    {
        temp += SERVERIP[i];
    }

    return temp;
}

bool LobbyServer::_getMsgPlayerReady()
{
    return msg.playerReady;
}

int LobbyServer::_getMsgKey()
{
    return msg.key;
}

void LobbyServer::_listenForConn()
{

    //Check for new clients

    returnVal = WSAWaitForMultipleEvents(1, &ListenEvent, false, 0, false);

    if ((returnVal != WSA_WAIT_TIMEOUT) && (returnVal != WSA_WAIT_FAILED)) {
        eventIndex = returnVal - WSA_WAIT_EVENT_0; //In practice, eventIndex will equal returnVal, but this is here for compatability

        if (WSAEnumNetworkEvents(sock, ListenEvent, &NetworkEvents) == SOCKET_ERROR) {
            printf("Retrieving event information failed");
        }
        if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
        {
            if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
                printf("FD_ACCEPT failed with error %d\n", NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
                return;
            }

            SOCKET clientSock = accept(sock, NULL, NULL);
            printf("Socket trying to connect");
            if (clientSockets.size() == 3)
            {
                printf("Maximum number of clients reached. Terminating connection.\n");
                closesocket(clientSock);
                return;
            }
            else
            {
                clientSockets.push_back(clientSock);

                LobbyMessage message;
                message.playerID = clientSockets.size();
                message.key = 2;
                message.playerReady = false;

                int count = send(clientSock, (const char*)&message, sizeof(LobbyMessage), 0);
                if (count != sizeof(LobbyMessage))
                {
                    printf("sendto failed\n");
                }

                clientEvents[clientSockets.size()-1] = WSACreateEvent();

                WSAEventSelect(clientSockets[clientSockets.size() -1], clientEvents[clientSockets.size()-1], FD_CLOSE | FD_READ | FD_WRITE);
                printf("Socket %d connected\n", clientSockets.back());
            }
        }
    }
    else if (returnVal == WSA_WAIT_TIMEOUT) {
        //All good, we just have no activity
    }
    else if (returnVal == WSA_WAIT_FAILED) {
        printf("WSAWaitForMultipleEvents failed!\n");
    }
}

int LobbyServer::_recieveMsg()
{

    //check for events
    if (clientSockets.size() > 0)
    {
        returnVal = WSAWaitForMultipleEvents(clientSockets.size(), clientEvents, false, 0, false);
        if ((returnVal != WSA_WAIT_TIMEOUT) && (returnVal != WSA_WAIT_FAILED)) {
            eventIndex = returnVal - WSA_WAIT_EVENT_0; //In practice, eventIndex will equal returnVal, but this is here for compatability
            if (WSAEnumNetworkEvents(clientSockets[eventIndex], clientEvents[eventIndex], &NetworkEvents) == SOCKET_ERROR) {
                printf("Retrieving event information failed");
            }
            if (NetworkEvents.lNetworkEvents & FD_CLOSE)
            {
                //We ignore the error if the client just force quit
                if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0 && NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 10053)
                {
                    printf("FD_CLOSE failed with error %d\n", NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
                    return -1;
                }
                printf("closing socket %i, setting key to -2\n", eventIndex+1);
                _closeClientSocket(eventIndex);
                msg.key = 4;
                return (eventIndex+1);
            }
            else if (NetworkEvents.lNetworkEvents & FD_READ)
            {
                recv(clientSockets[eventIndex], (char*)&msg, sizeof(LobbyMessage), 0);
                printf("Recieved Object %i, with key: %i, with ready state of: %b\n", msg.playerID, msg.key, msg.playerReady);

                _sendMsg(msg.playerID, msg.key, msg.playerReady);

                return msg.playerID;
            }
        }
        else if (returnVal == WSA_WAIT_TIMEOUT) {
            //All good, we just have no activity
        }
        else if (returnVal == WSA_WAIT_FAILED) {
            printf("WSAWaitForMultipleEvents failed!\n");
        }
    }

    return -1;
}

void LobbyServer::_sendMsg(int id, int key, bool pReady)
{
    LobbyMessage message;
    message.playerID = id;
    message.key = key;
    message.playerReady = pReady;

    // send to each client
    for (auto it = clientSockets.begin(); it != clientSockets.end(); )  // note no ++it here
    {
        SOCKET conn = *it;
        int count = send(conn, (const char*)&message, sizeof(LobbyMessage), 0);
        if (count != sizeof(LobbyMessage))
        {
            std::cout << "sendto failed\n";
        }
        ++it;
    }
}

void LobbyServer::_closeClientSocket(int i)
{
    closesocket(clientSockets[i]);
    if (i == (clientSockets.size() - 1))
    {
        clientSockets.pop_back();
        clientEvents[clientSockets.size()] = NULL;
    }
    else
    {
        clientSockets[i] = clientSockets.back();
        clientSockets.pop_back();
        clientEvents[i] = clientEvents[clientSockets.size()];
        clientEvents[clientSockets.size()] = NULL;
    }
    
}

void LobbyServer::_closeSocket()
{
    closesocket(sock);
    for (int i = 0; i < clientSockets.size(); i++)
    {
        closesocket(clientSockets[i]);
        clientSockets[i] = NULL;

    }
    WSACleanup();
}

inline bool findAddress(const sockaddr_in& left, std::vector<sockaddr_in>& right)
{
    for (int i = 0; i < right.size(); i++)
    {
        if ((left.sin_port == right[i].sin_port) && (memcmp(&left.sin_addr, &right[i].sin_addr, sizeof(left.sin_addr)) == 0))
        {
            return true;
        }
    }
    return false;
}

inline bool operator==(const sockaddr_in& left, const sockaddr_in& right)
{
    return (left.sin_port == right.sin_port)
        && (memcmp(&left.sin_addr, &right.sin_addr, sizeof(left.sin_addr)) == 0);
}