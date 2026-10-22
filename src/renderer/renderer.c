#include "renderer.h"
#include "../utils/parallel_unit.h"
#include "../transform_functions/transform_functions.h"

#include <math.h>
#include <immintrin.h>

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
parallel_unit draw_unit      = {0};
parallel_unit clear_unit     = {0};
parallel_unit merge_unit     = {0};
   
asset_bundle_instance current_instance;
model                 current_model;

float_al* transformed_triangles = {0};
float_al* transformed_triangles_uv = {0};

static void* transform_and_cull(void* arg);
static void* draw_and_transform(void* arg);
static void* clear(void* arg);
static void* merge(void* arg);

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
    clear_unit = create_parallel_unit(num_threads,clear);
    merge_unit = create_parallel_unit(num_threads,merge);
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
    destroy_parallel_unit(draw_unit     );
    destroy_parallel_unit(clear_unit    );
    destroy_parallel_unit(merge_unit    );
}

typedef struct{
    int first_index;
    int last_index;
    size_t thread_id;
}transform_and_cull_args;
static void* transform_and_cull(void* arg){
    transform_and_cull_args* args = (transform_and_cull_args*)arg;        
    
    int first_index            = args->first_index;
    int last_index             = args->last_index;
    size_t thread_id           = args->thread_id;
    
    int* index_buffer       = current_model.index_buffer.items;
    int* uv_index_buffer    = current_model.uv_index_buffer.items;
    int offset              = current_model.offset;
    float* vertex_buffer    = current_instance.to_render->vertex_buffer.items;
    float* uv_vertex_buffer = current_instance.to_render->uv_vertex_buffer.items;
    vec3 position           = current_instance.position;
    vec3 rotation           = current_instance.rotation;
    float scale_f           = current_instance.scale;
  
    int offset_uv = offset/3*2;

    if(first_index == last_index){
        return NULL;
    }

    float x[144];
    float y[144];
    float z[144];

    float x_normal[144/3];
    float y_normal[144/3];
    float z_normal[144/3];

    float x_uv[144];
    float y_uv[144];

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
            x[number_of_vertices] = vertex_buffer[index_buffer[i+number_of_vertices]*3+offset  ];
            y[number_of_vertices] = vertex_buffer[index_buffer[i+number_of_vertices]*3+offset+1];
            z[number_of_vertices] = vertex_buffer[index_buffer[i+number_of_vertices]*3+offset+2];
            
            x_uv[number_of_vertices] = uv_vertex_buffer[uv_index_buffer[i+number_of_vertices]*2+offset_uv  ];
            y_uv[number_of_vertices] = uv_vertex_buffer[uv_index_buffer[i+number_of_vertices]*2+offset_uv+1];
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

            append_al(transformed_triangles_uv[thread_id],x_uv[i*3]);
            append_al(transformed_triangles_uv[thread_id],y_uv[i*3]);
            
            append_al(transformed_triangles_uv[thread_id],x_uv[i*3+1]);
            append_al(transformed_triangles_uv[thread_id],y_uv[i*3+1]);
            
            append_al(transformed_triangles_uv[thread_id],x_uv[i*3+2]);
            append_al(transformed_triangles_uv[thread_id],y_uv[i*3+2]);

        }
    
    }
    
    return NULL;
}

static inline void project(float x[3], float y[3], float z[3]) {
    float s = fminf(renderer_width, renderer_height) / 2.;  // pixels per unit

    for(size_t i=0;i<3;i++){
        x[i] = renderer_width / 2. + (x[i] / z[i]) * s;
        y[i] = renderer_height / 2. - (y[i] / z[i]) * s;
    }
}

