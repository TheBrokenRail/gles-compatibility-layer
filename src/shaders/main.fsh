#version 300 es
precision highp float;
// Result
out vec4 frag_color;
// Texture
uniform bool u_has_texture;
uniform sampler2D u_texture_unit;
// Color
in vec4 v_color;
in vec4 v_texture_pos;
// Highlight Mode
uniform bool u_highlight_mode;
uniform vec4 u_highlight_mode_color;
// Alpha Test
uniform bool u_alpha_test;
// Fog
uniform bool u_fog;
uniform vec4 u_fog_color;
uniform bool u_fog_is_linear;
uniform float u_fog_start;
uniform float u_fog_end;
in vec4 v_fog_eye_position;
// Main
void main(void) {
    frag_color = v_color;
    // Texture
    if (u_has_texture) {
        vec4 texture_color = texture(u_texture_unit, v_texture_pos.xy);
        if (u_highlight_mode) {
            texture_color.rgb = u_highlight_mode_color.rgb;
            texture_color.a *= u_highlight_mode_color.a;
            frag_color = texture_color;
        } else {
            frag_color *= texture_color;
        }
    }
    // Fog
    if (u_fog) {
        float fog_factor;
        if (u_fog_is_linear) {
            fog_factor = (u_fog_end - length(v_fog_eye_position)) / (u_fog_end - u_fog_start);
        } else {
            fog_factor = exp(-u_fog_start * length(v_fog_eye_position));
        }
        fog_factor = clamp(fog_factor, 0.0, 1.0);
        frag_color.rgb = mix(frag_color, u_fog_color, 1.0 - fog_factor).rgb;
    }
    // Alpha Test
    if (u_alpha_test && frag_color.a <= 0.1) {
        discard;
    }
}
