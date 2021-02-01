#include "OpenVRInterface.h"
#include <core/core_bind.h>

void OpenVRInterface::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_application_type"), &OpenVRInterface::get_application_type);
	ClassDB::bind_method(D_METHOD("set_application_type"), &OpenVRInterface::set_application_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "application_type", PROPERTY_HINT_NONE), "set_application_type", "get_application_type");

	ClassDB::bind_method(D_METHOD("get_tracking_universe"), &OpenVRInterface::get_tracking_universe);
	ClassDB::bind_method(D_METHOD("set_tracking_universe"), &OpenVRInterface::set_tracking_universe);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tracking_universe", PROPERTY_HINT_NONE), "set_tracking_universe", "get_tracking_universe");

	ClassDB::bind_method(D_METHOD("get_default_action_set"), &OpenVRInterface::get_default_action_set);
	ClassDB::bind_method(D_METHOD("set_default_action_set"), &OpenVRInterface::set_default_action_set);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_action_set", PROPERTY_HINT_NONE), "set_default_action_set", "get_default_action_set");

	ClassDB::bind_method(D_METHOD("register_action_set"), &OpenVRInterface::register_action_set);
	ClassDB::bind_method(D_METHOD("set_active_action_set"), &OpenVRInterface::set_active_action_set);
	ClassDB::bind_method(D_METHOD("toggle_action_set_active"), &OpenVRInterface::toggle_action_set_active);
	ClassDB::bind_method(D_METHOD("is_action_set_active"), &OpenVRInterface::is_action_set_active);

	ClassDB::bind_method(D_METHOD("play_area_available"), &OpenVRInterface::play_area_available);
	ClassDB::bind_method(D_METHOD("get_play_area"), &OpenVRInterface::get_play_area);
}

OpenVRInterface::OpenVRInterface() {
	arvr_data = static_cast< arvr_data_struct *>(memalloc(sizeof(arvr_data_struct)));
	arvr_data->ovr = openvr_data::retain_singleton();
}

OpenVRInterface::~OpenVRInterface() {
	if (arvr_data != NULL) {
		if (arvr_data->ovr != NULL) {
			// this should have already been called... But just in case...
			uninitialize();

			arvr_data->ovr->release();
			arvr_data->ovr = NULL;
		}

		memfree(arvr_data);
	}
}

StringName OpenVRInterface::get_name() const {
	return String("OpenVR");
}

int OpenVRInterface::get_capabilities() const {
	return XR_STEREO + XR_EXTERNAL;
}

bool OpenVRInterface::is_initialized() const {
	if (arvr_data == NULL) {
		return false;
	}

	if (arvr_data->ovr == NULL) {
		return false;
	}

	return arvr_data->ovr->is_initialised();
}

bool OpenVRInterface::initialize() {
	if (arvr_data->ovr->initialise()) {
		// go and get our recommended target size
		arvr_data->ovr->get_recommended_rendertarget_size(&arvr_data->width, &arvr_data->height);

		XRServer *arvr_server = XRServer::get_singleton();
		if ((arvr_server != NULL) && (arvr_server->get_primary_interface() == NULL)) {
			arvr_server->set_primary_interface(this);
		};
	}

	return arvr_data->ovr->is_initialised();
}

void OpenVRInterface::uninitialize() {
	arvr_data->ovr->cleanup();
}

bool OpenVRInterface::get_anchor_detection_is_enabled() const {
	return false;
}

void OpenVRInterface::set_anchor_detection_is_enabled(bool p_enable) {
}

Size2 OpenVRInterface::get_render_targetsize() {
	Size2 size;

	if (arvr_data->ovr->is_initialised()) {
		// TODO: we should periodically check if the recommended size has changed (the user can adjust this) and if so update our width/height
		// and reset our render texture (RID)
		size = Size2((real_t)arvr_data->width, (real_t)arvr_data->height);
	} else {
		size = Size2(500.0f, 500.0f);
	}

	return size;
}

bool OpenVRInterface::is_stereo() {
	return true;
}

