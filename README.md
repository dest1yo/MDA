# MDA  

## What's this
MDA is a plugin of Unreal Engine that provides an animation blueprint node for using additive animations.

![node_preview](Intro/images/node_preview.png)

## What's special
As described in the label of [Mode](#mode), it is suitable for the additive animations from `Call of Duty` titles.

## Mode
* Add  
  `Location`: BasePose + AdditivePose  
  `Rotation`: BasePose * AdditivePose

* Subtract  
  `Location`: BasePose - AdditivePose  
  `Rotation`: BasePose / AdditivePose

* CoD Add  
  `Location`: BasePose + AdditivePose - ReferencePose  
  `Rotation`: BasePose * AdditivePose

## How to use
* Create new nodes:  
Search for `MDA` in Animation Blueprint.  
![create_new_nodes](Intro/images/create_new_nodes.png)

* Change the modes of layers:  
Options of the modes can be found in the node details panel.  
![node_settings](Intro/images/node_settings.png)

* Add new layer pins:  
Click the button named `Add pin` of the node.  
![add_new_pins](Intro/images/add_new_pins.png)

* Remove layer pins:  
Right-click the layer pin and click the Remove button.  
![remove_pins](Intro/images/remove_pins.png)
