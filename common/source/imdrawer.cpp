ImDrawer::ImDrawer(){
}

ImDrawer::~ImDrawer(){
    for(auto& buffer : buffers){
        get_engine().render_layer.free_buffer(buffer.buffer);
    }
}

void ImDrawer::new_frame(){
    commands.clear();
    for(auto& buffer : buffers){
        get_engine().render_layer.checkout(buffer.buffer);
        buffer.vertex_count = 0u;
    }
}

void ImDrawer::draw(){
    for(auto& buffer : buffers){
        get_engine().render_layer.commit(buffer.buffer);
    }

    for(auto& command : commands){
        switch(command.type){
            case LINE:
                get_engine().render_layer.use_shader(polygon);
                get_engine().render_layer.draw(buffers[command.line.buffer_index].buffer, PRIMITIVE_LINES, command.line.vertex_index, command.line.vertex_count);
                break;
            case POLYGON:
                get_engine().render_layer.use_shader(polygon);
                get_engine().render_layer.draw(buffers[command.polygon.buffer_index].buffer, PRIMITIVE_TRIANGLES, command.polygon.vertex_index, command.polygon.vertex_count);
                break;
            case POLYGON_TEXTURED:
                get_engine().render_layer.use_shader(polygon_tex);
                get_engine().render_layer.setup_texture_unit(0u, command.polygon_textured.texture, nearest_clamp);
                get_engine().render_layer.draw(buffers[command.polygon_textured.buffer_index].buffer, PRIMITIVE_TRIANGLES, command.polygon_textured.vertex_index, command.polygon_textured.vertex_count);
                break;
            default:
                break;
        }
    }
}

static u32 get_buffer_with_format(ImDrawer& drawer, Vertex_Format_Name vformat, size_t vbytesize, u32 nvertices){
    for(u32 ibuffer = 0u; ibuffer != drawer.buffers.size(); ++ibuffer){
        ImDrawer::Buffer& buffer = drawer.buffers[ibuffer];

        if(buffer.vertex_format_name == vformat && (buffer.buffer.bytesize / vbytesize - buffer.vertex_count) >= nvertices){
            return ibuffer;
        }
    }

    u32 new_buffer_index = drawer.buffers.size();

    ImDrawer::Buffer new_buffer;
    new_buffer.vertex_format_name = vformat;
    new_buffer.buffer = get_engine().render_layer.get_transient_buffer(nvertices * 512u * sizeof(vertex_xyzuv));
    new_buffer.vertex_count = 0u;

    get_engine().render_layer.format(new_buffer.buffer, vformat);
    get_engine().render_layer.checkout(new_buffer.buffer);

    drawer.buffers.push_back(new_buffer);
    return new_buffer_index;
}

void ImDrawer::command_sprite(const Texture_Asset* asset, vec2 position, vec2 size, float depth){
    // NOTE(hugo): find buffer
    u32 buffer_index = get_buffer_with_format(*this, xyzuv, sizeof(vertex_xyzuv), 6u);
    Buffer& buffer = buffers[buffer_index];

    // NOTE(hugo): emit vertices
    u32 vindex = buffer.vertex_count;
    vertex_xyzuv* vptr = (vertex_xyzuv*)buffer.buffer.ptr + vindex;

    vec2 hsize = size * 0.5f;

    *vptr++ = {{position.x - hsize.x, position.y - hsize.y, depth}, uv32(0.f, 0.f)};
    *vptr++ = {{position.x + hsize.x, position.y - hsize.y, depth}, uv32(1.f, 0.f)};
    *vptr++ = {{position.x + hsize.x, position.y + hsize.y, depth}, uv32(1.f, 1.f)};

    *vptr++ = {{position.x - hsize.x, position.y - hsize.y, depth}, uv32(0.f, 0.f)};
    *vptr++ = {{position.x + hsize.x, position.y + hsize.y, depth}, uv32(1.f, 1.f)};
    *vptr++ = {{position.x - hsize.x, position.y + hsize.y, depth}, uv32(0.f, 1.f)};

    buffer.vertex_count += 6u;

    // NOTE(hugo): queue command
    Command command;
    command.type = Command_Type::POLYGON_TEXTURED;
    command.polygon_textured.texture = asset->texture;
    command.polygon_textured.buffer_index = buffer_index;
    command.polygon_textured.vertex_index = vindex;
    command.polygon_textured.vertex_count = 6u;

    commands.push_back(command);
}

void ImDrawer::command_quad(vec2 BL, vec2 BR, vec2 UR, vec2 UL, float depth, u32 rgba){
    // NOTE(hugo): find buffer
    u32 buffer_index = get_buffer_with_format(*this, xyzrgba, sizeof(vertex_xyzrgba), 6u);
    Buffer& buffer = buffers[buffer_index];

    // NOTE(hugo): emit vertices
    u32 vindex = buffer.vertex_count;
    vertex_xyzrgba* vptr = (vertex_xyzrgba*)buffer.buffer.ptr + vindex;

    *vptr++ = {{BL.x, BL.y, depth}, rgba};
    *vptr++ = {{BR.x, BR.y, depth}, rgba};
    *vptr++ = {{UR.x, UR.y, depth}, rgba};

    *vptr++ = {{BL.x, BL.y, depth}, rgba};
    *vptr++ = {{UR.x, UR.y, depth}, rgba};
    *vptr++ = {{UL.x, UL.y, depth}, rgba};

    buffer.vertex_count += 6u;

    // NOTE(hugo): queue command
    Command command;
    command.type = Command_Type::POLYGON;
    command.polygon.buffer_index = buffer_index;
    command.polygon.vertex_index = vindex;
    command.polygon.vertex_count = 6u;

    commands.push_back(command);
}

void ImDrawer::command_line(vec2 A, vec2 B, float depth, u32 rgba){
    // NOTE(hugo): find buffer
    u32 buffer_index = get_buffer_with_format(*this, xyzrgba, sizeof(vertex_xyzrgba), 2u);
    Buffer& buffer = buffers[buffer_index];

    // NOTE(hugo): emit vertices
    u32 vindex = buffer.vertex_count;
    vertex_xyzrgba* vptr = (vertex_xyzrgba*)buffer.buffer.ptr + vindex;

    *vptr++ = {{A.x, A.y, depth}, rgba};
    *vptr++ = {{B.x, B.y, depth}, rgba};

    buffer.vertex_count += 2u;

    // NOTE(hugo): queue command
    Command command;
    command.type = Command_Type::LINE;
    command.polygon.buffer_index = buffer_index;
    command.polygon.vertex_index = vindex;
    command.polygon.vertex_count = 2u;

    commands.push_back(command);
}
