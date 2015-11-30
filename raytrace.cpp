#include <embree2/rtcore.h>
#include <embree2/rtcore_ray.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <tiny_obj_loader.h>
#include <stb_image_write.h>
#include <vector>
#include <cassert>
#include <cmath>
#include <cfloat>

using namespace std;
using namespace tinyobj;

// http://www.altdevblogaday.com/2012/05/03/generating-uniformly-distributed-points-on-sphere/
void random_direction(float result[3])
{
    double z =  (double)rand() / (double)RAND_MAX*2.0 - 1.0;
    double t =  (double)rand() / (double)RAND_MAX*2.0 * 3.14;
    double r = sqrt(1.0-z*z);
    result[0] = r * cos(t);
    result[1] = r * sin(t);
    result[2] = z;
}

void raytrace(const char* meshobj, const char* coordsbin, const char* normsbin,
    const char* resultpng, int nsamples)
{
    // Intel says to do this, so we're doing it.
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    // Load the mesh.
    vector<shape_t> shapes;
    vector<material_t> materials;
    string err;
    bool ret = LoadObj(shapes, materials, err, meshobj);
    assert(ret && "Unable to load OBJ mesh for raytracing.");
    assert(shapes.size() > 0 && "OBJ mesh contains zero shapes.");
    vector<float>& verts = shapes[0].mesh.positions;
    vector<uint32_t>& indices = shapes[0].mesh.indices;

    // Create the embree device and scene.
    RTCDevice device = rtcNewDevice(NULL);
    assert(device && "Unable to create embree device.");
    RTCScene scene = rtcDeviceNewScene(device, RTC_SCENE_STATIC,
        RTC_INTERSECT1);
    assert(scene);

    // Populate the embree mesh.
    uint32_t gid = rtcNewTriangleMesh(scene, RTC_GEOMETRY_STATIC,
        indices.size() / 3, verts.size() / 3);
    float* vertices = (float*) rtcMapBuffer(scene, gid, RTC_VERTEX_BUFFER);
    float const* srcp = &verts[0];
    for (size_t i = 0; i < verts.size() / 3; i++) {
        *vertices++ = *srcp++;
        *vertices++ = *srcp++;
        *vertices++ = *srcp++;
        vertices++;
    }
    rtcUnmapBuffer(scene, gid, RTC_VERTEX_BUFFER);
    verts.clear();
    verts.shrink_to_fit();
    uint32_t* triangles = (uint32_t*) rtcMapBuffer(scene, gid, RTC_INDEX_BUFFER);
    uint32_t const* srci = &indices[0];
    for (size_t i = 0; i < indices.size() / 3; i++) {
        *triangles++ = *srci++;
        *triangles++ = *srci++;
        *triangles++ = *srci++;
    }
    rtcUnmapBuffer(scene, gid, RTC_INDEX_BUFFER);
    indices.clear();
    indices.shrink_to_fit();
    rtcCommit(scene);

    // Crawl over each pixel in the light map and cast rays.
    RTCRay ray;
    ray.primID = RTC_INVALID_GEOMETRY_ID;
    ray.instID = RTC_INVALID_GEOMETRY_ID;
    ray.mask = 0xFFFFFFFF;
    ray.time = 0.f;
    FILE* coordsfile = fopen(coordsbin, "rb");
    FILE* normsfile = fopen(normsbin, "rb");
    uint32_t size[2];
    fread(size, 1, sizeof(size), coordsfile);
    fread(size, 1, sizeof(size), normsfile);
    unsigned char* results = (unsigned char*) calloc(size[0] * size[1], 1);
    unsigned char* presult = results;
    const float E = 0.001f;
    float norm[3];
    float origin[3];
    uint32_t npixels = size[0] * size[1];
    for (uint32_t i = 0; i < npixels; i++) {

        // Poor man's progress indicator.
        if ((i % (npixels / 10)) == 0) {
            printf("%d\n", 10 - i / (npixels / 10));
        }

        fread(origin, 1, sizeof(float) * 3, coordsfile);
        fread(norm, 1, sizeof(float) * 3, normsfile);
        if (norm[0] == 0 && norm[1] == 0 && norm[2] == 0) {
            *presult++ = 0;
            continue;
        }

        ray.org[0] = origin[0];
        ray.org[1] = origin[1];
        ray.org[2] = origin[2];

        int nhits = 0;
        for (int nsamp = 0; nsamp < nsamples; nsamp++) {
            random_direction(ray.dir);
            float dotp = norm[0] * ray.dir[0] +
                norm[1] * ray.dir[1] +
                norm[2] * ray.dir[2];
            if (dotp < 0) {
                ray.dir[0] = -ray.dir[0];
                ray.dir[1] = -ray.dir[1];
                ray.dir[2] = -ray.dir[2];
            }
            ray.tnear = E;
            ray.tfar = FLT_MAX;
            ray.geomID = RTC_INVALID_GEOMETRY_ID;
            rtcOccluded(scene, ray);
            if (ray.geomID == 0) {
                nhits++;
            }
        }
        *presult++ = 255 - 255.0f * nhits / nsamples;
    }
    fclose(coordsfile);
    fclose(normsfile);

    // Write the image.
    printf("Writing %s...\n", resultpng);
    stbi_write_png(resultpng, size[0], size[1], 1, results, 0);
    free(results);

    // Free all embree data.
    rtcDeleteGeometry(scene, gid);
    rtcDeleteScene(scene);
    rtcDeleteDevice(device);
}