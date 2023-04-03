#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColour;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 texCoord;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outAlbedo;
layout (location = 2) out vec4 outMetallicRoughnessAo;
layout (location = 3) out vec4 outNormal;

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

layout(set=2, binding=0) uniform sampler2D tex1;

void main() {
    vec3 colour = texture(tex1, texCoord).xyz;

    outPosition = vec4(inPosition, 1.0f);
    outAlbedo = vec4(colour, 1.0f);
    outMetallicRoughnessAo = vec4(0.f, 1.f, 1.f, 1.0f);
    outNormal = vec4(inNormal, 1.0f);
}
