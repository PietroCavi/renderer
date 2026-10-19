#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/array_list.h"
   
#define TERMINAL_SCREEN_IMPLEMENTATION
#include "utils/terminal_screen.h"
#include "renderer_structs/renderer_structs.h"
#include "asset_loader/asset_loader.h"

int main(){

    printf("\n---- test dir ----\n\n");

    asset_bundle test = load_asset_bundle_dir("assets/Anor_Londo");
    
    print_asset_bundle(test);
    
    printf("\n");

    // while(1);
}

