#ifndef OPENVR_POSE_H
#define OPENVR_POSE_H

#include "openvr_data.h"
#include <servers/xr_server.h>
#include <scene/3d/node_3d.h>

class OpenVRPose : public Node3D {
	GDCLASS(OpenVRPose, Node3D)

private:
	openvr_data *ovr;
	XRServer *server;

	String action;
	int action_idx;
	int on_hand;

	bool is_active;

protected:
	static void _bind_methods();

public:
	void _process(float delta);

	OpenVRPose();
	~OpenVRPose();

	String get_action() const;
	void set_action(String p_action);
	bool get_is_active() const;

	int get_on_hand() const;
	void set_on_hand(int p_hand);
};

#endif /* !OPENVR_POSE_H */
