struct Physics_Body{
    float half_width;
    float half_height;
};

struct Position_Constraint{
};

// NOTE(hugo): normal towards /ray_pos/ and away from the /box_pos/
static inline void intersection_ray_AABB(
        const vec2 ray_pos, const vec2 ray_dir,
        const vec2 box_pos, const vec2 box_dim,
        float infinity,
        float& out_t, vec2& out_normal
        ){

    float t = infinity;
    vec2 normal;

    vec2 relative_position = ray_pos - box_pos;

    if(ray_dir.x > 0.f){
        float t_left = (- box_dim.x - relative_position.x) / ray_dir.x;
        float hit_y = relative_position.y + t_left * ray_dir.y;
        if(t_left >= 0.f && !(hit_y < - box_dim.y || hit_y > box_dim.y) && t_left < t){
            t = t_left;
            normal = {-1.f, 0.f};
        }
    }else if(ray_dir.x < 0.f){
        float t_right = (box_dim.x - relative_position.x) / ray_dir.x;
        float hit_y = relative_position.y + t_right * ray_dir.y;
        if(t_right >= 0.f && !(hit_y < - box_dim.y || hit_y > box_dim.y) && t_right < t){
            t = t_right;
            normal = {1.f, 0.f};
        }
    }

    if(ray_dir.y > 0.f){
        float t_bot = (- box_dim.y - relative_position.y) / ray_dir.y;
        float hit_x = relative_position.x + t_bot * ray_dir.x;
        if(t_bot >= 0.f && !(hit_x < - box_dim.x || hit_x > box_dim.x) && t_bot < t){
            t = t_bot;
            normal = {0.f, -1.f};
        }
    }else if(ray_dir.y < 0.f){
        float t_top = (box_dim.y - relative_position.y) / ray_dir.y;
        float hit_x = relative_position.x + t_top * ray_dir.x;
        if(t_top >= 0.f && !(hit_x < - box_dim.x || hit_x > box_dim.x) && t_top < t){
            t = t_top;
            normal = {0.f, 1.f};
        }
    }

    out_t = t;
    out_normal = normal;
}

static inline void moving_body_AABB(
        const vec2 position, const vec2 velocity, const Physics_Body body,
        vec2& out_min, vec2& out_max
        ){

    vec2 start_min = {position.x - body.half_width, position.y - body.half_height};
    vec2 start_max = {position.x + body.half_width, position.y + body.half_height};

    vec2 end_min = start_min + velocity;
    vec2 end_max = start_max + velocity;

    out_min = {min(start_min.x, end_min.x), min(start_min.y, end_min.y)};
    out_max = {max(start_max.x, end_max.x), max(start_max.y, end_max.y)};
}

struct Physics_Context{
    enum Shape_Type{
        NONE,
        STATIC,
        KINEMATIC,
    };

    struct Static_Shape{
        vec2 position;
        Physics_Body body;
        indexmap_handle handle;
    };
    struct Static_Handle : indexmap_handle {};

    struct Kinematic_Shape{
        vec2 position;
        vec2 velocity;
        Physics_Body body;
        indexmap_handle handle;

        struct{
            u32 constraint_group;
            vec2 speculative_velocity;
            // NOTE(hugo): normal towards this body
            vec2 contact_normal;
            Shape_Type contact_type;
        } internal;
    };
    struct Kinematic_Handle : indexmap_handle {};

    struct Report{
        float t;
        vec2 normal;
    };

    // ----

    void create(){
        static_map.create();
        static_data.create();

        kinematic_map.create();
        kinematic_data.create();
    }
    void destroy(){
        static_map.destroy();
        static_data.destroy();

        kinematic_map.destroy();
        kinematic_data.destroy();
    }

    // -- STATIC

    Static_Handle create_static(vec2 position, Physics_Body body){
        u32 index = static_data.size;

        indexmap_handle handle = static_map.borrow_handle();
        *static_map.search(handle) = index;

        Static_Shape shape;
        shape.position = position;
        shape.body = body;
        shape.handle = handle;
        static_data.push(shape);

        Static_Handle out = {handle};
        return out;
    }

    void destroy_static(Static_Handle handle){
        u32* index = static_map.search(handle);
        if(index){
            static_data[*index] = static_data[static_data.size - 1u];
            *static_map.search(static_data[static_data.size - 1u].handle) = *index;
            static_data.pop();
        }
    }

