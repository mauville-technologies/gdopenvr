#include "OpenVRAction.h"

void OpenVRAction::_bind_methods() {
	ADD_SIGNAL(MethodInfo(String("pressed")));
	ADD_SIGNAL(MethodInfo(String("released")));

	ClassDB::bind_method(D_METHOD("_process"), &OpenVRAction::_process);

	ClassDB::bind_method(D_METHOD("get_pressed_action"), &OpenVRAction::get_pressed_action);
	ClassDB::bind_method(D_METHOD("set_pressed_action"), &OpenVRAction::set_pressed_action);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "pressed_action", PROPERTY_HINT_NONE), "set_pressed_action", "get_pressed_action");

	ClassDB::bind_method(D_METHOD("is_pressed"), &OpenVRAction::get_is_pressed);

	ClassDB::bind_method(D_METHOD("get_analog_action"), &OpenVRAction::get_analog_action);
	ClassDB::bind_method(D_METHOD("set_analog_action"), &OpenVRAction::set_analog_action);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "analog_action", PROPERTY_HINT_NONE), "set_analog_action", "get_analog_action");

	ClassDB::bind_method(D_METHOD("get_analog"), &OpenVRAction::get_analog);

	ClassDB::bind_method(D_METHOD("get_on_hand"), &OpenVRAction::get_on_hand);
	ClassDB::bind_method(D_METHOD("set_on_hand"), &OpenVRAction::set_on_hand);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "on_hand", PROPERTY_HINT_NONE, "get on hand", PROPERTY_USAGE_DEFAULT), "set_on_hand", "get_on_hand");
}

void OpenVRAction::_process(float delta) {
	bool was_pressed = is_pressed;
	is_pressed = false;

	is_pressed = ovr->get_custom_digital_data(pressed_action_idx, on_hand);
	analog = ovr->get_custom_analog_data(analog_action_idx, on_hand);

	if (was_pressed && !is_pressed) {
		// signal release
		emit_signal("released");
	} else if (!was_pressed && is_pressed) {
		// signal pressed
		emit_signal("pressed");
	}
}

OpenVRAction::OpenVRAction() {
	ovr = openvr_data::retain_singleton();
	pressed_action_idx = -1;
	analog_action_idx = -1;
	on_hand = 0;
	is_pressed = false;
}

OpenVRAction::~OpenVRAction() {
	if (ovr != NULL) {
		ovr->release();
		ovr = NULL;
	}
}

String OpenVRAction::get_pressed_action() const {
	return pressed_action;
}

void OpenVRAction::set_pressed_action(const String p_action) {
	pressed_action = p_action;
	pressed_action_idx = ovr->register_custom_action(pressed_action);
}

bool OpenVRAction::get_is_pressed() const {
	return is_pressed;
}

String OpenVRAction::get_analog_action() const {
	return analog_action;
}

void OpenVRAction::set_analog_action(const String p_action) {
	analog_action = p_action;
	analog_action_idx = ovr->register_custom_action(analog_action);
}

Vector2 OpenVRAction::get_analog() const {
	return analog;
}

int OpenVRAction::get_on_hand() const {
	return on_hand;
}

void OpenVRAction::set_on_hand(int p_hand) {
	if (p_hand < 0 || p_hand > 2) {
		return;
	}

	on_hand = p_hand;
}
