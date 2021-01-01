#ifndef GDOVR_ARVR_INTERFACE_H
#define GDOVR_ARVR_INTERFACE_H

#include <openvr.h>
#include <servers/arvr/arvr_interface.h>
#include <servers/visual/visual_server_globals.h>
#include <servers/arvr/arvr_positional_tracker.h>
#include <core/os/input.h>
#include <servers/visual/visual_server_globals.h>
#include <main/input_default.h>

#include "openvr_data.h"

class OpenVRInterface : public ARVRInterface {
	GDCLASS(OpenVRInterface, ARVRInterface);

	typedef struct arvr_data_struct {
		openvr_data *ovr;
		uint32_t width;
		uint32_t height;

		int video_driver;
		int texture_id;
	} arvr_data_struct;

	arvr_data_struct *arvr_data = NULL;

private:
protected:
	static void _bind_methods();

public:
	OpenVRInterface();
	~OpenVRInterface();
	StringName get_name() const override;
	int get_capabilities() const override;

	bool is_initialized() const override; /* returns true if we've initialized this interface */
	bool initialize() override; /* initialize this interface, if this has an HMD it becomes the primary interface */
	void uninitialize() override; /* deinitialize this interface */


	/** specific to VR **/
	// nothing yet

	/** specific to AR **/
	bool get_anchor_detection_is_enabled() const override;
	void set_anchor_detection_is_enabled(bool p_enable) override;

	/** rendering and internal **/

	Size2 get_render_targetsize() override; /* returns the recommended render target size per eye for this device */
	bool is_stereo() override; /* returns true if this interface requires stereo rendering (for VR HMDs) or mono rendering (for mobile AR) */
	Transform get_transform_for_eye(ARVRInterface::Eyes p_eye, const Transform &p_cam_transform) override; /* get each eyes camera transform, also implement EYE_MONO */
	void fill_projection_for_eye(float *p_projection, int p_eye, float p_aspect, float p_z_near, float p_z_far);
	CameraMatrix get_projection_for_eye(ARVRInterface::Eyes p_eye, real_t p_aspect, real_t p_z_near, real_t p_z_far) override; /* get each eyes projection matrix */
	unsigned int get_external_texture_for_eye(ARVRInterface::Eyes p_eye) override; /* if applicable return external texture to render to */
	void commit_for_eye(ARVRInterface::Eyes p_eye, RID p_render_target, const Rect2 &p_screen_rect) override; /* output the left or right eye */

	void process() override;
	void notification(int p_what) override;

	/************************************************************************/
	// CONFIGURATION BINDINGS
	int get_application_type() const;
	void set_application_type(int p_type);

	int get_tracking_universe() const;
	void set_tracking_universe(int p_universe);

	String get_default_action_set() const;
	void set_default_action_set(const String p_name);

	void register_action_set(const String p_action_set);
	void set_active_action_set(const String p_action_set);
	void toggle_action_set_active(const String p_action_set, const bool p_is_active);
	bool is_action_set_active(const String p_action_set) const;

	bool play_area_available() const;
	PoolVector3Array get_play_area() const;









	/************************************************************************/
	/* Utility functions / Glue to ARVRServer */
	std::vector<ARVRPositionalTracker *> trackers{};

	float get_worldscale() {
		ARVRServer *arvr_server = ARVRServer::get_singleton();
		ERR_FAIL_NULL_V(arvr_server, 1.0);

		return arvr_server->get_world_scale();
	}

	void set_controller_transform(int p_controller_id, Transform *p_transform, bool p_tracks_orientation, bool p_tracks_position) {
		ARVRServer *arvr_server = ARVRServer::get_singleton();
		ERR_FAIL_NULL(arvr_server);

		ARVRPositionalTracker *tracker = arvr_server->find_by_type_and_id(ARVRServer::TRACKER_CONTROLLER, p_controller_id);
		if (tracker != NULL) {
			Transform *transform = (Transform *)p_transform;
			if (p_tracks_orientation) {
				tracker->set_orientation(transform->basis);
			}
			if (p_tracks_position) {
				tracker->set_rw_position(transform->origin);
			}
		}
	}

	int add_controller(char *p_device_name, int p_hand, bool p_tracks_orientation, bool p_tracks_position) {
		ARVRServer *arvr_server = ARVRServer::get_singleton();
		ERR_FAIL_NULL_V(arvr_server, 0);

		InputDefault *input = (InputDefault *)Input::get_singleton();
		ERR_FAIL_NULL_V(input, 0);

		ARVRPositionalTracker *new_tracker = memnew(ARVRPositionalTracker);
		trackers.push_back(new_tracker);

		new_tracker->set_name(p_device_name);
		new_tracker->set_type(ARVRServer::TRACKER_CONTROLLER);
		if (p_hand == 1) {
			new_tracker->set_hand(ARVRPositionalTracker::TRACKER_LEFT_HAND);
		} else if (p_hand == 2) {
			new_tracker->set_hand(ARVRPositionalTracker::TRACKER_RIGHT_HAND);
		}

		// also register as joystick...
		int joyid = input->get_unused_joy_id();
		if (joyid != -1) {
			new_tracker->set_joy_id(joyid);
			input->joy_connection_changed(joyid, true, p_device_name, "");
		}

		if (p_tracks_orientation) {
			Basis orientation;
			new_tracker->set_orientation(orientation);
		}
		if (p_tracks_position) {
			Vector3 position;
			new_tracker->set_position(position);
		}

		// add our tracker to our server and remember its pointer
		arvr_server->add_tracker(new_tracker);

		// note, this ID is only unique within controllers!
		return new_tracker->get_tracker_id();
	}

