#version 330 core

uniform sampler2D tex;
uniform vec2 spriteSheetSize;

in vec3 Position;
in vec2 TexCoord;
in vec3 Normal;
in float Index;

out vec4 color;

uniform vec3 lightPosition;

void main() {
    vec2 targetCell = vec2(mod(floor(Index), spriteSheetSize.x), spriteSheetSize.y - floor(Index / spriteSheetSize.y));
    vec2 targetCellPos = targetCell / spriteSheetSize;
    vec4 texColor = texture(tex, targetCellPos + vec2(0.8) * TexCoord / vec2(spriteSheetSize.x, -spriteSheetSize.y));

    if (texColor.a < 0.1) {
        discard;
    }

    vec3 lightDir = normalize(lightPosition - Position);
    // vec3 viewDir = normalize(Position - lightPosition);
    // vec3 reflectDir = reflect(-lightDir, adjustedNormal);

    float diff = max(dot(Normal, lightDir), 0.1);

    color = vec4(vec3(texColor) * diff, 1.0);
}