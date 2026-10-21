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

int main(){

    printf("\n---- test dir ----\n\n");

    asset_bundle test = load_asset_bundle_dir("assets/Anor_Londo");
    
    print_asset_bundle(test);
    
    printf("\n");

    init_renderer(160,77,14);
    
    asset_bundle_instance instance = {0};
    instance.to_render = &test;
    instance.scale = 1;

    //double total_time = 0;
    //
    //for(size_t i=0;i<10;i++){
    //    render_asset_bundle(instance);
    //}

    //for(size_t i=0;i<10000;i++){
    //    start_timerino();
    //    render_asset_bundle(instance);
    //    stop_timerino();

    //    total_time += get_timerino("ms");

    //}

    //printf("avg time on 10000 frames: %fms\n",total_time/10000.0);
    
    render_asset_bundle(instance);

    stop_renderer();

    // while(1);
}

