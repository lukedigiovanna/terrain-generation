#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;
out vec3 Position;
out vec3 Normal;

void main() {
    TexCoord = inTexCoord;
    Normal = mat3(transpose(inverse(model))) * inNormal;
    Position = (model * vec4(inPos, 1.0)).xyz;
    gl_Position = projection * view * model * vec4(inPos, 1.0);
}