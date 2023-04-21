#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColour;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec2 texCoord;

layout (location = 5) in mat3 TBN;

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

layout(set=2, binding=0) uniform sampler2D albedo;
layout(set=2, binding=1) uniform sampler2D metallic;
layout(set=2, binding=2) uniform sampler2D normal;
layout(set=2, binding=3) uniform sampler2D roughness;

void main() {
    vec3 colour = texture(albedo, texCoord).xyz;
    vec3 normal = texture(normal, texCoord).xyz;
    float metallicComp = texture(metallic, texCoord).x;
    float roughnessComp = texture(roughness, texCoord).x;

    outPosition = vec4(inPosition, 1.f);
    outAlbedo = vec4(colour, 1.f);
    outMetallicRoughnessAo = vec4(metallicComp, roughnessComp, 1, 1.f);
    outNormal = vec4(normalize(TBN * normal), 1.f);
}
