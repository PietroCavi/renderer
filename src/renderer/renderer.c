#include "renderer.h"
#include "../utils/parallel_unit.h"

size_t num_threads = 0;
size_t renderer_width = 0;
size_t renderer_height = 0;

int* pixel_buffer = NULL; //size will be renderer_width*renderer_height*sizeof(int)
float* z_buffer = NULL;

renderer_camera camera= {0};

int** thread_pixel_buffer = {0};
float** thread_z_buffer = {0};

parallel_unit transform_unit = {0};

float_al* transformed_triangles;

static void* transform_and_cull(void* arg);

void init_renderer(size_t width, size_t height, size_t num_threads){
    
    stop_renderer(); // if possible.

    num_threads     = num_threads;
    renderer_width  = width;
    renderer_height = height;

    pixel_buffer = malloc(renderer_width*renderer_height*sizeof(int));
    z_buffer     = malloc(renderer_width*renderer_height*sizeof(int));

    camera = (renderer_camera){(vec3){0,0,0},(vec3){0,0,0}};
    
    transformed_triangles = malloc(num_threads*sizeof(float_al*));
    thread_pixel_buffer = malloc(num_threads*sizeof(int*));
    thread_z_buffer     = malloc(num_threads*sizeof(float*));
    for(size_t i=0;i<num_threads;i++){
        thread_pixel_buffer[i]   = malloc(renderer_width*renderer_height*sizeof(int));
        thread_z_buffer[i]       = malloc(renderer_width*renderer_height*sizeof(int));
    }

    transform_unit = create_parallel_unit(num_threads, transform_and_cull);    

}

void stop_renderer(){
    renderer_width  = 0;
    renderer_height = 0;
    
    if(pixel_buffer != NULL){
        free(pixel_buffer);
        pixel_buffer = NULL;
    }
    if(z_buffer!= NULL){
        free(z_buffer);
        z_buffer = NULL;
    }

    camera = (renderer_camera){(vec3){0,0,0},(vec3){0,0,0}};
   
    if(transformed_triangles != NULL){
        free(transformed_triangles);
        transformed_triangles = NULL;
    }

    if(thread_pixel_buffer!=NULL){
        for(size_t i=0;i<num_threads;i++){
            free(thread_pixel_buffer[i]);
            thread_pixel_buffer[i] = NULL;
        }
        free(thread_pixel_buffer);
        thread_pixel_buffer = NULL;
    }
    
    if(thread_z_buffer!=NULL){
        for(size_t i=0;i<num_threads;i++){
            free(thread_z_buffer[i]);
            thread_z_buffer[i] = NULL;
        }

        free(thread_z_buffer);
        thread_z_buffer = NULL;
    }

    num_threads = 0;

    destroy_parallel_unit(transform_unit);
}

typedef struct{
    int* index_buffer;
    int offset;
    float* vertex_buffer;
    size_t number_of_triangles;
    size_t thread_id;
}transform_and_cull_args;
static void* transform_and_cull(void* arg){
    transform_and_cull_args* args = (transform_and_cull_args*)arg;        
    
    int* index_buffer          = args->index_buffer;
    int offset                 = args->offset;
    float* vertex_buffer       = args->vertex_buffer;
    size_t number_of_triangles = args->number_of_triangles;
    size_t thread_id           = args->thread_id;
}

void render_asset_bundle(asset_bundle_instance instance){
    printf("rendering instance\n");
    
    model_al models          = instance.to_render->models;
    float_al vertex_buffer    = instance.to_render->vertex_buffer;
    float_al uv_vertex_buffer = instance.to_render->uv_vertex_buffer;
    
    model current_model;

    for(size_t i=0;i<models.size;i++){
        current_model = models.items[i];
    }

}
