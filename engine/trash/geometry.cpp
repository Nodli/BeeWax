// NOTE(hugo):
//
//  4___2
//  |  /|
//  | / |
//  |/  |
//  0___1
//
Mesh rectangle2D(vec2 center, float width, float height){
    Mesh output;
    output.primitive_type = GL_TRIANGLE_STRIP;
    output.nvertices = 4;
    output.vertices = (float*)malloc(output.nvertices * 2 * sizeof(float));
    assert(output.vertices);

    float hwidth = width * 0.5f;
    float hheight = height * 0.5f;

    float left = center.x - hwidth;
    float right = center.x + hwidth;
    float bottom = center.y - hheight;
    float top = center.y + hheight;

    output.vertices[0] = left;
    output.vertices[1] = bottom;

    output.vertices[2] = right;
    output.vertices[3] = bottom;

    output.vertices[4] = left;
    output.vertices[5] = top;

    output.vertices[6] = right;
    output.vertices[7] = top;

    return output;
}

// NOTE(hugo): trigonometric circle ie starts at 0 and ends at 2 pi in counter-clockwise order
Mesh circle2D(vec2 center, float radius, u32 nsegments){
    assert(nsegments > 2);

    Mesh output;
    output.primitive_type = GL_TRIANGLE_FAN;
    output.nvertices = 1 + nsegments + 1; // center + nsegments + repeat first vertex for GL_TRIANGLE_FAN
    output.vertices = (float*)malloc(output.nvertices * 2 * sizeof(float));
    assert(output.vertices);

    float dtheta = bw::TWO_PI<float> / (float)nsegments;

    output.vertices[0] = center.x;
    output.vertices[1] = center.y;

    u32 ilast = 2 * (nsegments + 1);
    output.vertices[ilast] = output.vertices[2] = center.x + radius;
    output.vertices[ilast + 1] = output.vertices[3] = center.y;

    float current_theta = 0.f;
    for(u32 ivertex = 1; ivertex != nsegments; ++ivertex){
        current_theta += dtheta;
        output.vertices[2 * (ivertex + 1)] = center.x + radius * std::cos(current_theta);
        output.vertices[2 * (ivertex + 1) + 1] = center.y + radius * std::sin(current_theta);
    }

    return output;
}

Mesh segment3D(vec3 begin, vec3 end){
    Mesh output;
    output.primitive_type = GL_LINES;
    output.nvertices = 2;
    output.vertices = (float*)malloc(output.nvertices * 3 * sizeof(float));
    assert(output.vertices);

    output.vertices[0] = begin.x;
    output.vertices[1] = begin.y;
    output.vertices[2] = begin.z;

    output.vertices[3] = end.x;
    output.vertices[4] = end.y;
    output.vertices[5] = end.z;

    return output;
}

Mesh rectangle3D(vec3 center, vec3 width_vector, vec3 height_vector){
    Mesh output;
    output.primitive_type = GL_TRIANGLE_STRIP;
    output.nvertices = 4;
    output.vertices = (float*)malloc(output.nvertices * 3 * sizeof(float));
    assert(output.vertices);

    vec3 hwidth = 0.5f * width_vector;
    vec3 hheight = 0.5f * height_vector;

    vec3 bottom = center - hheight;
    vec3 top = center + hheight;

    vec3 bottom_left = bottom - hwidth;
    vec3 bottom_right = bottom + hwidth;
    vec3 top_left = top - hwidth;
    vec3 top_right = top + hwidth;

    output.vertices[0] = bottom_left.x;
    output.vertices[1] = bottom_left.y;
    output.vertices[2] = bottom_left.z;

    output.vertices[3] = bottom_right.x;
    output.vertices[4] = bottom_right.y;
    output.vertices[5] = bottom_right.z;

    output.vertices[6] = top_left.x;
    output.vertices[7] = top_left.y;
    output.vertices[8] = top_left.z;

    output.vertices[9] = top_right.x;
    output.vertices[10] = top_right.y;
    output.vertices[11] = top_right.z;

    return output;
}

