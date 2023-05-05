/**
 * The implementations for this shader has been created by following these tutorials:
 * https://learnopengl.com/PBR/Theory
 * https://learnopengl.com/PBR/Lighting
 * https://github.com/SaschaWillems/Vulkan/tree/master/data/shaders/glsl/pbrbasic
 *
 * Their implementations have been mixed and optimised, with edits made to make the implementation fit with this project
 */

#version 450

layout (location = 0) out vec2 outUv;

layout (set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
} cameraData;

layout(set=0, binding=1) uniform CameraMeta {
    vec3 cameraPos;
} cameraMeta;

void main() {
    outUv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUv * 2.0f - 1.0f, 0.0f, 1.0f);
}