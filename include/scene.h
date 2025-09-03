#pragma once
#include "mathx.h"
#include "mesh.h"

typedef struct {
    Vec3 position;
    Vec3 rotation; 
    Vec3 scale;
} Transform;

typedef struct {
    Mesh* mesh;
    Transform transform;
} SceneObject;

Mat4 transform_get_matrix(const Transform* t);