Mesh debug_cube3D_position_color(vec3 center, float size){
    Mesh output;
    output.primitive_type = GL_TRIANGLES;
    output.nvertices = 36;
    output.vertices = (float*)malloc(output.nvertices * 7 * sizeof(float));
    assert(output.vertices);

    float hsize = size / 2.f;

    float data[36 * 7] = {
          // front face
        center.x - hsize,   center.y + hsize,	center.z + hsize,	1.f, 0.f, 0.f, 1.f,
        center.x - hsize,	center.y - hsize,	center.z + hsize,	1.f, 0.f, 0.f, 1.f,
        center.x + hsize,	center.y - hsize,	center.z + hsize,	1.f, 0.f, 0.f, 1.f,

        center.x - hsize,	center.y + hsize,	center.z + hsize,	.5f, 0.f, 0.f, 1.f,
        center.x + hsize,	center.y - hsize,	center.z + hsize,	.5f, 0.f, 0.f, 1.f,
        center.x + hsize,	center.y + hsize,	center.z + hsize,	.5f, 0.f, 0.f, 1.f,

        // right face
        center.x + hsize,	center.y + hsize,	center.z + hsize,	0.f, 1.f, 0.f, 1.f,
        center.x + hsize,	center.y - hsize,	center.z + hsize,	0.f, 1.f, 0.f, 1.f,
        center.x + hsize,	center.y - hsize,	center.z - hsize,	0.f, 1.f, 0.f, 1.f,

        center.x + hsize,	center.y + hsize,	center.z + hsize,	0.f, .5f, 0.f, 1.f,
        center.x + hsize,	center.y - hsize,	center.z - hsize,	0.f, .5f, 0.f, 1.f,
        center.x + hsize,	center.y + hsize,	center.z - hsize,	0.f, .5f, 0.f, 1.f,

        // back face
        center.x + hsize,	center.y + hsize,	center.z - hsize,	0.f, 0.f, 1.f, 1.f,
        center.x + hsize,	center.y - hsize,	center.z - hsize,	0.f, 0.f, 1.f, 1.f,
        center.x - hsize,	center.y - hsize,	center.z - hsize,	0.f, 0.f, 1.f, 1.f,

        center.x + hsize,	center.y + hsize,	center.z - hsize,	0.f, 0.f, .5f, 1.f,
        center.x - hsize,	center.y - hsize,	center.z - hsize,	0.f, 0.f, .5f, 1.f,
        center.x - hsize,	center.y + hsize,	center.z - hsize,	0.f, 0.f, .5f, 1.f,

        // left face
        center.x - hsize,	center.y + hsize,	center.z - hsize,	1.f, 1.f, 0.f, 1.f,
        center.x - hsize,	center.y - hsize,	center.z - hsize,	1.f, 1.f, 0.f, 1.f,
        center.x - hsize,	center.y - hsize,	center.z + hsize,	1.f, 1.f, 0.f, 1.f,

        center.x - hsize,	center.y + hsize,	center.z - hsize,	.5f, .5f, 0.f, 1.f,
        center.x - hsize,	center.y - hsize,	center.z + hsize,	.5f, .5f, 0.f, 1.f,
        center.x - hsize,	center.y + hsize,	center.z + hsize,	.5f, .5f, 0.f, 1.f,

        // bottom face
        center.x - hsize,	center.y - hsize,	center.z + hsize,	1.f, 0.f, 1.f, 1.f,
        center.x - hsize,	center.y - hsize,	center.z - hsize,	1.f, 0.f, 1.f, 1.f,
        center.x + hsize,	center.y - hsize,	center.z - hsize,	1.f, 0.f, 1.f, 1.f,

        center.x - hsize,	center.y - hsize,	center.z + hsize,	.5f, 0.f, .5f, 1.f,
        center.x + hsize,	center.y - hsize,	center.z - hsize,	.5f, 0.f, .5f, 1.f,
        center.x + hsize,	center.y - hsize,	center.z + hsize,	.5f, 0.f, .5f, 1.f,

        // top face
        center.x - hsize,	center.y + hsize,	center.z - hsize,	0.f, 1.f, 1.f, 1.f,
        center.x - hsize,	center.y + hsize,	center.z + hsize,	0.f, 1.f, 1.f, 1.f,
        center.x + hsize,	center.y + hsize,	center.z + hsize,	0.f, 1.f, 1.f, 1.f,

        center.x - hsize,	center.y + hsize,	center.z - hsize,	0.f, .5f, .5f, 1.f,
        center.x + hsize,	center.y + hsize,	center.z + hsize,	0.f, .5f, .5f, 1.f,
        center.x + hsize,	center.y + hsize,	center.z - hsize,	0.f, .5f, .5f, 1.f
    };

    memcpy(output.vertices, data, output.nvertices * 7 * sizeof(float));

    return output;
}

