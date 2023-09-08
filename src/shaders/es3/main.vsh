#version 300 es
precision highp float;
// Matrices
uniform mat4 u_projection;
uniform mat4 u_model_view;
uniform mat4 u_texture;
// Texture
in vec3 a_vertex_coords;
in vec2 a_texture_coords;
out vec4 v_texture_pos;
// Color
in vec4 a_color;
out vec4 v_color;
// Normal
in vec3 a_normal;
uniform float u_normal_rescale_factor;
// Lighting
uniform bool u_lighting;
uniform vec4 u_lighting_ambient;
uniform bool u_lighting_light_source_0;
uniform vec3 u_lighting_light_source_0_position;
uniform vec4 u_lighting_light_source_0_diffuse;
uniform bool u_lighting_light_source_1;
uniform vec3 u_lighting_light_source_1_position;
uniform vec4 u_lighting_light_source_1_diffuse;
// Fog
out vec4 v_fog_eye_position;
// Main
void main(void) {
    v_texture_pos = u_texture * vec4(a_texture_coords.xy, 0.0, 1.0);
    gl_Position = u_projection * u_model_view * vec4(a_vertex_coords.xyz, 1.0);
    v_color = a_color;
    v_fog_eye_position = u_model_view * vec4(a_vertex_coords.xyz, 1.0);

    // Lighting
    if (u_lighting) {
        vec4 transformed_normal = u_model_view * vec4(a_normal, 0.0) * u_normal_rescale_factor;
        vec4 total_light = u_lighting_ambient;
        if (u_lighting_light_source_0) {
            total_light += max(0.0, dot(transformed_normal.xyz, u_lighting_light_source_0_position)) * u_lighting_light_source_0_diffuse;
        }
        if (u_lighting_light_source_1) {
            total_light += max(0.0, dot(transformed_normal.xyz, u_lighting_light_source_1_position)) * u_lighting_light_source_1_diffuse;
        }
        total_light = clamp(total_light, 0.0, 1.0);
        v_color *= total_light;
    }
}
