#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D minimapTexture;

void main() {
    FragColor = texture(minimapTexture, TexCoord);
}