#include "GameClient.h"
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

// The IP address for the server
#define SERVERIP "127.0.0.1"

// The UDP port number for the server
#define SERVERPORT 4444

bool findAddress(const sockaddr_in& left, std::vector<sockaddr_in>& right);
bool operator==(const sockaddr_in& left, const sockaddr_in& right);

void GameClient::_register_methods()
{
    register_method("_setServerIP", &GameClient::_setServerIP);

    register_method("_getServerIP", &GameClient::_getServerIP);
    register_method("_getMsgPlayerID", &GameClient::_getMsgPlayerID);
    register_method("_getMsgPos", &GameClient::_getMsgPos);
    register_method("_getMsgBodyRot", &GameClient::_getMsgBodyRot);
    register_method("_getMsgBarrelRot", &GameClient::_getMsgBarrelRot);
    register_method("_getMsgStateChange", &GameClient::_getMsgStateChange);
    register_method("_getMsgTime", &GameClient::_getMsgTime);

    register_method("_recieveMsg", &GameClient::_recieveMsg);
    register_method("_sendMsg", &GameClient::_sendMsg);
    register_method("_closeSocket", &GameClient::_closeSocket);
}

void GameClient::_init()
{
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    msg.playerID = 0;
    msg.pos.x = 0;
    msg.pos.y = 0;
    msg.stateChange = false;
    msg.time = 0;

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

    // Create a UDP socket.
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cout << "socket failed\n";
    }

}

void GameClient::_setServerIP(String address)
{
    serverIP = address;
    servAddr.sin_family = AF_INET;
    // htons converts the port number to network byte order (big-endian).
    servAddr.sin_port = htons(SERVERPORT);
    servAddr.sin_addr.s_addr = inet_addr(address.alloc_c_string());
    //servAddr.sin_addr.s_addr = inet_addr(SERVERIP);

    // inet_ntoa formats an IP address as a string.
    std::cout << "IP address to send to: " << inet_ntoa(servAddr.sin_addr) << "\n";
    // ntohs does the opposite of htons.
    std::cout << "Port number to send to: " << ntohs(servAddr.sin_port) << "\n\n";

    _sendMsg(-1, Vector2(0.0f, 0.0f), 0.0f, 0.0f, false, 0.0f);
    std::cout << "First message Sent to Server\n";

}

String GameClient::_getServerIP()
{
    return serverIP;
}

int GameClient::_getMsgPlayerID()
{
    return msg.playerID;
}

Vector2 GameClient::_getMsgPos()
{
    return Vector2(msg.pos.x, msg.pos.y);
}

float GameClient::_getMsgBodyRot()
{
    return msg.bodyRot;
}

float GameClient::_getMsgBarrelRot() 
{
    return msg.barrelRot;
}

bool GameClient::_getMsgStateChange()
{
    return msg.stateChange;
}

float GameClient::_getMsgTime()
{
    return msg.time;
}

int GameClient::_recieveMsg()
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
        //std::cout << "Socket are ready to be read from.\n";
    }

    if (FD_ISSET(sock, &readable))
    {
        // Read a response back from the server (or from anyone, in fact).
        sockaddr_in fromAddr;
        int fromAddrSize = sizeof(fromAddr);
        int count = recvfrom(sock, (char*)&msg, sizeof(GameMessage), 0, (sockaddr*)&fromAddr, &fromAddrSize);
        if (count > 0)
        {
            //std::cout << "Received object: " << msg.playerID << ", with position: " << msg.pos.x << ", " << msg.pos.y << ". With state change of: " << msg.stateChange << ". At time: " << msg.time << ". from address " << inet_ntoa(fromAddr.sin_addr) << " port: " << ntohs(fromAddr.sin_port) << "\n";
            return msg.playerID;
        }
    }
    return -1;
}

void GameClient::_sendMsg(int playerID, Vector2 pos, float bodyRot, float barrelRot, bool stateChange, float time)
{
    GameMessage message;
    message.playerID = playerID;
    message.pos.x = pos.x;
    message.pos.y = pos.y;
    message.bodyRot = bodyRot;
    message.barrelRot = barrelRot;
    message.stateChange = stateChange;
    message.time = time;

    if (sendto(sock, (const char*)&message, sizeof(GameMessage), 0, (const sockaddr*)&servAddr, sizeof(servAddr)) != sizeof(GameMessage))
    {
        std::cout << "sendto failed\n";
    }
}

void GameClient::_closeSocket()
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