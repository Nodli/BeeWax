void import_asset_from_json(const File_Path& json, Engine* engine, Asset_Catalog_Description* description, u32 ndescription){
    char* json_str = read_file_cstring(json);

    cJSON* json_tree = cJSON_Parse(json_str);
    ENGINE_CHECK(json_tree, "cJSON Error: %s", cJSON_GetErrorPtr() ? cJSON_GetErrorPtr() : "no error");

    cJSON* json_assets = cJSON_GetObjectItemCaseSensitive(json_tree, "assets");
    ENGINE_CHECK(json_assets && cJSON_IsArray(json_assets),
            "import_asset_catalog_from_json expects an 'assets' array as root");

    for(u32 iasset = 0u; iasset != cJSON_GetArraySize(json_assets); ++iasset){
        cJSON* json_asset = cJSON_GetArrayItem(json_assets, iasset);

        // NOTE(hugo): extract the asset metadata
        cJSON* json_tag = cJSON_GetObjectItemCaseSensitive(json_asset, "tag");
        cJSON* json_type = cJSON_GetObjectItemCaseSensitive(json_asset, "type");

        assert(json_tag     && cJSON_IsString(json_tag)     && !(strlen(json_tag->valuestring) > asset_string_capacity));
        assert(json_type    && cJSON_IsString(json_type)    && !(strlen(json_type->valuestring) > asset_string_capacity));

        // NOTE(hugo): register the asset in a catalog
        Asset_Type_Tag json_type_tag;
        json_type_tag = json_type->valuestring;

        u32 idesc;
        for(idesc = 0u; idesc != ndescription; ++idesc){
            if(description[idesc].type == json_type_tag){
                void* asset_ptr;
                description[idesc].create_asset_from_json(asset_ptr, engine, json_asset);

                Asset_Tag asset_tag;
                asset_tag = json_tag->valuestring;

                void** map_ptr;
                u32 get_result = description[idesc].map_ptr->get(asset_tag, map_ptr);
                assert(get_result);
                *map_ptr = asset_ptr;

                break;
            }
        }

        if(idesc == ndescription){
            LOG_WARNING("unknown asset type: %s for asset with tag: %s", json_type->valuestring, json_tag->valuestring);
        }
    }

    cJSON_Delete(json_tree);
    bw_free(json_str);
}

void remove_asset_from_json(const File_Path& json, Engine* engine, Asset_Catalog_Description* description, u32 ndescription){
    char* json_str = read_file_cstring(json);

    cJSON* json_tree = cJSON_Parse(json_str);
    ENGINE_CHECK(json_tree, "cJSON Error: %s", cJSON_GetErrorPtr() ? cJSON_GetErrorPtr() : "no error");

    cJSON* json_assets = cJSON_GetObjectItemCaseSensitive(json_tree, "assets");
    ENGINE_CHECK(json_assets && cJSON_IsArray(json_assets),
            "import_asset_catalog_from_json expects an 'assets' array as root");

    for(u32 iasset = 0u; iasset != cJSON_GetArraySize(json_assets); ++iasset){
        cJSON* json_asset = cJSON_GetArrayItem(json_assets, iasset);

        // NOTE(hugo): extract the asset metadata
        cJSON* json_tag = cJSON_GetObjectItemCaseSensitive(json_asset, "tag");
        cJSON* json_type = cJSON_GetObjectItemCaseSensitive(json_asset, "type");

        assert(json_tag     && cJSON_IsString(json_tag)     && !(strlen(json_tag->valuestring) > asset_string_capacity));
        assert(json_type    && cJSON_IsString(json_type)    && !(strlen(json_type->valuestring) > asset_string_capacity));

        // NOTE(hugo): register the asset in a catalog
        Asset_Type_Tag json_type_tag;
        json_type_tag = json_type->valuestring;

        u32 idesc;
        for(idesc = 0u; idesc != ndescription; ++idesc){
            if(description[idesc].type == json_type_tag){
                Asset_Tag asset_tag;
                asset_tag = json_tag->valuestring;

                void** map_ptr;
                u32 search_result = description[idesc].map_ptr->search(asset_tag, map_ptr);
                assert(search_result);

                description[idesc].destroy_asset(*map_ptr, engine);
                break;
            }
        }

        if(idesc == ndescription){
            LOG_WARNING("unknown asset type: %s for asset with tag: %s", json_type->valuestring, json_tag->valuestring);
        }
    }

    cJSON_Delete(json_tree);
    bw_free(json_str);
}

void create_Audio_Asset_from_json(void*& out_ptr, Engine* engine, cJSON* json){
    cJSON* json_path = cJSON_GetObjectItemCaseSensitive(json, "file");
    assert(json_path && cJSON_IsString(json_path) && !(strlen(json_path->valuestring) > file_path_capacity));

    File_Path path = asset_path() / json_path->valuestring;

    void* memory = bw_malloc(sizeof(Audio_Asset));
    assert(memory);

    make_audio_asset_from_wav_file((Audio_Asset*)memory, path, &engine->audio);

    out_ptr = memory;
}

void destroy_Audio_Asset(void* asset, Engine* engine){
    free_audio_asset((Audio_Asset*)asset);
    bw_free(asset);
}

void create_Texture_Asset_from_json(void*& out_ptr, Engine* engine, cJSON* json){
    cJSON* json_path = cJSON_GetObjectItemCaseSensitive(json, "file");
    assert(json_path && cJSON_IsString(json_path) && !(strlen(json_path->valuestring) > file_path_capacity));

    File_Path path = asset_path() / json_path->valuestring;

    void* memory = bw_malloc(sizeof(Texture_Asset));
    assert(memory);

    make_texture_asset_from_png_file((Texture_Asset*)memory, path, &engine->render_layer);

    out_ptr = memory;
}

void destroy_Texture_Asset(void* asset, Engine* engine){
    free_texture_asset((Texture_Asset*)asset, &engine->render_layer);
    bw_free(asset);
}

void create_Texture_Animation_Asset_from_json(void*& out_ptr, Engine* engine, cJSON* json){
}

void destroy_Texture_Animation_Asset(void* asset, Engine* engine){
}