#if 0
std::vector<float> cube_shape(const float size){

	const float half_size = size / 2.f;

	return {
		// front face
		-half_size,	half_size,	half_size,	0.f, 0.f, 1.f,
		-half_size,	-half_size,	half_size,	0.f, 0.f, 1.f,
		half_size,	-half_size,	half_size,	0.f, 0.f, 1.f,

		-half_size,	half_size,	half_size,	0.f, 0.f, 1.f,
		half_size,	-half_size,	half_size,	0.f, 0.f, 1.f,
		half_size,	half_size,	half_size,	0.f, 0.f, 1.f,

		// right face
		half_size,	half_size,	half_size,	1.f, 0.f, 0.f,
		half_size,	-half_size,	half_size,	1.f, 0.f, 0.f,
		half_size,	-half_size,	-half_size,	1.f, 0.f, 0.f,

		half_size,	half_size,	half_size,	1.f, 0.f, 0.f,
		half_size,	-half_size,	-half_size,	1.f, 0.f, 0.f,
		half_size,	half_size,	-half_size,	1.f, 0.f, 0.f,

		// back face
		half_size,	half_size,	-half_size,	0.f, 0.f, -1.f,
		half_size,	-half_size,	-half_size,	0.f, 0.f, -1.f,
		-half_size,	-half_size,	-half_size,	0.f, 0.f, -1.f,

		half_size,	half_size,	-half_size,	0.f, 0.f, -1.f,
		-half_size,	-half_size,	-half_size,	0.f, 0.f, -1.f,
		-half_size,	half_size,	-half_size,	0.f, 0.f, -1.f,

		// left face
		-half_size,	half_size,	-half_size,	-1.f, 0.f, 0.f,
		-half_size,	-half_size,	-half_size,	-1.f, 0.f, 0.f,
		-half_size,	-half_size,	half_size,	-1.f, 0.f, 0.f,

		-half_size,	half_size,	-half_size,	-1.f, 0.f, 0.f,
		-half_size,	-half_size,	half_size,	-1.f, 0.f, 0.f,
		-half_size,	half_size,	half_size,	-1.f, 0.f, 0.f,

		// bottom face
		-half_size,	-half_size,	half_size,	0.f, -1.f, 0.f,
		-half_size,	-half_size,	-half_size,	0.f, -1.f, 0.f,
		half_size,	-half_size,	-half_size,	0.f, -1.f, 0.f,

		-half_size,	-half_size,	half_size,	0.f, -1.f, 0.f,
		half_size,	-half_size,	-half_size,	0.f, -1.f, 0.f,
		half_size,	-half_size,	half_size,	0.f, -1.f, 0.f,

		// top face
		-half_size,	half_size,	-half_size,	0.f, 1.f, 0.f,
		-half_size,	half_size,	half_size,	0.f, 1.f, 0.f,
		half_size,	half_size,	half_size,	0.f, 1.f, 0.f,

		-half_size,	half_size,	-half_size,	0.f, 1.f, 0.f,
		half_size,	half_size,	half_size,	0.f, 1.f, 0.f,
		half_size,	half_size,	-half_size,	0.f, 1.f, 0.f
	};
}