	void remove_controller(int p_controller_id) {
		ARVRServer *arvr_server = ARVRServer::get_singleton();
		ERR_FAIL_NULL(arvr_server);

		InputDefault *input = (InputDefault *)Input::get_singleton();
		ERR_FAIL_NULL(input);

		ARVRPositionalTracker *remove_tracker = arvr_server->find_by_type_and_id(ARVRServer::TRACKER_CONTROLLER, p_controller_id);
		if (remove_tracker != NULL) {
			// unset our joystick if applicable
			int joyid = remove_tracker->get_joy_id();
			if (joyid != -1) {
				input->joy_connection_changed(joyid, false, "", "");
				remove_tracker->set_joy_id(-1);
			}

			// remove our tracker from our server
			arvr_server->remove_tracker(remove_tracker);
			memdelete(remove_tracker);
		}
	}

	void set_controller_button(int p_controller_id, int p_button, bool p_is_pressed) {
		ARVRServer *arvr_server = ARVRServer::get_singleton();
		ERR_FAIL_NULL(arvr_server);

		InputDefault *input = (InputDefault *)Input::get_singleton();
		ERR_FAIL_NULL(input);

		ARVRPositionalTracker *tracker = arvr_server->find_by_type_and_id(ARVRServer::TRACKER_CONTROLLER, p_controller_id);
		if (tracker != NULL) {
			int joyid = tracker->get_joy_id();
			if (joyid != -1) {
				input->joy_button(joyid, p_button, p_is_pressed);
			}
		}
	}

	void set_controller_axis(int p_controller_id, int p_axis, float p_value, float p_can_be_negative) {
		ARVRServer *arvr_server = ARVRServer::get_singleton();
		ERR_FAIL_NULL(arvr_server);

		InputDefault *input = (InputDefault *)Input::get_singleton();
		ERR_FAIL_NULL(input);

		ARVRPositionalTracker *tracker = arvr_server->find_by_type_and_id(ARVRServer::TRACKER_CONTROLLER, p_controller_id);
		if (tracker != NULL) {
			int joyid = tracker->get_joy_id();
			if (joyid != -1) {
				InputDefault::JoyAxis jx;
				jx.min = p_can_be_negative ? -1 : 0;
				jx.value = p_value;
				input->joy_axis(joyid, p_axis, jx);
			}
		}
	}

	float get_controller_rumble(int p_controller_id) {
		ARVRServer *arvr_server = ARVRServer::get_singleton();
		ERR_FAIL_NULL_V(arvr_server, 0.0);

		ARVRPositionalTracker *tracker = arvr_server->find_by_type_and_id(ARVRServer::TRACKER_CONTROLLER, p_controller_id);
		if (tracker != NULL) {
			return tracker->get_rumble();
		}

		return 0.0;
	}

	void register_interface() {
		ARVRServer::get_singleton()->add_interface(this);
	}

	void unregister_interface() {
		ARVRServer::get_singleton()->remove_interface(this);
	}

	Transform get_reference_frame() {
		Transform reference_frame;
		Transform *reference_frame_ptr = (Transform *)&reference_frame;

		ARVRServer *arvr_server = ARVRServer::get_singleton();
		if (arvr_server != NULL) {
			*reference_frame_ptr = arvr_server->get_reference_frame();
		}

		return reference_frame;
	}

	void blit(int p_eye, RID *p_render_target, Rect2 *p_rect) {
		// blits out our texture as is, handy for preview display of one of the eyes that is already rendered with lens distortion on an external HMD
		ARVRInterface::Eyes eye = (ARVRInterface::Eyes)p_eye;
		RID *render_target = (RID *)p_render_target;
		Rect2 screen_rect = *(Rect2 *)p_rect;

		if (eye == ARVRInterface::EYE_LEFT) {
			screen_rect.size.x /= 2.0;
		} else if (p_eye == ARVRInterface::EYE_RIGHT) {
			screen_rect.size.x /= 2.0;
			screen_rect.position.x += screen_rect.size.x;
		}

		VSG::rasterizer->set_current_render_target(RID());
		VSG::rasterizer->blit_render_target_to_screen(*render_target, screen_rect, 0);
	}

	int get_texid(RID *p_render_target) {
		// In order to send off our textures to display on our hardware we need the opengl texture ID instead of the render target RID
		// This is a handy function to expose that.
		RID *render_target = (RID *)p_render_target;

		RID eye_texture = VSG::storage->render_target_get_texture(*render_target);
		uint32_t texid = VS::get_singleton()->texture_get_texid(eye_texture);

		return texid;
	}
};

#endif
