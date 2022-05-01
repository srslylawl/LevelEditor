#version 460 core
out vec4 FragColor;

in vec3 vertexPos;
in vec2 texCoord;

void main() {
	vec4 col = vec4(0, 0, 0, 1);
	float xMod = mod(vertexPos.x, 1);
	float yMod = mod(vertexPos.y, 1);
	float x = xMod > 0.99 || yMod > 0.99 ? 1 : 0;
	float y = xMod < 0.01 || yMod < 0.01 ? 1 : 0;
	col.a = y;
	FragColor = col;
} 