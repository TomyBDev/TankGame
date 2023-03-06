extends Control

var _timer: = 10.0


# Called when the node enters the scene tree for the first time.
func _ready():
	for i in range(0,4):
		if(Global._score[i] == 3):
			get_node("TankDisplay" + String(i)).visible = true

func _process(delta):
	_timer -= delta
	if ((int(_timer) % 1) == 0):
		get_node("Panel/Timer").text = String(int(_timer))
	if (_timer < 0):
		if (Global._host):
			var ip = Global._gameNetwork._getLocalIP()
			Global._gameNetwork._closeSocket() #close the lobby socket
			Global._lobbyNetwork = load("res://Scripts/LobbyServer.gdns").new()
			Global._lobbyNetwork._setUpServer()
		else:
			var ip = Global._gameNetwork._getServerIP()
			Global._gameNetwork._closeSocket() #close the lobby socket
			Global._lobbyNetwork = load("res://Scripts/LobbyClient.gdns").new()
			Global._lobbyNetwork._connectToSock(Global._ip)
		Global._score = [0,0,0,0]
		get_tree().change_scene("res://Levels/Lobby.tscn")
		queue_free()
