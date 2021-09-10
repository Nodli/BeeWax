#ifndef H_ASSET_MANAGEMENT
#define H_ASSET_MANAGEMENT

typedef sstring<32u> Asset_Type;
typedef sstring<64u> Asset_Name;

static const char* asset_folder_path = "./data";

struct Asset_Library{
    Asset_Type type;
    void (*func_create_asset)(Asset_Library* this_ptr, cJSON* json);
    void (*func_destroy_asset)(Asset_Library* this_ptr, Asset_Name name);
};

struct Asset_Loader{
    void create();

    // --

    void create_from_json(const File_Path& json_path);
    void destroy_from_json(const File_Path& json_path);

    // ----

    u32 nlibraries;
    Asset_Library* libraries[120u];
};

struct Audio_Library : Asset_Library{
    void create();
    void destroy();

    // --

    void asset_create_from_json(cJSON* json);
    void asset_destroy(Asset_Name name);

    Audio_Asset* search(Asset_Name name);

    // ----

    Audio_Player* audio;
    hashmap<Asset_Name, Audio_Asset*> map;
};

struct Texture_Library : Asset_Library{
    void create();
    void destroy();

    // --

    void asset_create_from_json(cJSON* json);
    void asset_destroy(Asset_Name name);

    Texture_Asset* search(Asset_Name name);

    // ----

    Render_Layer* rlayer;
    hashmap<Asset_Name, Texture_Asset*> map;
};

struct Texture_Animation_Library : Asset_Library{
    void create();
    void destroy();

    // --

    void asset_create_from_json(cJSON* json);
    void asset_destroy(Asset_Name name);

    Texture_Animation_Asset* search(Asset_Name name);

    // ----

    Render_Layer* rlayer;
    hashmap<Asset_Name, Texture_Animation_Asset*> map;
};

struct Font_Library : Asset_Library{
    void create();
    void destroy();

    // --

    void asset_create_from_json(cJSON* json);
    void asset_destroy(Asset_Name name);

    Font_Asset* search(Asset_Name name);

    // ----

    hashmap<Asset_Name, Font_Asset*> map;
};

#endif
