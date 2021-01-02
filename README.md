# OpenVR Module for Godot

Module implementation using https://github.com/GodotVR/godot_openvr as a base.

Working with GDNative modules is cumbersome at best and I don't like it. So I'm putting it in module form. 

This will make it easy to pin to godot versions, include in heavily customized engines and also make it infinitely easier to debug as it will be much more tightly integrated into the engine.

___

```
cd godot/modules
git clone --recursive https://github.com/mauville-technologies/gdopenvr
```

This should be all that's needed.

___

## Usage

The goal of this module is to adhere as closely to this documentation as possible:
https://github.com/GodotVR/godot-openvr-asset/wiki

### Differences

1) All types contained within `gdns` files in the original module become built-in types.
1) `actions` folder should be located in either your project directory or your Godot engine directory
(https://github.com/GodotVR/godot_openvr/tree/master/demo/actions)
1) OpenVRConfig doesn't exist and its functions can be accessed directly from the interface itself.

```
func _init():
	var arvr_interface : OpenVRInterface = ARVRServer.find_interface("OpenVR")
	arvr_interface.application_type = 2; # 2 overlay, 1 normal. Defaults to normal
	arvr_interface.tracking_universe = 1; # 1 Standing,, 0 SEATED, 2 RAW
	if arvr_interface and arvr_interface.initialize():
		
		pass
```