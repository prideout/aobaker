#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define AOBAKER_VERSION "1.0.0"

int aobaker_bake(
    char const* inputmesh,   // suzanne.obj
    char const* outputmesh,  // result.obj
    char const* outputatlas, // result.png
    int sizehint,            // 32
    int nsamples,            // 128
    bool gbuffer,            // false
    bool chartinfo);         // false

#ifdef __cplusplus
}
#endif
