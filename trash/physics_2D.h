#ifndef H_PHYSICS_2D
#define H_PHYSICS_2D

// REF(hugo):
//  Normal game physics:
//  * Detect collisions
//  * Compute forces
//  * Apply forces (this step updates the bodies' velocities)
//  * Solve constraints (contacts and joints; this step corrects velocities so that constraints are all satisfied)
//  * Update positions (using the corrected velocities)
//  Continuous game physics (using motion clamping):
//  * Detect collisions
//  * Compute forces
//  * Apply forces
//  * Solve constraints
//  * Update !target! positions
//  * Use CCD to compute the first time of impact of each body
//  * Move bodies to their first time of impact position
//  Motion clamping has issues when objects have multiple impacts ie multiple time clamping across multiple frames

struct Collider{
    enum Type{
        STATIC,
        KINEMATIC,
    };

    union Data{
        u64 as_u64;
        void* as_ptr;
    };

    union Shape{
        vec2 hsize;
    };

    // ----

    Type type;
    Data data;
    void (*callback)(void* context_data, Data collider_data, Data other_collider_data);

    vec2 position;
    Shape shape;

    // ----

    indexmap_handle handle;

    vec2 movement;
    u32 npenetrations;
    float integrator;
    s32 island;
};

struct Collider_Handle : indexmap_handle {};

struct Collision_Context{
    struct Collision{
        u32 colliderA;
        u32 colliderB;
        vec2 normal;
    };

    void create(){
        collider_map.create();
        colliders.create();
    }
    void destroy(){
        colliders.destroy();
        collider_map.destroy();
    }

    // --

    Collider_Handle create_collider(const Collider& collider){
        assert(collider.shape.hsize.x > 0.f && collider.shape.hsize.y > 0.f);

        u32 index = colliders.size;

        indexmap_handle handle = collider_map.borrow_handle();
        *collider_map.search(handle) = index;

        Collider& pushed = colliders.push(collider);
        pushed.handle = handle;

        pushed.movement = {0.f, 0.f};
        pushed.npenetrations = 0u;
        pushed.integrator = 1.f;

        return (Collider_Handle){handle};
    }

    void destroy_collider(const Collider_Handle handle){
        u32* index = collider_map.search(handle);
        if(index){
            colliders[*index] = colliders[colliders.size - 1u];
            *collider_map.search(colliders[colliders.size - 1u].handle) = *index;
            colliders.pop();
        }
    }

    void move(const Collider_Handle handle, const vec2& velocity){
        u32* index = collider_map.search(handle);
        if(index){
            Collider& c = colliders[*index];
            c.movement += movement;
        }
    }

    vec2 get_position(const Collider_Handle handle){
        u32* index = collider_map.search(handle);
        assert(index);
        if(index){
            Collider& c = colliders[*index];
            return c.position;
        }
        return vec2({0.f, 0.f});
    }