static void* draw_and_transform(void* arg){
    size_t thread_id = ((size_t*)arg)[0];

    float* vertices = transformed_triangles[thread_id].items;
    float* uv_vertices = transformed_triangles_uv[thread_id].items;

    texture tex = {0};
    if(current_model.texture_index != -1)
        tex = global_texture_buffer.items[current_model.texture_index];
    
    int* local_pixel_buffer = thread_pixel_buffer[thread_id];
    float* local_z_buffer     = thread_z_buffer[thread_id];

    float x[3];
    float y[3];
    float z[3];

    float x_uv[3];
    float y_uv[3];
    
    for(size_t i=0;i<transformed_triangles[thread_id].size/9;i++){
        x[0] = vertices[i*9  ];   y[0] = vertices[i*9+1];   z[0] = vertices[i*9+2];
        x[1] = vertices[i*9+3];   y[1] = vertices[i*9+4];   z[1] = vertices[i*9+5];
        x[2] = vertices[i*9+6];   y[2] = vertices[i*9+7];   z[2] = vertices[i*9+8];

        x_uv[0] = uv_vertices[i*6  ];   y_uv[0] = uv_vertices[i*6+1]; 
        x_uv[1] = uv_vertices[i*6+2];   y_uv[1] = uv_vertices[i*6+3]; 
        x_uv[2] = uv_vertices[i*6+4];   y_uv[2] = uv_vertices[i*6+5]; 

        project(x,y,z);
    
        float fx0 = fminf(x[0], fminf(x[1], x[2]));
        float fx1 = fmaxf(x[0], fmaxf(x[1], x[2]));
        float fy0 = fminf(y[0], fminf(y[1], y[2]));
        float fy1 = fmaxf(y[0], fmaxf(y[1], y[2]));
        if (fx1 < 0 || fx0 >= renderer_width ||
            fy1 < 0 || fy0 >= renderer_height)
            continue;

        int min_x = (int)fmaxf(fx0, 0.0f);
        int max_x = (int)fminf(fx1, renderer_width  - 1.0f);
        int min_y = (int)fmaxf(fy0, 0.0f);
        int max_y = (int)fminf(fy1, renderer_height - 1.0f);
     
        //if (min_x > max_x || min_y > max_y){continue;}
        float depth;
        const float Yv1v2 = y[1]-y[2];
        const float Xv2v1 = x[2]-x[1];
        const float Xv0v2 = x[0]-x[2];
        const float Yv2v0 = y[2]-y[0];
        const float sub = Yv1v2*Xv0v2+Xv2v1*(y[0]-y[2]);
        
        float w0;
        float w1;
        float w2;

        float u;
        float v;

        int tx;
        int ty;

        int color;

        // drawing

        for(int i=min_x;i <= max_x;i++){
            for(int j=min_y;j <= max_y;j++){
                    
                w0 = (Yv1v2*(i-x[2])+Xv2v1*(j-y[2]))/sub;     
                w1 = (Yv2v0*(i-x[2])+Xv0v2*(j-y[2]))/sub;
                w2 = 1 - w0 - w1;
                
                if (w0 < 0 || w1 < 0 || w2 < 0){ 
                    continue;
                }
                else{
                    depth = w0*z[0] + w1*z[1] + w2*z[2];

                    if(depth < local_z_buffer[i+j*renderer_width]){                
                        local_z_buffer[i+j*renderer_width] = depth;
                        
                        u = w0 * x_uv[0] + w1 * x_uv[1] + w2 * x_uv[2];
                        v = w0 * y_uv[0] + w1 * y_uv[1] + w2 * y_uv[2];
                       
                        u = u-floor(u);
                        v = v-floor(v);
                        
                        if(tex.colors!=NULL){
                            tx = (int)(u * (tex.length - 1));
                            ty = (int)(v * (tex.height - 1));
                            color = tex.colors[ty * tex.length + tx];
                        }
                        else{
                            color = 0xFF00FF;
                        }

                        //draw_point_thread(a,i,j,shade_for_normal(shade_for_distance(a,color,depth),normal.z),thread_id);
                        local_pixel_buffer[i+j*renderer_width] = color;
                    }    
                }
              
            }
        }


    }
}

