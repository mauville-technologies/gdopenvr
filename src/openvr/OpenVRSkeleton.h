#ifndef OPENVR_SKELETON_H
#define OPENVR_SKELETON_H

#include "openvr_data.h"
#include <scene/3d/skeleton_3d.h>

#include <stdlib.h>

class OpenVRSkeleton : public Skeleton3D {
	GDCLASS(OpenVRSkeleton, Skeleton3D)

private:
	openvr_data *ovr;

	String action;
	int action_idx;
	int on_hand;

	bool is_active;
	bool keep_bones;

	struct bone {
		vr::BoneIndex_t parent;
		char name[256];
		Transform rest_transform;
		Transform pose_transform;
	};

	vr::EVRSkeletalTransformSpace transform_space;
	vr::EVRSkeletalMotionRange motion_range;
	vr::EVRSkeletalReferencePose reference_pose;

	uint32_t bone_count;
	bone *bones;
	void cleanup_bones();

protected:
	static void _bind_methods();

public:
	void _process(float delta);

	OpenVRSkeleton();
	~OpenVRSkeleton();

	String get_action() const;
	void set_action(String p_action);

	bool get_keep_bones() const;
	void set_keep_bones(bool p_keep_bones);

	int get_motion_range() const;
	void set_motion_range(int p_motion_range);

	bool get_is_active() const;
};

#endif /* !OPENVR_SKELETON_H */
