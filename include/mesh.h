#pragma once
#include "mathx.h"
#include "color.h"

typedef struct {
    Vec3 pos;
} Vertex;

typedef struct {
    Vertex* vertices;
    int*    indices;
    Color32* face_colors; 

    int num_vertices;
    int num_indices;
    int num_faces;
} Mesh;

Mesh* mesh_create_cube();
void  mesh_destroy(Mesh* mesh);