static void* merge(void* arg) {
    size_t thread_id = ((size_t*)arg)[0];

    const size_t total_length = renderer_width * renderer_height;
    const size_t step         = total_length / num_threads;
    const size_t first_index  = step * thread_id;
    const size_t last_index   = (thread_id == num_threads - 1) ? total_length : first_index + step;

    size_t i = first_index;
    for (; i + 8 <= last_index; i += 8) {
        __m256  zv = _mm256_loadu_ps(z_buffer + i);
        __m256i pv = _mm256_loadu_si256((const __m256i*)(pixel_buffer + i));

        for (size_t t = 0; t < num_threads; t++) {
            __m256 tzv = _mm256_loadu_ps(thread_z_buffer[t] + i);
            __m256 m   = _mm256_cmp_ps(tzv, zv, _CMP_LT_OQ);
            zv = _mm256_blendv_ps(zv, tzv, m);
            __m256i tpv = _mm256_loadu_si256((const __m256i*)(thread_pixel_buffer[t] + i));
            pv = _mm256_blendv_epi8(pv, tpv, _mm256_castps_si256(m));
        }

        _mm256_storeu_ps(z_buffer + i, zv);
        _mm256_storeu_si256((__m256i*)(pixel_buffer + i), pv);
    }
    for (; i < last_index; i++) {
        for (size_t t = 0; t < num_threads; t++) {
            if (thread_z_buffer[t][i] < z_buffer[i]) {
                z_buffer[i]     = thread_z_buffer[t][i];
                pixel_buffer[i] = thread_pixel_buffer[t][i];
            }
        }
    }
    return NULL;

}

static void* clear(void* arg) {
    size_t thread_id = ((size_t*)arg)[0];

    const size_t total_length = renderer_width * renderer_height;
    const size_t step         = total_length / num_threads;
    const size_t first_index  = step * thread_id;
    const size_t last_index   = (thread_id == num_threads - 1) ? total_length : first_index + step;

    const __m256i background = _mm256_set1_epi32(0);
    const __m256  z_far      = _mm256_set1_ps(zFAR);

    size_t i = first_index;
    for (; i + 8 <= last_index; i += 8) {
        _mm256_storeu_si256((__m256i*)(pixel_buffer + i), background);
        _mm256_storeu_ps(z_buffer + i, z_far);
    }
    for (; i < last_index; i++) {
        pixel_buffer[i] = 0;
        z_buffer[i]     = zFAR;
    }
    
    int *tp = thread_pixel_buffer[thread_id];
    float    *tz = thread_z_buffer[thread_id];
    i = 0;
    for (; i + 8 <= total_length; i += 8) {
        _mm256_storeu_si256((__m256i*)(tp + i), background);
        _mm256_storeu_ps(tz + i, z_far);
    }
    for (; i < total_length; i++) {
        tp[i] = 0;
        tz[i] = zFAR;
    }

    return NULL;
}

void render_asset_bundle(asset_bundle_instance instance){
    size_t to_draw[num_threads];
    for(size_t k=0;k<num_threads;k++){
        to_draw[k] = k;
    }

    for(size_t k=0;k<num_threads;k++){
        clear_unit.args[k] = &to_draw[k];
    }
    execute_parallel_unit(clear_unit);

    current_instance = instance;
    model_al models  = current_instance.to_render->models;

    transform_and_cull_args to_transform[num_threads];
        
    for(size_t i=0;i<models.size;i++){
        current_model = models.items[i];
        
        size_t triangle_count = current_model.index_buffer.size / 3;
        size_t step = triangle_count / num_threads;

        for(size_t k=0;k<num_threads;k++){
           
       
            size_t first_index = k * step * 3;
            size_t last_index = (k == num_threads - 1) ? triangle_count * 3 : first_index+step*3;

            to_transform[k] = (transform_and_cull_args){
                .first_index      = first_index,
                .last_index       = last_index,
                .thread_id        = k
            };
            
            transform_unit.args[k] = &to_transform[k];
        }
                    
        execute_parallel_unit(transform_unit);
        
        for(size_t k=0;k<num_threads;k++){
            draw_unit.args[k] = &to_draw[k];
        }

        execute_parallel_unit(draw_unit);

        for(size_t k=0;k<num_threads;k++){
            //printf("Thread %d: %d\n",k,transformed_triangles[k].size);
            clean_al(transformed_triangles[k]);
            clean_al(transformed_triangles_uv[k]);
        }
        
    }

    
    for(size_t k=0;k<num_threads;k++){
        merge_unit.args[k] = &to_draw[k];
    }
    execute_parallel_unit(merge_unit);
        

}
