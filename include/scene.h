#pragma once
#include "mathx.h"
#include "mesh.h"
#include "draw.h"
    
typedef struct {
    Vec3 position;
    Vec3 rotation; 
    Vec3 scale;
} Transform;

typedef struct {
    Mesh* mesh;
    Transform transform;
    Color32 base_color;
} SceneObject;

Mat4 transform_get_matrix(const Transform* t);