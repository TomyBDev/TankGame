#ifndef LOBBYCLIENT_H
#define LOBBYCLIENT_H

#include "Common.h"
#include <Node.hpp>
#include <string.hpp>
#include <vector>
#include <map>

class LobbyClient : public Node
{
	GODOT_CLASS(LobbyClient, Node);

private:
	WSADATA w;
	SOCKET sock;
	timeval timeout;
	sockaddr_in servAddr;
	LobbyMessage msg;
	String serverIP;

public:
	static void _register_methods();

	void _init();
	bool _connectToSock(String ip);

	bool _getMsgPlayerReady();
	int _getMsgKey();
	String _getServerIP();

	int _recieveMsg();
	void _sendMsg(int id, int key, bool pReady);
	void _closeSocket();
};
#endif