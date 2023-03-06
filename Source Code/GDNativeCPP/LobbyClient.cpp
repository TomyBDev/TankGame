#include "LobbyClient.h"
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

// The IP address for the server
#define SERVERIP "127.0.0.1"

// The UDP port number for the server
#define SERVERPORT 4444

bool findAddress(const sockaddr_in& left, std::vector<sockaddr_in>& right);
bool operator==(const sockaddr_in& left, const sockaddr_in& right);

void LobbyClient::_register_methods()
{
    register_method("_connectToSock", &LobbyClient::_connectToSock);
    register_method("_getMsgPlayerReady", &LobbyClient::_getMsgPlayerReady);
    register_method("_getMsgKey", &LobbyClient::_getMsgKey);
    register_method("_getServerIP", &LobbyClient::_getServerIP);

    register_method("_recieveMsg", &LobbyClient::_recieveMsg);
    register_method("_sendMsg", &LobbyClient::_sendMsg);
    register_method("_closeSocket", &LobbyClient::_closeSocket);
}

void LobbyClient::_init()
{
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;
    msg.key = 0;
    msg.playerID = -1;
    msg.playerReady = false;

    // Initialise the WinSock library -- we want version 2.2.
    int error = WSAStartup(0x0202, &w);
    if (error != 0)
    {
        std::cout << "WSAStartup failed\n";
    }
    if (w.wVersion != 0x0202)
    {
        std::cout << "Wrong WinSock version\n";
    }

    // Create a TCP socket.
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cout << "socket failed\n";
    }
}

bool LobbyClient::_connectToSock(String ip)
{
    serverIP = ip;

    // Fill out a sockaddr_in structure to describe the address we'll listen on.
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip.alloc_c_string());
    // htons converts the port number to network byte order (big-endian).
    serverAddr.sin_port = htons(SERVERPORT);

    // inet_ntoa formats an IP address as a string.
    std::cout << "IP address to send to: " << inet_ntoa(serverAddr.sin_addr) << "\n";
    // ntohs does the opposite of htons.
    std::cout << "Port number to send to: " << ntohs(serverAddr.sin_port) << "\n\n";

    // Connect the socket to the server.
    if (connect(sock, (const sockaddr*)&serverAddr, sizeof serverAddr) == SOCKET_ERROR)
    {
        std::cout << "connect failed\n";
        return false;
    }
    return true;
}

bool LobbyClient::_getMsgPlayerReady()
{
    return msg.playerReady;
}

int LobbyClient::_getMsgKey()
{
    return msg.key;
}

String LobbyClient::_getServerIP()
{
    return serverIP;
}

int LobbyClient::_recieveMsg()
{
    fd_set readable;
    FD_ZERO(&readable);
    FD_SET(sock, &readable);

    // Wait for one of the sockets to become readable.

    int count = select(0, &readable, NULL, NULL, &timeout);
    if (count == SOCKET_ERROR)
    {
        std::cout << "select failed\n";
    }
    if (count > 0)
    {
        std::cout << "Socket are ready to be read from.\n";
    }

    if (FD_ISSET(sock, &readable))
    {
        sockaddr_in fromAddr;
        int fromAddrSize = sizeof(fromAddr);
        SOCKET clientSocket = recv(sock, (char*)&msg, sizeof(LobbyMessage), 0);

        std::cout << "Received object: " << msg.playerID << ", with key: " << msg.key << ", with ready state of: " << msg.playerReady << "\n";

        return msg.playerID;
    }
    return -1;
}

void LobbyClient::_sendMsg(int id, int key, bool pReady)
{
    LobbyMessage message;
    message.key = key;
    message.playerID = id;
    message.playerReady = pReady;

    int count = send(sock, (const char*)&message, sizeof(LobbyMessage), 0);
    if (count != sizeof(LobbyMessage))
    {
        std::cout << "sendto failed\n";
    }
}

void LobbyClient::_closeSocket()
{
    closesocket(sock);
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