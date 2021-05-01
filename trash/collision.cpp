namespace cshape{
    vec2 Rect::center() const{
        return (min + max) * 0.5f;
    }
    vec2 Rect::size() const{
        return max - min;
    }

    Rect move(const Rect& rect, const vec2 movement){
        return {rect.min + movement, rect.max + movement};
    }

    Rect scale(const Rect& rect, const vec2 scale){
        vec2 rect_center = rect.center();
        vec2 rect_hs = rect.size() * 0.5f;
        return {rect_center - rect_hs * scale, rect_center + rect_hs * scale};
    }

    Rect extend(const Rect& rect, const vec2 extension){
        return {rect.min - extension, rect.max + extension};
    }
}

// ---- boolean collision

bool point_circle(const vec2& point, const cshape::Circle& circle){
    vec2 point_to_circle = circle.center - point;
    return sqlength(point_to_circle) < circle.radius * circle.radius;
}

bool point_rect(const vec2& point, const cshape::Rect& rect){
    return !((point.x < rect.min.x)
            | (point.x > rect.max.x)
            | (point.y < (rect.min.y))
            | (point.y > rect.max.y));
}

// REF(hugo): https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
bool point_capsule(const vec2& point, const cshape::Capsule& caps){
    float caps_begin_to_point[2] = {point.data[0] - caps.begin[0], point.data[1] - caps.begin[1]};
    float caps_dist[2] = {caps.end[0] - caps.begin[0], caps.end[1] - caps.begin[1]};

    float caps_sqlength = caps_dist[0] * caps_dist[0] + caps_dist[1] * caps_dist[1];
    float caps_param = (caps_begin_to_point[0] * caps_dist[0]
            + caps_begin_to_point[1] * caps_dist[1]) / caps_sqlength;
    caps_param = min_max(caps_param, 0.f, 1.f);

    float orthogonal[2] = {caps_begin_to_point[0] - caps_dist[0] * caps_param, caps_begin_to_point[1] - caps_dist[1] * caps_param};

    return (orthogonal[0] * orthogonal[0] + orthogonal[1] + orthogonal[1]) < (caps.radius * caps.radius);
}

bool point_ray(const vec2& point, const cshape::Ray& ray){
    return dot(ray.direction, point - ray.origin) > 0.f;
}

bool circle_circle(const cshape::Circle& circleA, const cshape::Circle& circleB){
    float center_distance[2] = {circleB.center[0] - circleA.center[0], circleB.center[1] - circleA.center[1]};
    float collision_sqdistance = circleB.radius + circleA.radius;
    return (center_distance[0] * center_distance[0] + center_distance[1] * center_distance[1])
        < (collision_sqdistance * collision_sqdistance);
}

bool circle_rect(const cshape::Circle& A, const cshape::Rect& B){
    float project_circle_center_on_rect[2] = {min_max(A.center[0], B.min[0], B.max[0]), min_max(A.center[1], B.min[1], B.max[1])};
    float center_circle_to_proj[2] = {
        project_circle_center_on_rect[0] - A.center[0],
        project_circle_center_on_rect[1] - A.center[1]
    };
    return (center_circle_to_proj[0] * center_circle_to_proj[0] + center_circle_to_proj[1] * center_circle_to_proj[1]) < (A.radius * A.radius);
}

bool circle_capsule(const cshape::Circle& circ, const cshape::Capsule& caps){
    float caps_begin_to_circle_center[2] = {circ.center[0] - caps.begin[0], circ.center[1] - caps.begin[1]};
    float caps_distance[2] = {caps.end[0] - caps.begin[0], caps.end[1] - caps.begin[1]};

    float caps_sqlength = caps_distance[0] * caps_distance[0] + caps_distance[1] * caps_distance[1];
    float caps_param =
        (caps_begin_to_circle_center[0] * caps_distance[0] + caps_begin_to_circle_center[1] * caps_distance[1])
        / caps_sqlength;
    caps_param = min_max(caps_param, 0.f, 1.f);

    float orthogonal[2] = {
        caps_begin_to_circle_center[0] - caps_distance[0] * caps_param,
        caps_begin_to_circle_center[1] - caps_distance[1] * caps_param
    };
    float collision_sqdistance = circ.radius + caps.radius;

    return (orthogonal[0] * orthogonal[0] + orthogonal[1] * orthogonal[1]) < (collision_sqdistance * collision_sqdistance);
}

