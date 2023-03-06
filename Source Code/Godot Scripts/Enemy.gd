extends "res://Scripts/Tank.gd"

var _messagePos: = []
var _messageTime: = []
var _bodyRot: = 0.0
var _barrelRot: = 0.0
var _timeElapsed:= 0.0
var _currentTime:= 0.0

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta) -> void:
	_timeElapsed += delta #increment time
	position = prediction(delta)
	bodySprite.set_rotation(_bodyRot)
	barrelSprite.set_rotation(_barrelRot)

func prediction(dt: float) -> Vector2:
	if (_messagePos.size() < 3): #If less than 3 messages sent don't move
		return position
	var speedl = (_messagePos[2] - _messagePos[1]) / (_messageTime[2] - _messageTime[1])
	var displacement = speedl * (_timeElapsed-_messageTime[2])
	var _predictedPos = _messagePos[2] + displacement

	return _predictedPos

func addMessage(msgPos: Vector2, bodyRot: float, barrelRot: float, msgTime: float):	
	if(_messagePos.size() > 2):
		
		if (msgTime == _messageTime.back()): #Check for duplicate packet
			return
			
		for n in range(2,-1, -1):
			if (_messageTime[n] < msgTime):
				_messagePos.insert(n+1, msgPos)
				_messageTime.insert(n+1, msgTime)
				break
		
		_messagePos.pop_front()
		_messageTime.pop_front()
		_bodyRot = bodyRot
		_barrelRot = barrelRot
	else:
		_messagePos.push_back(msgPos)
		_messageTime.push_back(msgTime)
		_bodyRot = bodyRot
		_barrelRot = barrelRot

func dead():
	_exp = _explosion.instance()
	get_parent().add_child(_exp)
	_exp._initialise(position)
	get_parent().get_parent()._playerAlive[_id] = false
	queue_free()
