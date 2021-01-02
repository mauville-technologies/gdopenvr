#ifndef OPENVR_CONTROLLER_H
#define OPENVR_CONTROLLER_H

#include "openvr_data.h"
#include <scene/3d/arvr_nodes.h>

#include <vector>

class OpenVRController : public ARVRController {
	GDCLASS(OpenVRController, ARVRController)

private:
	openvr_data *ovr;

	struct input_action {
		String name;
		int action_id;
		bool last_state;
	};

	std::vector<input_action> button_actions;

protected:
	static void _bind_methods();

public:
	void _process(float delta);

	OpenVRController();
	~OpenVRController();

	PoolStringArray get_button_actions();
	void set_button_actions(PoolStringArray p_actions);

	Vector2 get_analog(String p_action);
	void trigger_haptic(String p_action, float p_duration, float p_frequency, float p_amplitude);
};

#endif /* !OPENVR_CONTROLLER_H */
