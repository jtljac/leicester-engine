#version 460

layout (push_constant) uniform constants {
    mat4 renderMatrix;
    vec4 colour;
} pushConstants;

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vTangent;
layout (location = 3) in vec3 vColour;
layout (location = 4) in vec2 vTexCoord;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outColour;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outTangent;
layout (location = 4) out vec2 texCoord;

layout (set = 0, binding = 0) uniform CameraBuffer {
mat4 view;
mat4 proj;
} cameraData;

layout (set = 0, binding = 1) uniform CameraMeta {
    vec3 cameraPos;
} cameraMeta;

struct ObjectData {
    mat4 model;
};

layout (std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
} objectBuffer;

void main(){
    mat4 modelMatrix = objectBuffer.objects[gl_BaseInstance].model;
    vec4 worldPosition = modelMatrix * vec4(vPosition, 1.f);
    gl_Position = cameraData.proj * cameraData.view * worldPosition;

    outPosition = vec3(worldPosition);
    outColour = vec3(pushConstants.colour);
    outNormal = vNormal;
    outTangent = vTangent;
    texCoord = vTexCoord;
}