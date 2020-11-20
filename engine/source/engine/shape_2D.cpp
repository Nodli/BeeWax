namespace c2D{
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
