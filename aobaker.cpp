#include <thekla_atlas.h>

#include <cstdio>
#include <cassert>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace Thekla;
using namespace std;

void raytrace(const char* meshobj, const char* coordsbin, const char* normsbin,
    const char* resultpng, int nsamples);

static Atlas_Input_Mesh* obj_mesh_load(const char* filename)
{
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;
    string err;
    bool ret = tinyobj::LoadObj(shapes, materials, err, filename);
    if (!ret) {
        printf("%s\n", err.c_str());
        return NULL;
    }

    printf("%lu shapes\n", shapes.size());
    assert(shapes.size() > 0);

    Atlas_Input_Mesh* mesh = new Atlas_Input_Mesh();
    mesh->vertex_count = shapes[0].mesh.positions.size() / 3;
    mesh->vertex_array = new Atlas_Input_Vertex[mesh->vertex_count];
    for (int nvert = 0; nvert < mesh->vertex_count; nvert++) {
        const auto& pos = shapes[0].mesh.positions;
        mesh->vertex_array[nvert].position[0] = pos[nvert * 3];
        mesh->vertex_array[nvert].position[1] = pos[nvert * 3 + 1];
        mesh->vertex_array[nvert].position[2] = pos[nvert * 3 + 2];
        const auto& nor = shapes[0].mesh.normals;
        if (nor.size() > 0) {
            mesh->vertex_array[nvert].normal[0] = nor[nvert * 3];
            mesh->vertex_array[nvert].normal[1] = nor[nvert * 3 + 1];
            mesh->vertex_array[nvert].normal[2] = nor[nvert * 3 + 2];
        }
        mesh->vertex_array[nvert].uv[0] = 0;
        mesh->vertex_array[nvert].uv[1] = 0;
        mesh->vertex_array[nvert].first_colocal = nvert;
    }
    mesh->face_count = shapes[0].mesh.indices.size() / 3;
    mesh->face_array = new Atlas_Input_Face[mesh->face_count];
    for (int nface = 0; nface < mesh->face_count; nface++) {
        const auto& ind = shapes[0].mesh.indices;
        mesh->face_array[nface].material_index = 0;
        mesh->face_array[nface].vertex_index[0] = ind[nface * 3];
        mesh->face_array[nface].vertex_index[1] = ind[nface * 3 + 1];
        mesh->face_array[nface].vertex_index[2] = ind[nface * 3 + 2];
    }
    printf("Reading %d verts\n", mesh->vertex_count);
    printf("Reading %d triangles\n", mesh->face_count);
    return mesh;
}


static void obj_mesh_free(Atlas_Input_Mesh* mesh)
{
    if (mesh != NULL) {
        delete [] mesh->vertex_array;
        delete [] mesh->face_array;
        delete mesh;
    }
}

int main(int argc, char * argv[])
{
    if (argc != 2) {
        printf("Usage: %s input_file.obj\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load ObjMesh.
    Atlas_Input_Mesh* obj_mesh = obj_mesh_load(argv[1]);
    if (!obj_mesh) {
        printf("Error loading obj file.\n");
        return EXIT_FAILURE;
    }

    // Generate Atlas_Output_Mesh.
    Atlas_Options atlas_options;
    atlas_set_default_options(&atlas_options);
    atlas_options.packer_options.witness.texel_area = 32;
    Atlas_Error error = Atlas_Error_Success;
    Atlas_Output_Mesh* output_mesh = atlas_generate(obj_mesh,
        &atlas_options, &error);
    printf("Atlas mesh has %d verts\n", output_mesh->vertex_count);
    printf("Atlas mesh has %d triangles\n", output_mesh->index_count / 3);

    // Write a bunch of files to disk.
    atlas_dump(output_mesh, obj_mesh);

    // Free meshes.
    obj_mesh_free(obj_mesh);
    atlas_free(output_mesh);

    // Perform raytracing.
    raytrace("result.obj", "object_coords.bin", "facet_normals.bin",
        "result.png", 256);

    return EXIT_SUCCESS;
}
