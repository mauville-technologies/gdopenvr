#ifndef OPENVR_HAPTICS_H
#define OPENVR_HAPTICS_H

#include "openvr_data.h"
#include <scene/3d/spatial.h>

class OpenVRHaptics : public Spatial {
	GDCLASS(OpenVRHaptics, Spatial)

private:
	openvr_data *ovr;

	String action;
	int action_idx;

	int on_hand;

	float duration;
	float frequency;
	float amplitude;

protected:
	static void _bind_methods();

public:

	OpenVRHaptics();
	~OpenVRHaptics();

	String get_action() const;
	void set_action(String p_action);

	int get_on_hand() const;
	void set_on_hand(int p_hand);

	void set_duration(float duration);
	float get_duration() const;
	void set_frequency(float frequency);
	float get_frequency() const;
	void set_amplitude(float amplitude);
	float get_amplitude() const;

	void trigger_pulse();
};

#endif /* !OPENVR_HAPTICS_H */
