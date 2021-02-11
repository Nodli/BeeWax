void make_texture_asset_from_png_file(Texture_Asset* asset, const File_Path& path, Renderer* renderer){
    s32 width, height, nchannels;
    asset->bitmap = stbi_load(path.data, &width, &height, &nchannels, 4u);
    asset->width = width;
    asset->height = height;
    ENGINE_CHECK(asset->bitmap, "failed to stbi_load with path: %s", path.data);

    asset->texture = renderer->get_texture(TEXTURE_FORMAT_RGBA, width, height, TYPE_UBYTE, asset->bitmap);
}

void free_texture_asset(Texture_Asset* asset, Renderer* renderer){
    ::free(asset->bitmap);
    renderer->free_texture(asset->texture);
}
