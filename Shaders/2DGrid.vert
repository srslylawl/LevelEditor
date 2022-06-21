#version 460 core
layout (location = 0) in vec3 in_vertexPos;
layout (location = 1) in vec2 in_texCoord;

out vec3 vertexPos;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    mat4 modelViewProjectionMatrix = projection*view*model;
    gl_Position = modelViewProjectionMatrix*vec4(in_vertexPos, 1.0);
    vertexPos = (model*vec4(in_vertexPos, 1.0)).xyz;
    texCoord = in_texCoord;
}