    // NOTE(hugo): collision when Report::t is equal to or less than 1.0 such as dposition = Report::t * velocity
    // ! no contact margin ; epsilon is not substracted !
    Report move_against_static(vec2 position, vec2 velocity, Physics_Body body){
        assert(velocity.x != 0.f || velocity.y != 0.f);

        Report report;
        report.t = infinity;
        report.normal = {0.f, 0.f};

        for(auto& s : static_data){
            vec2 minkowski_shape = {
                body.half_width + s.body.half_width,
                body.half_height + s.body.half_height
            };

            float t;
            vec2 normal;
            intersection_ray_AABB(position, velocity, s.position, minkowski_shape, infinity, t, normal);

            if(t < report.t){
                report.t = t;
                report.normal = normal;
            }
        }

        return report;
    }

    // NOTE(hugo): ray hit when Report::t is less than infinity
    // ! no contact margin ; epsilon is not substracted !
    Report raycast_against_static(vec2 position, vec2 direction){
        assert(direction.x != 0.f || direction.y != 0.f);

        Report report;
        report.t = infinity;
        report.normal = {0.f, 0.f};

        for(auto& s : static_data){
            vec2 minkowski_shape = {
                s.body.half_width,
                s.body.half_height
            };

            float t;
            vec2 normal;
            intersection_ray_AABB(position, direction, s.position, minkowski_shape, infinity, t, normal);

            if(t < report.t){
                report.t = t;
                report.normal = normal;
            }
        }

        return report;
    }

    // -- KINEMATIC

    Kinematic_Handle create_kinematic(vec2 position, Physics_Body body){
        u32 index = kinematic_data.size;

        indexmap_handle handle = kinematic_map.borrow_handle();
        *kinematic_map.search(handle) = index;

        Kinematic_Shape shape;
        shape.position = position;
        shape.velocity = {0.f, 0.f};
        shape.body = body;
        shape.handle = handle;
        kinematic_data.push(shape);

        Kinematic_Handle out = {handle};
        return out;
    }

    void destroy_kinematic(Kinematic_Handle handle){
        u32* index = kinematic_map.search(handle);
        if(index){
            kinematic_data[*index] = kinematic_data[kinematic_data.size - 1u];
            *kinematic_map.search(kinematic_data[kinematic_data.size - 1u].handle) = *index;
            kinematic_data.pop();
        }
    }

    vec2 get_position(Kinematic_Handle handle){
        u32* index = kinematic_map.search(handle);
        if(index){
            return kinematic_data[*index].position;
        }
        return {0.f, 0.f};
    }

    void set_velocity(Kinematic_Handle handle, vec2 movement){
        u32* index = kinematic_map.search(handle);
        if(index){
            kinematic_data[*index].velocity = movement;
        }
    }

    // --

    // REF(hugo):
    // https://www.gamasutra.com/view/feature/131790/simple_intersection_tests_for_games.php?print=1
    // https://blog.hamaluik.ca/posts/swept-aabb-collision-using-minkowski-difference/
    // https://digitalrune.github.io/DigitalRune-Documentation/html/138fc8fe-c536-40e0-af6b-0fb7e8eb9623.htm#Bisection
    // https://box2d.org/files/ErinCatto_UnderstandingConstraints_GDC2014.pdf

    // NOTE(hugo): position constraint solver (cf. REF 4)

