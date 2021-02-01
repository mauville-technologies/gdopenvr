#include "OpenVRController.h"

void OpenVRController::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_process"), &OpenVRController::_process);

	ClassDB::bind_method(D_METHOD("get_button_actions"), &OpenVRController::get_button_actions);
	ClassDB::bind_method(D_METHOD("set_button_actions"), &OpenVRController::set_button_actions);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "button_actions", PROPERTY_HINT_NONE), "set_button_actions", "get_button_actions");

	ClassDB::bind_method(D_METHOD("get_analog"), &OpenVRController::get_analog);
	ClassDB::bind_method(D_METHOD("trigger_haptic"), &OpenVRController::trigger_haptic);

	ADD_SIGNAL(MethodInfo(String("activated")));
	ADD_SIGNAL(MethodInfo(String("deactivated")));
	ADD_SIGNAL(MethodInfo(String("action_pressed"), PropertyInfo(Variant::STRING, "action_name")));
	ADD_SIGNAL(MethodInfo(String("action_released"), PropertyInfo(Variant::STRING, "action_name")));
}

void OpenVRController::_process(float delta) {
	// controller id 1 matches up with our left hand and 2 with our right hand...
	int on_hand = (int)get_controller_id();
	if (on_hand != 1 && on_hand != 2) {
		return;
	}

	if (ovr != NULL) {
		for (std::vector<input_action>::iterator it = button_actions.begin(); it != button_actions.end(); ++it) {
			bool is_pressed = ovr->get_custom_digital_data(it->action_id, on_hand);

			if (is_pressed != it->last_state) {
				Array arr = Array();
				arr.push_back(it->name);

				it->last_state = is_pressed;
				if (it->last_state) {
					emit_signal("action_pressed", arr);
				} else {
					emit_signal("action_released", arr);
				}
			}
		}
	}
}

OpenVRController::OpenVRController() {
	ovr = openvr_data::retain_singleton();

	// TODO if we ever end up parsing our action.json we can load up button_actions automatically here
}

OpenVRController::~OpenVRController() {
	if (ovr != NULL) {
		ovr->release();
		ovr = NULL;
	}
}

PackedStringArray OpenVRController::get_button_actions() {
	PackedStringArray actions;

	for (std::vector<input_action>::iterator it = button_actions.begin(); it != button_actions.end(); ++it) {
		actions.push_back(it->name);
	}

	return actions;
}

void OpenVRController::set_button_actions(PackedStringArray p_actions) {
	// we're assuming this only gets set once so we can be a little careless and start anew
	button_actions.clear();

	for (int i = 0; i < p_actions.size(); i++) {
		input_action new_action;

		new_action.name = p_actions[i];
		if (ovr != NULL) {
			new_action.action_id = ovr->register_custom_action(new_action.name);
		} else {
			new_action.action_id = -1;
		}
		new_action.last_state = false;

		button_actions.push_back(new_action);
	}
}

Vector2 OpenVRController::get_analog(String p_action) {
	Vector2 analog;

	int on_hand = (int)get_controller_id();
	if (on_hand != 1 && on_hand != 2) {
		return analog;
	}

	if (ovr != NULL) {
		// TODO improve this, this lookup is more costly then I'd like even though it will only register our action the first time it is called.
		int action_id = ovr->register_custom_action(p_action);
		analog = ovr->get_custom_analog_data(action_id, on_hand);
	}

	return analog;
}

void OpenVRController::trigger_haptic(String p_action, float p_duration, float p_frequency, float p_amplitude) {
	int on_hand = (int)get_controller_id();
	if (on_hand != 1 && on_hand != 2) {
		return;
	}

	if (ovr != NULL) {
		// TODO improve this, this lookup is more costly then I'd like even though it will only register our action the first time it is called.
		int action_id = ovr->register_custom_action(p_action);
		ovr->trigger_custom_haptic(action_id, 0.0f, p_duration, p_frequency, p_amplitude, on_hand);
	}
}
