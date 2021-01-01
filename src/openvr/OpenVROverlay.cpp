#include "OpenVROverlay.h"
#include "OpenVRInterface.h"
#include <servers/arvr_server.h>
#include <core/engine.h>

void OpenVROverlay::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_exit_tree"), &OpenVROverlay::_exit_tree);

	ClassDB::bind_method(D_METHOD("is_overlay_visible"), &OpenVROverlay::is_overlay_visible);
	ClassDB::bind_method(D_METHOD("set_overlay_visible"), &OpenVROverlay::set_overlay_visible);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "overlay_visible", PROPERTY_HINT_NONE), "set_overlay_visible", "is_overlay_visible");

	ClassDB::bind_method(D_METHOD("get_overlay_width_in_meters"), &OpenVROverlay::get_overlay_width_in_meters);
	ClassDB::bind_method(D_METHOD("set_overlay_width_in_meters"), &OpenVROverlay::set_overlay_width_in_meters);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "overlay_width_in_meters", PROPERTY_HINT_NONE), "set_overlay_width_in_meters", "get_overlay_width_in_meters");

	ClassDB::bind_method(D_METHOD("track_relative_to_device"), &OpenVROverlay::track_relative_to_device);
	ClassDB::bind_method(D_METHOD("overlay_position_absolute"), &OpenVROverlay::overlay_position_absolute);
}

OpenVROverlay::OpenVROverlay() {
	ovr = openvr_data::retain_singleton();
	overlay_width_in_meters = 1.0;
	overlay_visible = true;

	set_update_mode(Viewport::UpdateMode::UPDATE_ALWAYS);
}

OpenVROverlay::~OpenVROverlay() {
	if (ovr != NULL) {
		ovr->release();
		ovr = NULL;
	}
}

void OpenVROverlay::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	String appname = ProjectSettings::get_singleton()->get_setting("application/config/name");
	String overlay_identifier = appname + String::num_int64(ovr->get_overlay_count() + 1);

	CharString overlay_key = overlay_identifier.ascii();
	CharString overlay_name = overlay_identifier.ascii();

	vr::EVROverlayError vrerr = vr::VROverlay()->CreateOverlay(overlay_key.get_data(), overlay_name.get_data(), &overlay);
	if (vrerr != vr::VROverlayError_None) {
		print_line(String("Could not create overlay, OpenVR error:") + String::num_int64(vrerr) + ", " + String(vr::VROverlay()->GetOverlayErrorNameFromEnum(vrerr)));
	}

	overlay_id = ovr->add_overlay(overlay, get_viewport_rid());

	Transform initial_transform;
	initial_transform = initial_transform.translated(Vector3(0, 0, 1) * -1.4);

	overlay_position_absolute(initial_transform);
	set_overlay_width_in_meters(overlay_width_in_meters);
	set_overlay_visible(overlay_visible);
	set_use_arvr(true);
}

void OpenVROverlay::_exit_tree() {
	if (overlay && !Engine::get_singleton()->is_editor_hint()) {
		vr::EVROverlayError vrerr = vr::VROverlay()->DestroyOverlay(overlay);
		if (vrerr != vr::VROverlayError_None) {
			print_line(String("Could not destroy overlay, OpenVR error: ") + String::num_int64(vrerr) + ", " + String(vr::VROverlay()->GetOverlayErrorNameFromEnum(vrerr)));
		}

		overlay = 0;
		ovr->remove_overlay(overlay_id);
	}
}

float OpenVROverlay::get_overlay_width_in_meters() const {
	if (overlay && !Engine::get_singleton()->is_editor_hint()) {
		float overlay_size;

		vr::VROverlay()->GetOverlayWidthInMeters(overlay, &overlay_size);
		return overlay_size;
	} else {
		if (Engine::get_singleton()->is_editor_hint()) {
			return overlay_width_in_meters;
		}
		return -1;
	}
}

