#ifndef H_CAMERA_MATH
#define H_CAMERA_MATH

// NOTE(hugo): switching between OpenGL and DirectX coordinate systems
// ref : https://anteru.net/blog/2011/using-right-left-handed-viewing-systems-with-both-directx-opengl/
// the OpenGL right-handed coordinate system can be forced in DirectX by:
// * switching backface culling to FrontCounterClockwise
// * using clip_near = 0.f, clip_far = 1.f
// (l, c)[3][3] = (clip_far * zfar - clip_near * znear) / (zfar - znear)
// (l, c)[3][4] = (clip_far - clip_near) * zfar * znear / (zfar - znear)

// NOTE(hugo): translates p(x, y) to p'(x + vec.x, y + vec.y)
mat3 mat_translation2D(vec2 vec);
mat3 mat_orthographic2D(float screen_width, float screen_height);

// NOTE(hugo): translates p(x, y, z) to p'(x + vec.x, y + vec.y, z + vec.z)
mat4 mat_translation3D(vec3 vec);

// NOTE(hugo): assumes that the world is already centered on the camera
//             width, height, depth are the 3D camera box dimensions
mat4 mat_orthographic3D(float width, float height, float znear, float zfar, float clip_near, float clip_far);

// NOTE(hugo): wfov, hfov in radians
mat4 mat_perspective3D(float vfov, float aspect_ratio, float znear, float zfar);

// NOTE(hugo): http://www.terathon.com/gdc07_lengyel.pdf
mat4 mat_infinite_perspective3D(float vfov, float aspect_ratio, float znear);

#endif
