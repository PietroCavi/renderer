#include "polly.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

static int scoreboard[KEY_MAX + 1] = {0};
static pthread_t polly_thread;
static int keyboard_file = -1;

static void* key_poll(void* arg) {

	struct input_event ie = {0};
	
	while(1){
		read(keyboard_file, &ie, sizeof(struct input_event));
	    if(ie.type == EV_KEY && ie.code <= KEY_MAX)
            scoreboard[ie.code] = ie.value;
	}
}

int is_down(int key) {
    return (key >= 0 && key <= KEY_MAX) ? scoreboard[key] : 0;
}

int start_polly(){
    keyboard_file = open("/dev/input/event0", O_RDONLY);
    if (keyboard_file == -1) { 
        printf("open keyboard_file for polly has failed\n"); 
        return 0; 
    }

    if (pthread_create(&polly_thread, NULL, key_poll, NULL) != 0) {
        printf("Thread creation for polly has failed\n");
        close(keyboard_file);
        keyboard_file = -1;
        return 0;
    }
    
    return 1;
}

int stop_polly(void) {
    pthread_cancel(polly_thread);
    pthread_join(polly_thread, NULL);  // wait until it has really stopped
    if(keyboard_file != -1){
        close(keyboard_file);
        keyboard_file = -1;
    }
    return 1;
}