std::vector<float> hexagonal_prism_shape(const float base_height, const float base_width, const float height){

	const float hbh = base_height / 2.f; // half base height
	const float qbh = base_height / 4.f; // quarter base height
	const float hbw = base_width / 2.f; // half base width
	const float hh = height / 2.f; // half height

	const float div = 1.f / (1.f + std::sqrt(3.f));
	const float sqdiv = std::sqrt(3.f) / (1.f + std::sqrt(3.f));

	return {

		// bottom face
		0.f, - hh, hbh,  		0.f, -1.f, 0.f,
		hbw, - hh, - qbh, 		0.f, -1.f, 0.f,
		hbw, - hh, qbh,  		0.f, -1.f, 0.f,

		0.f, - hh, hbh,  		0.f, -1.f, 0.f,
		0.f, - hh, - hbh, 		0.f, -1.f, 0.f,
		hbw, - hh, - qbh, 		0.f, -1.f, 0.f,

		0.f, - hh, hbh,  		0.f, -1.f, 0.f,
		- hbw, - hh, - qbh, 		0.f, -1.f, 0.f,
		0.f, - hh, - hbh, 		0.f, -1.f, 0.f,

		0.f, - hh, hbh,  		0.f, -1.f, 0.f,
		- hbw, - hh, qbh,  	0.f, -1.f, 0.f,
		- hbw, - hh, - qbh, 		0.f, -1.f, 0.f,

		// front-right face
		0.f, hh, hbh,			div, 0.f, sqdiv,
		0.f, - hh, hbh,		div, 0.f, sqdiv,
		hbw, - hh, qbh,		div, 0.f, sqdiv,

		0.f, hh, hbh,			div, 0.f, sqdiv,
		hbw, - hh, qbh,		div, 0.f, sqdiv,
		hbw, hh, qbh,			div, 0.f, sqdiv,

		// right face
		hbw, hh, qbh,			1.f, 0.f, 0.f,
		hbw, - hh, qbh,		1.f, 0.f, 0.f,
		hbw, - hh, - qbh, 		1.f, 0.f, 0.f,

		hbw, hh, qbh,			1.f, 0.f, 0.f,
		hbw, - hh, - qbh, 		1.f, 0.f, 0.f,
		hbw, hh, - qbh, 			1.f, 0.f, 0.f,

		// back-right face
		hbw, hh, - qbh,			div, 0.f, - sqdiv,
		hbw, - hh, - qbh, 		div, 0.f, - sqdiv,
		0.f, - hh, - hbh,			div, 0.f, - sqdiv,

		hbw, hh, - qbh,			div, 0.f, - sqdiv,
		0.f, - hh, - hbh,			div, 0.f, - sqdiv,
		0.f, hh, - hbh,			div, 0.f, - sqdiv,

		// back-left face
		0.f, hh, - hbh,			- div, 0.f, - sqdiv,
		0.f, - hh, - hbh, 		- div, 0.f, - sqdiv,
		- hbw, - hh, - qbh,		- div, 0.f, - sqdiv,

		0.f, hh, - hbh,			- div, 0.f, - sqdiv,
		- hbw, - hh, - qbh,		- div, 0.f, - sqdiv,
		- hbw, hh, - qbh,			- div, 0.f, - sqdiv,

		// left face
		- hbw, hh, - qbh,			-1.f, 0.f, 0.f,
		- hbw, - hh, - qbh,		-1.f, 0.f, 0.f,
		- hbw, - hh, qbh,  	-1.f, 0.f, 0.f,

		- hbw, hh, - qbh,			-1.f, 0.f, 0.f,
		- hbw, - hh, qbh,  	-1.f, 0.f, 0.f,
		- hbw, hh, qbh,  		-1.f, 0.f, 0.f,

		// front-left face
		- hbw, hh, qbh,  		- div, 0.f, sqdiv,
		- hbw, - hh, qbh,  	- div, 0.f, sqdiv,
		0.f, - hh, hbh,  		- div, 0.f, sqdiv,

		- hbw, hh, qbh,  		- div, 0.f, sqdiv,
		0.f, - hh, hbh,  		- div, 0.f, sqdiv,
		0.f, hh, hbh,  		- div, 0.f, sqdiv,

		// top face
		0.f, hh, hbh,  		0.f, 1.f, 0.f,
		hbw, hh, qbh,  		0.f, 1.f, 0.f,
		hbw, hh, - qbh, 			0.f, 1.f, 0.f,

		0.f, hh, hbh,  		0.f, 1.f, 0.f,
		hbw, hh, - qbh, 			0.f, 1.f, 0.f,
		0.f, hh, - hbh, 			0.f, 1.f, 0.f,

		0.f, hh, hbh,  		0.f, 1.f, 0.f,
		0.f, hh, - hbh, 			0.f, 1.f, 0.f,
		- hbw, hh, - qbh, 		0.f, 1.f, 0.f,

		0.f, hh, hbh,  		0.f, 1.f, 0.f,
		- hbw, hh, - qbh, 		0.f, 1.f, 0.f,
		- hbw, hh, qbh,  		0.f, 1.f, 0.f
		};
}
#endif
