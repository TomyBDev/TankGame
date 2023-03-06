#include "GameServer.h"
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

// The IP address for the server
#define SERVERIP "127.0.0.1"

// The UDP port number for the server
#define SERVERPORT 4444

bool findAddress(const sockaddr_in& left, std::vector<sockaddr_in>& right);
bool operator==(const sockaddr_in& left, const sockaddr_in& right);

void GameServer::_register_methods()
{
    register_method("_bindSocket", &GameServer::_bindSocket);
    
    register_method("_getLocalIP", &GameServer::_getLocalIP);
    register_method("_getMsgPlayerID", &GameServer::_getMsgPlayerID);
    register_method("_getMsgPos", &GameServer::_getMsgPos);
    register_method("_getMsgBodyRot", &GameServer::_getMsgBodyRot);
    register_method("_getMsgBarrelRot", &GameServer::_getMsgBarrelRot);
    register_method("_getMsgStateChange", &GameServer::_getMsgStateChange);
    register_method("_getMsgTime", &GameServer::_getMsgTime);

    register_method("_recieveMsg", &GameServer::_recieveMsg);
    register_method("_sendMsg", &GameServer::_sendMsg);
    register_method("_closeSocket", &GameServer::_closeSocket);
    register_method("_removeClient", &GameServer::_removeClient);
}

void GameServer::_init()
{
    //Init Variables
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    msg.playerID = 0;
    msg.pos.x = 0;
    msg.pos.y = 0;
    msg.stateChange = false;
    msg.time = 0;

    //Init Server

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

void GameServer::_bindSocket(String address, int cCount)
{
    clientCount = cCount;
    serverIP = address;

    // Fill out a sockaddr_in structure to describe the address we'll listen on.
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(address.alloc_c_string());
    // htons converts the port number to network byte order (big-endian).
    serverAddr.sin_port = htons(SERVERPORT);

    // Bind the socket to that address.
    if (bind(sock, (const sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
    {
        std::cout << "bind failed\n";
    }

    std::cout << "Server socket bound to address " << inet_ntoa(serverAddr.sin_addr) << " port " << ntohs(serverAddr.sin_port) << "\n";
}

String GameServer::_getLocalIP()
{
    return serverIP;
}

int GameServer::_getMsgPlayerID()
{
    return msg.playerID;
}

Vector2 GameServer::_getMsgPos()
{
    return Vector2(msg.pos.x, msg.pos.y);
}

float GameServer::_getMsgBodyRot()
{
    return msg.bodyRot;
}

float GameServer::_getMsgBarrelRot()
{
    return msg.barrelRot;
}


bool GameServer::_getMsgStateChange()
{
    return msg.stateChange;
}

float GameServer::_getMsgTime()
{
    return msg.time;
}

int GameServer::_recieveMsg()
{
    //Readbility stuff

    fd_set readable;
    FD_ZERO(&readable);
    FD_SET(sock, &readable);

    int count = select(0, &readable, NULL, NULL, &timeout);
    if (count == SOCKET_ERROR)
    {
        std::cout << "select failed\n";
    }

    //std::cout << count << " sockets are ready to be read from.\n";

    //If a socket is ready to be read from

    if (FD_ISSET(sock, &readable))
    {
        //Read from the socket storing the message in msg and storing the address in fromAddr

        sockaddr_in fromAddr;
        int fromAddrSize = sizeof(fromAddr);
        int count = recvfrom(sock, (char*)&msg, sizeof(GameMessage), 0,
            (sockaddr*)&fromAddr, &fromAddrSize); //Recieve message

        //std::cout << "Received object: " << msg.playerID << ", with position: " << msg.pos.x << ", " << msg.pos.y << ". With state change of: " << msg.stateChange << ". At time: " << msg.time << ". from address " << inet_ntoa(fromAddr.sin_addr) << " port: " << ntohs(fromAddr.sin_port) << "\n";

        //If new client and there is less than 4 clients

        if (!findAddress(fromAddr, clientAddresses) && (clientAddresses.size() < 3))
        {
            clientAddresses.push_back(fromAddr); //Add client address to vector

            std::cout << "New Client Connected, client number = " << clientAddresses.size() << "\n";
        }
        //std::cout << "Received " << count << " bytes from address " << inet_ntoa(fromAddr.sin_addr) << " port " << ntohs(fromAddr.sin_port) << ":\n";

        for (int i = 0; i < clientAddresses.size(); i++) //Loop through all clients
        {
            if (!(fromAddr == clientAddresses[i])) //If the client isn't the client just recieved from
            {
                if (sendto(sock, (const char*)&msg, sizeof(GameMessage), 0,
                    (const sockaddr*)&clientAddresses[i], sizeof(clientAddresses[i])) != sizeof(GameMessage)) //Send message just recieved to clients
                {
                    std::cout << "sendto failed\n";
                }
            }
        }
        return msg.playerID;
    }
    return -1;
}

void GameServer::_sendMsg(int playerID, Vector2 pos, float bodyRot, float barrelRot, bool stateChange, float time)
{
    GameMessage message;
    message.playerID = playerID;
    message.pos.x = pos.x;
    message.pos.y = pos.y;
    message.bodyRot = bodyRot;
    message.barrelRot = barrelRot;
    message.stateChange = stateChange;
    message.time = time;

    for (int i = 0; i < clientAddresses.size(); i++) //Loop through all clients
    {
        if (sendto(sock, (const char*)&message, sizeof(GameMessage), 0,
            (const sockaddr*)&clientAddresses[i], sizeof(clientAddresses[i])) != sizeof(GameMessage)) //Send message just recieved to clients
        {
            std::cout << "sendto failed\n";
        }
    }
}

void GameServer::_closeSocket()
{
    closesocket(sock);
    WSACleanup();
}

void GameServer::_removeClient(int id)
{
    clientAddresses.erase(clientAddresses.begin() + (id-1));
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