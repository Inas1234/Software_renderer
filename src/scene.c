#include "scene.h"
#include "mathx.h"

Mat4 transform_get_matrix(const Transform* t) {
    Mat4 scale_mat = mat4_scaling(t->scale.x, t->scale.y, t->scale.z);
    Mat4 rot_x_mat = mat4_rotation_x(t->rotation.x);
    Mat4 rot_y_mat = mat4_rotation_y(t->rotation.y);
    Mat4 rot_z_mat = mat4_rotation_z(t->rotation.z);
    Mat4 trans_mat = mat4_translation(t->position.x, t->position.y, t->position.z);

    Mat4 rot_mat = mat4_mul(rot_z_mat, mat4_mul(rot_y_mat, rot_x_mat));
    Mat4 model_mat = mat4_mul(trans_mat, mat4_mul(rot_mat, scale_mat));

    return model_mat;
}