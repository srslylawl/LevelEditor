#version 460 core
out vec4 FragColor;

in vec3 vertexPos;
in vec2 texCoord;

uniform vec2 mousePos;

float drawGridLine(float thickness, float pos, float p) {
	float f = fract(pos+(thickness/2)) / thickness;
	f = clamp(f, 0, 1)* 2 - 1;
	f = pow(f, p);
	return 1-(abs(f));
}

void main() {

	// Grid Display
	float gridGradient = .65;
	vec4 col;
	col.rgb = gridGradient.xxx;
	float thickness = 0.01f;
	float exponent = 1;
	float gridDvX = length(vec2(dFdx(vertexPos.x), dFdy(vertexPos.x))) + 0.01;
	float gridDvY = length(vec2(dFdx(vertexPos.y), dFdy(vertexPos.y))) + 0.01;
	float gridX = drawGridLine(max(thickness, gridDvX), vertexPos.x, exponent);
	float gridY = drawGridLine(max(thickness, gridDvY), vertexPos.y, exponent);
	col.a = max(gridX, gridY);

	// Is Mouse is same grid cell as vertex?
	vec2 cellDiff = min(vec2(1), abs(floor(vertexPos.xy) - floor(mousePos)));
	float isSame = 1-length(cellDiff); // + gridX + gridY;
    if (isSame == 1) {
		// Highlight Cell
		vec2 closeToEdge = abs(fract(vertexPos.xy) * 2 - (1).xx);
		float exponent = 20;
		closeToEdge.x = pow(closeToEdge.x, exponent);
		closeToEdge.y = pow(closeToEdge.y, exponent);
		col.a = length(closeToEdge);

		col.rgb = (1).xxx;
	}

	FragColor = col;
}

