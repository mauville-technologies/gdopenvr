////////////////////////////////////////////////////////////////////////////////////////////////
// Helper calls and singleton container for accessing openvr

#ifndef GDOPENVR_DATA_H
#define GDOPENVR_DATA_H

#include "modules/gdopenvr/lib/openvr/headers/openvr.h"
#include <scene/resources/mesh.h>
#include <vector>
#include <core/templates/vector.h>

class openvr_data {
public:
	// enums
	enum OpenVRApplicationType {
		OTHER,
		SCENE,
		OVERLAY
	};

	enum OpenVRTrackingUniverse {
		SEATED,
		STANDING,
		RAW
	};

private:
	static openvr_data *singleton;

	int use_count;

	vr::IVRRenderModels *render_models;

	// structure to record which overlays go with which viewport
	struct overlay {
		vr::VROverlayHandle_t handle;
		RID viewport_rid;
	};

	std::vector<overlay> overlays;

	OpenVRApplicationType application_type;
	OpenVRTrackingUniverse tracking_universe;

	vr::IVRChaperone *chaperone;
	bool play_area_is_dirty;
	Vector3 play_area[4];
	void update_play_area();

	// interact with openvr
	char *get_device_name(vr::TrackedDeviceIndex_t p_tracked_device_index, uint32_t pMaxLen);
	int32_t get_controller_role(vr::TrackedDeviceIndex_t p_tracked_device_index);
	bool is_tracked_device_connected(vr::TrackedDeviceIndex_t p_tracked_device_index);
	vr::TrackedDeviceClass get_tracked_device_class(vr::TrackedDeviceIndex_t p_tracked_device_index);

	// actions
	struct action_set {
		vr::VRActionSetHandle_t handle;
		String name;
		bool is_active;
	};

	std::vector<action_set> action_sets;

	std::vector<vr::VRActiveActionSet_t> active_action_sets;
	int active_action_set_count = 0;

	Ref<Image> image;

	enum DeviceInputActionHandles {
		DAH_IN_TRIGGER,
		DAH_IN_ANALOG_TRIGGER,
		DAH_IN_GRIP,
		DAH_IN_ANALOG_GRIP,
		DAH_IN_ANALOG,
		DAH_IN_ANALOG_CLICK,
		DAH_IN_BUTTON_AX,
		DAH_IN_BUTTON_BY,
		DAH_IN_MAX
	};

	enum DeviceOutputActionHandles {
		DAH_OUT_HAPTIC,
		DAH_OUT_MAX
	};

	vr::VRActionHandle_t input_action_handles[DAH_IN_MAX];
	vr::VRActionHandle_t output_action_handles[DAH_OUT_MAX];

	void bind_default_action_handles();

	// tracked devices
	struct tracked_device {
		int64_t tracker_id;
		uint64_t last_rumble_update;

		/* add our controller source */
		vr::VRInputValueHandle_t source_handle;
	};

	bool device_hands_are_available;
	uint32_t left_hand_device;
	uint32_t right_hand_device;

	tracked_device tracked_devices[vr::k_unMaxTrackedDeviceCount];

	void attach_device(uint32_t p_device_index);
	void detach_device(uint32_t p_device_index);
	void process_device_actions(tracked_device *p_device, uint64_t p_msec);

	Transform hmd_transform;

	// custom actions
	struct custom_action {
		vr::VRActionHandle_t handle;
		String name;
	};

	std::vector<custom_action> custom_actions;

	// structure to record which model we're loading for our mesh so we can async load this.
	struct model_mesh {
		char model_name[1024];
		ArrayMesh *mesh;
	};

	std::vector<model_mesh> load_models;

	bool _load_render_model(model_mesh *p_model);

	// structure to record which texture we're loading for our mesh so we can async load this.
	enum TextureType {
		TT_ALBEDO,
		TT_MAX
	};

	struct texture_material {
		TextureType type;
		vr::TextureID_t texture_id;
		Ref<StandardMaterial3D> material;
	};

	std::vector<texture_material> load_textures;

	void load_texture(TextureType p_type, vr::TextureID_t p_texture_id, Ref<StandardMaterial3D> p_material);
	bool _load_texture(texture_material *p_texture);

public:
	vr::IVRSystem *hmd; // make this private?

	openvr_data();
	~openvr_data();
	void cleanup();

	static openvr_data *retain_singleton();
	void release();

	bool is_initialised();
	bool initialise();
	void process();

	// properties
	int get_overlay_count();
	overlay get_overlay(int p_overlay_id);
	int add_overlay(vr::VROverlayHandle_t p_new_value, RID p_viewport_rid);
	void remove_overlay(int p_overlay_id);
	OpenVRApplicationType get_application_type();
	void set_application_type(OpenVRApplicationType p_new_value);
	OpenVRTrackingUniverse get_tracking_universe();
	void set_tracking_universe(OpenVRTrackingUniverse p_new_value);
	bool play_area_available() const;
	const Vector3 *get_play_area() const;

	// interact with openvr
	void get_recommended_rendertarget_size(uint32_t *p_width, uint32_t *p_height);
	void get_eye_to_head_transform(Transform *p_transform, int p_eye, float p_world_scale = 1.0);

	// interact with tracking info
	String get_default_action_set() const;
	void set_default_action_set(const String p_name);
	const Transform *get_hmd_transform() const;
	int register_action_set(const String p_action_set);
	void set_active_action_set(const String p_action_set);
	void toggle_action_set_active(const String p_action_set, bool p_is_active);
	bool is_action_set_active(const String p_action_set) const;

	int register_custom_action(const String p_action);
	vr::VRActionHandle_t get_custom_handle(int p_action_idx);
	bool get_custom_pose_data(int p_action_idx, vr::InputPoseActionData_t *p_data, int p_on_hand = 0);
	bool get_custom_digital_data(int p_action_idx, int p_on_hand = 0);
	Vector2 get_custom_analog_data(int p_action_idx, int p_on_hand = 0);
	bool trigger_custom_haptic(int p_action_idx, float p_start_from_now, float p_duration, float p_frequency, float p_amplitude, int p_on_hand = 0);

	// interact with render models
	uint32_t get_render_model_count();
	String get_render_model_name(uint32_t p_model_index);
	void load_render_model(const String &p_model_name, ArrayMesh *p_mesh);

	// clear async data
	void remove_mesh(ArrayMesh *p_mesh);

	// helper functions
	void transform_from_matrix(Transform *p_dest, vr::HmdMatrix34_t *p_matrix, float p_world_scale);
	void matrix_from_transform(vr::HmdMatrix34_t *p_matrix, Transform *p_transform, float p_world_scale);
	void transform_from_bone(Transform &p_transform, const vr::VRBoneTransform_t *p_bone_transform);
};

#endif /* !OPENVR_DATA_H */
