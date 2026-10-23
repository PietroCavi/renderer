#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "utils/array_list.h"
   
#define TERMINAL_SCREEN_IMPLEMENTATION
#include "utils/terminal_screen.h"
#include "utils/timerino.h"
#include "utils/polly.h"
#include "renderer_structs/renderer_structs.h"
#include "asset_loader/asset_loader.h"
#include "renderer/renderer.h"
#include "camera_transform/camera_transform.h"

#define WIDTH 384
#define HEIGHT 168

// #define WIDTH 640
// #define HEIGHT 253

#define MOVEMENT_SPEED 30
#define ROTATION_SPEED 1.75

void sig_handler(int signal);
void handle_camera(float dt);

int main(){
    if (signal(SIGINT, sig_handler) == SIG_ERR){
        printf("Can't safely close the program!\n");
        printf("There may be semaphores left behind\n");
        exit(1);
    }

    start_polly();

    printf("\n---- loading models ----\n\n");
    asset_bundle test = load_asset_bundle_dir("assets/teapot");
    print_asset_bundle(test);
    printf("\n");

    init_terminal_screen(WIDTH,HEIGHT);
    init_renderer(WIDTH,HEIGHT,8);
    
    asset_bundle_instance instance = {0};
    instance.to_render = &test;
    instance.scale = 1;

    float time_taken = 0.0166f;
    while(!is_down(KEY_Q)){
        // Update your camera position using delta time
        // camera.x += camera_speed * time_taken;
        handle_camera(time_taken);

        start_timerino();
        render_asset_bundle(instance);
        load_terminal_screen(pixel_buffer);
        printf("\033\143%s",screen_str);
        stop_timerino();

        time_taken = get_timerino("s");

    }
  
    stop_renderer();
    stop_polly();

    // while(1);
}


void sig_handler(int signal){
  printf("\nProgram is closing...\n");

  stop_renderer();
  exit(0);
}

void handle_camera(float dt){
   
    vec3 forward = get_forward_incr(MOVEMENT_SPEED*dt);
    vec3 right   = get_right_incr(MOVEMENT_SPEED*dt);


    if(is_down(KEY_W)){
        camera.position.x += forward.x; 
        camera.position.y += forward.y; 
        camera.position.z += forward.z; 
    }
    if(is_down(KEY_A)){
        camera.position.x -= right.x; 
        camera.position.y -= right.y; 
        camera.position.z -= right.z; 
    }
    if(is_down(KEY_S)){
        camera.position.x -= forward.x; 
        camera.position.y -= forward.y; 
        camera.position.z -= forward.z; 
    }
    if(is_down(KEY_D)){
        camera.position.x += right.x; 
        camera.position.y += right.y; 
        camera.position.z += right.z; 
    }
    if(is_down(KEY_SPACE)){
        camera.position.y += MOVEMENT_SPEED*dt;
    }
    if(is_down(KEY_LEFTSHIFT)){
        camera.position.y -= MOVEMENT_SPEED*dt;
    }
    if(is_down(KEY_I)){
        camera.rotation.x += ROTATION_SPEED*dt;
    }
    if(is_down(KEY_J)){
        camera.rotation.y += ROTATION_SPEED*dt;
    }
    if(is_down(KEY_K)){
        camera.rotation.x -= ROTATION_SPEED*dt;
    }
    if(is_down(KEY_L)){
        camera.rotation.y -= ROTATION_SPEED*dt;
    }
}
