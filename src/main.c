#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/array_list.h"
   
#define TERMINAL_SCREEN_IMPLEMENTATION
#include "utils/terminal_screen.h"
#include "renderer_structs/renderer_structs.h"
#include "asset_loader/asset_loader.h"
#include "renderer/renderer.h"

int main(){

    printf("\n---- test dir ----\n\n");

    asset_bundle test = load_asset_bundle_dir("assets/teapot");
    
    print_asset_bundle(test);
    
    printf("\n");

    init_renderer(1,1,1);
    
    asset_bundle_instance instance = {0};
    instance.to_render = &test;
    instance.scale = 1;

    render_asset_bundle(instance);

    stop_renderer();

    // while(1);
}

