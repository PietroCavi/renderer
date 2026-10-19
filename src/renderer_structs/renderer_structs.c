#include "renderer_structs.h"
#include "../utils/array_list.h"
#include <stdio.h>

texture_al global_texture_buffer = {0};

asset_bundle merge_asset_bundle(asset_bundle a, asset_bundle b){
    
    for(size_t i=0;i<b.models.size;i++){
        b.models.items[i].offset = a.vertex_buffer.size;
        b.models.items[i].uv_offset = a.uv_vertex_buffer.size;
    }
    
    
    append_all_al(a.models,b.models);

    append_all_al(a.vertex_buffer, b.vertex_buffer);
    append_all_al(a.uv_vertex_buffer, b.uv_vertex_buffer);

    trim_to_size_al(a.models);
    trim_to_size_al(a.vertex_buffer);
    trim_to_size_al(a.uv_vertex_buffer);

    return a;
}

void print_asset_bundle(asset_bundle a){
    printf("Number of models: %d\n",a.models.size); 
    
    float triangles = 0;

    for(size_t i=0;i<a.models.size;i++){
        triangles += a.models.items[i].index_buffer.size;
    }

    printf("Number of triangles: %f\n",triangles/3);

    printf("Global number of textures: %d\n",global_texture_buffer.size); 
    printf("Number of vertices: %d\n",a.vertex_buffer.size); 
    printf("Number of uv_vertices: %d\n",a.uv_vertex_buffer.size); 
}
