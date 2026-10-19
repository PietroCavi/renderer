#ifndef __ASSET_LOADER_CAVI__
#define __ASSET_LOADER_CAVI__

#include "../renderer_structs/renderer_structs.h"
#include <dirent.h>
#include <stdio.h>

texture load_texture(FILE* texture_file, char* texture_name);

char* get_texture_name_from_mtllib(FILE_al mtllib, char* material_id);

asset_bundle load_asset_bundle(char* obj_file_path);

asset_bundle load_asset_bundle_dir(char* directory);


#endif
