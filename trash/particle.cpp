void Particle_Emitter::terminate(){
    storage.free();
}

void Particle_Emitter::spawn_process(Particle& p){
    float theta = theta_min_range[0] + random_float() * theta_min_range[1];
    vec2 direction = {cos(theta), sin(theta)};
    float velocity = velocity_min_range[0] + random_float() * velocity_min_range[1];

    p.velocity = velocity * direction;
    p.angle = theta;
    p.angular_velocity = angular_velocity_min_range[0] + random_float() * angular_velocity_min_range[1];
    p.size = size_min_range[0] + random_float() * size_min_range[1];
    p.frame_counter = duration_min_range[0] + (u32)(random_float() * duration_min_range[1]);

    switch(shape){
        case Disc:
            {
                p.position = desc.circle.center + random_on_unit_disc() * desc.circle.radius;
                p.position = p.position + direction * p.size * 0.5f;
                break;
            }
        case Rect:
            {
                p.position = {desc.rect.min.x + random_float() * desc.rect.max.x,
                    desc.rect.min.y + random_float() * desc.rect.max.y};
                break;
            }
    };
}

void Particle_Emitter::update_process(Particle& p){
    p.angle += p.angular_velocity;
    p.position += p.velocity;
}

void Particle_Emitter::burst(u32 nparticles){
    for(u32 iparticle = 0u; iparticle != nparticles; ++iparticle){
        Particle p;
        spawn_process(p);
        storage.push(p);
    }
}

void Particle_Emitter::update(){
    assert(duration_min_range[1] > 0u);

    if(particles_per_frame > 0){
        for(s32 iparticle = 0; iparticle != particles_per_frame; ++iparticle){
            Particle p;
            spawn_process(p);
            storage.push(p);
        }
    }else if(particles_per_frame < 0 && (frame_counter % (u32)(- particles_per_frame)) == 0){
        Particle p;
        spawn_process(p);
        storage.push(p);
    }

    // NOTE(hugo): update running particles
    u32 ipart = 0u;
    while(ipart < storage.size){
        Particle& p = storage[ipart];
        if(p.frame_counter-- == 0u){
            storage.remove_swap(ipart);
        }else{
            update_process(p);
            ++ipart;
        }
    }

    ++frame_counter;
}

Vertex_Batch_ID Particle_Emitter::batch_as_quad_xyuv(Renderer* renderer){
    Vertex_Batch_ID batch = renderer->get_vertex_batch(xyuv, PRIMITIVE_TRIANGLES);
    vertex_xyuv* vertices = (vertex_xyuv*)renderer->get_vertices(batch, storage.size * 6u);

    for(u32 ipart = 0u; ipart != storage.size; ++ipart){
        Particle& p = storage[ipart];

        vec2 p_up = {cos(p.angle), sin(p.angle)};
        vec2 p_right = {p_up.y, - p_up.x};

        vertices[0].vposition = p.position - p_right * p.size - p_up * p.size;
        vertices[1].vposition = p.position + p_right * p.size - p_up * p.size;
        vertices[2].vposition = p.position - p_right * p.size + p_up * p.size;
        vertices[5].vposition = p.position + p_right * p.size + p_up * p.size;

        vertices[0].vtexcoord = {0.f, 0.f};
        vertices[1].vtexcoord = {1.f, 0.f};
        vertices[2].vtexcoord = {0.f, 1.f};
        vertices[5].vtexcoord = {1.f, 1.f};

        vertices[3] = vertices[2];
        vertices[4] = vertices[1];

        vertices += 6u;
    }

    return batch;
}
