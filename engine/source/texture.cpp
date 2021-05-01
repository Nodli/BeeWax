void make_texture_asset_from_png_file(Texture_Asset* asset, const File_Path& path, Render_Layer* render_layer){
    s32 width, height, nchannels;
    asset->bitmap = stbi_load(path.data, &width, &height, &nchannels, 4u);
    asset->width = width;
    asset->height = height;
    ENGINE_CHECK(asset->bitmap, "failed to stbi_load with path: %s", path.data);

    asset->texture = render_layer->get_texture(TEXTURE_FORMAT_SRGBA_BYTE, width, height, TYPE_UBYTE, asset->bitmap);
}

void free_texture_asset(Texture_Asset* asset, Render_Layer* render_layer){
    stbi_image_free(asset->bitmap);
    render_layer->free_texture(asset->texture);
}

void free_texture_animation_asset(Texture_Animation_Asset* asset){
    asset->animation_frames.destroy();
}
