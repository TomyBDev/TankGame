extends Control

func _ready():
	if (Global._dc):
		Global._dc = false
		get_node("ServerLost").visible = true

func _on_CreateButton_pressed(): #Sets the client to host and loads the lobby server script
	Global._lobbyNetwork = load("res://Scripts/LobbyServer.gdns").new()
	if (Global._lobbyNetwork._setUpServer()):
		Global._host = true
		Global._id = 0
		Global._ip = Global._lobbyNetwork._getLocalIP()
		get_tree().change_scene("res://Levels/Lobby.tscn") #Change to the Lobby Scene
		queue_free()
	else:
		get_node("ServerCreateFailed").visible = true

func _on_JoinButton_pressed(): #Sets the client not to host
	Global._lobbyNetwork = load("res://Scripts/LobbyClient.gdns").new()
	if (Global._lobbyNetwork._connectToSock(get_node("Tabs/JoinLobbyPanel/EnterIP").text)):
		Global._ip = get_node("Tabs/JoinLobbyPanel/EnterIP").text
		get_tree().change_scene("res://Levels/Lobby.tscn")
		queue_free()
	else:
		get_node("ServerJoinFailed").visible = true

func _on_ClientRetryButton_button_up():
	get_node("ServerJoinFailed").visible = false
	_on_JoinButton_pressed()


func _on_ClientCloseButton_button_up():
	get_node("ServerJoinFailed").visible = false


func _on_ServerRetryButton_button_up():
	get_node("ServerCreateFailed").visible = false
	_on_CreateButton_pressed()


func _on_ServerCloseButton_button_up():
	get_node("ServerCreateFailed").visible = false


func _on_CloseButton_button_up():
	get_node("ServerLost").visible = false
