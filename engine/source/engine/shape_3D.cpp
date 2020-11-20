namespace c3D{
    vec3 Cube::center() const{
        return (min + max) * 0.5f;
    }
    vec3 Cube::size() const{
        return max - min;
    }
}
