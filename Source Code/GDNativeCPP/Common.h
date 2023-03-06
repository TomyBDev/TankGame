#ifndef COMMON_H
#define COMMON_H

#include <Godot.hpp>
#include <WinSock2.h>

using namespace std;
using namespace godot;

struct Position
{
    float x, y;
};

struct LobbyMessage
{
    int playerID;
    int key;
    bool playerReady;
};

struct GameMessage
{
    int playerID;
    Position pos;
    float bodyRot;
    float barrelRot;
    bool stateChange;
    float time;
};

#endif