bool rect_rect(const cshape::Rect& rectA, const cshape::Rect& rectB){
    bool cA = rectB.max.x < rectA.min.x;
    bool cB = rectA.max.x < rectB.min.x;
    bool cC = rectB.max.y < rectA.min.y;
    bool cD = rectA.max.y < rectB.min.y;
    return !(cA | cB | cC | cD);
    return !(
            (rectA.max.x < rectB.min.x)
            | (rectB.max.x < rectA.min.x)
            | (rectA.max.y < rectB.min.y)
            | (rectB.max.y < rectA.min.y)
            );
}

bool rect_ray_axisX(const cshape::Rect& rect, cshape::Ray& ray){
    assert(ray.inv_direction.x != 0.f);

    ray.length = min((rect.min.x - ray.origin.x) * ray.inv_direction.x,
            (rect.max.x - ray.origin.x) * ray.inv_direction.x);
    return (ray.length > 0.) && (ray.origin.y > rect.min.y && ray.origin.y < rect.max.y);
}

bool rect_ray_axisY(const cshape::Rect& rect, cshape::Ray& ray){
    assert(ray.inv_direction.x != 0.f);

    ray.length = min((rect.min.y - ray.origin.y) * ray.inv_direction.y,
            (rect.max.y - ray.origin.y) * ray.inv_direction.y);
    return (ray.length > 0.) && (ray.origin.x > rect.min.x && ray.origin.x < rect.max.x);
}

// REF(hugo): https://tavianator.com/fast-branchless-raybounding-box-intersections/
bool rect_ray_axisXY(const cshape::Rect& A, cshape::Ray& R){
    assert(R.inv_direction.x != 0.f && R.inv_direction.y != 0.f);

    float tmin, tmax;

    float tx1 = (A.min.x - R.origin.x) * R.inv_direction.x;
    float tx2 = (A.max.x - R.origin.x) * R.inv_direction.x;

    tmin = min(tx1, tx2);
    tmax = max(tx1, tx2);

    float ty1 = (A.min.y - R.origin.y) * R.inv_direction.y;
    float ty2 = (A.max.y - R.origin.y) * R.inv_direction.y;

    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));

    R.length = tmin;
    return (tmax >= tmin);
}

// ---- detailed collision

inline void circle_circle_manifold(const cshape::Circle& circleA, const cshape::Circle& circleB, cshape::Manifold& manifold){
    vec2 center_center = circleB.center - circleA.center;
    float radius = circleA.radius + circleB.radius;

    float sqdistance = sqlength(center_center);
    float sqradius = sqradius * sqradius;

    manifold.count = 0u;
    if(sqdistance < sqradius){
        float distance = sqrt(sqdistance);
        vec2 normal;
        if(distance != 0.f){
            normal = center_center / distance;
        }else{
            normal.x = 0.f;
            normal.x = 1.f;
        }

        manifold.count = 1u;
        manifold.depth[0u] = radius - distance;
        manifold.contact[0u] = circleB.center - normal * circleB.radius;
        manifold.normal[0u] = normal;
    }
}

// ---- GJK & predicates

vec2 furthest_point(const vec2& direction, const cshape::Polygon& poly){
    assert(poly.nvertices > 0u);

    vec2 min_support = poly.vertices[0u];
    float min_dot = dot(poly.vertices[0u], direction);

    for(u32 ivert = 1u; ivert != poly.nvertices; ++ivert){
        float vert_dot = dot(poly.vertices[ivert], direction);
        if(vert_dot > min_dot){
            min_support = poly.vertices[ivert];
            min_dot = vert_dot;
        }
    }

    return min_support;
}

