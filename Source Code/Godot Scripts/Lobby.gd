extends Control

var id = 0 #Player ID (will be updated later for clients)

onready var _network = Global._lobbyNetwork #Stores the network script in variable
onready var host = Global._host #Checks if we are host (server) or client

var playersReady = [false, false, false, false] #Array for holding if the players are ready
var playerExists = [true, false,false,false] #Array for hold if player exists
var levels = ["Iron Garden", "Factory Line", "Metal Maze"]
var currentLevel = 0

# Called when the node enters the scene tree for the first time.
func _ready():
	if (host):
		Global._id = 0
		get_node("IP").text = _network._getLocalIP() #Gets the IP for displaying on screen
		get_node("TankDisplay1/Panel/Tank Name").add_color_override("font_color", Color(1, 1, 0, 1))
		get_node("Map Panel/Left Button").visible = true
		get_node("Map Panel/Right Button").visible = true

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if (host):
		_network._listenForConn()
		_serverRecieveFunc()
	else:
		_clientRecieveFunc()

func _serverRecieveFunc():
	var i = _network._recieveMsg()
	var j = _network._getMsgKey()
	if (i > 0):
		if (j == 1 or j == 2): #If first message from client
			get_node("TankDisplay" + String(i+1)).visible = true #Sets the client representation to visible
			playerExists[i] = true #Sets the client to existing :D
			_network._sendMsg(id,0,playersReady[id])
		_updateReady(i) #stores if the player is ready and changes the text to show it visually

		if (j == 4):
			_swapClients(i)
			_network._sendMsg(i, 4, false)

func _clientRecieveFunc():
	var i = _network._recieveMsg()
	
	if (i == 0): #Button Update
		_updateReady(0)
		
	if (i > 0 and i < 4):
		_updateReady(i)
		var j = _network._getMsgKey()
		if (j == 1 or j == 2): #New client detected, tell that client you exist
			get_node("TankDisplay" + String(i+1)).visible = true #Display the new tank 
			playerExists[i] = true
			_network._sendMsg(id, 3, playersReady[id])
			
		if (j == 2): #Server Telling current client its id so it knows which tank to be
			id = i
			_network._sendMsg(i, 1, false)
			get_node("TankDisplay" + String(i+1) + "/Panel/Tank Name").add_color_override("font_color", Color(1, 1, 0, 1))
			
		if (j == 3): #Client Telling you it exists
			get_node("TankDisplay" + String(i+1)).visible = true #Display the new tank 
			playerExists[i] = true
		
		if (j == 4): #Player Disconnected
			_swapClients(i)
		
	if (i == 4): #Host Disconnected
		_network._closeSocket()
		get_tree().change_scene("res://Levels/CreateOrJoin.tscn")
		queue_free()
		
	if (i == 5): #Game Start
		var temp = _network._getServerIP()
		_network._closeSocket() #close the lobby socket
		Global._id = id
		Global._playerExists = playerExists
		Global._currentScene = String("res://Levels/" + levels[currentLevel] + ".tscn")
		
		_network._closeSocket() #close the lobby socket
		Global._gameNetwork = load("res://Scripts/GameClient.gdns").new()
		Global._gameNetwork._setServerIP(temp)

		get_tree().change_scene(Global._currentScene)
		queue_free()
		
	if (i == 6):
		currentLevel = _network._getMsgKey()
		_mapUpdate()

#ID 0 is server ready state update
#ID 1-3 is 
#ID 4 is Host Disconnect
#ID 5 is Game Start
#ID 6 is Map Update

#--- Key 1 is a new client has connect (send a new message saying you exist to the new client)
#--- Key 2 is you are the new client and are told your id
#--- Key 3 same as 1, but doesn't send a message (this is to stop infinite sending back and forth)
#--- Key 4 is client has disconnected so removed them from screen.

func _on_Ready_Button_button_up():
	if (playersReady[id]):
		playersReady[id] = false
		get_node("Ready Panel/Ready Button").set_normal_texture(load("res://Assets/ReadyButton2.png"))
		get_node("TankDisplay" + String(id+1) + "/Panel/Ready Image").texture = load("res://Assets/NotReady.png")
	else:
		playersReady[id] = true
		get_node("Ready Panel/Ready Button").set_normal_texture(load("res://Assets/ReadyButton1.png"))
		get_node("TankDisplay" + String(id+1) + "/Panel/Ready Image").texture = load("res://Assets/Ready.png")
	
	_network._sendMsg(id,0,playersReady[id])
	if (host): #If we are the host
		_gameReady()

func _updateReady(i: int):
	playersReady[i] = _network._getMsgPlayerReady()
	if (playersReady[i]):
		get_node("TankDisplay" + String(i+1) + "/Panel/Ready Image").texture = load("res://Assets/Ready.png")
	else:
		get_node("TankDisplay" + String(i+1) + "/Panel/Ready Image").texture = load("res://Assets/NotReady.png")
	
	if (host):
		_gameReady()

func _gameReady():
	if (playersReady.count(true) == max(playerExists.count(true),2)): #If the number of players ready is equal to the number of clients+server (at least 2)
		_network._sendMsg(5,0, true)
		_network._closeSocket()
		Global._gameNetwork = load("res://Scripts/GameServer.gdns").new()
		Global._gameNetwork._bindSocket(get_node("IP").text, playerExists.count(true))
		Global._playerExists = playerExists
		Global._currentScene = String("res://Levels/" + levels[currentLevel] + ".tscn")
		get_tree().change_scene(Global._currentScene)
		queue_free()

func _mapUpdate():
	get_node("Map Panel/Map Picture").texture = load("res://Assets/MapPrev"+String(currentLevel)+".png")
	get_node("Map Panel/Map Name").texture = load("res://Assets/MapName"+String(currentLevel)+".png")

func _swapClients(i :int):
	if (playerExists.find_last(true) == i):
		playerExists[i] = false
		playersReady[i] = false
		get_node("TankDisplay" + String(i+1)).visible = false #Hide disconnected tank
	else:
		var temp = playerExists.find_last(true)
		playerExists[temp] = false
		playersReady[temp] = false
		get_node("TankDisplay" + String(temp+1)).visible = false #Hide disconnected tank
		if (id == temp):
			id = i
			get_node("TankDisplay" + String(i+1) + "/Panel/Tank Name").add_color_override("font_color", Color(1, 1, 0, 1))
			get_node("TankDisplay" + String(temp+1) + "/Panel/Tank Name").add_color_override("font_color", Color(1, 1, 1, 1))

func _on_Leave_Button_button_up():
	if (host):
		_network._sendMsg(4,0, true)
	_network._closeSocket()
	get_tree().change_scene("res://Levels/CreateOrJoin.tscn")
	queue_free()


func _on_Right_Button_button_up():
	if (currentLevel < 2):
		currentLevel += 1
	else:
		currentLevel = 0
	_network._sendMsg(6, currentLevel, false)
	_mapUpdate()

func _on_Left_Button_button_up():
	if (currentLevel > 0):
		currentLevel -= 1
	else:
		currentLevel = 2
	_network._sendMsg(6, currentLevel, false)
	_mapUpdate()
