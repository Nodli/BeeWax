void Asset_Loader::create(){
        nlibraries = 0u;
}

void Asset_Loader::create_from_json(const File_Path& json_path){
    File_Data json_file = read_file_cstring(json_path);

    cJSON* json_tree = cJSON_Parse((char*)json_file.data);
    ENGINE_CHECK(json_tree, "cJSON Error: %s", cJSON_GetErrorPtr() ? cJSON_GetErrorPtr() : "no error");

    cJSON* json_assets = cJSON_GetObjectItemCaseSensitive(json_tree, "assets");
    ENGINE_CHECK(json_assets && cJSON_IsArray(json_assets), "Asset_Loader::create_from_json Error: expecting an 'assets' array as root");

    for(u32 iasset = 0u; iasset != cJSON_GetArraySize(json_assets); ++iasset){
        cJSON* json_asset = cJSON_GetArrayItem(json_assets, iasset);

        cJSON* json_type = cJSON_GetObjectItemCaseSensitive(json_asset, "type");
        ENGINE_CHECK(json_type && cJSON_IsString(json_type), "Asset_Loader::create_from_json Error: expecting a 'type' string for asset %d", iasset);

        Asset_Type type;
        type = json_type->valuestring;

        u32 ilib;
        for(ilib = 0u; ilib != nlibraries; ++ilib){
            if(libraries[ilib]->type == json_type->valuestring){
                libraries[ilib]->func_create_asset(libraries[ilib], json_asset);
                break;
            }
        }

        if(ilib == nlibraries) LOG_WARNING("Asset_Loader::create_from_json Error: unknown asset type: %s for asset %d", type.data, iasset);
    }

    cJSON_Delete(json_tree);
    bw_free(json_file.data);
}

void Audio_Library::create(){
    type = "audio";
    func_create_asset = [](Asset_Library* this_ptr, cJSON* json){
        Audio_Library* tptr = (Audio_Library*)this_ptr;
        (*tptr).asset_create_from_json(json);
    };
    func_destroy_asset = [](Asset_Library* this_ptr, Asset_Name name){
        Audio_Library* tptr = (Audio_Library*)this_ptr;
        (*tptr).asset_destroy(name);
    };
    map.create();
}

void Audio_Library::destroy(){
    for(auto& kv : map){
        free_audio_asset(kv.value());
        bw_free(kv.value());
    }
    map.destroy();
}

void Audio_Library::asset_create_from_json(cJSON* json){
    cJSON* json_name = cJSON_GetObjectItemCaseSensitive(json, "name");
    assert(json_name && cJSON_IsString(json_name) && strlen(json_name->valuestring) <= Asset_Name::strcap());

    Asset_Name name;
    name = json_name->valuestring;

    Audio_Asset** asset;
    if(map.get(name, asset)){

        cJSON* json_path = cJSON_GetObjectItemCaseSensitive(json, "file");
        assert(json_path && cJSON_IsString(json_path));

        File_Path path;
        path = asset_folder_path;
        path /= json_path->valuestring;

        void* memory = bw_malloc(sizeof(Audio_Asset));
        assert(memory);

        *asset = (Audio_Asset*)memory;
        make_audio_asset_from_wav_file(*asset, path, audio);

    }else{
        LOG_WARNING("asset name: %s was already in Audio_Library", name.data);

    }
}

void Audio_Library::asset_destroy(Asset_Name name){
    auto remove_procedure = [](Audio_Asset*& asset){
        free_audio_asset(asset);
        bw_free(asset);
    };
    if(!map.remove_func(name, remove_procedure)){
        LOG_WARNING("asset name: %s was not in Audio_Library", name.data);
    }
}

Audio_Asset* Audio_Library::search(Asset_Name name){
    Audio_Asset** out_search;
    if(map.search(name, out_search)){
        return *out_search;
    }else{
        return nullptr;
    }
}
