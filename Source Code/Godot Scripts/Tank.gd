extends KinematicBody2D

const FLOOR_NORMAL: = Vector2.UP

export var speed: = Vector2(10000, 10000)
export var _id: = -1

var _velocity: = Vector2.ZERO
var _rotSpeed: = 100.0
var _explosion: = preload("res://Prefabs/Explosion.tscn")
var _exp

onready var barrelSprite = $TankBarrelSprite
onready var bodySprite = $TankBodySprite
