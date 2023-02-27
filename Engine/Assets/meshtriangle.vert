#version 460

layout (push_constant) uniform constants {
    mat4 renderMatrix;
} pushConstants;

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColour;
layout (location = 3) in vec3 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 texCoord;

layout (set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
} cameraData;

struct ObjectData {
    mat4 model;
};

layout (std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
} objectBuffer;

void main(){
    mat4 modelMatrix = objectBuffer.objects[gl_BaseInstance].model;
    gl_Position = cameraData.proj * cameraData.view * modelMatrix * vec4(vPosition, 1.0f);
    outColor = vNormal;
    texCoord = vTexCoord;
}