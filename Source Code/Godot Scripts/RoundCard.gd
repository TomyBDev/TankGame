extends Control

var _timer: = 7.0


# Called when the node enters the scene tree for the first time.
func _ready():
	for i in range(0,4):
		if(Global._playerExists[i]):
			get_node("TankDisplay" + String(i)).visible = true
			get_node("TankDisplay" + String(i) + "/Panel/Score").text = String(Global._score[i])

func _process(delta):
	_timer -= delta
	if ((int(_timer) % 1) == 0):
		get_node("Panel/Timer").text = String(int(_timer))
	if (_timer < 0):
		get_tree().change_scene(Global._currentScene)