Transform OpenVRInterface::get_transform_for_eye(XRInterface::Eyes p_eye, const Transform &p_cam_transform) {
	Transform transform_for_eye;
	Transform reference_frame = get_reference_frame();
	Transform ret;
	float world_scale = get_worldscale();

	if (p_eye == 0) {
		// we want a monoscopic transform.. shouldn't really apply here
		transform_for_eye;
	} else if (arvr_data->ovr != NULL) {
		arvr_data->ovr->get_eye_to_head_transform(&transform_for_eye, p_eye, world_scale);
	} else {
		// really not needed, just being paranoid..
		Vector3 offset;
		if (p_eye == 1) {
			offset = Vector3(-0.035f * world_scale, 0.0f, 0.0f);
		} else {
			offset = Vector3(0.035f * world_scale, 0.0f, 0.0f);
		};

		transform_for_eye = transform_for_eye.translated(offset);
	};

	// Now construct our full transform, the order may be in reverse, have to test
	ret = p_cam_transform;
	ret = ret * reference_frame;
	ret = ret * *arvr_data->ovr->get_hmd_transform();
	ret = ret * transform_for_eye;

	return ret;
}

void OpenVRInterface::fill_projection_for_eye(float *p_projection, int p_eye, float p_aspect, float p_z_near, float p_z_far) {
	if (arvr_data->ovr->is_initialised()) {
		vr::HmdMatrix44_t matrix = arvr_data->ovr->hmd->GetProjectionMatrix(
				p_eye == 1 ? vr::Eye_Left : vr::Eye_Right, p_z_near, p_z_far);

		int k = 0;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				p_projection[k++] = matrix.m[j][i];
			}
		}
	} else {
		// uhm, should do something here really..
	}
}

CameraMatrix OpenVRInterface::get_projection_for_eye(XRInterface::Eyes p_eye, real_t p_aspect, real_t p_z_near, real_t p_z_far) {
	CameraMatrix cm;
	ERR_FAIL_COND_V(arvr_data == NULL, CameraMatrix());

	fill_projection_for_eye((float *)cm.matrix, (int)p_eye, p_aspect, p_z_near, p_z_far);
	return cm;
}

unsigned int OpenVRInterface::get_external_texture_for_eye(XRInterface::Eyes p_eye) {
	return 0;
}

void OpenVRInterface::commit_for_eye(XRInterface::Eyes p_eye, RID p_render_target, const Rect2 &p_screen_rect) {
	// This function is responsible for outputting the final render buffer for
	// each eye.
	// p_screen_rect will only have a value when we're outputting to the main
	// viewport.

	// For an interface that must output to the main viewport (such as with mobile
	// VR) we should give an error when p_screen_rect is not set
	// For an interface that outputs to an external device we should render a copy
	// of one of the eyes to the main viewport if p_screen_rect is set, and only
	// output to the external device if not.

	Rect2 screen_rect = p_screen_rect;

	if (p_eye == 1 && !screen_rect.has_no_area()) {
		// blit as mono, attempt to keep our aspect ratio and center our render buffer
		Vector2 render_size = get_render_targetsize();

		float new_height = screen_rect.size.x * (render_size.y / render_size.x);
		if (new_height > screen_rect.size.y) {
			screen_rect.position.y = (0.5f * screen_rect.size.y) - (0.5f * new_height);
			screen_rect.size.y = new_height;
		} else {
			float new_width = screen_rect.size.y * (render_size.x / render_size.y);

			screen_rect.position.x = (0.5f * screen_rect.size.x) - (0.5f * new_width);
			screen_rect.size.x = new_width;
		}

		blit(0, &p_render_target, &screen_rect);
	}

	if (arvr_data->ovr->is_initialised()) {
		vr::VRTextureBounds_t bounds;
		bounds.uMin = 0.0;
		bounds.uMax = 1.0;
		bounds.vMin = 0.0;
		bounds.vMax = 1.0;

		uint32_t texid = get_texid(&p_render_target);

		vr::Texture_t eyeTexture = { (void *)(uintptr_t)texid, vr::TextureType_OpenGL, vr::ColorSpace_Auto };

		if (arvr_data->ovr->get_application_type() == openvr_data::OpenVRApplicationType::OVERLAY) {
			// Overlay mode
			if (p_eye == 1) {
				vr::EVROverlayError vrerr;

				// TODO: THIS LOOP CAUSES PROBLEMS.
				for (int i = 0; i < arvr_data->ovr->get_overlay_count(); i++) {
					vr::TextureID_t texidov = 0;
						//DO things here
						//= (vr::TextureID_t)RS::get_singleton()->texture_get_texid(RS::get_singleton()->viewport_get_texture(arvr_data->ovr->get_overlay(i).viewport_rid));

					if (texid == texidov) {
						vrerr = vr::VROverlay()->SetOverlayTexture(arvr_data->ovr->get_overlay(i).handle, &eyeTexture);

						if (vrerr != vr::VROverlayError_None) {
							print_line(String("OpenVR could not set texture for overlay: ") + String::num_int64(vrerr) + String(vr::VROverlay()->GetOverlayErrorNameFromEnum(vrerr)));
						}

						vrerr = vr::VROverlay()->SetOverlayTextureBounds(arvr_data->ovr->get_overlay(i).handle, &bounds);

						if (vrerr != vr::VROverlayError_None) {
							print_line(String("OpenVR could not set textute bounds for overlay: ") + String::num_int64(vrerr) + String(vr::VROverlay()->GetOverlayErrorNameFromEnum(vrerr)));
						}
					}
				}
			}
		} else {
			vr::EVRCompositorError vrerr = vr::VRCompositor()->Submit(p_eye == 1 ? vr::Eye_Left : vr::Eye_Right, &eyeTexture, &bounds);
			if (vrerr != vr::VRCompositorError_None) {
				printf("OpenVR reports: %i\n", vrerr);
			}
		}
	}
}

