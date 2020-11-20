namespace c2D{
    // ---- boolean collision

    bool point_circle(const vec2& point, const Circle& circle){
        vec2 point_to_circle = circle.center - point;
        return sqlength(point_to_circle) < circle.radius * circle.radius;
    }

    bool point_rect(const vec2& point, const Rect& rect){
        return !((point.x < rect.min.x)
                | (point.x > rect.max.x)
                | (point.y < (rect.min.y))
                | (point.y > rect.max.y));
    }

    // REF(hugo): https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
    bool point_capsule(const vec2& point, const Capsule& caps){
        float caps_begin_to_point[2] = {point.data[0] - caps.begin[0], point.data[1] - caps.begin[1]};
        float caps_dist[2] = {caps.end[0] - caps.begin[0], caps.end[1] - caps.begin[1]};

        float caps_sqlength = caps_dist[0] * caps_dist[0] + caps_dist[1] * caps_dist[1];
        float caps_param = (caps_begin_to_point[0] * caps_dist[0]
                + caps_begin_to_point[1] * caps_dist[1]) / caps_sqlength;
        caps_param = clamp(caps_param, 0.f, 1.f);

        float orthogonal[2] = {caps_begin_to_point[0] - caps_dist[0] * caps_param, caps_begin_to_point[1] - caps_dist[1] * caps_param};

        return (orthogonal[0] * orthogonal[0] + orthogonal[1] + orthogonal[1]) < (caps.radius * caps.radius);
    }

    bool point_ray(const vec2& point, const Ray& ray){
        return dot(ray.direction, point - ray.origin) > 0.f;
    }

    bool circle_circle(const Circle& circleA, const Circle& circleB){
        float center_distance[2] = {circleB.center[0] - circleA.center[0], circleB.center[1] - circleA.center[1]};
        float collision_sqdistance = circleB.radius + circleA.radius;
        return (center_distance[0] * center_distance[0] + center_distance[1] * center_distance[1])
            < (collision_sqdistance * collision_sqdistance);
    }

    bool circle_rect(const Circle& A, const Rect& B){
        float project_circle_center_on_rect[2] = {clamp(A.center[0], B.min[0], B.max[0]), clamp(A.center[1], B.min[1], B.max[1])};
        float center_circle_to_proj[2] = {
            project_circle_center_on_rect[0] - A.center[0],
            project_circle_center_on_rect[1] - A.center[1]
        };
        return (center_circle_to_proj[0] * center_circle_to_proj[0] + center_circle_to_proj[1] * center_circle_to_proj[1]) < (A.radius * A.radius);
    }

    bool circle_capsule(const Circle& circ, const Capsule& caps){
        float caps_begin_to_circle_center[2] = {circ.center[0] - caps.begin[0], circ.center[1] - caps.begin[1]};
        float caps_distance[2] = {caps.end[0] - caps.begin[0], caps.end[1] - caps.begin[1]};

        float caps_sqlength = caps_distance[0] * caps_distance[0] + caps_distance[1] * caps_distance[1];
        float caps_param =
            (caps_begin_to_circle_center[0] * caps_distance[0] + caps_begin_to_circle_center[1] * caps_distance[1])
            / caps_sqlength;
        caps_param = clamp(caps_param, 0.f, 1.f);

        float orthogonal[2] = {
            caps_begin_to_circle_center[0] - caps_distance[0] * caps_param,
            caps_begin_to_circle_center[1] - caps_distance[1] * caps_param
        };
        float collision_sqdistance = circ.radius + caps.radius;

        return (orthogonal[0] * orthogonal[0] + orthogonal[1] * orthogonal[1]) < (collision_sqdistance * collision_sqdistance);
    }

    bool rect_rect(const Rect& rectA, const Rect& rectB){
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

    bool rect_ray_axisX(const Rect& rect, Ray& ray){
        assert(ray.inv_direction.x != 0.f);

        ray.length = min((rect.min.x - ray.origin.x) * ray.inv_direction.x,
                (rect.max.x - ray.origin.x) * ray.inv_direction.x);
        return (ray.length > 0.) && (ray.origin.y > rect.min.y && ray.origin.y < rect.max.y);
    }
    bool rect_ray_axisY(const Rect& rect, Ray& ray){
        assert(ray.inv_direction.x != 0.f);

        ray.length = min((rect.min.y - ray.origin.y) * ray.inv_direction.y,
                (rect.max.y - ray.origin.y) * ray.inv_direction.y);
        return (ray.length > 0.) && (ray.origin.x > rect.min.x && ray.origin.x < rect.max.x);
    }

    // REF(hugo): https://tavianator.com/fast-branchless-raybounding-box-intersections/
    bool rect_ray_axisXY(const Rect& A, Ray& R){
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

    inline void circle_circle_manifold(const Circle& circleA, const Circle& circleB, Manifold& manifold){
        vec2 center_center = circleB.center - circleA.center;
        float radius = circleA.radius + circleB.radius;

        float sqdistance = sqlength(center_center);
        float sqradius = sqradius * sqradius;

        manifold.count = 0u;
        if(sqdistance < sqradius){
            float distance = std::sqrt(sqdistance);
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
}
