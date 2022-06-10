#version 460 core
out vec4 FragColor;

in vec4 vertexColor;
in vec2 TexCoord;

uniform vec4 ourColor;

uniform sampler2D texture1;

void main() {
//    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
	FragColor = texture(texture1, TexCoord);
} 