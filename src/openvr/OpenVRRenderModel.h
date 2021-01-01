#ifndef OPENVR_RENDER_MODEL_H
#define OPENVR_RENDER_MODEL_H

#include "openvr_data.h"
#include <scene/resources/mesh.h>

class OpenVRRenderModel : public ArrayMesh {
	GDCLASS(OpenVRRenderModel, ArrayMesh)

private:
	openvr_data *ovr;

protected:
	static void _bind_methods();

public:
	OpenVRRenderModel();
	~OpenVRRenderModel();

	Array model_names();
	bool load_model(String p_model_name);
};

#endif /* !OPENVR_RENDER_MODEL_H */