void OpenVROverlay::set_overlay_width_in_meters(float p_new_size) {
	overlay_width_in_meters = p_new_size;


	if (overlay && !Engine::get_singleton()->is_editor_hint()) {
		vr::EVROverlayError vrerr = vr::VROverlay()->SetOverlayWidthInMeters(overlay, p_new_size);

		if (vrerr != vr::VROverlayError_None) {
			print_line(String("Could not set overlay width in meters, OpenVR error: ") + String::num_int64(vrerr) + ", " + String(vr::VROverlay()->GetOverlayErrorNameFromEnum(vrerr)));
		}
	}
}

bool OpenVROverlay::is_overlay_visible() const {
	if (overlay && !Engine::get_singleton()->is_editor_hint()) {
		return vr::VROverlay()->IsOverlayVisible(overlay);
	} else {
		if (Engine::get_singleton()->is_editor_hint()) {
			return overlay_visible;
		}
		return false;
	}
}

void OpenVROverlay::set_overlay_visible(bool p_visible) {
	overlay_visible = p_visible;

	if (overlay && !Engine::get_singleton()->is_editor_hint()) {
		if (p_visible) {
			vr::EVROverlayError vrerr = vr::VROverlay()->ShowOverlay(overlay);

			if (vrerr != vr::VROverlayError_None) {
				print_line(String("Could not show overlay, OpenVR error: ") + String::num_int64(vrerr) + ", " + String(vr::VROverlay()->GetOverlayErrorNameFromEnum(vrerr)));
			}
		} else {
			vr::EVROverlayError vrerr = vr::VROverlay()->HideOverlay(overlay);

			if (vrerr != vr::VROverlayError_None) {
				print_line(String("Could not hide overlay, OpenVR error: ") + String::num_int64(vrerr) + ", " + String(vr::VROverlay()->GetOverlayErrorNameFromEnum(vrerr)));
			}
		}
	}
}

bool OpenVROverlay::track_relative_to_device(vr::TrackedDeviceIndex_t p_tracked_device_index, Transform p_transform) {
	if (overlay && !Engine::get_singleton()->is_editor_hint()) {
		vr::HmdMatrix34_t matrix;
		Ref<OpenVRInterface> ovr_interface = static_cast<Ref<OpenVRInterface> >(ARVRServer::get_singleton()->find_interface("OpenVR"));
		ovr->matrix_from_transform(&matrix, (Transform *)&p_transform, ovr_interface->get_worldscale());

		vr::EVROverlayError vrerr = vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(overlay, p_tracked_device_index, &matrix);

		if (vrerr != vr::VROverlayError_None) {
			print_line(String("Could not track overlay relative to device, OpenVR error: ") + String::num_int64(vrerr) + ", " + String(vr::VROverlay()->GetOverlayErrorNameFromEnum(vrerr)));

			return false;
		}

		return true;
	}
	return false;
}

bool OpenVROverlay::overlay_position_absolute(Transform p_transform) {
	if (overlay && !Engine::get_singleton()->is_editor_hint()) {
		vr::HmdMatrix34_t matrix;
		vr::TrackingUniverseOrigin origin;
		Ref<OpenVRInterface> ovr_interface = static_cast<Ref<OpenVRInterface> >(ARVRServer::get_singleton()->find_interface("OpenVR"));
		ovr->matrix_from_transform(&matrix, (Transform *)&p_transform, ovr_interface->get_worldscale());

		openvr_data::OpenVRTrackingUniverse tracking_universe = ovr->get_tracking_universe();
		if (tracking_universe == openvr_data::OpenVRTrackingUniverse::SEATED) {
			origin = vr::TrackingUniverseSeated;
		} else if (tracking_universe == openvr_data::OpenVRTrackingUniverse::STANDING) {
			origin = vr::TrackingUniverseStanding;
		} else {
			origin = vr::TrackingUniverseRawAndUncalibrated;
		}

		vr::EVROverlayError vrerr = vr::VROverlay()->SetOverlayTransformAbsolute(overlay, origin, &matrix);

		if (vrerr != vr::VROverlayError_None) {
			print_line(String("Could not track overlay absolute, OpenVR error: ") + String::num_int64(vrerr) + ", " + String(vr::VROverlay()->GetOverlayErrorNameFromEnum(vrerr)));

			return false;
		}

		return true;
	}
	return false;
}

void OpenVROverlay::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		_ready();
	}
}

