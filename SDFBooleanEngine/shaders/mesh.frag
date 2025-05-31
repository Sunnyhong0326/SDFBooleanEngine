#version 430 core

in vec3 vNormal;

out vec4 FragColor;

uniform vec3 lights[4];
uniform int numLights;

void main() {
    vec3 normal = normalize(vNormal);
    vec3 baseColor = vec3(1.0, 1.0, 1.0);
    vec3 diffuse = vec3(0.0f);

    for (int i = 0; i < numLights; ++i) {
        vec3 lightDir = normalize(lights[i]);
        float diff = max(dot(normal, lightDir), 0.0);
        diffuse += diff * baseColor * 2.0;
    }
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    FragColor = vec4(diffuse / float(numLights), 1.0);
}
