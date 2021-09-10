#ifndef H_FONT
#define H_FONT

struct Font_Asset{
    File_Data ttf;
    stbtt_fontinfo info;
};

void make_font_asset_from_ttf_file(Font_Asset* asset, const File_Path& path);
void free_font_asset(Font_Asset* asset);

#endif
