#include "asset_loader.h"


int to_color_cavigl(const unsigned char red,const unsigned char green,const unsigned char blue){
	int rgb = red;
	rgb = (rgb << 8) + green;
	return (rgb << 8) + blue;
}

int skip_line(FILE *fp)
{
    int c;
    while (c = fgetc(fp), c != '\n' && c != EOF);
    return c;
}

int starts_with(char*line,char*start){
    if(strlen(start) > strlen(line)){
        return 0;
    }

    for(int i=0;start[i];i++){
        if(start[i] != line[i])
            return 0;
    }

    return 1;
}

// Metodo fatto dall'AI per mancanza di voglia
char* get_dir(char* path){
    const char* slash = strrchr(path, '/');
    if(slash == NULL){
        return strdup("");          // no directory part: file is in the cwd
    }

    size_t len = (size_t)(slash - path) + 1;   // +1 to keep the trailing '/'
    char* dir = malloc(len + 1);
    memcpy(dir, path, len);
    dir[len] = '\0';
    return dir;                     // caller frees
}

texture load_texture(FILE* texture_file, char* texture_name){
    char type_str[3];
    fscanf(texture_file,"%s",type_str);
    skip_line(texture_file);
   
    int length;
    int height;
    fscanf(texture_file,"%d",&length);
    fscanf(texture_file,"%d",&height);
    skip_line(texture_file);

    skip_line(texture_file); // we assume 24 bit color, this skipped line should contain 255 

    int num1;
    int num2;
    int num3;
    
    int* colors = malloc(sizeof(int)*length*height);
    
    int i=0;
  
    const char *parse_string = strcmp("P3",type_str)==0 ? "%d%d%d" : "%c%c%c"; 

    while(fscanf(texture_file,parse_string,&num1,&num2,&num3) != EOF){
        colors[i] = to_color_cavigl(num1,num2,num3);
        i++;
    }

    rewind(texture_file);
    
    return (texture){
                     .file_name = texture_name,
                     .colors = colors,
                     .height = height,
                     .length = length
                    };
}

char* get_texture_name_from_mtllib(FILE_al mtllib, char* material_id){
    char* ret = NULL;
    
    char *line = NULL;
    size_t cap = 0;
    
    int ok = 0;
    
    for(size_t i=0;i<mtllib.size;i++){
        
        FILE* mtl_file = mtllib.items[i];

        while (getline(&line, &cap, mtl_file) != EOF) {
            if(starts_with(line,"newmtl")){
                ret = calloc(strlen(line)+1,sizeof(char));
                sscanf(line,"newmtl %s",ret);
                ok = strcmp(ret,material_id)==0;
                free(ret);
                ret = NULL;
            }
            if(ok && starts_with(line,"map_Kd")){
                ret = calloc(strlen(line)+1,sizeof(char));
                sscanf(line,"map_Kd %s",ret);
                break;
            }
        }

        rewind(mtl_file);

        if(ret != NULL){
            break;
        }
    }

    free(line);

    return ret;
}

