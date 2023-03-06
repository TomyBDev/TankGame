extends StaticBody2D

var _id: = -1
var _explosion: = preload("res://Prefabs/Explosion.tscn")
var _exp

func _ready():
	_id = int(name.lstrip("Block"))

func _on_ShellDetector_area_entered(area):
	_Dead()

func _Dead():
	if (Global._host):
		Global._gameNetwork._sendMsg(7, Vector2(0,0), _id, 0.0, false, 0.0)
	_exp = _explosion.instance()
	get_parent().add_child(_exp)
	_exp._initialise(position)
	queue_free()
