extends Node2D

var _timeElapsed: = 0.0
var _interval: = 0.0
var _lastMsgtime
var _clientActivity:= 0.0


var tank: = []

var _playerAlive:= [false, false, false, false]

var _shells: = []
var _shell: = preload("res://Prefabs/Shell.tscn")
var _blankShell: = preload("res://Prefabs/BlankShell.tscn")

onready var _network = Global._gameNetwork #Stores the network script in variable
onready var id = Global._id #Stores the network script in variable

# Called when the node enters the scene tree for the first time.
func _ready():
	#An array of the tanks for easy access
	for i in range(0,4):
		if Global._playerExists[i]:
			_playerAlive[i] = true
			tank.push_back(get_node("TileMap/Tank" + String(i)))
		else:
			 tank.push_back(null)
			 get_node("TileMap/Tank" + String(i)).queue_free()
	
	#Deletes the enemy node corresponding to the current player and recreates it as a player node
	var tempPos = tank[id].position
	get_node("TileMap/Tank" + String(id)).queue_free()
	tank[id] = load("res://Prefabs/Player.tscn").instance()
	get_node("TileMap").add_child(tank[id])
	tank[id].get_node("TankBodySprite").texture = load("res://Assets/Tank" + String(id) + "Body.png")
	tank[id].get_node("TankBarrelSprite").texture = load("res://Assets/Tank" + String(id) + "Barrel.png")
	tank[id].position = tempPos
	tank[id]._id = id
	
	if (Global._host):
		_lastMsgtime = [0.0,0.0,0.0,0.0]
	else:
		0.0

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	_timeElapsed += delta
	get_node("FPS").text = String(Engine.get_frames_per_second())
	
	var i = _network._recieveMsg()
	
	if ((i != id) and (i >= 0) and (i < 3) and _playerAlive[i]):
		tank[i].addMessage(_network._getMsgPos(), _network._getMsgBodyRot(), _network._getMsgBarrelRot(), _network._getMsgTime())
		if (_network._getMsgStateChange()):
			tank[i].dead()
			if (Global._host):
				_checkWin()
			return
		
		if (!Global._host):
			_lastMsgtime = _timeElapsed

	if (i == 5):
		_createShell(_network._getMsgPos(), _network._getMsgBodyRot(), false)
		if (Global._host):
			_shells.back()._id = _shells.size()-1
			_network._sendMsg(5, _network._getMsgPos(), _network._getMsgBodyRot(), _shells.size()+5, false, _timeElapsed)

	if (i == 6):
		if (_network._getMsgStateChange()):
			_shells[_network._getMsgBodyRot()]._dead()
			return
		_shells[_network._getMsgBarrelRot()]._sync(_network._getMsgPos(), _network._getMsgBodyRot(), _network._getMsgTime())

	if (i == 7):
		get_node("TileMap/Block" + String(_network._getMsgBodyRot()))._Dead()

	if (i == 8):
		Global._score[_network._getMsgBodyRot()] += 1
		get_tree().change_scene("res://Levels/RoundCard.tscn")
		queue_free()
		
	if (i == 9):
		Global._score[_network._getMsgBodyRot()] += 1
		_network._closeSocket()
		get_tree().change_scene("res://Levels/GameCard.tscn")
		queue_free()
	
	if (i == 10):
		var temp = _network._getMsgBodyRot()
		tank[temp].visible = false
		Global._playerExists[temp] = false
		_playerAlive[temp] = false

	
	if (i == 11):
		_lastMsgtime[_network._getMsgBodyRot()] = _timeElapsed
	
	if (_interval < _timeElapsed and _playerAlive[id]):
		_network._sendMsg(id, tank[id].position, tank[id].get_BodyRot(), tank[id].get_BarrelRot(), false, _timeElapsed)
		_interval += 0.1
	
	if (_clientActivity < _timeElapsed):
		if (Global._host):
			for n in range(1, _playerAlive.count(true)):
				if (_timeElapsed - _lastMsgtime[n] > 10):
					_network._removeClient(n)
					Global._playerExists[n] = false
					_playerAlive[n] = false
					tank[n].visible = false
					_network._sendMsg(10, Vector2(0,0), n, 0.0, false, 0.0)
		else:
			_network._sendMsg(11, Vector2(0,0), Global._id, 0.0, false, 0.0)
			if (_timeElapsed - _lastMsgtime > 10):
				_network._closeSocket()
				Global._dc = true
				get_tree().change_scene("res://Levels/CreateOrJoin.tscn")
		_clientActivity += 5

func _createShell(pos: Vector2, dir: float, b: bool):
	if (Global._host):
		_shells.push_back(_shell.instance())
		add_child(_shells.back())
		_shells.back()._initialise(pos, dir)
		if (b):
			_shells.back()._id = _shells.size()-1
			_network._sendMsg(5, pos, dir, 0.0, false, _timeElapsed)
	else:
		if (b):
			_network._sendMsg(5, pos, dir, 0.0, false, _timeElapsed)
			return
		_shells.push_back(_blankShell.instance())
		add_child(_shells.back())
		_shells.back()._initialise(pos, dir)

func _removeShell(_id: int):
	if (Global._host):
		_network._sendMsg(6, Vector2(0,0), _id, 0.0, true, 0.0)
	_shells[_id].queue_free()
	if (_id != _shells.size()-1):
		_shells[_id] = _shells.back()
		_shells[_id]._id = _id
	_shells.pop_back()

func _checkWin():
	if(_playerAlive.count(true)==1):
		var temp = _playerAlive.find(true)
		Global._score[temp] += 1
		if (Global._score[temp] == 3):
			get_tree().change_scene("res://Levels/GameCard.tscn")
			_network._sendMsg(9, Vector2(0,0), temp, 0.0, false, 0.0)
			_network._closeSocket()
			queue_free()
			return
		get_tree().change_scene("res://Levels/RoundCard.tscn")
		_network._sendMsg(8, Vector2(0,0), temp, 0.0, false, 0.0)
		queue_free()
