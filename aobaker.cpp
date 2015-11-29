
#include <thekla_atlas.h>

#include <stdio.h>
#include <assert.h>

#include <stdio.h> // FILE

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace Thekla {

struct Obj_Vertex {
    float position[3];
    float normal[3];
    float uv[2];
    int first_colocal;
};

struct Obj_Face {
    int vertex_index[3];
    int material_index;
};

struct Obj_Material {
    // @@ Read obj mtl parameters as is.
};

struct Obj_Mesh {
    int vertex_count;
    Obj_Vertex * vertex_array;

    int face_count;
    Obj_Face * face_array;

    int material_count;
    Obj_Material * material_array;
};

enum Load_Flags {
    Load_Flag_Weld_Attributes,
};

struct Obj_Load_Options {
    int load_flags;
};

Obj_Mesh * obj_mesh_load(const char * filename, const Obj_Load_Options * options);
void obj_mesh_free(Obj_Mesh * mesh);



Obj_Mesh * obj_mesh_load(const char * filename, const Obj_Load_Options * options) {

    using namespace std;
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;
    string err;
    bool ret = tinyobj::LoadObj(shapes, materials, err, filename);
    if (!ret) {
        printf("%s\n", err.c_str());
        return NULL;
    }

    printf("%lu shapes\n", shapes.size());
    printf("%lu materials\n", materials.size());

    assert(shapes.size() > 0);

    Obj_Mesh* mesh = new Obj_Mesh();

    mesh->vertex_count = shapes[0].mesh.positions.size() / 3;
    mesh->vertex_array = new Obj_Vertex[mesh->vertex_count];
    for (int nvert = 0; nvert < mesh->vertex_count; nvert++) {
        mesh->vertex_array[nvert].position[0] = shapes[0].mesh.positions[nvert * 3];
        mesh->vertex_array[nvert].position[1] = shapes[0].mesh.positions[nvert * 3 + 1];
        mesh->vertex_array[nvert].position[2] = shapes[0].mesh.positions[nvert * 3 + 2];
        if (shapes[0].mesh.normals.size() > 0) {
            mesh->vertex_array[nvert].normal[0] = shapes[0].mesh.normals[nvert * 3];
            mesh->vertex_array[nvert].normal[1] = shapes[0].mesh.normals[nvert * 3 + 1];
            mesh->vertex_array[nvert].normal[2] = shapes[0].mesh.normals[nvert * 3 + 2];
        }
        mesh->vertex_array[nvert].uv[0] = 0;
        mesh->vertex_array[nvert].uv[1] = 0;
        mesh->vertex_array[nvert].first_colocal = nvert;
    }

    mesh->face_count = shapes[0].mesh.indices.size() / 3;
    mesh->face_array = new Obj_Face[mesh->face_count];
    for (int nface = 0; nface < mesh->face_count; nface++) {
        mesh->face_array[nface].material_index = 0;
        mesh->face_array[nface].vertex_index[0] = shapes[0].mesh.indices[nface * 3];
        mesh->face_array[nface].vertex_index[1] = shapes[0].mesh.indices[nface * 3 + 1];
        mesh->face_array[nface].vertex_index[2] = shapes[0].mesh.indices[nface * 3 + 2];
    }

    printf("Reading %d verts\n", mesh->vertex_count);
    printf("Reading %d triangles\n", mesh->face_count);

    mesh->material_count = 0;
    mesh->material_array = 0;

    return mesh;
}


void obj_mesh_free(Obj_Mesh * mesh) {
    if (mesh != NULL) {
        delete [] mesh->vertex_array;
        delete [] mesh->face_array;
        delete [] mesh->material_array;
        delete mesh;
    }
}

} // Thekla namespace

using namespace Thekla;

int main(int argc, char * argv[]) {

    if (argc != 2) {
        printf("Usage: %s input_file.obj\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load Obj_Mesh.
    Obj_Load_Options load_options = {0};
    Obj_Mesh * obj_mesh = obj_mesh_load(argv[1], &load_options);

    if (obj_mesh == NULL) {
        printf("Error loading obj file.\n");
        return 1;
    }

    // Convert Obj_Mesh to Atlast_Input_Mesh.
    assert(sizeof(Atlas_Input_Vertex) == sizeof(Obj_Vertex));
    assert(sizeof(Atlas_Input_Face) == sizeof(Obj_Face));

    Atlas_Input_Mesh input_mesh;
    input_mesh.vertex_count = obj_mesh->vertex_count;
    input_mesh.vertex_array = (Atlas_Input_Vertex *)obj_mesh->vertex_array;
    input_mesh.face_count = obj_mesh->face_count;
    input_mesh.face_array = (Atlas_Input_Face *)obj_mesh->face_array;


    // Generate Atlas_Output_Mesh.
    Atlas_Options atlas_options;
    atlas_set_default_options(&atlas_options);

    // Avoid brute force packing, since it can be unusably slow in some situations.
    atlas_options.packer_options.witness.packing_quality = 1;

    Atlas_Error error = Atlas_Error_Success;
    Atlas_Output_Mesh * output_mesh = atlas_generate(&input_mesh, &atlas_options, &error);

    printf("Atlas mesh has %d verts\n", output_mesh->vertex_count);
    printf("Atlas mesh has %d triangles\n", output_mesh->index_count / 3);

    atlas_dump(output_mesh, &input_mesh);

    // Free meshes.
    obj_mesh_free(obj_mesh);
    atlas_free(output_mesh);
 
    return 0;

}