    void update(){

        for(u32 iter = 0u; iter != 2u; ++iter){
            // NOTE(hugo): detect and solve constraints with static shapes
            for(auto& s : kinematic_data){
                float t = 1.f;
                vec2 normal = {0.f, 0.f};
                Shape_Type type = NONE;

                if(s.velocity.x != 0.f || s.velocity.y != 0.f){
                    // NOTE(hugo): margin velocity to avoid interpenetration
                    vec2 margin_velocity = epsilon * normalized(s.velocity);
                    Report report = move_against_static(s.position, s.velocity + margin_velocity, s.body);

                    if(report.t <= 1.f){
                        t = report.t;
                        normal = report.normal;
                        type = STATIC;
                    }
                }

                s.internal.constraint_group = UINT32_MAX;
                s.internal.speculative_velocity = t * s.velocity;
                s.internal.contact_normal = normal;
                s.internal.contact_type = type;
            }

// NOTE(hugo): without constraint groups ie cannot pile objects otherwise the response depends on the order
#if 0
            // NOTE(hugo): query constraints with kinematic shapes
            for(u32 iB = 1u; iB < kinematic_data.size; ++iB){
                Kinematic_Shape& kB = kinematic_data[iB];

                for(u32 iA = 0u; iA != iB; ++iA){
                    Kinematic_Shape& kA = kinematic_data[iA];

                    vec2 minkowski_shape = {
                        kA.body.half_width + kB.body.half_width,
                        kA.body.half_height + kB.body.half_height
                    };

                    vec2 relative_velocity = kB.internal.speculative_velocity - kA.internal.speculative_velocity;

                    // NOTE(hugo): margin velocity to avoid interpenetration
                    if(relative_velocity.x != 0.f || relative_velocity.y != 0.f) relative_velocity += epsilon * normalized(relative_velocity);

                    float t;
                    vec2 normal;
                    intersection_ray_AABB(kB.position, relative_velocity, kA.position, minkowski_shape, infinity, t, normal);

                    if(t <= 1.f){
                        // NOTE(hugo): movement clamping only when the collision occurs because the body moves into another
                        vec2 normal_kA = - normal;
                        if(dot(normal_kA, kA.internal.speculative_velocity) < 0.f){
                            kA.internal.speculative_velocity *= t;
                            kA.internal.contact_normal = normal_kA;
                            kA.internal.contact_type = KINEMATIC;
                        }

                        vec2 normal_kB = normal;
                        if(dot(normal_kB, kB.internal.speculative_velocity) < 0.f){
                            kB.internal.speculative_velocity *= t;
                            kB.internal.contact_normal = normal_kB;
                            kB.internal.contact_type = KINEMATIC;
                        }
                    }
                }
            }

// NOTE(hugo): with constraint groups ie can pile objects but unoptimized
#elif 1
            // NOTE(hugo): detect constraint groups using overlapping movement AABBs
            std::vector<std::vector<u32>> constraint_groups;
            for(u32 iB = 1u; iB < kinematic_data.size; ++iB){
                Kinematic_Shape& kB = kinematic_data[iB];

                vec2 min_kB;
                vec2 max_kB;
                moving_body_AABB(kB.position, kB.internal.speculative_velocity, kB.body, min_kB, max_kB);

                for(u32 iA = 0u; iA != iB; ++iA){
                    Kinematic_Shape& kA = kinematic_data[iA];

                    vec2 min_kA;
                    vec2 max_kA;
                    moving_body_AABB(kA.position, kA.internal.speculative_velocity, kA.body, min_kA, max_kA);

                    if(max_kB.x < min_kA.x || max_kA.x < min_kB.x || max_kB.y < min_kA.y || max_kA.y < min_kB.y) continue;

                    // NOTE(hugo): register constraint
                    // favor kB's group because it's probably bigger
                    if(kB.internal.constraint_group != UINT32_MAX){
                        if(kA.internal.constraint_group != UINT32_MAX){
                            for(auto& ik : constraint_groups[kA.internal.constraint_group]){
                                constraint_groups[kB.internal.constraint_group].push_back(ik);
                            }
                            constraint_groups[kA.internal.constraint_group].clear();
                            kA.internal.constraint_group = kB.internal.constraint_group;
                        }else{
                            constraint_groups[kB.internal.constraint_group].push_back(iA);
                        }
                    }else if(kA.internal.constraint_group != UINT32_MAX){
                        constraint_groups[kA.internal.constraint_group].push_back(iB);
                    }else{
                        u32 igroup = constraint_groups.size();
                        kA.internal.constraint_group = igroup;
                        kB.internal.constraint_group = igroup;

                        std::vector<u32> group = {iA, iB};
                        constraint_groups.emplace_back(group);
                    }
                }
            }

            // NOTE(hugo): solve each constraint group
            for(auto& group : constraint_groups){
                assert(group.size() == 0u || group.size() > 1u);

                for(u32 iter = 0u; iter != group.size(); ++iter){
                    for(u32 iB = 1u; iB < group.size(); ++iB){
                        Kinematic_Shape& kB = kinematic_data[group[iB]];

                        for(u32 iA = 0u; iA != iB; ++iA){
                            Kinematic_Shape& kA = kinematic_data[group[iA]];

                            vec2 minkowski_shape = {
                                kA.body.half_width + kB.body.half_width,
                                kA.body.half_height + kB.body.half_height
                            };

                            vec2 relative_velocity = kB.internal.speculative_velocity - kA.internal.speculative_velocity;

                            // NOTE(hugo): margin velocity to avoid interpenetration
                            if(relative_velocity.x != 0.f || relative_velocity.y != 0.f) relative_velocity += epsilon * normalized(relative_velocity);

                            float t;
                            vec2 normal;
                            intersection_ray_AABB(kB.position, relative_velocity, kA.position, minkowski_shape, infinity, t, normal);

                            if(t <= 1.f){
                                // NOTE(hugo): movement clamping only when the collision occurs because the body moves into another
                                vec2 normal_kA = - normal;
                                if(dot(normal_kA, kA.internal.speculative_velocity) < 0.f){
                                    kA.internal.speculative_velocity *= t;
                                    kA.internal.contact_normal = normal_kA;
                                    kA.internal.contact_type = KINEMATIC;
                                }

                                vec2 normal_kB = normal;
                                if(dot(normal_kB, kB.internal.speculative_velocity) < 0.f){
                                    kB.internal.speculative_velocity *= t;
                                    kB.internal.contact_normal = normal_kB;
                                    kB.internal.contact_type = KINEMATIC;
                                }
                            }
                        }
                    }
                }
            }
#elif 1
            // NOTE(hugo): detect constraint groups using overlapping movement AABBs
            std::vector<std::vector<u32>> constraint_groups;
            for(u32 iB = 1u; iB < kinematic_data.size; ++iB){
                Kinematic_Shape& kB = kinematic_data[iB];

                vec2 min_kB;
                vec2 max_kB;
                moving_body_AABB(kB.position, kB.internal.speculative_velocity, kB.body, min_kB, max_kB);

                for(u32 iA = 0u; iA != iB; ++iA){
                    Kinematic_Shape& kA = kinematic_data[iA];

                    vec2 min_kA;
                    vec2 max_kA;
                    moving_body_AABB(kA.position, kA.internal.speculative_velocity, kA.body, min_kA, max_kA);

                    if(max_kB.x < min_kA.x || max_kA.x < min_kB.x || max_kB.y < min_kA.y || max_kA.y < min_kB.y) continue;

                    // NOTE(hugo): register constraint
                    // favor kB's group because it's probably bigger
                    if(kB.internal.constraint_group != UINT32_MAX){
                        if(kA.internal.constraint_group != UINT32_MAX){
                            for(auto& ik : constraint_groups[kA.internal.constraint_group]){
                                constraint_groups[kB.internal.constraint_group].push_back(ik);
                            }
                            constraint_groups[kA.internal.constraint_group].clear();
                            kA.internal.constraint_group = kB.internal.constraint_group;
                        }else{
                            constraint_groups[kB.internal.constraint_group].push_back(iA);
                        }
                    }else if(kA.internal.constraint_group != UINT32_MAX){
                        constraint_groups[kA.internal.constraint_group].push_back(iB);
                    }else{
                        u32 igroup = constraint_groups.size();
                        kA.internal.constraint_group = igroup;
                        kB.internal.constraint_group = igroup;

                        std::vector<u32> group = {iA, iB};
                        constraint_groups.emplace_back(group);
                    }
                }
            }
#endif

            // NOTE(hugo): integrate positions and constraints
            for(auto& s : kinematic_data){
                if(s.velocity.x == 0.f && s.velocity.y == 0.f) continue;

                vec2 normalized_velocity = normalized(s.velocity);

                // NOTE(hugo): contact margin to avoid interpenetration
                if(s.internal.contact_type != NONE){
                    float t_margin = min(1.f, epsilon / dot(normalized_velocity, s.internal.contact_normal));
                    s.internal.speculative_velocity += t_margin * normalized_velocity;
                }

                s.position += s.internal.speculative_velocity;
                s.velocity -= s.internal.speculative_velocity;

                // NOTE(hugo): dot product is negative because contact_normal is towards s.position
                s.velocity -= s.internal.contact_normal * min(0.f, dot(s.velocity, s.internal.contact_normal));
            }
        }
    }

    // ----

    float epsilon;
    float infinity;

    indexmap<u32> static_map;
    array<Static_Shape> static_data;

    indexmap<u32> kinematic_map;
    array<Kinematic_Shape> kinematic_data;
};
