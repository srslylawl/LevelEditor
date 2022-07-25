#version 460 core
out vec4 FragColor;

in vec3 vertexPos;
in vec2 texCoord;

uniform vec2 mousePos;
uniform vec2 gridDimensions;

void main() {

	// Grid Display
	float gridGradient = .65;
	vec4 col;
	col.rgb = gridGradient.xxx;

	float gridDiffX = dFdx(vertexPos.x);
	float gridDiffY = dFdy(vertexPos.y);
//	vec2 vertexDistanceFromGrid = vec2(mod(vertexPos.x, gridDimensions.x), mod(vertexPos.y, gridDimensions.y));
	vec2 vertexDistanceFromGrid = mod(vertexPos.xy, gridDimensions);
	float drawGridX = vertexDistanceFromGrid.x <= gridDiffX ? 1 : 0;
	float drawGridY = vertexDistanceFromGrid.y <= gridDiffY ? 1 : 0;

	col.a = max(drawGridX, drawGridY);

	// Is Mouse is same grid cell as vertex?
	vec2 mouseDistFromGrid = mod(mousePos, gridDimensions);
	vec2 mouseGridPos = mousePos - mouseDistFromGrid;
	vec2 vertGridPos = vertexPos.xy - vertexDistanceFromGrid;
	bool isSame = vertGridPos == mouseGridPos;;
//	bool isSame = length(abs(vertGridPos - mouseGridPos)) < 0.001;
    if (isSame) {
		// Highlight Cell
		vec2 closeToEdge = abs(vertexDistanceFromGrid / gridDimensions * 2 - (1).xx);
		float exponent = 20;
		closeToEdge.x = pow(closeToEdge.x, exponent);
		closeToEdge.y = pow(closeToEdge.y, exponent);

		col.a = length(closeToEdge);

		col.rgb = (1).xxx;
	}

	FragColor = col;
}

