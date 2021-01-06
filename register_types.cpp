
#include "register_types.h"
#include "core/object/class_db.h"
#include "src/openvr/OpenVRInterface.h"
#include "src/openvr/OpenVRAction.h"
#include "src/openvr/OpenVRController.h"
#include "src/openvr/OpenVRHaptics.h"
#include "src/openvr/OpenVROverlay.h"
#include "src/openvr/OpenVRPose.h"
#include "src/openvr/OpenVRSkeleton.h"
#include "src/openvr/OpenVRRenderModel.h"


Ref<OpenVRInterface> new_interface = nullptr;

void register_gdopenvr_types() {
	ClassDB::register_virtual_class<OpenVRInterface>();
	ClassDB::register_class<OpenVRAction>();
	ClassDB::register_class<OpenVRController>();
	ClassDB::register_class<OpenVRHaptics>();
	ClassDB::register_class<OpenVROverlay>();
	ClassDB::register_class<OpenVRPose>();
	ClassDB::register_class<OpenVRSkeleton>();
	ClassDB::register_class<OpenVRRenderModel>();

	new_interface = static_cast<Ref<OpenVRInterface>>(memnew(OpenVRInterface));
	new_interface->register_interface();
}

void unregister_gdopenvr_types() {
	if (new_interface == nullptr) {
		return;
	}
	new_interface = nullptr;
}
