#ifndef OPENVR_ACTION_H
#define OPENVR_ACTION_H

#include "openvr_data.h"
#include <scene/3d/node_3d.h>

class OpenVRAction : public Node3D {
	GDCLASS(OpenVRAction, Node3D)

private:
	openvr_data *ovr;

	String pressed_action = String();
	int pressed_action_idx;
	bool is_pressed;

	String analog_action;
	int analog_action_idx;
	Vector2 analog;

	int on_hand;

protected:
	static void _bind_methods();

public:

	void _process(float delta);

	OpenVRAction();
	~OpenVRAction();

	String get_pressed_action() const;
	void set_pressed_action(const String p_action);
	bool get_is_pressed() const;

	String get_analog_action() const;
	void set_analog_action(const String p_action);
	Vector2 get_analog() const;

	int get_on_hand() const;
	void set_on_hand(int p_hand);
};


#endif /* !OPENVR_ACTION_H */
