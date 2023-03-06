extends KinematicBody2D

var _velocity
var _speed: = 400
var _id: = -1
var _timer: = 0.0
var _count: = 0
var _explosion: = preload("res://Prefabs/Explosion.tscn")
var _exp
var _alive = true

func _initialise(pos: Vector2, ang: float):
	_velocity = Vector2.RIGHT.rotated(ang-1.5708) * _speed
	rotation = _velocity.angle()+1.5708
	position = pos + (_velocity.normalized()*40)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta) -> void:
	_timer += delta
	var _collision = move_and_collide(_velocity*delta)
	if (_collision):
		_velocity = _velocity.bounce(_collision.normal)
		rotation = _velocity.angle()+1.5708
	
	if (_timer > 0.5):
		Global._gameNetwork._sendMsg(6, position, _velocity.angle(), _id, false, get_parent()._timeElapsed)
		_timer = 0.0
		_count += 1
		if (_count > 19):
			_on_Collision_Detector_area_entered(Area2D)

func _sync(pos: Vector2, ang: float, time: float) -> void:
	if (ang != _velocity.angle()):
		_velocity = Vector2.RIGHT.rotated(ang) * _speed
		rotation = _velocity.angle()+1.5708
	if (pos != position):
		position = pos + (_velocity*(abs(time-get_parent()._timeElapsed)))

func _getPos() -> Vector2:
	return position

func _getRot() -> float:
	return rotation

func _on_Collision_Detector_area_entered(area):

	_exp = _explosion.instance()
	get_parent().add_child(_exp)
	_exp._initialise(position)
	if (_alive):
		_alive = false
		get_parent()._removeShell(_id)
