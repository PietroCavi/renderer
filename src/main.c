#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/array_list.h"
   
#define TERMINAL_SCREEN_IMPLEMENTATION
#include "utils/terminal_screen.h"
#include "utils/timerino.h"
#include "renderer_structs/renderer_structs.h"
#include "asset_loader/asset_loader.h"
#include "renderer/renderer.h"

#define WIDTH 240
#define HEIGHT 126

int main(){

    printf("\n---- test dir ----\n\n");

    asset_bundle test = load_asset_bundle_dir("assets/teapot");
    
    print_asset_bundle(test);
    
    printf("\n");

    init_terminal_screen(WIDTH,HEIGHT);
    init_renderer(WIDTH,HEIGHT,14);
    
    asset_bundle_instance instance = {0};
    instance.to_render = &test;
    instance.scale = 1;
    instance.position = (vec3){0,0,5};


    double total_time = 0;
    
    for(size_t i=0;i<10;i++){
        render_asset_bundle(instance);
    }

    vec3 rotation = {0,0,0};
    for(size_t i=0;i<1000;i++){
        start_timerino();
        render_asset_bundle(instance);
        stop_timerino();
        
        load_terminal_screen(pixel_buffer);
        printf("\033\143%s",screen_str);
        
        rotation.x += 0.1;
        rotation.y += 0.1;
        instance.rotation = rotation; 

        total_time += get_timerino("ms");
        
        //usleep(50000);
    }

    printf("avg time on 1000 frames: %fms\n",total_time/1000.0);

    stop_renderer();

    // while(1);
}

