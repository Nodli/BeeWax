#ifndef H_CAMERA_MATH
#define H_CAMERA_MATH

// NOTES(hugo):
// * right-handed coordinate system in world space
// * left-handed  coordinate system in clip  space
// * clipping panes at ([-1, 1], [-1, 1], [0, 1])
//   ie requires glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE)

// REF(hugo):
// http://www.codinglabs.net/article_world_view_projection_matrix.aspx
// https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/

mat4 mat3D_from_mat2D(const mat3& mat);

// NOTE(hugo): translates p(x, y) to p'(x + vec.x, y + vec.y)
mat3 mat_translation2D(vec2 vec);
// NOTE(hugo): scales p(x, y) to p'(x * vec.x, y * vec.y)
mat3 mat_scaling2D(vec2 vec);

// NOTE(hugo): translates p(x, y, z) to p'(x + vec.x, y + vec.y, z + vec.z)
mat4 mat_translation3D(vec3 vec);

// NOTE(hugo): assumes that the world is already centered on the camera
//             width, height, depth are the 3D camera box dimensions
mat4 mat_orthographic3D(float width, float height, float znear, float zfar, float clip_near, float clip_far);

// NOTE(hugo): wfov, hfov in radians
mat4 mat_perspective3D(float vfov, float aspect_ratio, float znear, float zfar);

// REF(hugo): http://www.terathon.com/gdc07_lengyel.pdf
mat4 mat_infinite_perspective3D(float vfov, float aspect_ratio, float znear);

#endif
