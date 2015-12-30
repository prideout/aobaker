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

#ifdef __llvm__
double omp_get_wtime() { return 1; }
int omp_get_max_threads() { return 1; }
int omp_get_thread_num() { return 1; }
#else
#include <omp.h>
#endif

using namespace std;
using namespace tinyobj;

// http://www.altdevblogaday.com/2012/05/03/generating-uniformly-distributed-points-on-sphere/
void random_direction(float* result)
{
    float z = 2.0f * rand() / RAND_MAX - 1.0f;
    float t = 2.0f * rand() / RAND_MAX * 3.14f;
    float r = sqrt(1.0f - z * z);
    result[0] = r * cos(t);
    result[1] = r * sin(t);
    result[2] = z;
}

void raytrace(const char* meshobj, int size[2], const float* coordsdata,
    const float* normsdata, const uint8_t* chartids, const char* resultpng,
    int nsamples)
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

    // Iterate over each pixel in the light map, row by row.
    printf("Rendering ambient occlusion (%d threads)...\n",
        omp_get_max_threads());
    double begintime = omp_get_wtime();
    unsigned char* results = (unsigned char*) calloc(size[0] * size[1], 1);
    const uint32_t npixels = size[0] * size[1];
    const float E = 0.001f;
#pragma omp parallel
{
    srand(omp_get_thread_num());
    RTCRay ray;
    ray.primID = RTC_INVALID_GEOMETRY_ID;
    ray.instID = RTC_INVALID_GEOMETRY_ID;
    ray.mask = 0xFFFFFFFF;
    ray.time = 0.f;
#pragma omp for
    for (uint32_t i = 0; i < npixels; i++) {
        float const* norm = normsdata + i * 3;
        float const* origin = coordsdata + i * 3;
        if (norm[0] == 0 && norm[1] == 0 && norm[2] == 0) {
            results[i] = 0;
            continue;
        }
        ray.org[0] = origin[0];
        ray.org[1] = origin[1];
        ray.org[2] = origin[2];
        int nhits = 0;

        // Shoot rays through the differential hemisphere.
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
        results[i] = 255 - 255.0f * nhits / nsamples;
    }
}

    // Print a one-line performance report.
    double duration = omp_get_wtime() - begintime;
    printf("%f seconds\n", duration);

    // Dilate the image by 2 pixels to allow bilinear texturing near seams.
    // Note that this still allows seams when mipmapping, unless mipmap levels
    // are generated very carefully.
    for (int step = 0; step < 2; step++) {
        unsigned char* tmp = (unsigned char*) calloc(size[0] * size[1], 1);
        float const* pnormsdata = normsdata;
        for (int y = 0; y < size[1]; y++) {
            for (int x = 0; x < size[0]; x++) {
                float const* norm = pnormsdata;
                pnormsdata += 3;
                int center = x + y * size[0];
                tmp[center] = results[center];
                if (norm[0] == 0 && norm[1] == 0 && norm[2] == 0 &&
                    results[center] == 0) {
                    for (int k = 0; k < 9; k++) {
                        int i = (k / 3) - 1, j = (k % 3) - 1;
                        if (i == 0 && j == 0) {
                            continue;
                        }
                        i += x; j += y;
                        if (i < 0 || j < 0 || i >= size[0] || j >= size[1]) {
                            continue;
                        }
                        int loc = i + j * size[0];
                        if (results[loc] > 0) {
                            tmp[center] = results[loc];
                            break;
                        }
                    }
                }
            }
        }
        std::swap(results, tmp);
        free(tmp);
    }

    // Write the image.
    printf("Writing %s...\n", resultpng);
    if (chartids) {
        uint8_t* merged = (uint8_t*) malloc(size[0] * size[1] * 2);
        uint8_t* pmerged = merged;
        uint8_t const* presults = results;
        uint8_t const* pchartids = chartids;
        for (int i = 0; i < size[0] * size[1]; i++) {
            *pmerged++ = *presults++;
            *pmerged++ = 255 - (*pchartids++);
        }
        stbi_write_png(resultpng, size[0], size[1], 2, merged, 0);
        free(merged);
    } else {
        stbi_write_png(resultpng, size[0], size[1], 1, results, 0);
    }
    free(results);

    // Free all embree data.
    rtcDeleteGeometry(scene, gid);
    rtcDeleteScene(scene);
    rtcDeleteDevice(device);
}