asset_bundle load_asset_bundle(char* obj_file_path){
    
    FILE* obj_file = fopen(obj_file_path,"r");
    char* dir_name = get_dir(obj_file_path);

    model_al models = {0};
    float_al vertex_buffer = {0};
    float_al uv_vertex_buffer = {0};
    
    FILE_al mtllib = {0};

    //---------//

    // Implementation of the actual loading of the bundle will be here
    
    size_t cap = 0;
    char * line = NULL;

    model current_model = {0};
    
    current_model.texture_index = -1;

    char first_time = 1;
    
    int parsed_ints[9];
    float parsed_floats[3];

    while (getline(&line, &cap, obj_file) != EOF) {
        if(starts_with(line,"f")){
            sscanf(line,"f %d/%d/%d %d/%d/%d %d/%d/%d",
                &parsed_ints[0],&parsed_ints[1],&parsed_ints[2],
                &parsed_ints[3],&parsed_ints[4],&parsed_ints[5],
                &parsed_ints[6],&parsed_ints[7],&parsed_ints[8]
            );

            parsed_ints[0]--;
            parsed_ints[3]--;
            parsed_ints[6]--;

            parsed_ints[1]--;
            parsed_ints[4]--;
            parsed_ints[7]--;
            
            append_al(current_model.index_buffer,parsed_ints[0]);
            append_al(current_model.index_buffer,parsed_ints[3]);
            append_al(current_model.index_buffer,parsed_ints[6]);
            
            append_al(current_model.uv_index_buffer,parsed_ints[1]);
            append_al(current_model.uv_index_buffer,parsed_ints[4]);
            append_al(current_model.uv_index_buffer,parsed_ints[7]);
        
            // parsed_ints 2,5,8 are the normals of the triangle as we ignore "vn" we will just ignore it
        }
        else if(starts_with(line,"v ")){
            sscanf(line,"v %f %f %f",&parsed_floats[0],&parsed_floats[1],&parsed_floats[2]);
            
            append_al(vertex_buffer,parsed_floats[0]);
            append_al(vertex_buffer,parsed_floats[1]);
            append_al(vertex_buffer,parsed_floats[2]);
        }
        else if(starts_with(line,"vt")){
            sscanf(line,"vt %f %f",&parsed_floats[0],&parsed_floats[1]);
            
            append_al(uv_vertex_buffer,parsed_floats[0]);
            append_al(uv_vertex_buffer,parsed_floats[1]);

            // parsed_floats 2 in this case is ignored, the uv coordinates ("vt") are 2D vectors on a textures, so it's fine.
        }
        else if(starts_with(line,"usemtl ")){
            // I'll have to go and get the texture and add it to the model
            char* texture_name = calloc(strlen(line)+1,sizeof(char));
            sscanf(line,"usemtl %s",texture_name);

            char* texture_file_name = get_texture_name_from_mtllib(mtllib,texture_name);
            
            if(texture_file_name == NULL){
                printf("Warning: missing texture - [%s]\n",texture_name);
                free(texture_name);

                current_model.texture_index = -1;

                continue;
            }

            char* texture_complete_path = calloc(strlen(texture_file_name)+strlen(dir_name)+1,sizeof(char));
            strcpy(texture_complete_path,dir_name);
            strcat(texture_complete_path,texture_file_name);
            
            char found = 0;
            if(texture_complete_path!= NULL){
                
                for(size_t i=0;i<global_texture_buffer.size;i++){
                    if(strcmp(global_texture_buffer.items[i].file_name,texture_complete_path)==0){
                        found = 1;

                        current_model.texture_index = i;
                        break;
                    }
                }
                
                if(!found){
                    FILE* texture_file = fopen(texture_complete_path,"r");
                    
                    // TODO: aggiungere la texture al bundle e al current_model
                    if(texture_file != NULL){
                        texture current_texture = load_texture(texture_file, texture_complete_path);

                        append_al(global_texture_buffer, current_texture);

                        current_model.texture_index = global_texture_buffer.size-1;
                        
                        fclose(texture_file);
                    }
                }
                else{
                    // in teoria è giusto
                    free(texture_complete_path);
                }
            }

            free(texture_name);
            free(texture_file_name);

        }
        else if(starts_with(line,"mtllib ")){
            // I'll have to get the referenced file and add it to my array_list of files
            char* mtl_file_name = calloc(strlen(line)+1,sizeof(char));
            sscanf(line,"mtllib %s",mtl_file_name);

            char* mtl_complete_path = calloc(strlen(mtl_file_name)+strlen(dir_name)+1,sizeof(char));
            
            strcpy(mtl_complete_path,dir_name);

            strcat(mtl_complete_path,mtl_file_name);
            
            FILE* mtl_file = fopen(mtl_complete_path,"r");
            
            if(mtl_file != NULL){
                append_al(mtllib,mtl_file);
            }

            free(mtl_file_name);
            free(mtl_complete_path);

        }
        else if(starts_with(line,"o ")){
            // this means that I have a new model, it will act as a reset for my currentlty parsed model
            
            if(!first_time){
                trim_to_size_al(current_model.index_buffer);
                trim_to_size_al(current_model.uv_index_buffer);
                append_al(models,current_model);
            }

            current_model = (model){0};
            
            first_time = 0;
        }

    }

    if(current_model.index_buffer.size != 0){
        append_al(models,current_model);
    }

    //---------//

    for(size_t i=0;i<mtllib.size;i++){
        fclose(mtllib.items[i]);
    }
    if(mtllib.items != NULL){
        free(mtllib.items);
    }
    
    fclose(obj_file);
    
    free(dir_name);

    trim_to_size_al(models);
    trim_to_size_al(global_texture_buffer);
    trim_to_size_al(vertex_buffer);
    trim_to_size_al(uv_vertex_buffer);

    free(line);

    return (asset_bundle){
                          .models = models,
                          .vertex_buffer = vertex_buffer,
                          .uv_vertex_buffer = uv_vertex_buffer
                         };
}

asset_bundle load_asset_bundle_dir(char* directory_name){
    // Here I'll be cycling all the .obj files of a dir and then merge all the bundles into a final big bundle!
     
    DIR* directory = opendir(directory_name);
    
    asset_bundle ret = {0};
    asset_bundle prov = {0};

    if(directory != NULL){

        struct dirent *entry;
        
        while ((entry = readdir(directory)) != NULL) {
            char *name = entry->d_name;
            
            char *complete_path = calloc(strlen(directory_name)+strlen(name)+10,sizeof(char));

            strcat(complete_path,directory_name);
            if(directory_name[strlen(directory_name)-1]!='/'){
                strcat(complete_path,"/");
            }
            strcat(complete_path,name);

            int len = strlen(name);
            if (len > 4 && strcmp(name + len - 4, ".obj") == 0) {
                prov = load_asset_bundle(complete_path);
                ret = merge_asset_bundle(ret,prov);
                free(prov.models.items);
                free(prov.vertex_buffer.items);
                free(prov.uv_vertex_buffer.items);
            }

            free(complete_path);
        }
    
    }

    closedir(directory);
    
    trim_to_size_al(global_texture_buffer);
    trim_to_size_al(ret.models);
    trim_to_size_al(ret.vertex_buffer);
    trim_to_size_al(ret.uv_vertex_buffer);

    return ret;
}
