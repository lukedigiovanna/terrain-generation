#version 330 core

uniform sampler2D tex;

in vec3 Position;
in vec2 TexCoord;
in vec3 Normal;

out vec4 color;

uniform vec3 lightPosition;

void main() {
    vec4 texColor = texture(tex, TexCoord);

    if (texColor.a < 0.1) {
        discard;
    }

    vec3 lightDir = normalize(lightPosition - Position);
    // vec3 viewDir = normalize(Position - lightPosition);
    // vec3 reflectDir = reflect(-lightDir, adjustedNormal);

    float diff = max(dot(Normal, lightDir), 0.0);

    color = vec4(vec3(texColor) * diff, 1.0);
}