    void update(){
        auto movable = [](Collider::Type type){
            return type == Collider::KINEMATIC;
        };

        auto pushable = [](Collider::Type type){
            return type == Collider::DYNAMIC;
        };

        array<Penetration> penetrations;
        penetrations.create();

        array<Contact> contacts;
        contacts.create();

        auto process_pair = [&](Collider& cA, Collider& cB){
            if(cA.movement.x == 0.f && cA.movement.y == 0.f) continue;

            vec2 shape_sum = cA.shape.hsize + cB.shape.hsize;
            vec2 position_rB = cB.position - cA.position;

            vec2 penetration = {shape_sum.x - abs(position_rB.x), shape_sum.y - abs(position_rB.y)};

            // NOTE(hugo): compute time of impact for non-penetrating bodies
            if(penetration.x < 0.f || penetration.y < 0.f){

                // NOTE(hugo): cB.movement is not used otherwise there is a dependency problem when piling objects
                // which means that the other colliders are considered static
                vec2 movement_rB = - cA.movement;

                float t = 1e20f;
                vec2 a = {0.f, 0.f};

                if(movement_rB.x > 0.f){
                    float tleft = (- shape_sum.x - position_rB.x) / movement_rB.x;
                    float hity = position_rB.y + tleft * movement_rB.y;
                    if(tleft >= 0.f && !(hity < - shape_sum.y || hity > shape_sum.y) && tleft < t){
                        t = tleft;
                        a = {-1.f, 0.f};
                    }
                }else if(movement_rB.x < 0.f){
                    float tright = (shape_sum.x - position_rB.x) / movement_rB.x;
                    float hity = position_rB.y + tright * movement_rB.y;
                    if(tright >= 0.f && !(hity < - shape_sum.y || hity > shape_sum.y) && tright < t){
                        t = tright;
                        a = {1.f, 0.f};
                    }
                }

                if(movement_rB.y > 0.f){
                    float tbot = (- shape_sum.y - position_rB.y) / movement_rB.y;
                    float hitx = position_rB.x + tbot * movement_rB.x;
                    if(tbot >= 0.f && !(hitx < - shape_sum.x || hitx > shape_sum.x) && tbot < t){
                        t = tbot;
                        a = {0.f, -1.f};
                    }
                }else if(movement_rB.y < 0.f){
                    float ttop = (shape_sum.y - position_rB.y) / movement_rB.y;
                    float hitx = position_rB.x + ttop * movement_rB.x;
                    if(ttop >= 0.f && !(hitx < - shape_sum.x || hitx > shape_sum.x) && ttop < t){
                        t = ttop;
                        a = {0.f, 1.f};
                    }
                }

                if(t > 1.f) continue;

                Contact contact;
                contact.colliderA = iA;
                contact.colliderB = iB;
                contact.normal = a;
                contacts.push(contact);

                cA.integrator = min(cA.integrator, t);

            // NOTE(hugo): register penetration
            }else{
                Penetration pen;
                pen.colliderA = iA;
                pen.colliderB = iB;

                if(penetration.x < penetration.y){
                    pen.normal = {sign(position_rB.x), 0.f};
                    pen.depth = penetration.x + epsilon;
                }else{
                    pen.normal = {0.f, sign(position_rB.y)};
                    pen.depth = penetration.y + epsilon;
                }

                penetrations.push(pen);
            }
        };

        for(u32 iA = 0u; iA != colliders.size; ++iA){
            for(u32 iB = 0u; iB != iA; ++iB){
                process_pair(colliders[iA], colliders[iB]);
            }
            for(u32 iB = iA + 1u; iB != colliders.size; ++iB){
                process_pair(colliders[iA], colliders[iB]);
            }

            // NOTE(hugo): filter out non-occuring contacts and apply contact constraints
            // ex: with contact(A, B, 0.1) and contact(A, C, 0.5) ; the second contact never occurs
            u32 iter_contacts = 0u;
            while(iter_contacts != contacts.size){
                Contact& contact = contacts[iter_contacts];

                Collider& cA = colliders[contact.colliderA];
                Collider& cB = colliders[contact.colliderB];

                if(cA.integrator != cB.integrator){
                    contacts.remove_swap(iter_contacts);
                }else{
                    u32 npushable = pushable(cA.type) + pushable(cB.type);
                    if(pushable(cA.type))   cA.movement -= pen.normal * epsilon / (float)npushable;
                    if(pushable(cB.type))   cB.movement += pen.normal * epsilon / (float)npushable;
                    ++iter_contacts;
                }
            }
        }

        // NOTE(hugo): apply penetrations constraints
        for(auto& pen : penetrations){
            Collider& cA = colliders[pen.colliderA];
            Collider& cB = colliders[pen.colliderB];

            u32 npushable = pushable(cA.type) + pushable(cB.type);
            if(pushable(cA.type)){
                vec2 movement = - pen.normal * pen.depth / (float)npushable;
                if(cA.npenetrations++)  cA.movement += movement;
                else                    cA.movement = movement;
            }
            if(pushable(cB.type)){
                vec2 movement = pen.normal * pen.depth / (float)npushable;
                if(cB.npenetrations++)  cB.movement += movement;
                else                    cB.movement = movement;
            }
        }

        // NOTE(hugo): integrate
        for(auto& collider : colliders){
            if(movable(collider.type)){
                vec2 dpos = collider.movement * max(0.f, collider.integrator - epsilon);

                collider.position += dpos;
                collider.movement -= dpos;
            }

            collider.integrator = 1.f;
            collider.npenetrations = 0u;
        }

        // NOTE(hugo): movement transmission

        penetrations.destroy();
        contacts.destroy();

#if 0
        array<Penetration> penetrations;
        penetrations.create();

        // NOTE(hugo): detect penetrations
        for(u32 iA = 0u; iA != colliders.size - 1u; ++iB){
            for(u32 iB = 0u; iB != iA; ++iA){
                Collider& cA = colliders[iA];
                Collider& cB = colliders[iB];

                vec2 shape_sum = cA.shape.hsize + cB.shape.hsize;
                vec2 position_rB = cB.position - cA.position;

                vec2 penetration = {shape_sum.x - abs(position_rB.x), shape_sum.y - abs(position_rB.y)};

                if(penetration.x < 0.f || penetration.y < 0.f) continue;

                Penetration pen;
                pen.colliderA = iA;
                pen.colliderB = iB;

                if(penetration.x < penetration.y){
                    pen.normal = {sign(position_rB.x), 0.f};
                    pen.depth = penetration.x + epsilon;
                }else{
                    pen.normal = {0.f, sign(position_rB.y)};
                    pen.depth = penetration.y + epsilon;
                }

                penetrations.push(pen);
            }
        }

        // NOTE(hugo): override movement to remove penetrations
        for(auto& pen : penetrations){
            Collider& cA = colliders[pen.colliderA];
            Collider& cB = colliders[pen.colliderB];

            u32 npushable = pushable(cA.type) + pushable(cB.type);
            if(pushable(cA.type)){
                vec2 movement = - pen.normal * pen.depth / (float)npushable;
                if(cA.npenetrations++)  cA.movement += movement;
                else                    cA.movement = movement;
            }
            if(pushable(cB.type)){
                vec2 movement = pen.normal * pen.depth / (float)npushable;
                if(cB.npenetrations++)  cB.movement += movement;
                else                    cB.movement = movement;
            }
        }

        array<Contact> contacts;
        contacts.create();

        u32 ipen = 0u;
        Penetration pen;
        pen.colliderA = colliders.size;
        pen.colliderB = colliders.size;
        penetrations.push(pen);

        for(u32 iB = 1u; iB < colliders.size; ++iB){
            for(u32 iA = 0u; iA != iB; ++iA){

        //for(u32 iB = 0u; iB != colliders.size; ++iB){
        //    for(u32 iA = 0u; iA != colliders.size; ++iA){
                if(iA == iB) continue;
                if(penetrations[ipen].colliderB == iB && penetrations[ipen].colliderA == iA){
                    ++ipen;
                    continue;
                }

                Collider& cA = colliders[iA];
                Collider& cB = colliders[iB];

                vec2 shape_sum = cA.shape.hsize + cB.shape.hsize;
                vec2 position_rB = cB.position - cA.position;

                // NOTE(hugo): cB.movement is not used otherwise there is a dependency problem when piling objects
                vec2 movement_rB = cB.movement - cA.movement;

                if(movement_rB.x == 0.f && movement_rB.y == 0.f) continue;

                float t = 1e20f;
                vec2 a = {0.f, 0.f};

                if(movement_rB.x > 0.f){
                    float tleft = (- shape_sum.x - position_rB.x) / movement_rB.x;
                    float hity = position_rB.y + tleft * movement_rB.y;
                    if(tleft >= 0.f && !(hity < - shape_sum.y || hity > shape_sum.y) && tleft < t){
                        t = tleft;
                        a = {-1.f, 0.f};
                    }
                }else if(movement_rB.x < 0.f){
                    float tright = (shape_sum.x - position_rB.x) / movement_rB.x;
                    float hity = position_rB.y + tright * movement_rB.y;
                    if(tright >= 0.f && !(hity < - shape_sum.y || hity > shape_sum.y) && tright < t){
                        t = tright;
                        a = {1.f, 0.f};
                    }
                }

                if(movement_rB.y > 0.f){
                    float tbot = (- shape_sum.y - position_rB.y) / movement_rB.y;
                    float hitx = position_rB.x + tbot * movement_rB.x;
                    if(tbot >= 0.f && !(hitx < - shape_sum.x || hitx > shape_sum.x) && tbot < t){
                        t = tbot;
                        a = {0.f, -1.f};
                    }
                }else if(movement_rB.y < 0.f){
                    float ttop = (shape_sum.y - position_rB.y) / movement_rB.y;
                    float hitx = position_rB.x + ttop * movement_rB.x;
                    if(ttop >= 0.f && !(hitx < - shape_sum.x || hitx > shape_sum.x) && ttop < t){
                        t = ttop;
                        a = {0.f, 1.f};
                    }
                }

                if(t > 1.f) continue;

                Contact contact;
                contact.colliderA = iA;
                contact.colliderB = iB;
                contact.normal = a;

                contacts.push(contact);

                cA.integrator = min(cA.integrator, t);
                //cB.integrator = min(cB.integrator, t);
            }
        }

        penetrations.pop();

#if 0
        if(contacts.size) LOG_TRACE("-- impacts");
        for(auto& contact : contacts){
            LOG_TRACE("iA:%d iB:%d n(%f, %f) d:%f", contact.colliderA, contact.colliderB, contact.normal.x, contact.normal.y);
            Collider& cA = colliders[contact.colliderA];
            Collider& cB = colliders[contact.colliderB];
            LOG_TRACE("p: %f %f t: %f", cA.position.x, cA.position.y);
            LOG_TRACE("p: %f %f t: %f", cB.position.x, cB.position.y);
        }
#endif

        // NOTE(hugo): filter out non-occuring contacts and push away occuring contacts
        // ex: with contact(A, B, 0.1) and contact(A, C, 0.5) ; the second contact never occurs
        u32 iter_contacts = 0u;
        while(iter_contacts != contacts.size){
            Contact& contact = contacts[iter_contacts];

            Collider& cA = colliders[contact.colliderA];
            Collider& cB = colliders[contact.colliderB];

            if(cA.integrator != cB.integrator)  contacts.remove_swap(iter_contacts);
            else                                ++iter_contacts;
        }

#if 0
        // NOTE(hugo): skin
        for(auto& contact : contacts){
            Collider& cA = colliders[contact.colliderA];
            Collider& cB = colliders[contact.colliderB];

            if(pushable(cA.type) && !cA.npenetrations) cA.movement -= contact.normal * epsilon;
            if(pushable(cB.type) && !cB.npenetrations) cB.movement += contact.normal * epsilon;
        }
#endif


        // NOTE(hugo): move
        for(auto& collider : colliders){
            if(movable(collider.type)){
                vec2 dpos = collider.movement * max(0.f, collider.integrator - epsilon);

                collider.position += dpos;
                collider.movement -= dpos;
            }

            collider.integrator = 1.f;
            collider.npenetrations = 0u;
        }

        for(auto& contact : contacts){
            Collider& cA = colliders[contact.colliderA];
            Collider& cB = colliders[contact.colliderB];

            vec2 movA = cA.movement;
            vec2 movB = cB.movement;

            // NOTE(hugo): transmit movement to avoid impacting next iteration
            float nA = dot(contact.normal, cA.movement);
            float nB = dot(contact.normal, cB.movement);

            if(movable(cA.type) && pushable(cB.type) && !cB.npenetrations && max(0.f, nA) > max(0.f, nB)){
                movB += (max(0.f, nA) - max(0.f, nB)) * contact.normal;
            }
            if(movable(cB.type) && pushable(cA.type) && !cA.npenetrations && max(0.f, - nB) > max(0.f, - nA)){
                movA -= (max(0.f, - nB) - max(0.f, - nA)) * contact.normal;
            }

            // NOTE(hugo): modify movement to avoid 'slide' tangentially to the contact next iteration
            if(pushable(cA.type) && !pushable(cB.type)) movA -= max(0.f,   dot(contact.normal, movA)) * contact.normal;
            if(pushable(cB.type) && !pushable(cA.type)) movB += max(0.f, - dot(contact.normal, movB)) * contact.normal;

            cA.movement = movA;
            cB.movement = movB;
        }


        // TODO(hugo): callbacks

        if(penetrations.size) LOG_TRACE("-- penetrations");
        for(auto& contact : penetrations){
            LOG_TRACE("iA:%d iB:%d n(%f, %f) d:%f", contact.colliderA, contact.colliderB, contact.normal.x, contact.normal.y, contact.depth);
            Collider& cA = colliders[contact.colliderA];
            Collider& cB = colliders[contact.colliderB];
            LOG_TRACE("p: %f %f m: %f %f t: %f", cA.position.x, cA.position.y, cA.movement.x, cA.movement.y, cA.integrator);
            LOG_TRACE("p: %f %f m: %f %f t: %f", cB.position.x, cB.position.y, cB.movement.x, cB.movement.y, cB.integrator);
        }

        if(contacts.size) LOG_TRACE("-- impacts");
        for(auto& contact : contacts){
            LOG_TRACE("iA:%d iB:%d n(%f, %f) d:%f", contact.colliderA, contact.colliderB, contact.normal.x, contact.normal.y);
            Collider& cA = colliders[contact.colliderA];
            Collider& cB = colliders[contact.colliderB];
            LOG_TRACE("p: %f %f m: %f %f t: %f", cA.position.x, cA.position.y, cA.movement.x, cA.movement.y, cA.integrator);
            LOG_TRACE("p: %f %f m: %f %f t: %f", cB.position.x, cB.position.y, cB.movement.x, cB.movement.y, cB.integrator);
        }

#if 0
        LOG_TRACE("-- colliders");
        for(auto& collider : colliders){
            LOG_TRACE("p: %f %f m: %f %f pen: %d t: %f", collider.position.x, collider.position.y, collider.movement.x, collider.movement.y, collider.npenetrations, collider.integrator);

        }
#endif

            penetrations.clear();
            contacts.clear();
        }

