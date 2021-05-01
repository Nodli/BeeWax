#ifndef H_ASSET_IMPORTER
#define H_ASSET_IMPORTER

struct Asset_Catalog_Description{
    Asset_Type_Tag type;
    hashmap<Asset_Tag, void*>* map_ptr;
    void (*create_asset_from_json)(void*& out_asset_ptr, Engine* engine, cJSON* json);
    void (*destroy_asset)(void* asset_ptr, Engine* engine);
};

void import_asset_from_json(const File_Path& json, Engine* engine, Asset_Catalog_Description* description, u32 ndescription);
void remove_asset_from_json(const File_Path& json, Engine* engine, Asset_Catalog_Description* description, u32 ndescription);

void create_Audio_Asset_from_json(void*& out_ptr, Engine* engine, cJSON* json);
void destroy_Audio_Asset(void* asset, Engine* engine);

void create_Texture_Asset_from_json(void*& out_ptr, Engine* engine, cJSON* json);
void destroy_Texture_Asset(void* asset, Engine* engine);

void create_Texture_Animation_Asset_from_json(void*& out_ptr, Engine* engine, cJSON* json);
void destroy_Texture_Animation_Asset(void* asset, Engine* engine);

#endif
