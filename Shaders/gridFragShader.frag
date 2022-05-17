#version 460 core
out vec4 FragColor;

in vec3 vertexPos;
in vec2 texCoord;

float drawGridLine(float thickness, float pos, float p) {
	float f = fract(pos+(thickness/2)) / thickness;
	f = clamp(f, 0, 1)* 2 - 1;
	f = pow(f, p);
	return 1-(abs(f));
}

void main() {
	vec4 col = vec4(0, 0, 0, 1);
	float thickness = 0.01f;
	float exponent = 1;
	float gridDvX = length(vec2(dFdx(vertexPos.x), dFdy(vertexPos.x))) + 0.01;
	float gridDvY = length(vec2(dFdx(vertexPos.y), dFdy(vertexPos.y))) + 0.01;
	float gridX = drawGridLine(max(thickness, gridDvX), vertexPos.x, exponent);
	float gridY = drawGridLine(max(thickness, gridDvY), vertexPos.y, exponent);
	col.a = max(gridX, gridY);

	FragColor = col;
}

