void Particle_Emitter::terminate(){
    particles.free();
}

void Particle_Emitter::spawn_process(Particle& p){
    float theta = theta_min_range[0] + random_float_normalized_positive() * theta_min_range[1];
    vec2 direction = {cos(theta), sin(theta)};
    float velocity = velocity_min_range[0] + random_float_normalized_positive() * velocity_min_range[1];

    p.velocity = velocity * direction;
    p.angle = theta;
    p.angular_velocity = angular_velocity_min_range[0] + random_float_normalized_positive() * angular_velocity_min_range[1];
    p.size = size_min_range[0] + random_float_normalized_positive() * size_min_range[1];
    p.frame_counter = duration_min_range[0] + (u32)(random_float_normalized_positive() * duration_min_range[1]);

    switch(shape){
        case Disc:
            {
                p.position = desc.circle.center + random_on_unit_disc() * desc.circle.radius;
                p.position = p.position + direction * p.size * 0.5f;
                break;
            }
        case Rect:
            {
                p.position = {desc.rect.min.x + random_float_normalized_positive() * desc.rect.max.x,
                    desc.rect.min.y + random_float_normalized_positive() * desc.rect.max.y};
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
        particles.insert(p);
    }
}

void Particle_Emitter::update(){
    assert(duration_min_range[1] > 0u);

    if(particles_per_frame > 0){
        for(s32 iparticle = 0; iparticle != particles_per_frame; ++iparticle){
            Particle p;
            spawn_process(p);
            particles.insert(p);
        }
    }else if(particles_per_frame < 0 && (frame_counter % (u32)(- particles_per_frame)) == 0){
        Particle p;
        spawn_process(p);
        particles.insert(p);
    }

    // NOTE(hugo): update running particles
    u32 index, counter;
    for(index = particles.get_first(), counter = 0u;
        index < particles.capacity && counter != particles.size;
        index = particles.get_next(index), ++counter){

        Particle& p = particles[index];

        if(p.frame_counter-- == 0u){
            particles.remove(index);
            --counter;
        }else{
            update_process(p);
        }
    }

    ++frame_counter;
}

Vertex_Batch_ID Particle_Emitter::batch_as_quad_xyuv(Renderer* renderer){
    Vertex_Batch_ID batch = renderer->get_vertex_batch(xyuv, PRIMITIVE_TRIANGLES);
    vertex_xyuv* vertices = (vertex_xyuv*)renderer->get_vertices(batch, particles.size * 6u);

    u32 index, counter;
    for(index = particles.get_first(), counter = 0u;
        index < particles.capacity && counter != particles.size;
        index = particles.get_next(index), ++counter){

        Particle& p = particles[index];

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
