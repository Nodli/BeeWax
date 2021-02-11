#ifndef H_ASSET_IMPORT
#define H_ASSET_IMPORT

void import_audio_asset_from_json(Audio_Asset* asset, cJSON* json_asset, Audio_Player* audio_player);
void import_texture_asset_from_json(Texture_Asset* asset, cJSON* json_asset, Renderer* renderer);

void import_asset_catalog_from_json(const File_Path& filename,

        Asset_Catalog<Audio_Asset>* audio_catalog,
        Asset_Catalog<Texture_Asset>* texture_catalog,

        Audio_Player* audio_player,
        Renderer* renderer
    );

#endif
