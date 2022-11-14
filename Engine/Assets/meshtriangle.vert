#version 450

layout (push_constant) uniform constants {
    mat4 renderMatrix;
} pushConstants;

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColour;

layout (location = 0) out vec3 outColor;

layout (set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
} cameraData;

void main(){
    gl_Position = cameraData.proj * cameraData.view * pushConstants.renderMatrix * vec4(vPosition, 1.0f);
    outColor = vColour;
}