template<typename TypeA, typename TypeB>
bool GJK(const TypeA& shapeA, const TypeB& shapeB){
    vec2 simplex[3u];
    u32 simplex_dimension;
    vec2 support_direction;

    auto get_support_vector = [&](){
        return furthest_point(support_direction, shapeA) - furthest_point(- support_direction, shapeB);
    };

    auto update_simplex = [&](){
        assert(simplex_dimension == 2u || simplex_dimension == 3u);

        if(simplex_dimension == 2u){
            // NOTE(hugo):
            // * new support      = A = simplex[1u]
            // * previous support = B = simplex[0u]
            //
            //    |       |
            //    |   1'  |
            // 3  B ----- A  2
            //    |   1   |
            //    |       |
            //
            // 1: closest simplex to origin is segment (B, A) ; dot(AO, AB) > 0.f
            // 2: closest simplex to origin is point A ; dot(AO, AB) <= 0.f
            // 3: impossible because A is in direction of the origin wrt. B (in Minkowski coord.) (cf. direction check in the GJK iteration)

            vec2 AB = simplex[0u] - simplex[1u];
            vec2 AO = - simplex[1u];

            float dot_AB_AO = dot(AB, AO);

            if(dot_AB_AO > 0.f){
                // NOTE(hugo): 2D version of cross(cross(AB, AO), AB) to find a perpendicular to AB in the direction of AO
                //support_dir   = { AB.x * AB.y * AO.y - AB.y * AB.y * AO.x, AB.x * AB.y * AO.x - AB.x * AB.x * AO.y };
                //              = { AB.y * (AB.x * AO.y - AB.y * AO.x), - AB.x * (AB.x * AO.y - AB.y * AO.x)}
                //              = { AB.y, - AB.x } * (AB.x * AO.y - AB.y * AO.x)
                //              = { AB.y, - AB.x } * cross(AB, AO)
                support_direction = cross(AB, AO) * vec2({- AB.y, AB.x});

            }else{
                simplex[0u] = simplex[1u];
                simplex_dimension = 1u;
                support_direction = AO;
            }

        }else{
            // NOTE(hugo):
            // * new support              = A = simplex[2u]
            // * previous support         = B = simplex[1u]
            // * next_to_previous support = C = simplex[0u]
            //
            //     x   /
            //        /
            //  ---- C
            //       |\
            //       | \   5
            //       |  \
            //    x  |   \
            //       |    \   /
            //       |  6  \ /
            //  ---- B ---- A   4
            //       |      |\  2
            //    x  |   1  | \
            //       |      |3 \
            //
            // x: impossible because the C and B are not the closest simplex to the origin and the origin is on A's side of (C, B) (in Minkowski coord.)
            // 1: closest simplex to origin is segment (B, A)  ; dot(AB_ortho, AO) > 0. && dot(AB, AO) > 0.
            // 2: closest simplex to origin is segment(C, A)   ; dot(AB_ortho, AO) > 0. && dot(AB, AO) <= 0. && dot(AC, AO) > 0.
            // 3: closest simplex to origin is point A         ; dot(AB_ortho, AO) > 0. && dot(AB, AO) <= 0. && dot(AC, AO) <= 0.
            // 4: closest simplex to origin is point A         ; dot(AC_ortho, AO) > 0. && dot(AC, AO) <= 0.
            // 5: closest simplex to origin is segment(C, A)   ; dot(AC_ortho, AO) > 0. && dot(AC, AO) > 0.
            // 6: origin inside simplex                        ; dot(AB_ortho, AO) <= 0. && dot(AC_ortho, AO) <= 0.


            vec2 AC = simplex[0u] - simplex[2u];
            vec2 AB = simplex[1u] - simplex[2u];
            vec2 AO = - simplex[2u];

            float winding = point_side_segment(simplex[2u], simplex[1u], simplex[0u]);
            vec2 AC_ortho = winding * vec2({AC.y, - AC.x});
            vec2 AB_ortho = winding * vec2({- AB.y, AB.x});

            if(dot(AB_ortho, AO) > 0.f){
                if(dot(AB, AO) > 0.f){
                    simplex[1u] = simplex[2u];
                    simplex_dimension = 2u;
                    support_direction = AB_ortho;

                }else if(dot(AC, AO) > 0.f){
                    simplex[0u] = simplex[1u];
                    simplex[1u] = simplex[2u];
                    simplex_dimension = 2u;
                    support_direction = AC_ortho;

                }else{
                    simplex[0u] = simplex[2u];
                    simplex_dimension = 1u;
                    support_direction = AO;
                }

            }else if(dot(AC_ortho, AO) > 0.f){
                if(dot(AC, AO) > 0.f){
                    simplex[0u] = simplex[1u];
                    simplex[1u] = simplex[2u];
                    simplex_dimension = 2u;
                    support_direction = AC_ortho;
                }else{
                    simplex[0u] = simplex[2u];
                    simplex_dimension = 1u;
                    support_direction = AO;
                }
            }else{
                return true;
            }
        }

        return false;
    };

    simplex[0u] = get_support_vector();
    simplex_dimension = 1u;
    support_direction = {-1.f, 0.f};

    while(true){
        vec2 support_vector = get_support_vector();
        if(dot(support_vector, support_direction) <= 0.f) return false;
        simplex[simplex_dimension++] = support_vector;
        if(update_simplex()) return true;
    }
}