void OpenVRInterface::process() {
	// this method gets called before every frame is rendered, here is where you
	// should update tracking data, update controllers, etc.
	if (arvr_data->ovr->is_initialised()) {
		// Call process on our ovr system.
		arvr_data->ovr->process();
	}
}

void OpenVRInterface::notification(int p_what) {

}

int OpenVRInterface::get_application_type() const {
	return arvr_data->ovr->get_application_type();
}

void OpenVRInterface::set_application_type(int p_type) {
	arvr_data->ovr->set_application_type((openvr_data::OpenVRApplicationType)p_type);
}

int OpenVRInterface::get_tracking_universe() const {
	return arvr_data->ovr->get_tracking_universe();
}

void OpenVRInterface::set_tracking_universe(int p_universe) {
	arvr_data->ovr->set_tracking_universe((openvr_data::OpenVRTrackingUniverse)p_universe);
}

String OpenVRInterface::get_default_action_set() const {
	return arvr_data->ovr->get_default_action_set();
}

void OpenVRInterface::set_default_action_set(const String p_name) {
	arvr_data->ovr->set_default_action_set(p_name);
}

void OpenVRInterface::register_action_set(const String p_action_set) {
	arvr_data->ovr->register_action_set(p_action_set);
}

void OpenVRInterface::set_active_action_set(const String p_action_set) {
	arvr_data->ovr->set_active_action_set(p_action_set);
}

void OpenVRInterface::toggle_action_set_active(const String p_action_set, const bool p_is_active) {
	arvr_data->ovr->toggle_action_set_active(p_action_set, p_is_active);
}

bool OpenVRInterface::is_action_set_active(const String p_action_set) const {
	return arvr_data->ovr->is_action_set_active(p_action_set);
}

bool OpenVRInterface::play_area_available() const {
	return arvr_data->ovr->play_area_available();
}

PackedVector3Array OpenVRInterface::get_play_area() const {
	XRServer *server = XRServer::get_singleton();

	if (server == NULL) {
		print_error("Failed to get XRServer");
		return PackedVector3Array();
	}

	const Vector3 *play_area = arvr_data->ovr->get_play_area();
	Transform reference = server->get_reference_frame();
	float ws = server->get_world_scale();

	PackedVector3Array arr;
	arr.resize(4);

	{
		Vector3 *w = arr.ptrw();

		for (int i = 0; i < 4; i++) {
			w[i] = reference.xform_inv(play_area[i]) * ws;
		}
	}

	return arr;
}
