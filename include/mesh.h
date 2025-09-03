#pragma once
#include "mathx.h"
#include "color.h"

typedef struct {
    Vec3 pos;
    Vec3 normal;
} Vertex;

typedef struct {
    Vertex* vertices;
    int*    indices;

    int num_vertices;
    int num_indices;
    int num_faces;
} Mesh;

Mesh* mesh_create_cube();
void  mesh_destroy(Mesh* mesh);