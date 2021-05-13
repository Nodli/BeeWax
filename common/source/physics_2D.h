#ifndef H_PHYSICS_2D
#define H_PHYSICS_2D

#include <vector>

struct Collider{
    enum Type{
        STATIC,
        DYNAMIC,
        KINEMATIC
    };

    vec2 position;
    u32 context_index;
    Type type;
    void* collider_data;
    void (*collision_callback)(void* context_data, void* collider_data, void* other_collider_data);

    struct{
        vec2 hsize;
    } shape;
};

struct Collision_Context{
    void register_collider(Collider& collider){
        collider.context_index = colliders.size();
        colliders.push_back(&collider);
    }
    void unregister_collider(Collider& collider){
        u32 index = collider.context_index;

        colliders[index] = colliders[colliders.size() - 1u];
        (*colliders[index]).context_index = index;

        colliders.pop_back();
    }

    struct Collision_Pair{
        u32 indexA;
        u32 indexB;
        vec2 pendir; // NOTE(hugo): from the point of view of A
        float pendepth;
    };

    void update(){
        std::vector<Collision_Pair> pairs;

        LOG_TRACE("%d", colliders.size());

        // NOTE(hugo): detect collisions
        for(u32 iB = 1u; iB < colliders.size(); ++iB){
            for(u32 iA = 0u; iA != iB; ++iA){
                LOG_TRACE("%d %d", iA, iB);
                Collider& cA = (*colliders[iA]);
                Collider& cB = (*colliders[iB]);

                if(cA.type != Collider::DYNAMIC && !cA.collision_callback && cB.type != Collider::DYNAMIC && !cB.collision_callback) continue;

                vec2 dpos = cB.position - cA.position;
                vec2 pen = {cA.shape.hsize.x + cB.shape.hsize.x - abs(dpos.x), cA.shape.hsize.y + cB.shape.hsize.y - abs(dpos.y)};

                LOG_TRACE("A: %f %f B: %f %f", cA.position.x, cA.position.y, cB.position.x, cB.position.y);
                LOG_TRACE("dpos: %f %f", dpos.x, dpos.y);
                LOG_TRACE("pen:  %f %f", pen.x, pen.y);

                if(pen.x > 0.f && pen.y > 0.f){
                    Collision_Pair pair;

                    pair.indexA = iA;
                    pair.indexB = iB;

                    if(pen.x < pen.y){
                        pair.pendepth = pen.x;
                        pair.pendir = {sign(dpos.x) * 1.f, 0.f};

                    }else{
                        pair.pendepth = pen.y;
                        pair.pendir = {0.f, sign(dpos.y) * 1.f};

                    }

                    LOG_TRACE("pendepth:  %f", pair.pendepth);
                    LOG_TRACE("pendir:  %f %f", pair.pendir.x, pair.pendir.y);

                    pairs.push_back(pair);
                }
            }
        }

        // NOTE(hugo): resolve collisions
        for(auto& pair : pairs){
            Collider& cA = (*colliders[pair.indexA]);
            Collider& cB = (*colliders[pair.indexB]);

            u32 impulse_sum = (u32)(cA.type == Collider::DYNAMIC) + (u32)(cB.type == Collider::DYNAMIC);
            LOG_TRACE("impulse_sum:  %d", impulse_sum);
            if(impulse_sum){
                vec2 impulse = (1.f / (float)impulse_sum) * pair.pendepth * pair.pendir;

                cA.position = cA.position - (float)(cA.type == Collider::DYNAMIC) * impulse;
                cB.position = cB.position + (float)(cB.type == Collider::DYNAMIC) * impulse;
                LOG_TRACE("A: %f %f B: %f %f", cA.position.x, cA.position.y, cB.position.x, cB.position.y);
            }

            if(cA.collision_callback) cA.collision_callback(context_data, cA.collider_data, cB.collider_data);
            if(cB.collision_callback) cB.collision_callback(context_data, cB.collider_data, cA.collider_data);
        }
    }

    // ----

    void* context_data;
    std::vector<Collider*> colliders;
};

#ifdef H_IMDRAWER

void debug_draw_collision_context(const Collision_Context& context, ImDrawer& drawer, float depth, u32 rgba){
    for(auto& iter_collider : context.colliders){
        Collider& collider = *iter_collider;


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
