#pragma once

#include "../renderer_structs/renderer_structs.h"

typedef struct{
    vec3 position;
    vec3 rotation;
}renderer_camera;

extern size_t renderer_width;
extern size_t renderer_height;

extern int* pixel_buffer; //size will be renderer_width*renderer_height*sizeof(int)
extern float* z_buffer;

extern renderer_camera camera;

void init_renderer(size_t width, size_t height, size_t num_threads);

void stop_renderer();

void render_asset_bundle(asset_bundle_instance instance);
