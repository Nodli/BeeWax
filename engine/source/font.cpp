void make_font_asset_from_ttf_file(Font_Asset* asset, const File_Path& path){
    (*asset).ttf = read_file(path, "rb");
    stbtt_InitFont(&(*asset).info, (uchar*)(*asset).ttf.data, 0u);
}

void free_font_asset(Font_Asset* asset){
    bw_free((*asset).ttf.data);
}
