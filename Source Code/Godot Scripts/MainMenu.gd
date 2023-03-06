extends Control

func _on_PlayButton_pressed():
	get_tree().change_scene("res://Levels/CreateOrJoin.tscn") #Change to Create or Join Scene
	

func _on_QuitButton_pressed(): #Quit
	get_tree().quit()
