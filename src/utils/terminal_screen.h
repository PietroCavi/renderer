/*

To use the library:

    #define TERMINAL_SCREEN_IMPLEMENTATION
    #include "terminal_screen.h"

*/



#ifndef __TERMINAL_SCREEN__
#define __TERMINAL_SCREEN__

#include <stdlib.h>

extern char* screen_str;
extern int   screen_width;
extern int   screen_height;

int init_terminal_screen(int width, int height);

void terminate_terminal_screen();

void load_terminal_screen(int* colors);


#endif

#ifdef TERMINAL_SCREEN_IMPLEMENTATION

char* screen_str = NULL;
int   screen_width;
int   screen_height;

int init_terminal_screen(int width, int height){
    screen_width  = width;
    screen_height = height;
    
    if(screen_str!=NULL){
        free(screen_str);
    }

    screen_str = malloc(sizeof(char)*(width*height*24 + height+1));
    if(screen_str==NULL){
        return 0;
    }

    return 1;
}

void terminate_terminal_screen(){
    free(screen_str);
}

static const char* dec255[256] = { 
                    "0"  , "1"  , "2"  , "3"  , "4"  , "5"  , "6"  , "7"  , "8"  , "9"  , 
                    "10" , "11" , "12" , "13" , "14" , "15" , "16" , "17" , "18" , "19" , 
                    "20" , "21" , "22" , "23" , "24" , "25" , "26" , "27" , "28" , "29" , 
                    "30" , "31" , "32" , "33" , "34" , "35" , "36" , "37" , "38" , "39" , 
                    "40" , "41" , "42" , "43" , "44" , "45" , "46" , "47" , "48" , "49" , 
                    "50" , "51" , "52" , "53" , "54" , "55" , "56" , "57" , "58" , "59" , 
                    "60" , "61" , "62" , "63" , "64" , "65" , "66" , "67" , "68" , "69" , 
                    "70" , "71" , "72" , "73" , "74" , "75" , "76" , "77" , "78" , "79" , 
                    "80" , "81" , "82" , "83" , "84" , "85" , "86" , "87" , "88" , "89" , 
                    "90" , "91" , "92" , "93" , "94" , "95" , "96" , "97" , "98" , "99" , 
                    "100", "101", "102", "103", "104", "105", "106", "107", "108", "109", 
                    "110", "111", "112", "113", "114", "115", "116", "117", "118", "119", 
                    "120", "121", "122", "123", "124", "125", "126", "127", "128", "129", 
                    "130", "131", "132", "133", "134", "135", "136", "137", "138", "139", 
                    "140", "141", "142", "143", "144", "145", "146", "147", "148", "149", 
                    "150", "151", "152", "153", "154", "155", "156", "157", "158", "159", 
                    "160", "161", "162", "163", "164", "165", "166", "167", "168", "169", 
                    "170", "171", "172", "173", "174", "175", "176", "177", "178", "179", 
                    "180", "181", "182", "183", "184", "185", "186", "187", "188", "189", 
                    "190", "191", "192", "193", "194", "195", "196", "197", "198", "199", 
                    "200", "201", "202", "203", "204", "205", "206", "207", "208", "209", 
                    "210", "211", "212", "213", "214", "215", "216", "217", "218", "219", 
                    "220", "221", "222", "223", "224", "225", "226", "227", "228", "229", 
                    "230", "231", "232", "233", "234", "235", "236", "237", "238", "239", 
                    "240", "241", "242", "243", "244", "245", "246", "247", "248", "249", 
                    "250", "251", "252", "253", "254", "255" };

unsigned char get_red_cavigl(const int color){
	return (color >> 16) & 0xFF;
}

unsigned char get_green_cavigl(const int color){
	return (color >> 8) & 0xFF;
}
	
unsigned char get_blue_cavigl(const int color){
	return color & 0xFF;
}

void load_terminal_screen(int* colors){
    unsigned char red_c; 
    unsigned char green_c; 
    unsigned char blue_c; 
    const char* red; 
    const char* green; 
    const char* blue; 
    int holder = -1; 
    int j=0; 
    for(int i=0;i<screen_height*screen_width;i++){ 
        if(colors[i]!=holder){ 
            holder=colors[i]; 
            
            red_c   = get_red_cavigl(holder); 
            green_c = get_green_cavigl(holder); 
            blue_c  = get_blue_cavigl(holder); 

            screen_str[j++]=27; 
            screen_str[j++]='['; 
            screen_str[j++]='4'; 
            screen_str[j++]='8'; 
            screen_str[j++]=';'; 
            screen_str[j++]='2'; 
            screen_str[j++]=';'; 

            red = dec255[red_c];
            blue = dec255[blue_c];
            green = dec255[green_c];

            for(int c=0;red[c]!=0;c++) 
                screen_str[j++]=red[c]; 

            screen_str[j++]=';'; 

            for(int c=0;green[c]!=0;c++) 
                screen_str[j++]=green[c]; 

            screen_str[j++]=';'; 

            for(int c=0;blue[c]!=0;c++) 
                screen_str[j++]=blue[c]; 

            screen_str[j++]='m'; 
        } 

        screen_str[j++]=' '; 

        if((i+1) % screen_width == 0){ 
            holder = -1; 

            screen_str[j++]=27; 
            screen_str[j++]='['; 
            screen_str[j++]='0'; 
            screen_str[j++]='m'; 
            screen_str[j++]='\n'; 
        } 
    } 
    
    screen_str[j]='\0';
}

#endif
