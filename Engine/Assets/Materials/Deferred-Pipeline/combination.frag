#version 450

layout (location = 0) in vec2 texCoord;
layout (location = 0) out vec4 outFragColor;

layout(set=0, binding=1) uniform CameraMeta {
    vec3 cameraPos;
} cameraMeta;

layout(set=0, binding=2) uniform SceneData {
    vec4 fogColor; // w is for exponent
    vec4 fogDistances; //x for min, y for max, zw unused.
    vec4 ambientColor;
    vec4 sunlightDirection; //w for sun power
    vec4 sunlightColor;
} sceneData;

layout(set=1, binding=0) uniform sampler2D position;
layout(set=1, binding=1) uniform sampler2D albedo;
layout(set=1, binding=2) uniform sampler2D metallicRoughtnessAO;
layout(set=1, binding=3) uniform sampler2D normal;

void main() {
    vec3 colour = texture(albedo, texCoord).xyz;
    outFragColor = vec4(colour, 1.0f);
}
