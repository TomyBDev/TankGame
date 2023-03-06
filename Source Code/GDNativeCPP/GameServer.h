#ifndef GAMESERVER_H
#define GAMESERVER_H

#include "Common.h"
#include <Node.hpp>
#include <WinSock2.h>
#include <string.hpp>
#include <vector>
#include <map>

class GameServer : public Node
{
	GODOT_CLASS(GameServer, Node);

private:
	WSADATA w;
	SOCKET sock;
	timeval timeout;
	std::vector<sockaddr_in> clientAddresses;
	GameMessage msg;
	int clientCount;
	String serverIP;

public:
	static void _register_methods();

	void _init();

	void _bindSocket(String address, int cCount);

	String _getLocalIP();
	int _getMsgPlayerID();
	Vector2 _getMsgPos();
	float _getMsgBodyRot();
	float _getMsgBarrelRot();
	bool _getMsgStateChange();
	float _getMsgTime();

	int _recieveMsg();
	void _sendMsg(int playerID, Vector2 pos, float bodyRot, float barrelRot, bool stateChange, float time);
	void _closeSocket();
	void _removeClient(int id);
};
#endif