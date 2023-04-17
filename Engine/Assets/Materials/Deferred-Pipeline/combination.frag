#version 450

const float PI = 3.14159265359;

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

struct PointLight {
    vec4 lightColour;   // w for brightness
    vec3 lightPosition;
};

layout(set=1, binding=0) uniform sampler2D positionTex;
layout(set=1, binding=1) uniform sampler2D albedoTex;
layout(set=1, binding=2) uniform sampler2D metallicRoughtnessAOTex;
layout(set=1, binding=3) uniform sampler2D normalTex;

// Hardcode lights for now
PointLight testLights[4] = {
    PointLight(vec4(1, 1, 1, 150), vec3(3, 3, 4)),
    PointLight(vec4(1, 1, 1, 150), vec3(-3, 3, 4)),
    PointLight(vec4(1, 1, 1, 150), vec3(3, -3, 4)),
    PointLight(vec4(1, 1, 1, 150), vec3(-3, -3, 4)),
};

/**
 * Calculate the fresnel effect
 * Uses the Fresnel-Schlick approximation
 * @param cosTheta The angle of incidence
 * @param F0 A precomputed base surface reflection
 * @return the percentage of light reflected by the surface
 */
vec3 fresnel(float cosTheta, vec3 F0) {
    return F0 + (1.f - F0) * pow(clamp(1.f - cosTheta, 0.f, 1.f), 5.f);
}

/**
 * Statistically approximates the surface area of microfacets exactly aligned with the given halfway vector
 * Uses the Trowbridge-Reitz GGX normal distribution function
 * @param normal The normal of the surface
 * @param halfway The vector halfway between the camera direction and the light direction from the surface
 * @param roughness The roughness of the surface
 */
float normalDistribution(vec3 normal, vec3 halfway, float roughness) {
    // Looks better with the roughness squared
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(normal, halfway), 0.f);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.f) + 1.f);
    denom = PI * denom * denom;

    return a2 / denom;
}

/**
 * Statistically approximate the relative surface area where it's microsurface-details overshadow eachother
 * Uses the Schlick-GGX Method
 */
float geometrySchlick(float NdotV, float roughness) {
    float r = roughness + 1.f;
    float k = (r * r) / 8.f;

    float denom = NdotV * (1.f - k) + k;

    return NdotV / denom;
}

/**
 * Approximate both geoetry obstruction and geometry shadowing
 * @param normal The normal of the surface
 * @param cameraDirection The direction of the camera from the surface
 * @param lighDirection The direction of the light from the surface
 * @param roughness The roughness of the surface
 */
float geometrySmith(vec3 normal, vec3 cameraDirection, vec3 lightDirection, float roughness) {
    float NdotV = max(dot(normal, cameraDirection), 0.f);
    float NdotL = max(dot(normal, lightDirection), 0.f);

    float ggx2 = geometrySchlick(NdotV, roughness);
    float ggx1 = geometrySchlick(NdotL, roughness);

    return ggx1 * ggx2;
}

void main() {
    vec4 normal = texture(normalTex, texCoord);
    vec4 albedo = texture(albedoTex, texCoord);
    vec4 worldPos = texture(positionTex, texCoord);

    vec4 metallicRoughtnessAO = texture(metallicRoughtnessAOTex, texCoord);
    float metallic = metallicRoughtnessAO.x;
    float roughness = metallicRoughtnessAO.y;
    float AO = metallicRoughtnessAO.z;


    vec3 N = normalize(normal.xyz);
    vec3 cameraDirection = normalize(cameraMeta.cameraPos - worldPos.xyz);

    // Calculate F0 for material
    vec3 F0 = mix(vec3(0.04), albedo.xyz, metallic);

    // Direct Lighting
    vec3 Lo = vec3(0.f);
    for(int i = 0; i < 4; ++i) {
        vec3 lightToPixel = testLights[i].lightPosition - worldPos.xyz;

        vec3 lightDirection = normalize(lightToPixel);
        vec3 halfwayPoint = normalize(cameraDirection + lightDirection);

        float lightDistance = length(lightToPixel);
        float attenuation = 1.f / (lightDistance * lightDistance);
        vec3 radiance = testLights[i].lightColour.xyz * testLights[i].lightColour.w * attenuation;

        vec3 F = fresnel(max(dot(halfwayPoint, cameraDirection), 0.f), F0);
        float NDF = normalDistribution(N, halfwayPoint, roughness);
        float G = geometrySmith(N, cameraDirection, lightDirection, roughness);

        // Calculate Cook Torrance BRDF
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, cameraDirection), 0.f) * max(dot(N, lightDirection), 0.f) + 0.0001f;
        vec3 specular = numerator / denominator;

        vec3 KD = (vec3(1.f) - F) * (1.f - metallic);

        float NdotL = max(dot(N, lightDirection), 0.f);
        Lo += (KD * albedo.xyz / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = /*sceneData.ambientColor.xyz * */ vec3(0.03) * albedo.xyz * AO;

    vec3 color = Lo;

    // HDR Tonemapping
//    color = color / (color + vec3(1.0));

    outFragColor = vec4(color, 1.0f);
}
