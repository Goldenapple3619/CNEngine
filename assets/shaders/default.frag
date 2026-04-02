#version 330 core
in vec3 v_normal;
in vec2 v_uv;

uniform vec4      u_color;
uniform float     u_ambient;
uniform float     u_diffuse;
uniform sampler2D u_texture;
uniform int       u_has_texture;

out vec4 frag_color;

void main() {
    vec4 base = u_has_texture == 1
        ? texture(u_texture, v_uv)
        : u_color;

    // basic directional light from above-front
    vec3  light_dir = normalize(vec3(0.3, 1.0, 0.5));
    float diff      = max(dot(normalize(v_normal), light_dir), 0.0);
    float lighting  = u_ambient + u_diffuse * diff;

    frag_color = vec4(base.rgb * lighting, base.a);
}