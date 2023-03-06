extends "res://Scripts/Tank.gd"

var _direction: = Vector2(0,-1)
var _timer:= 0.0

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta) -> void:
	_timer += delta
	handle_Inputs(delta)
	_velocity = move_and_slide(_velocity*_direction*delta, FLOOR_NORMAL)

func handle_Inputs(dt: float) -> void:
	if (Input.get_action_strength("move_forward") == 1):
		_velocity = speed
	else:
		_velocity = Vector2.ZERO
	
	if ((Input.get_action_strength("turn_right") - Input.get_action_strength("turn_left"))!=0):
		if (Input.get_action_strength("turn_right") == 1):
			_direction = _direction.rotated(deg2rad(_rotSpeed*dt))
			bodySprite.rotate(deg2rad(_rotSpeed*dt))
		elif (Input.get_action_strength("turn_left") == 1):
			 _direction = _direction.rotated(-deg2rad(_rotSpeed*dt))
			 bodySprite.rotate(-deg2rad(_rotSpeed*dt))

	barrelSprite.set_rotation(get_local_mouse_position().angle()+1.5708)

	if (Input.get_action_strength("shoot") == 1 and _timer > 1.5):
		get_parent().get_parent()._createShell(position, get_local_mouse_position().angle()+1.5708, true)
		_timer = 0.0

func get_BodyRot() -> float:
	return bodySprite.get_rotation()

func get_BarrelRot() -> float:
	return barrelSprite.get_rotation()

func _on_ShellDetector_body_entered(body):
	if (_timer > 0.25):
		Global._gameNetwork._sendMsg(Global._id, position, 0.0, 0.0, true, 0.0)
		_exp = _explosion.instance()
		get_parent().add_child(_exp)
		_exp._initialise(position)
		get_parent().get_parent()._playerAlive[_id] = false
		if (Global._host):
			get_parent().get_parent()._checkWin()
		queue_free()
