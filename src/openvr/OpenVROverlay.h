#ifndef OPENVR_OVERLAY_H
#define OPENVR_OVERLAY_H

#include <scene/main/viewport.h>
#include "openvr_data.h"

class OpenVROverlay : public SubViewport {
	GDCLASS(OpenVROverlay, SubViewport)

private:
	openvr_data *ovr;
	vr::VROverlayHandle_t overlay;
	int overlay_id;

	real_t overlay_width_in_meters;
	bool overlay_visible;

protected:
	static void _bind_methods();

public:
	OpenVROverlay();
	~OpenVROverlay();

	void _ready();
	void _exit_tree();

	float get_overlay_width_in_meters() const;
	void set_overlay_width_in_meters(float p_new_size);

	bool is_overlay_visible() const;
	void set_overlay_visible(bool p_visible);

	bool track_relative_to_device(vr::TrackedDeviceIndex_t p_tracked_device_index, Transform p_transform);
	bool overlay_position_absolute(Transform p_transform);

	void _notification(int p_what);
};


#endif /* !OPENVR_OVERLAY_H */
