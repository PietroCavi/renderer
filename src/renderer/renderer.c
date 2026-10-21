#include "renderer.h"
#include "../utils/parallel_unit.h"
#include "../transform_functions/transform_functions.h"

#include <math.h>

#define zNEAR    0.
#define zFAR     3000 

size_t num_threads = 0;
size_t renderer_width = 0;
size_t renderer_height = 0;

int* pixel_buffer = NULL; //size will be renderer_width*renderer_height*sizeof(int)
float* z_buffer = NULL;

renderer_camera camera= {0};

int** thread_pixel_buffer = {0};
float** thread_z_buffer = {0};

parallel_unit transform_unit = {0};
parallel_unit draw_unit = {0};

float_al* transformed_triangles = {0};
float_al* transformed_triangles_uv = {0};

static void* transform_and_cull(void* arg);
static void* draw_and_transform(void* arg);

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
    transformed_triangles_uv = calloc(num_threads, sizeof(float_al));
    thread_pixel_buffer = malloc(num_threads*sizeof(int*));
    thread_z_buffer     = malloc(num_threads*sizeof(float*));
    for(size_t i=0;i<num_threads;i++){
        thread_pixel_buffer[i]   = malloc(renderer_width*renderer_height*sizeof(int));
        thread_z_buffer[i]       = malloc(renderer_width*renderer_height*sizeof(float));
    }

    transform_unit = create_parallel_unit(num_threads, transform_and_cull);    
    draw_unit = create_parallel_unit(num_threads,draw_and_transform);
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
    
    if(transformed_triangles_uv != NULL){
        for(size_t i=0;i<num_threads;i++){
            free_al(transformed_triangles_uv[i]);
        }
        free(transformed_triangles_uv);
        transformed_triangles_uv = NULL;
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

static void rebalance(){

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
   
    if(first_index == last_index){
        return NULL;
    }

    vec3 position = args->position;
    vec3 rotation = args->rotation;
    float scale_f   = args->scale;

    float x[144];
    float y[144];
    float z[144];

    float x_normal[144/3];
    float y_normal[144/3];
    float z_normal[144/3];

    float dot[144/3];

    size_t number_of_vertices = 0;

    //printf("first_index:%d last_index:%d\n",first_index,last_index);

    float half_x_world = 0.5f * rotation.x;
    float s_x_world = sinf(half_x_world);
    float half_y_world = 0.5f * rotation.y;
    float s_y_world = sinf(half_y_world);
    float half_z_world = 0.5f * rotation.z;
    float s_z_world = sinf(half_z_world);
    vec4 quat_x_world = {s_x_world,0,0,cosf(half_x_world)};
    vec4 quat_y_world = {0,s_y_world,0,cosf(half_y_world)};
    vec4 quat_z_world = {0,0,s_z_world,cosf(half_z_world)};

    float half_x_camera = 0.5f * camera.rotation.x;
    float s_x_camera = sinf(half_x_camera);
    float half_y_camera = 0.5f * camera.rotation.y;
    float s_y_camera = sinf(half_y_camera);
    float half_z_camera = 0.5f * camera.rotation.z;
    float s_z_camera = sinf(half_z_camera);
    vec4 quat_x_camera = {s_x_camera,0,0,cosf(half_x_camera)};
    vec4 quat_y_camera = {0,s_y_camera,0,cosf(half_y_camera)};
    vec4 quat_z_camera = {0,0,s_z_camera,cosf(half_z_camera)};
    
    vec4 quat_world = qmul(quat_z_world,  qmul(quat_x_world,  quat_y_world));
    vec4 quat_camera   = qmul(quat_z_camera, qmul(quat_x_camera, quat_y_camera));
    
    number_of_vertices = 0;
    for(size_t i=first_index;i<last_index;i+=144){
        for(number_of_vertices=0;number_of_vertices<144 && i+number_of_vertices < last_index;number_of_vertices++){
            x[number_of_vertices] = vertex_buffer[index_buffer[i+number_of_vertices]+offset  ];
            y[number_of_vertices] = vertex_buffer[index_buffer[i+number_of_vertices]+offset+1];
            z[number_of_vertices] = vertex_buffer[index_buffer[i+number_of_vertices]+offset+2];
        }
        
        // world transformation
        scale(x,y,z,scale_f); 
        quat_rotate(x,y,z,quat_world);
        translate_vec3(x,y,z,position);

        //camera transformation
        sub_vec3(x,y,z,camera.position);
        quat_rotate(x,y,z,quat_camera);
      
        triangle_normal(x,y,z,x_normal,y_normal,z_normal);
        
        triangle_dot(x,y,z,x_normal,y_normal,z_normal,dot);

        for(size_t i=0;i<number_of_vertices/3;i++){
            
            if(z[i*3] <= zNEAR || z[i*3+1] <= zNEAR || z[i*3+2] <= zNEAR)
                continue;

            if(z[i*3] > zFAR || z[i*3+1] > zFAR || z[i*3+2] > zFAR)
                continue;
            
            if(dot[i] >= 0)
                continue;
                
            append_al(transformed_triangles[thread_id],x[i*3]);
            append_al(transformed_triangles[thread_id],y[i*3]);
            append_al(transformed_triangles[thread_id],z[i*3]);
            
            append_al(transformed_triangles[thread_id],x[i*3+1]);
            append_al(transformed_triangles[thread_id],y[i*3+1]);
            append_al(transformed_triangles[thread_id],z[i*3+1]);
            
            append_al(transformed_triangles[thread_id],x[i*3+2]);
            append_al(transformed_triangles[thread_id],y[i*3+2]);
            append_al(transformed_triangles[thread_id],z[i*3+2]);
        }
    
    }
    
    return NULL;
}

static void* draw_and_transform(void* arg){
}

void render_asset_bundle(asset_bundle_instance instance){
    
    model_al models          = instance.to_render->models;
    float_al vertex_buffer    = instance.to_render->vertex_buffer;
    float_al uv_vertex_buffer = instance.to_render->uv_vertex_buffer;
    
    model current_model;
    
    transform_and_cull_args to_transform[num_threads];

    for(size_t i=0;i<models.size;i++){
        current_model = models.items[i];
        
        size_t triangle_count = current_model.index_buffer.size / 3;
        size_t step = triangle_count / num_threads;

        for(size_t k=0;k<num_threads;k++){
           
       
            size_t first_index = k * step * 3;
            size_t last_index = (k == num_threads - 1) ? triangle_count * 3 : first_index+step*3;

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
        
    execute_parallel_unit(draw_unit);
    
    for(size_t k=0;k<num_threads;k++){
        printf("Thread %d: %d\n",k,transformed_triangles[k].size);
        clean_al(transformed_triangles[k]);
    }

}
