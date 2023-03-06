#ifndef LOBBYSERVER_H
#define LOBBYSERVER_H

#include "Common.h"
#include <Node.hpp>
#include <string.hpp>
#include <vector>
#include <list>

class LobbyServer : public Node
{
	GODOT_CLASS(LobbyServer, Node);

private:
	WSADATA w;
	SOCKET sock;
	WSAEVENT ListenEvent;
	timeval timeout;
	std::vector<SOCKET> clientSockets;
	WSAEVENT clientEvents[3];
	int clientsID[3];
	WSANETWORKEVENTS NetworkEvents;
	DWORD returnVal;
	int eventIndex;
	LobbyMessage msg;

public:
	static void _register_methods();

	void _init();
	bool _setUpServer();

	String _getLocalIP();
	bool _getMsgPlayerReady();
	int _getMsgKey();

	void _listenForConn();
	int _recieveMsg();
	void _sendMsg(int id, int key, bool pReady);
	void _closeClientSocket(int i);
	void _closeSocket();
};
#endif