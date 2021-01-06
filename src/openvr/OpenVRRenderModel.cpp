#include "OpenVRRenderModel.h"

void OpenVRRenderModel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("model_names"), &OpenVRRenderModel::model_names);
	ClassDB::bind_method(D_METHOD("load_model"), &OpenVRRenderModel::load_model);
}

OpenVRRenderModel::OpenVRRenderModel() {
	ovr = openvr_data::retain_singleton();
}

OpenVRRenderModel::~OpenVRRenderModel() {
	if (ovr != NULL) {
		ovr->remove_mesh(this);
		ovr->release();
		ovr = NULL;
	}
}

Array OpenVRRenderModel::model_names() {
	Array arr;

	if (ovr->is_initialised()) {
		int model_count = ovr->get_render_model_count();
		for (int m = 0; m < model_count; m++) {
			String s = ovr->get_render_model_name(m);

			arr.push_back(s);
		}
	}

	return arr;
}

bool OpenVRRenderModel::load_model(String p_model_name) {
	bool success = true;

	clear_surfaces();

	print_line("Loading: " + p_model_name);
	ovr->load_render_model(p_model_name, this);

	return success;
}
