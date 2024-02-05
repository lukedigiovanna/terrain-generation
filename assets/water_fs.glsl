#version 330 core

in vec2 TexCoord;
in vec3 Position;

out vec4 color;

uniform float t;

void main() {
    float b = (cos(Position.x * 0.3 + t * 0.2) + sin(Position.y * 0.1 + t * 0.9 + 0.3)) * 0.5 + 0.5;
    color = vec4(0.0, b * 0.2 + 0.2, b * 0.1 + 0.86, 0.6);
}