        penetrations.destroy();
        contacts.destroy();
#endif

#if 0
        // NOTE(hugo): detect contacts
        for(u32 iB = 1u; iB < colliders.size; ++iB){
            for(u32 iA = 0u; iA != iB; ++iA){
                Collider& cA = colliders[iA];
                Collider& cB = colliders[iB];

                vec2 shape_sum = cA.shape.hsize + cB.shape.hsize;
                vec2 position_rB = cB.position - cA.position;

                vec2 penetration = {shape_sum.x - abs(position_rB.x), shape_sum.y - abs(position_rB.y)};

                // NOTE(hugo): no contact or penetration ; evaluate potential time of impact using minkowski & ray-box intersection
                if(penetration.x < 0.f || penetration.y < 0.f){
                    vec2 movement_rB = cB.movement - cA.movement;


                    // NOTE(hugo): already in contact or penetrating
                }else{
                    Contact contact;
                    contact.colliderA = iA;
                    contact.colliderB = iB;

                    if(penetration.x < penetration.y){
                        contact.normal = {sign(position_rB.x), 0.f};
                        contact.depth = penetration.x + epsilon;
                    }else{
                        contact.normal = {0.f, sign(position_rB.y)};
                        contact.depth = penetration.y + epsilon;
                    }

                    penetrations.push(contact);

                    cA.toi = 0.f;
                    cB.toi = 0.f;
                }
            }




            // NOTE(hugo): solve penetrations
            for(auto& contact : penetrations){
                Collider& cA = colliders[contact.colliderA];
                Collider& cB = colliders[contact.colliderB];

                if(cA.callback) cA.callback(nullptr, cA.data, cB.data);
                if(cB.callback) cB.callback(nullptr, cB.data, cA.data);

                u32 npushable = pushable(cA.type) + pushable(cB.type);
                if(pushable(cA.type)) cA.position -= contact.normal * contact.depth / (float)npushable;
                if(pushable(cB.type)) cB.position += contact.normal * contact.depth / (float)npushable;
            }


            // NOTE(hugo): solve impacts
            for(auto& contact : impacts){
                Collider& cA = colliders[contact.colliderA];
                Collider& cB = colliders[contact.colliderB];

                if(cA.callback) cA.callback(nullptr, cA.data, cB.data);
                if(cB.callback) cB.callback(nullptr, cB.data, cA.data);

                // NOTE(hugo): skin push away
                if(pushable(cA.type)) cA.position -= contact.normal * contact.depth;
                if(pushable(cB.type)) cB.position += contact.normal * contact.depth;

                vec2 movA = cA.movement;
                vec2 movB = cB.movement;

                // NOTE(hugo): transmit movement to avoid impacting next iteration
                if(pushable(cA.type) && movable(cB.type)) movA -= max(0.f, - dot(contact.normal, cB.movement)) * contact.normal;
                if(pushable(cB.type) && movable(cA.type)) movB += max(0.f,   dot(contact.normal, cA.movement)) * contact.normal;

                // NOTE(hugo): modify movement to avoid impacting next iteration
                if(pushable(cA.type) && !pushable(cB.type)) movA -= max(0.f,   dot(contact.normal, movA)) * contact.normal;
                if(pushable(cB.type) && !pushable(cA.type)) movB += max(0.f, - dot(contact.normal, movB)) * contact.normal;

                cA.movement = movA;
                cB.movement = movB;
            }

            ++i;

            impacts.size = 0u;
            penetrations.size = 0u;
        }

