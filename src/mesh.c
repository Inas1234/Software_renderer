#include "mesh.h"
#include <stdlib.h>
#include <string.h> 

static const Vertex CUBE_VERTICES[24] = {
    // Back face (Z=-1)
    {{-1,-1,-1}, {0,0,-1}}, {{ 1,-1,-1}, {0,0,-1}}, {{ 1, 1,-1}, {0,0,-1}}, {{-1, 1,-1}, {0,0,-1}},
    // Front face (Z=+1)
    {{-1,-1, 1}, {0,0, 1}}, {{ 1,-1, 1}, {0,0, 1}}, {{ 1, 1, 1}, {0,0, 1}}, {{-1, 1, 1}, {0,0, 1}},
    // Bottom face (Y=-1)
    {{-1,-1,-1}, {0,-1,0}}, {{-1,-1, 1}, {0,-1,0}}, {{ 1,-1, 1}, {0,-1,0}}, {{ 1,-1,-1}, {0,-1,0}},
    // Top face (Y=+1)
    {{-1, 1,-1}, {0, 1,0}}, {{ 1, 1,-1}, {0, 1,0}}, {{ 1, 1, 1}, {0, 1,0}}, {{-1, 1, 1}, {0, 1,0}},
    // Left face (X=-1)
    {{-1,-1,-1}, {-1,0,0}}, {{-1, 1,-1}, {-1,0,0}}, {{-1, 1, 1}, {-1,0,0}}, {{-1,-1, 1}, {-1,0,0}},
    // Right face (X=+1)
    {{ 1,-1,-1}, { 1,0,0}}, {{ 1,-1, 1}, { 1,0,0}}, {{ 1, 1, 1}, { 1,0,0}}, {{ 1, 1,-1}, { 1,0,0}},
};

// Indices now reference the 24 distinct vertices.
static const int CUBE_INDICES[36] = {
     0, 2, 1,  0, 3, 2, // Back
     4, 5, 6,  4, 6, 7, // Front
     8, 9,10,  8,10,11, // Bottom
    12,13,14, 12,14,15, // Top
    16,17,18, 16,18,19, // Left
    20,21,22, 20,22,23  // Right
};



Mesh* mesh_create_cube() {
    Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));
    if (!mesh) return NULL;

    mesh->num_vertices = 24;
    mesh->num_indices = 36;
    mesh->num_faces = 12;

    mesh->vertices = (Vertex*)malloc(mesh->num_vertices * sizeof(Vertex));
    mesh->indices = (int*)malloc(mesh->num_indices * sizeof(int));
    
    memcpy(mesh->vertices, CUBE_VERTICES, mesh->num_vertices * sizeof(Vertex));
    memcpy(mesh->indices, CUBE_INDICES, mesh->num_indices * sizeof(int));

    return mesh;
}

void mesh_destroy(Mesh* mesh) {
    if (!mesh) return;
    free(mesh->vertices);
    free(mesh->indices);
    free(mesh);
}
