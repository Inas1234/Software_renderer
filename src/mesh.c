#include "mesh.h"
#include <stdlib.h>
#include <string.h> 

static const Vertex CUBE_VERTICES[8] = {
    {{-1,-1,-1}}, {{ 1,-1,-1}}, {{ 1, 1,-1}}, {{-1, 1,-1}},
    {{-1,-1, 1}}, {{ 1,-1, 1}}, {{ 1, 1, 1}}, {{-1, 1, 1}},
};

static const int CUBE_INDICES[36] = {
    0, 2, 1,   0, 3, 2,  
    4, 5, 6,   4, 6, 7,  
    0, 1, 5,   0, 5, 4,  
    3, 6, 2,   3, 7, 6,  
    1, 2, 6,   1, 6, 5,  
    0, 4, 7,   0, 7, 3   
};

static const Color32 CUBE_FACE_COLORS[6] = {
    RGB8(220, 60, 60), RGB8(60, 220, 60), RGB8(60, 60, 220),
    RGB8(220, 180, 60), RGB8(60, 200, 200), RGB8(200, 60, 200)
};

Mesh* mesh_create_cube() {
    Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));
    if (!mesh) return NULL;

    mesh->num_vertices = 8;
    mesh->num_indices = 36;
    mesh->num_faces = 12;

    mesh->vertices = (Vertex*)malloc(mesh->num_vertices * sizeof(Vertex));
    mesh->indices = (int*)malloc(mesh->num_indices * sizeof(int));
    mesh->face_colors = (Color32*)malloc(mesh->num_faces / 2 * sizeof(Color32)); 
    memcpy(mesh->vertices, CUBE_VERTICES, mesh->num_vertices * sizeof(Vertex));
    memcpy(mesh->indices, CUBE_INDICES, mesh->num_indices * sizeof(int));
    memcpy(mesh->face_colors, CUBE_FACE_COLORS, mesh->num_faces / 2 * sizeof(Color32));

    return mesh;
}

void mesh_destroy(Mesh* mesh) {
    if (!mesh) return;
    free(mesh->vertices);
    free(mesh->indices);
    free(mesh->face_colors);
    free(mesh);
}