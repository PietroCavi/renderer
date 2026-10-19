#ifndef __RENDERER_STRUCTS__
#define __RENDERER_STRUCTS__

#include <stddef.h>
#include "../utils/array_list.h"

// struct necessarie per i calcoli

typedef struct{
	float x;
	float y;
    float z;
}vec3;

// struct relative ai modelli 3D

typedef struct{
    
    char* file_name;

    int* colors;
	
    size_t height;
    size_t length;
}texture;

typedef struct{

    texture* items;
    size_t size;
    size_t capacity;

}texture_al;
        
typedef struct{
    int_al index_buffer;
    int_al uv_index_buffer;
                            
    int texture_index;
                            
    int offset;
    int uv_offset;
}model;
    
typedef struct{
    
    model* items;
    size_t size;
    size_t capacity;
}model_al;

typedef struct{
    model_al models;
    
    float_al vertex_buffer;
    float_al uv_vertex_buffer;
}asset_bundle;

typedef struct{
    asset_bundle* to_render;

    vec3 position;
    vec3 rotation;
    float scale;
}asset_bundle_instance;

extern texture_al global_texture_buffer;

asset_bundle merge_asset_bundle(asset_bundle a, asset_bundle b);

void print_asset_bundle(asset_bundle a);

#endif
