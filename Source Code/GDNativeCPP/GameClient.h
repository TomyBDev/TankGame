#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include "Common.h"
#include <Node.hpp>
#include <WinSock2.h>
#include <string.hpp>
#include <vector>
#include <map>

class GameClient : public Node
{
	GODOT_CLASS(GameClient, Node);

private:
	WSADATA w;
	SOCKET sock;
	timeval timeout;
	sockaddr_in servAddr;
	GameMessage msg;
	String serverIP;

public:
	static void _register_methods();

	void _init();

	void _setServerIP(String address);

	String _getServerIP();
	int _getMsgPlayerID();
	Vector2 _getMsgPos();
	float _getMsgBodyRot();
	float _getMsgBarrelRot();
	bool _getMsgStateChange();
	float _getMsgTime();

	int _recieveMsg();
	void _sendMsg(int playerID, Vector2 pos, float bodyRot, float barrelRot, bool stateChange, float time);
	void _closeSocket();
};
#endif