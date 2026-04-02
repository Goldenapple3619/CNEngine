#version 330 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec3 v_normal;
out vec2 v_uv;

void main() {
    gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);
    v_normal = a_normal;
    v_uv = a_uv;
}