        for(auto& collider : colliders){
            collider.movement = vec2({0.f, 0.f});

        }

        i = (i + 10u) / 10u * 10u;

        impacts.destroy();
        penetrations.destroy();
#endif
    }

    // ----

    indexmap<u32> collider_map;
    array<Collider> colliders;
    float epsilon;
};

#ifdef H_IMDRAWER

void debug_draw_collision_context(const Collision_Context& context, ImDrawer& drawer, float depth, u32 rgba){
    for(auto& collider : context.colliders){
        vec2 vertices[4u];
        vertices[0u] = collider.position - collider.shape.hsize;
        vertices[1u] = collider.position + vec2({  collider.shape.hsize.x, - collider.shape.hsize.y});
        vertices[2u] = collider.position + vec2({  collider.shape.hsize.x,   collider.shape.hsize.y});
        vertices[3u] = collider.position + vec2({- collider.shape.hsize.x,   collider.shape.hsize.y});

        drawer.command_line(vertices[0u], vertices[1u], depth, rgba);
        drawer.command_line(vertices[1u], vertices[2u], depth, rgba);
        drawer.command_line(vertices[2u], vertices[3u], depth, rgba);
        drawer.command_line(vertices[3u], vertices[0u], depth, rgba);
    }
}

#endif

#endif
