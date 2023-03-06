extends Node2D

onready var Exp = $ExplosionAnim

func _initialise(pos: Vector2):
	position = pos
	Exp.play("Default")

func _on_ExplosionAnim_animation_finished():
	Exp.stop()
	queue_free()
