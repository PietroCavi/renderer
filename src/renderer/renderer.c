#include "renderer.h"
#include "../utils/parallel_unit.h"
#include "../transform_functions/transform_functions.h"

size_t num_threads = 0;
size_t renderer_width = 0;
size_t renderer_height = 0;

int* pixel_buffer = NULL; //size will be renderer_width*renderer_height*sizeof(int)
float* z_buffer = NULL;

renderer_camera camera= {0};

int** thread_pixel_buffer = {0};
float** thread_z_buffer = {0};

parallel_unit transform_unit = {0};

float_al* transformed_triangles = {0};

static void* transform_and_cull(void* arg);

void init_renderer(size_t width, size_t height, size_t threads){
    
    if(threads == 0 || width == 0 || height == 0){
        printf("Initialization error => width:%lu   height:%lu   threads:%lu\n");
        printf("These values can't be 0\n");

        exit(1);
    }

    stop_renderer(); // if possible.

    num_threads     = threads;
    renderer_width  = width;
    renderer_height = height;

    pixel_buffer = malloc(renderer_width*renderer_height*sizeof(int));
    z_buffer     = malloc(renderer_width*renderer_height*sizeof(int));

    camera = (renderer_camera){(vec3){0,0,0},(vec3){0,0,0}};
    
    transformed_triangles = calloc(num_threads, sizeof(float_al));
    thread_pixel_buffer = malloc(num_threads*sizeof(int*));
    thread_z_buffer     = malloc(num_threads*sizeof(float*));
    for(size_t i=0;i<num_threads;i++){
        thread_pixel_buffer[i]   = malloc(renderer_width*renderer_height*sizeof(int));
        thread_z_buffer[i]       = malloc(renderer_width*renderer_height*sizeof(float));
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
        for(size_t i=0;i<num_threads;i++){
            free_al(transformed_triangles[i]);
        }
        free(transformed_triangles);
        transformed_triangles = NULL;
    }

    if(thread_pixel_buffer!=NULL){
        for(size_t i=0;i<num_threads;i++){
            if(thread_pixel_buffer != NULL)
                free(thread_pixel_buffer[i]);
            thread_pixel_buffer[i] = NULL;
        }
        free(thread_pixel_buffer);
        thread_pixel_buffer = NULL;
    }
    
    if(thread_z_buffer!=NULL){
        for(size_t i=0;i<num_threads;i++){
            if(thread_z_buffer != NULL)
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
    int first_index;
    int last_index;
    size_t thread_id;
    
    vec3 position;
    vec3 rotation;
    float scale;
}transform_and_cull_args;
static void* transform_and_cull(void* arg){
    transform_and_cull_args* args = (transform_and_cull_args*)arg;        
    
    int* index_buffer          = args->index_buffer;
    int offset                 = args->offset;
    float* vertex_buffer       = args->vertex_buffer;
    int first_index    = args->first_index;
    int last_index    = args->last_index;
    size_t thread_id           = args->thread_id;
    
    vec3 position = args->position;
    vec3 rotation = args->rotation;
    float scale_f   = args->scale;

    float x[24];
    float y[24];
    float z[24];

    size_t number_of_vertices = 0;

    //printf("first_index:%d last_index:%d\n",first_index,last_index);

    for(size_t i=first_index;i<=last_index;i+=24){
        for(number_of_vertices=0;number_of_vertices<24 && i+number_of_vertices <= last_index;number_of_vertices+=3){
            x[number_of_vertices] = vertex_buffer[index_buffer[i+number_of_vertices]+offset  ];
            y[number_of_vertices] = vertex_buffer[index_buffer[i+number_of_vertices]+offset+1];
            z[number_of_vertices] = vertex_buffer[index_buffer[i+number_of_vertices]+offset+2];
        }
        
        // world transformation
        scale(x,y,z,scale_f); 
        rotate_yxz(x,y,z,rotation);
        translate_vec3(x,y,z,position);

        //camera transformation
        sub_vec3(x,y,z,camera.position);
        rotate_yxz(x,y,z,camera.rotation);

    }
    
    return NULL;
}

void render_asset_bundle(asset_bundle_instance instance){
    
    model_al models          = instance.to_render->models;
    float_al vertex_buffer    = instance.to_render->vertex_buffer;
    float_al uv_vertex_buffer = instance.to_render->uv_vertex_buffer;
    
    model current_model;
    
    size_t prov;
    size_t model_size;

    size_t portion_size;

    size_t first_index;
    size_t last_index;
    transform_and_cull_args to_transform[num_threads];

    for(size_t i=0;i<models.size;i++){
        current_model = models.items[i];
        model_size = current_model.index_buffer.size;

        portion_size = model_size / num_threads;

        for(size_t k=0;k<num_threads;k++){
            first_index = portion_size*k;
            
            prov = portion_size*(k+1)-1;

            last_index  = prov < model_size ? prov : model_size-1;

            to_transform[k] = (transform_and_cull_args){
                .index_buffer = current_model.index_buffer.items,
                .offset = current_model.offset,
                .vertex_buffer = vertex_buffer.items,
                .first_index = first_index,
                .last_index  = last_index,
                .thread_id = k,
                .position = instance.position,
                .rotation = instance.rotation,
                .scale = instance.scale
            };
            
            transform_unit.args[k] = &to_transform[k];
        }
            
        execute_parallel_unit(transform_unit);
    }

}
