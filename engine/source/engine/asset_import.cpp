void import_audio_asset_from_json(Audio_Asset* asset, cJSON* json_asset, Audio_Player* audio_player){
    cJSON* json_path = cJSON_GetObjectItemCaseSensitive(json_asset, "file");
    ENGINE_CHECK(json_path && cJSON_IsString(json_path) && json_path->valuestring != nullptr && strlen(json_path->valuestring) <= file_path_capacity,
        "import_audio_asset_from_json expects a 'file' string variable of max. size %u", file_path_capacity);

    File_Path path = asset_path() / json_path->valuestring;

    make_audio_asset_from_wav_file(asset, path, audio_player);
}
void import_texture_asset_from_json(Texture_Asset* asset, cJSON* json_asset, Renderer* renderer){
    cJSON* json_path = cJSON_GetObjectItemCaseSensitive(json_asset, "file");
    ENGINE_CHECK(json_path && cJSON_IsString(json_path) && json_path->valuestring != nullptr && strlen(json_path->valuestring) <= file_path_capacity,
        "import_texture_asset_from_json expects a 'file' string variable of max. size %u", file_path_capacity);

    File_Path path = asset_path() / json_path->valuestring;

    make_texture_asset_from_png_file(asset, path, renderer);
}

void import_asset_catalog_from_json(const File_Path& filename,

        Asset_Catalog<Audio_Asset>* audio_catalog,
        Asset_Catalog<Texture_Asset>* texture_catalog,

        Audio_Player* audio_player,
        Renderer* renderer
    ){

    char* json_str = read_file_cstring(filename);

    cJSON* json_tree = cJSON_Parse(json_str);
    ENGINE_CHECK(json_tree, "cJSON Error: %s", cJSON_GetErrorPtr() ? cJSON_GetErrorPtr() : "no error");

    cJSON* json_assets = cJSON_GetObjectItemCaseSensitive(json_tree, "assets");
    ENGINE_CHECK(json_assets && cJSON_IsArray(json_assets),
            "import_asset_catalog_from_json expects an 'assets' array as root");

    for(u32 iasset = 0u; iasset != cJSON_GetArraySize(json_assets); ++iasset){
        cJSON* json_asset = cJSON_GetArrayItem(json_assets, iasset);

        // NOTE(hugo): extract the asset metadata
        cJSON* json_tag = cJSON_GetObjectItemCaseSensitive(json_asset, "tag");
        ENGINE_CHECK(json_tag && cJSON_IsString(json_tag) && json_tag->valuestring != nullptr && strlen(json_tag->valuestring) <= asset_tag_capacity,
                "import_asset_catalog_from_json expects a 'tag' string variable of max. size %u for asset %u", asset_tag_capacity, iasset);

        cJSON* json_type = cJSON_GetObjectItemCaseSensitive(json_asset, "type");
        ENGINE_CHECK(json_type && cJSON_IsString(json_type) && json_type->valuestring != nullptr,
                "import_asset_catalog_from_json expects a 'type' string variable for asset %u", iasset);

        // NOTE(hugo): register the asset
        Asset_Tag asset_tag;
        asset_tag = json_tag->valuestring;

        if(strcmp(json_type->valuestring, "audio") == 0){
            ENGINE_CHECK(audio_catalog, "asset with tag %s is an audio asset but no /audio_catalog/ was provided", json_tag->valuestring);
            Audio_Asset* asset = audio_catalog->create(asset_tag);
            import_audio_asset_from_json(asset, json_asset, audio_player);

        }else if(strcmp(json_type->valuestring, "texture") == 0){
            ENGINE_CHECK(texture_catalog, "asset with tag %s is a texture asset but no /texture_catalog/ was provided", json_tag->valuestring);
            Texture_Asset* asset = texture_catalog->create(asset_tag);
            import_texture_asset_from_json(asset, json_asset, renderer);

        }else{
            LOG_WARNING("unknown asset type: %s for asset with tag: %s", json_type->valuestring, json_tag->valuestring);
        }
    }

    cJSON_Delete(json_tree);
    ::free(json_str);
}
