// ---- vec2

float dot(const vec2& vA, const vec2& vB){
    return vA.x * vB.x + vA.y * vB.y;
}

float length(const vec2& vec){
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}

float sqlength(const vec2& vec){
    return vec.x * vec.x + vec.y * vec.y;
}

vec2 normalized(const vec2& vec){
    return vec / length(vec);
}

// ---- vec3

float dot(const vec3& vA, const vec3& vB){
    return vA.x * vB.x + vA.y * vB.y + vA.z * vB.z;
}

vec3 cross(const vec3& vA, const vec3& vB){
    return {vA.y * vB.z - vA.z * vB.y,
        vA.z * vB.x - vA.x * vB.z,
        vA.x * vB.y - vA.y * vB.x};
}

// NOTE(hugo): XY, ZX, YZ
vec3 wedge(const vec3& vA, const vec3& vB){
    return {vA.x * vB.y - vA.y * vB.x,
        vA.z * vB.x - vA.x * vB.z,
        vA.y * vB.z - vA.z * vB.y};
}

float length(const vec3& vec){
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

float sqlength(const vec3& vec){
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

vec3 normalized(const vec3& vec){
    return vec / length(vec);
}

// ---- vec4

float dot(const vec4& vA, const vec4& vB){
    return vA.x * vB.x + vA.y * vB.y + vA.z * vB.z + vA.w * vB.w;
}

float length(const vec4& vec){
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
}

float sqlength(const vec4& vec){
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
}

vec4 normalize(const vec4& vec){
    return vec / length(vec);
}
